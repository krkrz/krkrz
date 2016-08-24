

#ifndef __TVP_SURFACE_H__
#define __TVP_SURFACE_H__

class iTVPSurface {
	// Key Event
	virtual void OnKeyUp( unsigned short vk, int shift ) = 0;
	virtual void OnKeyDown( unsigned short vk, int shift, int repeat, bool prevkeystate ) = 0;
	virtual void OnKeyPress( unsigned short vk, int repeat, bool prevkeystate, bool convertkey ) = 0;

	// Touch Event
	virtual void OnTouchDown( double x, double y, double cx, double cy, int id, unsigned int tick ) = 0;
	virtual void OnTouchMove( double x, double y, double cx, double cy, int id, unsigned int tick ) = 0;
	virtual void OnTouchUp( double x, double y, double cx, double cy, int id, unsigned int tick ) = 0;
};


#endif // __TVP_SURFACE_H__
