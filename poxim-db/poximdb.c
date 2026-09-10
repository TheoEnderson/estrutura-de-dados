/**
 * @file poximdb.c
 * @brief Sistema de banco de dados baseado em Árvore B (B-Tree).
 *
 * Implementa estrutura de Árvore B de ordem parametrizável para armazenamento
 * e busca rápida de registros com chave hash e controle de divisão de nós (split).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    char nome[31];
    uint32_t tamanho;
    char hash[33];
} Arquivo;

typedef struct no {
    Arquivo* registros;
    struct no** filhos;
    uint32_t qtd;
    int eh_folha;
} No;

No* novo_no(uint32_t ordem, int folha);
void liberar(No* raiz, uint32_t ordem);
void inserir(No** raiz, Arquivo arq, uint32_t ordem);
void inserir_interno(No* x, Arquivo arq, uint32_t ordem);
void dividir(No* pai, int idx, uint32_t ordem);
No* localizar(No* raiz, char* chave);



// Leitura e Execução

void carregar_iniciais(FILE* entrada, No** raiz, uint32_t ordem, uint32_t qtd);
void executar(FILE* entrada, FILE* saida, No** raiz, uint32_t ordem);

// Implementações

No* novo_no(uint32_t ordem, int folha) {
    No* n = malloc(sizeof(No));
    if (!n) {
        fprintf(stderr, "Red ao alocar nó\n");
        exit(EXIT_FAILURE);
    }

    n->registros = malloc((ordem - 1) * sizeof(Arquivo));
    if (!n->registros) {
        fprintf(stderr, "Red ao alocar vetor de registros\n");
        free(n);
        exit(EXIT_FAILURE);
    }

    n->filhos = malloc(ordem * sizeof(No*));
    if (!n->filhos) {
        fprintf(stderr, "Red ao alocar ponteiros de filhos\n");
        free(n->registros);
        free(n);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < ordem; i++)
        n->filhos[i] = NULL;

    n->qtd = 0;
    n->eh_folha = folha;
    return n;
}

void liberar(No* raiz, uint32_t ordem) {
    if (!raiz) return;
    if (!raiz->eh_folha)
        for (uint32_t i = 0; i <= raiz->qtd; i++)
            liberar(raiz->filhos[i], ordem);
    free(raiz->registros);
    free(raiz->filhos);
    free(raiz);
}

void dividir(No *pai, int idx, uint32_t ordem)
{
    No *y = pai->filhos[idx];                 /* nó a ser dividido   */
    No *z = novo_no(ordem, y->eh_folha);      /* novo irmão direito  */

    int meio = y->qtd / 2;                    /* posição do meio     */

    if (y->eh_folha) {
        /* -------- divisão de FOLHA -------- */
        /* quantidade que fica no irmão direito               */
        z->qtd = y->qtd - meio;               /* inclui a chave[meio]*/

        /* copia da chave[meio] até o fim para z               */
        for (int j = 0; j < z->qtd; ++j)
            z->registros[j] = y->registros[meio + j];

        y->qtd = meio;                        /* y fica com [0..meio-1] */

        /* coloca z entre os filhos do pai                     */
        for (int j = pai->qtd; j >= idx + 1; --j)
            pai->filhos[j + 1] = pai->filhos[j];
        pai->filhos[idx + 1] = z;

        /* *** separador é a 1ª chave do novo irmão direito *** */
        for (int j = pai->qtd - 1; j >= idx; --j)
            pai->registros[j + 1] = pai->registros[j];
        pai->registros[idx] = z->registros[0];

    } else {

        Arquivo sobe = y->registros[meio];     

        z->qtd = y->qtd - meio - 1;            
        for (int j = 0; j < z->qtd; ++j)
            z->registros[j] = y->registros[meio + 1 + j];
        for (int j = 0; j <= z->qtd; ++j)
            z->filhos[j]   = y->filhos[meio + 1 + j];
        y->qtd = meio;

        for (int j = pai->qtd; j >= idx + 1; --j)
            pai->filhos[j + 1] = pai->filhos[j];
        pai->filhos[idx + 1] = z;

        for (int j = pai->qtd - 1; j >= idx; --j)
            pai->registros[j + 1] = pai->registros[j];
        pai->registros[idx] = sobe;
    }

    pai->qtd++;
}



void inserir_interno(No* x, Arquivo arq, uint32_t ordem) {
    int i = x->qtd - 1;

    if (x->eh_folha) {
        while (i >= 0 && strcmp(arq.hash, x->registros[i].hash) < 0) {
            x->registros[i + 1] = x->registros[i];
            i--;
        }
        x->registros[i + 1] = arq;
        x->qtd++;
    } else {
        while (i >= 0 && strcmp(arq.hash, x->registros[i].hash) < 0) i--;
        i++;
        if (x->filhos[i]->qtd == ordem - 1) {
            dividir(x, i, ordem);
            if (strcmp(arq.hash, x->registros[i].hash) > 0)
                i++;
        }
        inserir_interno(x->filhos[i], arq, ordem);
    }
}

void inserir(No** raiz, Arquivo arq, uint32_t ordem) {
    if ((*raiz)->qtd == ordem - 1) {
        No* nova = novo_no(ordem, 0);
        nova->filhos[0] = *raiz;
        dividir(nova, 0, ordem);
        inserir_interno(nova, arq, ordem);
        *raiz = nova;
    } else {
        inserir_interno(*raiz, arq, ordem);
    }
}

No *localizar(No *x, char *hash)
{
    if (!x) return NULL;

    uint32_t i = 0;
    while (i < x->qtd && strcmp(hash, x->registros[i].hash) > 0)
        i++;

    if (x->eh_folha)
        return x;

    return localizar(x->filhos[i], hash);
}

void carregar_iniciais(FILE* entrada, No** raiz, uint32_t ordem, uint32_t qtd) {
    for (uint32_t i = 0; i < qtd; i++) {
        Arquivo a;
        if (fscanf(entrada, "%30s %u %32s", a.nome, &a.tamanho, a.hash) != 3) {
            fprintf(stderr, "Red ao ler arquivo inicial %u.\n", i + 1);
            exit(4);
        }
        inserir(raiz, a, ordem);
    }
}

void executar(FILE* entrada, FILE* saida, No** raiz, uint32_t ordem) {
    uint32_t ops;
    fscanf(entrada, "%u", &ops);

    for (uint32_t i = 0; i < ops; i++) {
        char cmd[10];
        fscanf(entrada, "%9s", cmd);

        if (!strcmp(cmd, "INSERT")) {
            Arquivo a;
            fscanf(entrada, "%30s %u %32s", a.nome, &a.tamanho, a.hash);
            inserir(raiz, a, ordem);
        } else if (!strcmp(cmd, "SELECT")) {
            char chave[33];
            fscanf(entrada, "%32s", chave);

            fprintf(saida, "[%s]\n", chave);

            No* no_alvo = localizar(*raiz, chave);
            int achou = 0;

            if (no_alvo) {
                for (uint32_t j = 0; j < no_alvo->qtd; j++) {
                    if (!strcmp(no_alvo->registros[j].hash, chave)) {
                        achou = 1;
                        break;
                    }
                }
            }

            if (achou) {
                for (uint32_t j = 0; j < no_alvo->qtd; j++) {
                    fprintf(saida, "%s:size=%u,hash=%s\n",
                            no_alvo->registros[j].nome,
                            no_alvo->registros[j].tamanho,
                            no_alvo->registros[j].hash);
                }
            } else {
                fprintf(saida, "-\n");
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso correto: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    if (!input || !output) {
        perror("Erro nos arquivos");
        return 2;
    }

    uint32_t ordem, total;
    if (fscanf(input, "%u %u", &ordem, &total) != 2) {
        fprintf(stderr, "Erro ao ler ordem e quantidade inicial de arquivos.\n");
        fclose(input);
        fclose(output);
        return 3;
    }

    No* raiz = novo_no(ordem, 1);
    carregar_iniciais(input, &raiz, ordem, total);
    executar(input, output, &raiz, ordem);

  

    liberar(raiz, ordem);
    fclose(input);
    fclose(output);
    return 0;
}

