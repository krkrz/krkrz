#pragma once

class OGGSRecogniser
{
public:
	OGGSRecogniser(void);
	~OGGSRecogniser(void);

	enum eOggRecogState {
		STATE_START = 0,
		STATE_O,
		STATE_G1,
		STATE_G2


	};

	long feed(unsigned char* inBuff, unsigned long inNumBytes);
	void resetState();

protected:
	eOggRecogState mState;

};
