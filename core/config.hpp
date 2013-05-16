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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Raphael Zhou

   configuration file,
   parsing config file rdpproxy.ini
*/

#ifndef _REDEMPTION_CORE_CONFIG_HPP_
#define _REDEMPTION_CORE_CONFIG_HPP_

#include <dirent.h>
#include <stdio.h>

#include "log.hpp"

#include <istream>
#include <string>
#include <stdint.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <string.hpp>

using namespace std;

/*
static inline bool bool_from_string(string str)
{
    return (boost::iequals(string("1"),str))
        || (boost::iequals(string("yes"),str))
        || (boost::iequals(string("on"),str))
        || (boost::iequals(string("true"),str));
}
*/

static inline bool bool_from_cstr(const char * str)
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

static inline unsigned level_from_cstr(const char * str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
    if (0 == strcasecmp("medium", str)) { res = 1; }
    else if (0 == strcasecmp("high", str)) { res = 2; }
    return res;
}

static inline unsigned logtype_from_cstr(const char * str)
{ // null = 0, print = 1, syslog = 2, file = 3, encryptedfile = 4
    unsigned res = 0;
         if (0 == strcasecmp("print",         str)) { res = 1; }
    else if (0 == strcasecmp("syslog",        str)) { res = 2; }
    else if (0 == strcasecmp("file",          str)) { res = 3; }
    else if (0 == strcasecmp("encryptedfile", str)) { res = 4; }
    return res;
}

static inline unsigned ulong_from_cstr(const char * str)
{ // 10 = 10, 0x10 = 16
    if ((*str == '0') && (*(str + 1) == 'x')){
        return strtol(str + 2, 0, 16);
    }

    return atol(str);
}

static inline signed _long_from_cstr(const char * str)
{
    return atol(str);
}

static inline bool check_name(const char * str)
{
    return ((strlen(str) > 0) && (strlen(str) < 250));
}

static inline bool check_ask(const char * str)
{
    return (0 == strcmp(str, "ask"));
}

static inline void ask_string(const char * str, char buffer[], bool & flag)
{
    flag = check_ask(str);
    if (!flag){
        strncpy(buffer, str, strlen(str));
        buffer[strlen(str)] = 0;
    }
    else {
        buffer[0] = 0;
    }
}

struct IniAccounts {
    char accountname[255];
    char username[255]; // should use string
    char password[255]; // should use string
};

struct Inifile {
    struct Inifile_globals {
        bool capture_png;
        bool capture_wrm;
        bool capture_flv;
        bool capture_ocr;
        bool capture_chunk;
        bool movie;
        char movie_path[512];
        char codec_id[512];
        char video_quality[512];
        char auth_user[512];
        char host[512];
        char target_device[512];
        char target_user[512];
        char auth_channel[512];

        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        int encryptionLevel;     // 0=low, 1=medium, 2=high
        char authip[255];
        int authport;
        bool nomouse;
        bool notimestamp;
        bool autovalidate;       // dialog autovalidation for test
        char dynamic_conf_path[1024]; // directory where to look for dynamic configuration files

        unsigned capture_flags;  // 1 PNG capture, 2 WRM
        unsigned png_interval;   // time between 2 png captures (in 1/10 seconds)
        unsigned frame_interval; // time between 2 frame captures (in 1/100 seconds)
        unsigned break_interval; // time between 2 wrm movies (in seconds)
        uint64_t flv_break_interval;  // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval; 
        unsigned ocr_interval;
        unsigned capture_groupid;

        unsigned png_limit;    // number of png captures to keep

        int l_bitrate;         // bitrate for low quality
        int l_framerate;       // framerate for low quality
        int l_height;          // height for low quality
        int l_width;           // width for low quality
        int l_qscale;          // qscale (parameter given to ffmpeg) for low quality

        // Same for medium quality
        int m_bitrate;
        int m_framerate;
        int m_height;
        int m_width;
        int m_qscale;

        // Same for high quality
        int h_bitrate;
        int h_framerate;
        int h_height;
        int h_width;
        int h_qscale;

        // keepalive and no traffic auto deconnexion
        int max_tick;
        int keepalive_grace_delay;

        char replay_path[1024];
        bool internal_domain;

        bool enable_file_encryption;
        bool enable_tls;
        char listen_address[256];
        bool enable_ip_transparent;
        char certificate_password[256];

        char png_path[1024];
        char wrm_path[1024];

        char alternate_shell[1024];
        char shell_working_directory[1024];

        // Section "debug"
        struct {
            uint32_t x224;
            uint32_t mcs;
            uint32_t sec;
            uint32_t rdp;
            uint32_t primary_orders;
            uint32_t secondary_orders;
            uint32_t bitmap;
            uint32_t capture;
            uint32_t auth;
            uint32_t session;
            uint32_t front;
            uint32_t mod_rdp;
            uint32_t mod_vnc;
            uint32_t mod_int;
            uint32_t mod_xup;
            uint32_t widget;
            uint32_t input;

            int  log_type;
            char log_file_path[1024]; // log file location
        } debug;

        // section "client"
        struct {
            bool ignore_logon_password; // if true, ignore password provided by RDP client, user need do login manually. default false

            uint32_t performance_flags_default;
            uint32_t performance_flags_force_present;
            uint32_t performance_flags_force_not_present;

            bool tls_fallback_legacy;

            bool clipboard;
            bool device_redirection;
        } client;

        // section "translation"
        struct {
            char button_ok[128];
            char button_cancel[128];
            char button_help[128];
            char button_close[128];
            char button_refused[128];
            char login[128];
            char username[128];
            char password[128];
            char target[256];
            char diagnostic[1024];
            char connection_closed[1024];
            char help_message[1024];
        } translation;

        // section "context"
        struct {
            unsigned           opt_bitrate;
            unsigned           opt_framerate;
            unsigned           opt_qscale;

            bool               ask_opt_bpp;
            bool               ask_opt_height;
            bool               ask_opt_width;

            unsigned           opt_bpp;
            unsigned           opt_height;
            unsigned           opt_width;

            redemption::string auth_error_message;

            bool               ask_selector;
            bool               ask_selector_current_page;
            bool               ask_selector_device_filter;
            bool               ask_selector_group_filter;
            bool               ask_selector_lines_per_page;

            bool               selector;
            unsigned           selector_current_page;
            redemption::string selector_device_filter;
            redemption::string selector_group_filter;
            unsigned           selector_lines_per_page;
            unsigned           selector_number_of_pages;

            bool               ask_target_device;
            bool               ask_target_password;
            bool               ask_target_port;
            bool               ask_target_protocol;
            bool               ask_target_user;

//            redemption::string target_device;
            redemption::string target_password;
            unsigned           target_port;
            redemption::string target_protocol;
//            redemption::string target_user;

            bool               ask_auth_user;

            bool               ask_host;
            bool               ask_password;

            redemption::string password;

            bool               ask_authchannel_target;
            bool               ask_authchannel_result;

            redemption::string authchannel_answer;
            redemption::string authchannel_result;
            redemption::string authchannel_target;

            bool               ask_accept_message;
            bool               ask_display_message;

            redemption::string message;
            redemption::string accept_message;
            redemption::string display_message;

            redemption::string rejected;

            bool               authenticated;

            bool               ask_keepalive;
            bool               ask_proxy_type;

            bool               keepalive;
            redemption::string proxy_type;

            bool               ask_trace_seal;

            redemption::string trace_seal;

            redemption::string session_id;

            unsigned           end_date_cnx;
            redemption::string end_time;

            redemption::string mode_console;
            signed             timezone;
        } context;
    } globals;

    struct IniAccounts account;

    Inifile() {
        std::stringstream oss("");
        this->init();
        this->cparse(oss);
    }

    Inifile(const char * filename) {
        this->init();
        this->cparse(filename);
    }


    Inifile(istream & Inifile_stream) {
        this->init();
        this->cparse(Inifile_stream);
    }

    void init(){
        this->globals.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
        this->globals.capture_wrm   = true;
        this->globals.capture_png   = true;
        this->globals.capture_flv   = false;
        this->globals.capture_ocr   = false;
        this->globals.capture_chunk = false;
        this->globals.movie            = false;
        this->globals.movie_path[0]    = 0;
        this->globals.auth_user[0]     = 0;
        this->globals.host[0]          = 0;
        this->globals.target_device[0] = 0;
        this->globals.target_user[0]   = 0;

        this->globals.bitmap_cache = true;
        this->globals.bitmap_compression = true;
        this->globals.port = 3389;
        this->globals.nomouse = false;
        this->globals.notimestamp = false;
        this->globals.encryptionLevel = level_from_cstr("low");
        strcpy(this->globals.authip, "127.0.0.1");
        this->globals.authport = 3350;
        this->globals.autovalidate = false;
        strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");
        strcpy(this->globals.codec_id, "flv");
        TODO("this could be some kind of enumeration")
        strcpy(this->globals.video_quality, "medium");

        this->globals.png_interval = 3000;
        this->globals.ocr_interval = 100; // 1 every second
        this->globals.frame_interval = 40;
        this->globals.break_interval = 600;
        this->globals.flv_break_interval = 600000000l;
        this->globals.flv_frame_interval = 1000000L; 
        this->globals.capture_groupid = 33; 

        this->globals.png_limit = 3;
        this->globals.l_bitrate   = 20000;
        this->globals.l_framerate = 1;
        this->globals.l_height    = 480;
        this->globals.l_width     = 640;
        this->globals.l_qscale    = 25;
        this->globals.m_bitrate   = 40000;
        this->globals.m_framerate = 1;
        this->globals.m_height    = 768;
        this->globals.m_width     = 1024;
        this->globals.m_qscale    = 15;
        this->globals.h_bitrate   = 200000;
        this->globals.h_framerate = 5;
        this->globals.h_height    = 1024;
        this->globals.h_width     = 1280;
        this->globals.h_qscale    = 15;
        this->globals.max_tick    = 30;
        this->globals.keepalive_grace_delay = 30;
        strcpy(this->globals.replay_path, "/tmp/");
        this->globals.internal_domain = false;
        this->globals.enable_file_encryption = false;
        this->globals.enable_tls             = true;
        strcpy(this->globals.listen_address, "0.0.0.0");
        this->globals.enable_ip_transparent  = false;
        strcpy(this->globals.certificate_password, "inquisition");

        strcpy(this->globals.png_path, PNG_PATH);
        strcpy(this->globals.wrm_path, WRM_PATH);

        this->globals.alternate_shell[0]         = 0;
        this->globals.shell_working_directory[0] = 0;

        memcpy(this->globals.auth_channel, "\0\0\0\0\0\0\0\0", 8);

        strcpy(this->account.accountname, "");
        strcpy(this->account.username,    "");
        strcpy(this->account.password,    "");

        // Section "debug".
        this->globals.debug.x224              = 0;
        this->globals.debug.mcs               = 0;
        this->globals.debug.sec               = 0;
        this->globals.debug.rdp               = 0;
        this->globals.debug.primary_orders    = 0;
        this->globals.debug.secondary_orders  = 0;
        this->globals.debug.bitmap            = 0;
        this->globals.debug.capture           = 0;
        this->globals.debug.auth              = 0;
        this->globals.debug.session           = 0;
        this->globals.debug.front             = 0;
        this->globals.debug.mod_rdp           = 0;
        this->globals.debug.mod_vnc           = 0;
        this->globals.debug.mod_int           = 0;
        this->globals.debug.mod_xup           = 0;
        this->globals.debug.widget            = 0;
        this->globals.debug.input             = 0;

        this->globals.debug.log_type         = 2; // syslog by default
        this->globals.debug.log_file_path[0] = 0;

        // Section "client".
        this->globals.client.ignore_logon_password               = false;
//      this->globals.client.performance_flags_default           = PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        this->globals.client.performance_flags_default           = 0;
        this->globals.client.performance_flags_force_present     = 0;
        this->globals.client.performance_flags_force_not_present = 0;
        this->globals.client.tls_fallback_legacy                 = false;
        this->globals.client.clipboard                           = true;
        this->globals.client.device_redirection                  = true;

        // Section "translation"
        strcpy(this->globals.translation.button_ok,         "OK");
        strcpy(this->globals.translation.button_cancel,     "Cancel");
        strcpy(this->globals.translation.button_help,       "Help");
        strcpy(this->globals.translation.button_close,      "Close");
        strcpy(this->globals.translation.button_refused,    "Refused");
        strcpy(this->globals.translation.login,             "login");
        strcpy(this->globals.translation.username,          "username");
        strcpy(this->globals.translation.password,          "password");
        strcpy(this->globals.translation.target,            "target");
        strcpy(this->globals.translation.diagnostic,        "diagnostic");
        strcpy(this->globals.translation.connection_closed, "Connection closed");
        strcpy(this->globals.translation.help_message,      "Help message");

        // section "context"
        this->globals.context.opt_bitrate                 = 40000;
        this->globals.context.opt_framerate               = 5;
        this->globals.context.opt_qscale                  = 15;

        this->globals.context.ask_opt_bpp                 = false;
        this->globals.context.ask_opt_height              = false;
        this->globals.context.ask_opt_width               = false;

        this->globals.context.opt_bpp                     = 24;
        this->globals.context.opt_height                  = 600;
        this->globals.context.opt_width                   = 800;

        this->globals.context.auth_error_message          = "";

        this->globals.context.ask_selector                = false;
        this->globals.context.ask_selector_current_page   = false;
        this->globals.context.ask_selector_device_filter  = false;
        this->globals.context.ask_selector_group_filter   = false;
        this->globals.context.ask_selector_lines_per_page = false;

        this->globals.context.selector                    = false;
        this->globals.context.selector_current_page       = 1;
        this->globals.context.selector_device_filter      = "";
        this->globals.context.selector_group_filter       = "";
        this->globals.context.selector_lines_per_page     = 20;
        this->globals.context.selector_number_of_pages    = 1;

        this->globals.context.ask_target_device           = false;
        this->globals.context.ask_target_password         = false;
        this->globals.context.ask_target_port             = false;
        this->globals.context.ask_target_protocol         = false;
        this->globals.context.ask_target_user             = false;

        this->globals.context.target_password             = "";
        this->globals.context.target_port                 = 3389;
        this->globals.context.target_protocol             = "RDP";

        this->globals.context.password                    = "";

        this->globals.context.ask_authchannel_target      = false;
        this->globals.context.ask_authchannel_result      = false;

        this->globals.context.authchannel_answer          = "";
        this->globals.context.authchannel_result          = "";
        this->globals.context.authchannel_target          = "";

        this->globals.context.ask_accept_message          = false;
        this->globals.context.ask_display_message         = false;

        this->globals.context.message                     = "";
        this->globals.context.accept_message              = "";
        this->globals.context.display_message             = "";

        this->globals.context.rejected                    = "Connection refused by authentifier.";

        this->globals.context.authenticated               = false;

        this->globals.context.ask_keepalive               = true;
        this->globals.context.ask_proxy_type              = false;

        this->globals.context.keepalive                   = false;
        this->globals.context.proxy_type                  = "RDP";

        this->globals.context.ask_trace_seal              = false;

        this->globals.context.trace_seal                  = "";

        this->globals.context.session_id                  = "";

        this->globals.context.end_date_cnx                = 0;
        this->globals.context.end_time                    = "";

        this->globals.context.mode_console                = "allow";
        this->globals.context.timezone                    = -3600;
    };

    void cparse(istream & ifs){
        const size_t maxlen = 256;
        char line[maxlen];
        char context[128] = {0};
        bool truncated = false;
        while (ifs.good()){
            ifs.getline(line, maxlen);
            if (ifs.fail() && ifs.gcount() == (maxlen-1)){
                if (!truncated){
                    LOG(LOG_INFO, "Line too long in configuration file");
                    hexdump(line, maxlen-1);
                }
                ifs.clear();
                truncated = true;
                continue;
            }
            if (truncated){
                truncated = false;
                continue;
            }
            this->parseline(line, context);
        };
    }

    void parseline(const char * line, char * context)
    {
        char key[128];
        char value[128];

        const char * startkey = line;
        for (; *startkey ; startkey++) {
            if (!isspace(*startkey)){
                if (*startkey == '['){
                    const char * startcontext = startkey + 1;
                    const char * endcontext = strchr(startcontext, ']');
                    if (endcontext){
                        memcpy(context, startcontext, endcontext - startcontext);
                        context[endcontext - startcontext] = 0;
                    }
                    return;
                }
                break;
            }
        }
        const char * endkey = strchr(startkey, '=');
        if (endkey && endkey != startkey){
            const char * sep = endkey;
            for (--endkey; endkey >= startkey ; endkey--) {
                if (!isspace(*endkey)){
                    TODO("RZ: Possible buffer overflow if length of key is larger than 128 bytes")
                    memcpy(key, startkey, endkey - startkey + 1);
                    key[endkey - startkey + 1] = 0;

                    const char * startvalue = sep + 1;
                    for ( ; *startvalue ; startvalue++) {
                        if (!isspace(*startvalue)){
                            break;
                        }
                    }
                    const char * endvalue;
/*
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        TODO("RZ: Support space in value")
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                    }
                    TODO("RZ: Possible buffer overflow if length of value is larger than 128 bytes")
                    memcpy(value, startvalue, endvalue - startvalue + 1);
                    value[endvalue - startvalue + 1] = 0;
*/
                    char *curvalue = value;
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                        else if ((*endvalue == '\\') && *(endvalue + 1)) {
                            if (endvalue > startvalue) {
                                memcpy(curvalue, startvalue, endvalue - startvalue);
                                curvalue += (endvalue - startvalue);
                            }

                            endvalue++;

                            *curvalue++ = *endvalue;

                            startvalue = endvalue + 1;
                        }
                    }
                    if (endvalue > startvalue) {
                        memcpy(curvalue, startvalue, endvalue - startvalue);
                        curvalue += (endvalue - startvalue);
                    }
                    *curvalue = 0;

                    this->setglobal(key, value, context);
                    break;
                }
            }
        }
    }

    void setglobal(const char * key, const char * value, const char * context)
    {
        if (0 == strcmp(context, "globals")){
            if (0 == strcmp(key, "bitmap_cache")){
                this->globals.bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap_compression")){
                this->globals.bitmap_compression = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "port")){
                this->globals.port = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "nomouse")){
                this->globals.nomouse = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "notimestamp")){
                this->globals.notimestamp = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "encryptionLevel")){
                this->globals.encryptionLevel = level_from_cstr(value);
            }
            else if (0 == strcmp(key, "authip")){
                strncpy(this->globals.authip, value, sizeof(this->globals.authip));
                this->globals.authip[sizeof(this->globals.authip) - 1] = 0;
            }
            else if (0 == strcmp(key, "authport")){
                this->globals.authport = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "autovalidate")){
                this->globals.autovalidate = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_tick")){
                this->globals.max_tick = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "keepalive_grace_delay")){
                this->globals.keepalive_grace_delay = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "internal_domain")){
                this->globals.internal_domain = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "dynamic_conf_path")){
                strncpy(this->globals.dynamic_conf_path, value, sizeof(this->globals.dynamic_conf_path));
                this->globals.dynamic_conf_path[sizeof(this->globals.dynamic_conf_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "auth_channel")){
                strncpy(this->globals.auth_channel, value, 8);
                this->globals.auth_channel[7] = 0;
            }
            else if (0 == strcmp(key, "enable_file_encryption")){
                this->globals.enable_file_encryption = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_tls")){
                this->globals.enable_tls = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "listen_address")){
                strncpy(this->globals.listen_address, value, sizeof(this->globals.listen_address));
                this->globals.listen_address[sizeof(this->globals.listen_address) - 1] = 0;
            }
            else if (0 == strcmp(key, "enable_ip_transparent")){
                this->globals.enable_ip_transparent = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "certificate_password")){
                strncpy(this->globals.certificate_password, value, sizeof(this->globals.certificate_password));
                this->globals.certificate_password[sizeof(this->globals.certificate_password) - 1] = 0;
            }
            else if (0 == strcmp(key, "png_path")){
                strncpy(this->globals.png_path, value, sizeof(this->globals.png_path));
                this->globals.png_path[sizeof(this->globals.png_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "wrm_path")){
                strncpy(this->globals.wrm_path, value, sizeof(this->globals.wrm_path));
                this->globals.wrm_path[sizeof(this->globals.wrm_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "alternate_shell")) {
                strncpy(this->globals.alternate_shell, value, sizeof(this->globals.alternate_shell));
                this->globals.alternate_shell[sizeof(this->globals.alternate_shell) - 1] = 0;
            }
            else if (0 == strcmp(key, "shell_working_directory")) {
                strncpy(this->globals.shell_working_directory, value, sizeof(this->globals.shell_working_directory));
                this->globals.shell_working_directory[sizeof(this->globals.shell_working_directory) - 1] = 0;
            }
            else if (0 == strcmp(key, "codec_id")) {
                strncpy(this->globals.codec_id, value, sizeof(this->globals.codec_id));
                this->globals.codec_id[sizeof(this->globals.codec_id) - 1] = 0;
            }
            else if (0 == strcmp(key, "movie")){
                this->globals.movie = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "movie_path")) {
                strncpy(this->globals.movie_path, value, sizeof(this->globals.movie_path));
                this->globals.movie_path[sizeof(this->globals.movie_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "video_quality")) {
                strncpy(this->globals.video_quality, value, sizeof(this->globals.video_quality));
                this->globals.video_quality[sizeof(this->globals.video_quality) - 1] = 0;
            }
            // Context
            else if (0 == strcmp(key, "target_device")) {
                strncpy(this->globals.target_device, value, sizeof(this->globals.target_device));
                this->globals.target_device[sizeof(this->globals.target_device) - 1] = 0;
            }
            else if (0 == strcmp(key, "target_user")) {
                strncpy(this->globals.target_user, value, sizeof(this->globals.target_user));
                this->globals.target_user[sizeof(this->globals.target_user) - 1] = 0;
            }
            else if (0 == strcmp(key, "auth_user")) {
                strncpy(this->globals.auth_user, value, sizeof(this->globals.auth_user));
                this->globals.auth_user[sizeof(this->globals.auth_user) - 1] = 0;
            }
            else if (0 == strcmp(key, "host")) {
                strncpy(this->globals.host, value, sizeof(this->globals.host));
                this->globals.host[sizeof(this->globals.host) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "client")){
            if (0 == strcmp(key, "ignore_logon_password")){
                this->globals.client.ignore_logon_password = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_default")){
                this->globals.client.performance_flags_default = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_present")){
                this->globals.client.performance_flags_force_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_not_present")){
                this->globals.client.performance_flags_force_not_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "tls_fallback_legacy")){
                this->globals.client.tls_fallback_legacy = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "clipboard")){
                this->globals.client.clipboard = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "device_redirection")){
                this->globals.client.device_redirection = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "video")){ 
            if (0 == strcmp(key, "capture_flags")){
                this->globals.capture_flags   = ulong_from_cstr(value);
                this->globals.capture_png = 0 != (this->globals.capture_flags & 1);
                this->globals.capture_wrm = 0 != (this->globals.capture_flags & 2);
                this->globals.capture_flv = 0 != (this->globals.capture_flags & 4);
                this->globals.capture_ocr = 0 != (this->globals.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")){
                this->globals.ocr_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_interval")){
                this->globals.png_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture_groupid")){
                this->globals.capture_groupid  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "frame_interval")){
                this->globals.frame_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "break_interval")){
                this->globals.break_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_limit")){
                this->globals.png_limit   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "replay_path")){
                strncpy(this->globals.replay_path, value, sizeof(this->globals.replay_path));
                this->globals.replay_path[sizeof(this->globals.replay_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "l_bitrate")){
                this->globals.l_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_framerate")){
                this->globals.l_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_height")){
                this->globals.l_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_width")){
                this->globals.l_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_qscale")){
                this->globals.l_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_bitrate")){
                this->globals.m_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_framerate")){
                this->globals.m_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_height")){
                this->globals.m_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_width")){
                this->globals.m_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_qscale")){
                this->globals.m_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_bitrate")){
                this->globals.h_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_framerate")){
                this->globals.h_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_height")){
                this->globals.h_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_width")){
                this->globals.h_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_qscale")){
                this->globals.h_qscale    = ulong_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "debug")){
            if (0 == strcmp(key, "x224")){
                this->globals.debug.x224              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mcs")){
                this->globals.debug.mcs               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "sec")){
                this->globals.debug.sec               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "rdp")){
                this->globals.debug.rdp               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "primary_orders")){
                this->globals.debug.primary_orders    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "secondary_orders")){
                this->globals.debug.secondary_orders  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap")){
                this->globals.debug.bitmap            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture")){
                this->globals.debug.capture           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth")){
                this->globals.debug.auth              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "session")){
                this->globals.debug.session           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "front")){
                this->globals.debug.front             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_rdp")){
                this->globals.debug.mod_rdp           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_vnc")){
                this->globals.debug.mod_vnc           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_int")){
                this->globals.debug.mod_int           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_xup")){
                this->globals.debug.mod_xup           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "widget")){
                this->globals.debug.widget            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "input")){
                this->globals.debug.input             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_type")){
                this->globals.debug.log_type = logtype_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_file_path")){
                strncpy(this->globals.debug.log_file_path, value, sizeof(this->globals.debug.log_file_path));
                this->globals.debug.log_file_path[sizeof(this->globals.debug.log_file_path) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "translation")){
            if (0 == strcmp(key, "button_ok")){
                strncpy(this->globals.translation.button_ok, value, sizeof(this->globals.translation.button_ok));
                this->globals.translation.button_ok[sizeof(this->globals.translation.button_ok) - 1] = 0;
            }
            else if (0 == strcmp(key, "button_cancel")){
                strncpy(this->globals.translation.button_cancel, value, sizeof(this->globals.translation.button_cancel));
                this->globals.translation.button_cancel[sizeof(this->globals.translation.button_cancel) - 1] = 0;
            }
            else if (0 == strcmp(key, "button_help")){
                strncpy(this->globals.translation.button_help, value, sizeof(this->globals.translation.button_help));
                this->globals.translation.button_help[sizeof(this->globals.translation.button_help) - 1] = 0;
            }
            else if (0 == strcmp(key, "button_close")){
                strncpy(this->globals.translation.button_close, value, sizeof(this->globals.translation.button_close));
                this->globals.translation.button_close[sizeof(this->globals.translation.button_close) - 1] = 0;
            }
            else if (0 == strcmp(key, "button_refused")){
                strncpy(this->globals.translation.button_refused, value, sizeof(this->globals.translation.button_refused));
                this->globals.translation.button_refused[sizeof(this->globals.translation.button_refused) - 1] = 0;
            }
            else if (0 == strcmp(key, "button_refused")){
                strncpy(this->globals.translation.button_refused, value, sizeof(this->globals.translation.button_refused));
                this->globals.translation.button_refused[sizeof(this->globals.translation.button_refused) - 1] = 0;
            }
            else if (0 == strcmp(key, "login")){
                strncpy(this->globals.translation.login, value, sizeof(this->globals.translation.login));
                this->globals.translation.login[sizeof(this->globals.translation.login) - 1] = 0;
            }
            else if (0 == strcmp(key, "username")){
                strncpy(this->globals.translation.username, value, sizeof(this->globals.translation.username));
                this->globals.translation.username[sizeof(this->globals.translation.username) - 1] = 0;
            }
            else if (0 == strcmp(key, "password")){
                strncpy(this->globals.translation.password, value, sizeof(this->globals.translation.password));
                this->globals.translation.password[sizeof(this->globals.translation.password) - 1] = 0;
            }
            else if (0 == strcmp(key, "target")){
                strncpy(this->globals.translation.target, value, sizeof(this->globals.translation.target));
                this->globals.translation.target[sizeof(this->globals.translation.target) - 1] = 0;
            }
            else if (0 == strcmp(key, "diagnostic")){
                strncpy(this->globals.translation.diagnostic, value, sizeof(this->globals.translation.diagnostic));
                this->globals.translation.diagnostic[sizeof(this->globals.translation.diagnostic) - 1] = 0;
            }
            else if (0 == strcmp(key, "connection_closed")){
                strncpy(this->globals.translation.connection_closed, value, sizeof(this->globals.translation.connection_closed));
                this->globals.translation.connection_closed[sizeof(this->globals.translation.connection_closed) - 1] = 0;
            }
            else if (0 == strcmp(key, "help_message")){
                strncpy(this->globals.translation.help_message, value, sizeof(this->globals.translation.help_message));
                this->globals.translation.help_message[sizeof(this->globals.translation.help_message) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "context")){
            // LOG(LOG_INFO, "parameter %s in section [%s]: value=%s", key, context, value);
            if (0 == strcmp(key, "opt_bitrate")){
                this->globals.context.opt_bitrate              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "opt_framerate")){
                this->globals.context.opt_framerate            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "opt_qscale")){
                this->globals.context.opt_qscale               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "opt_width")){
                this->globals.context.opt_width                = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "opt_height")){
                this->globals.context.opt_height               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "opt_bpp")){
                this->globals.context.opt_bpp                  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth_error_message")){
                this->globals.context.auth_error_message = value;
            }
            else if (0 == strcmp(key, "selector")){
                this->globals.context.selector                 = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "selector_group_filter")){
                this->globals.context.selector_group_filter    = value;
            }
            else if (0 == strcmp(key, "selector_device_filter")){
                this->globals.context.selector_device_filter   = value;
            }
            else if (0 == strcmp(key, "selector_lines_per_page")){
                this->globals.context.selector_lines_per_page  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "selector_number_of_pages")){
                this->globals.context.selector_number_of_pages = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "selector_current_page")){
                this->globals.context.selector_current_page    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "target_password")){
                this->globals.context.target_password          = value;
            }
            else if (0 == strcmp(key, "target_port")){
                this->globals.context.target_port              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "target_protocol")){
                this->globals.context.target_protocol          = value;
            }
            else if (0 == strcmp(key, "password")){
                this->globals.context.password                 = value;
            }
            else if (0 == strcmp(key, "authchannel_answer")){
                this->globals.context.authchannel_answer       = value;
            }
            else if (0 == strcmp(key, "authchannel_result")){
                this->globals.context.authchannel_result       = value;
            }
            else if (0 == strcmp(key, "authchannel_target")){
                this->globals.context.authchannel_target       = value;
            }
            else if (0 == strcmp(key, "message")){
                this->globals.context.message                  = value;
            }
            else if (0 == strcmp(key, "accept_message")){
                this->globals.context.accept_message           = value;
            }
            else if (0 == strcmp(key, "display_message")){
                this->globals.context.display_message          = value;
            }
            else if (0 == strcmp(key, "rejected")){
                this->globals.context.rejected                 = value;
            }
            else if (0 == strcmp(key, "authenticated")){
                this->globals.context.authenticated            = bool_from_cstr(value);
            }

            else if (0 == strcmp(key, "keepalive")){
                this->globals.context.keepalive                = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "proxy_type")){
                this->globals.context.proxy_type               = value;
            }
            else if (0 == strcmp(key, "trace_seal")){
                this->globals.context.trace_seal               = value;
            }
            else if (0 == strcmp(key, "session_id")){
                this->globals.context.session_id               = value;
            }
            else if (0 == strcmp(key, "end_date_cnx")){
                this->globals.context.end_date_cnx             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "end_time")){
                this->globals.context.end_time                 = value;
            }
            else if (0 == strcmp(key, "mode_console")){
                this->globals.context.mode_console             = value;
            }
            else if (0 == strcmp(key, "timezone")){
                this->globals.context.timezone                 = _long_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }

    const char * context_get_value(const char * key, char * buffer, size_t size) {
        const char * pszReturn = "";

        if (size) {
            *buffer = 0;

            if (!strcasecmp(key, "opt_bpp")) {
                snprintf(buffer, size, "%u", this->globals.context.opt_bpp);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "opt_height")) {
                snprintf(buffer, size, "%u", this->globals.context.opt_height);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "opt_width")) {
                snprintf(buffer, size, "%u", this->globals.context.opt_width);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector")) {
                strncpy(buffer, (this->globals.context.selector ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_current_page")) {
                snprintf(buffer, size, "%u", this->globals.context.selector_current_page);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_device_filter")) {
                strncpy(buffer, this->globals.context.selector_device_filter, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_group_filter")) {
                strncpy(buffer, this->globals.context.selector_group_filter, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_lines_per_page")) {
                snprintf(buffer, size, "%u", this->globals.context.selector_lines_per_page);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_number_of_pages")) {
                snprintf(buffer, size, "%u", this->globals.context.selector_number_of_pages);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_device")) {
                strncpy(buffer, this->globals.target_device, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_password")) {
                strncpy(buffer, this->globals.context.target_password, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_port")) {
                snprintf(buffer, size, "%u", this->globals.context.target_port);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_protocol")) {
                strncpy(buffer, this->globals.context.target_protocol, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_user")) {
                strncpy(buffer, this->globals.target_user, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "auth_user")) {
                strncpy(buffer, this->globals.auth_user, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "host")) {
                strncpy(buffer, this->globals.host, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "password")) {
                strncpy(buffer, this->globals.context.password, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authchannel_answer")) {
                strncpy(buffer, this->globals.context.authchannel_answer, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authchannel_result")) {
                strncpy(buffer, this->globals.context.authchannel_result, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authchannel_target")) {
                strncpy(buffer, this->globals.context.authchannel_target, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "message")) {
                strncpy(buffer, this->globals.context.message, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "accept_message")) {
                strncpy(buffer, this->globals.context.accept_message, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "display_message")) {
                strncpy(buffer, this->globals.context.display_message, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "rejected")) {
                strncpy(buffer, this->globals.context.rejected, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authenticated")) {
                strncpy(buffer, (this->globals.context.authenticated ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "keepalive")) {
                strncpy(buffer, (this->globals.context.keepalive ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "proxy_type")) {
                strncpy(buffer, this->globals.context.proxy_type, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "trace_seal")) {
                strncpy(buffer, this->globals.context.trace_seal, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "session_id")) {
                strncpy(buffer, this->globals.context.session_id, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "end_date_cnx")) {
                snprintf(buffer, size, "%u", this->globals.context.end_date_cnx);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "end_time")) {
                strncpy(buffer, this->globals.context.end_time, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "mode_console")) {
                strncpy(buffer, this->globals.context.mode_console, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "timezone")) {
                snprintf(buffer, size, "%d", this->globals.context.timezone);

                pszReturn = buffer;
            }
            else {
                LOG(LOG_WARNING, "context_get_value: unknown key=\"%s\"", key);
            }
        }

        //LOG(LOG_INFO, "parameter %s out section [%s]: value=%s", key, "context", pszReturn);
        return pszReturn;
    }

    void cparse(const char * filename){
        ifstream inifile(filename);
        this->cparse(inifile);
    }
};

#endif
