/**
 * @file arvore_binaria.c
 * @brief Implementação de Árvore Binária de Busca (BST) para indexação de registros.
 *
 * Estrutura de dados hierárquica baseada em ordenação lexicográfica de rótulos.
 * Suporta inserção dinâmica, busca, e travessias em pré-ordem, em-ordem e pós-ordem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define LIMITE_NOME 51
#define LIMITE_TIPO 4

typedef struct nodo {
    char rotulo[LIMITE_NOME];
    char acesso[LIMITE_TIPO];
    uint32_t bytes;
    uint32_t chave;
    struct nodo* esq;
    struct nodo* dir;
} Nodo;

Nodo* criarNodo(const char* rotulo, const char* acesso, uint32_t bytes, uint32_t chave) {
    Nodo* novo = (Nodo*) malloc(sizeof(Nodo));
    if (!novo) {
        fprintf(stderr, "Red na alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    snprintf(novo->rotulo, LIMITE_NOME, "%s", rotulo);
    snprintf(novo->acesso, LIMITE_TIPO, "%s", acesso);

    novo->bytes = bytes;
    novo->chave = chave;
    novo->esq = novo->dir = NULL;
    return novo;
}

void adicionar(Nodo** raiz, const char* rotulo, const char* acesso, uint32_t bytes, uint32_t chave) {
    if (*raiz == NULL) {
        *raiz = criarNodo(rotulo, acesso, bytes, chave);
        return;
    }

    int cmp = strcmp(rotulo, (*raiz)->rotulo);
    if (cmp == 0) {
        if (strcmp((*raiz)->acesso, "rw") == 0) {
            snprintf((*raiz)->acesso, LIMITE_TIPO, "%s", acesso);
            (*raiz)->bytes = bytes;
            (*raiz)->chave = chave;
        }
    } else if (cmp < 0) {
        adicionar(&(*raiz)->esq, rotulo, acesso, bytes, chave);
    } else {
        adicionar(&(*raiz)->dir, rotulo, acesso, bytes, chave);
    }
}

void percursoEmOrdem(Nodo* raiz, FILE* output) {
    if (!raiz) return;
    percursoEmOrdem(raiz->esq, output);
    fprintf(output, "%u:%s|%s|%u_%s\n", raiz->chave, raiz->rotulo, raiz->acesso, raiz->bytes,
            (raiz->bytes == 1 ? "byte" : "bytes"));
    percursoEmOrdem(raiz->dir, output);
}

void percursoPreOrdem(Nodo* raiz, FILE* output) {
    if (!raiz) return;
    fprintf(output, "%u:%s|%s|%u_%s\n", raiz->chave, raiz->rotulo, raiz->acesso, raiz->bytes,
            (raiz->bytes == 1 ? "byte" : "bytes"));
    percursoPreOrdem(raiz->esq, output);
    percursoPreOrdem(raiz->dir, output);
}

void percursoPosOrdem(Nodo* raiz, FILE* output) {
    if (!raiz) return;
    percursoPosOrdem(raiz->esq, output);
    percursoPosOrdem(raiz->dir, output);
    fprintf(output, "%u:%s|%s|%u_%s\n", raiz->chave, raiz->rotulo, raiz->acesso, raiz->bytes,
            (raiz->bytes == 1 ? "byte" : "bytes"));
}

void desalocar(Nodo* raiz) {
    if (!raiz) return;
    desalocar(raiz->esq);
    desalocar(raiz->dir);
    free(raiz);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso correto: %s <input> <output>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    if (!input || !output) {
        fprintf(stderr, "Red ao abrir arquivos.\n");
        return EXIT_FAILURE;
    }

    uint32_t total;
    fscanf(input, "%u", &total);

    Nodo* raiz = NULL;
    char nome[LIMITE_NOME], acesso[LIMITE_TIPO];
    uint32_t tamanho;

    for (uint32_t i = 0; i < total; i++) {
        fscanf(input, "%s %s %u", nome, acesso, &tamanho);
        adicionar(&raiz, nome, acesso, tamanho, i);
    }

    fprintf(output, "[EPD]\n");
    percursoEmOrdem(raiz, output);

    fprintf(output, "[PED]\n");
    percursoPreOrdem(raiz, output);

    fprintf(output, "[EDP]\n");
    percursoPosOrdem(raiz, output);

    desalocar(raiz);
    fclose(input);
    fclose(output);

    return 0;
}
