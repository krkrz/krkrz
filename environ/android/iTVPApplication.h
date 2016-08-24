
#ifndef __APPLICATION_INTERFACE_H__
#define __APPLICATION_INTERFACE_H__

class iTVPApplication {
public:
	virtual void startApplication( struct android_app* state ) = 0;
};

/*
iTVPApplication* CreateApplication();
void DestroyApplication( iTVPApplication* app );
*/

#endif // __APPLICATION_INTERFACE_H__

