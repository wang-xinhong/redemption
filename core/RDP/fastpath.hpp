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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common fastpath layer at core module

*/

#ifndef _REDEMPTION_CORE_RDP_FASTPATH_HPP_
#define _REDEMPTION_CORE_RDP_FASTPATH_HPP_

#include "RDP/slowpath.hpp"

// 2.2.9.1.2 Server Fast-Path Update PDU (TS_FP_UPDATE_PDU)
// ========================================================

// Fast-path revises server output packets from the first byte with the goal of
// improving bandwidth.

// The TPKT Header ([T123] section 8), X.224 Class 0 Data TPDU ([X224] section
//  13.7), and MCS Send Data Indication ([T125] section 11.33) are replaced; the
//  Security Header (section 2.2.8.1.1.2) is collapsed into the fast-path output
//  header; and the Share Data Header (section 2.2.8.1.1.1.2) is replaced by a
//  new fast-path format. The contents of the graphics and pointer updates (see
//  sections 2.2.9.1.1.3 and 2.2.9.1.1.4) are also changed to reduce their size,
//  particularly by removing or reducing headers. Support for fast-path output
//  is advertised in the General Capability Set (section 2.2.7.1.1).

// fpOutputHeader (1 byte): An 8-bit, unsigned integer. One-byte, bit-packed
//  header. This byte coincides with the first byte of the TPKT Header (see
// [T123] section 8). Two pieces of information are collapsed into this byte:
// - Encryption data
// - Action code

// actionCode (2 bits): Code indicating whether the PDU is in fast-path or
// slow-path format.

// +-------------------------------------+-----------------------------+
// | 0x0 FASTPATH_OUTPUT_ACTION_FASTPATH | Indicates that the PDU is a |
// |                                     | fast-path output PDU.       |
// +-------------------------------------+-----------------------------+
// | 0x3 FASTPATH_OUTPUT_ACTION_X224     | Indicates the presence of a |
// |                                     | TPKT Header (see [T123]     |
// |                                     | section 8) initial version  |
// |                                     | byte which indicates that   |
// |                                     | the PDU is a slow-path      |
// |                                     | output PDU (in this case the|
// |                                     | full value of the initial   |
// |                                     | byte MUST be 0x03).         |
// +-------------------------------------+-----------------------------+

// reserved (4 bits): Unused bits reserved for future use. This bitfield MUST
//  be set to 0.

// encryptionFlags (2 bits): Flags describing cryptographic parameters of the
//  PDU.

// +-------------------------------------+-------------------------------------+
// | 0x1 FASTPATH_OUTPUT_SECURE_CHECKSUM | Indicates that the MAC signature for|
// |                                     | the PDU was generated using the     |
// |                                     | "salted MAC generation" technique   |
// |                                     | (see section 5.3.6.1.1). If this bit|
// |                                     | is not set, then the standard       |
// |                                     | technique was used (see sections    |
// |                                     | 2.2.8.1.1.2.2 and 2.2.8.1.1.2.3).   |
// +-------------------------------------+-------------------------------------+
// | 0x2 FASTPATH_OUTPUT_ENCRYPTED       | Indicates that the PDU contains an  |
// |                                     | 8-byte MAC signature after the      |
// |                                     | optional length2 field (that is,    |
// |                                     | the dataSignature field is present),|
// |                                     | and the contents of the PDU are     |
// |                                     | encrypted using the negotiated      |
// |                                     | encryption package (see sections    |
// |                                     | 5.3.2 and 5.3.6).                   |
// +-------------------------------------+-------------------------------------+

// length1 (1 byte): An 8-bit, unsigned integer. If the most significant bit of
//  the length1 field is not set, then the size of the PDU is in the range 1 to
//  127 bytes and the length1 field contains the overall PDU length (the length2
//  field is not present in this case). However, if the most significant bit of
//  the length1 field is set, then the overall PDU length is given by the low
//  7 bits of the length1 field concatenated with the 8 bits of the length2
//  field, in big-endian order (the length2 field contains the low-order bits).

// length2 (1 byte): An 8-bit, unsigned integer. If the most significant bit of
//  the length1 field is not set, then the length2 field is not present. If the
//  most significant bit of the length1 field is set, then the overall PDU
//  length is given by the low 7 bits of the length1 field concatenated with the
//  8 bits of the length2 field, in big-endian order (the length2 field contains
//  the low-order bits).

// fipsInformation (4 bytes): Optional FIPS header information, present when the
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3) is
//  ENCRYPTION_METHOD_FIPS (0x00000010). The Fast-Path FIPS Information
//  structure is specified in section 2.2.8.1.2.1.

// dataSignature (8 bytes): MAC generated over the packet using one of the
// techniques specified in section 5.3.6 (the FASTPATH_OUTPUT_SECURE_CHECKSUM
// flag, which is set in the fpOutputHeader field, describes the method used to
// generate the signature). This field MUST be present if the
//  FASTPATH_OUTPUT_ENCRYPTED flag is set in the fpOutputHeader field.

// fpOutputUpdates (variable): An array of Fast-Path Update (section
// 2.2.9.1.2.1) structures to be processed by the client.

TODO("To implement fastpath, the idea is to replace the current layer stack X224->Mcs->Sec with only one FastPath object. The FastPath layer would also handle legacy packets still using several independant layers. That should lead to a much simpler code in both front.hpp and rdp.hpp but still keep a flat easy to test model.")

namespace FastPath {

// 2.2.8.1.2 Client Fast-Path Input Event PDU (TS_FP_INPUT_PDU)
// ============================================================
// The Fast-Path Input Event PDU is used to transmit input events from client to
//  server.<18> Fast-path revises client input packets from the first byte with
//  the goal of improving bandwidth. The TPKT Header ([T123] section 8), X.224
//  Class 0 Data TPDU ([X224] section 13.7), and MCS Send Data Request ([T125]
//  section 11.32) are replaced; the Security Header (section 2.2.8.1.1.2) is
//  collapsed into the fast-path input header, and the Share Data Header
//  (section 2.2.8.1.1.1.2) is replaced by a new fast-path format. The contents
//  of the input notification events (section 2.2.8.1.1.3.1.1) are also changed
//  to reduce their size, particularly by removing or reducing headers. Support
//  for fast-path input is advertised in the Input Capability Set (section
//  2.2.7.1.6).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | fpInputHeader |    length1    |    length2    | fipsInformation
// |               |               |   (optional)  |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |                      ...                      | dataSignature |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                              ...                              |
// +-----------------------------------------------+---------------+
// |                      ...                      |   numEvents   |
// |                                               |   (optional)  |
// +-----------------------------------------------+---------------+
// |                   fpInputEvents (variable)                    |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// fpInputHeader (1 byte): An 8-bit, unsigned integer. One-byte, bit-packed
//  header. This byte coincides with the first byte of the TPKT Header (see
//  [T123] section 8). Three pieces of information are collapsed into this byte:
//  1. Security flags
//  2. Number of events in the fast-path input PDU
//  3. Action code
//  The format of the fpInputHeader byte is described by the following bitmask
//   diagram.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   |       | s |
// | a |       | e |
// | c |       | c |
// | t | numEv | F |
// | i |  ents | l |
// | o |       | a | 
// | n |       | g |
// |   |       | s |
// +---+-------+---+

// action (2 bits): A 2-bit code indicating whether the PDU is in fast-path or
//  slow-path format.

// +------------------------------------+--------------------------------------+
// | 2-Bit Codes                        | Meaning                              |
// +------------------------------------+--------------------------------------+
// | 0x0 FASTPATH_INPUT_ACTION_FASTPATH | Indicates the PDU is a fast-path     |
// |                                    | input PDU.                           |
// +------------------------------------+--------------------------------------+
// | 0x3 FASTPATH_INPUT_ACTION_X224     | Indicates the presence of a TPKT     |
// |                                    | Header initial version byte, which   |
// |                                    | indicates that the PDU is a          |
// |                                    | slow-path input PDU (in this case    |
// |                                    | the full value of the initial byte   |
// |                                    | MUST be 0x03).                       |
// +------------------------------------+--------------------------------------+

    enum {
          FASTPATH_OUTPUT_ACTION_FASTPATH = 0x0
        , FASTPATH_OUTPUT_ACTION_X224     = 0x3
    };

// numEvents (4 bits): Collapses the number of fast-path input events packed
//  together in the fpInputEvents field into 4 bits if the number of events is
//  in the range 1 to 15. If the number of input events is greater than 15, then
//  the numEvents bit field in the fast-path header byte MUST be set to zero,
//  and the numEvents optional field inserted after the dataSignature field.
//  This allows up to 255 input events in one PDU.

// secFlags (2 bits): A 2-bit field containing the flags that describe the
//  cryptographic parameters of the PDU.

// +------------------------------------+--------------------------------------+
// | Flag (2 Bits)                      | Meaning                              |
// +------------------------------------+--------------------------------------+
// | 0x1 FASTPATH_INPUT_SECURE_CHECKSUM | Indicates that the MAC signature for |
// |                                    | the PDU was generated using the      |
// |                                    | "salted MAC generation" technique    |
// |                                    | (see section 5.3.6.1.1). If this bit |
// |                                    | is not set, then the standard        |
// |                                    | technique was used (see sections     |
// |                                    | 2.2.8.1.1.2.2 and 2.2.8.1.1.2.3).    |
// +------------------------------------+--------------------------------------+
// | 0x2 FASTPATH_INPUT_ENCRYPTED       | Indicates that the PDU contains an   |
// |                                    | 8-byte MAC signature after the       |
// |                                    | optional length2 field (that is, the |
// |                                    | dataSignature field is present) and  |
// |                                    | the contents of the PDU are          |
// |                                    | encrypted using the negotiated       |
// |                                    | encryption package (see sections     |
// |                                    | 5.3.2 and 5.3.6).                    |
// +------------------------------------+--------------------------------------+

    enum {
          FASTPATH_INPUT_SECURE_CHECKSUM
        , FASTPATH_INPUT_ENCRYPTED
    };

// length1 (1 byte): An 8-bit, unsigned integer. If the most significant bit of
//  the length1 field is not set, then the size of the PDU is in the range 1 to
//  127 bytes and the length1 field contains the overall PDU length (the length2
//  field is not present in this case). However, if the most significant bit of
//  the length1 field is set, then the overall PDU length is given by the low 7
//  bits of the length1 field concatenated with the 8 bits of the length2 field,
//  in big-endian order (the length2 field contains the low-order bits).

// length2 (1 byte): An 8-bit, unsigned integer. If the most significant bit of
//  the length1 field is not set, then the length2 field is not present. If the
//  most significant bit of the length1 field is set, then the overall PDU
//  length is given by the low 7 bits of the length1 field concatenated with the
//  8 bits of the length2 field, in big-endian order (the length2 field contains
//  the low-order bits).

// fipsInformation (4 bytes): Optional FIPS header information, present when the
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3) is
//  ENCRYPTION_METHOD_FIPS (0x00000010). The Fast-Path FIPS Information
//  structure is specified in section 2.2.8.1.2.1.

// dataSignature (8 bytes): MAC generated over the packet using one of the
//  techniques described in section 5.3.6 (the FASTPATH_INPUT_SECURE_CHECKSUM
//  flag, which is set in the fpInputHeader field, describes the method used to
//  generate the signature). This field MUST be present if the
//  FASTPATH_INPUT_ENCRYPTED flag is set in the fpInputHeader field.

// numEvents (1 byte): An 8-bit, unsigned integer. The number of fast-path input
//  events packed together in the fpInputEvents field (up to 255). This field is
//  present if the numEvents bit field in the fast-path header byte is zero.

// fpInputEvents (variable): An array of Fast-Path Input Event (section
//  2.2.8.1.2.2) structures to be processed by the server. The number of events
//  present in this array is given by the numEvents bit field in the fast-path
//  header byte, or by the numEvents field in the Fast-Path Input Event PDU (if
//  it is present).

    struct ClientInputEventPDU_Recv {
        uint8_t   numEvents;
        uint8_t   secFlags;
        uint32_t  fipsInformation;
        uint8_t   dataSignature[8];
        SubStream payload;

        ClientInputEventPDU_Recv( Transport & trans
                                , Stream & stream
                                , CryptContext & decrypt)
        : numEvents(0)
        , secFlags(0)
        , fipsInformation(0)
        , payload(stream) {
            ::memset(dataSignature, 0, sizeof(dataSignature));

            stream.rewind();

            if (stream.size() < 1)
                trans.recv(&stream.end, 1);

            uint8_t byte;

            byte = stream.in_uint8();

            int action = byte & 0x03;
            if (action != 0) {
                LOG(LOG_INFO, "Fast-path PDU excepted: action=0x%X", action);
                throw Error(ERR_RDP_FASTPATH);
            }

            this->numEvents = (byte & 0x3C) >> 2;
            this->secFlags  = (byte & 0xC0) >> 6;

            trans.recv(&stream.end, 1);

            uint16_t length;
            byte = stream.in_uint8();
            if (byte & 0x80){
                length = (byte & 0x7F) << 8;

                trans.recv(&stream.end, 1);
                byte = stream.in_uint8();
                length += byte;
            }
            else{
                length = byte;
            }

            trans.recv(&stream.end, length - stream.size());

            TODO("RZ: Should we treat fipsInformation ?")

            unsigned expected =
                  8                               // dataSignature
                + ((this->numEvents == 0) ? 1 : 0) // numEvent
                ;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "FastPath::ClientInputEventPDU: data truncated, expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDP_FASTPATH);
            }

            stream.in_copy_bytes(this->dataSignature, 8);

            if (this->numEvents == 0){
                this->numEvents = stream.in_uint8();
            }

            this->payload.resize(stream, stream.in_remain());

            decrypt.decrypt(payload);
        }   // ClientInputEventPDU_Recv(Transport & trans, Stream & stream)
    };  // struct ClientInputEventPDU_Recv

// 2.2.8.1.2.2 Fast-Path Input Event (TS_FP_INPUT_EVENT)
// =====================================================

// The TS_FP_INPUT_EVENT structure is used to describe the type and encapsulate
//  the data for a fast-path input event sent from client to server. All
//  fast-path input events conform to this basic structure (see sections
//  2.2.8.1.2.2.1 to 2.2.8.1.2.2.5).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  eventHeader  |              eventData (variable)             |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// eventHeader (1 byte): An 8-bit, unsigned integer. One byte bit-packed event
//  header. Two pieces of information are collapsed into this byte:
//  1. Fast-path input event type
//  2. Flags specific to the input event
//  The eventHeader field is structured as follows:

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         | eve |
// | eventFl | ntC |
// |   ags   | ode |
// +---------+-----+

// eventFlags (5 bits): 5 bits. The flags specific to the input event.

// eventCode (3 bits): 3 bits. The type code of the input event.

// +-----------------------------------+---------------------------------------+
// | 3-Bit Codes                       | Meaning                               |
// +-----------------------------------+---------------------------------------+
// | 0x0 FASTPATH_INPUT_EVENT_SCANCODE | Indicates a Fast-Path Keyboard Event  |
// |                                   | (section 2.2.8.1.2.2.1).              |
// +-----------------------------------+---------------------------------------+
// | 0x1 FASTPATH_INPUT_EVENT_MOUSE    | Indicates a Fast-Path Mouse Event     |
// |                                   | (section 2.2.8.1.2.2.3).              |
// +-----------------------------------+---------------------------------------+
// | 0x2 FASTPATH_INPUT_EVENT_MOUSEX   | Indicates a Fast-Path Extended Mouse  |
// |                                   | Event (section 2.2.8.1.2.2.4).        |
// +-----------------------------------+---------------------------------------+
// | 0x3 FASTPATH_INPUT_EVENT_SYNC     | Indicates a Fast-Path Synchronize     |
// |                                   | Event (section 2.2.8.1.2.2.5).        |
// +-----------------------------------+---------------------------------------+
// | 0x4 FASTPATH_INPUT_EVENT_UNICODE  | Indicates a Fast-Path Unicode         |
// |                                   | Keyboard Event                        |
// |                                   | (section 2.2.8.1.2.2.2).              |
// +-----------------------------------+---------------------------------------+

// eventData (variable): Optional and variable-length data specific to the input
//  event.

    enum {
          FASTPATH_INPUT_EVENT_SCANCODE = 0x0
        , FASTPATH_INPUT_EVENT_MOUSE    = 0x1
        , FASTPATH_INPUT_EVENT_MOUSEX   = 0x2
        , FASTPATH_INPUT_EVENT_SYNC     = 0x3
        , FASTPATH_INPUT_EVENT_UNICODE  = 0x4
    };

// 2.2.8.1.2.2.1 Fast-Path Keyboard Event (TS_FP_KEYBOARD_EVENT)
// =============================================================

// The TS_FP_KEYBOARD_EVENT structure is the fast-path variant of the
//  TS_KEYBOARD_EVENT (section 2.2.8.1.1.3.1.1.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  eventHeader  |    keyCode    |
// +---------------+---------------+

// eventHeader (1 byte): An 8-bit, unsigned integer. The format of this field is
//  the same as the eventHeader byte field described in section 2.2.8.1.2.2.
//  The eventCode bitfield (3 bits in size) MUST be set to
//  FASTPATH_INPUT_EVENT_SCANCODE (0). The eventFlags bitfield (5 bits in size)
//  contains flags describing the keyboard event.

// +---------------------------------------+-----------------------------------+
// | 5-Bit Codes                           | Meaning                           |
// +---------------------------------------+-----------------------------------+
// | 0x01 FASTPATH_INPUT_KBDFLAGS_RELEASE  | The absence of this flag          |
// |                                       | indicates a key-down event,       |
// |                                       | while its presence indicates a    |
// |                                       | key-release event.                |
// +---------------------------------------+-----------------------------------+
// | 0x02 FASTPATH_INPUT_KBDFLAGS_EXTENDED | The keystroke message contains an |
// |                                       | extended scancode. For enhanced   |
// |                                       | 101-key and 102-key keyboards,    |
// |                                       | extended keys include the right   |
// |                                       | ALT and right CTRL keys on the    |
// |                                       | main section of the keyboard; the |
// |                                       | INS, DEL, HOME, END, PAGE UP,     |
// |                                       | PAGE DOWN and ARROW keys in the   |
// |                                       | clusters to the left of the       |
// |                                       | numeric keypad; and the Divide    |
// |                                       | ("/") and ENTER keys in the       |
// |                                       | numeric keypad.                   |
// +---------------------------------------+-----------------------------------+

    enum {
          FASTPATH_INPUT_KBDFLAGS_RELEASE  = 0x01
        , FASTPATH_INPUT_KBDFLAGS_EXTENDED = 0x02
    };

// keyCode (1 byte): An 8-bit, unsigned integer. The scancode of the key which
//  triggered the event.

    struct KeyboardEvent_Recv {
        uint8_t  eventFlags;
        uint16_t spKeyboardFlags; // Slow-path compatible flags
        uint8_t  keyCode;

        KeyboardEvent_Recv(Stream & stream, uint8_t eventHeader)
        : eventFlags(0)
        , spKeyboardFlags(0)
        , keyCode(0) {
            uint8_t eventCode = (eventHeader & 0xE0) >> 5;
            if (eventCode != FASTPATH_INPUT_EVENT_SCANCODE) {
                LOG(LOG_ERR, "FastPath::KeyboardEvent: unexpected event code, expected=0x%X got=0x%X",
                    FASTPATH_INPUT_EVENT_SCANCODE, eventCode);
                throw Error(ERR_RDP_FASTPATH);
            }

            this->eventFlags = eventHeader & 0x1F;

            if (this->eventFlags & FASTPATH_INPUT_KBDFLAGS_RELEASE){
                this->spKeyboardFlags |= SlowPath::KBDFLAGS_DOWN | SlowPath::KBDFLAGS_RELEASE;
            }

            if (this->eventFlags & FastPath::FASTPATH_INPUT_KBDFLAGS_EXTENDED){
                this->spKeyboardFlags |= SlowPath::KBDFLAGS_EXTENDED; 
            }

            if (!stream.in_check_rem(1)) {
                LOG(LOG_ERR, "FastPath::KeyboardEvent: data truncated, expected=1 remains=%u",
                    stream.in_remain());
                throw Error(ERR_RDP_FASTPATH);
            }

            this->keyCode = stream.in_uint8();
        }
    };

// 2.2.8.1.2.2.3 Fast-Path Mouse Event (TS_FP_POINTER_EVENT)
// =========================================================

// The TS_FP_POINTER_EVENT structure is the fast-path variant of the
//  TS_POINTER_EVENT (section 2.2.8.1.1.3.1.1.3) structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  eventHeader  |          pointerFlags         |      xPos     |
// +---------------+-------------------------------+---------------+
// |      ...      |              xPos             |
// +---------------+-------------------------------+

// eventHeader (1 byte): An 8-bit, unsigned integer. The format of this field is
//  the same as the eventHeader byte field, specified in section 2.2.8.1.2.2.
//  The eventCode bitfield (3 bits in size) MUST be set to
//  FASTPATH_INPUT_EVENT_MOUSE (1). The eventFlags bitfield (5 bits in size)
//  MUST be zeroed out.

// pointerFlags (2 bytes): A 16-bit, unsigned integer. The flags describing the
//  pointer event. The possible flags are identical to those found in the
//  pointerFlags field of the TS_POINTER_EVENT structure.

// xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate of the pointer.

// yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate of the pointer.

    struct MouseEvent_Recv {
        uint16_t pointerFlags;
        uint16_t xPos;
        uint16_t yPos;

        MouseEvent_Recv(Stream & stream, uint8_t eventHeader)
        : pointerFlags(0)
        , xPos(0)
        , yPos(0) {
            uint8_t eventCode = (eventHeader & 0xE0) >> 5;
            if (eventCode != FASTPATH_INPUT_EVENT_MOUSE) {
                LOG(LOG_ERR, "FastPath::MouseEvent: unexpected event code, expected=0x%X got=0x%X",
                    FASTPATH_INPUT_EVENT_MOUSE, eventCode);
                throw Error(ERR_RDP_FASTPATH);
            }

            unsigned expected =
                  6; // pointerFlags(2) + xPos(2) + yPos(2)
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "FastPath::MouseEvent: data truncated, expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDP_FASTPATH);
            }

            this->pointerFlags = stream.in_uint16_le();
            this->xPos         = stream.in_uint16_le();
            this->yPos         = stream.in_uint16_le();
        }
    };

// 2.2.8.1.2.2.5 Fast-Path Synchronize Event (TS_FP_SYNC_EVENT)
// ============================================================

// The TS_FP_SYNC_EVENT structure is the fast-path variant of the TS_SYNC_EVENT
//  (section 2.2.8.1.1.3.1.1.5) structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  eventHeader  |
// +---------------+

// eventHeader (1 byte): An 8-bit, unsigned integer. The format of this field is
//  the same as the eventHeader byte field, specified in section 2.2.8.1.2.2.
//  The eventCode bitfield (3 bits in size) MUST be set to
//  FASTPATH_INPUT_EVENT_SYNC (3). The eventFlags bitfield (5 bits in size)
//  contains flags indicating the "on" status of the keyboard toggle keys.

// +--------------------------------------+------------------------------------+
// | 5-Bit Codes                          | Meaning                            |
// +--------------------------------------+------------------------------------+
// | 0x01 FASTPATH_INPUT_SYNC_SCROLL_LOCK | Indicates that the Scroll Lock     |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+
// | 0x02 FASTPATH_INPUT_SYNC_NUM_LOCK    | Indicates that the Num Lock        |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+
// | 0x04 FASTPATH_INPUT_SYNC_CAPS_LOCK   | Indicates that the Caps Lock       |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+
// | 0x08 FASTPATH_INPUT_SYNC_KANA_LOCK   | Indicates that the Kana Lock       |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+

    struct SynchronizeEvent_Recv {
        uint8_t  eventFlags;

        SynchronizeEvent_Recv(Stream & stream, uint8_t eventHeader)
        : eventFlags(0) {
            uint8_t eventCode = (eventHeader & 0xE0) >> 5;
            if (eventCode != FASTPATH_INPUT_EVENT_SYNC) {
                LOG(LOG_ERR, "FastPath::SynchronizeEvent: unexpected event code, expected=0x%X got=0x%X",
                    FASTPATH_INPUT_EVENT_MOUSE, eventCode);
                throw Error(ERR_RDP_FASTPATH);
            }

            this->eventFlags = eventHeader & 0x1F;
        }
    };

} // namespace FastPath

#endif // #ifndef _REDEMPTION_CORE_RDP_FASTPATH_HPP_
