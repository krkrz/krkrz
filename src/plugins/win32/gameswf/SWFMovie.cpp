// ---------------------------------------------------------------
// SWF ムービー情報処理クラス
// ---------------------------------------------------------------

#include <stdio.h>
#include "SWFMovie.hpp"
#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_impl.h"

extern void message_log(const char* format, ...);
extern void error_log(const char *format, ...);

SWFMovie::SWFMovie()
{
}

SWFMovie::~SWFMovie()
{
	if (m) {
		m->drop_ref();
	}
	if (md) {
		md->drop_ref();
	}
}

static int s_total_tags = 0, s_loaded_tags = 0;

void
SWFMovie::load(const char *name)
{
	gameswf::get_movie_info(name, &movie_version,
							&movie_width, &movie_height, &movie_fps,
							NULL, &s_total_tags);
	if (movie_version == 0) {
		error_log("error: %s についての情報が取得できません", name);
		return;
	} else if (movie_version > 6) {
		message_log("warning: %s は対応していないバージョン %d のファイルです", movie_version);
	}

	md = gameswf::create_movie(name);
	if (md != NULL) {
		m = md->create_instance();
		// 開始する
		m->get_root_movie()->execute_frame_tags(0);		
	}
	lastFrame = -1;
}

void
SWFMovie::draw(int width, int height)
{
	if (m) {
		m->set_display_viewport(0, 0, width, height);
		//m->set_background_alpha(s_background ? 1.0f : 0.05f);
		m->display();
	}
}

bool
SWFMovie::update(int advance)
{
	bool updated = false;
	if (m) {
		if (advance > 0) {
			m->advance(advance / 1000.0f);
		}
		int frame = m->get_current_frame();
		updated = frame != lastFrame;
		lastFrame = frame;
	}
	return updated;
}

void
SWFMovie::notifyMouse(int x, int y, int buttons)
{
	if (m) {
		m->notify_mouse_state(x, y, buttons);
	}
}

void
SWFMovie::play()
{
	if (m) {
		if (m->get_play_state() == gameswf::movie_interface::STOP) {
			m->set_play_state(gameswf::movie_interface::PLAY);
		}
	}
}

void
SWFMovie::stop()
{
	if (m) {
		if (m->get_play_state() == gameswf::movie_interface::PLAY) {
			m->set_play_state(gameswf::movie_interface::STOP);
		}
	}
}

void
SWFMovie::restart()
{
	if (m) {
		m->restart();
	}
}

void
SWFMovie::back()
{
	if (m) {
		m->goto_frame(m->get_current_frame()-1);
	}
}

void
SWFMovie::next()
{
	if (m) {
		m->goto_frame(m->get_current_frame()+1);
	}
}

void
SWFMovie::gotoFrame(int frame)
{
	if (m) {
		m->goto_frame(frame);
	}
}
