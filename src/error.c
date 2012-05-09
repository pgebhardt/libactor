#include "actor.h"

// error strings
const char* actor_error_string_success = "success";
const char* actor_error_string_error = "error";
const char* actor_error_string_invalue = "invalid argument";
const char* actor_error_string_memory = "memory error";
const char* actor_error_string_timeout = "timeout";
const char* actor_error_string_dispatch = "dispatch error";
const char* actor_error_string_too_many_processes = "too many processes";
const char* actor_error_string_network = "network error";
const char* actor_error_string_message_passing = "message passing error";

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
