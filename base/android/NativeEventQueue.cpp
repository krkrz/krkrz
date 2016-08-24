
#include "tjsCommHead.h"
#include "NativeEventQueue.h"
#include "Application.h"

void NativeEventQueueImplement::Allocate() {
	Application->addEventHandler(this);
}
void NativeEventQueueImplement::Deallocate() {
	Application->removeEventHandler(this);
}
void NativeEventQueueImplement::PostEvent( const NativeEvent& event ) {
	Application->postEvent( &event );
}

