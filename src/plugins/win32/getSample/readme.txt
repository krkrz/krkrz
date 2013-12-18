Title: getSampleプラグイン
Author: わたなべごう，miahmie

●これはなに？

WaveSoundBuffer に口パクアニメ用のためのサンプル取得の機能を追加します。


●使い方

manual.tjs 参照


●使用例（新方式）

	// soundBuffer : 声を鳴らしているサウンドバッファ
	var voiceLevel; // 口パクレベル
	var a = soundBuffer.sampleValue;
	//dm("ボイス値: "+soundBuffer.position+" : %0.3f".sprintf(a));
	if      (a > 0.3 ) voiceLevel = 2;
	else if (a > 0.03) voiceLevel = 1;
	else               voiceLevel = 0;
	// voiceLevelにあわせて口のアニメを設定


●ライセンス

このプラグインのライセンスは吉里吉里本体に準拠してください。
