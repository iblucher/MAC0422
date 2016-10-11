#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int d, n, debug;
char speed;
int **track;
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
    int r = rand() % 2; // PODE ESTAR ERRADO 
    if (s == 0) return 30;
    else return 60;
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
            if (track[r.pos + 1][0] == -1) {
                track[r.pos][r.lane] = -1;
                r.lane = 0, r.pos = (r.pos + 1) % d;
                track[r.pos][r.lane] = r.id;
            } else if (track[r.pos + 1][1] == -1) {
                track[r.pos][r.lane] = -1;
                r.lane = 1, r.pos = (r.pos + 1) % d;
                track[r.pos][r.lane] = r.id;
            }
            if (track[r.pos][0] == -1) {
                track[r.pos][r.lane] = -1;
                r.lane = 0;
                track[r.pos][r.lane] = r.id;
            }
        }
        turn = (turn + 1) % 2;
        pthread_mutex_unlock(&mutex[r.pos]);
        pthread_mutex_unlock(&mutex[(r.pos - 1 + d) % d]);
    }
}

int main (int ac, char **av) {

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
        team_1[i].speed = 60; // ARRUMAR PROS DOIS CASOS
        team_1[i].lane = 0;
        team_1[i].pos = i;
        team_1[i].lap = 1;
        team_2[i].id = n + i;
        team_2[i].team = 2;
        team_2[i].speed = 60; // ARRUMAR PROS DOIS CASOS
        team_2[i].lane = 0;
        team_2[i].pos = d/2 + i;
        team_2[i].lap = 1;
        track[team_1[i].pos][team_1[i].lane] = team_1[i].id;
        track[team_2[i].pos][team_2[i].lane] = team_2[i].id;
    }

    /* alocando o vetor de mutexes */
    mutex = malloc(d*sizeof(pthread_mutex_t));

    /* ./ep2 d n [v||u] -d
     * d = tamanho do velódromo
     * n = número de ciclistas por equipe
     * [v||u] = determinar modo de velocidade das simulações
     * -d = modo de debug (opcional)
     */
    if (ac < 4) {
        printf("Entrada incorreta. Número insuficiente de argumentos.\n");
        return EXIT_FAILURE;
    }

    d = av[1];
    n = av[2];
    if (d <= 249 || n <= 4 || n > (d/4 + 1))
        return EXIT_FAILURE;
    speed = av[3];
    if (ac == 5)
        if (strcmp(av[4], "-d") == 0)
            debug = 1;
    }

    for (i = 0; i < n; i++) {
        w.r = team_1[i];
        pthread_create(&rider_t[team_1[i].id], NULL, rider_int, &w);
        w.r = team_2[i];
        pthread_create(&rider_t[team_2[i].id], NULL, rider_int, &w);
    }



    return EXIT_SUCCESS;

}