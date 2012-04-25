#ifndef ACTOR_ERROR_H
#define ACTOR_ERROR_H

// error type
typedef unsigned int actor_error_t;

// standard errors
#define ACTOR_SUCCESS                   (0)
#define ACTOR_ERROR                     (1)
#define ACTOR_ERROR_INVALUE             (2)
#define ACTOR_ERROR_MEMORY              (3)
#define ACTOR_ERROR_TIMEOUT             (4)
#define ACTOR_ERROR_DISPATCH            (5)
#define ACTOR_ERROR_TOO_MANY_PROCESSES  (6)
#define ACTOR_ERROR_NETWORK             (7)
#define ACTOR_ERROR_MESSAGE_PASSING     (8)

#endif
