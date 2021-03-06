/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    Unit test to RDP Orders coder/decoder
    Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOrderMultiDstBlt
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"

#include "test_orders.hpp"

BOOST_AUTO_TEST_CASE(TestMultiDstBlt)
{
    using namespace RDP;

    {
        BStream stream(1000);

        RDPOrderCommon state_common(0, Rect(0, 0, 0, 0));
        RDPMultiDstBlt state_multidstblt;
        RDPOrderCommon newcommon(MULTIDSTBLT, Rect(0, 0, 1024, 768));

        BStream deltaRectangles(1024);

        deltaRectangles.out_sint16_le(316);
        deltaRectangles.out_sint16_le(378);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);

        for (int i = 0; i < 19; i++) {
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
        }

        deltaRectangles.mark_end();
        deltaRectangles.rewind();

        RDPMultiDstBlt multidstblt(316, 378, 200, 200, 0x55, 20, deltaRectangles);


        multidstblt.emit(stream, newcommon, state_common, state_multidstblt);

        BOOST_CHECK_EQUAL((uint8_t)MULTIDSTBLT, newcommon.order);
        BOOST_CHECK_EQUAL(Rect(0, 0, 0, 0), newcommon.clip);

        uint8_t datas[] = {
            CHANGE | STANDARD,
            MULTIDSTBLT,
            0x7F,                   // header fields
            0x3C, 0x01, 0x7a, 0x01, // nLeftRect = 316, nTopRect = 378
            0xc8, 0x00, 0xc8, 0x00, // nWidth = 200, nHeight = 200
            0x55,                   // bRop
            0x14,                   // nDeltaEntries
            0x5c, 0x00,             // cbData
 /* 0000 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x3c, 0x81, 0x7a, 0x0a, 0x0a,  // ...........<.z..
 /* 0010 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0020 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0030 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0040 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,  // ................
 /* 0050 */ 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,                          // ............
        };
        check_datas(stream.p - stream.get_data(), stream.get_data(), sizeof(datas), datas, "MultiDstBlt 1");

        stream.mark_end(); stream.p = stream.get_data();

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)0x09, header.control);
        BOOST_CHECK_EQUAL((uint32_t)0x7F, header.fields);
        BOOST_CHECK_EQUAL((uint8_t)MULTIDSTBLT, common_cmd.order);
        BOOST_CHECK_EQUAL(Rect(0, 0, 0, 0), common_cmd.clip);

        RDPMultiDstBlt cmd = state_multidstblt;
        cmd.receive(stream, header);

        deltaRectangles.reset();

        deltaRectangles.out_sint16_le(316);
        deltaRectangles.out_sint16_le(378);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);

        for (int i = 0; i < 19; i++) {
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
            deltaRectangles.out_sint16_le(10);
        }

        deltaRectangles.mark_end();
        deltaRectangles.rewind();

        check<RDPMultiDstBlt>(common_cmd, cmd,
            RDPOrderCommon(MULTIDSTBLT, Rect(0, 0, 0, 0)),
            RDPMultiDstBlt(316, 378, 200, 200, 0x55, 20, deltaRectangles),
            "MultiDstBlt 2");
    }
}
