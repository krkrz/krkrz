//---------------------------------------------------------------------------
/*
	Risa [りさ]      alias 吉里吉里3 [kirikiri-3]
	 stands for "Risa Is a Stagecraft Architecture"
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
//! @file
//! @brief Waveセグメント/ラベルキュー管理
//---------------------------------------------------------------------------
#ifndef WAVESEGMENTH
#define WAVESEGMENTH


#include <deque>


/*[*/
//---------------------------------------------------------------------------
//! @brief 再生セグメント情報
//---------------------------------------------------------------------------
#pragma pack(push, 4)
struct tTVPWaveSegment
{
	//! @brief コンストラクタ
	tTVPWaveSegment(tjs_int64 start, tjs_int64 length)
		{ Start = start; Length = FilteredLength = length; }
	tTVPWaveSegment(tjs_int64 start, tjs_int64 length, tjs_int64 filteredlength)
		{ Start = start; Length = length; FilteredLength = filteredlength; }
	tjs_int64 Start; //!< オリジナルデコーダ上でのセグメントのスタート位置 (PCM サンプルグラニュール数単位)
	tjs_int64 Length; //!< オリジナルデコーダ上でのセグメントの長さ (PCM サンプルグラニュール数単位)
	tjs_int64 FilteredLength; //!< フィルタ後の長さ (PCM サンプルグラニュール数単位)
};
#pragma pack(pop)
//---------------------------------------------------------------------------


/*]*/
/*[*/
//---------------------------------------------------------------------------
//! @brief 再生ラベル情報
//---------------------------------------------------------------------------
#pragma pack(push, 4)
struct tTVPWaveLabel
{
	//! @brief コンストラクタ
	tjs_int64 Position; //!< オリジナルデコーダ上でのラベル位置 (PCM サンプルグラニュール数単位)
	ttstr Name; //!< ラベル名
	tjs_int Offset;
		/*!< オフセット
			@note
			This member will be set in tTVPWaveLoopManager::Render,
			and will contain the sample granule offset from first decoding
			point at call of tTVPWaveLoopManager::Render().
		*/
/*]*/

#ifdef TVP_IN_LOOP_TUNER
	// these are only used by the loop tuner
	tjs_int NameWidth; // display name width
	tjs_int Index; // index
#endif

	struct tSortByPositionFuncObj
	{
		bool operator()(
			const tTVPWaveLabel &lhs,
			const tTVPWaveLabel &rhs) const
		{
			return lhs.Position < rhs.Position;
		}
	};

	struct tSortByOffsetFuncObj
	{
		bool operator()(
			const tTVPWaveLabel &lhs,
			const tTVPWaveLabel &rhs) const
		{
			return lhs.Offset < rhs.Offset;
		}
	};

#ifdef TVP_IN_LOOP_TUNER
	struct tSortByIndexFuncObj
	{
		bool operator()(
			const tTVPWaveLabel &lhs,
			const tTVPWaveLabel &rhs) const
		{
			return lhs.Index < rhs.Index;
		}
	};
#endif

	//! @brief コンストラクタ
	tTVPWaveLabel()
	{
		Position = 0;
		Offset = 0;
#ifdef TVP_IN_LOOP_TUNER
		NameWidth = 0;
		Index = 0;
#endif
	}

	//! @brief コンストラクタ
	tTVPWaveLabel(tjs_int64 position, const ttstr & name, tjs_int offset)
		: Position(position), Name(name), Offset(offset)
	{
#ifdef TVP_IN_LOOP_TUNER
		NameWidth = 0;
		Index = 0;
#endif
	}
/*[*/
};
#pragma pack(pop)
/*]*/
//---------------------------------------------------------------------------
bool inline operator < (const tTVPWaveLabel & lhs, const tTVPWaveLabel & rhs)
{
	return lhs.Position < rhs.Position;
}
//---------------------------------------------------------------------------
/*[*/




//---------------------------------------------------------------------------
//! @brief Waveのセグメント・ラベルのキューを管理するインターフェース
//---------------------------------------------------------------------------
class iTVPWaveSegmentQueue
{
public:
	virtual ~iTVPWaveSegmentQueue() { ; }

	//! @brief		内容をクリアする
	virtual void Clear() = 0;

	//! @brief		tTVPWaveSegmentQueueをエンキューする
	//! @param		queue		エンキューしたいtTVPWaveSegmentQueueオブジェクト
	virtual void Enqueue(const tTVPWaveSegmentQueue & queue) = 0;

	//! @brief		tTVPWaveSegmentをエンキューする
	//! @param		queue		エンキューしたいtTVPWaveSegmentオブジェクト
	virtual void Enqueue(const tTVPWaveSegment & segment) = 0;

	//! @brief		tTVPWaveLabelをエンキューする
	//! @param		queue		エンキューしたいtTVPWaveLabelオブジェクト
	//! @note		Offset は修正されないので注意
	virtual void Enqueue(const tTVPWaveLabel & Label) = 0;

	//! @brief		先頭から指定長さ分をデキューする
	//! @param		dest		格納先キュー(内容はクリアされる)
	//! @param		length		切り出す長さ(サンプルグラニュール単位)
	virtual void Dequeue(tTVPWaveSegmentQueue & dest, tjs_int64 length) = 0;

	//! @brief		このキューの全体の長さを得る
	//! @return		このキューの長さ (サンプルグラニュール単位)
	virtual tjs_int64 GetFilteredLength() const = 0;

	//! @brief		このキューの長さを変化させる
	//! @param		new_total_filtered_length 新しいキューの長さ (サンプルグラニュール単位)
	//! @note		キュー中のSegments などの長さや Labelsの位置は線形補間される
	virtual void Scale(tjs_int64 new_total_length) = 0;

	//! @brief		フィルタされた位置からデコード位置へ変換を行う
	//! @param		pos フィルタされた位置
	//! @note		デコード位置
	virtual tjs_int64 FilteredPositionToDecodePosition(tjs_int64 pos) const = 0;
};
//---------------------------------------------------------------------------
/*]*/
#ifdef __TP_STUB_H__
/*[*/
class tTVPWaveSegmentQueue : public iTVPWaveSegmentQueue { };
/*]*/
#endif

//---------------------------------------------------------------------------
//! @brief Waveのセグメント・ラベルのキューを管理するクラス
//---------------------------------------------------------------------------
class tTVPWaveSegmentQueue : public iTVPWaveSegmentQueue
{
	// deque による Segments と Labels の配列。
	// 実用上は、これらの配列に大量のデータが入ることはまずないので
	// vector で十分なのかもしれないが ...
	std::deque<tTVPWaveSegment> Segments; //!< セグメントの配列
	std::deque<tTVPWaveLabel> Labels; //!< ラベルの配列

public:
	// iTVPWaveSegmentQueue intarfaces
	virtual void Clear();
	virtual void Enqueue(const tTVPWaveSegmentQueue & queue);
	virtual void Enqueue(const tTVPWaveSegment & segment);
	virtual void Enqueue(const tTVPWaveLabel & Label);
	virtual void Dequeue(tTVPWaveSegmentQueue & dest, tjs_int64 length);
	virtual tjs_int64 GetFilteredLength() const;
	virtual void Scale(tjs_int64 new_total_length);
	virtual tjs_int64 FilteredPositionToDecodePosition(tjs_int64 pos) const;

	//! @brief		セグメントの配列を得る
	//! @return		セグメントの配列
	const std::deque<tTVPWaveSegment> & GetSegments() const { return Segments; }

	//! @brief		ラベルの配列を得る
	//! @return		ラベルの配列
	const std::deque<tTVPWaveLabel> & GetLabels() const { return Labels; }

	//! @brief		tTVPWaveSegmentの配列をエンキューする
	//! @param		queue		エンキューしたい std::dequeue<tTVPWaveSegment>オブジェクト
	void Enqueue(const std::deque<tTVPWaveSegment> & segments);

	//! @brief		tTVPWaveLabelの配列をエンキューする
	//! @param		queue		エンキューしたい std::dequeue<tTVPWaveLabel>オブジェクト
	void Enqueue(const std::deque<tTVPWaveLabel> & Labels);
};

#endif

