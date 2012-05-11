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

#ifndef ACTOR_TYPE_H
#define ACTOR_TYPE_H

// data type
typedef int actor_data_type_t;

// data types
#define ACTOR_INVALID_TYPE          (-1)
#define ACTOR_TYPE_CHAR             (1)
#define ACTOR_TYPE_SHORT            (2)
#define ACTOR_TYPE_USHORT           (3)
#define ACTOR_TYPE_INT              (4)
#define ACTOR_TYPE_UINT             (5)
#define ACTOR_TYPE_LONG             (6)
#define ACTOR_TYPE_ULONG            (7)
#define ACTOR_TYPE_LONGLONG         (8)
#define ACTOR_TYPE_ULONGLONG        (9)
#define ACTOR_TYPE_FLOAT            (10)
#define ACTOR_TYPE_DOUBLE           (11)
#define ACTOR_TYPE_LONGDOUBLE       (12)
#define ACTOR_TYPE_ERROR_MESSAGE    (13)
#define ACTOR_TYPE_KEY              (14)
#define ACTOR_TYPE_CUSTOM           (15)

#endif
