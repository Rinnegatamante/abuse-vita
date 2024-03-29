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

#if defined __SWITCH__
#include <SDL_thread.h>
#include "setup.h"
#endif

#include <vitasdk.h>

#include "common.h"

#include "image.h"
#include "palette.h"
#include "video.h"
#include "event.h"
#include "timing.h"
#include "sprite.h"
#include "game.h"
#include "setup.h"

extern int get_key_binding(char const *dir, int i);
extern int mouse_xscale, mouse_yscale;
short mouse_buttons[5] = { 0, 0, 0, 0, 0 };

#if defined __SWITCH__
SDL_Joystick* joy;
SDL_Thread* event_thread;
bool event_thread_running = true;

extern flags_struct flags;

enum class Switch_Joy
{
  KEY_LSTICK_LEFT=SDL_CONTROLLER_BUTTON_MAX+1, 
  KEY_LSTICK_UP, KEY_LSTICK_RIGHT, KEY_LSTICK_DOWN,
  KEY_RSTICK_LEFT, KEY_RSTICK_UP, KEY_RSTICK_RIGHT, KEY_RSTICK_DOWN,
  KEY_SL_LEFT, KEY_SR_LEFT, KEY_SL_RIGHT, KEY_SR_RIGHT
};

#endif
SDL_GameController *controller;

extern SDL_Window *window;
extern flags_struct flags;

// Pre-declarations
void controller_to_mouse( Event &ev, SDL_Event *sdl_event, bool=true );
void controller_to_buttons( Event &ev, SDL_Event *sdl_event);

void EventHandler::SysInit()
{
	SDL_SetHint( SDL_HINT_TOUCH_MOUSE_EVENTS, "0" );
    // enable game controller
    // still needs to be detected to be used though
    controller_enabled = true;

    // Ignore activate events (still needed in SDL2?)
    //SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
}

void EventHandler::SysWarpMouse(ivec2 pos)
{
    //SDL_WarpMouseInWindow(window, pos.x, pos.y);
}

//
// IsPending()
// Are there any events in the queue?
//
int EventHandler::IsPending()
{
    if (!m_pending && SDL_PollEvent(NULL))
        m_pending = 1;

    return m_pending;
}

//
// Get and handle waiting events
//
void EventHandler::SysEvent(Event &ev)
{
    // No more events
    m_pending = 0;

    // NOTE : that the mouse status should be known
    // even if another event has occurred.
    ev.mouse_move.x = m_pos.x;
    ev.mouse_move.y = m_pos.y;
    ev.mouse_button = -1;
    ev.key = -1;
    //ev.mouse_button = m_button;

    // Gather next event
    SDL_Event sdlev;
    if (!SDL_PollEvent(&sdlev))
        return; // This should not happen
	
	bool circle = (the_game->state == RUN_STATE) && controller_enabled;
	controller_to_mouse(ev, &sdlev, circle);
	
	m_pos = ivec2(ev.mouse_move.x, ev.mouse_move.y);
    m_button = ev.mouse_button;
	
    // Sort out other kinds of events
    switch(sdlev.type)
    {
    case SDL_QUIT:
        exit(0);
        break;
    case SDL_MOUSEBUTTONUP:
        switch(sdlev.button.button)
        {
        case 4:        // Mouse wheel goes up...
            ev.key = get_key_binding("b4", 0);
            ev.type = EV_KEYRELEASE;
            break;
        case 5:        // Mouse wheel goes down...
            ev.key = get_key_binding("b3", 0);
            ev.type = EV_KEYRELEASE;
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        switch(sdlev.button.button)
        {
        case 4:        // Mouse wheel goes up...
            ev.key = get_key_binding("b4", 0);
            ev.type = EV_KEY;
            break;
        case 5:        // Mouse wheel goes down...
            ev.key = get_key_binding("b3", 0);
            ev.type = EV_KEY;
            break;
        }
        break;

    case SDL_FINGERDOWN:
    case SDL_FINGERUP:
    {
      if(sdlev.type == SDL_FINGERDOWN)
      {
		ev.mouse_move.x = sdlev.tfinger.x * main_screen->Size().x,
		ev.mouse_move.y = sdlev.tfinger.y * main_screen->Size().y;
		ev.mouse_button = LEFT_BUTTON;
		ev.type = EV_MOUSE_BUTTON;
		m_pos = ivec2(ev.mouse_move.x, ev.mouse_move.y);
	  } else {
		ev.mouse_button = -1;
	  }
      break;
    }
    break;
	case SDL_CONTROLLERAXISMOTION:
		controller_to_buttons(ev, &sdlev);
		break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
	{
	   /* if( sdlev.type == SDL_JOYBUTTONDOWN )
	    {
		ev.type = EV_KEY;
	    }
	    else
	    {
		ev.type = EV_KEYRELEASE;
	    }

	    switch (sdlev.jbutton.button) 
	    {
			  case 2: // X
				ev.key = get_key_binding("b1", 0);
				break;
			  case 3: // Square
				ev.key = get_key_binding("b3", 0);
				break;
			  case 1: // Circle
				ev.key = get_key_binding("b2", 0);
				break;
			  case 0: // Triangle
				ev.key = get_key_binding("b4", 0);
				break;
			  case 4: // L
				ev.key = get_key_binding("b3", 0);
				break;
			  case 5: // R
				ev.key = get_key_binding("b4", 0);
				break;
              case 7: // LEFT
                ev.key = get_key_binding("left", 0);
              	break;
              case 8: // UP
                ev.key = get_key_binding("up", 0);
              	break;
              case 6: // DOWN
                ev.key = get_key_binding("down", 0);
              	break;
              case 9: // RIGHT
                ev.key = get_key_binding("right", 0);
                break;
			  case 10: // SELECT
			    ev.key = JK_ESC;
	           break;
			  case 11: // START
				ev.key = JK_ENTER;
				break;
	      default:
		printf("Uknown joy key %d\n", sdlev.jbutton.button);
	    };*/
	}
	break;
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
	{
		    
	    if( sdlev.type == SDL_CONTROLLERBUTTONDOWN )
	    {
		ev.type = EV_KEY;
	    }
	    else
	    {
		ev.type = EV_KEYRELEASE;
	    }

	    // Default to EV_SPURIOUS
	    ev.key = EV_SPURIOUS;
		    
	    switch (sdlev.cbutton.button) 
	    {
		case SDL_CONTROLLER_BUTTON_A:
	        ev.key = get_key_binding("b1", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_B:
	        ev.key = get_key_binding("b2", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_X:
            ev.key = get_key_binding("b3", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_Y:
            ev.key = get_key_binding("b4", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		    ev.key = get_key_binding("left", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		    ev.key = get_key_binding("right", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
		    ev.key = get_key_binding("up", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		    ev.key = get_key_binding("down", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_BACK:
	        ev.key = JK_ESC;
	        break;
	    case SDL_CONTROLLER_BUTTON_START:
	        ev.key = JK_ENTER;
		    break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
	        ev.key = get_key_binding("b3", 0);
		    break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
	        ev.key = get_key_binding("b2", 0);
		    break;
		default:
		  printf("Unknown key %d\n", sdlev.cbutton.button);
	    }
	    break;
	}
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        // Default to EV_SPURIOUS
        ev.key = EV_SPURIOUS;
        if(sdlev.type == SDL_KEYDOWN)
        {
            ev.type = EV_KEY;
        }
        else
        {
            ev.type = EV_KEYRELEASE;
        }
        switch(sdlev.key.keysym.sym)
        {
        case SDLK_DOWN:         ev.key = JK_DOWN; break;
        case SDLK_UP:           ev.key = JK_UP; break;
        case SDLK_LEFT:         ev.key = JK_LEFT; break;
        case SDLK_RIGHT:        ev.key = JK_RIGHT; break;
        case SDLK_LCTRL:        ev.key = JK_CTRL_L; break;
        case SDLK_RCTRL:        ev.key = JK_CTRL_R; break;
        case SDLK_LALT:         ev.key = JK_ALT_L; break;
        case SDLK_RALT:         ev.key = JK_ALT_R; break;
        case SDLK_LSHIFT:       ev.key = JK_SHIFT_L; break;
        case SDLK_RSHIFT:       ev.key = JK_SHIFT_R; break;
        case SDLK_NUMLOCKCLEAR: ev.key = JK_NUM_LOCK; break;
        case SDLK_HOME:         ev.key = JK_HOME; break;
        case SDLK_END:          ev.key = JK_END; break;
        case SDLK_BACKSPACE:    ev.key = JK_BACKSPACE; break;
        case SDLK_TAB:          ev.key = JK_TAB; break;
        case SDLK_RETURN:       ev.key = JK_ENTER; break;
        case SDLK_SPACE:        ev.key = JK_SPACE; break;
        case SDLK_CAPSLOCK:     ev.key = JK_CAPS; break;
        case SDLK_ESCAPE:       ev.key = JK_ESC; break;
        case SDLK_F1:           ev.key = JK_F1; break;
        case SDLK_F2:           ev.key = JK_F2; break;
        case SDLK_F3:           ev.key = JK_F3; break;
        case SDLK_F4:           ev.key = JK_F4; break;
        case SDLK_F5:           ev.key = JK_F5; break;
        case SDLK_F6:           ev.key = JK_F6; break;
        case SDLK_F7:           ev.key = JK_F7; break;
        case SDLK_F8:           ev.key = JK_F8; break;
        case SDLK_F9:           ev.key = JK_F9; break;
        case SDLK_F10:          ev.key = JK_F10; break;
        case SDLK_INSERT:       ev.key = JK_INSERT; break;
        case SDLK_KP_0:         ev.key = JK_INSERT; break;
        case SDLK_PAGEUP:       ev.key = JK_PAGEUP; break;
        case SDLK_PAGEDOWN:     ev.key = JK_PAGEDOWN; break;
        case SDLK_KP_8:         ev.key = JK_UP; break;
        case SDLK_KP_2:         ev.key = JK_DOWN; break;
        case SDLK_KP_4:         ev.key = JK_LEFT; break;
        case SDLK_KP_6:         ev.key = JK_RIGHT; break;
        case SDLK_F11:
            // Only handle key down
            if(ev.type == EV_KEY)
            {
		// Toggle fullscreen
		if(flags.fullscreen)
		{
		    flags.fullscreen = 0;
		    //SDL_SetWindowFullscreen(window, 0);
		    //SDL_SetWindowSize(window, flags.xres, flags.yres);
		}
		else
		{
		    flags.fullscreen = 1;
		    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
            }
            ev.key = EV_SPURIOUS;
            break;
        case SDLK_F12:
            // Only handle key down
            if(ev.type == EV_KEY)
            {
		// Toggle grab mouse
		//if( SDL_GetWindowGrab(window) == SDL_TRUE )
		//{
		    the_game->show_help( "Grab Mouse: OFF\n" );
		    //SDL_SetWindowGrab(window, SDL_FALSE);
		/*}
		else
		{
		    the_game->show_help( "Grab Mouse: ON\n" );
		    SDL_SetWindowGrab(window, SDL_TRUE);
		}*/
            }
            ev.key = EV_SPURIOUS;
            break;
        case SDLK_PRINTSCREEN:    // print-screen key
            // Only handle key down
            if(ev.type == EV_KEY)
            {
                // Grab a screenshot
		// need to figure this out for SDL2
                //SDL_SaveBMP(SDL_GetVideoSurface(), "screenshot.bmp");
                //the_game->show_help("Screenshot saved to: screenshot.bmp.\n");
            }
            ev.key = EV_SPURIOUS;
            break;
        default:
            ev.key = (int)sdlev.key.keysym.sym;
            // Need to handle the case of shift being pressed
            // There has to be a better way
            if((sdlev.key.keysym.mod & KMOD_SHIFT) != 0)
            {
                if(sdlev.key.keysym.sym >= SDLK_a &&
                    sdlev.key.keysym.sym <= SDLK_z)
                {
                    ev.key -= 32;
                }
                else if(sdlev.key.keysym.sym >= SDLK_1 &&
                         sdlev.key.keysym.sym <= SDLK_5)
                {
                    ev.key -= 16;
                }
                else
                {
                    switch(sdlev.key.keysym.sym)
                    {
                    case SDLK_6:
                        ev.key = SDLK_CARET; break;
                    case SDLK_7:
                    case SDLK_9:
                    case SDLK_0:
                        ev.key -= 17; break;
                    case SDLK_8:
                        ev.key = SDLK_ASTERISK; break;
                    case SDLK_MINUS:
                        ev.key = SDLK_UNDERSCORE; break;
                    case SDLK_EQUALS:
                        ev.key = SDLK_PLUS; break;
                    case SDLK_COMMA:
                        ev.key = SDLK_LESS; break;
                    case SDLK_PERIOD:
                        ev.key = SDLK_GREATER; break;
                    case SDLK_SLASH:
                        ev.key = SDLK_QUESTION; break;
                    case SDLK_SEMICOLON:
                        ev.key = SDLK_COLON; break;
                    case SDLK_QUOTE:
                        ev.key = SDLK_QUOTEDBL; break;
                    default:
                        break;
                    }
                }
            }
            break;
        }
    }
}

// simulate dpad presses with left analog
bool x_last[2];
bool y_last[2];
void controller_to_buttons( Event &ev, SDL_Event *sdl_event)
{
	Sint16 axis;
	if (sdl_event->caxis.axis == 0) {
		axis = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)0); 
		
		SDL_Event sdlevent[2];
		bool x_state[2];
		x_state[0] = axis < -32767 / 2;
		if (x_state[0] != x_last[0]) {
			sdlevent[0].type = x_state[0] ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
			sdlevent[0].cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
			SDL_PushEvent(&sdlevent[0]);
			x_last[0] = x_state[0];
		}
		x_state[1] = axis > 32767 / 2;
		if (x_state[1] != x_last[1]) {
			sdlevent[1].type = x_state[1] ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
			sdlevent[1].cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
			SDL_PushEvent(&sdlevent[1]);
			x_last[1] = x_state[1];
		}
	} else if (sdl_event->caxis.axis == 1) {
		axis = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)1);
		
		SDL_Event sdlevent[2];
		bool y_state[2];
		y_state[0] = axis < -32767 / 2;
		if (y_state[0] != y_last[0]) {
			sdlevent[0].type = y_state[0] ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
			sdlevent[0].cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_UP;
			SDL_PushEvent(&sdlevent[0]);
			y_last[0] = y_state[0];
		}
		y_state[1] = axis > 32767 / 2;
		if (y_state[1] != y_last[1]) {
			sdlevent[1].type = y_state[1] ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
			sdlevent[1].cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
			SDL_PushEvent(&sdlevent[1]);
			y_last[1] = y_state[1];
		}
	}
}

void controller_to_mouse( Event &ev, SDL_Event *sdl_event, bool circle )
{
	Sint16 lr_axis, ud_axis;
    int x, y;
    
	if (sdl_event->caxis.axis == 2 || sdl_event->caxis.axis == 3)
    {
      lr_axis = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)2); 
      ud_axis = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)3); 
      
      lr_axis /= 1024;
      ud_axis /= -1024; // flip this axis

      // convert to mouse position
      float theta = 0.0;
      float cos_theta;
      float mag = sqrtf((lr_axis * lr_axis) + (ud_axis * ud_axis));
      float udotv = 1.0 * lr_axis;

      // give a little dead zone
      if(mag > 0.001 || !circle)
      {
          if (circle)
          {
            int radius;
            
            // calculate the aim angle in terms of a unit circle
            cos_theta = udotv / mag;
            theta = acosf(cos_theta);

            if(ud_axis < 0) theta *= -1.0f;

          // calculate the origin
            if(the_game->first_view)
            {
                x = the_game->first_view->m_focus->x - the_game->first_view->xoff();
                y = the_game->first_view->m_focus->y - the_game->first_view->yoff();

                y -= 20;
                radius = 75;
            }
            else
            {
                x = main_screen->Size().x / 2;
                y = main_screen->Size().y / 2;
                radius = 200;
            }
            x += (int)roundf(cosf(theta) * radius);
            y += (int)roundf(sinf(theta) * -1.0 * radius);
          }
          else
          {
            if(sdl_event->caxis.axis == 2)
            {
              x= ev.mouse_move.x + (sdl_event->caxis.value / 0x1500);
              y = ev.mouse_move.y;
            }
            else
            {
              x = ev.mouse_move.x;
              y= ev.mouse_move.y + (sdl_event->caxis.value / 0x1500);
            }
          }

          if( x > main_screen->Size().x - 1 )
          {
              x = main_screen->Size().x - 1;
          }
          if( y > main_screen->Size().y - 1 )
          {
              y = main_screen->Size().y - 1;
          }

          ev.mouse_move.x = x;
          ev.mouse_move.y = y;
          ev.type = EV_MOUSE_MOVE;

      }
    }

    ev.mouse_button=0;
}
