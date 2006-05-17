/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>

#include <compiz.h>

#define SWITCH_INITIATE_KEY_DEFAULT       "Tab"
#define SWITCH_INITIATE_MODIFIERS_DEFAULT CompAltMask

#define SWITCH_INITIATE_ALL_MODIFIERS_DEFAULT ControlMask

#define SWITCH_PREV_WINDOW_MODIFIERS_DEFAULT ShiftMask

#define SWITCH_SPEED_DEFAULT   1.5f
#define SWITCH_SPEED_MIN       0.1f
#define SWITCH_SPEED_MAX       50.0f
#define SWITCH_SPEED_PRECISION 0.1f

#define SWITCH_TIMESTEP_DEFAULT   1.2f
#define SWITCH_TIMESTEP_MIN       0.1f
#define SWITCH_TIMESTEP_MAX       50.0f
#define SWITCH_TIMESTEP_PRECISION 0.1f

#define SWITCH_MIPMAP_DEFAULT TRUE

#define SWITCH_BRINGTOFRONT_DEFAULT TRUE

#define SWITCH_SATURATION_DEFAULT 100
#define SWITCH_SATURATION_MIN     0
#define SWITCH_SATURATION_MAX     100

#define SWITCH_BRIGHTNESS_DEFAULT 65
#define SWITCH_BRIGHTNESS_MIN     0
#define SWITCH_BRIGHTNESS_MAX     100

#define SWITCH_OPACITY_DEFAULT    40
#define SWITCH_OPACITY_MIN        0
#define SWITCH_OPACITY_MAX        100

#define SWITCH_ZOOM_DEFAULT   1.0f
#define SWITCH_ZOOM_MIN       0.0f
#define SWITCH_ZOOM_MAX       5.0f
#define SWITCH_ZOOM_PRECISION 0.1f

#define SWITCH_ICON_DEFAULT TRUE

#define SWITCH_MINIMIZED_DEFAULT TRUE

static char *winType[] = {
    "Toolbar",
    "Utility",
    "Dialog",
    "Fullscreen",
    "Normal",
};
#define N_WIN_TYPE (sizeof (winType) / sizeof (winType[0]))

static int displayPrivateIndex;

typedef struct _SwitchDisplay {
    int		    screenPrivateIndex;
    HandleEventProc handleEvent;

    Atom selectWinAtom;
} SwitchDisplay;

#define SWITCH_SCREEN_OPTION_INITIATE	  0
#define SWITCH_SCREEN_OPTION_INITIATE_ALL 1
#define SWITCH_SCREEN_OPTION_PREV_WINDOW  2
#define SWITCH_SCREEN_OPTION_SPEED	  3
#define SWITCH_SCREEN_OPTION_TIMESTEP	  4
#define SWITCH_SCREEN_OPTION_WINDOW_TYPE  5
#define SWITCH_SCREEN_OPTION_MIPMAP	  6
#define SWITCH_SCREEN_OPTION_SATURATION	  7
#define SWITCH_SCREEN_OPTION_BRIGHTNESS	  8
#define SWITCH_SCREEN_OPTION_OPACITY	  9
#define SWITCH_SCREEN_OPTION_BRINGTOFRONT 10
#define SWITCH_SCREEN_OPTION_ZOOM	  11
#define SWITCH_SCREEN_OPTION_ICON	  12
#define SWITCH_SCREEN_OPTION_MINIMIZED	  13
#define SWITCH_SCREEN_OPTION_NUM	  14

typedef struct _SwitchScreen {
    PreparePaintScreenProc preparePaintScreen;
    DonePaintScreenProc    donePaintScreen;
    PaintScreenProc	   paintScreen;
    PaintWindowProc        paintWindow;
    DamageWindowRectProc   damageWindowRect;

    CompOption opt[SWITCH_SCREEN_OPTION_NUM];
    CompOption init_all_bind, prev_bind, prev_all_bind;

    Window popupWindow;

    Window	 selectedWindow;
    Window	 zoomedWindow;
    unsigned int lastActiveNum;

    float speed;
    float timestep;
    float zoom;

    unsigned int wMask;

    int grabIndex;

    Bool switching;
    Bool zooming;

    int moreAdjust;

    GLfloat mVelocity;
    GLfloat tVelocity;
    GLfloat sVelocity;

    CompWindow **windows;
    int        windowsSize;
    int        nWindows;

    int pos;
    int move;

    float translate;
    float sTranslate;

    GLushort saturation;
    GLushort brightness;
    GLushort opacity;

    Bool bringToFront;
    Bool allWindows;
} SwitchScreen;

#define MwmHintsDecorations (1L << 1)

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
} MwmHints;

#define SELECT_WIN_PROP "_SWITCH_SELECT_WINDOW"

#define WIDTH  212
#define HEIGHT 192
#define SPACE  10

#define SWITCH_ZOOM 0.1f

#define BOX_WIDTH 3

#define ICON_SIZE 48

static float _boxVertices[] =
{
    -(WIDTH >> 1), 0,
    -(WIDTH >> 1), BOX_WIDTH,
     (WIDTH >> 1), BOX_WIDTH,
     (WIDTH >> 1), 0,

    -(WIDTH >> 1),	       BOX_WIDTH,
    -(WIDTH >> 1),	       HEIGHT - BOX_WIDTH,
    -(WIDTH >> 1) + BOX_WIDTH, HEIGHT - BOX_WIDTH,
    -(WIDTH >> 1) + BOX_WIDTH, 0,

     (WIDTH >> 1) - BOX_WIDTH, BOX_WIDTH,
     (WIDTH >> 1) - BOX_WIDTH, HEIGHT - BOX_WIDTH,
     (WIDTH >> 1),	       HEIGHT - BOX_WIDTH,
     (WIDTH >> 1),	       0,

    -(WIDTH >> 1), HEIGHT - BOX_WIDTH,
    -(WIDTH >> 1), HEIGHT,
     (WIDTH >> 1), HEIGHT,
     (WIDTH >> 1), HEIGHT - BOX_WIDTH
};

#define WINDOW_WIDTH(count) (WIDTH * (count) + (SPACE << 1))
#define WINDOW_HEIGHT (HEIGHT + (SPACE << 1))

#define GET_SWITCH_DISPLAY(d)				       \
    ((SwitchDisplay *) (d)->privates[displayPrivateIndex].ptr)

#define SWITCH_DISPLAY(d)		       \
    SwitchDisplay *sd = GET_SWITCH_DISPLAY (d)

#define GET_SWITCH_SCREEN(s, sd)				   \
    ((SwitchScreen *) (s)->privates[(sd)->screenPrivateIndex].ptr)

#define SWITCH_SCREEN(s)						      \
    SwitchScreen *ss = GET_SWITCH_SCREEN (s, GET_SWITCH_DISPLAY (s->display))

#define NUM_OPTIONS(s) (sizeof ((s)->opt) / sizeof (CompOption))

static CompOption *
switchGetScreenOptions (CompScreen *screen,
			int	   *count)
{
    SWITCH_SCREEN (screen);

    *count = NUM_OPTIONS (ss);
    return ss->opt;
}

static Bool
switchSetScreenOption (CompScreen      *screen,
		       char	       *name,
		       CompOptionValue *value)
{
    CompOption  *o;
    int	        index;
    CompBinding binding;

    SWITCH_SCREEN (screen);

    o = compFindOption (ss->opt, NUM_OPTIONS (ss), name, &index);
    if (!o)
	return FALSE;

    switch (index) {
    case SWITCH_SCREEN_OPTION_INITIATE:
	if (value->bind.type != CompBindingTypeKey)
	    return FALSE;
	if (!addScreenBinding (screen, &value->bind))
	    return FALSE;
	removeScreenBinding (screen, &o->value.bind);

	/* fall through */
    case SWITCH_SCREEN_OPTION_INITIATE_ALL:
    case SWITCH_SCREEN_OPTION_PREV_WINDOW:
	if (value->bind.type != CompBindingTypeKey)
	    return FALSE;
	if (!compSetBindingOption (o, value))
	    return FALSE;

	binding = ss->opt[SWITCH_SCREEN_OPTION_INITIATE].value.bind;
	binding.u.key.modifiers |=
	    ss->opt[SWITCH_SCREEN_OPTION_INITIATE_ALL].value.bind.u.key.modifiers;
	if (addScreenBinding (screen, &binding)) {
	    removeScreenBinding (screen, &ss->init_all_bind.value.bind);
	    ss->init_all_bind.value.bind = binding;
	}

	binding.u.key.modifiers |=
	    ss->opt[SWITCH_SCREEN_OPTION_PREV_WINDOW].value.bind.u.key.modifiers;
	if (addScreenBinding (screen, &binding)) {
	    removeScreenBinding (screen, &ss->prev_all_bind.value.bind);
	    ss->prev_all_bind.value.bind = binding;
	}

	binding = ss->opt[SWITCH_SCREEN_OPTION_INITIATE].value.bind;
	binding.u.key.modifiers |=
	    ss->opt[SWITCH_SCREEN_OPTION_PREV_WINDOW].value.bind.u.key.modifiers;
	if (addScreenBinding (screen, &binding)) {
	    removeScreenBinding (screen, &ss->prev_bind.value.bind);
	    ss->prev_bind.value.bind = binding;
	}

	return TRUE;
    case SWITCH_SCREEN_OPTION_SPEED:
	if (compSetFloatOption (o, value))
	{
	    ss->speed = o->value.f;
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_TIMESTEP:
	if (compSetFloatOption (o, value))
	{
	    ss->timestep = o->value.f;
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_WINDOW_TYPE:
	if (compSetOptionList (o, value))
	{
	    ss->wMask = compWindowTypeMaskFromStringList (&o->value);
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_MIPMAP:
    case SWITCH_SCREEN_OPTION_ICON:
    case SWITCH_SCREEN_OPTION_MINIMIZED:
	if (compSetBoolOption (o, value))
	    return TRUE;
	break;
    case SWITCH_SCREEN_OPTION_SATURATION:
	if (compSetIntOption (o, value))
	{
	    ss->saturation = (COLOR * o->value.i) / 100;
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_BRIGHTNESS:
	if (compSetIntOption (o, value))
	{
	    ss->brightness = (0xffff * o->value.i) / 100;
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_OPACITY:
	if (compSetIntOption (o, value))
	{
	    ss->opacity = (OPAQUE * o->value.i) / 100;
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_BRINGTOFRONT:
	if (compSetBoolOption (o, value))
	{
	    ss->bringToFront = o->value.b;
	    return TRUE;
	}
	break;
    case SWITCH_SCREEN_OPTION_ZOOM:
	if (compSetFloatOption (o, value))
	{
	    if (o->value.f < 0.05f)
	    {
		ss->zooming = FALSE;
		ss->zoom    = 0.0f;
	    }
	    else
	    {
		ss->zooming = TRUE;
		ss->zoom    = o->value.f / 30.0f;
	    }

	    return TRUE;
	}
    default:
	break;
    }

    return FALSE;
}

static void
switchScreenInitOptions (SwitchScreen *ss,
			 Display      *display)
{
    CompOption *o;
    int	       i;

    o = &ss->opt[SWITCH_SCREEN_OPTION_INITIATE];
    o->name			  = "initiate";
    o->shortDesc		  = "Initiate Window Switcher";
    o->longDesc			  = "Show switcher";
    o->type			  = CompOptionTypeBinding;
    o->value.bind.type		  = CompBindingTypeKey;
    o->value.bind.u.key.modifiers = SWITCH_INITIATE_MODIFIERS_DEFAULT;
    o->value.bind.u.key.keycode   =
	XKeysymToKeycode (display,
			  XStringToKeysym (SWITCH_INITIATE_KEY_DEFAULT));

    ss->init_all_bind = ss->prev_bind = ss->prev_all_bind = *o;

    o = &ss->opt[SWITCH_SCREEN_OPTION_INITIATE_ALL];
    o->name			  = "initiate_all";
    o->shortDesc		  = "Initiate All Windows";
    o->longDesc			  = "Modifier to show switcher for all windows";
    o->type			  = CompOptionTypeBinding;
    o->value.bind.type		  = CompBindingTypeKey;
    o->value.bind.u.key.modifiers = SWITCH_INITIATE_ALL_MODIFIERS_DEFAULT;
    o->value.bind.u.key.keycode   = 0;

    ss->init_all_bind.value.bind.u.key.modifiers |=
	o->value.bind.u.key.modifiers;
    ss->prev_all_bind.value.bind.u.key.modifiers |=
	o->value.bind.u.key.modifiers;

    o = &ss->opt[SWITCH_SCREEN_OPTION_PREV_WINDOW];
    o->name			  = "prev_window";
    o->shortDesc		  = "Prev Window";
    o->longDesc			  = "Modifier key to select previous window";
    o->type			  = CompOptionTypeBinding;
    o->value.bind.type		  = CompBindingTypeKey;
    o->value.bind.u.key.modifiers = SWITCH_PREV_WINDOW_MODIFIERS_DEFAULT;
    o->value.bind.u.key.keycode   = 0;

    ss->prev_bind.value.bind.u.key.modifiers |=
	o->value.bind.u.key.modifiers;
    ss->prev_all_bind.value.bind.u.key.modifiers |=
	o->value.bind.u.key.modifiers;

    o = &ss->opt[SWITCH_SCREEN_OPTION_SPEED];
    o->name		= "speed";
    o->shortDesc	= "Speed";
    o->longDesc		= "Switcher speed";
    o->type		= CompOptionTypeFloat;
    o->value.f		= SWITCH_SPEED_DEFAULT;
    o->rest.f.min	= SWITCH_SPEED_MIN;
    o->rest.f.max	= SWITCH_SPEED_MAX;
    o->rest.f.precision = SWITCH_SPEED_PRECISION;

    o = &ss->opt[SWITCH_SCREEN_OPTION_TIMESTEP];
    o->name		= "timestep";
    o->shortDesc	= "Timestep";
    o->longDesc		= "Switcher timestep";
    o->type		= CompOptionTypeFloat;
    o->value.f		= SWITCH_TIMESTEP_DEFAULT;
    o->rest.f.min	= SWITCH_TIMESTEP_MIN;
    o->rest.f.max	= SWITCH_TIMESTEP_MAX;
    o->rest.f.precision = SWITCH_TIMESTEP_PRECISION;

    o = &ss->opt[SWITCH_SCREEN_OPTION_WINDOW_TYPE];
    o->name	         = "window_types";
    o->shortDesc         = "Window Types";
    o->longDesc	         = "Window types that should shown in switcher";
    o->type	         = CompOptionTypeList;
    o->value.list.type   = CompOptionTypeString;
    o->value.list.nValue = N_WIN_TYPE;
    o->value.list.value  = malloc (sizeof (CompOptionValue) * N_WIN_TYPE);
    for (i = 0; i < N_WIN_TYPE; i++)
	o->value.list.value[i].s = strdup (winType[i]);
    o->rest.s.string     = windowTypeString;
    o->rest.s.nString    = nWindowTypeString;

    ss->wMask = compWindowTypeMaskFromStringList (&o->value);

    o = &ss->opt[SWITCH_SCREEN_OPTION_MIPMAP];
    o->name	  = "mipmap";
    o->shortDesc  = "Mipmap";
    o->longDesc	  = "Generate mipmaps when possible for higher quality scaling";
    o->type	  = CompOptionTypeBool;
    o->value.b    = SWITCH_MIPMAP_DEFAULT;

    o = &ss->opt[SWITCH_SCREEN_OPTION_SATURATION];
    o->name	  = "saturation";
    o->shortDesc  = "Saturation";
    o->longDesc	  = "Amount of saturation in percent";
    o->type	  = CompOptionTypeInt;
    o->value.i    = SWITCH_SATURATION_DEFAULT;
    o->rest.i.min = SWITCH_SATURATION_MIN;
    o->rest.i.max = SWITCH_SATURATION_MAX;

    o = &ss->opt[SWITCH_SCREEN_OPTION_BRIGHTNESS];
    o->name	  = "brightness";
    o->shortDesc  = "Brightness";
    o->longDesc	  = "Amount of brightness in percent";
    o->type	  = CompOptionTypeInt;
    o->value.i    = SWITCH_BRIGHTNESS_DEFAULT;
    o->rest.i.min = SWITCH_BRIGHTNESS_MIN;
    o->rest.i.max = SWITCH_BRIGHTNESS_MAX;

    o = &ss->opt[SWITCH_SCREEN_OPTION_OPACITY];
    o->name	  = "opacity";
    o->shortDesc  = "Opacity";
    o->longDesc	  = "Amount of opacity in percent";
    o->type	  = CompOptionTypeInt;
    o->value.i    = SWITCH_OPACITY_DEFAULT;
    o->rest.i.min = SWITCH_OPACITY_MIN;
    o->rest.i.max = SWITCH_OPACITY_MAX;

    o = &ss->opt[SWITCH_SCREEN_OPTION_BRINGTOFRONT];
    o->name	  = "bring_to_front";
    o->shortDesc  = "Bring To Front";
    o->longDesc	  = "Bring selected window to front";
    o->type	  = CompOptionTypeBool;
    o->value.b    = SWITCH_BRINGTOFRONT_DEFAULT;

    o = &ss->opt[SWITCH_SCREEN_OPTION_ZOOM];
    o->name		= "zoom";
    o->shortDesc	= "Zoom";
    o->longDesc		= "Distance desktop should be zoom out while "
	"switching windows";
    o->type		= CompOptionTypeFloat;
    o->value.f		= SWITCH_ZOOM_DEFAULT;
    o->rest.f.min	= SWITCH_ZOOM_MIN;
    o->rest.f.max	= SWITCH_ZOOM_MAX;
    o->rest.f.precision = SWITCH_ZOOM_PRECISION;

    o = &ss->opt[SWITCH_SCREEN_OPTION_ICON];
    o->name	  = "icon";
    o->shortDesc  = "Icon";
    o->longDesc	  = "Show icon next to thumbnail";
    o->type	  = CompOptionTypeBool;
    o->value.b    = SWITCH_ICON_DEFAULT;

    o = &ss->opt[SWITCH_SCREEN_OPTION_MINIMIZED];
    o->name	  = "minimized";
    o->shortDesc  = "Minimized";
    o->longDesc	  = "Show minimized windows";
    o->type	  = CompOptionTypeBool;
    o->value.b    = SWITCH_MINIMIZED_DEFAULT;
}

static void
setSelectedWindowHint (CompScreen *s)
{
    SWITCH_DISPLAY (s->display);
    SWITCH_SCREEN (s);

    XChangeProperty (s->display->display, ss->popupWindow, sd->selectWinAtom,
		     XA_WINDOW, 32, PropModeReplace,
		     (unsigned char *) &ss->selectedWindow, 1);
}

static Bool
isSwitchWin (CompWindow *w)
{
    SWITCH_SCREEN (w->screen);

    if (!w->mapNum || w->attrib.map_state != IsViewable)
    {
	if (ss->opt[SWITCH_SCREEN_OPTION_MINIMIZED].value.b)
	{
	    if (!w->minimized && !w->inShowDesktopMode)
		return FALSE;
	}
	else
	{
	    return FALSE;
	}
    }

    if (w->attrib.override_redirect)
	return FALSE;

    if (!(ss->wMask & w->type))
	return FALSE;

    if (w->state & CompWindowStateSkipTaskbarMask)
	return FALSE;

    if (!ss->allWindows)
    {
	if (!w->mapNum || w->attrib.map_state != IsViewable)
	{
	    if (w->serverX + w->width  <= 0    ||
		w->serverY + w->height <= 0    ||
		w->serverX >= w->screen->width ||
		w->serverY >= w->screen->height)
		return FALSE;
	}
	else
	{
	    if (!(*w->screen->focusWindow) (w))
		return FALSE;
	}
    }

    return TRUE;
}

static int
compareWindows (const void *elem1,
		const void *elem2)
{
    CompWindow *w1 = *((CompWindow **) elem1);
    CompWindow *w2 = *((CompWindow **) elem2);

    return w2->activeNum - w1->activeNum;
}

static void
switchAddWindowToList (CompScreen *s,
		       CompWindow *w)
{
    SWITCH_SCREEN (s);

    if (ss->windowsSize <= ss->nWindows)
    {
	ss->windows = realloc (ss->windows,
			       sizeof (CompWindow *) * (ss->nWindows + 32));
	if (!ss->windows)
	    return;

	ss->windowsSize = ss->nWindows + 32;
    }

    ss->windows[ss->nWindows++] = w;
}

static void
switchUpdateWindowList (CompScreen *s,
			int	   count)
{
    CompWindow *w;

    SWITCH_SCREEN (s);

    ss->nWindows = 0;

    for (w = s->windows; w; w = w->next)
    {
	if (isSwitchWin (w))
	    switchAddWindowToList (s, w);
    }

    qsort (ss->windows, ss->nWindows, sizeof (CompWindow *), compareWindows);

    if (ss->nWindows == 2)
    {
	switchAddWindowToList (s, ss->windows[0]);
	switchAddWindowToList (s, ss->windows[1]);
    }

    if (count > 1)
    {
	count -= (count + 1) & 1;
	if (count < 3)
	    count = 3;
    }

    ss->pos  = ((count >> 1) - ss->nWindows) * WIDTH;
    ss->move = 0;

    ss->selectedWindow = ss->windows[0]->id;

    if (ss->popupWindow)
	XMoveResizeWindow (s->display->display, ss->popupWindow,
			   s->width  / 2 - WINDOW_WIDTH (count) / 2,
			   s->height / 2 - WINDOW_HEIGHT / 2,
			   WINDOW_WIDTH (count),
			   WINDOW_HEIGHT);
}

static void
switchToWindow (CompScreen *s,
		Bool	   toNext)
{
    CompWindow *next = NULL;
    CompWindow *prev = NULL;
    CompWindow *w;

    SWITCH_SCREEN (s);

    if (!ss->grabIndex)
	return;

    for (w = s->windows; w; w = w->next)
    {
	if (w->id == ss->selectedWindow)
	    continue;

	if (isSwitchWin (w))
	{
	    if (w->activeNum < ss->lastActiveNum)
	    {
		if (next)
		{
		    if (toNext)
		    {
			if (w->activeNum > next->activeNum)
			    next = w;
		    }
		    else
		    {
			if (w->activeNum < next->activeNum)
			    next = w;
		    }
		}
		else
		    next = w;
	    }
	    else if (w->activeNum > ss->lastActiveNum)
	    {
		if (prev)
		{
		    if (toNext)
		    {
			if (w->activeNum > prev->activeNum)
			    prev = w;
		    }
		    else
		    {
			if (w->activeNum < prev->activeNum)
			    prev = w;
		    }
		}
		else
		    prev = w;
	    }
	}
    }

    if (toNext)
    {
	if (next)
	    w = next;
	else
	    w = prev;
    }
    else
    {
	if (prev)
	    w = prev;
	else
	    w = next;
    }

    if (w)
    {
	Window old = ss->selectedWindow;

	ss->lastActiveNum  = w->activeNum;
	ss->selectedWindow = w->id;

	if (!ss->zoomedWindow)
	    ss->zoomedWindow = ss->selectedWindow;

	if (old != w->id)
	{
	    if (toNext)
		ss->move -= WIDTH;
	    else
		ss->move += WIDTH;

	    ss->moreAdjust = 1;
	}

	if (ss->popupWindow)
	{
	    CompWindow *popup;

	    popup = findWindowAtScreen (s, ss->popupWindow);
	    if (popup)
		addWindowDamage (popup);

	    setSelectedWindowHint (s);
	}

	addWindowDamage (w);

	if (old)
	{
	    w = findWindowAtScreen (s, old);
	    if (w)
		addWindowDamage (w);
	}
    }
}

static int
switchCountWindows (CompScreen *s)
{
    CompWindow *w;
    int	       count = 0;

    for (w = s->windows; w && count < 5; w = w->next)
	if (isSwitchWin (w))
	    count++;

    if (count == 5 && s->width <= WINDOW_WIDTH (5))
	count = 3;

    return count;
}

static Visual *
findArgbVisual (Display *dpy, int scr)
{
    XVisualInfo		*xvi;
    XVisualInfo		template;
    int			nvi;
    int			i;
    XRenderPictFormat	*format;
    Visual		*visual;

    template.screen = scr;
    template.depth  = 32;
    template.class  = TrueColor;

    xvi = XGetVisualInfo (dpy,
			  VisualScreenMask |
			  VisualDepthMask  |
			  VisualClassMask,
			  &template,
			  &nvi);
    if (!xvi)
	return 0;

    visual = 0;
    for (i = 0; i < nvi; i++)
    {
	format = XRenderFindVisualFormat (dpy, xvi[i].visual);
	if (format->type == PictTypeDirect && format->direct.alphaMask)
	{
	    visual = xvi[i].visual;
	    break;
	}
    }

    XFree (xvi);

    return visual;
}

static void
switchInitiate (CompScreen *s,
		Bool	   allWindows)
{
    int count;

    SWITCH_SCREEN (s);

    if (otherScreenGrabExist (s, "switcher", "scale", "cube", 0))
	return;

    ss->allWindows = allWindows;

    count = switchCountWindows (s);
    if (count < 1)
	return;

    if (!ss->popupWindow)
    {
	Display		     *dpy = s->display->display;
	XSizeHints	     xsh;
	XWMHints	     xwmh;
	Atom		     state[4];
	int		     nState = 0;
	XSetWindowAttributes attr;
	Visual		     *visual;
	Atom		     mwmHintsAtom;
	MwmHints	     mwmHints;

	visual = findArgbVisual (dpy, s->screenNum);
	if (!visual)
	    return;

	if (count > 1)
	{
	    count -= (count + 1) & 1;
	    if (count < 3)
		count = 3;
	}

	xsh.flags  = PSize | PPosition;
	xsh.width  = WINDOW_WIDTH (count);
	xsh.height = WINDOW_HEIGHT;

	xwmh.flags = InputHint;
	xwmh.input = 0;

	attr.background_pixel = 0;
	attr.border_pixel     = 0;
	attr.colormap	      = XCreateColormap (dpy, s->root, visual,
						 AllocNone);

	ss->popupWindow =
	    XCreateWindow (dpy, s->root,
			   s->width  / 2 - xsh.width / 2,
			   s->height / 2 - xsh.height / 2,
			   xsh.width, xsh.height, 0,
			   32, InputOutput, visual,
			   CWBackPixel | CWBorderPixel | CWColormap, &attr);

	XSetWMProperties (dpy, ss->popupWindow, NULL, NULL,
			  programArgv, programArgc,
			  &xsh, &xwmh, NULL);

	mwmHintsAtom = XInternAtom (dpy, "_MOTIF_WM_HINTS", 0);

	memset (&mwmHints, 0, sizeof (mwmHints));

	mwmHints.flags	     = MwmHintsDecorations;
	mwmHints.decorations = 0;

	XChangeProperty (dpy, ss->popupWindow, mwmHintsAtom, mwmHintsAtom,
			 8, PropModeReplace, (unsigned char *) &mwmHints,
			 sizeof (mwmHints));

	state[nState++] = s->display->winStateAboveAtom;
	state[nState++] = s->display->winStateStickyAtom;
	state[nState++] = s->display->winStateSkipTaskbarAtom;
	state[nState++] = s->display->winStateSkipPagerAtom;

	XChangeProperty (dpy, ss->popupWindow,
			 XInternAtom (dpy, "_NET_WM_STATE", 0),
			 XA_ATOM, 32, PropModeReplace,
			 (unsigned char *) state, nState);
    }

    if (!ss->grabIndex)
	ss->grabIndex = pushScreenGrab (s, s->invisibleCursor, "switcher");

    if (ss->grabIndex)
    {
	if (!ss->switching)
	{
	    ss->lastActiveNum = s->activeNum;

	    switchUpdateWindowList (s, count);

	    ss->sTranslate = ss->zoom;

	    if (ss->popupWindow)
	    {
		CompWindow *w;

		w = findWindowAtScreen (s, ss->popupWindow);
		if (w)
		{
		    w->hidden = FALSE;
		    showWindow (w);
		}
		else
		{
		    XMapWindow (s->display->display, ss->popupWindow);
		}
	    }

	    setSelectedWindowHint (s);
	}

	damageScreen (s);

	ss->switching  = TRUE;
	ss->moreAdjust = 1;
    }
}

static void
switchTerminate (CompScreen *s,
		 Bool	    select)
{
    SWITCH_SCREEN (s);

    if (ss->grabIndex)
    {
	CompWindow *w;

	if (ss->popupWindow)
	{
	    w = findWindowAtScreen (s, ss->popupWindow);
	    if (w)
	    {
		w->hidden = TRUE;
		hideWindow (w);
	    }
	    else
	    {
		XUnmapWindow (s->display->display, ss->popupWindow);
	    }
	}

	ss->switching = FALSE;

	if (select && ss->selectedWindow)
	{
	    w = findWindowAtScreen (s, ss->selectedWindow);
	    if (w)
		sendWindowActivationRequest (w->screen, w->id);
	}

	if (!ss->zooming)
	{
	    ss->selectedWindow = None;
	    ss->zoomedWindow   = None;

	    removeScreenGrab (s, ss->grabIndex, 0);
	    ss->grabIndex = 0;
	}
	else
	{
	    ss->moreAdjust = 1;
	}

	ss->lastActiveNum = 0;

	damageScreen (s);
    }
}

static void
switchWindowRemove (CompDisplay *d,
		    Window	id)
{
    CompWindow *w;

    w = findWindowAtDisplay (d, id);
    if (w)
    {
	int i;

	SWITCH_SCREEN (w->screen);

	for (i = 0; i < ss->nWindows; i++)
	{
	    if (ss->windows[i] == w)
	    {
		ss->lastActiveNum = w->screen->activeNum;

		switchUpdateWindowList (w->screen,
					switchCountWindows (w->screen));

		break;
	    }
	}
    }
}

static void
switchHandleEvent (CompDisplay *d,
		   XEvent      *event)
{
    CompScreen *s;

    SWITCH_DISPLAY (d);

    switch (event->type) {
    case KeyPress:
    case KeyRelease:
	s = findScreenAtDisplay (d, event->xkey.root);
	if (s)
	{
	    SWITCH_SCREEN (s);

	    if (!ss->switching)
	    {
		if (eventMatches (d, event,
				  &ss->opt[SWITCH_SCREEN_OPTION_INITIATE]) ||
		    eventMatches (d, event, &ss->prev_bind))
		    switchInitiate (s, FALSE);
		else if (eventMatches (d, event, &ss->init_all_bind) ||
			 eventMatches (d, event, &ss->prev_all_bind))
		    switchInitiate (s, TRUE);
	    }

	    if (eventMatches (d, event,
			      &ss->opt[SWITCH_SCREEN_OPTION_INITIATE]) ||
		eventMatches (d, event, &ss->init_all_bind))
		switchToWindow (s, TRUE);
	    else if (eventMatches (d, event, &ss->prev_bind) ||
		     eventMatches (d, event, &ss->prev_all_bind))
		switchToWindow (s, FALSE);
	    else if (event->type	 == KeyPress &&
		     event->xkey.keycode == s->escapeKeyCode)
		switchTerminate (s, FALSE);
	    else if (eventTerminates (d, event,
				      &ss->opt[SWITCH_SCREEN_OPTION_INITIATE]))
		switchTerminate (s, TRUE);
	}
    default:
	break;
    }

    UNWRAP (sd, d, handleEvent);
    (*d->handleEvent) (d, event);
    WRAP (sd, d, handleEvent, switchHandleEvent);

    switch (event->type) {
    case UnmapNotify:
	switchWindowRemove (d, event->xunmap.window);
	break;
    case DestroyNotify:
	switchWindowRemove (d, event->xdestroywindow.window);
    default:
	break;
    }
}

static int
adjustSwitchVelocity (CompScreen *s)
{
    float dx, adjust, amount;

    SWITCH_SCREEN (s);

    dx = ss->move;

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    ss->mVelocity = (amount * ss->mVelocity + adjust) / (amount + 1.0f);

    if (ss->zooming)
    {
	float dt, ds;

	if (ss->switching)
	    dt = ss->zoom - ss->translate;
	else
	    dt = 0.0f - ss->translate;

	adjust = dt * 0.15f;
	amount = fabs (dt) * 1.5f;
	if (amount < 0.2f)
	    amount = 0.2f;
	else if (amount > 2.0f)
	    amount = 2.0f;

	ss->tVelocity = (amount * ss->tVelocity + adjust) / (amount + 1.0f);

	if (ss->selectedWindow == ss->zoomedWindow)
	    ds = ss->zoom - ss->sTranslate;
	else
	    ds = 0.0f - ss->sTranslate;

	adjust = ds * 0.5f;
	amount = fabs (ds) * 5.0f;
	if (amount < 1.0f)
	    amount = 1.0f;
	else if (amount > 6.0f)
	    amount = 6.0f;

	ss->sVelocity = (amount * ss->sVelocity + adjust) / (amount + 1.0f);

	if (ss->selectedWindow == ss->zoomedWindow)
	{
	    if (fabs (dx) < 0.1f   && fabs (ss->mVelocity) < 0.2f   &&
		fabs (dt) < 0.002f && fabs (ss->tVelocity) < 0.002f &&
		fabs (ds) < 0.002f && fabs (ss->sVelocity) < 0.002f)
	    {
		ss->mVelocity = ss->tVelocity = ss->sVelocity = 0.0f;
		return 0;
	    }
	}
    }
    else
    {
	if (fabs (dx) < 0.1f  && fabs (ss->mVelocity) < 0.2f)
	{
	    ss->mVelocity = 0.0f;
	    return 0;
	}
    }

    return 1;
}

static void
switchPreparePaintScreen (CompScreen *s,
			  int	     msSinceLastPaint)
{
    SWITCH_SCREEN (s);

    if (ss->moreAdjust)
    {
	int   steps, m;
	float amount, chunk;

	amount = msSinceLastPaint * 0.05f * ss->speed;
	steps  = amount / (0.5f * ss->timestep);
	if (!steps) steps = 1;
	chunk  = amount / (float) steps;

	while (steps--)
	{
	    ss->moreAdjust = adjustSwitchVelocity (s);
	    if (!ss->moreAdjust)
	    {
		ss->pos += ss->move;
		ss->move = 0;

		if (ss->zooming)
		{
		    if (ss->switching)
		    {
			ss->translate  = ss->zoom;
			ss->sTranslate = ss->zoom;
		    }
		    else
		    {
			ss->translate  = 0.0f;
			ss->sTranslate = ss->zoom;

			if (ss->grabIndex)
			{
			    ss->selectedWindow = None;
			    ss->zoomedWindow   = None;

			    removeScreenGrab (s, ss->grabIndex, 0);
			    ss->grabIndex = 0;
			}
		    }
		}
		break;
	    }

	    m = ss->mVelocity * chunk;
	    if (!m)
	    {
		if (ss->mVelocity)
		    m = (ss->move > 0) ? 1 : -1;
	    }

	    ss->move -= m;
	    ss->pos  += m;
	    if (ss->pos < -ss->nWindows * WIDTH)
		ss->pos += ss->nWindows * WIDTH;
	    else if (ss->pos > 0)
		ss->pos -= ss->nWindows * WIDTH;

	    ss->translate  += ss->tVelocity * chunk;
	    ss->sTranslate += ss->sVelocity * chunk;

	    if (ss->selectedWindow != ss->zoomedWindow)
	    {
		if (ss->sTranslate < 0.01f)
		    ss->zoomedWindow = ss->selectedWindow;
	    }
	}
    }

    UNWRAP (ss, s, preparePaintScreen);
    (*s->preparePaintScreen) (s, msSinceLastPaint);
    WRAP (ss, s, preparePaintScreen, switchPreparePaintScreen);
}

static Bool
switchPaintScreen (CompScreen		   *s,
		   const ScreenPaintAttrib *sAttrib,
		   Region		   region,
		   unsigned int		   mask)
{
    Bool status;

    SWITCH_SCREEN (s);

    if (ss->grabIndex)
    {
	ScreenPaintAttrib sa = *sAttrib;
	CompWindow	  *zoomed;
	CompWindow	  *switcher;
	Window		  zoomedAbove = None;
	Bool		  saveDestroyed = FALSE;

	if (ss->zooming)
	{
	    mask &= ~PAINT_SCREEN_REGION_MASK;
	    mask |= PAINT_SCREEN_TRANSFORMED_MASK;

	    sa.zCamera -= ss->translate;
	}

	switcher = findWindowAtScreen (s, ss->popupWindow);
	if (switcher)
	{
	    saveDestroyed = switcher->destroyed;
	    switcher->destroyed = TRUE;
	}

	if (ss->bringToFront)
	{
	    zoomed = findWindowAtScreen (s, ss->zoomedWindow);
	    if (zoomed)
	    {
		CompWindow *w;

		for (w = zoomed->prev; w && w->id <= 1; w = w->prev);
		zoomedAbove = (w) ? w->id : None;

		unhookWindowFromScreen (s, zoomed);
		insertWindowIntoScreen (s, zoomed, s->reverseWindows->id);
	    }
	}
	else
	{
	    zoomed = NULL;
	}

	UNWRAP (ss, s, paintScreen);
	status = (*s->paintScreen) (s, &sa, region, mask);
	WRAP (ss, s, paintScreen, switchPaintScreen);

	if (zoomed)
	{
	    unhookWindowFromScreen (s, zoomed);
	    insertWindowIntoScreen (s, zoomed, zoomedAbove);
	}

	if (switcher)
	{
	    switcher->destroyed = saveDestroyed;

	    glPushMatrix ();
	    glTranslatef (-0.5f, -0.5f, -DEFAULT_Z_CAMERA);
	    glScalef (1.0f / s->width, -1.0f / s->height, 1.0f);
	    glTranslatef (0.0f, -s->height, 0.0f);

	    if (!switcher->destroyed			 &&
		switcher->attrib.map_state == IsViewable &&
		switcher->damaged)
	    {
		(*s->paintWindow) (switcher, &switcher->paint, region, 0);
	    }

	    glPopMatrix ();
	}
    }
    else
    {
	UNWRAP (ss, s, paintScreen);
	status = (*s->paintScreen) (s, sAttrib, region, mask);
	WRAP (ss, s, paintScreen, switchPaintScreen);
    }

    return status;
}

static void
switchDonePaintScreen (CompScreen *s)
{
    SWITCH_SCREEN (s);

    if (ss->grabIndex && ss->moreAdjust)
    {
	if (ss->zooming)
	{
	    damageScreen (s);
	}
	else
	{
	    CompWindow *w;

	    w = findWindowAtScreen (s, ss->popupWindow);
	    if (w)
		addWindowDamage (w);
	}
    }

    UNWRAP (ss, s, donePaintScreen);
    (*s->donePaintScreen) (s);
    WRAP (ss, s, donePaintScreen, switchDonePaintScreen);
}

static void
switchMoveWindow (CompWindow *w,
		  int        dx,
		  int        dy)
{
    w->attrib.x += dx;
    w->attrib.y += dy;

    XOffsetRegion (w->region, dx, dy);
}

static void
switchPaintThumb (CompWindow		  *w,
		  const WindowPaintAttrib *attrib,
		  unsigned int		  mask,
		  int			  x,
		  int			  y,
		  int			  x1,
		  int			  x2)
{
    DrawWindowGeometryProc oldDrawWindowGeometry;
    WindowPaintAttrib	   sAttrib = *attrib;
    int			   dx, dy;
    int			   wx, wy;
    float		   width, height;
    REGION		   reg;
    CompMatrix		   matrix;
    CompIcon		   *icon = NULL;

    /* Wrap drawWindowGeometry to make sure the general
       drawWindowGeometry function is used */
    oldDrawWindowGeometry = w->screen->drawWindowGeometry;
    w->screen->drawWindowGeometry = drawWindowGeometry;

    mask |= PAINT_WINDOW_TRANSFORMED_MASK;

    reg.rects    = &reg.extents;
    reg.numRects = 1;

    reg.extents.y1 = MINSHORT;
    reg.extents.y2 = MAXSHORT;

    if (w->mapNum)
    {
	if (!w->texture.pixmap)
	    bindWindow (w);
    }

    if (w->mapNum)
    {
	SWITCH_SCREEN (w->screen);

	width  = WIDTH  - (SPACE << 1);
	height = HEIGHT - (SPACE << 1);

	if (w->width > width)
	    sAttrib.xScale = width / w->width;
	else
	    sAttrib.xScale = 1.0f;

	if (w->height > height)
	    sAttrib.yScale = height / w->height;
	else
	    sAttrib.yScale = 1.0f;

	if (sAttrib.xScale < sAttrib.yScale)
	    sAttrib.yScale = sAttrib.xScale;
	else
	    sAttrib.xScale = sAttrib.yScale;

	width  = w->width  * sAttrib.xScale;
	height = w->height * sAttrib.yScale;

	wx = x + SPACE + ((WIDTH  - (SPACE << 1)) - width)  / 2;
	wy = y + SPACE + ((HEIGHT - (SPACE << 1)) - height) / 2;

	dx = wx - w->attrib.x;
	dy = wy - w->attrib.y;

	switchMoveWindow (w, dx, dy);

	matrix = w->texture.matrix;
	matrix.x0 -= (w->attrib.x * w->matrix.xx);
	matrix.y0 -= (w->attrib.y * w->matrix.yy);

	if (w->alpha || sAttrib.opacity != OPAQUE)
	    mask |= PAINT_WINDOW_TRANSLUCENT_MASK;
	else if (sAttrib.opacity == OPAQUE)
	    mask &= ~PAINT_WINDOW_TRANSLUCENT_MASK;

	reg.extents.x1 = wx + (x1 - wx) / sAttrib.xScale;
	reg.extents.x2 = wx + (x2 - wx) / sAttrib.xScale;

	w->vCount = 0;
	addWindowGeometry (w, &matrix, 1, w->region, &reg);
	if (w->vCount)
	    (*w->screen->drawWindowTexture) (w, &w->texture, &sAttrib, mask);

	switchMoveWindow (w, -dx, -dy);

	if (ss->opt[SWITCH_SCREEN_OPTION_ICON].value.b)
	{
	    icon = getWindowIcon (w, ICON_SIZE, ICON_SIZE);
	    if (icon)
	    {
		sAttrib.xScale = sAttrib.yScale = 1.0f;

		wx = x + WIDTH  - icon->width  - SPACE;
		wy = y + HEIGHT - icon->height - SPACE;
	    }
	}
    }
    else
    {
	width  = WIDTH  - (WIDTH  >> 2);
	height = HEIGHT - (HEIGHT >> 2);

	icon = getWindowIcon (w, width, height);
	if (!icon)
	    icon = w->screen->defaultIcon;

	if (icon)
	{
	    int iw, ih;

	    iw = width  - SPACE;
	    ih = height - SPACE;

	    if (icon->width < (iw >> 1))
		sAttrib.xScale = (iw / icon->width);
	    else
		sAttrib.xScale = 1.0f;

	    if (icon->height < (ih >> 1))
		sAttrib.yScale = (ih / icon->height);
	    else
		sAttrib.yScale = 1.0f;

	    if (sAttrib.xScale < sAttrib.yScale)
		sAttrib.yScale = sAttrib.xScale;
	    else
		sAttrib.xScale = sAttrib.yScale;

	    width  = icon->width  * sAttrib.xScale;
	    height = icon->height * sAttrib.yScale;

	    wx = x + SPACE + ((WIDTH  - (SPACE << 1)) - width)  / 2;
	    wy = y + SPACE + ((HEIGHT - (SPACE << 1)) - height) / 2;
	}
    }

    if (icon && (icon->texture.name || iconToTexture (w->screen, icon)))
    {
	REGION iconReg;

	mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	dx = wx - w->attrib.x;
	dy = wy - w->attrib.y;

	switchMoveWindow (w, dx, dy);

	iconReg.rects    = &iconReg.extents;
	iconReg.numRects = 1;

	iconReg.extents.x1 = w->attrib.x;
	iconReg.extents.y1 = w->attrib.y;
	iconReg.extents.x2 = w->attrib.x + icon->width;
	iconReg.extents.y2 = w->attrib.y + icon->height;

	matrix = icon->texture.matrix;
	matrix.x0 -= (wx * icon->texture.matrix.xx);
	matrix.y0 -= (wy * icon->texture.matrix.yy);

	reg.extents.x1 = wx + (x1 - wx) / sAttrib.xScale;
	reg.extents.x2 = wx + (x2 - wx) / sAttrib.xScale;

	w->vCount = 0;
	addWindowGeometry (w, &matrix, 1, &iconReg, &reg);
	if (w->vCount)
	    (*w->screen->drawWindowTexture) (w,
					     &icon->texture, &sAttrib,
					     mask);

	switchMoveWindow (w, -dx, -dy);
    }

    w->screen->drawWindowGeometry = oldDrawWindowGeometry;
}

static Bool
switchPaintWindow (CompWindow		   *w,
		   const WindowPaintAttrib *attrib,
		   Region		   region,
		   unsigned int		   mask)
{
    CompScreen *s = w->screen;
    Bool       status;

    SWITCH_SCREEN (s);

    if (w->id == ss->popupWindow)
    {
	GLenum filter;
	int    x, y, x1, x2, cx, i;

	if (mask & PAINT_WINDOW_SOLID_MASK)
	    return FALSE;

	UNWRAP (ss, s, paintWindow);
	status = (*s->paintWindow) (w, attrib, region, mask);
	WRAP (ss, s, paintWindow, switchPaintWindow);

	if (!(mask & PAINT_WINDOW_TRANSFORMED_MASK) && region->numRects == 0)
	    return TRUE;

	x1 = w->attrib.x + SPACE;
	x2 = w->attrib.x + w->width - SPACE;

	x = x1 + ss->pos;
	y = w->attrib.y + SPACE;

	filter = s->display->textureFilter;

	if (ss->opt[SWITCH_SCREEN_OPTION_MIPMAP].value.b)
	    s->display->textureFilter = GL_LINEAR_MIPMAP_LINEAR;

	for (i = 0; i < ss->nWindows; i++)
	{
	    if (x + WIDTH > x1)
		switchPaintThumb (ss->windows[i], &w->lastPaint, mask,
				  x, y, x1, x2);

	    x += WIDTH;
	}

	for (i = 0; i < ss->nWindows; i++)
	{
	    if (x > x2)
		break;

	    switchPaintThumb (ss->windows[i], &w->lastPaint, mask,
			      x, y, x1, x2);

	    x += WIDTH;
	}

	s->display->textureFilter = filter;

	cx = w->attrib.x + (w->width >> 1);

	glEnable (GL_BLEND);
	glColor4us (0, 0, 0, w->lastPaint.opacity);
	glPushMatrix ();
	glTranslatef (cx, y, 0.0f);
	glVertexPointer (2, GL_FLOAT, 0, _boxVertices);
	glDrawArrays (GL_QUADS, 0, 16);
	glColor4usv (defaultColor);
	glDisable (GL_BLEND);
	glPopMatrix ();

    }
    else if (w->id == ss->selectedWindow)
    {
	glPushMatrix ();

	if (ss->bringToFront)
	{
	    if (ss->selectedWindow == ss->zoomedWindow)
		glTranslatef (0.0f, 0.0f, MIN (ss->translate, ss->sTranslate));
	}

	UNWRAP (ss, s, paintWindow);
	status = (*s->paintWindow) (w, attrib, region, mask);
	WRAP (ss, s, paintWindow, switchPaintWindow);

	glPopMatrix ();
    }
    else if (ss->switching)
    {
	WindowPaintAttrib sAttrib = *attrib;

	if (ss->saturation != COLOR)
	    sAttrib.saturation = (sAttrib.saturation * ss->saturation) >> 16;

	if (ss->brightness != 0xffff)
	    sAttrib.brightness = (sAttrib.brightness * ss->brightness) >> 16;

	if ((ss->wMask & w->type) && ss->opacity != OPAQUE)
	    sAttrib.opacity = (sAttrib.opacity * ss->opacity) >> 16;

	if (ss->bringToFront && w->id == ss->zoomedWindow)
	{
	    glPushMatrix ();
	    glTranslatef (0.0f, 0.0f, MIN (ss->translate, ss->sTranslate));

	    UNWRAP (ss, s, paintWindow);
	    status = (*s->paintWindow) (w, &sAttrib, region, mask);
	    WRAP (ss, s, paintWindow, switchPaintWindow);

	    glPopMatrix ();
	}
	else
	{
	    UNWRAP (ss, s, paintWindow);
	    status = (*s->paintWindow) (w, &sAttrib, region, mask);
	    WRAP (ss, s, paintWindow, switchPaintWindow);
	}
    }
    else
    {
	UNWRAP (ss, s, paintWindow);
	status = (*s->paintWindow) (w, attrib, region, mask);
	WRAP (ss, s, paintWindow, switchPaintWindow);
    }

    return status;
}

static Bool
switchDamageWindowRect (CompWindow *w,
			Bool	   initial,
			BoxPtr     rect)
{
    Bool status;

    SWITCH_SCREEN (w->screen);

    if (ss->grabIndex && ss->switching)
    {
	if (initial)
	{
	    if (isSwitchWin (w))
	    {
		ss->lastActiveNum = w->screen->activeNum;

		switchUpdateWindowList (w->screen,
					switchCountWindows (w->screen));
	    }
	    else if (w->id == ss->popupWindow)
	    {
		updateWindowAttributes (w, TRUE);
	    }
	}
	else if (!ss->moreAdjust)
	{
	    if (isSwitchWin (w))
	    {
		CompWindow *popup;

		popup = findWindowAtScreen (w->screen, ss->popupWindow);
		if (popup)
		    addWindowDamage (popup);

	    }
	}
    }

    UNWRAP (ss, w->screen, damageWindowRect);
    status = (*w->screen->damageWindowRect) (w, initial, rect);
    WRAP (ss, w->screen, damageWindowRect, switchDamageWindowRect);

    return status;
}

static Bool
switchInitDisplay (CompPlugin  *p,
		   CompDisplay *d)
{
    SwitchDisplay *sd;

    sd = malloc (sizeof (SwitchDisplay));
    if (!sd)
	return FALSE;

    sd->screenPrivateIndex = allocateScreenPrivateIndex (d);
    if (sd->screenPrivateIndex < 0)
    {
	free (sd);
	return FALSE;
    }

    sd->selectWinAtom = XInternAtom (d->display, SELECT_WIN_PROP, 0);

    WRAP (sd, d, handleEvent, switchHandleEvent);

    d->privates[displayPrivateIndex].ptr = sd;

    return TRUE;
}

static void
switchFiniDisplay (CompPlugin  *p,
		   CompDisplay *d)
{
    SWITCH_DISPLAY (d);

    freeScreenPrivateIndex (d, sd->screenPrivateIndex);

    UNWRAP (sd, d, handleEvent);

    free (sd);
}

static Bool
switchInitScreen (CompPlugin *p,
		  CompScreen *s)
{
    SwitchScreen *ss;

    SWITCH_DISPLAY (s->display);

    ss = malloc (sizeof (SwitchScreen));
    if (!ss)
	return FALSE;

    ss->popupWindow = None;

    ss->selectedWindow = None;
    ss->zoomedWindow   = None;

    ss->lastActiveNum  = 0;

    ss->windows     = 0;
    ss->nWindows    = 0;
    ss->windowsSize = 0;

    ss->pos = ss->move = 0;

    ss->switching = FALSE;

    ss->grabIndex = 0;

    ss->speed    = SWITCH_SPEED_DEFAULT;
    ss->timestep = SWITCH_TIMESTEP_DEFAULT;
    ss->zoom     = SWITCH_ZOOM_DEFAULT / 30.0f;

    ss->zooming  = (SWITCH_ZOOM_DEFAULT > 0.05f) ? TRUE : FALSE;

    ss->moreAdjust = 0;

    ss->mVelocity = 0.0f;
    ss->tVelocity = 0.0f;
    ss->sVelocity = 0.0f;

    ss->translate  = 0.0f;
    ss->sTranslate = 0.0f;

    ss->saturation = (COLOR  * SWITCH_SATURATION_DEFAULT) / 100;
    ss->brightness = (0xffff * SWITCH_BRIGHTNESS_DEFAULT) / 100;
    ss->opacity    = (OPAQUE * SWITCH_OPACITY_DEFAULT)    / 100;

    ss->bringToFront = SWITCH_BRINGTOFRONT_DEFAULT;
    ss->allWindows   = FALSE;

    switchScreenInitOptions (ss, s->display->display);

    addScreenBinding (s, &ss->opt[SWITCH_SCREEN_OPTION_INITIATE].value.bind);
    addScreenBinding (s, &ss->init_all_bind.value.bind);
    addScreenBinding (s, &ss->prev_bind.value.bind);
    addScreenBinding (s, &ss->prev_all_bind.value.bind);

    WRAP (ss, s, preparePaintScreen, switchPreparePaintScreen);
    WRAP (ss, s, donePaintScreen, switchDonePaintScreen);
    WRAP (ss, s, paintScreen, switchPaintScreen);
    WRAP (ss, s, paintWindow, switchPaintWindow);
    WRAP (ss, s, damageWindowRect, switchDamageWindowRect);

    s->privates[sd->screenPrivateIndex].ptr = ss;

    return TRUE;
}

static void
switchFiniScreen (CompPlugin *p,
		  CompScreen *s)
{
    SWITCH_SCREEN (s);

    UNWRAP (ss, s, preparePaintScreen);
    UNWRAP (ss, s, donePaintScreen);
    UNWRAP (ss, s, paintScreen);
    UNWRAP (ss, s, paintWindow);
    UNWRAP (ss, s, damageWindowRect);

    if (ss->windowsSize)
	free (ss->windows);

    free (ss);
}

static Bool
switchInit (CompPlugin *p)
{
    displayPrivateIndex = allocateDisplayPrivateIndex ();
    if (displayPrivateIndex < 0)
	return FALSE;

    return TRUE;
}

static void
switchFini (CompPlugin *p)
{
    if (displayPrivateIndex >= 0)
	freeDisplayPrivateIndex (displayPrivateIndex);
}

CompPluginVTable switchVTable = {
    "switcher",
    "Application Switcher",
    "Application Switcher",
    switchInit,
    switchFini,
    switchInitDisplay,
    switchFiniDisplay,
    switchInitScreen,
    switchFiniScreen,
    0, /* InitWindow */
    0, /* FiniWindow */
    0, /* GetDisplayOptions */
    0, /* SetDisplayOption */
    switchGetScreenOptions,
    switchSetScreenOption,
    NULL,
    0
};

CompPluginVTable *
getCompPluginInfo (void)
{
    return &switchVTable;
}
