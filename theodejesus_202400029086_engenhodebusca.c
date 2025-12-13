#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define P1  7919U
#define P2  104729U
#define DESLOC 123U
#define MAX_TEXTO 101

typedef struct {
    char **requisicoes;
    size_t qtd, capacidade;
} Servidor;

void inicializar_servidor(Servidor *servidor) {
    servidor->qtd = 0;
    servidor->capacidade = 4;
    servidor->requisicoes = malloc(servidor->capacidade * sizeof(char*));
    if (!servidor->requisicoes) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void adicionar_requisicao(Servidor *servidor, char *requisicao) {
    if (servidor->qtd == servidor->capacidade) {
        servidor->capacidade *= 2;
        servidor->requisicoes = realloc(servidor->requisicoes, servidor->capacidade * sizeof(char*));
        if (!servidor->requisicoes) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }
    servidor->requisicoes[servidor->qtd++] = requisicao;
}

void liberar_servidor(Servidor *servidor) {
    for (size_t i = 0; i < servidor->qtd; ++i)
        free(servidor->requisicoes[i]);
    free(servidor->requisicoes);
}

uint32_t hash_principal(uint8_t checksum, uint32_t tamanho) {
    return (P1 * checksum) % tamanho;
}

uint32_t hash_secundario(uint8_t checksum, uint32_t tamanho) {
    uint32_t resultado = (P2 * checksum + DESLOC) % tamanho;
    return resultado ? resultado : 1;
}

char *montar_requisicao_completa(uint32_t qtd_partes, FILE *entrada, uint8_t *checksum) {
    char buffer[MAX_TEXTO];
    size_t tamanho_total = 1;
    char **partes = calloc(qtd_partes, sizeof(char*));

    for (uint32_t i = 0; i < qtd_partes; ++i) {
        fscanf(entrada, "%100s", buffer);
        partes[i] = strdup(buffer);
        for (char *p = buffer; *p; ++p)
            *checksum ^= (uint8_t)*p;
        tamanho_total += strlen(buffer) + (i > 0);
    }

    char *requisicao_completa = malloc(tamanho_total);
    requisicao_completa[0] = '\0';
    for (uint32_t i = 0; i < qtd_partes; ++i) {
        if (i > 0) strcat(requisicao_completa, "_");
        strcat(requisicao_completa, partes[i]);
        free(partes[i]);
    }
    free(partes);

    return requisicao_completa;
}

uint32_t encontrar_servidor(uint32_t pos_original, uint32_t passo, uint32_t qtd_servidores, uint32_t capacidade, uint32_t *carga_servidor) {
    uint32_t pos = pos_original;
    do {
        if (carga_servidor[pos] < capacidade)
            return pos;
        pos = (pos + passo) % qtd_servidores;
    } while (pos != pos_original);

    return qtd_servidores;
}

void processar_requisicoes(FILE *entrada, FILE *saida, Servidor *servidores, uint32_t *carga_servidor, uint32_t qtd_servidores, uint32_t capacidade, uint32_t qtd_requisicoes) {
    for (uint32_t idx = 0; idx < qtd_requisicoes; ++idx) {
        uint32_t qtd_partes;
        if (fscanf(entrada, "%u", &qtd_partes) != 1)
            break;

        uint8_t checksum = 0;
        char *requisicao_completa = montar_requisicao_completa(qtd_partes, entrada, &checksum);

        uint32_t pos_original = hash_principal(checksum, qtd_servidores);
        uint32_t passo = hash_secundario(checksum, qtd_servidores);
        uint32_t pos = encontrar_servidor(pos_original, passo, qtd_servidores, capacidade, carga_servidor);

        if (pos == qtd_servidores) {
            free(requisicao_completa);
            continue;
        }

        if (pos != pos_original && carga_servidor[pos_original] >= capacidade)
            fprintf(saida, "S%u->S%u\n", pos_original, pos);

        adicionar_requisicao(&servidores[pos], requisicao_completa);
        carga_servidor[pos]++;

        fprintf(saida, "S%u:%s", pos, servidores[pos].requisicoes[0]);
        for (size_t i = 1; i < servidores[pos].qtd; ++i)
            fprintf(saida, ",%s", servidores[pos].requisicoes[i]);
        fprintf(saida, "\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso correto: %s entrada.txt saida.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *entrada = fopen(argv[1], "r");
    FILE *saida = fopen(argv[2], "w");
    if (!entrada || !saida) {
        perror("Deu red ao abrir arquivos");
        return EXIT_FAILURE;
    }

    uint32_t qtd_servidores, capacidade, qtd_requisicoes;
    if (fscanf(entrada, "%u %u %u", &qtd_servidores, &capacidade, &qtd_requisicoes) != 3) {
        fprintf(stderr, "Formato de entrada inválido.\n");
        return EXIT_FAILURE;
    }

    Servidor *servidores = calloc(qtd_servidores, sizeof(Servidor));
    uint32_t *carga_servidor = calloc(qtd_servidores, sizeof(uint32_t));
    if (!servidores || !carga_servidor) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    for (uint32_t i = 0; i < qtd_servidores; ++i)
        inicializar_servidor(&servidores[i]);

    processar_requisicoes(entrada, saida, servidores, carga_servidor, qtd_servidores, capacidade, qtd_requisicoes);

    for (uint32_t i = 0; i < qtd_servidores; ++i)
        liberar_servidor(&servidores[i]);

    free(servidores);
    free(carga_servidor);
    fclose(entrada);
    fclose(saida);

    return EXIT_SUCCESS;
}
