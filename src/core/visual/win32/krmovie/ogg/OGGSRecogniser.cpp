#include "oggstdafx.h"
#include <OGGSRecogniser.h>

OGGSRecogniser::OGGSRecogniser(void)
:	mState(OGGSRecogniser::STATE_START)
{
}

OGGSRecogniser::~OGGSRecogniser(void)
{
}


long OGGSRecogniser::feed(unsigned char* inBuff, unsigned long inNumBytes) {
	
	for (unsigned long i = 0; i < inNumBytes; i++) {
		switch (mState) {
			case STATE_START:
				//Haven't matched anything
				if (inBuff[i] == 'O') {
					mState = STATE_O;
				}
				break;
			case STATE_O:
				//Already matched an O
				if (inBuff[i] == 'g') {
					//Now advance t the g state
					mState = STATE_G1;
				} else {
					//Anything but a g and we move back to the start state.
					mState = STATE_START;
				}
				break;
			case STATE_G1:
				if (inBuff[i] == 'g') {
					mState = STATE_G2;
				} else {
					mState = STATE_START;
				}
				break;
			case STATE_G2:
				if (inBuff[i] == 'S') {
					mState = STATE_START;
					return (i);
				} else {
					mState = STATE_START;
				}

				break;
			default:
				//Should never be in an invalid state.
				throw 0;
		}
	}

	return -1;
}
void OGGSRecogniser::resetState() {
	mState = STATE_START;
}
