/*******************************************************************************
 *  EP2 de MAC0422 (Sistemas Operacionais)
 *
 *  Isabela Blucher - 9298170
 *  Andre Victor dos Santos Nakazawa - 9298336
 * 
 *  SIMULADOR DE PERSEGUIÇÃO POR EQUIPES
 *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include "ep2_v.h"

typedef struct {
    int id;
    int team;
    int speed;
    int lane;
    int pos;
    int lap;
} rider;

int n, d, debug, dead = 0;
int **track, *lap_change, *delayed;
pthread_mutex_t *mutex;
sem_t sem;
pthread_barrier_t barrier;
rider *team_1, *team_2;

/* numero entre 0 ... max - 1 com igual prbabilidade */
static int random_int (int max) {
    double k;
    k = (double) rand () / ((double) RAND_MAX + 1);
    return k * max;
}

static int compare_rank_1 (const void *a, const void *b) {
    rider aa = team_1[*(int *) a];
    rider bb = team_1[*(int *) b];

    if (aa.lap < bb.lap) {
        return 1;
    } else if (aa.lap > bb.lap) {
        return -1;
    } else {
        if (aa.pos < bb.pos)
            return 1;
        else if (aa.pos > bb.pos)
            return -1;
    }
    return 0;
}

static int compare_rank_2 (const void *a, const void *b) {
    rider aa = team_2[*(int *) a];
    rider bb = team_2[*(int *) b];

    if (aa.lap < bb.lap) {
        return 1;
    } else if (aa.lap > bb.lap) {
        return -1;
    } else {
        if ((aa.pos + (d - d / 2)) % d < (bb.pos + (d - d / 2)) % d)
            return 1;
        else if ((aa.pos + (d - d / 2)) % d >
                 (bb.pos + (d - d / 2)) % d)
            return -1;
    }
    return 0;
}

static int outdistance (rider r1, rider r2) {
    int dist1, dist2;

    dist1 = d * r1.lap + r1.pos;
    dist2 = d * r2.lap + (r2.pos + (d - d / 2)) % d;

    if (dist1 - dist2 > d / 2)
        return 1;
    else if (dist2 - dist1 > d / 2)
        return 2;
    else
        return 0;
}

static void kill_rider (int id) {
    rider r;
    do {
        if (id < n)
            r = team_1[id];
        else
            r = team_2[id - n];
    } while (r.pos == -1);
    track[r.pos][r.lane] = -1;
    /* imprimir lap */
    r.speed = r.lane = r.pos = r.lap = -1;
    dead++;
}

static void *manager (void *args) {
    int i = 1, q = 1;
    int *rank_1, *rank_2;
    rider *r, first_1, first_2;
    rank_1 = malloc (n * sizeof (int));
    rank_2 = malloc (n * sizeof (int));
    while (1) {
        while (i)
            sem_getvalue (&sem, &i);
        for (i = 0; i < 2 * n; i++)
            sem_post (&sem);

        for (i = 0; i < n; i++)
            rank_1[i] = i, rank_2[i] = n + i;
        qsort (rank_1, n, sizeof (int), compare_rank_1);
        qsort (rank_2, n, sizeof (int), compare_rank_2);

        if ((i = outdistance (team_1[rank_1[2]], team_2[rank_2[2]]))) {
            printf ("Time %d venceu!\n", i);
            free (rank_1), rank_1 = NULL;
            free (rank_2), rank_2 = NULL;
            return NULL;
        }

        if (team_1[rank_1[2]].lap > 16 && team_2[rank_2[2]].lap > 16) {
            printf ("Empate!\n");
            free (rank_1), rank_1 = NULL;
            free (rank_2), rank_2 = NULL;
            return NULL;
        } else if (team_1[rank_1[2]].lap > 16) {
            printf ("Time 1 venceu!\n");
            free (rank_1), rank_1 = NULL;
            free (rank_2), rank_2 = NULL;
            return NULL;
        } else if (team_2[rank_2[2]].lap > 16) {
            printf ("Time 2 venceu!\n");
            free (rank_1), rank_1 = NULL;
            free (rank_2), rank_2 = NULL;
            return NULL;
        }

        first_1 = team_1[rank_1[0]], first_2 = team_2[rank_2[0]];
        if ((lap_change[first_1.id] && first_1.lap > 4 * q) ||
            (lap_change[first_2.id] && first_2.lap > 4 * q)) {
            if (!random_int (10))
                kill_rider (random_int (2 * n - dead));
            q++;
        }

        for (i = 0; i < n; i++) {
            r = &team_1[rank_1[i]];
            if (lap_change[r->id]) {
                if (delayed[r->id] == r->lap) {
                    r->speed = 30;
                    delayed[r->id] = 0;
                } else if (random_int (2)) {
                    r->speed = 60;
                    delayed[r->id] = 0;
                } else {
                    if (r->speed != 30) {
                        r->speed = 30;
                        for (i = i + 1; i < n; i++)
                            delayed[team_1[rank_1[i]].id] = r->lap;
                    }
                    delayed[r->id] = r->lap;
                }
            }

            r = &team_2[rank_2[i]];
            if (lap_change[r->id]) {
                if (delayed[r->id] == r->lap) {
                    r->speed = 30;
                    delayed[r->id] = 0;
                } else if (random_int (2)) {
                    r->speed = 60;
                    delayed[r->id] = 0;
                } else {
                    if (r->speed != 30) {
                        r->speed = 30;
                        for (i = i + 1; i < n; i++)
                            delayed[team_2[rank_2[i]].id] = r->lap;
                    }
                    delayed[r->id] = r->lap;
                }
            }
        }

        pthread_barrier_wait (&barrier);
    }
}

/* thread dos ciclistas */
/* casos que estamos lidando: QUEREMOS MANTER A PISTA DE ULTRAPASSAGEM LIVRE AO MAXIMO
 * - caso frente vazio: anda pra frente uma casa
 * - caso frente cheia: verificar diagonal superior pra saber se pode ultrapassar
 */
void *rider_int (void *args) {
    int walked = 0, turn = 0;
    rider r = *(rider *) args;
    while (1) {
        if (r.pos != -1) {
            if (!(r.speed == 30 && turn == 0)) {
                pthread_mutex_lock (&mutex[(r.pos - 1 + d) % d]);
                pthread_mutex_lock (&mutex[r.pos]);
                if (track[r.pos][0] == -1) {
                    track[r.pos][r.lane] = -1;
                    r.lane = 0;
                }
                if (track[(r.pos + 1) % d][0] == -1) {
                    track[r.pos][r.lane] = -1;
                    r.lane = 0, r.pos = (r.pos + 1) % d;
                    walked = 1;
                } else if (track[(r.pos + 1) % d][1] == -1) {
                    track[r.pos][r.lane] = -1;
                    r.lane = 1, r.pos = (r.pos + 1) % d;
                    walked = 1;
                }
                if (walked && r.team == 1 && r.pos == 0)
                    lap_change[r.id] = 1, r.lap++;
                else if (walked && r.team == 2 && r.pos == d / 2)
                    lap_change[r.id] = 1, r.lap++;
                track[r.pos][r.lane] = r.id;
                pthread_mutex_unlock (&mutex[r.pos]);
                pthread_mutex_unlock (&mutex[(r.pos - 1 + d) % d]);
            }
            turn = (turn + 1) % 2;
        }
        sem_wait (&sem);
        pthread_barrier_wait (&barrier);
    }
    return NULL;
}

int varied_run (int ad, int an, int adebug) {
    int i;
    pthread_t *rider_t;

    n = an, d = ad, debug = adebug;
    srand (time (NULL));

    /* alocando a pista */
    track = malloc (d * sizeof (*int));
    for (i = 0; i < d; i++) {
        track[i] = malloc (2 * sizeof (int));
        track[i] = -1;
    }

    /* alocando os times */
    team_1 = malloc (n * sizeof (rider));
    team_2 = malloc (n * sizeof (rider));

    /* inicializando os ciclistas e suas propriedades */
    for (i = 0; i < n; i++) {
        team_1[i].id = i;
        team_1[i].team = 1;
        team_1[i].speed = 30;
        team_1[i].lane = 0;
        team_1[i].pos = i;
        team_1[i].lap = 1;
        team_2[i].id = n + i;
        team_2[i].team = 2;
        team_2[i].speed = 30;
        team_2[i].lane = 0;
        team_2[i].pos = d / 2 + i;
        team_2[i].lap = 1;
        track[team_1[i].pos][team_1[i].lane] = team_1[i].id;
        track[team_2[i].pos][team_2[i].lane] = team_2[i].id;
    }

    lap_change = malloc (2 * n * sizeof (int));
    for (i = 0; i < 2 * n; i++) {
        lap_change[i] = 0;
    }

    delayed = malloc (2 * n * sizeof (int));
    for (i = 0; i < 2 * n; i++) {
        delayed[i] = 0;
    }

    /* alocando o vetor de mutexes */
    mutex = malloc (d * sizeof (pthread_mutex_t));

    for (i = 0; i < d; i++)
        pthread_mutex_init (&mutex[i], NULL);

    sem_init (&sem, 0, 2 * n);

    pthread_barrier_init (&barrier, NULL, 2 * n + 1);

    rider_t = malloc (2 * n * sizeof (pthread_t));

    for (i = 0; i < n; i++) {
        if (pthread_create
            (&rider_t[team_1[i].id], NULL, rider_int, &team_1[i]))
            return EXIT_FAILURE;
        if (pthread_create
            (&rider_t[team_2[i].id], NULL, rider_int, &team_2[i]))
            return EXIT_FAILURE;
    }

    if (pthread_create (&manager_t, NULL, manager, NULL))
        return EXIT_FAILURE;

    if (pthread_join (manager_t, NULL))
        return EXIT_FAILURE;

    for (i = 0; i < d; i++)
        pthread_mutex_destroy (&mutex[i]);

    sem_destroy (&sem);
    pthread_barrier_destroy (&barrier);

    for (i = 0; i < d; i++)
        free (track[i]), track[i] = NULL;
    free (track);

    free (team_1), team_1 = NULL;
    free (team_2), team_2 = NULL;
    free (lap_change), lap_change = NULL;
    free (mutex), mutex = NULL;
    free (rider_t), rider_t = NULL;

    return EXIT_SUCCESS;
}
