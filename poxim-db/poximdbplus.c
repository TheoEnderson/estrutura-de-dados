/**
 * @file poximdbplus.c
 * @brief Sistema de banco de dados baseado em Árvore B+ (B+ Tree).
 *
 * Implementa estrutura de Árvore B+ com nós folha encadeados sequencialmente,
 * permitindo buscas pontuais rápidas e varreduras por faixa (range queries) altamente eficientes.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HLEN 33
#define NLEN 31

typedef struct {
    char nome[NLEN];
    uint32_t tam;
    char hash[HLEN];
} Arquivo;

typedef struct node {
    char **keys;
    struct node **ptrs;
    Arquivo *reg;
    uint32_t qtd;
    int folha;
    struct node *prox;
    struct node *up;
} Node;

Node* criar_node(uint32_t ordem, int folha) {
    Node *n = malloc(sizeof(Node));
    n->keys = calloc(ordem, sizeof(char*));
    for (uint32_t i = 0; i < ordem; i++)
        n->keys[i] = calloc(HLEN, sizeof(char));
    n->ptrs = calloc(ordem + 1, sizeof(Node*));
    n->reg  = folha ? calloc(ordem, sizeof(Arquivo)) : NULL;
    n->qtd = 0;
    n->folha = folha;
    n->prox = n->up = NULL;
    return n;
}

Node* dividir_folha(Node *a, uint32_t ordem) {
    int m = ordem / 2;
    Node *b = criar_node(ordem, 1);
    for (int i = m, j = 0; i < (int)a->qtd; i++, j++)
        b->reg[j] = a->reg[i];
    b->qtd = a->qtd - m;
    a->qtd = m;
    b->prox = a->prox;
    a->prox = b;
    b->up = a->up;
    return b;
}

void inserir_interno(Node **raiz, Node *esq, Node *dir, const char *ch, uint32_t ordem) {
    Node *p = esq->up;
    if (!p) {
        Node *novo = criar_node(ordem, 0);
        strcpy(novo->keys[0], ch);
        novo->ptrs[0] = esq;
        novo->ptrs[1] = dir;
        novo->qtd = 1;
        esq->up = dir->up = novo;
        *raiz = novo;
        return;
    }

    int i = p->qtd - 1;
    while (i >= 0 && strcmp(ch, p->keys[i]) < 0) {
        strcpy(p->keys[i + 1], p->keys[i]);
        p->ptrs[i + 2] = p->ptrs[i + 1];
        i--;
    }
    strcpy(p->keys[i + 1], ch);
    p->ptrs[i + 2] = dir;
    p->qtd++;
    dir->up = p;

    if (p->qtd == ordem) {
        int m = p->qtd / 2;
        Node *q = criar_node(ordem, 0);
        int j = 0;
        for (int k = m + 1; k < (int)p->qtd; k++, j++) {
            strcpy(q->keys[j], p->keys[k]);
            q->ptrs[j] = p->ptrs[k];
            q->ptrs[j]->up = q;
        }
        q->ptrs[j] = p->ptrs[p->qtd];
        q->ptrs[j]->up = q;
        q->qtd = j;

        char sobe[HLEN];
        strcpy(sobe, p->keys[m]);
        p->qtd = m;

        inserir_interno(raiz, p, q, sobe, ordem);
    }
}

void inserir(Node **raiz, Arquivo a, uint32_t ordem) {
    Node *x = *raiz;
    while (!x->folha) {
        int i = 0;
        while (i < (int)x->qtd && strcmp(a.hash, x->keys[i]) >= 0) i++;
        x = x->ptrs[i];
    }

    int i = x->qtd - 1;
    while (i >= 0 && strcmp(a.hash, x->reg[i].hash) < 0) {
        x->reg[i + 1] = x->reg[i];
        i--;
    }
    x->reg[i + 1] = a;
    x->qtd++;

    if (x->qtd == ordem) {
        Node *y = dividir_folha(x, ordem);
        inserir_interno(raiz, x, y, y->reg[0].hash, ordem);
    }
}

void select_hash(Node *r, const char *h, FILE *output) {
    Node *x = r;
    while (!x->folha) {
        int i = 0;
        while (i < (int)x->qtd && strcmp(h, x->keys[i]) >= 0) i++;
        x = x->ptrs[i];
    }
    fprintf(output, "[%s]\n", h);

    int pos = -1;
    for (uint32_t i = 0; i < x->qtd; i++)
        if (!strcmp(h, x->reg[i].hash)) { pos = i; break; }

    if (pos == -1) {
        fprintf(output, "-\n");
        return;
    }

    for (uint32_t i = pos; i < x->qtd; i++)
        fprintf(output, "%s:size=%u,hash=%s\n",
                x->reg[i].nome, x->reg[i].tam, x->reg[i].hash);
}

void select_range(Node *r, const char *h1, const char *h2, FILE *output) {
    char ini[HLEN], fim[HLEN];
    if (strcmp(h1, h2) <= 0) { strcpy(ini, h1); strcpy(fim, h2); }
    else { strcpy(ini, h2); strcpy(fim, h1); }

    Node *f = r;
    while (!f->folha) {
        int i = 0;
        while (i < (int)f->qtd && strcmp(ini, f->keys[i]) >= 0) i++;
        f = f->ptrs[i];
    }

    fprintf(output, "[%s,%s]\n", ini, fim);

    int printed = 0, started = 0;
    for (; f; f = f->prox) {
        int ok = 0;
        for (uint32_t i = 0; i < f->qtd && !ok; i++)
            if (strcmp(f->reg[i].hash, ini) >= 0 &&
                strcmp(f->reg[i].hash, fim) <= 0)
                ok = 1;

        if (!started && !ok) continue;
        if (ok) started = 1;

        for (uint32_t i = 0; i < f->qtd; i++) {
            if (strcmp(f->reg[i].hash, fim) > 0) {
                if (!printed) fprintf(output, "-\n");
                return;
            }
            if (started) {
                fprintf(output, "%s:size=%u,hash=%s\n",
                        f->reg[i].nome, f->reg[i].tam, f->reg[i].hash);
                printed = 1;
            }
        }
    }
    if (!printed) fprintf(output, "-\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");
    if (!input || !output) { perror("fopen"); return 2; }

    uint32_t ordem, n;
    if (fscanf(input, "%u %u", &ordem, &n) != 2) return 3;

    Node *raiz = criar_node(ordem, 1);

    for (uint32_t i = 0; i < n; i++) {
        Arquivo a;
        if (fscanf(input, "%30s %u %32s", a.nome, &a.tam, a.hash) != 3) return 4;
        inserir(&raiz, a, ordem);
    }

    uint32_t m;
    if (fscanf(input, "%u", &m) != 1) return 5;

    for (uint32_t i = 0; i < m; i++) {
        char cmd[10];
        if (fscanf(input, "%9s", cmd) != 1) return 6;

        if (!strcmp(cmd, "INSERT")) {
            Arquivo a;
            if (fscanf(input, "%30s %u %32s", a.nome, &a.tam, a.hash) != 3) return 7;
            inserir(&raiz, a, ordem);
        } else if (!strcmp(cmd, "SELECT")) {
            char tok[HLEN];
            if (fscanf(input, "%32s", tok) != 1) return 8;

            if (!strcmp(tok, "RANGE")) {
                char h1[HLEN], h2[HLEN];
                if (fscanf(input, "%32s %32s", h1, h2) != 2) return 9;
                select_range(raiz, h1, h2, output);
            } else {
                select_hash(raiz, tok, output);
            }
        }
    }

    fclose(input);
    fclose(output);
    return 0;
}
