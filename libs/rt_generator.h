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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Dominique Lafages
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Front object (server), used to communicate with RDP client

*/

#ifndef _REDEMPTION_LIBS_RT_GENERATOR_H_
#define _REDEMPTION_LIBS_RT_GENERATOR_H_

#include "rt_constants.h"

struct RTGenerator {
    size_t current;
    uint8_t * data;
    size_t len;
};

extern "C" {
    inline RT_ERROR rt_m_generator_new(RTGenerator * self, const void * data, size_t len)
    {
        self->data = (uint8_t *)malloc(len);
        if (!self->data) { return RT_ERROR_MALLOC; }
        self->len = len;
        self->current = 0;
        memcpy(self->data, data, len);
        return RT_ERROR_OK;
    }
};

#endif

