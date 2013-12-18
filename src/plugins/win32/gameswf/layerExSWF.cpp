// ---------------------------------------------------------------
// SWF ムービー描画レイヤ
// ---------------------------------------------------------------

#include "layerExSWF.hpp"

/**
 * コンストラクタ
 */
layerExSWF::layerExSWF(DispatchT obj) : layerExCairo(obj)
{
}

/**
 * デストラクタ
 */
layerExSWF::~layerExSWF()
{
}

// 描画ターゲット
extern cairo_t *ctarget;


/**
 * @param swf ムービー
 * @param advance 経過時間(ms)
 */
void
layerExSWF::drawSWF(SWFMovie *swf)
{
	ctarget = cairo;
	swf->draw(_width, _height);
}
