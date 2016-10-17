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
#include "ep2_u.h"

typedef struct {
    int id;
    int team;
    int lane;
    int pos;
    int lap;
} rider;



int n, d, debug, dead, result, finished, arrived;
int **track, *lap_change, *order, deads[4][2];
float *final_time;
pthread_mutex_t *mutex;
sem_t sem1, sem2;
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
    rider *r;
    if (id < n)
        r = &team_1[id];
    else
        r = &team_2[id - n];
    track[r->pos][r->lane] = -1;
    lap_change[r->id] = 0;
    r->lane = r->pos = r->lap = -1;
    dead++;
}

static int kill_rider_r (int id) {
    rider *r;
    if (id < n)
        r = &team_1[id];
    else
        r = &team_2[id - n];
    for (id = id; r->pos == -1; id++) {
        if (id < n)
            r = &team_1[id];
        else
            r = &team_2[id - n];
    }
    deads[dead][0] = r->id, deads[dead][1] = r->lap;
    kill_rider (r->id);
    return r->id;
}

static void printTrack () {
    int i;
    for (i = 0; i < d; i++) {
        if (track[i][0] != -1) {
            fprintf (stderr, "%3d ", track[i][0]);
        } else {
            if (i == 0 || i == d / 2)
                fprintf (stderr, "  * ");
            else
                fprintf (stderr, "  - ");
        }
    }
    fprintf (stderr, "\n");
    for (i = 0; i < d; i++) {
        if (track[i][1] != -1) {
            fprintf (stderr, "%3d ", track[i][1]);
        } else {
            if (i == 0 || i == d / 2)
                fprintf (stderr, "  * ");
            else
                fprintf (stderr, "  - ");
        }
    }
    fprintf (stderr, "\n");
}

static void *manager (void *args) {
    int i, time = 0, q = 1, dying, sem;
    int broken_1 = 0, broken_2 = 0, offset_1 = 0, offset_2 = 0;
    int *rank_1, *rank_2;
    rider first_1, first_2;
    rank_1 = malloc (n * sizeof (int));
    rank_2 = malloc (n * sizeof (int));
    while (1) {
        dying = 0;
        sem = 1;

        if (!(2 * n - dead)) {
            free (rank_1), rank_1 = NULL;
            free (rank_2), rank_2 = NULL;
            return NULL;
        }

        while (sem)
            sem_getvalue (&sem1, &sem);

        time++;
        if (debug) {
            fprintf (stderr, "Tempo: %.2fs\n", (float) time * 0.06);
            printTrack ();
        }

        for (i = 0; i < n; i++)
            rank_1[i] = rank_2[i] = i;
        qsort (rank_1, n, sizeof (int), compare_rank_1);
        qsort (rank_2, n, sizeof (int), compare_rank_2);

        if (result == -1) {
            if (lap_change[team_1[rank_1[2 - offset_1]].id]) {
                printf ("O terceiro ciclista da equipe 1 ");
                printf ("acaba de completar a %da volta ",
                        team_1[rank_1[2 - offset_1]].lap - 1);
                printf ("aos %.2fs de corrida!\n", (float) time * 0.06);
                printf ("Ordem dos ciclistas de sua equipe - ");
                printf ("#%d: %d ", 1 + offset_1, team_1[rank_1[0]].id);
                if (offset_1 < 2) {
                    printf ("#%d: %d ", 2 + offset_1,
                            team_1[rank_1[1]].id);
                    if (offset_1 < 1)
                        printf ("#3: %d ", team_1[rank_1[2]].id);
                }
                printf ("\n");
            }
            if (lap_change[team_2[rank_2[2 - offset_2]].id]) {
                printf ("O terceiro ciclista da equipe 2 ");
                printf ("acaba de completar a %da volta ",
                        team_2[rank_2[2 - offset_2]].lap - 1);
                printf ("aos %.2fs de corrida!\n", (float) time * 0.06);
                printf ("Ordem dos ciclistas de sua equipe - ");
                printf ("#%d: %d ", 1 + offset_2, team_2[rank_2[0]].id);
                if (offset_2 < 2) {
                    printf ("#%d: %d ", 2 + offset_2,
                            team_2[rank_2[1]].id);
                    if (offset_2 < 1)
                        printf ("#3: %d ", team_2[rank_2[2]].id);
                }
                printf ("\n");
            }
        }

        if (result == -1) {
            if (team_1[rank_1[2 - offset_1]].lap > 16
                && team_2[rank_2[2 - offset_2]].lap > 16)
                result = 0;
            else if (team_1[rank_1[2 - offset_1]].lap > 16)
                result = 1;
            else if (team_2[rank_2[2 - offset_2]].lap > 16)
                result = 2;
        }

        if (result == -1
            && (i =
                outdistance (team_1[rank_1[2 - offset_1]],
                             team_2[rank_2[2 - offset_2]]))) {
            if (i == 1) {
                result = 1;
                printf ("O terceiro ciclista da equipe 1 acaba de ul");
                printf ("trapassar o terceiro ciclista da equipe 2!\n");
            } else {
                result = 2;
                printf ("O terceiro ciclista da equipe 2 acaba de ul");
                printf ("trapassar o terceiro ciclista da equipe 1!\n");
            }
            free (rank_1), rank_1 = NULL;
            free (rank_2), rank_2 = NULL;
            finished = 1;
            for (i = 0; i < 2 * n - dead; i++)
                sem_post (&sem2);
            return NULL;
        }

        if (result == -1) {
            first_1 = team_1[rank_1[0]], first_2 = team_2[rank_2[0]];
            if ((lap_change[first_1.id] && first_1.lap > 4 * q) ||
                (lap_change[first_2.id] && first_2.lap > 4 * q)) {
                if (!random_int (10)) {
                    if (n - broken_1 > 3 || n - broken_2 > 3) {
                        if (n - broken_1 < 3)
                            i = kill_rider_r (n +
                                              random_int (n -
                                                          broken_2));
                        else if (n - broken_2 < 3)
                            i = kill_rider_r (random_int
                                              (n - broken_1));
                        else
                            i = kill_rider_r (random_int
                                              (2 * n - dead));
                        if (i < n)
                            broken_1++;
                        else
                            broken_2++;
                        dying++;
                    }
                }
                q++;
            }
        }

        for (i = 0; i < n; i++) {
            if (team_1[i].lap > 16) {
                order[arrived] = team_1[i].id;
                final_time[arrived] = (float) time *0.06;
                arrived++;
                kill_rider (team_1[i].id), dying++;
                offset_1++;
            }
            if (team_2[i].lap > 16) {
                order[arrived] = team_2[i].id;
                final_time[arrived] = (float) time *0.06;
                arrived++;
                kill_rider (team_2[i].id), dying++;
                offset_2++;
            }
        }

        for (i = 0; i < 2 * n - dead + dying; i++)
            sem_post (&sem2);
        sem = 1;
        while (sem)
            sem_getvalue (&sem2, &sem);

        for (i = 0; i < 2 * n - dead; i++)
            sem_post (&sem1);
    }
    return NULL;
}

/* thread dos ciclistas */
/* casos que estamos lidando: QUEREMOS MANTER A PISTA DE ULTRAPASSAGEM LIVRE AO MAXIMO
 * - caso frente vazio: anda pra frente uma casa
 * - caso frente cheia: verificar diagonal superior pra saber se pode ultrapassar
 */
static void *rider_int (void *args) {
    int walked, pos;
    rider *r = (rider *) args;
    while (!finished && r->pos != -1) {
        pos = r->pos;
        walked = 0;
        lap_change[r->id] = 0;
        if (r->pos != -1) {
            pthread_mutex_lock (&mutex[(pos - 1 + d) % d]);
            pthread_mutex_lock (&mutex[pos]);
            if (track[r->pos][0] == -1) {
                track[r->pos][r->lane] = -1;
                r->lane = 0;
            }
            if (track[(r->pos + 1) % d][0] == -1) {
                track[r->pos][r->lane] = -1;
                r->lane = 0, r->pos = (r->pos + 1) % d;
                walked = 1;
            } else if (track[(r->pos + 1) % d][1] == -1) {
                track[r->pos][r->lane] = -1;
                r->lane = 1, r->pos = (r->pos + 1) % d;
                walked = 1;
            }
            if (walked && r->team == 1 && r->pos == 0)
                lap_change[r->id] = 1, r->lap++;
            else if (walked && r->team == 2 && r->pos == d / 2)
                lap_change[r->id] = 1, r->lap++;
            track[r->pos][r->lane] = r->id;
            pthread_mutex_unlock (&mutex[pos]);
            pthread_mutex_unlock (&mutex[(pos - 1 + d) % d]);
        }
        sem_wait (&sem1);
        sem_wait (&sem2);
    }
    return NULL;
}

int uniform_run (int ad, int an, int adebug) {
    int i;
    pthread_t manager_t, *rider_t;

    n = an, d = ad, debug = adebug;
    srand (time (NULL));

    result = -1;
    arrived = dead = finished = 0;

    /* alocando a pista */
    track = malloc (d * sizeof (int *));
    for (i = 0; i < d; i++) {
        track[i] = malloc (2 * sizeof (int));
        track[i][0] = track[i][1] = -1;
    }

    /* alocando os times */
    team_1 = malloc (n * sizeof (rider));
    team_2 = malloc (n * sizeof (rider));

    /* inicializando os ciclistas e suas propriedades */
    for (i = 0; i < n; i++) {
        team_1[i].id = i;
        team_1[i].team = 1;
        team_1[i].lane = 0;
        team_1[i].pos = i;
        team_1[i].lap = 1;
        team_2[i].id = n + i;
        team_2[i].team = 2;
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

    order = malloc (2 * n * sizeof (int));
    final_time = malloc (2 * n * sizeof (float));

    for (i = 0; i < 4; i++)
        deads[i][0] = -1;

    /* estado inicial da corrida */
    if (debug) {
        fprintf (stderr, "Tempo: 0.00s\n");
        printTrack ();
    }

    /* alocando o vetor de mutexes */
    mutex = malloc (d * sizeof (pthread_mutex_t));

    for (i = 0; i < d; i++)
        pthread_mutex_init (&mutex[i], NULL);

    sem_init (&sem1, 0, 2 * n);
    sem_init (&sem2, 0, 0);

    rider_t = malloc (2 * n * sizeof (pthread_t));

    if (pthread_create (&manager_t, NULL, manager, NULL)) {
        perror ("ERROR creating manager thread\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < n; i++) {
        if (pthread_create
            (&rider_t[team_1[i].id], NULL, rider_int, &team_1[i])) {
            perror ("ERROR creating rider thread\n");
            return EXIT_FAILURE;
        }
        if (pthread_create
            (&rider_t[team_2[i].id], NULL, rider_int, &team_2[i])) {
            perror ("ERROR creating rider thread\n");
            return EXIT_FAILURE;
        }
    }

    if (pthread_join (manager_t, NULL)) {
        perror ("ERROR joining manager thread\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < n; i++) {
        if (pthread_join (rider_t[team_1[i].id], NULL)) {
            perror ("ERROR joining rider thread\n");
            return EXIT_FAILURE;
        }
        if (pthread_join (rider_t[team_2[i].id], NULL)) {
            perror ("ERROR joining rider thread\n");
            return EXIT_FAILURE;
        }
    }

    if (!finished) {
        printf ("A vitoria e do time %d!\n", result);
        printf ("Segue a lista dos ciclistas de acordo com a ");
        printf ("sua ordem de chegada:\n");
        for (i = 0; i < arrived; i++) {
            if (order[i] < n)
                printf ("Ciclista %d (equipe 1), ", order[i]);
            else
                printf ("Ciclista %d (equipe 2), ", order[i]);
            printf ("aos %.2fs de tempo\n", final_time[i]);
        }
        if (deads[0][0] != -1)
            printf ("Infelizmente os ciclistas abaixo quebrararam:\n");
        i = 0;
        while (deads[i][0] != -1) {
            if (deads[i][0] < n)
                printf ("Ciclista %d (equipe 1) ", deads[i][0]);
            else
                printf ("Ciclista %d (equipe 2) ", deads[i][0]);
            printf ("na %da volta\n", deads[i][1]);
            i++;
        }
    } else {
        printf ("A vitoria e do time %d!\n", result);
        if (deads[0][0] != -1)
            printf ("Infelizmente os ciclistas abaixo quebrararam:\n");
        i = 0;
        while (deads[i][0] != -1) {
            if (deads[i][0] < n)
                printf ("Ciclista %d (equipe 1) ", deads[i][0]);
            else
                printf ("Ciclista %d (equipe 2) ", deads[i][0]);
            printf ("na %da volta\n", deads[i][1]);
            i++;
        }
    }

    for (i = 0; i < d; i++)
        pthread_mutex_destroy (&mutex[i]);

    sem_destroy (&sem1);
    sem_destroy (&sem2);

    for (i = 0; i < d; i++)
        free (track[i]), track[i] = NULL;
    free (track);

    free (team_1), team_1 = NULL;
    free (team_2), team_2 = NULL;
    free (lap_change), lap_change = NULL;
    free (order), order = NULL;
    free (final_time), final_time = NULL;
    free (mutex), mutex = NULL;
    free (rider_t), rider_t = NULL;

    return EXIT_SUCCESS;
}
