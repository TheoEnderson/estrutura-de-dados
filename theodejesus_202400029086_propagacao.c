#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");
    if (!in || !out) {
        fprintf(stderr, "Falha ao abrir arquivo(s).\n");
        if (in) fclose(in); if (out) fclose(out);
        return 1;
    }

    printf("DEU RED");
}
   