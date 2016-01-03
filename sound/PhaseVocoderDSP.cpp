//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Phase Vocoder の実装
//---------------------------------------------------------------------------

/*
	Phase Vocoder (フェーズ ボコーダ ; 位相ボコーダ)の実装

	参考資料:

		http://www.panix.com/~jens/pvoc-dolson.par
			Phase Vocoder のチュートリアル。「ミュージシャンにもわかるように」
			書かれており、数学音痴フレンドリー。

		http://www.dspdimension.com/
			無料(オープンソースではない)の Time Stretcher/Pitch Shifterの
			DIRACや、各種アルゴリズムの説明、
			Pitch Shifter の説明的なソースコードなど。

		http://soundlab.cs.princeton.edu/software/rt_pvc/
			real-time phase vocoder analysis/synthesis library + visualization
			ソースあり。
*/

#include "tjsCommHead.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "PhaseVocoderDSP.h"
#include <string.h>

#include "tjsUtils.h"

#include "tvpgl_ia32_intf.h"
//extern tjs_uint32 TVPCPUType;
#include "DetectCPU.h"

extern "C"
{
#ifdef _WIN32
#define TVP_CDECL __cdecl
#endif
	
#ifndef TJS_64BIT_OS
void TVP_CDECL sse__ZN20tRisaPhaseVocoderDSP11ProcessCoreEi(tRisaPhaseVocoderDSP *_this, int ch);
void TVP_CDECL def__ZN20tRisaPhaseVocoderDSP11ProcessCoreEi(tRisaPhaseVocoderDSP *_this, int ch);
void TVP_CDECL sse__Z31RisaInterleaveOverlappingWindowPfPKPKfS_ijj(float * dest, const float * const * src,
					float * win, int numch, size_t srcofs, size_t len);
void TVP_CDECL def__Z31RisaInterleaveOverlappingWindowPfPKPKfS_ijj(float * dest, const float * const * src,
					float * win, int numch, size_t srcofs, size_t len);
void TVP_CDECL sse__Z30RisaDeinterleaveApplyingWindowPPfPKfS_ijj(float * dest[], const float * src,
					float * win, int numch, size_t destofs, size_t len);
void TVP_CDECL def__Z30RisaDeinterleaveApplyingWindowPPfPKfS_ijj(float * dest[], const float * src,
					float * win, int numch, size_t destofs, size_t len);
#endif
}

//---------------------------------------------------------------------------
tRisaPhaseVocoderDSP::tRisaPhaseVocoderDSP(
				unsigned int framesize,
				unsigned int frequency, unsigned int channels) :
					InputBuffer(framesize * 4 * channels),
					OutputBuffer(framesize * 4 * channels)
		// InputBuffer は最低でも
		// channels * (framesize + (framesize/oversamp)) 必要で、
		// OutputBuffer は最低でも
		// channels * (framesize + (framesize/oversamp)*MAX_TIME_SCALE) 必要
{
	// フィールドの初期化
	FFTWorkIp = NULL;
	FFTWorkW = NULL;
	InputWindow = NULL;
	OutputWindow = NULL;
	AnalWork = NULL;
	SynthWork = NULL;
	LastAnalPhase = NULL;
	LastSynthPhase = NULL;

	FrameSize = framesize;
	OverSampling = 8;
	Frequency = frequency;
	Channels = channels;
	InputHopSize = OutputHopSize = FrameSize / OverSampling;

	TimeScale = 1.0;
	FrequencyScale = 1.0;
	RebuildParams = true; // 必ず初回にパラメータを再構築するように真

	LastSynthPhaseAdjustCounter = 0;

	try
	{
		// ワークなどの確保
		AnalWork  = (float **)TJSAlignedAlloc(sizeof(float *) * Channels, 4);
		SynthWork = (float **)TJSAlignedAlloc(sizeof(float *) * Channels, 4);
		for(unsigned int ch = 0; ch < Channels; ch++)
			AnalWork[ch] = NULL, SynthWork[ch] = NULL;
		for(unsigned int ch = 0; ch < Channels; ch++)
		{
			AnalWork[ch]  = (float *)TJSAlignedAlloc(sizeof(float) * (FrameSize), 4);
			SynthWork[ch] = (float *)TJSAlignedAlloc(sizeof(float) * (FrameSize), 4);
		}

		LastAnalPhase = (float **)TJSAlignedAlloc(sizeof(float *) * Channels, 4);
		for(unsigned int ch = 0; ch < Channels; ch++)
			LastAnalPhase[ch] = NULL;
		for(unsigned int ch = 0; ch < Channels; ch++)
		{
			LastAnalPhase[ch] = (float *)TJSAlignedAlloc(sizeof(float) * (FrameSize/2), 4);
			memset(LastAnalPhase[ch], 0, FrameSize/2 * sizeof(float)); // 0 でクリア
		}

		LastSynthPhase = (float **)TJSAlignedAlloc(sizeof(float *) * Channels, 4);
		for(unsigned int ch = 0; ch < Channels; ch++)
			LastSynthPhase[ch] = NULL;
		for(unsigned int ch = 0; ch < Channels; ch++)
		{
			LastSynthPhase[ch] = (float *)TJSAlignedAlloc(sizeof(float) * (FrameSize/2), 4);
			memset(LastSynthPhase[ch], 0, FrameSize/2 * sizeof(float)); // 0 でクリア
		}

		FFTWorkIp = (int *)TJSAlignedAlloc(sizeof(int) * (static_cast<int>(2+sqrt((double)FrameSize/4))), 4);
		FFTWorkIp[0] = FFTWorkIp[1] = 0;
		FFTWorkW = (float *)TJSAlignedAlloc(sizeof(float) * (FrameSize/2), 4);
		InputWindow = (float *)TJSAlignedAlloc(sizeof(float) * FrameSize, 4);
		OutputWindow = (float *)TJSAlignedAlloc(sizeof(float) * FrameSize, 4);
	}
	catch(...)
	{
		Clear();
		throw;
	}

	// 入出力バッファの内容をクリア
	float *bufp1;
	size_t buflen1;
	float *bufp2;
	size_t buflen2;

	InputBuffer.GetWritePointer(InputBuffer.GetSize(),
							bufp1, buflen1, bufp2, buflen2);
	if(bufp1) memset(bufp1, 0, sizeof(float)*buflen1);
	if(bufp2) memset(bufp2, 0, sizeof(float)*buflen2);

	OutputBuffer.GetWritePointer(OutputBuffer.GetSize(),
							bufp1, buflen1, bufp2, buflen2);
	if(bufp1) memset(bufp1, 0, sizeof(float)*buflen1);
	if(bufp2) memset(bufp2, 0, sizeof(float)*buflen2);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisaPhaseVocoderDSP::~tRisaPhaseVocoderDSP()
{
	Clear();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisaPhaseVocoderDSP::SetTimeScale(float v)
{
	if(TimeScale != v)
	{
		TimeScale = v;
		RebuildParams = true;
		InputHopSize = OutputHopSize = FrameSize / OverSampling;
		OutputHopSize = static_cast<unsigned int>(InputHopSize * TimeScale) & ~1;
			// ↑ 偶数にアライン(重要)
			// 複素数 re,im, re,im, ... の配列が逆FFTにより同数の(複素数の個数×2の)
			// PCMサンプルに変換されるため、PCMサンプルも２個ずつで扱わないとならない.
			// この実際の OutputHopSize に従って ExactTimeScale が計算される.
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisaPhaseVocoderDSP::SetFrequencyScale(float v)
{
	if(FrequencyScale != v)
	{
		FrequencyScale = v;
		RebuildParams = true;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisaPhaseVocoderDSP::SetOverSampling(unsigned int v)
{
	if(v == 0)
	{
		// TimeScale に従って値を設定
		// これらの閾値は実際のリスニングにより決定された数値であり、
		// 論理的な根拠はない。
		if(TimeScale <= 0.2) v = 2;
		else if(TimeScale <= 1.2) v = 4;
		else v = 8;
	}

	if(OverSampling != v)
	{
		OverSampling = v;
		InputHopSize = OutputHopSize = FrameSize / OverSampling;
		OutputHopSize = static_cast<unsigned int>(InputHopSize * TimeScale) & ~1;
		// ここのOutputHopSizeの計算については tRisaPhaseVocoderDSP::SetTimeScale
		// も参照のこと
		RebuildParams = true;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void tRisaPhaseVocoderDSP::Clear()
{
	// 全てのバッファなどを解放する
	if(AnalWork)
	{
		for(unsigned int ch = 0; ch < Channels; ch++)
			TJSAlignedDealloc(AnalWork[ch]), AnalWork[ch] = NULL;
		TJSAlignedDealloc(AnalWork), AnalWork = NULL;
	}
	if(SynthWork)
	{
		for(unsigned int ch = 0; ch < Channels; ch++)
			TJSAlignedDealloc(SynthWork[ch]), SynthWork[ch] = NULL;
		TJSAlignedDealloc(SynthWork), SynthWork = NULL;
	}
	if(LastAnalPhase)
	{
		for(unsigned int ch = 0; ch < Channels; ch++)
			TJSAlignedDealloc(LastAnalPhase[ch]), LastAnalPhase[ch] = NULL;
		TJSAlignedDealloc(LastAnalPhase), LastAnalPhase = NULL;
	}
	if(LastSynthPhase)
	{
		for(unsigned int ch = 0; ch < Channels; ch++)
			TJSAlignedDealloc(LastSynthPhase[ch]), LastSynthPhase[ch] = NULL;
		TJSAlignedDealloc(LastSynthPhase), LastSynthPhase = NULL;
	}
	TJSAlignedDealloc(FFTWorkIp), FFTWorkIp = NULL;
	TJSAlignedDealloc(FFTWorkW), FFTWorkW = NULL;
	TJSAlignedDealloc(InputWindow), InputWindow = NULL;
	TJSAlignedDealloc(OutputWindow), OutputWindow = NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
size_t tRisaPhaseVocoderDSP::GetInputFreeSize()
{
	return InputBuffer.GetFreeSize() / Channels;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisaPhaseVocoderDSP::GetInputBuffer(
	size_t numsamplegranules,
	float * & p1, size_t & p1size,
	float * & p2, size_t & p2size)
{
	size_t numsamples = numsamplegranules * Channels;

	if(InputBuffer.GetFreeSize() < numsamples) return false; // 十分な空き容量がない

	InputBuffer.GetWritePointer(numsamples, p1, p1size, p2, p2size);

	p1size /= Channels;
	p2size /= Channels;

	InputBuffer.AdvanceWritePos(numsamples);

	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
size_t tRisaPhaseVocoderDSP::GetOutputReadySize()
{
	return OutputBuffer.GetDataSize() / Channels;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool tRisaPhaseVocoderDSP::GetOutputBuffer(
	size_t numsamplegranules,
	const float * & p1, size_t & p1size,
	const float * & p2, size_t & p2size)
{
	size_t numsamples = numsamplegranules * Channels;

	if(OutputBuffer.GetDataSize() < numsamples) return false; // 十分な準備済みサンプルがない

	OutputBuffer.GetReadPointer(numsamples, p1, p1size, p2, p2size);

	p1size /= Channels;
	p2size /= Channels;

	OutputBuffer.AdvanceReadPos(numsamples);

	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tRisaPhaseVocoderDSP::tStatus tRisaPhaseVocoderDSP::Process()
{
	
#ifndef TJS_64BIT_OS
	bool use_sse =
			(TVPCPUType & TVP_CPU_HAS_MMX) &&
			(TVPCPUType & TVP_CPU_HAS_SSE) &&
			(TVPCPUType & TVP_CPU_HAS_CMOV);


	// パラメータの再計算の必要がある場合は再計算をする
	if(RebuildParams)
	{
		// 窓関数の計算(ここではVorbis I 窓)
		float recovery_of_loss_of_vorbis_window = 2.0;
				//         1            1         2
				//  2  =  ∫  1dx  /   ∫   vorbis (x) dx
				//         0            0
				// where vobis = vorbis I window function
		float output_volume =
			TimeScale / FrameSize  / sqrt(FrequencyScale) / OverSampling * 2 *
											recovery_of_loss_of_vorbis_window;
		for(unsigned int i = 0; i < FrameSize; i++)
		{
			double x = ((double)i+0.5)/FrameSize;
			double window = sin(M_PI/2*sin(M_PI*x)*sin(M_PI*x));
			InputWindow[i]  = (float)(window);
			OutputWindow[i] = (float)(window *output_volume);
		}

		// そのほかのパラメータの再計算
		OverSamplingRadian = (float)((2.0*M_PI)/OverSampling);
		OverSamplingRadianRecp = (float)(1.0/OverSamplingRadian);
		FrequencyPerFilterBand = (float)((double)Frequency/FrameSize);
		FrequencyPerFilterBandRecp = (float)(1.0/FrequencyPerFilterBand);
		ExactTimeScale = (float)OutputHopSize / InputHopSize;

		// フラグを倒す
		RebuildParams = false;
	}

	// 入力バッファ内のデータは十分か？
	if(InputBuffer.GetDataSize() < FrameSize * Channels)
		return psInputNotEnough; // 足りない

	// 出力バッファの空きは十分か？
	if(OutputBuffer.GetFreeSize() < FrameSize * Channels)
		return psOutputFull; // 足りない

	// これから書き込もうとする OutputBuffer の領域の最後の OutputHopSize サンプル
	// グラニュールは 0 で埋める (オーバーラップ時にはみ出す部分なので)
	{
		float *p1, *p2;
		size_t p1len, p2len;

		OutputBuffer.GetWritePointer(OutputHopSize*Channels,
				p1, p1len, p2, p2len, (FrameSize - OutputHopSize)*Channels);
		memset(p1, 0, p1len * sizeof(float));
		if(p2) memset(p2, 0, p2len * sizeof(float));
	}

	// 窓関数を適用しつつ、入力バッファから AnalWork に読み込む
	{
		const float *p1, *p2;
		size_t p1len, p2len;
		InputBuffer.GetReadPointer(FrameSize*Channels, p1, p1len, p2, p2len);
		p1len /= Channels;
		p2len /= Channels;
		(use_sse?
		sse__Z30RisaDeinterleaveApplyingWindowPPfPKfS_ijj:
		def__Z30RisaDeinterleaveApplyingWindowPPfPKfS_ijj)
			(AnalWork, p1, InputWindow, Channels, 0, p1len);

		if(p2)
			(use_sse?
			sse__Z30RisaDeinterleaveApplyingWindowPPfPKfS_ijj:
			def__Z30RisaDeinterleaveApplyingWindowPPfPKfS_ijj)
				(AnalWork, p2, InputWindow + p1len, Channels, p1len, p2len);
	}

	// チャンネルごとに処理
	for(unsigned int ch = 0; ch < Channels; ch++)
	{
		//------------------------------------------------
		// 解析
		//------------------------------------------------

		// 演算の根幹部分を実行する
			(use_sse?
			sse__ZN20tRisaPhaseVocoderDSP11ProcessCoreEi:
			def__ZN20tRisaPhaseVocoderDSP11ProcessCoreEi)
				(this, ch);
	}

	// 窓関数を適用しつつ、SynthWork から出力バッファに書き込む
	{
		float *p1, *p2;
		size_t p1len, p2len;

		OutputBuffer.GetWritePointer(FrameSize*Channels, p1, p1len, p2, p2len);
		p1len /= Channels;
		p2len /= Channels;
		(use_sse?
		sse__Z31RisaInterleaveOverlappingWindowPfPKPKfS_ijj:
		def__Z31RisaInterleaveOverlappingWindowPfPKPKfS_ijj)
			(p1, SynthWork, OutputWindow, Channels, 0, p1len);
		if(p2)
			(use_sse?
			sse__Z31RisaInterleaveOverlappingWindowPfPKPKfS_ijj:
			def__Z31RisaInterleaveOverlappingWindowPfPKPKfS_ijj)
				(p2, SynthWork, OutputWindow + p1len, Channels, p1len, p2len);
	}

	// LastSynthPhase を再調整するか
	LastSynthPhaseAdjustCounter += LastSynthPhaseAdjustIncrement;
	if(LastSynthPhaseAdjustCounter >= LastSynthPhaseAdjustInterval)
	{
		// LastSynthPhase を再調整するカウントになった
		LastSynthPhaseAdjustCounter = 0;

		// ここで行う調整は LastSynthPhase の unwrapping である。
		// LastSynthPhase は位相の差が累積されるので大きな数値になっていくが、
		// 適当な間隔でこれを unwrapping しないと、いずれ(数値が大きすぎて)精度
		// 落ちが発生し、正常に合成が出来なくなってしまう。
		// ただし、精度が保たれればよいため、毎回この unwrapping を行う必要はない。
		// ここでは LastSynthPhaseAdjustInterval/LastSynthPhaseAdjustIncrement 回ごとに調整を行う。
		for(unsigned int ch = 0; ch < Channels; ch++)
		{
			unsigned int framesize_d2 = FrameSize / 2;
			for(unsigned int i = 0; i < framesize_d2; i++)
			{
				long int n = static_cast<long int>(LastSynthPhase[ch][i] / (2.0*M_PI));
				LastSynthPhase[ch][i] -= static_cast<float>(n * (2.0*M_PI));
			}
		}
	}

	// 入出力バッファのポインタを進める
	OutputBuffer.AdvanceWritePos(OutputHopSize * Channels);
	InputBuffer.AdvanceReadPos(InputHopSize * Channels);
	
#endif
	// ステータス = no error
	return psNoError;
}
//---------------------------------------------------------------------------
