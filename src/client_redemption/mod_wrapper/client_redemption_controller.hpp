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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "utils/log.hpp"

#ifndef Q_MOC_RUN

#include "keyboard/keymap2.hpp"

#include "client_redemption/client_redemption_api.hpp"
// # include "client_redemption/client_config/client_redemption_config.hpp

#endif



class ClientModController /*: public ClientRedemptionAPI*/
{
private:
    Keymap2           keymap;
    StaticOutStream<256> decoded_data;    // currently not initialised
    int                  _timer;

    mod_api            * mod = nullptr;
    ClientRedemptionAPI * client;

public:
    struct MouseData {
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;

    ClientModController(ClientRedemptionAPI * client)
//       : ClientRedemptionAPI(/*session_reactor, argv, argc, verbose*/)
  :  _timer(0)
  , client(client)
    {
        //this->keymap.init_layout(LICD);
    }

    void replay(const std::string & movie_name, const std::string & movie_dir) {
        this->client->replay(movie_name, movie_dir);
    }

    timeval reload_replay_mod(int begin, timeval now_stop) {
        return this->client->reload_replay_mod(begin, now_stop);
    }

    void replay_set_sync() {
        this->client->replay_set_sync();
    }

    bool is_replay_on() {
        return this->client->is_replay_on();
    }

    bool load_replay_mod(std::string const & movie_dir, std::string const & movie_name, timeval time_1, timeval time_2) {
        return this->load_replay_mod(movie_dir, movie_name, time_1, time_2);
    }

    void delete_replay_mod() {
        this->delete_replay_mod();
    }

    void closeFromScreen() {
        this->client->closeFromScreen();
    }

    time_t get_real_time_movie_begin() {
        return this->client->get_real_time_movie_begin();
    }

    void replay_set_pause(timeval time) {
        this->client->replay_set_pause(time);
    }

    time_t get_movie_time_length(char const * movie_path) {
        return this->client->get_movie_time_length(movie_path);
    }

    char const * get_mwrm_filename() {
        return this->client->get_mwrm_filename();
    }

//     std::vector<IconMovieData> get_icon_movie_data() {
//         this->client->get_icon_movie_data();
//     }



    void set_mod(mod_api * mod) {
        this->mod = mod;
    }

    void init_layout(int lcid) {
        this->keymap.init_layout(lcid);
    }

    bool connect() {
        return this->client->connect();
    }

    void disconnexionReleased() {
        this->client->disconnexionReleased();
    }

    void refreshPressed() {
//         if (this->mod != nullptr) {
//             Rect rect(0, 0, this->config.info.width, this->config.info.height);
//             this->mod->rdp_input_invalidate(rect);
//         }
    }

    void CtrlAltDelPressed() {
        int flag = Keymap2::KBDFLAGS_EXTENDED;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void CtrlAltDelReleased() {
        int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void mouseButtonEvent(int x, int y, int flag) {
        if (this->mod != nullptr) {
            this->mod->rdp_input_mouse(flag, x, y, &(this->keymap));
        }
    }

    void wheelEvent(int  /*unused*/,  int  /*unused*/, int /*delta*/) {
        // int flag(MOUSE_FLAG_HWHEEL);
        // if (delta < 0) {
        //     flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
        // }
        // if (this->mod != nullptr) {
        //     this->mod->rdp_input_mouse(flag, e->x(), e->y(), &(this->keymap));
        // }
    }

    bool mouseMouveEvent(int x, int y) {

        if (this->mod != nullptr /*&& y < this->config.info.height*/) {
            this->mouse_data.x = x;
            this->mouse_data.y = y;
            this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, this->mouse_data.x, this->mouse_data.y, &(this->keymap));
        }

        return false;
    }

    void send_rdp_scanCode(int keyCode, int flag) {
        bool tsk_switch_shortcuts = false;
        Keymap2::DecodedKeys decoded_keys = this->keymap.event(flag, keyCode, tsk_switch_shortcuts);
        switch (decoded_keys.count)
        {
        case 2:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[1]);
            }
            break;
        case 1:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            break;
        default:
        case 0:
            break;
        }
        if (this->mod != nullptr) {
            this->mod->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->keymap));
        }
    }

    void send_rdp_unicode(uint16_t unicode, uint16_t flag) {
        this->mod->rdp_input_unicode(unicode, flag);
    }

};