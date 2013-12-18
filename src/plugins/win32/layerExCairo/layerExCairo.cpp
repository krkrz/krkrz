#include "layerExCairo.hpp"

/**
 * コンストラクタ
 */
layerExCairo::layerExCairo(DispatchT obj) : layerExBase(obj)
{
	width = 0;
	height = 0;
	buffer = 0;
	pitch = 0;
	surface = NULL;
	cairo = NULL;
}

/**
 * デストラクタ
 */
layerExCairo::~layerExCairo()
{
	cairo_destroy(cairo);
	cairo_surface_destroy(surface);
}

/**
 * リセット処理
 */
void
layerExCairo::reset()
{
	// 基本処理
	layerExBase::reset();

	// レイヤの情報変更があったかどうか
	reseted = (width  != _width ||
			   height != _height ||
			   buffer != _buffer ||
			   pitch  != _pitch);

	if (reseted) {
		width  = _width;
		height = _height;
		buffer = _buffer;
		pitch  = _pitch;
		// cairo 用コンテキストの再生成
		cairo_destroy(cairo);
		cairo_surface_destroy(surface);
		surface = cairo_image_surface_create_for_data((BYTE*)_buffer, CAIRO_FORMAT_ARGB32, width, height, pitch);
		cairo = cairo_create(surface);
		if (cairo_status(cairo) != CAIRO_STATUS_SUCCESS) {
			TVPThrowExceptionMessage(L"can't create cairo context");
			cairo_destroy(cairo);
			cairo = NULL;
		}
	}
}
