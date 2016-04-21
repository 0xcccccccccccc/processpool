#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>

#include "processpool.h"

sem_t *sem;
void hello(void *arg) {
    //sem_wait(sem);
    printf("hello world\n");
    //sem_post(sem);
}

int main() {

    pp_pool_t *pool = NULL;
    int        i;

    sem_unlink("lock");
    sem = sem_open("lock", O_CREAT | O_EXCL, 0644, 1);
    if (sem == NULL) {
        printf("sem_open:%s\n", strerror(errno));
        return 1;
    }

    pool = pp_pool_new(5);
    printf("process pool create ok\n");

    for (i = 0; i < 1000; i++) {
        pp_pool_add(pool, hello, NULL);
        printf("%d\n", i);
    }

    pp_pool_wait(pool);

    pp_pool_free(pool);
    return 0;
}
