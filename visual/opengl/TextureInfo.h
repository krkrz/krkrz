
#ifndef TextureInfoH
#define TextureInfoH

class iTVPTextureInfoIntrface {
public:

	/**
	 * 幅を取得
	 * @return テクスチャ幅
	 */
	virtual tjs_uint GetWidth() const = 0;

	/**
	 * 高さを取得
	 * @return テクスチャ高さ
	 */
	virtual tjs_uint GetHeight() const = 0;

	/**
	 * ネイティブハンドルを取得。OpenGL ES2/3実装ではテクスチャID
	 * @return ネイティブハンドル
	 */
	virtual tjs_int64 GetNativeHandle() const = 0;

	/**
	 * テクスチャ全体を表す頂点データのVBOをハンドルを返す。
	 * @return VBO ID、0の時VBOがない
	 */
	virtual tjs_int64 GetVBOHandle() const = 0;

	/**
	 * テクスチャフォーマットを取得
	 * @return テクスチャフォーマット
	 */
	virtual tjs_int GetImageFormat() const = 0;
};

#endif
