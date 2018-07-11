#include <pthread.h>
#include "errors.h"

#define THREADS 5

typedef struct team_tag {
    int         join_i;
    pthread_t   workers[THREADS];
} team_t;

void *worker_routine(void *arg)
{
    int counter;
    
    for(counter = 0; ; counter++)
        if ((counter % 1000) == 0)
            pthread_testcancel();
}

void cleanup(void *arg)
{
    team_t *team = (team_t *)arg;
    int count, status;

    for (count = team->join_i; count < THREADS; count++) {
        status = pthread_cancel(team->workers[count]);
        if (status != 0)
            err_abort(status, "Cancel worker");

        status = pthread_detach(team->workers[count]);
        if (status != 0)
            err_abort(status, "Detach worker");

        printf("Cleanup: canceled %d\n", count);
    }
}

void *thread_routine(void *arg)
{
    team_t team;
    int count;
    void *result;
    int status;

    for (count = 0; count < THREADS; count++) {
        status = pthread_create(&team.workers[count], NULL, worker_routine, NULL);
        if (status != 0)
            err_abort(status, "Create worker");
    }

    pthread_cleanup_push(cleanup, (void *)&team);

    for (team.join_i = 0; team.join_i < THREADS; team.join_i++) {
        status = pthread_join(team.workers[team.join_i], &result);
        if (status != 0)
            err_abort(status, "Join worker");
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main()
{
    pthread_t thread_id;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create team");

    sleep(5);

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel team");

    status = pthread_join(thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join team");

    return 0;
}