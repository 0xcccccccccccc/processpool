#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/wait.h>
#include <sys/socket.h>

#include "processpool.h"

#define PARENT 0
#define CHILD  1

struct pp_pool_t {
    int fd[2];
    int max_process;
};

pp_pool_t *pp_pool_new(int np) {
    pp_pool_t *pool = NULL;

    if (np <= 0) {
        return NULL;
    }

    pool = malloc(sizeof(*pool));
    pool->max_process = np;
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, pool->fd) == -1) {
        goto fail;
    }

    if (pp_pool_process_addn(pool, np) == -1) {
        goto fail1;
    }

    return pool;

fail1:
    exit(1);
fail:
    free(pool);
    return NULL;
}

typedef struct process_arg_t process_arg_t;
struct process_arg_t {
    int                type;
    void (*func)(void *arg);
    void              *arg;
};

/*
 * @param pool  process pool
 * @return TODO
 */
static int pp_pool_process_loop(pp_pool_t *pool) {

    process_arg_t arg;
    int           rv;

    printf("loop start\n");
    for (;;) {
        rv = read(pool->fd[CHILD], &arg, sizeof(process_arg_t));
        if (rv <= 0) {
            printf("error reading data from the parent process:%s\n",
                    strerror(errno));
            break;
        }

        arg.func(arg.arg);
    }

    return 0;
}

/*
 * @param pool  process pool
 *
 * @return child process pid or -1
 */
static int pp_pool_process_add_core(pp_pool_t *pool) {
    pid_t pid;

    switch(pid = fork()) {
        case 0:/* child */
            pp_pool_process_loop(pool);
            break;
        case -1:/* error */
            return -1;
        default:/* parent */
            break;
    }

    return pid;
}

int pp_pool_add(pp_pool_t *pool, void (*func)(void *), void *arg) {
    process_arg_t a;

    a.func = func;
    a.arg  = arg;

    return write(pool->fd[PARENT], &a, sizeof(a));
}

int pp_pool_process_addn(pp_pool_t *pool, int n) {
    int i, pid;
    for (i = 0; i < n; i++) {
        pid = pp_pool_process_add_core(pool);
        if (pid == -1) {
            printf("create process:%s\n", strerror(errno));
            return pid;
        }
    }
    return 0;
}

int pp_pool_process_deln(pp_pool_t *pool, int n) {
    return 0;
}

int pp_pool_wait(pp_pool_t *pool) {
    int status;

    for (;;) {
        wait(&status);
    }
    return 0;
}

int pp_pool_free(pp_pool_t *pool) {
    free(pool);
    return 0;
}

