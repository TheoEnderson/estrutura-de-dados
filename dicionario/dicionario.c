/**
 * @file dicionario.c
 * @brief Dicionário de Sinônimos implementado com Árvore AVL Auto-Balanceada.
 *
 * Mantém termos léxicos e seus respectivos sinônimos ordenados em uma árvore AVL,
 * garantindo operações de inserção, busca e rotações de balanceamento em tempo O(log n).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define PAL_MAX 31
#define SIN_MAX 10

typedef struct nodo {
    char termo[PAL_MAX];
    char sinonimos[SIN_MAX][PAL_MAX];
    uint8_t qtd_sinonimos;
    uint32_t alt;
    struct nodo *esq, *dir;
} Nodo;

uint32_t altura(Nodo *n);
int32_t fator(Nodo *n);
Nodo* rotEsquerda(Nodo *n);
Nodo* rotDireita(Nodo *n);
Nodo* balancear(Nodo *n);
Nodo* adicionar(Nodo *n, Nodo *novo);
Nodo* criarNodo(char *termo);
void procurar(Nodo *n, char *busca, FILE *saida);
void destruir(Nodo *n);

uint32_t altura(Nodo *n) {
    return n ? n->alt : 0;
}

int32_t fator(Nodo *n) {
    return (int32_t)(altura(n->dir)) - (int32_t)(altura(n->esq));
}

Nodo* rotEsquerda(Nodo *n) {
    Nodo *aux = n->dir;
    n->dir = aux->esq;
    aux->esq = n;
    n->alt = 1 + (altura(n->esq) > altura(n->dir) ? altura(n->esq) : altura(n->dir));
    aux->alt = 1 + (altura(aux->esq) > altura(aux->dir) ? altura(aux->esq) : altura(aux->dir));
    return aux;
}

Nodo* rotDireita(Nodo *n) {
    Nodo *aux = n->esq;
    n->esq = aux->dir;
    aux->dir = n;
    n->alt = 1 + (altura(n->esq) > altura(n->dir) ? altura(n->esq) : altura(n->dir));
    aux->alt = 1 + (altura(aux->esq) > altura(aux->dir) ? altura(aux->esq) : altura(aux->dir));
    return aux;
}

Nodo* balancear(Nodo *n) {
    n->alt = 1 + (altura(n->esq) > altura(n->dir) ? altura(n->esq) : altura(n->dir));

    int32_t balanco = fator(n);

    if (balanco > 1) {
        if (fator(n->dir) < 0)
            n->dir = rotDireita(n->dir);
        return rotEsquerda(n);
    }

    if (balanco < -1) {
        if (fator(n->esq) > 0)
            n->esq = rotEsquerda(n->esq);
        return rotDireita(n);
    }

    return n;
}

Nodo* criarNodo(char *termo) {
    Nodo *novo = (Nodo*) malloc(sizeof(Nodo));
    strcpy(novo->termo, termo);
    novo->qtd_sinonimos = 0;
    novo->alt = 1;
    novo->esq = novo->dir = NULL;
    return novo;
}

Nodo* adicionar(Nodo *n, Nodo *novo) {
    if (!n) return novo;

    if (strcmp(novo->termo, n->termo) < 0)
        n->esq = adicionar(n->esq, novo);
    else if (strcmp(novo->termo, n->termo) > 0)
        n->dir = adicionar(n->dir, novo);
    else {
        free(novo);
        return n;
    }

    return balancear(n);
}

void procurar(Nodo *n, char *busca, FILE *saida) {
    bool encontrou = false;
    fprintf(saida, "[");

    while (n) {
        fprintf(saida, "%s", n->termo);
        int comp = strcmp(busca, n->termo);

        if (!comp) {
            encontrou = true;
            break;
        }

        fprintf(saida, "->");
        n = (comp < 0 ? n->esq : n->dir);
    }

    if (!encontrou) fprintf(saida, "->?");
    fprintf(saida, "]\n");

    if (encontrou) {
        if (!n->qtd_sinonimos) fprintf(saida, "-\n");
        else {
            for (uint32_t i = 0; i < n->qtd_sinonimos; i++)
                fprintf(saida, "%s%c", n->sinonimos[i], i == n->qtd_sinonimos - 1 ? '\n' : ',');
        }
    } else fprintf(saida, "-\n");
}

void destruir(Nodo *n) {
    if (!n) return;
    destruir(n->esq);
    destruir(n->dir);
    free(n);
}

int main(int argc, char **argv) {
    if (argc != 3) return 1;

    FILE *ent = fopen(argv[1], "r");
    FILE *saida = fopen(argv[2], "w");

    Nodo *raiz = NULL;
    char termo[PAL_MAX];
    uint32_t qtd_termos, qtd_sin, consultas;

    fscanf(ent, "%u", &qtd_termos);
    while (qtd_termos--) {
        fscanf(ent, "%30s %u", termo, &qtd_sin);
        Nodo *novo = criarNodo(termo);

        uint32_t ler_sin = qtd_sin > SIN_MAX ? SIN_MAX : qtd_sin;
        novo->qtd_sinonimos = (uint8_t)ler_sin;

        for (uint32_t i = 0; i < ler_sin; i++)
            fscanf(ent, "%30s", novo->sinonimos[i]);
        for (uint32_t i = ler_sin; i < qtd_sin; i++) fscanf(ent, "%30s", termo);

        raiz = adicionar(raiz, novo);
    }

    fscanf(ent, "%u", &consultas);
    while (consultas--) {
        fscanf(ent, "%30s", termo);
        procurar(raiz, termo, saida);
    }

    destruir(raiz);
    fclose(ent);
    fclose(saida);

    return 0;
}