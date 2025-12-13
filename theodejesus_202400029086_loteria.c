#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum { K_SORTEIO = 10, K_APOSTA = 15, K_MAXNUM = 50, K_COD = 96 };

typedef struct {
    char    cod[K_COD];
    int32_t acertos;
} Aposta;

void erro(const char* msg){ fputs(msg, stderr); exit(2); }

#if defined(__GNUC__) && !defined(_WIN32)
  #define GETC(f) getc_unlocked(f)
  #define PUTC(c,f) putc_unlocked((c),(f))
#else
  #define GETC(f) getc(f)
  #define PUTC(c,f) putc((c),(f))
#endif

void config_io_rapida(FILE* in, FILE* out){
    static char ibuf[1<<20], obuf[1<<20];
    setvbuf(in,  ibuf, _IOFBF, sizeof ibuf);
    setvbuf(out, obuf, _IOFBF, sizeof obuf);
}

int le_i32(FILE* f, int32_t* v){
    int c;
    do { c = GETC(f); if (c == EOF) return 0; } while (c <= ' ');
    int32_t x = 0;
    while (c > ' ') { x = x*10 + (c - '0'); c = GETC(f); }
    *v = x;
    return 1;
}

int le_token(FILE* f, char* buf, int cap){
    int c, k = 0;
    do { c = GETC(f); if (c == EOF) return 0; } while (c <= ' ');
    while (c > ' ') {
        if (k + 1 < cap) buf[k++] = (char)c;
        c = GETC(f);
    }
    buf[k] = '\0';
    return 1;
}

uint64_t liga_bit(uint64_t m, unsigned p){ return m | (1ull << p); }
int      teste_bit(uint64_t m, unsigned p){ return (int)((m >> p) & 1u); }

int32_t conta_acertos(uint64_t masc_sorteio, const int32_t *nums){
    uint64_t vistos = 0ull;
    int32_t s = 0;
    for (int32_t i = 0; i < K_APOSTA; ++i){
        int32_t x = nums[i];
        if ((uint32_t)x <= K_MAXNUM){
            unsigned pos = (unsigned)x;           
            uint64_t bit = 1ull << pos;
            if (!(vistos & bit)){                  
                vistos |= bit;
                if (masc_sorteio & bit) ++s;      
            }
        }
    }
    return s;
}

void ler_cabecalho(FILE* in, int32_t* premio, int32_t* qtd, uint64_t* masc_sorteio){
    if (!le_i32(in, premio)) erro("Entrada inválida (prêmio).\n");
    if (!le_i32(in, qtd) || *qtd < 0) erro("Entrada inválida (n).\n");

    *masc_sorteio = 0ull;
    for (int32_t i = 0; i < K_SORTEIO; ++i){
        int32_t x; if (!le_i32(in, &x)) erro("Entrada inválida (sorteado).\n");
        if (1 <= x && x <= K_MAXNUM) *masc_sorteio = liga_bit(*masc_sorteio, (unsigned)x);
    }
}

Aposta* ler_apostas_e_pontuacoes(FILE* in, int32_t qtd, uint64_t masc_sorteio){
    Aposta* V = (Aposta*)malloc((size_t)(qtd > 0 ? qtd : 1) * sizeof(Aposta));
    if (!V && qtd > 0) erro("Memória insuficiente.\n");

    int32_t buf[K_APOSTA];
    for (int32_t i = 0; i < qtd; ++i){
        if (!le_token(in, V[i].cod, K_COD)) erro("Entrada inválida (código).\n");
        for (int32_t j = 0; j < K_APOSTA; ++j)
            if (!le_i32(in, &buf[j])) erro("Entrada inválida (aposta).\n");
        V[i].acertos = conta_acertos(masc_sorteio, buf);
    }
    return V;
}

int esq (int i){ return 2*i + 1; }
int dir (int i){ return 2*i + 2; }

void heap_max_ajustar_idx(const Aposta* V, int32_t* idx, int32_t n, int32_t i){
    for(;;){
        int32_t l = esq(i), r = dir(i), p = i;
        if (l < n && V[idx[l]].acertos > V[idx[p]].acertos) p = l;
        if (r < n && V[idx[r]].acertos > V[idx[p]].acertos) p = r;
        if (p == i) break;
        int32_t tmp = idx[i]; idx[i] = idx[p]; idx[p] = tmp;
        i = p;
    }
}
void heap_max_constroi_idx(const Aposta* V, int32_t* idx, int32_t n){
    for (int32_t i = n/2 - 1; i >= 0; --i) heap_max_ajustar_idx(V, idx, n, i);
}

void heap_min_ajustar_idx(const Aposta* V, int32_t* idx, int32_t n, int32_t i){
    for(;;){
        int32_t l = esq(i), r = dir(i), p = i;
        if (l < n && V[idx[l]].acertos < V[idx[p]].acertos) p = l;
        if (r < n && V[idx[r]].acertos < V[idx[p]].acertos) p = r;
        if (p == i) break;
        int32_t tmp = idx[i]; idx[i] = idx[p]; idx[p] = tmp;
        i = p;
    }
}
void heap_min_constroi_idx(const Aposta* V, int32_t* idx, int32_t n){
    for (int32_t i = n/2 - 1; i >= 0; --i) heap_min_ajustar_idx(V, idx, n, i);
}

int32_t heap_max_pop_idx(const Aposta* V, int32_t* idx, int32_t* n){
    int32_t ret = idx[0];
    idx[0] = idx[*n - 1];
    (*n)--;
    if (*n > 0) heap_max_ajustar_idx(V, idx, *n, 0);
    return ret;
}
int32_t heap_min_pop_idx(const Aposta* V, int32_t* idx, int32_t* n){
    int32_t ret = idx[0];
    idx[0] = idx[*n - 1];
    (*n)--;
    if (*n > 0) heap_min_ajustar_idx(V, idx, *n, 0);
    return ret;
}

int32_t* novo_vetor_indices(int32_t n){
    int32_t* idx = (int32_t*)malloc((size_t)(n > 0 ? n : 1) * sizeof(int32_t));
    if (!idx && n > 0) erro("Memória insuficiente.\n");
    for (int32_t i = 0; i < n; ++i) idx[i] = i;
    return idx;
}

void imprimir_faixa(FILE* out, int32_t qtd, int32_t acertos, int32_t valor, char** cods){
    fprintf(out, "[%d:%d:%d]\n", qtd, acertos, valor);
    for (int32_t i = 0; i < qtd; ++i){
        fputs(cods[i], out);
        PUTC('\n', out);
    }
}

void dividir_premio_metade(int32_t premio, int32_t q_hi, int32_t q_lo,
                           int32_t* por_hi, int32_t* por_lo, int remainder_segunda){
    if (remainder_segunda){
        int32_t p1 = premio / 2;
        int32_t p2 = premio - p1;             
        *por_hi = (q_hi > 0 ? p1 / q_hi : 0);
        *por_lo = (q_lo > 0 ? p2 / q_lo : 0);
    } else {
        int32_t p1 = (premio + 1) / 2;        
        int32_t p2 = premio - p1;
        *por_hi = (q_hi > 0 ? p1 / q_hi : 0);
        *por_lo = (q_lo > 0 ? p2 / q_lo : 0);
    }
}

int main(int argc, char** argv){
    if (argc < 3){
        fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }
    FILE* in  = fopen(argv[1], "r");
    FILE* out = fopen(argv[2], "w");
    if (!in || !out){
        if (in) fclose(in);
        if (out) fclose(out);
        fprintf(stderr, "Falha ao abrir arquivo(s).\n");
        return 1;
    }

    config_io_rapida(in, out);

    int32_t premio, qtd;
    uint64_t masc_sorteio;
    ler_cabecalho(in, &premio, &qtd, &masc_sorteio);

    if (qtd == 0){
        fprintf(out, "[0:0:0]\n");
        fprintf(out, "[0:0:0]\n");
        fclose(in); fclose(out);
        return 0;
    }

    Aposta* V = ler_apostas_e_pontuacoes(in, qtd, masc_sorteio);

    int32_t* idx_max = novo_vetor_indices(qtd);
    int32_t* idx_min = novo_vetor_indices(qtd);
    heap_max_constroi_idx(V, idx_max, qtd);
    heap_min_constroi_idx(V, idx_min, qtd);

    int32_t nmax = qtd;
    int32_t topo_hits_hi = V[idx_max[0]].acertos;

    char** venc_hi = (char**)malloc((size_t)qtd * sizeof(char*));
    if (!venc_hi) erro("Memória insuficiente.\n");
    int32_t qtd_hi = 0;

    while (nmax > 0 && V[idx_max[0]].acertos == topo_hits_hi){
        int32_t j = heap_max_pop_idx(V, idx_max, &nmax);
        venc_hi[qtd_hi++] = (char*)V[j].cod;
    }

    int32_t nmin = qtd;
    int32_t topo_hits_lo = V[idx_min[0]].acertos;

    char** venc_lo = (char**)malloc((size_t)qtd * sizeof(char*));
    if (!venc_lo) erro("Memória insuficiente.\n");
    int32_t qtd_lo = 0;

    while (nmin > 0 && V[idx_min[0]].acertos == topo_hits_lo){
        int32_t j = heap_min_pop_idx(V, idx_min, &nmin);
        venc_lo[qtd_lo++] = (char*)V[j].cod;
    }

    int32_t por_hi = 0, por_lo = 0;
    dividir_premio_metade(premio, qtd_hi, qtd_lo, &por_hi, &por_lo, 1);

    imprimir_faixa(out, qtd_hi, topo_hits_hi, por_hi, venc_hi);
    imprimir_faixa(out, qtd_lo, topo_hits_lo, por_lo, venc_lo);

    free(venc_hi);
    free(venc_lo);
    free(idx_max);
    free(idx_min);
    free(V);
    fclose(in);
    fclose(out);
    return 0;
}
