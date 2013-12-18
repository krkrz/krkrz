#include "LayerExDraw.hpp"

extern void getPoints(const tTJSVariant &var, vector<PointF> &points);
extern void getRects(const tTJSVariant &var, vector<RectF> &rects);

Path::Path()
{
}

Path::~Path()
{
}

/**
 * 現在の図形を閉じずに次の図形を開始します
 */
void
Path::startFigure()
{
    path.StartFigure();
}


/**
 * 現在の図形を閉じます
 */
void
Path::closeFigure()
{
    path.CloseFigure();
}

/**
 * 円弧の描画
 * @param x 左上座標
 * @param y 左上座標
 * @param width 横幅
 * @param height 縦幅
 * @param startAngle 時計方向円弧開始位置
 * @param sweepAngle 描画角度
 */
void
Path::drawArc(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle)
{
	path.AddArc(x, y, width, height, startAngle, sweepAngle);
}

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
 */
void
Path::drawBezier(REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4)
{
	path.AddBezier(x1, y1, x2, y2, x3, y3, x4, y4);
}

/**
 * 連続ベジェ曲線の描画
 * @param app アピアランス
 * @param points 点の配列
 */
void
Path::drawBeziers(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddBeziers(&ps[0], (int)ps.size());
}

/**
 * Closed cardinal spline の描画
 * @param app アピアランス
 * @param points 点の配列
 */
void
Path::drawClosedCurve(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddClosedCurve(&ps[0], (int)ps.size());
}

/**
 * Closed cardinal spline の描画
 * @param app アピアランス
 * @param points 点の配列
 * @pram tension tension
 */
void
Path::drawClosedCurve2(tTJSVariant points, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddClosedCurve(&ps[0], (int)ps.size(), tension);
}

/**
 * cardinal spline の描画
 * @param app アピアランス
 * @param points 点の配列
 */
void
Path::drawCurve(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddCurve(&ps[0], (int)ps.size());
}

/**
 * cardinal spline の描画
 * @param app アピアランス
 * @param points 点の配列
 * @parma tension tension
 */
void
Path::drawCurve2(tTJSVariant points, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddCurve(&ps[0], (int)ps.size(), tension);
}

/**
 * cardinal spline の描画
 * @param app アピアランス
 * @param points 点の配列
 * @param offset
 * @param numberOfSegments
 * @param tension tension
 */
void
Path::drawCurve3(tTJSVariant points, int offset, int numberOfSegments, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddCurve(&ps[0], (int)ps.size(), offset, numberOfSegments, tension);
}

/**
 * 円錐の描画
 * @param x 左上座標
 * @param y 左上座標
 * @param width 横幅
 * @param height 縦幅
 * @param startAngle 時計方向円弧開始位置
 * @param sweepAngle 描画角度
 */
void
Path::drawPie(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle)
{
	path.AddPie(x, y, width, height, startAngle, sweepAngle);
}

/**
 * 楕円の描画
 * @param app アピアランス
 * @param x
 * @param y
 * @param width
 * @param height
 */
void
Path::drawEllipse(REAL x, REAL y, REAL width, REAL height)
{
	path.AddEllipse(x, y, width, height);
}

/**
 * 線分の描画
 * @param app アピアランス
 * @param x1 始点X座標
 * @param y1 始点Y座標
 * @param x2 終点X座標
 * @param y2 終点Y座標
 */
void
Path::drawLine(REAL x1, REAL y1, REAL x2, REAL y2)
{
	path.AddLine(x1, y1, x2, y2);
}

/**
 * 連続線分の描画
 * @param app アピアランス
 * @param points 点の配列
 */
void
Path::drawLines(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddLines(&ps[0], (int)ps.size());
}

/**
 * 多角形の描画
 * @param app アピアランス
 * @param points 点の配列

 */
void
Path::drawPolygon(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddPolygon(&ps[0], (int)ps.size());
}


/**
 * 矩形の描画
 * @param app アピアランス
 * @param x
 * @param y
 * @param width
 * @param height
 */
void
Path::drawRectangle(REAL x, REAL y, REAL width, REAL height)
{
	RectF rect(x, y, width, height);
	path.AddRectangle(rect);
}

/**
 * 複数矩形の描画
 * @param app アピアランス
 * @param rects 矩形情報の配列
 */
void
Path::drawRectangles(tTJSVariant rects)
{
	vector<RectF> rs;
	getRects(rects, rs);
	path.AddRectangles(&rs[0], (int)rs.size());
}
