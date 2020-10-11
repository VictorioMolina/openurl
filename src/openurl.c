/**
 * Universidad de Murcia
 *
 * Ampliación de Sistemas Operativos
 * Autor: Victorio Jesús Molina Bermejo
 * DNI: 48632380-F
 */

#define _POSIX_C_SOURCE 200809L
#define DEFAULT_URL "https://www.um.es/"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int fd, status;

    //  Argumentos que se esperan:
    //
    //	argv[1]
    //	En el primer argumento del programa se espera la opción '-n'
    //
    //  argv[2]
    //  En el segundo argumento del programa se espera el navegador

    if (argc < 3 || strcmp(argv[1], "-n") != 0 || !argv[2])
    {
        /*
            NOTA: No me ha quedado del todo claro el enunciado
            de la práctica:
            
                > 'Si no se especifica -n NAVEGADOR, se devolverá EXIT_FAILURE'
            
            Al leer esto, he supuesto que lo que se exige es que
            la opción -n se dé, sí o sí, como *única opción* del programa antes del navegador
            y la lista opcional de urls... y que, además, esta hace referencia a 
            la opción "--new-window" de los navegadores Firefox / Google Chrome.

            Es por esto que directamente he comprobado que argv[1] sea igual a "-n",
            ya que este, como he dicho, se tiene que dar, sí o sí, en esa posición.
            En una versión mas compleja del programa podría verificarse como opcional y
            con posición arbitraria junto a más opciones, 
            permitiendo así mostrar las urls con --new-window o no.

            Podría hacerse, por ejemplo, con getopt()
            https://man7.org/linux/man-pages/man3/getopt.3.html
        */
        fprintf(stderr, "Uso: ./openurl -n NAVEGADOR [URLs]\n");
        exit(EXIT_FAILURE);
    }

    char *navegador = argv[2];

    // Por cuestiones de simplicidad y legibilidad, obtenemos las urls en un array a parte
    int nUrls = argc - 3;
    char **urls = malloc(nUrls * sizeof(char *));  // ⚠️ malloc(0) basado en implementación

    if (nUrls)
    {
        memcpy(urls, argv + 3, nUrls * sizeof(argv[2]));
    }
    else
    {
        // Si no se ha especificado ninguna url como argumento,
        // abriremos la URL por defecto
        urls[0] = DEFAULT_URL;
        nUrls++;
    }

    // Array auxiliar en el que guardaremos los PIDs de los hijos creados en orden
    pid_t child_pids[nUrls];

    // Creamos tantos procesos como urls nos hayan dado
    for (int i = 0; i < nUrls; i++)
    {
        switch (child_pids[i] = fork())
        {
        case -1:
            perror("fork()");
            exit(EXIT_FAILURE);
        case 0: // hijo
            // Abrimos el navegador con la url correspondiente
            execlp(navegador, navegador, "--new-window", urls[i], NULL); // Si intentamos ejecutar una llamada al sistema exec*() y el binario no se puede encontrar, exec*() fallará.
            fprintf(stderr, "Error: %s no encontrado.\n", navegador);
            exit(EXIT_FAILURE);
        }
    }

    // Esperamos, en secuencia, desde el padre, la terminacion de todos los procesos paralelos
    for (int i = 0; i < nUrls; i++)
    {
        if (waitpid(child_pids[i], &status, 0) == -1)
        {
            perror("wait()");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            // Terminación normal del proceso hijo
            printf("./openurl: URL: %s, STATUS: %d\n", urls[i], WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            // Terminación por señal del proceso hijo
            printf("./openurl: URL: %s, STATUS: %d\n", urls[i], WEXITSTATUS(status));
        }
    }

    free(urls);

    return EXIT_SUCCESS;
}
