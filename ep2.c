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

int main (int ac, char **av) {
    char speed;
    int d, n, debug;

    if (ac < 4) {
        printf("Entrada incorreta. Número insuficiente de argumentos.\n");
        return EXIT_FAILURE;
    }

    d = atoi (av[1]);
    n = atoi (av[2]);
    if (d <= 249 || n <= 4 || n > (d/4 + 1))
        return EXIT_FAILURE;
    speed = *av[3];
    if (ac > 4 && *av[4] ==  "-" && *(av[4]+1) == "d")
        debug = 1;

    if (speed == "u")
        uniform_run (d, n, debug);
    else if (speed = "v")
        varied_run (d, n, debug);

    return EXIT_SUCCESS;
}
