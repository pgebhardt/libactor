// libactor
//
// Implementation of an erlang style actor model using libdispatch
// Copyright (C) 2012  Patrik Gebhardt
// Contact: patrik.gebhardt@rub.de
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "actor.h"

// error strings
static const char* actor_error_string_success = "success";
static const char* actor_error_string_error = "error";
static const char* actor_error_string_invalue = "invalid argument";
static const char* actor_error_string_memory = "memory error";
static const char* actor_error_string_timeout = "timeout";
static const char* actor_error_string_dispatch = "dispatch error";
static const char* actor_error_string_too_many_processes = "too many processes";
static const char* actor_error_string_network = "network error";
static const char* actor_error_string_message_passing = "message passing error";

// get error string by error
const char* actor_error_string(actor_error_t error) {
    // check error
    if (error == ACTOR_SUCCESS) {
        return actor_error_string_success;
    }
    else if (error == ACTOR_ERROR) {
        return actor_error_string_error;
    }
    else if (error == ACTOR_ERROR_INVALUE) {
        return actor_error_string_invalue;
    }
    else if (error == ACTOR_ERROR_MEMORY) {
        return actor_error_string_memory;
    }
    else if (error == ACTOR_ERROR_TIMEOUT) {
        return actor_error_string_timeout;
    }
    else if (error == ACTOR_ERROR_DISPATCH) {
        return actor_error_string_dispatch;
    }
    else if (error == ACTOR_ERROR_TOO_MANY_PROCESSES) {
        return actor_error_string_too_many_processes;
    }
    else if (error == ACTOR_ERROR_NETWORK) {
        return actor_error_string_network;
    }
    else if (error == ACTOR_ERROR_MESSAGE_PASSING) {
        return actor_error_string_message_passing;
    }
    else {
        return "invalid error";
    }
}
