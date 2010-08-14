#ifndef _GTK_WINDOW_DECORATOR_H
#define _GTK_WINDOW_DECORATOR_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "decoration.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xregion.h>

#ifndef GDK_DISABLE_DEPRECATED
#define GDK_DISABLE_DEPRECATED
#endif

#ifndef GTK_DISABLE_DEPRECATED
#define GTK_DISABLE_DEPRECATED
#endif

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#ifdef USE_GCONF
#include <gconf/gconf-client.h>
#endif

#ifdef USE_DBUS_GLIB
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#endif

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <libwnck/window-action-menu.h>

#ifndef HAVE_LIBWNCK_2_19_4
#define wnck_window_get_client_window_geometry wnck_window_get_geometry
#endif

#include <cairo.h>
#include <cairo-xlib.h>

#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 1, 0)
#define CAIRO_EXTEND_PAD CAIRO_EXTEND_NONE
#endif

#include <pango/pango-context.h>
#include <pango/pangocairo.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include <libintl.h>
#define _(x)  gettext (x)
#define N_(x) x

#ifdef USE_METACITY
#include <metacity-private/theme.h>
#endif

#define METACITY_GCONF_DIR "/apps/metacity/general"

#define COMPIZ_USE_SYSTEM_FONT_KEY		    \
METACITY_GCONF_DIR "/titlebar_uses_system_font"
		    
#define COMPIZ_TITLEBAR_FONT_KEY	\
METACITY_GCONF_DIR "/titlebar_font"

#define COMPIZ_DOUBLE_CLICK_TITLEBAR_KEY	       \
METACITY_GCONF_DIR "/action_double_click_titlebar"

#define COMPIZ_MIDDLE_CLICK_TITLEBAR_KEY	       \
METACITY_GCONF_DIR "/action_middle_click_titlebar"

#define COMPIZ_RIGHT_CLICK_TITLEBAR_KEY	       \
METACITY_GCONF_DIR "/action_right_click_titlebar"

#define COMPIZ_GCONF_DIR1 "/apps/compiz/plugins/decoration/allscreens/options"

#define COMPIZ_SHADOW_RADIUS_KEY \
COMPIZ_GCONF_DIR1 "/shadow_radius"

#define COMPIZ_SHADOW_OPACITY_KEY \
COMPIZ_GCONF_DIR1 "/shadow_opacity"

#define COMPIZ_SHADOW_COLOR_KEY \
COMPIZ_GCONF_DIR1 "/shadow_color"

#define COMPIZ_SHADOW_OFFSET_X_KEY \
COMPIZ_GCONF_DIR1 "/shadow_x_offset"

#define COMPIZ_SHADOW_OFFSET_Y_KEY \
COMPIZ_GCONF_DIR1 "/shadow_y_offset"

#define META_THEME_KEY		\
METACITY_GCONF_DIR "/theme"

#define META_BUTTON_LAYOUT_KEY		\
METACITY_GCONF_DIR "/button_layout"

#define GCONF_DIR "/apps/gwd"

#define USE_META_THEME_KEY	    \
GCONF_DIR "/use_metacity_theme"

#define META_THEME_OPACITY_KEY	        \
GCONF_DIR "/metacity_theme_opacity"

#define META_THEME_SHADE_OPACITY_KEY	      \
GCONF_DIR "/metacity_theme_shade_opacity"

#define META_THEME_ACTIVE_OPACITY_KEY	       \
GCONF_DIR "/metacity_theme_active_opacity"

#define META_THEME_ACTIVE_SHADE_OPACITY_KEY          \
GCONF_DIR "/metacity_theme_active_shade_opacity"

#define BLUR_TYPE_KEY	   \
GCONF_DIR "/blur_type"

#define WHEEL_ACTION_KEY   \
GCONF_DIR "/mouse_wheel_action"

#define DBUS_DEST       "org.freedesktop.compiz"
#define DBUS_PATH       "/org/freedesktop/compiz/decoration/allscreens"
#define DBUS_INTERFACE  "org.freedesktop.compiz"
#define DBUS_METHOD_GET "get"

#define STROKE_ALPHA 0.6

#define ICON_SPACE 20

#define DOUBLE_CLICK_DISTANCE 8.0

#define WM_MOVERESIZE_SIZE_TOPLEFT      0
#define WM_MOVERESIZE_SIZE_TOP          1
#define WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define WM_MOVERESIZE_SIZE_RIGHT        3
#define WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define WM_MOVERESIZE_SIZE_BOTTOM       5
#define WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define WM_MOVERESIZE_SIZE_LEFT         7
#define WM_MOVERESIZE_MOVE              8
#define WM_MOVERESIZE_SIZE_KEYBOARD     9
#define WM_MOVERESIZE_MOVE_KEYBOARD    10

#define SHADOW_RADIUS      8.0
#define SHADOW_OPACITY     0.5
#define SHADOW_OFFSET_X    1
#define SHADOW_OFFSET_Y    1
#define SHADOW_COLOR_RED   0x0000
#define SHADOW_COLOR_GREEN 0x0000
#define SHADOW_COLOR_BLUE  0x0000

#define META_OPACITY              0.75
#define META_SHADE_OPACITY        TRUE
#define META_ACTIVE_OPACITY       1.0
#define META_ACTIVE_SHADE_OPACITY TRUE

#define META_MAXIMIZED (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | \
WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY)

#define CMDLINE_OPACITY              (1 << 0)
#define CMDLINE_OPACITY_SHADE        (1 << 1)
#define CMDLINE_ACTIVE_OPACITY       (1 << 2)
#define CMDLINE_ACTIVE_OPACITY_SHADE (1 << 3)
#define CMDLINE_BLUR                 (1 << 4)
#define CMDLINE_THEME                (1 << 5)

#define MWM_HINTS_DECORATIONS (1L << 1)

#define MWM_DECOR_ALL      (1L << 0)
#define MWM_DECOR_BORDER   (1L << 1)
#define MWM_DECOR_HANDLE   (1L << 2)
#define MWM_DECOR_TITLE    (1L << 3)
#define MWM_DECOR_MENU     (1L << 4)
#define MWM_DECOR_MINIMIZE (1L << 5)
#define MWM_DECOR_MAXIMIZE (1L << 6)

#define PROP_MOTIF_WM_HINT_ELEMENTS 3

typedef struct {
unsigned long flags;
unsigned long functions;
unsigned long decorations;
} MwmHints;

enum {
    CLICK_ACTION_NONE,
    CLICK_ACTION_SHADE,
    CLICK_ACTION_MAXIMIZE,
    CLICK_ACTION_MINIMIZE,
    CLICK_ACTION_RAISE,
    CLICK_ACTION_LOWER,
    CLICK_ACTION_MENU
};

enum {
    WHEEL_ACTION_NONE,
    WHEEL_ACTION_SHADE
};

#define DOUBLE_CLICK_ACTION_DEFAULT CLICK_ACTION_MAXIMIZE
#define MIDDLE_CLICK_ACTION_DEFAULT CLICK_ACTION_LOWER
#define RIGHT_CLICK_ACTION_DEFAULT  CLICK_ACTION_MENU
#define WHEEL_ACTION_DEFAULT        WHEEL_ACTION_NONE

int double_click_action;
int middle_click_action;
int right_click_action;
int wheel_action;

extern gboolean minimal;
extern double decoration_alpha;

#define SWITCHER_SPACE 40

extern decor_extents_t _shadow_extents;
extern decor_extents_t _win_extents;
extern decor_extents_t _max_win_extents;
extern decor_extents_t _default_win_extents;
extern decor_extents_t _switcher_extents;

extern int titlebar_height;
extern int max_titlebar_height;

extern decor_context_t window_context;

extern decor_context_t max_window_context;

extern decor_context_t switcher_context;

extern decor_context_t shadow_context;

extern gdouble shadow_radius;
extern gdouble shadow_opacity;
extern gushort shadow_color[3];
extern gint    shadow_offset_x;
extern gint    shadow_offset_y;

#ifdef USE_METACITY
extern double   meta_opacity;
extern gboolean meta_shade_opacity;
extern double   meta_active_opacity;
extern gboolean meta_active_shade_opacity;

extern gboolean         meta_button_layout_set;
extern MetaButtonLayout meta_button_layout;
#endif

extern guint cmdline_options;

extern decor_shadow_t *no_border_shadow;
extern decor_shadow_t *border_shadow;
extern decor_shadow_t *max_border_shadow;
extern decor_shadow_t *switcher_shadow;

extern GdkPixmap *decor_normal_pixmap;
extern GdkPixmap *decor_active_pixmap;

extern Atom frame_input_window_atom;
extern Atom frame_output_window_atom;
extern Atom win_decor_atom;
extern Atom win_blur_decor_atom;
extern Atom wm_move_resize_atom;
extern Atom restack_window_atom;
extern Atom select_window_atom;
extern Atom mwm_hints_atom;
extern Atom switcher_fg_atom;

extern Atom toolkit_action_atom;
extern Atom toolkit_action_window_menu_atom;
extern Atom toolkit_action_force_quit_dialog_atom;

extern Time dm_sn_timestamp;

#define C(name) { 0, XC_ ## name }

struct _cursor {
    Cursor	 cursor;
    unsigned int shape;
};

extern struct _cursor cursor[3][3];

#define BUTTON_CLOSE   0
#define BUTTON_MAX     1
#define BUTTON_MIN     2
#define BUTTON_MENU    3
#define BUTTON_SHADE   4
#define BUTTON_ABOVE   5
#define BUTTON_STICK   6
#define BUTTON_UNSHADE 7
#define BUTTON_UNABOVE 8
#define BUTTON_UNSTICK 9
#define BUTTON_NUM     10

struct _pos {
    int x, y, w, h;
    int xw, yh, ww, hh, yth, hth;
};

extern struct _pos pos[3][3], bpos[];

typedef struct _decor_color {
    double r;
    double g;
    double b;
} decor_color_t;


#define IN_EVENT_WINDOW      (1 << 0)
#define PRESSED_EVENT_WINDOW (1 << 1)

typedef struct _decor_event {
    guint time;
    guint window;
    guint x;
    guint y;
    guint x_root;
    guint y_root;
    guint button;
} decor_event;

typedef enum _decor_event_type {
    GButtonPress = 1,
    GButtonRelease,
    GEnterNotify,
    GLeaveNotify,
    GMotionNotify
} decor_event_type;

typedef void (*event_callback) (WnckWindow       *win,
				decor_event      *gtkwd_event,
				decor_event_type gtkwd_type);

typedef struct {
    Window         window;
    Box            pos;
    event_callback callback;
} event_window;

typedef struct _decor {
    WnckWindow	      *win;
    event_window      event_windows[3][3];
    event_window      button_windows[BUTTON_NUM];
    Box		      *last_pos_entered;
    guint	      button_states[BUTTON_NUM];
    GdkPixmap	      *pixmap;
    GdkPixmap	      *buffer_pixmap;
    GdkWindow	      *frame_window;
    GtkWidget         *decor_window;
    GtkWidget	      *decor_event_box;
    GtkWidget         *decor_image;
    GdkGC	      *gc;
    decor_layout_t    border_layout;
    decor_context_t   *context;
    decor_shadow_t    *shadow;
    Picture	      picture;
    gint	      button_width;
    gint	      width;
    gint	      height;
    gint	      client_width;
    gint	      client_height;
    gboolean	      decorated;
    gboolean	      active;
    PangoLayout	      *layout;
    gchar	      *name;
    cairo_pattern_t   *icon;
    GdkPixmap	      *icon_pixmap;
    GdkPixbuf	      *icon_pixbuf;
    WnckWindowState   state;
    WnckWindowActions actions;
    XID		      prop_xid;
    GtkWidget	      *force_quit_dialog;
    Bool	      created;
    void	      (*draw) (struct _decor *d);
} decor_t;

void     (*theme_draw_window_decoration)    (decor_t *d);
gboolean (*theme_calc_decoration_size)      (decor_t *d,
					     int     client_width,
					     int     client_height,
					     int     text_width,
					     int     *width,
					     int     *height);
void     (*theme_update_border_extents)     (gint    text_height);
void     (*theme_get_event_window_position) (decor_t *d,
					     gint    i,
					     gint    j,
					     gint    width,
					     gint    height,
					     gint    *x,
					     gint    *y,
					     gint    *w,
					     gint    *h);
gboolean (*theme_get_button_position)       (decor_t *d,
					     gint    i,
					     gint    width,
					     gint    height,
					     gint    *x,
					     gint    *y,
					     gint    *w,
					     gint    *h);

extern char *program_name;

extern GtkWidget     *style_window_rgba;
extern GtkWidget     *style_window_rgb;
extern GtkWidget     *switcher_label;

extern GHashTable    *frame_table;
extern GtkWidget     *action_menu;
extern gboolean      action_menu_mapped;
extern decor_color_t _title_color[2];
extern PangoContext  *pango_context;
extern gint	     double_click_timeout;

extern GtkWidget     *tip_window;
extern GtkWidget     *tip_label;
extern GTimeVal	     tooltip_last_popdown;
extern gint	     tooltip_timer_tag;

extern GSList *draw_list;
extern guint  draw_idle_id;

extern PangoFontDescription *titlebar_font;
extern gboolean		    use_system_font;
extern gint		    text_height;

#define BLUR_TYPE_NONE     0
#define BLUR_TYPE_TITLEBAR 1
#define BLUR_TYPE_ALL      2

extern gint blur_type;

extern GdkPixmap *switcher_pixmap;
extern GdkPixmap *switcher_buffer_pixmap;
extern gint      switcher_width;
extern gint      switcher_height;
extern Window    switcher_selected_window;

extern XRenderPictFormat *xformat_rgba;
extern XRenderPictFormat *xformat_rgb;

/* gtk-window-decorator.c */

double
dist (double x1, double y1,
      double x2, double y2);

/* decorator.c */

gboolean
update_window_decoration_size (WnckWindow *win);

void
update_window_decoration_name (WnckWindow *win);

gint
max_window_name_width (WnckWindow *win);

void
update_default_decorations (GdkScreen *screen);

void
update_window_decoration_state (WnckWindow *win);

void
update_window_decoration_actions (WnckWindow *win);

void
update_window_decoration_icon (WnckWindow *win);

void
update_event_windows (WnckWindow *win);

int
update_shadow (void);

void
update_titlebar_font (void);

void
update_window_decoration_name (WnckWindow *win);

void
update_window_decoration (WnckWindow *win);

void
queue_decor_draw (decor_t *d);

/* wnck.c*/

void
decorations_changed (WnckScreen *screen);

void
connect_screen (WnckScreen *screen);

void
add_frame_window (WnckWindow *win,
		  Window     frame,
		  Bool	     mode);

void
remove_frame_window (WnckWindow *win);

void
restack_window (WnckWindow *win,
		int	   stack_mode);


/* blur.c */

void
decor_update_blur_property (decor_t *d,
			    int     width,
			    int     height,
			    Region  top_region,
			    int     top_offset,
			    Region  bottom_region,
			    int     bottom_offset,
			    Region  left_region,
			    int     left_offset,
			    Region  right_region,
			    int     right_offset);

/* decorprops.c */

void
decor_update_window_property (decor_t *d);

void
decor_update_switcher_property (decor_t *d);

/* cairo.c */

#define CORNER_TOPLEFT     (1 << 0)
#define CORNER_TOPRIGHT    (1 << 1)
#define CORNER_BOTTOMRIGHT (1 << 2)
#define CORNER_BOTTOMLEFT  (1 << 3)

#define SHADE_LEFT   (1 << 0)
#define SHADE_RIGHT  (1 << 1)
#define SHADE_TOP    (1 << 2)
#define SHADE_BOTTOM (1 << 3)

void
draw_shadow_background (decor_t		*d,
			cairo_t		*cr,
			decor_shadow_t  *s,
			decor_context_t *c);

void
draw_window_decoration (decor_t *d);

void
fill_rounded_rectangle (cairo_t       *cr,
			double        x,
			double        y,
			double        w,
			double        h,
			double	      radius,
			int	      corner,
			decor_color_t *c0,
			double        alpha0,
			decor_color_t *c1,
			double	      alpha1,
			int	      gravity);

void
rounded_rectangle (cairo_t *cr,
		   double  x,
		   double  y,
		   double  w,
		   double  h,
		   double  radius,
		   int	   corner);

gboolean
calc_decoration_size (decor_t *d,
		      gint    w,
		      gint    h,
		      gint    name_width,
		      gint    *width,
		      gint    *height);

void
update_border_extents (gint text_height);

gboolean
get_button_position (decor_t *d,
		     gint    i,
		     gint    width,
		     gint    height,
		     gint    *x,
		     gint    *y,
		     gint    *w,
		     gint    *h);

void
get_event_window_position (decor_t *d,
			   gint    i,
			   gint    j,
			   gint    width,
			   gint    height,
			   gint    *x,
			   gint    *y,
			   gint    *w,
			   gint    *h);

/* gdk.c */

void
gdk_cairo_set_source_color_alpha (cairo_t  *cr,
				  GdkColor *color,
				  double   alpha);

inline GdkWindow *
create_gdk_window (Window xframe);

GdkColormap *
get_colormap_for_drawable (GdkDrawable *d);

XRenderPictFormat *
get_format_for_drawable (decor_t *d, GdkDrawable *drawable);

GdkPixmap *
create_pixmap (int w,
	       int h,
	       int depth);

GdkPixmap *
pixmap_new_from_pixbuf (GdkPixbuf *pixbuf, int depth);

/* metacity.c */

void
meta_draw_window_decoration (decor_t *d);

void
meta_get_decoration_geometry (decor_t		*d,
			      MetaTheme	        *theme,
			      MetaFrameFlags    *flags,
			      MetaFrameGeometry *fgeom,
			      MetaButtonLayout  *button_layout,
			      GdkRectangle      *clip);

void
meta_calc_button_size (decor_t *d);

gboolean
meta_calc_decoration_size (decor_t *d,
			   gint    w,
			   gint    h,
			   gint    name_width,
			   gint    *width,
			   gint    *height);

gboolean
meta_get_button_position (decor_t *d,
			  gint    i,
			  gint	  width,
			  gint	  height,
			  gint    *x,
			  gint    *y,
			  gint    *w,
			  gint    *h);

gboolean
meta_button_present (MetaButtonLayout   *button_layout,
		     MetaButtonFunction function);

void
meta_get_event_window_position (decor_t *d,
				gint    i,
				gint    j,
				gint	width,
				gint	height,
				gint    *x,
				gint    *y,
				gint    *w,
				gint    *h);
void
meta_update_border_extents (gint text_height);

void
meta_update_button_layout (const char *value);

/* switcher.c */

#define SWITCHER_ALPHA 0xa0a0

void
draw_switcher_decoration (decor_t *d);

gboolean
update_switcher_window (WnckWindow *win,
			Window     selected);

/* events.c */

void
move_resize_window (WnckWindow *win,
		    int	       direction,
		    decor_event *gtkwd_event);

void
common_button_event (WnckWindow *win,
		     decor_event *gtkwd_event,
		     decor_event_type gtkwd_type,
		     int	button,
		     int	max,
		     char	*tooltip);

void
close_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
max_button_event (WnckWindow *win,
		  decor_event *gtkwd_event,
		  decor_event_type gtkwd_type);

void
min_button_event (WnckWindow *win,
		  decor_event *gtkwd_event,
		  decor_event_type gtkwd_type);

void
menu_button_event (WnckWindow *win,
		   decor_event *gtkwd_event,
		   decor_event_type gtkwd_type);

void
shade_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
above_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
stick_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);
void
unshade_button_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type gtkwd_type);

void
unabove_button_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type gtkwd_type);

void
unstick_button_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type gtkwd_type);

void
handle_title_button_event (WnckWindow   *win,
			   int          action,
			   decor_event *gtkwd_event);

void
handle_mouse_wheel_title_event (WnckWindow   *win,
				unsigned int button);

void
title_event (WnckWindow       *win,
	     decor_event      *gtkwd_event,
	     decor_event_type gtkwd_type);

void
frame_common_event (WnckWindow       *win,
		    int              direction,
		    decor_event      *gtkwd_event,
		    decor_event_type gtkwd_type);

void
top_left_event (WnckWindow       *win,
		decor_event      *gtkwd_event,
		decor_event_type gtkwd_type);

void
top_event (WnckWindow       *win,
	   decor_event      *gtkwd_event,
	   decor_event_type gtkwd_type);

void
top_right_event (WnckWindow       *win,
		 decor_event      *gtkwd_event,
		 decor_event_type gtkwd_type);

void
left_event (WnckWindow       *win,
	    decor_event      *gtkwd_event,
	    decor_event_type gtkwd_type);
void
right_event (WnckWindow       *win,
	     decor_event      *gtkwd_event,
	     decor_event_type gtkwd_type);

void
bottom_left_event (WnckWindow *win,
		   decor_event *gtkwd_event,
		   decor_event_type gtkwd_type);

void
bottom_event (WnckWindow *win,
	      decor_event *gtkwd_event,
	      decor_event_type gtkwd_type);
void
bottom_right_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
frame_window_realized (GtkWidget *widget,
		       gpointer  data);

event_callback
find_event_callback_for_point (decor_t *d,
			       int     x,
			       int     y,
			       Bool    *enter,
			       Bool    *leave,
			       BoxPtr  *entered_box);

event_callback
find_leave_event_callback (decor_t *d);

void
frame_handle_button_press (GtkWidget      *widget,
			   GdkEventButton *event,
			   gpointer       user_data);

void
frame_handle_button_release (GtkWidget      *widget,
			     GdkEventButton *event,
			     gpointer       user_data);

void
frame_handle_motion (GtkWidget      *widget,
		     GdkEventMotion *event,
		     gpointer       user_data);

GdkFilterReturn
selection_event_filter_func (GdkXEvent *gdkxevent,
			     GdkEvent  *event,
			     gpointer  data);

GdkFilterReturn
event_filter_func (GdkXEvent *gdkxevent,
		   GdkEvent  *event,
		   gpointer  data);

/* tooltip.c */

gboolean
create_tooltip_window (void);

void
handle_tooltip_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type   gtkwd_type,
		      guint	 state,
		      const char *tip);

/* forcequit.c */

void
show_force_quit_dialog (WnckWindow *win,
			Time        timestamp);

void
hide_force_quit_dialog (WnckWindow *win);

/* actionmenu.c */

void
action_menu_map (WnckWindow *win,
		 long	     button,
		 Time	     time);

/* util.c */

double
square (double x);

double
dist (double x1, double y1,
      double x2, double y2);

void
shade (const decor_color_t *a,
       decor_color_t	   *b,
       float		   k);

gboolean
get_window_prop (Window xwindow,
		 Atom   atom,
		 Window *val);

unsigned int
get_mwm_prop (Window xwindow);


/* style.c */

void
update_style (GtkWidget *widget);

void
style_changed (GtkWidget *widget);

/* settings.c */

gboolean
init_settings (WnckScreen *screen);

#endif