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

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#if !defined(__CORE_RDP_GCC_HPP__)
#define __CORE_RDP_GCC_HPP__

#include "stream.hpp"

enum DATA_BLOCK_TYPE {
    //  The data block that follows contains Client Core Data (section 2.2.1.3.2).
    CS_CORE = 0xC001,
    // The data block that follows contains Client Security Data (section 2.2.1.3.3).
    CS_SECURITY = 0xC002,
    // The data block that follows contains Client Network Data (section 2.2.1.3.4).
    CS_NET = 0xC003,
    // The data block that follows contains Client Cluster Data (section 2.2.1.3.5).
    CS_CLUSTER = 0xC004,
    // The data block that follows contains Client Monitor Data (section 2.2.1.3.6).
    CS_MONITOR = 0xC005,
    // The data block that follows contains Server Core Data (section 2.2.1.4.2).
    SC_CORE = 0x0C01,
    // The data block that follows contains Server Security Data (section 2.2.1.4.3).
    SC_SECURITY = 0x0C02,
    // The data block that follows contains Server Network Data (section 2.2.1.4.4).
    SC_NET = 0x0C03
};

#include "gcc_conference_user_data/cs_core.hpp"
#include "gcc_conference_user_data/cs_cluster.hpp"
#include "gcc_conference_user_data/cs_monitor.hpp"
#include "gcc_conference_user_data/cs_net.hpp"
#include "gcc_conference_user_data/cs_sec.hpp"
//#include "gcc_conference_user_data/sc_core.hpp"
#include "gcc_conference_user_data/sc_net.hpp"
#include "gcc_conference_user_data/sc_sec1.hpp"

namespace GCC
{
// ConferenceName ::= SEQUENCE
// {
//    numeric     SimpleNumericString,
//    text        SimpleTextString OPTIONAL,
//    ...,
//    unicodeText TextString OPTIONAL
// }

// ConferenceCreateRequest ::= SEQUENCE
// { -- MCS-Connect-Provider request user data
//    conferenceName         ConferenceName,
//    convenerPassword       Password OPTIONAL,
//    password               Password OPTIONAL,
//    lockedConference       BOOLEAN,
//    listedConference       BOOLEAN,
//    conductibleConference  BOOLEAN,
//    terminationMethod      TerminationMethod,
//    conductorPrivileges    SET OF Privilege OPTIONAL,
//    conductedPrivileges    SET OF Privilege OPTIONAL,
//    nonConductedPrivileges SET OF Privilege OPTIONAL,
//    conferenceDescription  TextString OPTIONAL,
//    callerIdentifier       TextString OPTIONAL,
//    userData               UserData OPTIONAL,
//    ...,
//    conferencePriority     ConferencePriority OPTIONAL,
//    conferenceMode         ConferenceMode OPTIONAL
// }

//    PER encoded (ALIGNED variant of BASIC-PER) GCC Connection Data (ConnectData):
//    00 05 00 14 7c 00 01 81 2a 00 08 00 10 00 01 c0
//    00 44 75 63 61 81 1c

//    0 - CHOICE: From Key select object (0) of type OBJECT IDENTIFIER
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding

//    05 -> object length = 5 bytes

//    00 14 7c 00 01 -> object
//    The first byte gives the first two values in the sextuple (m and n), as it is encoded as
//    40m + n. Hence, decoding the remaining data yields the correct results:

//    OID = { 0 0 20 124 0 1 } = {itu-t(0) recommendation(0) t(20) t124(124) version(0) 1}
//    Description = v.1 of ITU-T Recommendation T.124 (Feb 1998): "Generic Conference Control"

//    81 2a -> ConnectData::connectPDU length = 298 bytes
//    Since the most significant bit of the first byte (0x81) is set to 1 and the following bit is
//    set to 0, the length is given by the low six bits of the first byte and the second byte.
//    Hence, the value is 0x12a, which is 298 bytes.

//    PER encoded (ALIGNED variant of BASIC-PER) GCC Conference Create Request PDU:
//    00 08 00 10 00 01 c0 00 44 75 63 61 81 1c

//    0x00:
//    0 - extension bit (ConnectGCCPDU)
//    0 - --\.
//    0 -   | CHOICE: From ConnectGCCPDU select conferenceCreateRequest (0)
//    0 - --/ of type ConferenceCreateRequest
//    0 - extension bit (ConferenceCreateRequest)
//    0 - ConferenceCreateRequest::convenerPassword present
//    0 - ConferenceCreateRequest::password present
//    0 - ConferenceCreateRequest::conductorPrivileges present


//    0x08:
//    0 - ConferenceCreateRequest::conductedPrivileges present
//    0 - ConferenceCreateRequest::nonConductedPrivileges present
//    0 - ConferenceCreateRequest::conferenceDescription present
//    0 - ConferenceCreateRequest::callerIdentifier present
//    1 - ConferenceCreateRequest::userData present
//    0 - extension bit (ConferenceName)
//    0 - ConferenceName::text present
//    0 - padding

//    0x00
//    0 - --\.
//    0 -   |
//    0 -   |
//    0 -   | ConferenceName::numeric length = 0 + 1 = 1 character
//    0 -   | (minimum for SimpleNumericString is 1)
//    0 -   |
//    0 -   |
//    0 - --/

//    0x10:
//    0 - --\.
//    0 -   | ConferenceName::numeric = "1"
//    0 -   |
//    1 - --/
//    0 - ConferenceCreateRequest::lockedConference
//    0 - ConferenceCreateRequest::listedConference
//    0 - ConferenceCreateRequest::conducibleConference
//    0 - extension bit (TerminationMethod)

//    0x00:
//    0 - TerminationMethod::automatic
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding

//    0x01:
//    0 - --\.
//    0 -   |
//    0 -   |
//    0 -   | number of UserData sets = 1
//    0 -   |
//    0 -   |
//    0 -   |
//    1 - --/

//    0xc0:
//    1 - UserData::value present
//    1 - CHOICE: From Key select h221NonStandard (1) of type H221NonStandardIdentifier
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding

//    0x00:
//    0 - --\.
//    0 -   |
//    0 -   |
//    0 -   | h221NonStandard length = 0 + 4 = 4 octets
//    0 -   | (minimum for H221NonStandardIdentifier is 4)
//    0 -   |
//    0 -   |
//    0 - --/

//    44 75 63 61 -> h221NonStandard (client-to-server H.221 key) = "Duca"
//    81 1c -> UserData::value length = 284 bytes
//    Since the most significant bit of the first byte (0x81) is set to 1 and the following bit is
//    set to 0, the length is given by the low six bits of the first byte and the second byte.
//    Hence, the value is 0x11c, which is 284 bytes.


    class Create_Request_Send {
        public:
        Create_Request_Send(Stream & stream, size_t payload_size) {
            // ConnectData
            stream.out_per_choice(0); // From Key select object (0) of type OBJECT_IDENTIFIER
            const uint8_t t124_02_98_oid[6] = { 0, 0, 20, 124, 0, 1 };
            stream.out_per_object_identifier(t124_02_98_oid); // ITU-T T.124 (02/98) OBJECT_IDENTIFIER

            //  ConnectData::connectPDU (OCTET_STRING)
            uint16_t offset_len = stream.get_offset();
            stream.out_per_length(256); // connectPDU length (reserve 16 bits)

            //  ConnectGCCPDU
            stream.out_per_choice(0); // From ConnectGCCPDU select conferenceCreateRequest (0) of type ConferenceCreateRequest
            stream.out_per_selection(0x08); // select optional userData from ConferenceCreateRequest

            //  ConferenceCreateRequest::conferenceName
            //	stream.out_per_numeric_string(s, (uint8*)"1", 1, 1); /* ConferenceName::numeric */
            stream.out_uint16_be(16);
            stream.out_per_padding(1); /* padding */

            //  UserData (SET OF SEQUENCE)
            stream.out_per_number_of_sets(1); // one set of UserData
            stream.out_per_choice(0xC0); // UserData::value present + select h221NonStandard (1)

            //  h221NonStandard
            const uint8_t h221_cs_key[4] = {'D', 'u', 'c', 'a'};
            stream.out_per_octet_string(h221_cs_key, 4, 4); // h221NonStandard, client-to-server H.221 key, "Duca"

            stream.out_per_length(payload_size); // user data length
            stream.mark_end();

            stream.set_out_per_length(payload_size + stream.get_offset() - 9, offset_len); // length including header
        }
    };

    class Create_Request_Recv {
        public:
        size_t payload_size;

        SubStream payload;

        Create_Request_Recv(Stream & stream) {
            // Key select object (0) of type OBJECT_IDENTIFIER
            // ITU-T T.124 (02/98) OBJECT_IDENTIFIER
            stream.in_skip_bytes(7);
            uint16_t length_with_header = stream.in_per_length();

            // ConnectGCCPDU
            // From ConnectGCCPDU select conferenceCreateRequest (0) of type ConferenceCreateRequest
            // select optional userData from ConferenceCreateRequest
            // ConferenceCreateRequest::conferenceName
            // UserData (SET OF SEQUENCE), one set of UserData
            // UserData::value present + select h221NonStandard (1)
            // h221NonStandard, client-to-server H.221 key, "Duca"

            stream.in_skip_bytes(12);
            uint16_t length = stream.in_per_length();

            if (length_with_header != length + 14){
                LOG(LOG_WARNING, "GCC Conference Create Request User data Length mismatch with header+data length %u %u", length, length_with_header);
                throw Error(ERR_GCC);
            }

            if (length != stream.size() - stream.get_offset()){
                LOG(LOG_WARNING, "GCC Conference Create Request User data Length mismatch with header %u %u", length, stream.size() - stream.get_offset());
                throw Error(ERR_GCC);
            }
            this->payload.resize(stream, stream.size() - stream.get_offset());
        }
    };

    // GCC Conference Create Response
    // ------------------------------

    // ConferenceCreateResponse Parameters
    // -----------------------------------

    // Generic definitions used in parameter descriptions:

    // simpleTextFirstCharacter UniversalString ::= {0, 0, 0, 0}

    // simpleTextLastCharacter UniversalString ::= {0, 0, 0, 255}

    // SimpleTextString ::=  BMPString (SIZE (0..255)) (FROM (simpleTextFirstCharacter..simpleTextLastCharacter))

    // TextString ::= BMPString (SIZE (0..255)) -- Basic Multilingual Plane of ISO/IEC 10646-1 (Unicode)

    // SimpleNumericString ::= NumericString (SIZE (1..255)) (FROM ("0123456789"))

    // DynamicChannelID ::= INTEGER (1001..65535) -- Those created and deleted by MCS

    // UserID ::= DynamicChannelID

    // H221NonStandardIdentifier ::= OCTET STRING (SIZE (4..255))
    //      -- First four octets shall be country code and
    //      -- Manufacturer code, assigned as specified in
    //      -- Annex A/H.221 for NS-cap and NS-comm

    // Key ::= CHOICE   -- Identifier of a standard or non-standard object
    // {
    //      object              OBJECT IDENTIFIER,
    //      h221NonStandard     H221NonStandardIdentifier
    // }

    // UserData ::= SET OF SEQUENCE
    // {
    //      key     Key,
    //      value   OCTET STRING OPTIONAL
    // }

    // ConferenceCreateResponse ::= SEQUENCE
    // {    -- MCS-Connect-Provider response user data
    //      nodeID              UserID, -- Node ID of the sending node
    //      tag                 INTEGER,
    //      result              ENUMERATED
    //      {
    //          success                         (0),
    //          userRejected                    (1),
    //          resourcesNotAvailable           (2),
    //          rejectedForSymmetryBreaking     (3),
    //          lockedConferenceNotSupported    (4),
    //          ...
    //      },
    //      userData            UserData OPTIONAL,
    //      ...
    //}


    // User Data                 : Optional
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // User Data: Optional user data which may be used for functions outside
    // the scope of this Recommendation such as authentication, billing,
    // etc.

    // Result                    : Mandatory
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // An indication of whether the request was accepted or rejected, and if
    // rejected, the reason why. It contains one of a list of possible
    // results: successful, user rejected, resources not available, rejected
    // for symmetry-breaking, locked conference not supported, Conference
    // Name and Conference Name Modifier already exist, domain parameters
    // unacceptable, domain not hierarchical, lower-layer initiated
    // disconnect, unspecified failure to connect. A negative result in the
    // GCC-Conference-Create confirm does not imply that the physical
    // connection to the node to which the connection was being attempted
    // is disconnected.

    // The ConferenceCreateResponse PDU is shown in Table 8-4. The Node ID
    // parameter, which is the User ID assigned by MCS in response to the
    // MCS-Attach-User request issued by the GCC Provider, shall be supplied
    // by the GCC Provider sourcing this PDU. The Tag parameter is assigned
    // by the source GCC Provider to be locally unique. It is used to
    // identify the returned UserIDIndication PDU. The Result parameter
    // includes GCC-specific failure information sourced directly from
    // the Result parameter in the GCC-Conference-Create response primitive.
    // If the Result parameter is anything except successful, the Result
    // parameter in the MCS-Connect-Provider response is set to
    // user-rejected.

    //            Table 8-4 – ConferenceCreateResponse GCCPDU
    // +------------------+------------------+--------------------------+
    // | Content          |     Source       |         Sink             |
    // +==================+==================+==========================+
    // | Node ID          | Top GCC Provider | Destination GCC Provider |
    // +------------------+------------------+--------------------------+
    // | Tag              | Top GCC Provider | Destination GCC Provider |
    // +------------------+------------------+--------------------------+
    // | Result           | Response         | Confirm                  |
    // +------------------+------------------+--------------------------+
    // | User Data (opt.) | Response         | Confirm                  |
    // +------------------+------------------+--------------------------+

    //PER encoded (ALIGNED variant of BASIC-PER) GCC Connection Data (ConnectData):
    // 00 05 00
    // 14 7c 00 01
    // 2a
    // 14 76 0a 01 01 00 01 c0 00 4d 63 44 6e
    // 81 08

    class Create_Response_Send {
        public:
        Create_Response_Send(Stream & stream, size_t payload_size) {
            // ConnectData
            // 00 05 -> Key::object length = 5 bytes
            // 00 14 7c 00 01 -> Key::object = { 0 0 20 124 0 1 }
            stream.out_uint16_be(5);
            stream.out_copy_bytes("\x00\x14\x7c\x00\x01", 5);

            // 2a -> ConnectData::connectPDU length = 42 bytes
            // This length MUST be ignored by the client.
            stream.out_uint8(0x2a);

            // PER encoded (ALIGNED variant of BASIC-PER) GCC Conference Create Response
            // PDU:
            // 14 76 0a 01 01 00 01 c0 00 00 4d 63 44 6e 81 08

            // 0x14:
            // 0 - extension bit (ConnectGCCPDU)
            // 0 - --\ ...
            // 0 -   | CHOICE: From ConnectGCCPDU select conferenceCreateResponse (1)
            // 1 - --/ of type ConferenceCreateResponse
            // 0 - extension bit (ConferenceCreateResponse)
            // 1 - ConferenceCreateResponse::userData present
            // 0 - padding
            // 0 - padding
            stream.out_uint8(0x10 | 4);

            // ConferenceCreateResponse::nodeID
            //  = 0x760a + 1001 = 30218 + 1001 = 31219
            //  (minimum for UserID is 1001)
            stream.out_uint16_le(0x760a);

            // ConferenceCreateResponse::tag length = 1 byte
            stream.out_uint8(1);

            // ConferenceCreateResponse::tag = 1
            stream.out_uint8(1);

            // 0x00:
            // 0 - extension bit (Result)
            // 0 - --\ ...
            // 0 -   | ConferenceCreateResponse::result = success (0)
            // 0 - --/
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            stream.out_uint8(0);

            // number of UserData sets = 1
            stream.out_uint8(1);

            // 0xc0:
            // 1 - UserData::value present
            // 1 - CHOICE: From Key select h221NonStandard (1)
            //               of type H221NonStandardIdentifier
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            stream.out_uint8(0xc0);

            // h221NonStandard length = 0 + 4 = 4 octets
            //   (minimum for H221NonStandardIdentifier is 4)
            stream.out_uint8(0);

            // h221NonStandard (server-to-client H.221 key) = "McDn"
            stream.out_copy_bytes("McDn", 4);

            // set user_data_len (TWO_BYTE_UNSIGNED_ENCODING)
            stream.out_uint16_be(0x8000 | payload_size);
            stream.mark_end();
        }
    };

    class Create_Response_Recv {
        public:
        size_t payload_size;

        SubStream payload;

        Create_Response_Recv(Stream & stream) {
            stream.in_skip_bytes(21); /* header (T.124 ConferenceCreateResponse) */
            size_t length = stream.in_per_length();
            if (length != stream.size() - stream.get_offset()){
                LOG(LOG_WARNING, "GCC Conference Create Response User data Length mismatch with header %u %u",
                    length, stream.size() - stream.get_offset());
                throw Error(ERR_GCC);
            }
            
            this->payload.resize(stream, stream.size() - stream.get_offset());
        }
    };


    // 2.2.1.3.1 User Data Header (TS_UD_HEADER)
    // =========================================

    // type (2 bytes): A 16-bit, unsigned integer. The type of the data
    //                 block that this header precedes.

    // +-------------------+-------------------------------------------------------+
    // | CS_CORE 0xC001    | The data block that follows contains Client Core      |
    // |                   | Data (section 2.2.1.3.2).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_SECURITY 0xC002| The data block that follows contains Client           |
    // |                   | Security Data (section 2.2.1.3.3).                    |
    // +-------------------+-------------------------------------------------------+
    // | CS_NET 0xC003     | The data block that follows contains Client Network   |
    // |                   | Data (section 2.2.1.3.4).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_CLUSTER 0xC004 | The data block that follows contains Client Cluster   |
    // |                   | Data (section 2.2.1.3.5).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_MONITOR 0xC005 | The data block that follows contains Client           |
    // |                   | Monitor Data (section 2.2.1.3.6).                     |
    // +-------------------+-------------------------------------------------------+
    // | SC_CORE 0x0C01    | The data block that follows contains Server Core      |
    // |                   | Data (section 2.2.1.4.2)                              |
    // +-------------------+-------------------------------------------------------+
    // | SC_SECURITY 0x0C02| The data block that follows contains Server           |
    // |                   | Security Data (section 2.2.1.4.3).                    |
    // +-------------------+-------------------------------------------------------+
    // | SC_NET 0x0C03     | The data block that follows contains Server Network   |
    // |                   | Data (section 2.2.1.4.4)                              |
    // +-------------------+-------------------------------------------------------+

    // length (2 bytes): A 16-bit, unsigned integer. The size in bytes of the data
    //   block, including this header.

    namespace UserData
    {
        struct RecvFactory
        {
            uint16_t tag;
            uint16_t length;
            SubStream payload;

            RecvFactory(Stream & stream) : payload(stream, stream.get_offset())
            {
                if (!stream.check_rem(4)){
                    LOG(LOG_WARNING, "Incomplete GCC::UserData data block header");                      
                    throw Error(ERR_GCC);
                }
                this->tag = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                LOG(LOG_INFO, "GCC::UserData tag=%0.4x length=%u", tag, length);
                if (!stream.check_rem(length - 4)){
                    LOG(LOG_WARNING, "Incomplete GCC::UserData data block"
                                     " tag=%u length=%u available_length=%u",
                                     tag, length, stream.size() - 4);                      
                    throw Error(ERR_GCC);
                }
                stream.in_skip_bytes(length - 4);
                this->payload.resize(this->payload, length);
            }
        };


        // 2.2.1.4.2 Server Core Data (TS_UD_SC_CORE)
        // ==========================================

        // The TS_UD_SC_CORE data block contains core server connection-related
        // information.

        // header (4 bytes): GCC user data block header, as specified in User Data
        //  Header (section 2.2.1.3.1). The User Data Header type field MUST be set to
        //  SC_CORE (0x0C01).

        // version (4 bytes): A 32-bit, unsigned integer. The server version number for
        //  the RDP. The major version number is stored in the high two bytes, while the
        //  minor version number is stored in the low two bytes.

        // 0x00080001 RDP 4.0 servers
        // 0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 servers

        // If the server advertises a version number greater than or equal to 0x00080004,
        // it MUST support a maximum length of 512 bytes for the UserName field in the
        // Info Packet (section 2.2.1.11.1.1).

        // clientRequestedProtocols (4 bytes): A 32-bit, unsigned integer that contains
        //  the flags sent by the client in the requestedProtocols field of the RDP
        //  Negotiation Request (section 2.2.1.1.1). In the event that an RDP
        //  Negotiation Request was not received from the client, this field MUST be
        //  initialized to PROTOCOL_RDP (0).

        // Exemple:
        //01 0c 0c 00 -> TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12 bytes
        //04 00 08 00 -> TS_UD_SC_CORE::version = 0x0080004
        //00 00 00 00 -> TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP

        struct SCCore {
            uint16_t userDataType;
            uint16_t length;
            uint32_t version;
            bool option_clientRequestedProtocols;
            uint32_t clientRequestedProtocols;

            SCCore()
            : userDataType(SC_CORE)
            , length(8)
            , version(0x00080001)
            , option_clientRequestedProtocols(false)
            , clientRequestedProtocols(0)
            {
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->version);
                this->emit_optionals(stream);
                stream.mark_end();
            }

            private:
            void emit_optionals(Stream & stream)
            {
                if (this->length < 12){ return; }
                stream.out_uint32_le(this->clientRequestedProtocols);
            }

            public:
            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->version = stream.in_uint32_le();
                if (this->length < 12) { return; }
                this->clientRequestedProtocols = stream.in_uint32_le();
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data SC_CORE (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "sc_core::version [%04x] %s", this->version,
                      (this->version==0x00080001) ? "RDP 4 client"
                     :(this->version==0x00080004) ? "RDP 5.0, 5.1, 5.2, and 6.0 clients)"
                                                  : "Unknown client");
                if (this->length < 12) { return; }
                this->option_clientRequestedProtocols = true;
                LOG(LOG_INFO, "sc_core::clientRequestedProtocols  = %u", this->clientRequestedProtocols);
            }
        };

        // 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
        // -------------------------------------
        // Below relevant quotes from MS-RDPBCGR v20100601 (2.2.1.3.2)

        // header (4 bytes): GCC user data block header, as specified in section
        //                   2.2.1.3.1. The User Data Header type field MUST be
        //                   set to CS_CORE (0xC001).

        // version (4 bytes): A 32-bit, unsigned integer. Client version number
        //                    for the RDP. The major version number is stored in
        //                    the high 2 bytes, while the minor version number
        //                    is stored in the low 2 bytes.
        //
        //         Value Meaning
        //         0x00080001 RDP 4.0 clients
        //         0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 clients

        // desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested
        //                         desktop width in pixels (up to a maximum
        //                         value of 4096 pixels).

        // desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested
        //                         desktop height in pixels (up to a maximum
        //                         value of 2048 pixels).

        // colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
        //                       depth. Values in this field MUST be ignored if
        //                       the postBeta2ColorDepth field is present.
        //          Value Meaning
        //          RNS_UD_COLOR_4BPP 0xCA00 4 bits-per-pixel (bpp)
        //          RNS_UD_COLOR_8BPP 0xCA01 8 bpp

        // SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access
        //                        sequence. This field SHOULD be set to
        //                        RNS_UD_SAS_DEL (0xAA03).

        // keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout
        //                           (active input locale identifier). For a
        //                           list of possible input locales, see
        //                           [MSDN-MUI].

        // clientBuild (4 bytes): A 32-bit, unsigned integer. The build number
        // of the client.

        // clientName (32 bytes): Name of the client computer. This field
        //                        contains up to 15 Unicode characters plus a
        //                        null terminator.

        // keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
        //              Value Meaning
        //              0x00000001 IBM PC/XT or compatible (83-key) keyboard
        //              0x00000002 Olivetti "ICO" (102-key) keyboard
        //              0x00000003 IBM PC/AT (84-key) and similar keyboards
        //              0x00000004 IBM enhanced (101-key or 102-key) keyboard
        //              0x00000005 Nokia 1050 and similar keyboards
        //              0x00000006 Nokia 9140 and similar keyboards
        //              0x00000007 Japanese keyboard

        // keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard
        //                        subtype (an original equipment manufacturer-
        //                        -dependent value).

        // keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number
        //                        of function keys on the keyboard.

        // If the Layout Manager entry points for LayoutMgrGetKeyboardType and
        // LayoutMgrGetKeyboardLayoutName do not exist, the values in certain registry
        // keys are queried and their values are returned instead. The following
        // registry key example shows the registry keys to configure to support RDP.

        // [HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\KEYBD]
        //    "Keyboard Type"=dword:<type>
        //    "Keyboard SubType"=dword:<subtype>
        //    "Keyboard Function Keys"=dword:<function keys>
        //    "Keyboard Layout"="<layout>"

        // To set these values for the desired locale, set the variable DEFINE_KEYBOARD_TYPE
        // in Platform.reg before including Keybd.reg. The following code sample shows
        // how to set the DEFINE_KEYBOARD_TYPE in Platform.reg before including Keybd.reg.

        //    #define DEFINE_KEYBOARD_TYPE
        //    #include "$(DRIVERS_DIR)\keybd\keybd.reg"
        //    This will bring in the proper values for the current LOCALE, if it is
        //    supported. Logic in Keybd.reg sets these values. The following registry
        //    example shows this logic.
        //    ; Define this variable in platform.reg if your keyboard driver does not
        //    ; report its type information.
        //    #if defined DEFINE_KEYBOARD_TYPE

        //    #if $(LOCALE)==0411

        //    ; Japanese keyboard layout
        //        "Keyboard Type"=dword:7
        //        "Keyboard SubType"=dword:2
        //        "Keyboard Function Keys"=dword:c
        //        "Keyboard Layout"="00000411"

        //    #elif $(LOCALE)==0412

        //    ; Korean keyboard layout
        //        "Keyboard Type"=dword:8
        //        "Keyboard SubType"=dword:3
        //        "Keyboard Function Keys"=dword:c
        //        "Keyboard Layout"="00000412"

        //    #else

        //    ; Default to US keyboard layout
        //        "Keyboard Type"=dword:4
        //        "Keyboard SubType"=dword:0
        //        "Keyboard Function Keys"=dword:c
        //        "Keyboard Layout"="00000409"

        //    #endif

        //    #endif ; DEFINE_KEYBOARD_TYPE


        // imeFileName (64 bytes): A 64-byte field. The Input Method Editor
        //                        (IME) file name associated with the input
        //                        locale. This field contains up to 31 Unicode
        //                        characters plus a null terminator.

        // --> Note By CGR How do we know that the following fields are
        //     present of Not ? The only rational method I see is to look
        //     at the length field in the preceding User Data Header
        //     120 bytes without optional data
        //     216 bytes with optional data present

        // postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The
        //                        requested color depth. Values in this field
        //                        MUST be ignored if the highColorDepth field
        //                        is present.
        //       Value Meaning
        //       RNS_UD_COLOR_4BPP 0xCA00        : 4 bits-per-pixel (bpp)
        //       RNS_UD_COLOR_8BPP 0xCA01        : 8 bpp
        //       RNS_UD_COLOR_16BPP_555 0xCA02   : 15-bit 555 RGB mask
        //                                         (5 bits for red, 5 bits for
        //                                         green, and 5 bits for blue)
        //       RNS_UD_COLOR_16BPP_565 0xCA03   : 16-bit 565 RGB mask
        //                                         (5 bits for red, 6 bits for
        //                                         green, and 5 bits for blue)
        //       RNS_UD_COLOR_24BPP 0xCA04       : 24-bit RGB mask
        //                                         (8 bits for red, 8 bits for
        //                                         green, and 8 bits for blue)
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // clientProductId (2 bytes): A 16-bit, unsigned integer. The client
        //                          product ID. This field SHOULD be initialized
        //                          to 1. If this field is present, all of the
        //                          preceding fields MUST also be present. If
        //                          this field is not present, all of the
        //                          subsequent fields MUST NOT be present.

        // serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number.
        //                         This field SHOULD be initialized to 0. If
        //                         this field is present, all of the preceding
        //                         fields MUST also be present. If this field
        //                         is not present, all of the subsequent fields
        //                         MUST NOT be present.

        // highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
        //                         color depth.
        //          Value Meaning
        // HIGH_COLOR_4BPP  0x0004             : 4 bpp
        // HIGH_COLOR_8BPP  0x0008             : 8 bpp
        // HIGH_COLOR_15BPP 0x000F             : 15-bit 555 RGB mask
        //                                       (5 bits for red, 5 bits for
        //                                       green, and 5 bits for blue)
        // HIGH_COLOR_16BPP 0x0010             : 16-bit 565 RGB mask
        //                                       (5 bits for red, 6 bits for
        //                                       green, and 5 bits for blue)
        // HIGH_COLOR_24BPP 0x0018             : 24-bit RGB mask
        //                                       (8 bits for red, 8 bits for
        //                                       green, and 8 bits for blue)
        //
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies
        //                                 the high color depths that the client
        //                                 is capable of supporting.
        //
        //         Flag Meaning
        //   RNS_UD_24BPP_SUPPORT 0x0001       : 24-bit RGB mask
        //                                       (8 bits for red, 8 bits for
        //                                       green, and 8 bits for blue)
        //   RNS_UD_16BPP_SUPPORT 0x0002       : 16-bit 565 RGB mask
        //                                       (5 bits for red, 6 bits for
        //                                       green, and 5 bits for blue)
        //   RNS_UD_15BPP_SUPPORT 0x0004       : 15-bit 555 RGB mask
        //                                       (5 bits for red, 5 bits for
        //                                       green, and 5 bits for blue)
        //   RNS_UD_32BPP_SUPPORT 0x0008       : 32-bit RGB mask
        //                                       (8 bits for the alpha channel,
        //                                       8 bits for red, 8 bits for
        //                                       green, and 8 bits for blue)
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // earlyCapabilityFlags (2 bytes)      : A 16-bit, unsigned integer. It
        //                                       specifies capabilities early in
        //                                       the connection sequence.
        //        Flag                        Meaning
        //  RNS_UD_CS_SUPPORT_ERRINFO_PDU Indicates that the client supports
        //    0x0001                        the Set Error Info PDU
        //                                 (section 2.2.5.1).
        //
        //  RNS_UD_CS_WANT_32BPP_SESSION Indicates that the client is requesting
        //    0x0002                     a session color depth of 32 bpp. This
        //                               flag is necessary because the
        //                               highColorDepth field does not support a
        //                               value of 32. If this flag is set, the
        //                               highColorDepth field SHOULD be set to
        //                               24 to provide an acceptable fallback
        //                               for the scenario where the server does
        //                               not support 32 bpp color.
        //
        //  RNS_UD_CS_SUPPORT_STATUSINFO_PDU  Indicates that the client supports
        //    0x0004                          the Server Status Info PDU
        //                                    (section 2.2.5.2).
        //
        //  RNS_UD_CS_STRONG_ASYMMETRIC_KEYS  Indicates that the client supports
        //    0x0008                          asymmetric keys larger than
        //                                    512 bits for use with the Server
        //                                    Certificate (section 2.2.1.4.3.1)
        //                                    sent in the Server Security Data
        //                                    block (section 2.2.1.4.3).
        //
        //  RNS_UD_CS_VALID_CONNECTION_TYPE Indicates that the connectionType
        //     0x0020                       field contains valid data.
        //
        //  RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU Indicates that the client
        //     0x0040                            supports the Monitor Layout PDU
        //                                       (section 2.2.12.1).
        //
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // clientDigProductId (64 bytes): Contains a value that uniquely
        //                                identifies the client. If this field
        //                                is present, all of the preceding
        //                                fields MUST also be present. If this
        //                                field is not present, all of the
        //                                subsequent fields MUST NOT be present.

        // connectionType (1 byte): An 8-bit unsigned integer. Hints at the type
        //                      of network connection being used by the client.
        //                      This field only contains valid data if the
        //                      RNS_UD_CS_VALID_CONNECTION_TYPE (0x0020) flag
        //                      is present in the earlyCapabilityFlags field.
        //
        //    Value                          Meaning
        //  CONNECTION_TYPE_MODEM 0x01 : Modem (56 Kbps)
        //  CONNECTION_TYPE_BROADBAND_LOW 0x02 : Low-speed broadband
        //                                 (256 Kbps - 2 Mbps)
        //  CONNECTION_TYPE_SATELLITE 0x03 : Satellite
        //                                 (2 Mbps - 16 Mbps with high latency)
        //  CONNECTION_TYPE_BROADBAND_HIGH 0x04 : High-speed broadband
        //                                 (2 Mbps - 10 Mbps)
        //  CONNECTION_TYPE_WAN 0x05 : WAN (10 Mbps or higher with high latency)
        //  CONNECTION_TYPE_LAN 0x06 : LAN (10 Mbps or higher)

        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // pad1octet (1 byte): An 8-bit, unsigned integer. Padding to align the
        //   serverSelectedProtocol field on the correct byte boundary. If this
        //   field is present, all of the preceding fields MUST also be present.
        //   If this field is not present, all of the subsequent fields MUST NOT
        //   be present.

        // serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer that
        //   contains the value returned by the server in the selectedProtocol
        //   field of the RDP Negotiation Response (section 2.2.1.2.1). In the
        //   event that an RDP Negotiation Response was not received from the
        //   server, this field MUST be initialized to PROTOCOL_RDP (0). This
        //   field MUST be present if an RDP Negotiation Request (section
        //   2.2.1.1.1) was sent to the server. If this field is present,
        //   then all of the preceding fields MUST also be present.


        struct CSCore {
            // header
            uint16_t userDataType;
            uint16_t length;
            uint32_t version;
            uint16_t desktopWidth;
            uint16_t desktopHeight;
            uint16_t colorDepth;
            uint16_t SASSequence;
            uint32_t keyboardLayout;
            uint32_t clientBuild;
            uint16_t clientName[16];
            uint32_t keyboardType;
            uint32_t keyboardSubType;
            uint32_t keyboardFunctionKey;
            uint16_t imeFileName[32];
            // optional payload
            uint16_t postBeta2ColorDepth;
            uint16_t clientProductId;
            uint32_t serialNumber;
            uint16_t highColorDepth;
            uint16_t supportedColorDepths;
            uint16_t earlyCapabilityFlags;
            uint8_t  clientDigProductId[64];
            uint8_t  connectionType;
            uint8_t  pad1octet;
            uint32_t serverSelectedProtocol;

            // we do not provide parameters in constructor, 
            // because setting them one field at a time is more explicit and maintainable
            // (drawback: danger is different, not swapping parameters, but we may forget to define some...)
            CSCore()
            : userDataType(CS_CORE)
            , length(216) // default: everything except serverSelectedProtocol
            , version(0x00080001)  // RDP version. 1 == RDP4, 4 == RDP5.
            , colorDepth(0xca01)
            , SASSequence(0xaa03)
            , keyboardLayout(0x040c) // default to French
            , clientBuild(2600)
            // clientName = ""
            , keyboardType(4)
            , keyboardSubType(0)
            , keyboardFunctionKey(12)
            // imeFileName = ""
            , postBeta2ColorDepth(0xca01)
            , clientProductId(1)
            , serialNumber(0)
            , highColorDepth(0)
            , supportedColorDepths(7)
            , earlyCapabilityFlags(1)
            // clientDigProductId = ""
            , connectionType(0)
            , pad1octet(0)
            , serverSelectedProtocol(0)
            {
                bzero(this->clientName, 32);
                bzero(this->imeFileName, 64);
                bzero(this->clientDigProductId, 64);
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->version = stream.in_uint32_le();
                this->desktopWidth = stream.in_uint16_le();
                this->desktopHeight = stream.in_uint16_le();
                this->colorDepth = stream.in_uint16_le();
                this->SASSequence = stream.in_uint16_le();
                this->keyboardLayout =stream.in_uint32_le();
                this->clientBuild = stream.in_uint32_le();
                // utf16 hostname fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.in_utf16(this->clientName, 16);
                this->keyboardType = stream.in_uint32_le();
                this->keyboardSubType = stream.in_uint32_le();
                this->keyboardFunctionKey = stream.in_uint32_le();
                // utf16 fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.in_utf16(this->imeFileName, 32);
                // --------------------- Optional Fields ---------------------------------------
                if (this->length < 134) { return; }
                this->postBeta2ColorDepth = stream.in_uint16_le();
                if (this->length < 136) { return; }
                this->clientProductId = stream.in_uint16_le();
                if (this->length < 140) { return; }
                this->serialNumber = stream.in_uint32_le();
                if (this->length < 142) { return; }
                this->highColorDepth = stream.in_uint16_le();
                if (this->length < 144) { return; }
                this->supportedColorDepths = stream.in_uint16_le();
                if (this->length < 146) { return; }
                this->earlyCapabilityFlags = stream.in_uint16_le();
                if (this->length < 210) { return; }
                stream.in_copy_bytes(this->clientDigProductId, sizeof(this->clientDigProductId));
                if (this->length < 211) { return; }
                this->connectionType = stream.in_uint8();
                if (this->length < 212) { return; }
                this->pad1octet = stream.in_uint8();
                if (this->length < 216) { return; }
                this->serverSelectedProtocol = stream.in_uint32_le();
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->version);
                stream.out_uint16_le(this->desktopWidth);
                stream.out_uint16_le(this->desktopHeight);
                stream.out_uint16_le(this->colorDepth);
                stream.out_uint16_le(this->SASSequence);
                stream.out_uint32_le(this->keyboardLayout);
                stream.out_uint32_le(this->clientBuild);
                // utf16 hostname fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.out_utf16(this->clientName, 16);
                stream.out_uint32_le(this->keyboardType);
                stream.out_uint32_le(this->keyboardSubType);
                stream.out_uint32_le(this->keyboardFunctionKey);
                // utf16 fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.out_utf16(this->imeFileName, 32);

                // --------------------- Optional Fields ---------------------------------------
                this->emit_optional(stream);
                stream.mark_end();
            }
            
            private:
            void emit_optional(Stream & stream)
            {
                if (this->length < 134) { return; }
                stream.out_uint16_le(this->postBeta2ColorDepth);
                if (this->length < 136) { return; }
                stream.out_uint16_le(this->clientProductId);
                if (this->length < 140) { return; }
                stream.out_uint32_le(this->serialNumber);
                if (this->length < 142) { return; }
                stream.out_uint16_le(this->highColorDepth);
                if (this->length < 144) { return; }
                stream.out_uint16_le(this->supportedColorDepths);
                if (this->length < 146) { return; }
                stream.out_uint16_le(this->earlyCapabilityFlags);
                if (this->length < 210) { return; }
                stream.out_copy_bytes(this->clientDigProductId, sizeof(this->clientDigProductId));
                if (this->length < 211) { return; }
                stream.out_uint8(this->connectionType);
                if (this->length < 212) { return; }
                stream.out_uint8(this->pad1octet);
                if (this->length < 216) { return; }
                stream.out_uint32_le(this->serverSelectedProtocol);
            }

            public:
            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data CS_CORE (%u bytes)", msg, this->length);

                if (this->length < 132){
                    LOG(LOG_INFO, "GCC User Data CS_CORE truncated");
                    return;
                }

                LOG(LOG_INFO, "cs_core::version [%04x] %s", this->version,
                      (this->version==0x00080001) ? "RDP 4 client"
                     :(this->version==0x00080004) ? "RDP 5.0, 5.1, 5.2, and 6.0 clients)"
                                                  : "Unknown client");
                LOG(LOG_INFO, "cs_core::desktopWidth  = %u",  this->desktopWidth);
                LOG(LOG_INFO, "cs_core::desktopHeight = %u", this->desktopHeight);
                LOG(LOG_INFO, "cs_core::colorDepth    = [%04x] [%s] superseded by postBeta2ColorDepth", this->colorDepth,
                    (this->colorDepth == 0xCA00) ? "RNS_UD_COLOR_4BPP"
                  : (this->colorDepth == 0xCA01) ? "RNS_UD_COLOR_8BPP"
                                                 : "Unknown");
                LOG(LOG_INFO, "cs_core::SASSequence   = [%04x] [%s]", this->SASSequence,
                    (this->SASSequence == 0xCA00) ? "RNS_UD_SAS_DEL"
                                                  : "Unknown");
                LOG(LOG_INFO, "cs_core::keyboardLayout= %04x",  this->keyboardLayout);
                LOG(LOG_INFO, "cs_core::clientBuild   = %u",  this->clientBuild);
                char hostname[16];
                for (size_t i = 0; i < 16 ; i++) {
                    hostname[i] = (uint8_t)this->clientName[i];
                }
                LOG(LOG_INFO, "cs_core::clientName    = %s",  hostname);
                LOG(LOG_INFO, "cs_core::keyboardType  = [%04x] %s",  this->keyboardType,
                      (this->keyboardType == 0x00000001) ? "IBM PC/XT or compatible (83-key) keyboard"
                    : (this->keyboardType == 0x00000002) ? "Olivetti \"ICO\" (102-key) keyboard"
                    : (this->keyboardType == 0x00000003) ? "IBM PC/AT (84-key) and similar keyboards"
                    : (this->keyboardType == 0x00000004) ? "IBM enhanced (101-key or 102-key) keyboard"
                    : (this->keyboardType == 0x00000005) ? "Nokia 1050 and similar keyboards"
                    : (this->keyboardType == 0x00000006) ? "Nokia 9140 and similar keyboards"
                    : (this->keyboardType == 0x00000007) ? "Japanese keyboard"
                                                         : "Unknown");
                LOG(LOG_INFO, "cs_core::keyboardSubType      = [%04x] OEM code",  this->keyboardSubType);
                LOG(LOG_INFO, "cs_core::keyboardFunctionKey  = %u function keys",  this->keyboardFunctionKey);
                char imename[32];
                for (size_t i = 0; i < 32 ; i++){
                    imename[i] = (uint8_t)this->imeFileName[i];
                }
                LOG(LOG_INFO, "cs_core::imeFileName    = %s",  imename);

                // --------------------- Optional Fields ---------------------------------------
                if (this->length < 134) { return; }
                LOG(LOG_INFO, "cs_core::postBeta2ColorDepth  = [%04x] [%s]", this->postBeta2ColorDepth,
                    (this->postBeta2ColorDepth == 0xCA00) ? "4 bpp"
                  : (this->postBeta2ColorDepth == 0xCA01) ? "8 bpp"
                  : (this->postBeta2ColorDepth == 0xCA02) ? "15-bit 555 RGB mask"
                  : (this->postBeta2ColorDepth == 0xCA03) ? "16-bit 565 RGB mask"
                  : (this->postBeta2ColorDepth == 0xCA04) ? "24-bit RGB mask"
                                                          : "Unknown");
                if (this->length < 136) { return; }
                LOG(LOG_INFO, "cs_core::clientProductId = %u", this->clientProductId);
                if (this->length < 140) { return; }
                LOG(LOG_INFO, "cs_core::serialNumber = %u", this->serialNumber);
                if (this->length < 142) { return; }
                LOG(LOG_INFO, "cs_core::highColorDepth  = [%04x] [%s]", this->highColorDepth,
                    (this->highColorDepth == 4)  ? "4 bpp"
                  : (this->highColorDepth == 8)  ? "8 bpp"
                  : (this->highColorDepth == 15) ? "15-bit 555 RGB mask"
                  : (this->highColorDepth == 16) ? "16-bit 565 RGB mask"
                  : (this->highColorDepth == 24) ? "24-bit RGB mask"
                                                 : "Unknown");
                if (this->length < 144) { return; }
                LOG(LOG_INFO, "cs_core::supportedColorDepths  = [%04x] [%s/%s/%s/%s]", this->supportedColorDepths,
                    (this->supportedColorDepths & 1) ? "24":"",
                    (this->supportedColorDepths & 2) ? "16":"",
                    (this->supportedColorDepths & 4) ? "15":"",
                    (this->supportedColorDepths & 8) ? "32":"");
                if (this->length < 146) { return; }
                LOG(LOG_INFO, "cs_core::earlyCapabilityFlags  = [%04x]", this->earlyCapabilityFlags);
                if (this->earlyCapabilityFlags & 0x0001){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU");
                }
                if (this->earlyCapabilityFlags & 0x0002){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_WANT_32BPP_SESSION");
                }
                if (this->earlyCapabilityFlags & 0x0004){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU");
                }
                if (this->earlyCapabilityFlags & 0x0008){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS");
                }
                if (this->earlyCapabilityFlags & 0x00020){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE");
                }
                if (this->earlyCapabilityFlags & 0x00040){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU");
                }
                if (this->earlyCapabilityFlags & 0xFF10){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:Unknown early capability flag");
                }
                if (this->length < 210) { return; }
                const uint8_t (& cdpid)[64] = this->clientDigProductId;
                LOG(LOG_INFO, "cs_core::clientDigProductId=["
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                cdpid[0x00], cdpid[0x01], cdpid[0x02], cdpid[0x03],
                cdpid[0x04], cdpid[0x05], cdpid[0x06], cdpid[0x07],
                cdpid[0x08], cdpid[0x09], cdpid[0x0A], cdpid[0x0B],
                cdpid[0x0C], cdpid[0x0D], cdpid[0x0E], cdpid[0x0F],

                cdpid[0x10], cdpid[0x11], cdpid[0x12], cdpid[0x13],
                cdpid[0x14], cdpid[0x15], cdpid[0x16], cdpid[0x17],
                cdpid[0x18], cdpid[0x19], cdpid[0x1A], cdpid[0x1B],
                cdpid[0x1C], cdpid[0x1D], cdpid[0x1E], cdpid[0x1F],

                cdpid[0x20], cdpid[0x21], cdpid[0x22], cdpid[0x23],
                cdpid[0x24], cdpid[0x25], cdpid[0x26], cdpid[0x27],
                cdpid[0x28], cdpid[0x29], cdpid[0x2A], cdpid[0x2B],
                cdpid[0x2C], cdpid[0x2D], cdpid[0x2E], cdpid[0x2F],

                cdpid[0x30], cdpid[0x31], cdpid[0x32], cdpid[0x33],
                cdpid[0x34], cdpid[0x35], cdpid[0x36], cdpid[0x37],
                cdpid[0x38], cdpid[0x39], cdpid[0x3A], cdpid[0x3B],
                cdpid[0x3C], cdpid[0x3D], cdpid[0x3E], cdpid[0x3F]
                );
                if (this->length < 211) { return; }
                LOG(LOG_INFO, "cs_core::connectionType  = %u", this->connectionType);
                if (this->length < 212) { return; }
                LOG(LOG_INFO, "cs_core::pad1octet  = %u", this->pad1octet);
                if (this->length < 216) { return; }
                LOG(LOG_INFO, "cs_core::serverSelectedProtocol = %u", this->serverSelectedProtocol);
            }
        };
    };
};

#endif
