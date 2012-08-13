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

#ifndef ACTOR_COMMON_H
#define ACTOR_COMMON_H

// size
typedef unsigned int actor_size_t;
#define ACTOR_TYPE_SIZE ACTOR_TYPE_UINT

// time
typedef double actor_time_t;
#define ACTOR_TYPE_TIME ACTOR_TYPE_DOUBLE

// node id
typedef int actor_node_id_t;
#define ACTOR_TYPE_NODEID   ACTOR_TYPE_INT

// process id
typedef int actor_process_id_t;
#define ACTOR_TYPE_PROCESSID    ACTOR_TYPE_INT

#define ACTOR_INVALID_ID (-1)

#endif
