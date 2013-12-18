#ifndef _layerExText_hpp_
#define _layerExText_hpp_

#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include <vector>
using namespace std;

#include "layerExBase.hpp"

/**
 * GDIPlus 固有処理用
 */
struct GdiPlus {
	/**
	 * プライベートフォントの追加
	 * @param fontFileName フォントファイル名
	 */
	static void addPrivateFont(const tjs_char *fontFileName);

	/**
	 * フォントファミリー名を取得
	 * @param privateOnly true ならプライベートフォントのみ取得
	 */
	static tTJSVariant getFontList(bool privateOnly);
};

/**
 * フォント情報
 */
class FontInfo {
	friend class LayerExDraw;
	friend class Path;

protected:
	FontFamily *fontFamily; //< フォントファミリー
	ttstr familyName;
	REAL emSize; //< フォントサイズ 
	INT style; //< フォントスタイル
        bool gdiPlusUnsupportedFont; //< GDI+未サポートフォント
        bool forceSelfPathDraw; // 自前パス描画強制
        mutable bool propertyModified;
        mutable REAL ascent;
        mutable REAL descent;
        mutable REAL lineSpacing;
        mutable REAL ascentLeading;
        mutable REAL descentLeading;

	/**
	 * フォント情報のクリア
	 */
	void clear();

        OUTLINETEXTMETRIC *createFontMetric(void) const;


public:

	FontInfo();
	/**
	 * コンストラクタ
	 * @param familyName フォントファミリー
	 * @param emSize フォントのサイズ
	 * @param style フォントスタイル
	 */
	FontInfo(const tjs_char *familyName, REAL emSize, INT style);
	FontInfo(const FontInfo &orig);

	/**
	 * デストラクタ
	 */
	virtual ~FontInfo();

	void setFamilyName(const tjs_char *familyName);
	const tjs_char *getFamilyName() { return familyName.c_str(); }
	void setEmSize(REAL emSize) { this->emSize = emSize; propertyModified = true; }
	REAL getEmSize() {  return emSize; }
	void setStyle(INT style) { this->style = style; propertyModified = true; }
	INT getStyle() { return style; }
        void setForceSelfPathDraw(bool state);
        bool getForceSelfPathDraw(void) const;
        bool getSelfPathDraw(void) const;

        void updateSizeParams(void) const;
	REAL getAscent() const;
	REAL getDescent() const;
	REAL getAscentLeading() const;
	REAL getDescentLeading() const;
	REAL getLineSpacing() const;
};

/**
 * 描画外観情報
 */
class Appearance {
	friend class LayerExDraw;
public:
	// 描画情報
	struct DrawInfo{
		int type;   // 0:ブラシ 1:ペン
		void *info; // 情報オブジェクト
		REAL ox; //< 表示オフセット
		REAL oy; //< 表示オフセット
		DrawInfo() : ox(0), oy(0), type(0), info(NULL) {}
		DrawInfo(REAL ox, REAL oy, Pen *pen) : ox(ox), oy(oy), type(0), info(pen) {}
		DrawInfo(REAL ox, REAL oy, Brush *brush) : ox(ox), oy(oy), type(1), info(brush) {}
		DrawInfo(const DrawInfo &orig) {
			ox = orig.ox;
			oy = orig.oy;
			type = orig.type;
			if (orig.info) {
				switch (type) {
				case 0:
					info = (void*)((Pen*)orig.info)->Clone();
					break;
				case 1:
					info = (void*)((Brush*)orig.info)->Clone();
					break;
				}
			} else {
				info = NULL;
			}
		}
		virtual ~DrawInfo() {
			if (info) {
				switch (type) {
				case 0:
					delete (Pen*)info;
					break;
				case 1:
					delete (Brush*)info;
					break;
				}
			}
		}	
	};
	vector<DrawInfo>drawInfos;

public:
	Appearance();
	virtual ~Appearance();

	/**
	 * 情報のクリア
	 */
	void clear();
	
	/**
	 * ブラシの追加
	 * @param colorOrBrush ARGB色指定またはブラシ情報（辞書）
	 * @param ox 表示オフセットX
	 * @param oy 表示オフセットY
	 */
	void addBrush(tTJSVariant colorOrBrush, REAL ox=0, REAL oy=0);
	
	/**
	 * ペンの追加
	 * @param colorOrBrush ARGB色指定またはブラシ情報（辞書）
	 * @param widthOrOption ペン幅またはペン情報（辞書）
	 * @param ox 表示オフセットX
	 * @param oy 表示オフセットY
	 */
	void addPen(tTJSVariant colorOrBrush, tTJSVariant widthOrOption, REAL ox=0, REAL oy=0);

protected:
	/**
	 * LineCapの取得
	 */
	bool getLineCap(tTJSVariant &in, LineCap &cap, CustomLineCap* &custom, REAL pw);
	vector<CustomLineCap*>customLineCaps;
};


/**
 * 描画外観情報
 */
class Path {
	friend class LayerExDraw;
public:
	Path();
	virtual ~Path();
	void startFigure();
	void closeFigure();
	void drawArc(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
	void drawBezier(REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4);
	void drawBeziers(tTJSVariant points);
	void drawClosedCurve(tTJSVariant points);
	void drawClosedCurve2(tTJSVariant points, REAL tension);
	void drawCurve(tTJSVariant points);
	void drawCurve2(tTJSVariant points, REAL tension);
	void drawCurve3(tTJSVariant points, int offset, int numberOfSegments, REAL tension);
	void drawPie(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
	void drawEllipse(REAL x, REAL y, REAL width, REAL height);
	void drawLine(REAL x1, REAL y1, REAL x2, REAL y2);
	void drawLines(tTJSVariant points);
	void drawPolygon(tTJSVariant points);
	void drawRectangle(REAL x, REAL y, REAL width, REAL height);
	void drawRectangles(tTJSVariant rects);
protected:
	GraphicsPath path;
};

/*
 * アウトラインベースのテキスト描画メソッドの追加
 */
class LayerExDraw : public layerExBase
{
protected:
	// 情報保持用
	GeometryT width, height;
	BufferT   buffer;
	PitchT    pitch;
	GeometryT clipLeft, clipTop, clipWidth, clipHeight;
	
	/// レイヤを参照するビットマップ
	Bitmap *bitmap;
	/// レイヤに対して描画するコンテキスト
	Graphics *graphics;

	// Transform 指定
	Matrix transform;
	Matrix viewTransform;
	Matrix calcTransform;

protected:
	// 描画スムージング指定
	SmoothingMode smoothingMode;
	// drawString のアンチエイリアス指定
	TextRenderingHint textRenderingHint;

public:
	int getSmoothingMode() {
		return (int)smoothingMode;
	}
	void setSmoothingMode(int mode) {
		smoothingMode = (SmoothingMode)mode;
	}

	int getTextRenderingHint() {
		return (int)textRenderingHint;
	}
	void setTextRenderingHint(int hint) {
		textRenderingHint = (TextRenderingHint)hint;
	}

protected:
	/// 描画内容記録用メタファイル
	HDC metaHDC;
	HGLOBAL metaBuffer;
	IStream *metaStream;
	Metafile *metafile;
	Graphics *metaGraphics;

	bool updateWhenDraw;
	void updateRect(RectF &rect);
	
public:
	void setUpdateWhenDraw(int updateWhenDraw) {
		this->updateWhenDraw = updateWhenDraw != 0;
	}
	int getUpdateWhenDraw() { return updateWhenDraw ? 1 : 0; }

	inline operator Image*() const { return (Image*)bitmap; }
	inline operator Bitmap*() const { return bitmap; }
	inline operator Graphics*() const { return graphics; }
	inline operator const Image*() const { return (const Image*)bitmap; }
	inline operator const Bitmap*() const { return bitmap; }
	inline operator const Graphics*() const { return graphics; }
	
	template <class T>
	struct BridgeFunctor {
		T* operator()(LayerExDraw *p) const {
			return (T*)*p;
		}
	};

public:	
	LayerExDraw(DispatchT obj);
	~LayerExDraw();
	virtual void reset();

	// ------------------------------------------------------------------
	// 描画パラメータ指定
	// ------------------------------------------------------------------

protected:
	void updateViewTransform();
	void updateTransform();
	
public:
	/**
	 * 表示トランスフォームの指定
	 */
	void setViewTransform(const Matrix *transform);
	void resetViewTransform();
	void rotateViewTransform(REAL angle);
	void scaleViewTransform(REAL sx, REAL sy);
	void translateViewTransform(REAL dx, REAL dy);
	
	/**
	 * トランスフォームの指定
	 * @param matrix トランスフォームマトリックス
	 */
	void setTransform(const Matrix *transform);
	void resetTransform();
	void rotateTransform(REAL angle);
	void scaleTransform(REAL sx, REAL sy);
	void translateTransform(REAL dx, REAL dy);

	// ------------------------------------------------------------------
	// 描画メソッド群
	// ------------------------------------------------------------------

protected:

	/**
	 * パスの更新領域情報を取得
	 * @param app 表示表現
	 * @param path 描画するパス
	 * @return 更新領域情報
	 */
	RectF getPathExtents(const Appearance *app, const GraphicsPath *path);

	/**
	 * パスの描画用下請け処理
	 * @param graphics 描画先
	 * @param pen 描画用ペン
	 * @param matrix 描画位置調整用matrix
	 * @param path 描画内容
	 */
	void draw(Graphics *graphics, const Pen *pen, const Matrix *matrix, const GraphicsPath *path);

	/**
	 * 塗りの描画用下請け処理
	 * @param graphics 描画先
	 * @param brush 描画用ブラシ
	 * @param matrix 描画位置調整用matrix
	 * @param path 描画内容
	 */
	void fill(Graphics *graphics, const Brush *brush, const Matrix *matrix, const GraphicsPath *path);
	
	/**
	 * パスの描画
	 * @param app アピアランス
	 * @param path 描画するパス
	 * @return 更新領域情報
	 */
	RectF _drawPath(const Appearance *app, const GraphicsPath *path);

        /**
         * グリフアウトラインの取得
         * @param font フォント
         * @param offset オフセット
         * @param path グリフを書き出すパス
         * @param glyph 描画するグリフ
         */
        void getGlyphOutline(const FontInfo *font, PointF &offset, GraphicsPath *path, UINT glyph);

        /*
         * テキストアウトラインの取得
         * @param font フォント
         * @param offset オフセット
         * @param path グリフを書き出すパス
         * @param text 描画するテキスト
         */
         void getTextOutline(const FontInfo *font, PointF &offset, GraphicsPath *path, ttstr text);

public:
	/**
	 * 画面の消去
	 * @param argb 消去色
	 */
	void clear(ARGB argb);

	/**
	 * パスの描画
	 * @param app アピアランス
	 * @param path パス
	 */
	RectF drawPath(const Appearance *app, const Path *path);
	
	/**
	 * 円弧の描画
	 * @param app アピアランス
	 * @param x 左上座標
	 * @param y 左上座標
	 * @param width 横幅
	 * @param height 縦幅
	 * @param startAngle 時計方向円弧開始位置
	 * @param sweepAngle 描画角度
	 * @return 更新領域情報
	 */
	RectF drawArc(const Appearance *app, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);

	/**
	 * 円錐の描画
	 * @param app アピアランス
	 * @param x 左上座標
	 * @param y 左上座標
	 * @param width 横幅
	 * @param height 縦幅
	 * @param startAngle 時計方向円弧開始位置
	 * @param sweepAngle 描画角度
	 * @return 更新領域情報
	 */
	RectF drawPie(const Appearance *app, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
	
	/**
	 * ベジェ曲線の描画
	 * @param app アピアランス
	 * @param x1
	 * @param y1
	 * @param x2
	 * @param y2
	 * @param x3
	 * @param y3
	 * @param x4
	 * @param y4
	 * @return 更新領域情報
	 */
	RectF drawBezier(const Appearance *app, REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4);

	/**
	 * 連続ベジェ曲線の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @return 更新領域情報
	 */
	RectF drawBeziers(const Appearance *app, tTJSVariant points);

	/**
	 * Closed cardinal spline の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @return 更新領域情報
	 */
	RectF drawClosedCurve(const Appearance *app, tTJSVariant points);

	/**
	 * Closed cardinal spline の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @pram tension tension
	 * @return 更新領域情報
	 */
	RectF drawClosedCurve2(const Appearance *app, tTJSVariant points, REAL tension);

	/**
	 * cardinal spline の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @return 更新領域情報
	 */
	RectF drawCurve(const Appearance *app, tTJSVariant points);

	/**
	 * cardinal spline の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @parma tension tension
	 * @return 更新領域情報
	 */
	RectF drawCurve2(const Appearance *app, tTJSVariant points, REAL tension);

	/**
	 * cardinal spline の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @param offset
	 * @param numberOfSegment
	 * @param tension tension
	 * @return 更新領域情報
	 */
	RectF drawCurve3(const Appearance *app, tTJSVariant points, int offset, int numberOfSegments, REAL tension);
	
	/**
	 * 楕円の描画
	 * @param app アピアランス
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 * @return 更新領域情報
	 */
	RectF drawEllipse(const Appearance *app, REAL x, REAL y, REAL width, REAL height);

	/**
	 * 線分の描画
	 * @param app アピアランス
	 * @param x1 始点X座標
	 * @param y1 始点Y座標
	 * @param x2 終点X座標
	 * @param y2 終点Y座標
	 * @return 更新領域情報
	 */
	RectF drawLine(const Appearance *app, REAL x1, REAL y1, REAL x2, REAL y2);

	/**
	 * 連続線分の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @return 更新領域情報
	 */
	RectF drawLines(const Appearance *app, tTJSVariant points);

	/**
	 * 多角形の描画
	 * @param app アピアランス
	 * @param points 点の配列
	 * @return 更新領域情報
	 */
	RectF drawPolygon(const Appearance *app, tTJSVariant points);
	
	/**
	 * 矩形の描画
	 * @param app アピアランス
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 * @return 更新領域情報
	 */
	RectF drawRectangle(const Appearance *app, REAL x, REAL y, REAL width, REAL height);

	/**
	 * 複数矩形の描画
	 * @param app アピアランス
	 * @param rects 矩形情報の配列
	 * @return 更新領域情報
	 */
	RectF drawRectangles(const Appearance *app, tTJSVariant rects);

	/**
	 * 文字列の描画
	 * @param font フォント
	 * @param app アピアランス
	 * @param x 描画位置X
	 * @param y 描画位置Y
	 * @param text 描画テキスト
	 * @return 更新領域情報
	 */
	RectF drawPathString(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text);

	/**
	 * 文字列の描画(OpenTypeのPostScriptフォント対応)
	 * @param font フォント
	 * @param app アピアランス
	 * @param x 描画位置X
	 * @param y 描画位置Y
	 * @param text 描画テキスト
	 * @return 更新領域情報
	 */
	RectF drawPathString2(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text);

	// -------------------------------------------------------------------------------
	
	/**
	 * 文字列の描画
	 * @param font フォント
	 * @param app アピアランス
	 * @param x 描画位置X
	 * @param y 描画位置Y
	 * @param text 描画テキスト
	 * @return 更新領域情報
	 */
	RectF drawString(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text);

	/**
	 * 文字列の描画更新領域情報の取得
	 * @param font フォント
	 * @param text 描画テキスト
	 * @return 更新領域情報の辞書 left, top, width, height
	 */
	RectF measureString(const FontInfo *font, const tjs_char *text);

	/**
	 * 文字列にぴったりと接っする矩形の取得
	 * @param font フォント
	 * @param text 描画テキスト
	 * @return 領域情報の辞書 left, top, width, height
	 */
	RectF measureStringInternal(const FontInfo *font, const tjs_char *text);

	/**
	 * 文字列の描画更新領域情報の取得(OpenTypeのPostScriptフォント対応)
	 * @param font フォント
	 * @param text 描画テキスト
	 * @return 更新領域情報の辞書 left, top, width, height
	 */
	RectF measureString2(const FontInfo *font, const tjs_char *text);

	/**
	 * 文字列にぴったりと接っする矩形の取得(OpenTypeのPostScriptフォント対応)
	 * @param font フォント
	 * @param text 描画テキスト
	 * @return 領域情報の辞書 left, top, width, height
	 */
	RectF measureStringInternal2(const FontInfo *font, const tjs_char *text);

	// -----------------------------------------------------------------------------
	
	/**
	 * 画像の描画。コピー先は元画像の Bounds を配慮した位置、サイズは Pixel 指定になります。
	 * @param x コピー先原点X
	 * @param y コピー先原点Y
	 * @param image コピー元画像
	 * @return 更新領域情報
	 */
	RectF drawImage(REAL x, REAL y, Image *src);

	/**
	 * 画像の矩形コピー
	 * @param dleft コピー先左端
	 * @param dtop  コピー先上端
	 * @param src コピー元画像
	 * @param sleft 元矩形の左端
	 * @param stop  元矩形の上端
	 * @param swidth 元矩形の横幅
	 * @param sheight  元矩形の縦幅
	 * @return 更新領域情報
	 */
	RectF drawImageRect(REAL dleft, REAL dtop, Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight);

	/**
	 * 画像の拡大縮小コピー
	 * @param dleft コピー先左端
	 * @param dtop  コピー先上端
	 * @param dwidth コピー先の横幅
	 * @param dheight  コピー先の縦幅
	 * @param src コピー元画像
	 * @param sleft 元矩形の左端
	 * @param stop  元矩形の上端
	 * @param swidth 元矩形の横幅
	 * @param sheight  元矩形の縦幅
	 * @return 更新領域情報
	 */
	RectF drawImageStretch(REAL dleft, REAL dtop, REAL dwidth, REAL dheight, Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight);

	/**
	 * 画像のアフィン変換コピー
	 * @param src コピー元画像
	 * @param sleft 元矩形の左端
	 * @param stop  元矩形の上端
	 * @param swidth 元矩形の横幅
	 * @param sheight  元矩形の縦幅
	 * @param affine アフィンパラメータの種類(true:変換行列, false:座標指定), 
	 * @return 更新領域情報
	 */
	RectF drawImageAffine(Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight, bool affine, REAL A, REAL B, REAL C, REAL D, REAL E, REAL F);

	// ------------------------------------------------
	// メタファイル操作
	// ------------------------------------------------

protected:

	/**
	 * 記録情報の生成
	 */
	void createRecord();

	/**
	 * 記録情報の生成
	 */
	void recreateRecord();
	
	/**
	 * 記録情報の破棄
	 */
	void destroyRecord();

	/**
	 * 再描画用
	 */
	bool redraw(Image *image);
	
public:
	/**
	 * @param record 描画内容を記録するかどうか
	 */
	void setRecord(bool record);

	/**
	 * @return record 描画内容を記録するかどうか
	 */
	bool getRecord() {
		return metafile != NULL;
	}

	/**
	 * 記録内容を Image として取得
	 * @return 成功したら true
	 */
	Image *getRecordImage();
	
	/**
	 * 記録内容の再描画
	 * @return 再描画したら true
	 */
	bool redrawRecord();

	/**
	 * 記録内容の保存
	 * @param filename 保存ファイル名
	 * @return 成功したら true
	 */
	bool saveRecord(const tjs_char *filename);

	/**
	 * 記録内容の読み込み
	 * @param filename 読み込みファイル名
	 * @return 成功したら true
	 */
	bool loadRecord(const tjs_char *filename);

	/**
	 * 画像の保存
	 */
	static tjs_error TJS_INTF_METHOD saveImage(tTJSVariant *result,
											   tjs_int numparams,
											   tTJSVariant **param,
											   iTJSDispatch2 *objthis);
};

#endif
