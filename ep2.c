#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int d, n, debug;
char speed;

struct rider_i {
    int id;
    int team;
    int speed;
    int lap;
};
typedef struct rider_i rider;

int **track;

rider *team_1, *team_2;

/* retorna 30 ou 60 com 50% de probabilidade */
int random_speed () {
    int r = rand() % 2; // PODE ESTAR ERRADO 
    if (s == 0) return 30;
    else return 60;
}

/* thread dos ciclistas */
void * rider_int (void * args) {

}

int main (int ac, char **av) {

    int i;
    pthread_t *rider_t;

    track = malloc(d*sizeof(*int));
    for (i = 0; i < d; i++) {
        track[i] = malloc (2 * sizeof (int));
    }

    team_1 = malloc (n*sizeof(rider));
    team_2 = malloc (n*sizeof(rider));
    for (i = 0; i < n; i++) {
        team_1[i].id = i;
        team_1[i].team = 1;
        team_1[i].speed = 60; // ARRUMAR PROS DOIS CASOS
        team_1[i].lap = 1;
        team_2[i].id = n + i;
        team_2[i].team = 2;
        team_2[i].speed = 60; // ARRUMAR PROS DOIS CASOS
        team_2[i].lap = 1;
        track[i][0] = team_1[i].id;
        track[d/2 + i][0] = team_2[i].id;
    }

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

    for (i = 0; i < 2*n; i++) {
        pthread_create(&rider_t[i], NULL, rider_int, NULL);
    }

    return EXIT_SUCCESS;

}