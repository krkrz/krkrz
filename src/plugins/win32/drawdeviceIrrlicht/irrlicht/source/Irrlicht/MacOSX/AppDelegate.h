// Copyright (C) 2005-2008 Etienne Petitjean
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#import <Cocoa/Cocoa.h>
#import "CIrrDeviceMacOSX.h"

@interface AppDelegate : NSObject
{
	BOOL					_quit;
	irr::CIrrDeviceMacOSX	*_device;
}

- (id)initWithDevice:(irr::CIrrDeviceMacOSX *)device;

@end
