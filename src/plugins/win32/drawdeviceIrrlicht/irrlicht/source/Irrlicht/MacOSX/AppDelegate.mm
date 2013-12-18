// Copyright (C) 2005-2008 Etienne Petitjean
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#import "AppDelegate.h"

@implementation AppDelegate

- (id)initWithDevice:(irr::CIrrDeviceMacOSX *)device
{
	self = [super init];
	if (self) _device = device;
	return (self);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	_quit = FALSE;
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
	[NSApp orderFrontStandardAboutPanel:sender];	
}

- (void)unhideAllApplications:(id)sender
{
	[NSApp unhideAllApplications:sender];	
}

- (void)hide:(id)sender
{
	[NSApp hide:sender];	
}

- (void)hideOtherApplications:(id)sender
{
	[NSApp hideOtherApplications:sender];	
}

- (void)terminate:(id)sender
{
	_quit = TRUE;
}

- (void)windowDidResize:(NSNotification *)aNotification
{
	NSWindow	*window;
	NSRect		frame;
	
	window = [aNotification object];
	frame = [window frame];
	_device->setResize((int)frame.size.width,(int)frame.size.height);
}

- (BOOL)isQuit
{
	return (_quit);
}

@end
