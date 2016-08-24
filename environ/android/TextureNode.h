

#ifndef __TEXTURE_NODE_H__
#define __TEXTURE_NODE_H__

#include "Node.h"

class TextureNode : public Node {
protected:
	Texture texure_;

	void setSizeToImageSize();

	virtual void OnDraw( NodeDrawDevice* dd );
}

#endif // __TEXTURE_NODE_H__

