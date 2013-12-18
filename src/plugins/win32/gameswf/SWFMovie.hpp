// ---------------------------------------------------------------
// SWF ムービー情報処理クラス
// ---------------------------------------------------------------

#ifndef _swfMovie_hpp_
#define _swfMovie_hpp_

#include "gameswf/gameswf.h"

class SWFMovie
{
private:
	gameswf::movie_definition *md;
	gameswf::movie_interface *m;
	int movie_version;
	int movie_width;
	int movie_height;
	float movie_fps;
	int lastFrame;
public:
	SWFMovie();
	~SWFMovie();
	void draw(int width, int height);
	
	void load(const char *name);
	
	bool update(int advance);
	void notifyMouse(int x, int y, int buttons);
	
	void play();
	void stop();
	void restart();
	void back();
	void next();
	void gotoFrame(int frame);
};

#endif
