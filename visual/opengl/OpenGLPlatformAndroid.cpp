
#include "tjsCommHead.h"
#include "DebugIntf.h"
#include "Application.h"
#include "FilePathUtil.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "SysInitIntf.h"

#include "OpenGLHeader.h"


//---------------------------------------------------------------------------
static bool TVPANGLEInit = false;
static bool TVPIsSupportES3 = false;
//---------------------------------------------------------------------------
void TVPInitializeOpenGLPlatform() {
	if( TVPANGLEInit == false ) {
		if( Application->IsSupportGLES3() ) {
			if( gl3stubInit() == GL_TRUE ) {
				TVPIsSupportES3 = true;
			}
		}
		TVPANGLEInit = true;
	}
}
//---------------------------------------------------------------------------
bool TVPIsSupportGLES3() { return TVPIsSupportES3; }
//---------------------------------------------------------------------------
