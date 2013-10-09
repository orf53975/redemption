/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP Control object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityControl
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityControlEmit)
{
    ControlCaps control_caps;
    control_caps.controlFlags = 0;
    control_caps.remoteDetachFlag = 1;
    control_caps.controlInterest = 2;
    control_caps.detachInterest = 3;

    BOOST_CHECK_EQUAL(control_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_CONTROL));
    BOOST_CHECK_EQUAL(control_caps.len, static_cast<uint16_t>(CAPLEN_CONTROL));
    BOOST_CHECK_EQUAL(control_caps.controlFlags, (uint16_t) 0);
    BOOST_CHECK_EQUAL(control_caps.remoteDetachFlag, (uint16_t) 1);
    BOOST_CHECK_EQUAL(control_caps.controlInterest, (uint16_t) 2);
    BOOST_CHECK_EQUAL(control_caps.detachInterest, (uint16_t) 3);

    BStream stream(1024);
    control_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.get_data();

    ControlCaps control_caps2;

    BOOST_CHECK_EQUAL(control_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_CONTROL));
    BOOST_CHECK_EQUAL(control_caps2.len, static_cast<uint16_t>(CAPLEN_CONTROL));

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_CONTROL, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_CONTROL, stream.in_uint16_le());

    control_caps2.recv(stream, CAPLEN_CONTROL);

    BOOST_CHECK_EQUAL(control_caps2.controlFlags, (uint16_t) 0);
    BOOST_CHECK_EQUAL(control_caps2.remoteDetachFlag, (uint16_t) 1);
    BOOST_CHECK_EQUAL(control_caps2.controlInterest, (uint16_t) 2);
    BOOST_CHECK_EQUAL(control_caps2.detachInterest, (uint16_t) 3);
}
