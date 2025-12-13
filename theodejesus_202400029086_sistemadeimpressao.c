#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TAM_STR 51

struct Documento {
    char nome[TAM_STR];
    uint32_t paginas;
};

struct FilaImpressao {
    int32_t topo;
    uint32_t *documentos;
    uint32_t limite;
};

struct Conclusao {
    uint32_t doc_idx;
    uint32_t tempo;
};

void inicializaFila(struct FilaImpressao* f, uint32_t tamanho);
int empilhaDoc(struct FilaImpressao* f, uint32_t id_doc);
uint32_t selecionaImpressora(uint32_t imp_total, uint32_t tempos[]);
void escreveFila(FILE* output, uint32_t imp_id, char** imp_nomes,
                 struct FilaImpressao* fila, struct Documento docs[]);
void ordenaConclusoes(struct Conclusao arr[], uint32_t tam);

void inicializaFila(struct FilaImpressao* f, uint32_t tamanho) {
    f->topo = -1;
    f->limite = tamanho;
    f->documentos = (uint32_t*) malloc(tamanho * sizeof(uint32_t));
    if (!f->documentos) {
        fprintf(stderr, "Erro: malloc fila documentos (tamanho=%u) falhou\n", tamanho);
        exit(EXIT_FAILURE);
    }
}

int empilhaDoc(struct FilaImpressao* f, uint32_t id_doc) {
    if (f->topo >= (int32_t)f->limite - 1) {
        uint32_t nova_lim = f->limite * 2;
        uint32_t *tmp = (uint32_t*) realloc(f->documentos, nova_lim * sizeof(uint32_t));
        if (!tmp) {
            fprintf(stderr, "Erro: realloc fila de %u para %u itens falhou\n", f->limite, nova_lim);
            return 0;
        }
        f->documentos = tmp;
        f->limite = nova_lim;
    }
    f->documentos[++f->topo] = id_doc;
    return 1;
}

uint32_t selecionaImpressora(uint32_t imp_total, uint32_t tempos[]) {
    uint32_t menor = tempos[0], id = 0;
    for (uint32_t i = 1; i < imp_total; i++) {
        if (tempos[i] < menor) {
            menor = tempos[i];
            id = i;
        }
    }
    return id;
}

void escreveFila(FILE* output, uint32_t imp_id, char** imp_nomes,
                 struct FilaImpressao* fila, struct Documento docs[]) {
    fprintf(output, "%s:", imp_nomes[imp_id]);
    for (int32_t i = fila->topo; i >= 0; i--) {
        uint32_t d = fila->documentos[i];
        fprintf(output, "%s-%up", docs[d].nome, docs[d].paginas);
        if (i > 0) fputc(',', output);
    }
    fputc('\n', output);
}

void ordenaConclusoes(struct Conclusao arr[], uint32_t tam) {
    for (uint32_t i = 1; i < tam; i++) {
        struct Conclusao chave = arr[i];
        int j = i - 1;
        while (j >= 0 && (arr[j].tempo > chave.tempo ||
                         (arr[j].tempo == chave.tempo && arr[j].doc_idx < chave.doc_idx))) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = chave;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <input> <output>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");
    if (!input || !output) {
        perror("Deu red ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    uint32_t imp_total, doc_total;
    if (fscanf(input, "%u", &imp_total) != 1) {
        fprintf(stderr, "Red: leitura do número de impressoras falhou\n");
        exit(EXIT_FAILURE);
    }

    char** nomes_imp = (char**) malloc(imp_total * sizeof(char*));
    if (!nomes_imp) {
        fprintf(stderr, "Red: malloc nomes_imp[%u] falhou\n", imp_total);
        exit(EXIT_FAILURE);
    }
    struct FilaImpressao* filas = (struct FilaImpressao*) malloc(imp_total * sizeof(*filas));
    if (!filas) {
        fprintf(stderr, "Red: malloc filas[%u] falhou\n", imp_total);
        exit(EXIT_FAILURE);
    }
    uint32_t* disponivel = (uint32_t*) calloc(imp_total, sizeof(uint32_t));
    if (!disponivel) {
        fprintf(stderr, "Red: calloc disponivel[%u] falhou\n", imp_total);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < imp_total; i++) {
        nomes_imp[i] = (char*) malloc(TAM_STR);
        if (!nomes_imp[i]) {
            fprintf(stderr, "Red: malloc nome_imp[%u] falhou\n", i);
            exit(EXIT_FAILURE);
        }
        fscanf(input, "%50s", nomes_imp[i]);
    }

    if (fscanf(input, "%u", &doc_total) != 1) {
        fprintf(stderr, "Red: leitura do número de documentos falhou\n");
        exit(EXIT_FAILURE);
    }

    struct Documento* docs = (struct Documento*) malloc(doc_total * sizeof(*docs));
    if (!docs) {
        fprintf(stderr, "Red: malloc docs[%u] falhou\n", doc_total);
        exit(EXIT_FAILURE);
    }
    struct Conclusao* concluidos = (struct Conclusao*) malloc(doc_total * sizeof(*concluidos));
    if (!concluidos) {
        fprintf(stderr, "Red: malloc concluidos[%u] falhou\n", doc_total);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < doc_total; i++) {
        fscanf(input, "%50s %u", docs[i].nome, &docs[i].paginas);
    }

    for (uint32_t i = 0; i < imp_total; i++) {
        inicializaFila(&filas[i], doc_total);
    }

    uint32_t total_paginas = 0;
    for (uint32_t d = 0; d < doc_total; d++) {
        uint32_t ip = selecionaImpressora(imp_total, disponivel);
        disponivel[ip] += docs[d].paginas;

        concluidos[d].doc_idx = d;
        concluidos[d].tempo   = disponivel[ip];
        total_paginas        += docs[d].paginas;

        if (!empilhaDoc(&filas[ip], d)) {
            fprintf(stderr, "Red: não foi possível empilhar doc %u na fila %u\n", d, ip);
            exit(EXIT_FAILURE);
        }
        escreveFila(output, ip, nomes_imp, &filas[ip], docs);
    }

    fprintf(output, "%up\n", total_paginas);
    ordenaConclusoes(concluidos, doc_total);
    for (int32_t i = doc_total - 1; i >= 0; i--) {
        uint32_t idx = concluidos[i].doc_idx;
        fprintf(output, "%s-%up\n", docs[idx].nome, docs[idx].paginas);
    }

    for (uint32_t i = 0; i < imp_total; i++) {
        free(nomes_imp[i]);
        free(filas[i].documentos);
    }
    free(nomes_imp);
    free(filas);
    free(disponivel);
    free(docs);
    free(concluidos);
    fclose(input);
    fclose(output);

    return EXIT_SUCCESS;
}
