#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum { TAM_NOME = 64 };

typedef struct {
    char    nome[TAM_NOME];
    int32_t prioridade;   
    int32_t idade;
    int32_t ordem;      
} Pessoa;

typedef struct {
    Pessoa** heap;      
    int32_t  tam;
    int32_t  cap;
} FilaPrioridade;

typedef struct {
    char           nome[TAM_NOME];
    int32_t        atendentes;
    FilaPrioridade fila;
} Orgao;

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
    *v = x; return 1;
}

int le_token_simples(FILE* f, char* buf, int cap){
    int c, k = 0;
    do { c = GETC(f); if (c == EOF) return 0; } while (c <= ' ');
    while (c > ' '){
        if (k + 1 < cap) buf[k++] = (char)c;
        c = GETC(f);
    }
    buf[k] = '\0';
    return 1;
}

int le_linha(FILE* f, char* buf, int cap){
    int c, k = 0, lido = 0;
    while ((c = GETC(f)) != EOF){
        lido = 1;
        if (c == '\n') break;
        if (k + 1 < cap) buf[k++] = (char)c;
    }
    if (!lido) return 0;
    if (k > 0 && buf[k-1] == '\r') k--;
    buf[k] = '\0';
    return 1;
}

void trim(char* s){
    int i = 0, j = (int)strlen(s);
    while (i < j && s[i]  <= ' ') i++;
    while (j > i && s[j-1]<= ' ') j--;
    if (i > 0) memmove(s, s+i, (size_t)(j - i));
    s[j - i] = '\0';
}

int split_org_nome_idade(char* linha, char** org, char** nome, char** idade){
    char* p1 = strchr(linha, '|'); if (!p1) return 0;
    char* p2 = strchr(p1+1,   '|'); if (!p2) return 0;
    *p1 = '\0'; *p2 = '\0';
    *org  = linha;
    *nome = p1 + 1;
    *idade= p2 + 1;
    trim(*org); trim(*nome); trim(*idade);
    return 1;
}

void imprime_nome_formatado(FILE* out, const char* s){
    for (; *s; ++s) PUTC(*s == ' ' ? '_' : *s, out);
}

int maior_que(const Pessoa* a, const Pessoa* b){
    if (a->prioridade != b->prioridade) return a->prioridade > b->prioridade;
    return a->ordem < b->ordem; 
}

void fila_inicializar(FilaPrioridade* F){
    F->heap = NULL; F->tam = 0; F->cap = 0;
}
void fila_reservar(FilaPrioridade* F, int32_t novo){
    if (novo <= F->cap) return;
    int32_t nc = F->cap ? F->cap*2 : 8;
    if (nc < novo) nc = novo;
    Pessoa** h = (Pessoa**)realloc(F->heap, (size_t)nc * sizeof(Pessoa*));
    if (!h) erro("Memória insuficiente.\n");
    F->heap = h; F->cap = nc;
}
void heap_sobe(FilaPrioridade* F, int32_t i){
    while (i > 0){
        int32_t p = (i - 1) / 2;
        if (!maior_que(F->heap[i], F->heap[p])) break;
        Pessoa* tmp = F->heap[i]; F->heap[i] = F->heap[p]; F->heap[p] = tmp;
        i = p;
    }
}
void heap_desce(FilaPrioridade* F, int32_t i){
    for(;;){
        int32_t l = 2*i + 1, r = 2*i + 2, m = i;
        if (l < F->tam && maior_que(F->heap[l], F->heap[m])) m = l;
        if (r < F->tam && maior_que(F->heap[r], F->heap[m])) m = r;
        if (m == i) break;
        Pessoa* tmp = F->heap[i]; F->heap[i] = F->heap[m]; F->heap[m] = tmp;
        i = m;
    }
}
void fila_push(FilaPrioridade* F, Pessoa* P){
    fila_reservar(F, F->tam + 1);
    F->heap[F->tam] = P;
    heap_sobe(F, F->tam);
    F->tam++;
}
Pessoa* fila_pop(FilaPrioridade* F){
    if (F->tam == 0) return NULL;
    Pessoa* topo = F->heap[0];
    F->tam--;
    if (F->tam > 0){
        F->heap[0] = F->heap[F->tam];
        heap_desce(F, 0);
    }
    return topo;
}

int busca_orgao(Orgao* orgs, int32_t n, const char* nome){
    for (int32_t i = 0; i < n; ++i) if (strcmp(orgs[i].nome, nome) == 0) return (int)i;
    return -1;
}

Orgao* carregar_orgaos(FILE* in, int32_t* n_out){
    int32_t n_org = 0;
    if (!le_i32(in, &n_org) || n_org <= 0) erro("Entrada inválida (n órgãos).\n");

    Orgao* orgs = (Orgao*)calloc((size_t)n_org, sizeof(Orgao));
    if (!orgs) erro("Memória insuficiente.\n");

    for (int32_t i = 0; i < n_org; ++i){
        if (!le_token_simples(in, orgs[i].nome, TAM_NOME)) erro("Órgão inválido.\n");
        if (!le_i32(in, &orgs[i].atendentes) || orgs[i].atendentes <= 0) erro("Atendentes inválido.\n");
        fila_inicializar(&orgs[i].fila);
    }
    *n_out = n_org;
    return orgs;
}

Pessoa* carregar_pessoas(FILE* in, Orgao* orgs, int32_t n_org, int32_t* m_out){
    int32_t m = 0;
    if (!le_i32(in, &m) || m < 0) erro("Entrada inválida (m pessoas).\n");

    Pessoa* pessoas = (Pessoa*)malloc((size_t)m * sizeof(Pessoa));
    if (!pessoas) erro("Memória insuficiente.\n");

    char linha[256];
    int32_t ordem = 0;

    for (int32_t i = 0; i < m; ++i){
        if (!le_linha(in, linha, (int)sizeof(linha))) erro("Linha de pessoa inválida.\n");
        if (linha[0] == '\0'){ i--; continue; } /* ignora em branco */

        char *org, *nome, *idade_s;
        if (!split_org_nome_idade(linha, &org, &nome, &idade_s)) erro("Formato Órgão|Nome|Idade inválido.\n");

        int idx = busca_orgao(orgs, n_org, org);
        if (idx < 0) erro("Órgão não cadastrado.\n");

        Pessoa* P = &pessoas[ordem];
        strncpy(P->nome, nome, TAM_NOME-1); P->nome[TAM_NOME-1] = '\0';

        int32_t idade = 0;
        const char* s = idade_s; while (*s == ' ') s++;
        while (*s >= '0' && *s <= '9'){ idade = idade*10 + (*s - '0'); s++; }

        P->idade = idade;
        P->prioridade = (idade >= 60) ? 1 : 0;
        P->ordem = ordem++;

        fila_push(&orgs[idx].fila, P);
    }

    *m_out = m;
    return pessoas;
}

void simular_atendimento(FILE* out, Orgao* orgs, int32_t n_org){
    int32_t cap = 1;
    for (int32_t i = 0; i < n_org; ++i)
        if (orgs[i].atendentes > cap) cap = orgs[i].atendentes;
    if (cap < 1) cap = 1;

    Pessoa** chamados = (Pessoa**)malloc((size_t)cap * sizeof(Pessoa*));
    if (!chamados) erro("Memória insuficiente.\n");

    int ativos;
    do{
        ativos = 0;
        for (int32_t i = 0; i < n_org; ++i){
            FilaPrioridade* F = &orgs[i].fila;
            if (F->tam == 0) continue;
            ativos = 1;

            int k = orgs[i].atendentes;
            int usados = 0;
            for (int j = 0; j < k && F->tam > 0; ++j)
                chamados[usados++] = fila_pop(F);

            fputs(orgs[i].nome, out);
            PUTC(':', out);
            for (int j = 0; j < usados; ++j){
                if (j) PUTC(',', out);
                imprime_nome_formatado(out, chamados[j]->nome);
            }
            PUTC('\n', out);
        }
    } while (ativos);

    free(chamados);
}

void liberar_sistema(Orgao* orgs, int32_t n_org, Pessoa* pessoas){
    for (int32_t i = 0; i < n_org; ++i) free(orgs[i].fila.heap);
    free(orgs);
    free(pessoas);
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

    int32_t n_org=0, m_total=0;
    Orgao*  orgs    = carregar_orgaos(in, &n_org);
    Pessoa* pessoas = carregar_pessoas(in, orgs, n_org, &m_total);

    simular_atendimento(out, orgs, n_org);

    liberar_sistema(orgs, n_org, pessoas);
    fclose(in); fclose(out);
    return 0;
}
