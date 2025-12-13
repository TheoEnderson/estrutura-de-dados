#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TAM_NOME 51

typedef struct Mano {
    char nome[TAM_NOME];
    struct Mano *ant;
    struct Mano *prox;
} Mano;

typedef struct {
    Mano sent;
    uint32_t tamanho;
} Lista;

Lista *criar_lista(void) {
    Lista *l = malloc(sizeof(Lista));
    if (!l) {
        fprintf(stderr, "Deu red na alocação da lista.\n");
        return NULL;
    }
    l->sent.ant = l->sent.prox = &l->sent;
    l->sent.nome[0] = '\0';
    l->tamanho = 0;
    return l;
}

void mudar_nome(char *nome) {
    for (uint32_t i = 0; nome[i] != '\0'; ++i) {
        if (nome[i] == ' ')
            nome[i] = '_';
    }
}

Mano *buscar(Lista *l, const char *nome) {
    if (l->tamanho == 0)
        return NULL;

    for (Mano *p = l->sent.prox; p != &l->sent; p = p->prox) {
        if (strcmp(p->nome, nome) == 0)
            return p;
    }
    return NULL;
}

uint32_t adicionar_pessoa(Lista *l, const char *nome) {
    if (buscar(l, nome))
        return 0;

    Mano *nova = malloc(sizeof(Mano));
    if (!nova) {
        fprintf(stderr, "Deu red na alocação da nova pessoa.\n");
        return 0;
    }
    strncpy(nova->nome, nome, TAM_NOME - 1);
    nova->nome[TAM_NOME - 1] = '\0';

    Mano *ultimo = l->sent.ant;
    nova->prox = &l->sent;
    nova->ant = ultimo;
    ultimo->prox = nova;
    l->sent.ant = nova;
    l->tamanho++;
    return 1;
}

uint32_t remover_pessoa(Lista *l, const char *nome) {
    Mano *alvo = buscar(l, nome);
    if (!alvo)
        return 0;

    alvo->ant->prox = alvo->prox;
    alvo->prox->ant = alvo->ant;
    free(alvo);
    l->tamanho--;
    return 1;
}

void mostrar(Lista *l, FILE *saida, const char *nome) {
    Mano *p = buscar(l, nome);
    char nome_fmt[TAM_NOME];
    strncpy(nome_fmt, nome, TAM_NOME);
    nome_fmt[TAM_NOME - 1] = '\0';
    mudar_nome(nome_fmt);

    if (!p) {
        fprintf(saida, "[FAILURE]SHOW=?<-%s->?\n", nome_fmt);
        return;
    }

    char ant[TAM_NOME], prox[TAM_NOME];
    const char *ant_src = (l->tamanho == 1) ? p->nome :
        (p->ant == &l->sent) ? l->sent.ant->nome : p->ant->nome;
    const char *prox_src = (l->tamanho == 1) ? p->nome :
        (p->prox == &l->sent) ? l->sent.prox->nome : p->prox->nome;

    strncpy(ant, ant_src, TAM_NOME);
    strncpy(prox, prox_src, TAM_NOME);
    ant[TAM_NOME - 1] = prox[TAM_NOME - 1] = '\0';
    mudar_nome(ant);
    mudar_nome(prox);
    fprintf(saida, "[SUCCESS]SHOW=%s<-%s->%s\n", ant, nome_fmt, prox);
}

void realizar_tudo(FILE *entrada, FILE *saida, Lista *l) {
    char linha[200], comando[10], nome[TAM_NOME];

    while (fgets(linha, sizeof(linha), entrada)) {
        if (sscanf(linha, "%9s %50[^\n]", comando, nome) != 2)
            continue;

        if (strcmp(comando, "ADD") == 0) {
            uint32_t ok = adicionar_pessoa(l, nome);
            mudar_nome(nome);
            fprintf(saida, ok ? "[SUCCESS]ADD=%s\n" : "[FAILURE]ADD=%s\n", nome);
        } else if (strcmp(comando, "REMOVE") == 0) {
            uint32_t ok = remover_pessoa(l, nome);
            mudar_nome(nome);
            fprintf(saida, ok ? "[SUCCESS]REMOVE=%s\n" : "[FAILURE]REMOVE=%s\n", nome);
        } else if (strcmp(comando, "SHOW") == 0) {
            mostrar(l, saida, nome);
        }
    }
}

void free_lista(Lista *l) {
    Mano *p = l->sent.prox;
    while (p != &l->sent) {
        Mano *tmp = p;
        p = p->prox;
        free(tmp);
    }
    l->sent.ant = l->sent.prox = &l->sent;
    l->tamanho = 0;
}

int main(int argc, char *argv[]) {
    if ((uint32_t)argc != 3) {
        fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");
    if (!input || !output) {
        fprintf(stderr, "Deu red pra abrir os arquivos.\n");
        if (input) fclose(input);
        if (output) fclose(output);
        return EXIT_FAILURE;
    }

    Lista *lista = criar_lista();
    if (!lista) {
        fclose(input);
        fclose(output);
        return EXIT_FAILURE;
    }

    realizar_tudo(input, output, lista);

    free_lista(lista);
    free(lista);
    fclose(input);
    fclose(output);
    return EXIT_SUCCESS;
}
