/*
 * Copyright © 2006 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "gtk-window-decorator.h"

cairo_surface_t *
create_native_surface_and_wrap (int        w,
                                int        h,
                                GtkWidget *parent_style_window)
{
    GdkWindow       *window;
    GdkVisual       *visual;
    cairo_surface_t *surface;
    Display         *display;
    Pixmap           pixmap;

    if (w <= 0 || h <= 0)
	abort ();

    display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    window = gtk_widget_get_window (parent_style_window);
    visual = gdk_window_get_visual (window);
    pixmap = XCreatePixmap (display, GDK_WINDOW_XID (window), w, h, gdk_visual_get_depth (visual));
    surface = cairo_xlib_surface_create (display, pixmap, GDK_VISUAL_XVISUAL (visual), w, h);

    return surface;
}

cairo_surface_t *
create_surface (int        w,
                int        h,
                GtkWidget *parent_style_window)
{
    GdkWindow       *window;
    cairo_surface_t *surface;

    if (w == 0 || h == 0)
	abort ();

    window = gtk_widget_get_window (parent_style_window);
    surface = gdk_window_create_similar_surface (window, CAIRO_CONTENT_COLOR_ALPHA, w, h);

    return surface;
}
