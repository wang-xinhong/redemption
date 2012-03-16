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
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Rect class : Copyright (C) Christophe Grosjean 2009

*/

#if !defined(__RECT_HPP__)
#define __RECT_HPP__

#include <algorithm>
#include <ostream>
#include "log.hpp"

struct Rect {
    int x;
    int y;
    int cx;
    int cy;

    struct RectIterator {
        virtual void callback(const Rect & rect) = 0;
    };

    int right() const {
        return this->x + this->cx;
    }

    int bottom() const {
        return this->y + this->cy;
    }

    Rect(int left = 0, int top = 0, int width = 0, int height = 0)
        : x(left), y(top), cx(width), cy(height)
    {
    }

    bool contains_pt(int x, int y) const {
        return    x  >= this->x
               && y  >= this->y
               && x < (this->x + this->cx)
               && y < (this->y + this->cy);
    }

    TODO(" contains should work when inner rect is empty except if outer is empty")
    bool contains(const Rect & inner) const {
        return (inner.x >= this->x
             && inner.y >= this->y
             && inner.x + inner.cx <= this->x + this->cx
             && inner.y + inner.cy <= this->y + this->cy);
    }

    TODO(" equal should work when inner and outer rect are both empty")
    bool equal(const Rect & other) const {
        return (other.x == this->x
             && other.y == this->y
             && other.x + other.cx == this->x + this->cx
             && other.y + other.cy == this->y + this->cy);
    }

    bool operator==(const Rect &other) const {
        return this->equal(other);
    }

    bool isempty() const {
        return this->cx <= 0 || this->cy <= 0;
    }

    int getCenteredX() const {
        return this->x + (this->cx / 2);
    }

    int getCenteredY() const {
        return this->y + (this->cy / 2);
    }

    Rect wh() const {
        return Rect(0, 0, this->cx, this->cy);
    }

    // compute a new rect containing old rect and given point
    Rect enlarge_to(int x, int y) const {
        if (this->isempty()){
            return Rect(x, y, 1, 1);
        }
        else {
            const int x0 = std::min(this->x, x);
            const int y0 = std::min(this->y, y);
            const int x1 = std::max(this->x + this->cx - 1, x);
            const int y1 = std::max(this->y + this->cy - 1, y);
            return Rect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
        }
    }

    Rect offset(int dx, int dy) const {
        return Rect(this->x + dx, this->y + dy, this->cx, this->cy);
    }

    Rect shrink(int margin) const {
        return Rect(this->x + margin, this->y + margin,
                    this->cx - margin * 2, this->cy - margin * 2);
    }

    Rect upper_side() const {
        return Rect(this->x, this->y, this->cx, 1);
    }

    Rect left_side() const {
        return Rect(this->x, this->y, 1, this->cy);
    }

    Rect lower_side() const {
        return Rect(this->x, this->y + this->cy - 1, this->cx, 1);
    }

    Rect right_side() const {
        return Rect(this->x + this->cx - 1, this->cy, 1, this->cy);
    }

    Rect intersect(const Rect & in) const
    {
        Rect res(0, 0, 0, 0);

        res.x = std::max(in.x, this->x);
        res.y = std::max(in.y, this->y);
        res.cx = std::min(in.x + in.cx, this->x + this->cx) - res.x;
        res.cy = std::min(in.y + in.cy, this->y + this->cy) - res.y;

        // Is it necessary to force empty rect to be canonical ?
        if (res.isempty()){
            Rect empty;
            res = empty;
        }
        return res;
    }

    Rect intersect(int w, int h) const
    {
        return Rect(this->x,
                    this->y,
                    std::min(w - this->x, this->cx),
                    std::min(h - this->y, this->cy));
    }

    // Ensemblist difference
    void difference(const Rect & a, RectIterator & it) const
    {
        const Rect & intersect = this->intersect(a);

        if (!intersect.isempty()) {
            if ( (intersect.y - this->y > 0) ) {
                it.callback(Rect(this->x, this->y, this->cx, intersect.y - this->y));
            }
            if ( ((intersect.x - this->x) > 0) ) {
                it.callback(Rect(this->x, intersect.y, intersect.x - this->x, intersect.cy));
            }
            if ( (((this->x + this->cx) - (intersect.x + intersect.cx)) > 0) ) {
                it.callback(Rect(intersect.x + intersect.cx, intersect.y, (this->x + this->cx) - (intersect.x + intersect.cx), intersect.cy));
            }
            if ( ((this->y + this->cy) - (intersect.y + intersect.cy)) > 0 ) {
                it.callback(Rect(this->x, intersect.y + intersect.cy, this->cx, (this->y + this->cy) - (intersect.y + intersect.cy)));
            }
        } else {
            it.callback(*this);
        }
    }

    /* adjust the bounds to fit in the bitmap */
    /* return false if there is nothing to draw else return true */
    bool check_bounds(int* x, int* y, int* cx, int* cy) const
    {
        if (*x >= this->cx) {
            return false;
        }
        if (*y >= this->cy) {
            return false;
        }
        if (*x < 0) {
            *cx += *x;
            *x = 0;
        }
        if (*y < 0) {
            *cy += *y;
            *y = 0;
        }
        if (*cx <= 0) {
            return false;
        }
        if (*cy <= 0) {
            return false;
        }
        if (*x + *cx > this->cx) {
            *cx = this->cx - *x;
        }
        if (*y + *cy > this->cy) {
            *cy = this->cy - *y;
        }
        return true;
    }

    /*****************************************************************************/
    /* returns boolean */
    bool rect_contained_by(int left, int top, int right, int bottom) const
    {
        return !( (left   <  this->x)
               || (top    <  this->y)
               || (right  > (this->x + this->cx))
               || (bottom > (this->y + this->cy)));
    }

    friend inline std::ostream & operator<<(std::ostream& os, const Rect &r) {
        return os << "(" << r.x << ", " << r.y << ", " << r.cx << ", " << r.cy << ")";
    }
};


// helper class used to compute differences between two rectangles
class DeltaRect {
    public:
    int dleft;
    int dtop;
    int dheight;
    int dwidth;

    DeltaRect(const Rect & r1, const Rect & r2) {
        this->dtop = r1.y - r2.y;
        this->dleft = r1.x - r2.x;
        this->dheight = r1.cy - r2.cy;
        this->dwidth = r1.cx - r2.cx;
    }

    bool fully_relative(){
        return (abs(this->dleft) < 128)
            && (abs(this->dtop) < 128)
            && (abs(this->dwidth) < 128)
            && (abs(this->dheight) < 128)
            ;
        }
};


#endif