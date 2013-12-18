// ---------------------------------------------------------------
// SWF ムービー描画用ハンドラ
// ---------------------------------------------------------------

#include "cairo.h"
#include "base/utility.h"
#include "base/container.h"
#include "base/tu_file.h"
#include "base/tu_types.h"
#include "base/image.h"
#include "gameswf/gameswf_impl.h"
#include "gameswf/gameswf_types.h"

extern void message_log(const char* format, ...);
extern void error_log(const char *format, ...);

using namespace gameswf;

// 描画ターゲット
cairo_t *ctarget;

struct bitmap_info_cairo : public gameswf::bitmap_info
{
	bitmap_info_cairo();
	bitmap_info_cairo(int width, int height, Uint8* data);
	bitmap_info_cairo(image::rgb* im);
	bitmap_info_cairo(image::rgba* im);
	~bitmap_info_cairo() {
	}
};

bitmap_info_cairo::bitmap_info_cairo()
{
}

bitmap_info_cairo::bitmap_info_cairo(int width, int height, Uint8* data)
{
}

bitmap_info_cairo::bitmap_info_cairo(image::rgb* im)
{
}

bitmap_info_cairo::bitmap_info_cairo(image::rgba* im)
{
}

struct render_handler_cairo : public gameswf::render_handler
{
	// Enable/disable antialiasing.
	bool	m_enable_antialias;

	// Output size.
	float	m_display_width;
	float	m_display_height;
	
	cairo_matrix_t m_current_matrix;
	gameswf::cxform	m_current_cxform;

	~render_handler_cairo() {}
	
	virtual bitmap_info* create_bitmap_info_empty() {
		return new bitmap_info_cairo;
	}
	
	virtual bitmap_info *create_bitmap_info_alpha(int w, int h, unsigned char* data) {
		return new bitmap_info_cairo(w, h, data);
	}
	
	virtual bitmap_info *create_bitmap_info_rgb(image::rgb* im) {
		return new bitmap_info_cairo(im);
	}
	
	virtual bitmap_info *create_bitmap_info_rgba(image::rgba* im) {
		return new bitmap_info_cairo(im);
	}
	
	virtual void delete_bitmap_info(bitmap_info* bi) {
		delete bi;
	}

	gameswf::YUV_video*	create_YUV_video(int w, int h)
	{
		return NULL;
	}

	void	delete_YUV_video(gameswf::YUV_video* yuv)
	{
	}

	cairo_matrix_t viewport;
	
	// Bracket the displaying of a frame from a movie.
	// Fill the background color, and set up default
	// transforms, etc.
	virtual void	begin_display(rgba bc,
								  int viewport_x0, int viewport_y0,
								  int viewport_width, int viewport_height,
								  float x0, float x1, float y0, float y1) {

		m_display_width = fabsf(x1 - x0);
		m_display_height = fabsf(y1 - y0);

		if (ctarget) {
			// ビューポート補正
			double sx = (double)viewport_width / m_display_width;
			double sy = (double)viewport_height / m_display_height;
			double scale = sx < sy ? sx : sy;
			cairo_matrix_init_scale(&viewport, scale, scale);
			cairo_matrix_translate(&viewport, viewport_x0 - x0 * sx, viewport_y0 - y0 * sy);
			cairo_set_matrix(ctarget, &viewport);

			// 背景塗りつぶし
//			cairo_set_source_rgba(ctarget,
//								  bc.m_r/255.0,
//								  bc.m_g/255.0,
//								  bc.m_b/255.0,
//								  bc.m_a/255.0);
//			cairo_rectangle(ctarget, x0, y0, m_display_width, m_display_height);
//			cairo_fill(ctarget);
		}
	}

	virtual void	end_display() {
	}
	
	// Geometric and color transforms for mesh and line_strip rendering.
	virtual void	set_matrix(const matrix& m) {
		cairo_matrix_init(&m_current_matrix,
						  m.m_[0][0],
						  m.m_[1][0],
						  m.m_[0][1],
						  m.m_[1][1],
						  m.m_[0][2],
						  m.m_[1][2]);
	}
	
	virtual void	set_cxform(const cxform& cx) {
		m_current_cxform = cx;
	}

	
	// Draw triangles using the current fill-style 0.
	// Clears the style list after rendering.
	//
	// coords is a list of (x,y) coordinate pairs, in
	// triangle-strip order.  The type of the array should
	// be Sint16[vertex_count*2]
	virtual void	draw_mesh_strip(const void* coords, int vertex_count) {
		if (ctarget) {
			Sint16 *c = (Sint16*)coords;
			int i = 0;
			cairo_save(ctarget);
			cairo_transform(ctarget, &m_current_matrix);
			cairo_new_path(ctarget);
			while (i < vertex_count) {
				int n = i*2;
				cairo_move_to(ctarget, c[n  ], c[n+1]);
				cairo_line_to(ctarget, c[n+2], c[n+3]);
				cairo_line_to(ctarget, c[n+4], c[n+5]);
				cairo_close_path(ctarget);
				i++;
			}
			cairo_fill (ctarget);
			cairo_restore(ctarget);
		}
	}

	// As above, but coords is in triangle list order.
	virtual void	draw_triangle_list(const void *coords, int vertex_count) {
		if (ctarget) {
			Sint16 *c = (Sint16*)coords;
			cairo_save(ctarget);
			cairo_transform(ctarget, &m_current_matrix);
			cairo_new_path(ctarget);
			int i = 0;
			while (i < vertex_count) {
				int n = i*2;
				cairo_move_to(ctarget, c[n  ], c[n+1]);
				cairo_line_to(ctarget, c[n+2], c[n+3]);
				cairo_line_to(ctarget, c[n+4], c[n+5]);
				cairo_close_path(ctarget);
				i += 3;
			}
			cairo_fill (ctarget);
			cairo_restore(ctarget);
		}
	}
		
	// Draw a line-strip using the current line style.
	// Clear the style list after rendering.
	//
	// Coords is a list of (x,y) coordinate pairs, in
	// sequence.  Each coord is a 16-bit signed integer.
	virtual void	draw_line_strip(const void* coords, int vertex_count) {
		if (ctarget) {
			Sint16 *c = (Sint16*)coords;
			cairo_save(ctarget);
			cairo_transform(ctarget, &m_current_matrix);
			cairo_new_path(ctarget);
			cairo_move_to(ctarget, c[0], c[1]);
			int i = 1;
			while (i < vertex_count) {
				int n = i*2;
				cairo_line_to(ctarget, c[n], c[n+1]);
				i++;
			}
			cairo_stroke(ctarget);
			cairo_restore(ctarget);
		}
	}
		
	// Set line and fill styles for mesh & line_strip
	// rendering.
	enum bitmap_wrap_mode {
		WRAP_REPEAT,
		WRAP_CLAMP
	};

	virtual void	fill_style_disable(int fill_side) {
		if (ctarget) {
		}
	}
	
	virtual void	fill_style_color(int fill_side, rgba c) {
		if (ctarget) {
			cairo_set_source_rgba(ctarget, c.m_r/255.0, c.m_g/255.0, c.m_b/255.0, c.m_a/255.0);
		}
	}

	virtual void	fill_style_bitmap(int fill_side, const gameswf::bitmap_info* bi, const gameswf::matrix& m, gameswf::render_handler::bitmap_wrap_mode wm) {
		if (ctarget) {
		}
	}
	
	virtual void	line_style_disable() {
		if (ctarget) {
		}
	}

	virtual void	line_style_color(rgba c) {
		if (ctarget) {
			cairo_set_source_rgba(ctarget, c.m_r/255.0, c.m_g/255.0, c.m_b/255.0, c.m_a/255.0);
		}
	}

	virtual void	line_style_width(float width) {
		if (ctarget) {
			cairo_set_line_width(ctarget, width);
		}
	}	

	// Special function to draw a rectangular bitmap;
	// intended for textured glyph rendering.  Ignores
	// current transforms.
	virtual void	draw_bitmap(
		const matrix&		m,
		const bitmap_info*	bi,
		const rect&		coords,
		const rect&		uv_coords,
		rgba			color) {
		if (ctarget) {


		}
	}

	virtual void set_antialiased(bool enable) {
		m_enable_antialias = enable;
	}
	
	virtual void begin_submit_mask() {}
	virtual void end_submit_mask() {}
	virtual void disable_mask() {}
};

/**
 * ログ処理呼び出し
 */
static void
log_callback(bool error, const char* message)
{
	if (error) {
		error_log(message);
	} else {
		message_log(message);
	}
}

static tu_file*	file_opener(const char* url)
{
	return new tu_file(url, "rb");
}

static void	fs_callback(gameswf::movie_interface* movie, const char* command, const char* args)
{
	// FS コマンド用
	// 最終的に TJS 呼び出しに置換するべし
	message_log("fs_callback: '");
	message_log(command);
	message_log("' '");
	message_log(args);
	message_log("'\n");
}

static void	test_progress_callback(unsigned int loaded_tags, unsigned int total_tags)
{
	// プログレスバー表示用
	// 最終的に TJS の固有メソッド呼び出しを入れる
}

// レンダラ
static gameswf::render_handler *render = NULL;


/**
 * gamswf の初期化
 */
void
initSWFMovie()
{
	gameswf::register_file_opener_callback(file_opener);
	gameswf::register_fscommand_callback(fs_callback);
	gameswf::register_log_callback(log_callback);
	render = (gameswf::render_handler*)new render_handler_cairo;
	gameswf::set_render_handler(render);
}

/**
 * gameswf の破棄
 */
void
destroySWFMovie()
{
	gameswf::clear();
	delete render;
}
