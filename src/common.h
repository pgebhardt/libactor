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
