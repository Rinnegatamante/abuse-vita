/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 2001 Anthony Kruize <trandor@labyrinth.net.au>
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 */

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif

#include <SDL.h>

#include "common.h"

#include "filter.h"
#include "video.h"
#include "image.h"
#include "setup.h"

#include <vitasdk.h>
#include <vita2d.h>

SDL_Surface *surface = NULL;
SDL_Surface *rgba_surface = NULL;
SDL_Texture *texture = NULL;
image *main_screen = NULL;
int win_xscale, win_yscale, mouse_xscale, mouse_yscale;
int xres, yres;

extern palette *lastl;
extern flags_struct flags;

static void update_window_part(SDL_Rect *rect);

//
// power_of_two()
// Get the nearest power of two
//
static int power_of_two(int input)
{
    int value;
    for(value = 1 ; value < input ; value <<= 1);
    return value;
}

vita2d_texture *tex_buffer = nullptr;
bool palette_set = false;
uint8_t *tex_ptr;

//
// set_mode()
// Set the video mode
//
void set_mode(int mode, int argc, char **argv)
{
	vita2d_init();
    tex_buffer = vita2d_create_empty_texture_format(xres, yres, SCE_GXM_TEXTURE_FORMAT_P8_ARGB);
	tex_ptr = (uint8_t*)vita2d_texture_get_datap(tex_buffer);

    // Create the screen image
    main_screen = new image(ivec2(xres, yres), NULL, 2);
    if(main_screen == NULL)
    {
        // Our screen image is no good, we have to bail.
        printf("Video : Unable to create screen image.\n");
        exit(1);
    }
    main_screen->clear();

    update_dirty(main_screen);
}

//
// close_graphics()
// Shutdown the video mode
//
void close_graphics()
{
    if(lastl)
        delete lastl;
    lastl = NULL;

    delete main_screen;
}

// put_part_image()
// Draw only dirty parts of the image
//
void put_part_image(image *im, int x, int y, int x1, int y1, int x2, int y2)
{
    int xe, ye;
    int ii, jj;
    int srcx, srcy, xstep, ystep;
    Uint8 *dpixel;
    Uint16 dinset;

    if(y > yres || x > xres)
        return;

    CHECK(x1 >= 0 && x2 >= x1 && y1 >= 0 && y2 >= y1);

    // Adjust if we are trying to draw off the screen
    if(x < 0)
    {
        x1 += -x;
        x = 0;
    }
	srcx = x1;
    if(x + (x2 - x1) >= xres)
        xe = xres - x + x1 - 1;
    else
        xe = x2;

    if(y < 0)
    {
        y1 += -y;
        y = 0;
    }
	srcy = y1;
    if(y + (y2 - y1) >= yres)
        ye = yres - y + y1 - 1;
    else
        ye = y2;

    if(x1 >= xe || y1 >= ye)
        return;

    // Scale the image onto the surface
    int w = xe - x1;
    int h = ye - y1;

    // Update surface part
    dpixel = ((Uint8 *)tex_ptr) + y * xres + x;
    for(ii=0 ; ii < h; ii++)
    {
        memcpy(dpixel, im->scan_line(srcy) + srcx , w);
        dpixel += xres;
        srcy ++;
    }

    // Now blit the surface
    update_window_part(NULL);
}

//
// load()
// Set the palette
//
void palette::load()
{
    if(lastl)
        delete lastl;
    lastl = copy();

    // Force to only 256 colours.
    // Shouldn't be needed, but best to be safe.
    if(ncolors > 256)
        ncolors = 256;

    uint32_t colors[ncolors];
    for(int ii = 0; ii < ncolors; ii++)
    {
        colors[ii] = blue(ii) | green(ii) << 8 | red(ii) << 16 | 0xFF << 24;
    }
	
	memcpy(vita2d_texture_get_palette(tex_buffer), colors, sizeof(uint32_t) * ncolors);

    // Now redraw the surface
    update_window_part(NULL);
    update_window_done();
}

//
// load_nice()
//
void palette::load_nice()
{
    load();
}

// ---- support functions ----

void update_window_done()
{
    vita2d_start_drawing();
	vita2d_draw_texture_scale(tex_buffer, 45, 0, 2.72, 2.72);
	vita2d_end_drawing();
	vita2d_wait_rendering_done();
	vita2d_swap_buffers();
}

static void update_window_part(SDL_Rect *rect)
{
    // no partial blit's in case of opengl
    // complete blit + scaling just before flip
    return;

#if 0
    SDL_BlitSurface(surface, rect, window, rect);

    // no window update needed until end of run
    if(flags.doublebuf)
        return;

    // update window part for single buffer
    if(rect == NULL)
        SDL_UpdateRect(window, 0, 0, 0, 0);
    else
        SDL_UpdateRect(window, rect->x, rect->y, rect->w, rect->h);
#endif
}
