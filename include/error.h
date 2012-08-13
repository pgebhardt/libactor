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

#ifndef ACTOR_ERROR_H
#define ACTOR_ERROR_H

// error type
typedef unsigned int actor_error_t;

// standard errors
#define ACTOR_SUCCESS                   ((actor_error_t)(0))
#define ACTOR_ERROR                     ((actor_error_t)(1))
#define ACTOR_ERROR_INVALUE             ((actor_error_t)(2))
#define ACTOR_ERROR_MEMORY              ((actor_error_t)(3))
#define ACTOR_ERROR_TIMEOUT             ((actor_error_t)(4))
#define ACTOR_ERROR_DISPATCH            ((actor_error_t)(5))
#define ACTOR_ERROR_TOO_MANY_PROCESSES  ((actor_error_t)(6))
#define ACTOR_ERROR_NETWORK             ((actor_error_t)(7))
#define ACTOR_ERROR_MESSAGE_PASSING     ((actor_error_t)(8))

// get error string by error
const char* actor_error_string(actor_error_t error);

#endif
