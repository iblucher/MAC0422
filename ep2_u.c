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
    int lane;
    int pos;
    int lap;
} rider;

typedef struct {
    rider r;
} wrapper;

int compare_rank_1 (const void *a, const void *b) {
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

/* compare especifico para o time 2 */

void * coordinator (void * args) {
    int i;
    int *rank_1, rank_2;
    rank_1 = malloc (n*sizeof (int));
    rank_2 = malloc (n*sizeof (int));
    while(1) {
        /* barreira */
        for (i = 0; i < 2*n; i++) {
            while (arrive[i] != 1);
            arrive[i] = 0;
        }
        /* barreira */

        for (i = 0; i < n; i++)
            rank_1[i] = rank_2[i] = i;
        qsort (rank_1, n, sizeof (int), compare_rank);
        qsort (rank_2, n, sizeof (int), compare_rank);
        
        /* Fazer a função que compara o quanto #1 e #2 andaram
        e verifica se a partida terminou! */

        if (team_1[rank_1[2]].lap > 16)
            printf ("Time 1 venceu!\n");
        else if (team_2[rank_2[2]].lap > 16)
            printf ("Time 2 venceu!\n");

        /* quebrar os caras */

        /* depois de uma iteração, antes de uma próxima iteração */
        /* barreira */
        for (i = 0; i < 2*n; i++)
            cont[i] = 1;
        /* barreira */
    }
}

/* thread dos ciclistas */
/* casos que estamos lidando: QUEREMOS MANTER A PISTA DE ULTRAPASSAGEM LIVRE AO MAXIMO
 * - caso frente vazio: anda pra frente uma casa
 * - caso frente cheia: verificar diagonal superior pra saber se pode ultrapassar
 */
void * rider_int (void * args) {
    rider r = (*(wrapper *)args).r;
    int walked = 0;
    while (1) {
        pthread_mutex_lock(&mutex[(r.pos - 1 + d) % d]);
        pthread_mutex_lock(&mutex[r.pos]);
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
            else if (walked && r.team == 2 && r.pos == d/2)
                lap_change[r.id] = 1, r.lap++;
            track[r.pos][r.lane] = r.id;
        pthread_mutex_unlock(&mutex[r.pos]);
        pthread_mutex_unlock(&mutex[(r.pos - 1 + d) % d]);

        /* barreira */
        arrive[i] = 1;
        while(cont[i] != 1);
        lap_change[i] = 0;
        cont[i] = 0;
        /* barreira */
    }
}

int uniform_run (int d, int n, int debug) {
    int i;
    pthread_t *rider_t;
    wrapper w;

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
        team_1[i].lane = 0;
        team_1[i].pos = i;
        team_1[i].lap = 1;
        team_2[i].id = n + i;
        team_2[i].team = 2;
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
