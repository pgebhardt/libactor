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
#define ACTOR_INVALID_TYPE          ((actor_data_type_t)(-1))
#define ACTOR_TYPE_CHAR             ((actor_data_type_t)(1))
#define ACTOR_TYPE_SHORT            ((actor_data_type_t)(2))
#define ACTOR_TYPE_USHORT           ((actor_data_type_t)(3))
#define ACTOR_TYPE_INT              ((actor_data_type_t)(4))
#define ACTOR_TYPE_UINT             ((actor_data_type_t)(5))
#define ACTOR_TYPE_LONG             ((actor_data_type_t)(6))
#define ACTOR_TYPE_ULONG            ((actor_data_type_t)(7))
#define ACTOR_TYPE_LONGLONG         ((actor_data_type_t)(8))
#define ACTOR_TYPE_ULONGLONG        ((actor_data_type_t)(9))
#define ACTOR_TYPE_FLOAT            ((actor_data_type_t)(10))
#define ACTOR_TYPE_DOUBLE           ((actor_data_type_t)(11))
#define ACTOR_TYPE_LONGDOUBLE       ((actor_data_type_t)(12))
#define ACTOR_TYPE_ERROR_MESSAGE    ((actor_data_type_t)(13))
#define ACTOR_TYPE_KEY              ((actor_data_type_t)(14))
#define ACTOR_TYPE_CUSTOM           ((actor_data_type_t)(15))

#endif
