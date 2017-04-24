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
  Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong

  Protocol layer for communication with ACL
  Updating context dictionnary from incoming acl traffic
*/


#pragma once
#include <unistd.h>
#include <fcntl.h>

#include <cinttypes>

#include "utils/sugar/exchange.hpp"
#include "utils/stream.hpp"
#include "configs/config.hpp"
#include "core/authid.hpp"
#include "transport/transport.hpp"
#include "utils/translation.hpp"
#include "utils/get_printable_password.hpp"
#include "transport/out_crypto_transport.hpp"
#include "utils/verbose_flags.hpp"
#include "module_manager.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include "utils/log.hpp"

class KeepAlive {
    // Keep alive Variables
    int  grace_delay;
    long timeout;
    long renew_time;
    bool wait_answer;     // true when we are waiting for a positive response
                          // false when positive response has been received and
                          // timers have been set to new timers.
    bool connected;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state = 0x10,
    };

    KeepAlive(std::chrono::seconds grace_delay_, Verbose verbose)
        : grace_delay(grace_delay_.count())
        , timeout(0)
        , renew_time(0)
        , wait_answer(false)
        , connected(false)
        , verbose(verbose)
    {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "KEEP ALIVE CONSTRUCTOR");
        }
    }

    ~KeepAlive() {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "KEEP ALIVE DESTRUCTOR");
        }
    }

    bool is_started() {
        return this->connected;
    }

    void start(time_t now) {
        this->connected = true;
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::start_keep_alive");
        }
        this->timeout    = now + 2 * this->grace_delay;
        this->renew_time = now + this->grace_delay;
    }

    void stop() {
        this->connected = false;
    }

    bool check(time_t now, Inifile & ini) {
        if (this->connected) {
            // LOG(LOG_INFO, "now=%u timeout=%u  renew_time=%u wait_answer=%s grace_delay=%u", now, this->timeout, this->renew_time, this->wait_answer?"Y":"N", this->grace_delay);
            // Keep alive timeout
            if (now > this->timeout) {
                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                // mm.invoke_close_box("Missed keepalive from ACL", signal, now, authentifier);
                return true;
            }

            // LOG(LOG_INFO, "keepalive state ask=%s bool=%s\n",
            //     ini.is_asked<cfg::context::keepalive>()?"Y":"N",
            //     ini.get<cfg::context::keepalive>()?"Y":"N");

            // Keepalive received positive response
            if (this->wait_answer
                && !ini.is_asked<cfg::context::keepalive>()
                && ini.get<cfg::context::keepalive>()) {
                if (bool(this->verbose & Verbose::state)) {
                    LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
                }
                this->timeout    = now + 2*this->grace_delay;
                this->renew_time = now + this->grace_delay;
                this->wait_answer = false;
            }

            // Keep alive asking for an answer from ACL
            if (!this->wait_answer
                && (now > this->renew_time)) {

                this->wait_answer = true;

                ini.ask<cfg::context::keepalive>();
            }
        }
        return false;
    }
};

class Inactivity {
    // Inactivity management
    // let t be the timeout of the blocking select in session loop,
    // the effective inactivity timeout detection will be between
    // inactivity_timeout and inactivity_timeout + t.
    // hence we should have t << inactivity_timeout.
    time_t inactivity_timeout;
    time_t last_activity_time;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state = 0x10,
    };

    Inactivity(std::chrono::seconds timeout, time_t start, Verbose verbose)
    : inactivity_timeout(std::max<time_t>(timeout.count(), 30))
    , last_activity_time(start)
    , verbose(verbose)
    {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "INACTIVITY CONSTRUCTOR");
        }
    }

    ~Inactivity() {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "INACTIVITY DESTRUCTOR");
        }
    }

    bool check_user_activity(time_t now, bool & has_user_activity) {
        if (!has_user_activity) {
            if (now > this->last_activity_time + this->inactivity_timeout) {
                LOG(LOG_INFO, "Session User inactivity : closing");
                // mm.invoke_close_box("Connection closed on inactivity", signal, now, authentifier);
                return true;
            }
        }
        else {
            has_user_activity = false;
            this->last_activity_time = now;
        }
        return false;
    }
};

class AclSerializer{
    enum {
        HEADER_SIZE = 4
    };

public:
    Inifile & ini;

private:
    Transport & auth_trans;
    char session_id[256];
    CryptoContext cctx;
    OutCryptoTransport ct;

public:
    std::string session_type;

    bool remote_answer;       // false initialy, set to true once response is
                              // received from acl and asked_remote_answer is
                              // set to false

    KeepAlive keepalive;

    Inactivity inactivity;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        variable = 0x2,
        buffer   = 0x40,
        state    = 0x10,
    };

public:
    AclSerializer(Inifile & ini, time_t acl_start_time, Transport & auth_trans, CryptoContext cctx, Random & rnd, Verbose verbose)
        : ini(ini)
        , auth_trans(auth_trans)
        , session_id{}
        , cctx(cctx)
        , ct(ini.get<cfg::crypto::session_log_with_encryption>(), ini.get<cfg::crypto::session_log_with_checksum>(), cctx, rnd)
        , remote_answer(false)
        , keepalive(ini.get<cfg::globals::keepalive_grace_delay>(), to_verbose_flags(ini.get<cfg::debug::auth>()))
        , inactivity(ini.get<cfg::globals::session_timeout>(), acl_start_time, to_verbose_flags(ini.get<cfg::debug::auth>()))
        , verbose(verbose)
    {
        std::snprintf(this->session_id, sizeof(this->session_id), "%d", getpid());
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::AclSerializer");
        }
    }

    ~AclSerializer()
    {
        this->auth_trans.disconnect();
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::~AclSerializer");
        }
        char session_file[256];
        std::snprintf(session_file, sizeof(session_file),
                      "%s/redemption/session_%s.pid", PID_PATH, this->session_id);
        unlink(session_file);
    }

    void report(const char * reason, const char * message) {
        this->ini.ask<cfg::context::keepalive>();
        char report[1024];
        snprintf(report, sizeof(report), "%s:%s:%s", reason,
            this->ini.get<cfg::globals::target_device>().c_str(), message);
        this->ini.set_acl<cfg::context::reporting>(report);
        this->send_acl_data();
    }

    void set_auth_error_message(const char * error_message) {
        this->ini.set<cfg::context::auth_error_message>(error_message);
    }

    void receive() {
        try {
            this->incoming();

            if (!this->ini.get<cfg::context::module>().compare("RDP")
            ||  !this->ini.get<cfg::context::module>().compare("VNC")) {
                this->session_type = this->ini.get<cfg::context::module>();
            }
            this->remote_answer = true;
        } catch (...) {
            // acl connection lost
            this->ini.set_acl<cfg::context::authenticated>(false);

            if (this->ini.get<cfg::context::manager_disconnect_reason>().empty()) {
                this->ini.set_acl<cfg::context::rejected>(
                    TR(trkeys::manager_close_cnx, language(this->ini)));
            }
            else {
                this->ini.set_acl<cfg::context::rejected>(
                    this->ini.get<cfg::context::manager_disconnect_reason>());
                this->ini.get_ref<cfg::context::manager_disconnect_reason>().clear();
            }
        }
    }

    void log4(bool duplicate_with_pid, const char * type, const char * extra) { 
        const bool session_log =
            this->ini.get<cfg::session_log::enable_session_log>();
        if (!duplicate_with_pid && !session_log) return;

        /* Log to file */
        const bool log_redir = this->ini.get<cfg::session_log::session_log_redirection>();
        if (log_redir) {

            if(!ct.is_open()) {
                char tmpname[128];
                try {
                    std::string const & filename = this->ini.get<cfg::session_log::log_path>();
                    ct.open(filename.c_str(), 0);
                    ::strcpy(tmpname, ct.get_tmp());
                }
                catch (Error e) {
                    LOG(LOG_INFO, "exception raised %d", e.id);
                };
            }
            
            std::time_t t = std::time(nullptr);
            char mbstr[100];
            if (std::strftime(mbstr, sizeof(mbstr), "%F %T ", std::localtime(&t))) {
                ct.send(mbstr, strlen(mbstr));
            }

            ct.send("type=\"", 6);
            ct.send(type, strlen(type));
            if(extra) {
                ct.send(" ", 1);
                ct.send(extra, strlen(extra));
            }
            ct.send("\n", 1);
        }

        /* Log to syslog */
        if (duplicate_with_pid) {
            LOG(LOG_INFO, "type='%s'%s%s", type, (extra ? " " : ""), (extra ? extra : ""));
        }
        if (session_log) {
            LOG__REDEMPTION__INTERNAL__IMPL(
                LOG_INFO
              , "[%s Session] "
                "type='%s' "
                "session_id='%s' "
                "client_ip='%s' "
                "target_ip='%s' "
                "user='%s' "
                "device='%s' "
                "service='%s' "
                "account='%s'"
                "%s%s"
              , (this->session_type.empty() ? "Neutral" : this->session_type.c_str())
              , type
              , this->ini.get<cfg::context::session_id>().c_str()
              , this->ini.get<cfg::globals::host>().c_str()
              , (isdigit(this->ini.get<cfg::context::target_host>()[0]) ?
                  this->ini.get<cfg::context::target_host>().c_str() :
                  this->ini.get<cfg::context::ip_target>().c_str())
              , this->ini.get<cfg::globals::auth_user>().c_str()
              , this->ini.get<cfg::globals::target_device>().c_str()
              , this->ini.get<cfg::context::target_service>().c_str()
              , this->ini.get<cfg::globals::target_user>().c_str()
              , (extra ? " " : ""), (extra ? extra : "")
            );
        }
    }

    bool check(auth_api & authentifier, MMApi & mm, time_t now, BackEvent_t & signal, BackEvent_t & front_signal, bool & has_user_activity) {
        //LOG(LOG_INFO, "================> ACL check: now=%u, signal=%u",
        //    (unsigned)now, static_cast<unsigned>(signal));
        if (signal == BACK_EVENT_STOP) {
            // here, mm.last_module should be false only when we are in login box
            mm.mod->get_event().reset();
            return false;
        }

        if (mm.last_module) {
            // at a close box (mm.last_module is true),
            // we are only waiting for a stop signal
            // and Authentifier should not exist anymore.
            return true;
        }

        const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
        if (enddate != 0 && (static_cast<uint32_t>(now) > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            const char * message = TR(trkeys::session_out_time, language(this->ini));
            mm.invoke_close_box(message, signal, now, authentifier);

            return true;
        }

        // Close by rejeted message received
        if (!this->ini.get<cfg::context::rejected>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(this->ini.get<cfg::context::rejected>());
//            const char * error_message = this->ini.get<cfg::context::rejected>();
//            this->set_auth_error_message(error_message);
            LOG(LOG_INFO, "Close by Rejected message received : %s",
                this->ini.get<cfg::context::rejected>());
            this->ini.set_acl<cfg::context::rejected>("");
            mm.invoke_close_box(nullptr, signal, now, authentifier);
            return true;
        }

        // Keep Alive
        if (this->keepalive.check(now, this->ini)) {
            mm.invoke_close_box(TR(trkeys::miss_keepalive, language(this->ini)), signal, now, authentifier);
            return true;
        }

        // Inactivity management
        if (this->inactivity.check_user_activity(now, has_user_activity)) {
            mm.invoke_close_box(TR(trkeys::close_inactivity, language(this->ini)), signal, now, authentifier);
            return true;
        }

        // Manage module (refresh or next)
        if (this->ini.changed_field_size()) {
            if (mm.connected) {
                // send message to acl with changed values when connected to
                // a module (rdp, vnc, xup ...) and something changed.
                // used for authchannel and keepalive.
                this->send_acl_data();
            }
            else if (signal == BACK_EVENT_REFRESH || signal == BACK_EVENT_NEXT) {
                this->remote_answer = false;
                this->send_acl_data();
            }
        }
        else if (this->remote_answer
        || (signal == BACK_EVENT_RETRY_CURRENT)
        || (front_signal == BACK_EVENT_NEXT)) {
            this->remote_answer = false;
            if (signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
                // TODO signal management (refresh/next) should go to ModuleManager, it's basically the same behavior. It could be implemented by closing module then opening another one of the same kind
                mm.mod->refresh_context(this->ini);
                mm.mod->get_event().signal = BACK_EVENT_NONE;
                mm.mod->get_event().set();
            }
            else if ((signal == BACK_EVENT_NEXT)
                    || (signal == BACK_EVENT_RETRY_CURRENT)
                    || (front_signal == BACK_EVENT_NEXT)) {
                if ((signal == BACK_EVENT_NEXT)
                   || (front_signal == BACK_EVENT_NEXT)) {
                    LOG(LOG_INFO, "===========> MODULE_NEXT");
                }
                else {
                    REDASSERT(signal == BACK_EVENT_RETRY_CURRENT);

                    LOG(LOG_INFO, "===========> MODULE_RETRY_CURRENT");
                }

                int next_state = (((signal == BACK_EVENT_NEXT)
                                  || (front_signal == BACK_EVENT_NEXT)) ? mm.next_module() : MODULE_RDP);

                front_signal = BACK_EVENT_NONE;

                if (next_state == MODULE_TRANSITORY) {
                    this->remote_answer = false;

                    return true;
                }

                signal = BACK_EVENT_NONE;
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    mm.invoke_close_box(nullptr, signal, now, authentifier);
                    return true;
                }
                if (next_state == MODULE_INTERNAL_CLOSE_BACK) {
                    this->keepalive.stop();
                }
                if (mm.mod) {
                    mm.mod->disconnect(now);
                }
                mm.remove_mod();
                try {
                    mm.new_mod(next_state, now, authentifier);
                }
                catch (Error & e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        this->ini.set_acl<cfg::context::module>(STRMODULE_TRANSITORY);

                        signal = BACK_EVENT_NEXT;

                        this->remote_answer = false;

                        this->report("CONNECTION_FAILED",
                            "Failed to connect to remote TCP host.");

                        return true;
                    }
                    else if (e.id == ERR_RDP_SERVER_REDIR) {
                        // SET new target in ini
                        const char * host = char_ptr_cast(this->ini.get<cfg::mod_rdp::redir_info>().host);
                        const char * password = char_ptr_cast(this->ini.get<cfg::mod_rdp::redir_info>().password);
                        const char * username = char_ptr_cast(this->ini.get<cfg::mod_rdp::redir_info>().username);
                        const char * change_user = "";
                        if (this->ini.get<cfg::mod_rdp::redir_info>().dont_store_username
                        && (username[0] != 0)) {
                            LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                            this->ini.set_acl<cfg::globals::target_user>(username);
                            change_user = username;
                        }
                        if (password[0] != 0) {
                            LOG(LOG_INFO, "SrvRedir: Change target password");
                            this->ini.set_acl<cfg::context::target_password>(password);
                        }
                        LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
                        this->ini.set_acl<cfg::context::target_host>(host);
                        char message[768] = {};
                        sprintf(message, "%s@%s", change_user, host);
                        this->report("SERVER_REDIRECTION", message);
                        this->remote_answer = true;
                        signal = BACK_EVENT_NEXT;
                        return true;
                    }
                    else {
                        throw;
                    }
                }
                if (!this->keepalive.is_started() && mm.connected) {
                    this->keepalive.start(now);
                }
            }
            else
            {
                if (!this->ini.get<cfg::context::disconnect_reason>().empty()) {
                    this->ini.set<cfg::context::manager_disconnect_reason>(
                        this->ini.get<cfg::context::disconnect_reason>());
                    this->ini.get_ref<cfg::context::disconnect_reason>().clear();

                    this->ini.set_acl<cfg::context::disconnect_reason_ack>(true);
                }
            }
        }

        // LOG(LOG_INFO, "connect=%s check=%s", this->connected?"Y":"N", check()?"Y":"N");

        // AuthCHANNEL CHECK
        // if an answer has been received, send it to
        // rdp serveur via mod (should be rdp module)
        // TODO Check if this->mod is RDP MODULE
        if (mm.connected && this->ini.get<cfg::mod_rdp::auth_channel>()[0]) {
            // Get sesman answer to AUTHCHANNEL_TARGET
            if (!this->ini.get<cfg::context::auth_channel_answer>().empty()) {
                // If set, transmit to auth_channel channel
                mm.mod->send_auth_channel_data(this->ini.get<cfg::context::auth_channel_answer>().c_str());
                // Erase the context variable
                this->ini.get_ref<cfg::context::auth_channel_answer>().clear();
            }
        }
        return true;
    }

private:
    class Reader
    {
        static constexpr size_t buf_len = 65535;
        char buf[buf_len];
        char key_name_buf[64];
        bool has_next_buffer = true;
        std::string data_multipacket;
        char * p;
        char * e;

        Transport & trans;
        const Verbose verbose;

    public:
        Reader(Transport & trans, Verbose verbose)
        : trans(trans)
        , verbose(verbose)
        {
            this->safe_read_packet();
        }

        char const * key(bool always_internal_copy) {
            auto m = std::find(this->p, this->e, '\n');
            if (m == e) {
                size_t key_buf_len = this->e - this->p;
                if (key_buf_len) {
                    if (key_buf_len > sizeof(this->key_name_buf)) {
                        LOG(LOG_ERR, "Error: ACL key length too big (got %zu max 64o)", key_buf_len);
                        throw Error(ERR_ACL_MESSAGE_TOO_BIG);
                    }
                    memcpy(this->key_name_buf, this->p, key_buf_len);
                }
                else if (!this->has_next_buffer) {
                    if (key_buf_len) {
                        throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
                    }
                    return nullptr;
                }
                this->safe_read_packet();
                m = std::find(this->p, this->e, '\n');
                if (key_buf_len) {
                    if (size_t(m - this->p) > sizeof(this->key_name_buf)) {
                        LOG(LOG_ERR, "Error: ACL key length too big (got %" PRIdPTR " max 64o)", m - this->p);
                        throw Error(ERR_ACL_MESSAGE_TOO_BIG);
                    }
                    *m = 0;
                    ++m;
                    memcpy(this->key_name_buf, this->p, m - this->p);
                    this->p = m;
                    return reinterpret_cast<char const *>(this->key_name_buf);
                }
            }
            if (always_internal_copy) {
                *m = 0;
                ++m;
                memcpy(this->key_name_buf, this->p, m - this->p);
                this->p = m;
                return this->key_name_buf;
            }
            *m = 0;
            return exchange(this->p, m+1);
        }

        bool is_set_value() {
            if (this->p == this->e) {
                this->read_packet();
            }
            return *this->p == '!';
        }

        bool consume_ask() {
            char c = this->getc();
            if (!('a' == c || 'A' == c)) {
                return false;
            }
            c = this->getc();
            if (!('s' == c || 'S' == c)) {
                return false;
            }
            c = this->getc();
            if (!('k' == c || 'K' == c)) {
                return false;
            }
            return this->getc() == '\n';
        }

        array_view_const_char get_val() {
            if (this->p == this->e) {
                this->read_packet();
            }
            else if (*this->p == '!') {
                ++this->p;
            }
            auto m = std::find(this->p, this->e, '\n');
            if (m != this->e) {
                *m = 0;
                std::size_t const sz = m - this->p;
                return {exchange(this->p, m+1), sz};
            }
            data_multipacket.clear();
            do {
                data_multipacket.insert(data_multipacket.end(), this->p, this->e);
                if (data_multipacket.size() > 1024*1024) {
                    LOG(LOG_ERR, "Error: ACL data too big (got %zu max 1M)", data_multipacket.size());
                    throw Error(ERR_ACL_MESSAGE_TOO_BIG);
                }
                this->read_packet();
                m = std::find(this->p, this->e, '\n');
            } while (m == e);
            data_multipacket.insert(data_multipacket.end(), this->p, m);
            this->p = m + 1;
            return {data_multipacket.data(), data_multipacket.size()};
        }

        void hexdump() const {
            ::hexdump(this->buf, this->e - this->buf);
        }

    private:
        char getc() {
            if (this->p == this->e) {
                this->read_packet();
            }
            char c = *this->p;
            ++this->p;
            return c;
        }

        void read_packet() {
            if (!this->has_next_buffer) {
                throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
            }
            this->safe_read_packet();
        }

        void safe_read_packet() {
            uint16_t buf_sz = 0;
            do {
                if (!this->trans.atomic_read(this->buf, HEADER_SIZE)){
                    throw Error(ERR_TRANSPORT_NO_MORE_DATA);
                }

                InStream in_stream(this->buf, 4);
                this->has_next_buffer = in_stream.in_uint16_be();
                buf_sz = in_stream.in_uint16_be();
            } while (buf_sz == 0 && this->has_next_buffer);

            this->p = this->buf;
            this->e = this->buf;
            if (!this->trans.atomic_read(e, buf_sz)){
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            }
            e += buf_sz;

            if (bool(this->verbose & Verbose::buffer)) {
                if (this->has_next_buffer){
                    LOG(LOG_INFO, "ACL SERIALIZER : multi buffer (receive)");
                }
                LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (receive) = %" PRIdPTR, this->e - this->p);
            }
        }
    };

public:
    void in_items()
    {
        Reader reader(this->auth_trans, this->verbose);

        while (auto key = reader.key(bool(this->verbose & Verbose::variable))) {
            auto authid = authid_from_string(key);
            if (auto field = this->ini.get_acl_field(authid)) {
                if (reader.is_set_value()) {
                    if (field.set(reader.get_val()) && bool(this->verbose & Verbose::variable)) {
                        const char * val         = field.c_str();
                        const char * display_val = val;
                        if (cfg::crypto::key0::index() == authid ||
                            cfg::crypto::key1::index() == authid ||
                            cfg::context::password::index() == authid ||
                            cfg::context::target_password::index() == authid ||
                            cfg::globals::target_application_password::index() == authid ||
                            (cfg::context::auth_channel_answer::index() == authid &&
                             strcasestr(val, "password") != nullptr)
                        ) {
                            display_val = ::get_printable_password(val, this->ini.get<cfg::debug::password>());
                        }
                        LOG(LOG_INFO, "receiving '%s'='%s'", key, display_val);
                    }
                }
                else if (reader.consume_ask()) {
                    field.ask();
                    if (bool(this->verbose & Verbose::variable)) {
                        LOG(LOG_INFO, "receiving ASK '%s'", key);
                    }
                }
                else {
                    reader.hexdump();
                    throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
                }
            }
            else {
                LOG(LOG_WARNING, "Unexpected receving '%s'", key);
            }
        }
    }

    void incoming()
    {
        bool flag = this->ini.get<cfg::context::session_id>().empty();
        this->in_items();
        if (flag && !this->ini.get<cfg::context::session_id>().empty()) {
            char old_session_file[256];
            std::snprintf(old_session_file, sizeof(old_session_file),
                          "%s/redemption/session_%s.pid", PID_PATH, this->session_id);
            char new_session_file[256];
            std::snprintf(new_session_file, sizeof(new_session_file),
                         "%s/redemption/session_%s.pid", PID_PATH,
                    this->ini.get<cfg::context::session_id>().c_str());
            std::rename(old_session_file, new_session_file);
            std::snprintf(this->session_id, sizeof(this->session_id), "%s",
                          this->ini.get<cfg::context::session_id>().c_str());
        }
        if (bool(this->verbose & Verbose::buffer)){
            LOG(LOG_INFO, "SESSION_ID = %s", this->ini.get<cfg::context::session_id>());
        }
    }

private:
    class Buffers
    {
        static constexpr uint16_t buf_len = 65535;

        struct Buffer
        {
            char data[buf_len];
            uint16_t flags = 0;
            uint16_t sz = HEADER_SIZE; // packet size
        };

        Buffer buf;
        Transport & trans;
        const Verbose verbose;

    public:
        Buffers(Transport & trans, Verbose verbose)
        : trans(trans)
        , verbose(verbose)
        {}

        void push(char c) {
            if (this->buf.sz == buf_len) {
                this->new_buffer();
            }
            this->buf.data[this->buf.sz++] = c;
        }

        void push(char const * s) {
            while (*s) {
                while (this->buf.sz != buf_len && *s) {
                    this->buf.data[this->buf.sz++] = *s;
                    ++s;
                }
                if (*s) {
                    this->new_buffer();
                }
            }
        }

        void send_buffer() {
            if (bool(this->verbose & Verbose::buffer)){
                LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (send) %d", this->buf.sz - HEADER_SIZE);
            }
            OutStream stream(this->buf.data, HEADER_SIZE);
            stream.out_uint16_be(this->buf.flags);
            stream.out_uint16_be(this->buf.sz - HEADER_SIZE);
            this->trans.send(this->buf.data, this->buf.sz);
            this->buf.flags = 0u;
            this->buf.sz = HEADER_SIZE;
        }

    private:
        enum { MULTIBUF = 1 };
        void new_buffer() {
            if (bool(this->verbose & Verbose::buffer)){
                LOG(LOG_INFO, "ACL SERIALIZER : multi buffer (send)");
            }
            this->buf.flags |= MULTIBUF;
            this->send_buffer();
        }
    };

public:
    void send_acl_data() {
        if (bool(this->verbose & Verbose::variable)){
            LOG(LOG_INFO, "Begin Sending data to ACL: numbers of changed fields = %zu", this->ini.changed_field_size());
        }
        if (this->ini.changed_field_size()) {
            auto const password_printing_mode = this->ini.get<cfg::debug::password>();

            try {
                Buffers buffers(this->auth_trans, this->verbose);

                for (auto && field : this->ini.get_fields_changed()) {
                    char const * key = string_from_authid(field.authid());
                    buffers.push(key);
                    buffers.push('\n');
                    if (field.is_asked()) {
                        buffers.push("ASK\n");
                        if (bool(this->verbose & Verbose::variable)) {
                            LOG(LOG_INFO, "sending %s=ASK", key);
                        }
                    }
                    else {
                        char const * val = field.c_str();
                        buffers.push('!');
                        buffers.push(val);
                        buffers.push('\n');
                        const char * display_val = val;
                        if ((strncasecmp("password", key, 8) == 0)
                         || (strncasecmp("target_password", key, 15) == 0)) {
                            display_val = get_printable_password(val, password_printing_mode);
                        }
                        if (bool(this->verbose & Verbose::variable)) {
                            LOG(LOG_INFO, "sending %s=%s", key, display_val);
                        }
                    }
                }

                buffers.send_buffer();
            }
            catch (Error const &) {
                this->ini.set_acl<cfg::context::authenticated>(false);
                this->ini.set_acl<cfg::context::rejected>(TR(trkeys::acl_fail, language(this->ini)));
                // this->ini.context.rejected.set_from_cstr("Authentifier service failed");
            }

            this->ini.clear_send_index();
        }
    }
};
