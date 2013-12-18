#ifndef _layExSWF_hpp_
#define _layExSWF_hpp_

#include "layerExCairo.hpp"
#include "SWFMovie.hpp"

/*
 * SWF •`‰æ—pƒŒƒCƒ„
 */
struct layerExSWF : public layerExCairo
{
public:
	layerExSWF(DispatchT obj);
	~layerExSWF();

	/**
	 * SWF •`‰æ
	 */
	void drawSWF(SWFMovie *swf);
};

#endif
