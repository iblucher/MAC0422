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
#include <pthread.h>

int **track;
int *arrive, *cont, *lap_change; 
pthread_mutex_t *mutex;
rider *team_1, *team_2;

typedef struct {
    int id;
    int team;
    int speed;
    int lane;
    int pos;
    int lap;
} rider;

typedef struct {
    rider r;
} wrapper;

/* retorna 30 ou 60 com 50% de probabilidade */
int random_speed () {
    int r;
    r = 2 * ((double) rand () / ((double) RAND_MAX + 1));
    if (r == 0) return 30;
    else return 60;
}

int compare_rank (const void *a, const void *b) {
    rider aa = *(rider *) a;
    rider bb = *(rider *) b;

    if (aa.lap < bb.lap) {
        return 1;
    } else if (aa.lap > bb.lap) {
        return -1;
    } else {
        if (aa.pos < bb.pos)
            return 1;
        else (aa.pos > bb.pos)
            return -1;
    }
    return 0;
}

void * coordinator (void * args) {
    int i;
    int *rank_1, rank_2;
    rank_1 = malloc (n*sizeof (int));
    rank_2 = malloc (n*sizeof (int));
    while(1) {
        for (i = 0; i < 2*n; i++) {
            while (arrive[i] != 1);
            arrive[i] = 0;
        }
        for (i = 0; i < n; i++)
            rank_1[i] = rank_2[i] = i;
        qsort (rank_1, n, sizeof (int), compare_rank);
        qsort (rank_2, n, sizeof (int), compare_rank);


        /* depois de uma iteração, antes de uma próxima iteração */
        for (i = 0; i < 2*n; i++)
            cont[i] = 1;
    }
}

/* thread dos ciclistas */
/* casos que estamos lidando: QUEREMOS MANTER A PISTA DE ULTRAPASSAGEM LIVRE AO MAXIMO
 * - caso frente vazio: anda pra frente uma casa
 * - caso frente cheia: verificar diagonal superior pra saber se pode ultrapassar
 */
void * rider_int (void * args) {
    rider r = (*(wrapper *)args).r;
    int turn = 0;
    while (1) {
        pthread_mutex_lock(&mutex[(r.pos - 1 + d) % d]);
        pthread_mutex_lock(&mutex[r.pos]);
        if (!(r.speed == 30 && turn == 0)) {
            if (track[r.pos][0] == -1) {
                track[r.pos][r.lane] = -1;
                r.lane = 0;
            }
            if (track[r.pos + 1][0] == -1) {
                track[r.pos][r.lane] = -1;
                r.lane = 0, r.pos = r.pos + 1;
            } else if (track[r.pos + 1][1] == -1) {
                track[r.pos][r.lane] = -1;
                r.lane = 1, r.pos = r.pos + 1;
            }
            if (r.pos >= d) {
                lap_change[r.id] = 1;
                r.pos = r.pos % d;
            }
            track[r.pos][r.lane] = r.id;
        }
        turn = (turn + 1) % 2;
        pthread_mutex_unlock(&mutex[r.pos]);
        pthread_mutex_unlock(&mutex[(r.pos - 1 + d) % d]);
        arrive[i] = 1;
        while(cont[i] != 1);
        lap_change[i] = 0;
        cont[i] = 0;
    }
}

int varied_run (int d, int n, int debug) {
    int i;
    pthread_t *rider_t;
    wrapper w;

    srand (time (NULL));

    /* alocando a pista */
    track = malloc(d*sizeof(*int));
    for (i = 0; i < d; i++) {
        track[i] = malloc (2 * sizeof (int));
        track[i] = -1;
    }

    /* alocando os times */
    team_1 = malloc (n*sizeof(rider));
    team_2 = malloc (n*sizeof(rider));

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
        team_2[i].pos = d/2 + i;
        team_2[i].lap = 1;
        track[team_1[i].pos][team_1[i].lane] = team_1[i].id;
        track[team_2[i].pos][team_2[i].lane] = team_2[i].id;
    }

    /* alocando o vetor de mutexes */
    mutex = malloc(d*sizeof(pthread_mutex_t));

    arrive = malloc(2*n*sizeof(int));
    cont = malloc(2*n*sizeof(int));
    for (i = 0; i < 2*n; i++) {
        arrive[i] = 0, cont[i] = 0;
    }
    lap_change = malloc(2*n*sizeof(int));
    for (i = 0; i < 2*n; i++) {
        lap_change[i] = 0;
    }

    return EXIT_SUCCESS;
}
