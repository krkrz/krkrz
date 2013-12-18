#ifndef __LayerExImage__
#define __LayerExImage__

#include "../layerExDraw/LayerExBase.hpp"

/**
 * レイヤ拡張 イメージ操作用ネイティブインスタンス
 */
class layerExImage : public layerExBase
{
public:
	// コンストラクタ
	layerExImage(DispatchT obj) : layerExBase(obj) {}

	virtual void reset();
	
	/**
	 * ルックアップテーブル反映
	 * @param pLut lookup table
	 */
	void lut(BYTE* pLut);
	
	/**
	 * 明度とコントラスト
	 * @param brightness 明度 -255 ～ 255, 負数の場合は暗くなる
	 * @param contrast コントラスト -100 ～100, 0 の場合変化しない
	 */
	void light(int brightness, int contrast);

	/**
	 * 色相と彩度を適応
	 * @param hue 色相
	 * @param sat 彩度
	 * @param blend ブレンド 0 (効果なし) ～ 1 (full effect)
	 */
	void colorize(int hue, int sat, double blend);

	/**
	 * 色相と彩度と輝度調整
	 * @param hue 色相 -180～180 (度)
	 * @param saturation 彩度 -100～100 (%)
	 * @param luminance 輝度 -100～100 (%)
	 */
	void modulate(int hue, int saturation, int luminance);
	
	/**
	 * ノイズ追加
	 * @param level ノイズレベル 0 (no noise) ～ 255 (lot of noise).
	 */
	void noise(int level);

	/**
	 * ノイズ生成（元の画像を無視してグレースケールのホワイトノイズを描画／α情報は維持）
	 */
	void generateWhiteNoise();

	/**
	 * ガウスぼかし
	 * @param radius ぼかし度合い
	 */
	void gaussianBlur(float radius);
};

#endif