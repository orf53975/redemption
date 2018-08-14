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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Clément Moroldo

*/

#define RED_TEST_MODULE Testiametrics
#include "system/redemption_unit_tests.hpp"

#include "utils/fileutils.hpp"
#include "test_only/get_file_contents.hpp"

#include "utils/log.hpp"

#include "main/iametrics.hpp"

RED_AUTO_TEST_CASE(Testiametrics)
{
    RED_CHECK_EQ(iametrics_version(), "1.3.5");
}


RED_AUTO_TEST_CASE(TestRDPMetricsH)
{
    uint8_t key[32] = {0};
    const char * data = "primaryuser";
    char sig[64];

    hmac_sha256(sig, data, strlen(data), key);

    std::string res(sig, 64);

    RED_CHECK_EQUAL(std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"),
    res);
}

constexpr const char * rdp_metrics_path_file = "/tmp";
constexpr const char * fields_rdp_metrics_version = "v1.0";
constexpr const char * protocol_name = "rdp";

RED_AUTO_TEST_CASE(TestRDPMetricsConstructor)
{
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logindex");

    // Should create rdp_metrics files if they do not exist
    time_t epoch = 1533211681; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                      , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                      , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                      , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                      , epoch
                      , 24
                      , 5
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));

    metrics_rotate(epoch + (3600*1), metrics);
    metrics_rotate(epoch + (3600*2), metrics);
    metrics_rotate(epoch + (3600*3), metrics);
    metrics_rotate(epoch + (3600*4), metrics);
    metrics_rotate(epoch + (3600*5), metrics);
    metrics_rotate(epoch + (3600*6), metrics);

    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics"));
    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logindex"));

    metrics_rotate(epoch + (3600*24), metrics);

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logindex"));

    metrics_delete(metrics);

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logindex");
}

RED_AUTO_TEST_CASE(TestRDPMetricsConstructorHoursRotation)
{
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex");

    // Should create rdp_metrics files if they do not exist
    time_t epoch = 0; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                      , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                      , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                      , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                      , epoch
                      , 7
                      , 5
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01.logindex"));

    metrics_rotate(epoch + (3600*1), metrics);
    metrics_rotate(epoch + (3600*2), metrics);
    metrics_rotate(epoch + (3600*3), metrics);
    metrics_rotate(epoch + (3600*4), metrics);
    metrics_rotate(epoch + (3600*5), metrics);
    metrics_rotate(epoch + (3600*6), metrics);
    metrics_rotate(epoch + (3600*6)+3599, metrics);

    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics"));
    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"));

    metrics_rotate(epoch + (3600*7), metrics);

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"));

    metrics_rotate(epoch + (24*3600*3), metrics);

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex"));

    metrics_delete(metrics);

    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex");
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle1) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    time_t epoch = 1533211681;
    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, true
                    , rdp_metrics_path_file
                    , "164d89c1a56957b752540093e178"
                    , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                    , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                    , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                    , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                    , epoch
                    , 24
                    , 5
                    );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));

    std::string expected_log_index("2018-08-02 12:08:01 connection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));

    metrics_delete(metrics);

    std::string expected_disconnected_index("2018-08-02 12:08:06 disconnection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index+expected_disconnected_index);
    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}
