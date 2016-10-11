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

/* retorna 30 ou 60 com 50% de probabilidade */
int random_speed () {
    int r = rand() % 2; // PODE ESTAR ERRADO 
    if (s == 0) return 30;
    else return 60;
}

int main (int ac, char **av) {

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

    return EXIT_SUCCESS;

}