/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni

*/

#if !defined(__INTERNAL_MOD_HPP__)
#define __INTERNAL_MOD_HPP__

#include "modcontext.hpp"
#include "../mod/internal/widget.hpp"
#include "client_mod.hpp"

struct internal_mod : public client_mod {
    public:
    widget_screen screen;
    int dragging;
    Rect dragging_rect;
    int draggingdx; // distance between mouse and top angle of dragged window
    int draggingdy; // distance between mouse and top angle of dragged window
    struct Widget* dragging_window;
    RDPBrush brush;

    internal_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front)
            : client_mod(keys, key_flags, keymap, front),
                screen(this,
                 this->get_client_info().width,
                 this->get_client_info().height,
                 this->get_client_info().bpp)
    {
        /* dragging info */
        this->dragging = 0;
        this->event = event;
        // dragging_rect is (0,0,0,0)
        this->draggingdx = 0; // distance between mouse and top angle of dragged window
        this->draggingdy = 0; // distance between mouse and top angle of dragged window
        this->dragging_window = 0;
    }

    void server_draw_dragging_rect(const Rect & r, const Rect & clip)
    {
        this->server_begin_update();

        RDPBrush brush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55");

        // draw rectangles by drawing each edge top/bottom/left/right
        // 0x66 = xor -> pat_blt( ... 0x5A ...
        // 0xAA = noop -> pat_blt( ... 0xFB ...
        // 0xCC = copy -> pat_blt( ... 0xF0 ...
        // 0x88 = and -> pat_blt( ...  0xC0 ...

        this->server_set_clip(clip);
        this->pat_blt(
            RDPPatBlt(Rect(r.x, r.y, r.cx, 5), 0x5A, BLACK, WHITE, this->brush));
        this->pat_blt(
            RDPPatBlt(Rect(r.x, r.y + (r.cy - 5), r.cx, 5), 0x5A, BLACK, WHITE, this->brush));
        this->pat_blt(
            RDPPatBlt(Rect(r.x, r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, this->brush));
        this->pat_blt(
            RDPPatBlt(Rect(r.x + (r.cx - 5), r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, this->brush));
        this->server_end_update();
    }


    virtual ~internal_mod()
    {
//        this->screen.delete_all_childs();
    }

    size_t nb_windows()
    {
        return this->screen.child_list.size();
    }


    virtual void front_resize() {
        this->screen.rect.cx = this->get_client_info().width;
        this->screen.rect.cy = this->get_client_info().height;
        this->screen.bpp     = this->get_client_info().bpp;
    }

    Widget * window(int i)
    {
        return this->screen.child_list[i];
    }

    widget_screen * get_screen_wdg(){
        return &(this->screen);
    }

    virtual void rdp_input_invalidate(const Rect & rect) = 0;
    virtual void rdp_input_mouse(int device_flags, int x, int y, const int key_flags, const int (& keys)[256]) = 0;
    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, const int key_flags, const int (& keys)[256], struct key_info* ki) = 0;

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual int draw_event()
    {
        return 0;
    }
};

#endif
