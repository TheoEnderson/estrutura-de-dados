#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_DE_AUTOR 51
#define MAX_DE_TITULO 101

typedef struct {
    unsigned long long isbn;
    char autor[MAX_DE_AUTOR];
    char titulo[MAX_DE_TITULO];
} Registro;

void insertionISBN(Registro *livros, uint32_t n) {
    for (uint32_t i = 1; i < n; i++) {
        Registro temp = livros[i];
        int32_t j = i - 1;
        while (j >= 0 && livros[j].isbn > temp.isbn) {
            livros[j + 1] = livros[j];
            j--;
        }
        livros[j + 1] = temp;
    }
}

int32_t buscaBin(const Registro *lista, uint32_t qtd, unsigned long long alvo, uint32_t *passos) {
    int32_t menor = 0, maior = qtd - 1;
    *passos = 0;
    while (menor <= maior) {
        (*passos)++;
        int32_t meio = (menor + maior) / 2;
        unsigned long long valor = lista[meio].isbn;
        if (valor == alvo)
            return meio;
        else if (valor < alvo)
            menor = meio + 1;
        else
            maior = meio - 1;
    }
    (*passos)++;  
    return -1;
}

int32_t buscaInterp(const Registro *livros, uint32_t total, unsigned long long isbnAlvo, uint32_t *etapas) {
    int32_t inicio = 0, fim = total - 1;
    *etapas = 0;

    while (inicio <= fim) {
        (*etapas)++; 

       /*  if (isbnAlvo < livros[inicio].isbn || isbnAlvo > livros[fim].isbn)
            break; 
        */

        if (livros[inicio].isbn == livros[fim].isbn) {
            return (livros[inicio].isbn == isbnAlvo) ? inicio : -1;
        }

        int32_t range = fim - inicio + 1;
        unsigned long long delta = livros[fim].isbn - livros[inicio].isbn;
        int32_t palpite = inicio + (delta % range);

        if (palpite < inicio || palpite > fim)
            break;

        if (livros[palpite].isbn == isbnAlvo)
            return palpite;
        else if (livros[palpite].isbn < isbnAlvo)
            inicio = palpite + 1;
        else
            fim = palpite - 1;
    }
    return -1;
}


void exibirLivro(FILE *output, const Registro *livro) {
    fprintf(output, "Author:");
    for (uint32_t i = 0; livro->autor[i]; i++)
        fputc(livro->autor[i] == ' ' ? '_' : livro->autor[i], output);

    fprintf(output, ",Title:");
    for (uint32_t i = 0; livro->titulo[i]; i++)
        fputc(livro->titulo[i] == ' ' ? '_' : livro->titulo[i], output);
}

int main(int argc, char *argv[]) {
    if ((uint32_t)argc != 3) {
        fprintf(stderr, "Uso correto: %s input.txt output.txt\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");
    if (!input || !output) {
        perror("Erro ao abrir arquivos");
        return 1;
    }

    uint32_t totalLivros;
    if (fscanf(input, "%u\n", &totalLivros) != 1) {
        fprintf(stderr, "Erro ao ler o número de livros.\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    Registro *colecao = malloc(totalLivros * sizeof(Registro));
    if (!colecao) {
        fprintf(stderr, "Erro: Faltou memoria ao tentar alocar %u livros (%zu bytes).\n", totalLivros, totalLivros * sizeof(Registro));
        fclose(input);
        fclose(output);
        return 1;
    }

    char *linha = NULL;
    size_t tamanho = 0;

    for (uint32_t i = 0; i < totalLivros; i++) {
        if (getline(&linha, &tamanho, input) == -1) {
            fprintf(stderr, "Erro ao ler linha do livro %u.\n", i+1);
            free(colecao);
            free(linha);
            fclose(input);
            fclose(output);
            return 1;
        }

        char *espaco = strchr(linha, ' ');
        char *ecomercial = strrchr(linha, '&');

        if (!espaco || !ecomercial) {
            fprintf(stderr, "Erro ao processar linha do livro %u.\n", i+1);
            free(colecao);
            free(linha);
            fclose(input);
            fclose(output);
            return 1;
        }

        colecao[i].isbn = strtoull(linha, NULL, 10);
        *ecomercial = '\0';

        while (*(espaco + 1) == ' ') espaco++;

        strncpy(colecao[i].autor, espaco + 1, MAX_DE_AUTOR - 1);
        colecao[i].autor[MAX_DE_AUTOR - 1] = '\0';

        strncpy(colecao[i].titulo, ecomercial + 1, MAX_DE_TITULO - 1);
        colecao[i].titulo[MAX_DE_TITULO - 1] = '\0';
        colecao[i].titulo[strcspn(colecao[i].titulo, "\n")] = '\0';
    }

    insertionISBN(colecao, totalLivros);

    uint32_t totalConsultas;
    if (fscanf(input, "%u\n", &totalConsultas) != 1) {
        fprintf(stderr, "Erro ao ler o número de consultas.\n");
        free(colecao);
        free(linha);
        fclose(input);
        fclose(output);
        return 1;
    }

    long long somaBin = 0, somaInterp = 0;
    uint32_t ganhoBin = 0, ganhoInterp = 0;

    for (uint32_t i = 0; i < totalConsultas; i++) {
        unsigned long long consulta;
        if (fscanf(input, "%llu\n", &consulta) != 1) {
            fprintf(stderr, "Erro ao ler consulta %u.\n", i+1);
            break;
        }

        uint32_t compBin, compInterp;
        int32_t posBin = buscaBin(colecao, totalLivros, consulta, &compBin);
        int32_t posInterp = buscaInterp(colecao, totalLivros, consulta, &compInterp);

        somaBin += compBin;
        somaInterp += compInterp;

        if (compBin < compInterp)
            ganhoBin++;
        else
            ganhoInterp++; 

        fprintf(output, "[%013llu]B=%u|I=%u|", consulta, compBin, compInterp);
        if (posInterp >= 0)
            exibirLivro(output, &colecao[posInterp]);
        else
            fputs("ISBN_NOT_FOUND", output);

        fputc('\n', output);
    }

    fprintf(output, "BINARY=%u:%lld\n", ganhoBin, somaBin / totalConsultas);
    fprintf(output, "INTERPOLATION=%u:%lld\n", ganhoInterp, somaInterp / totalConsultas);

    free(colecao);
    free(linha);
    fclose(input);
    fclose(output);
    return 0;
}
