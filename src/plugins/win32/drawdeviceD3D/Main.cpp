#include "ncbind/ncbind.hpp"
#include "DrawDeviceD3D.h"

NCB_REGISTER_CLASS(DrawDeviceD3D) {
	NCB_CONSTRUCTOR((int,int));
	NCB_PROPERTY_RO(interface, getDevice);
	NCB_PROPERTY(width, getWidth, setWidth);
	NCB_PROPERTY(height, getHeight, setHeight);
	NCB_METHOD(setSize);
	NCB_PROPERTY_RO(destWidth, getDestWidth);
	NCB_PROPERTY_RO(destHeight, getDestHeight);
	NCB_PROPERTY(defaultVisible, getDefaultVisible, setDefaultVisible);
	NCB_METHOD(getVisible);
	NCB_METHOD(setVisible);
};

