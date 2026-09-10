# 📚 Estruturas de Dados em C

![C](https://img.shields.io/badge/Language-C99%20%2F%20C11-00599C?logo=c&logoColor=white)
![GCC](https://img.shields.io/badge/Compiler-GCC-brightgreen?logo=gnu)
![Linux](https://img.shields.io/badge/Platform-Linux-FCC624?logo=linux&logoColor=black)
![Windows](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white)
![Status](https://img.shields.io/badge/Status-Completed-success)
![License](https://img.shields.io/badge/License-MIT-blue)

Coleção abrangente de implementações manuais de **Estruturas de Dados Clássicas e Avançadas** desenvolvidas em linguagem C. O projeto enfatiza eficiência algorítmica, gerenciamento estrito de memória (`malloc`/`free`), controle assintótico de complexidade (Big-O) e rotinas otimizadas de I/O em arquivos.

---

## 📑 Sumário

- [Visão Geral](#-visão-geral)
- [Projetos e Estruturas Implementadas](#-projetos-e-estruturas-implementadas)
- [Como Compilar e Executar](#-como-compilar-e-executar)
  - [Compilação no Linux (GCC / Clang)](#compilação-no-linux-gcc--clang)
  - [Compilação no Windows (GCC via MinGW / MSYS2)](#compilação-no-windows-gcc-via-mingw--msys2)
- [Exemplos Práticos de Execução](#-exemplos-práticos-de-execução)
- [Profiling e Análise de Desempenho](#-profiling-e-análise-de-desempenho)
- [Padrões e Boas Práticas](#-padrões-e-boas-práticas)
- [Licença](#-licença)

---

## 🧠 Visão Geral

Cada diretório implementa uma estrutura de dados voltada à resolução de um desafio prático de computação, tais como balanceamento de carga, indexação em árvores balanceadas e multidirecionais, gerenciamento de filas de atendimento e apuração em larga escala com Heaps e bitsets.

Principais fundamentos abordados:
- **Alocação Dinâmica e Gerenciamento de Memória**: Estruturas auto-expansíveis, árvores balanceadas e prevenção de *memory leaks*.
- **Estruturas Hierárquicas e Auto-Balanceamento**: Árvores Binárias de Busca (BST), Árvores AVL com cálculo de fator e rotações, Árvores B e B+ com divisão e encadeamento sequencial.
- **Estruturas de Seleção e Prioridade**: Filas de Prioridade baseadas em Min-Heap e Max-Heap com I/O de alta velocidade via buffers de 1 MB.
- **Tabelas Hash e Espalhamento**: Funções hash polinomiais utilizando números primos e endereçamento fechado com redistribuição em servidores.

---

## 📂 Projetos e Estruturas Implementadas

| Diretório | Estrutura de Dados | Complexidade (Tempo / Espaço) | Descrição do Caso de Uso |
| :--- | :--- | :--- | :--- |
| [`arvore-binaria/`](./arvore-binaria/) | **Árvore Binária de Busca (BST)** | Busca: $O(\log n)$ médio, $O(n)$ pior<br>Espaço: $O(n)$ | Indexação de registros com ordenação lexicográfica e percursos em pré-ordem, em-ordem e pós-ordem. |
| [`autocompletar/`](./autocompletar/) | **Árvore Trie / Prefixo** | Busca: $O(k)$ ($k$ = comprimento da palavra)<br>Espaço: $O(\Sigma \cdot k)$ | Sistema de consulta e autocompletar textual baseado em prefixos e arquivos de entrada/saída. |
| [`biblioteca/`](./biblioteca/) | **Insertion Sort & Busca Binária** | Ordenação: $O(n^2)$<br>Busca: $O(\log n)$ | Catálogo bibliográfico indexado por ISBN com métricas de passos e análise de complexidade empírica. |
| [`controle-de-senha/`](./controle-de-senha/) | **Fila de Prioridade (Heap Binário)** | Inserção: $O(\log n)$<br>Remoção Máx: $O(\log n)$ | Gerenciamento de múltiplos órgãos de atendimento com desempate por prioridade legal, idade e ordem de chegada. |
| [`dicionario/`](./dicionario/) | **Árvore AVL Auto-Balanceada** | Busca/Inserção: $O(\log n)$ rigoroso<br>Espaço: $O(n)$ | Dicionário de termos e sinônimos com rotações simples/duplas para garantia estrita de balanceamento. |
| [`engenho-de-busca/`](./engenho-de-busca/) | **Tabela Hash / Hashing Distribuído** | Média: $O(1)$ inserção e busca<br>Pior: $O(n)$ | Balanceador de requisições web utilizando números primos ($P_1, P_2$) e distribuição por hashing polinomial. |
| [`loteria/`](./loteria/) | **Min-Heap & Max-Heap + Bitsets** | Construção: $O(n)$<br>Apuração: $O(1)$ por aposta | Apuração massiva de cartelas de loteria utilizando mascaramento de bits (bitsets) e seleção dos maiores pontuadores via Heaps. |
| [`poxim-db/`](./poxim-db/) | **Árvore B (`poximdb.c`) e Árvore B+ (`poximdbplus.c`)** | Busca: $O(\log_m n)$<br>Varredura por Faixa: $O(k + \log_m n)$ | Motor de banco de dados para armazenamento de metadados de arquivos com nós parametrizáveis e nós-folha encadeados. |
| [`propagacao/`](./propagacao/) | **Grafos / Propagação em Redes** | Travessia: $O(V + E)$<br>Espaço: $O(V + E)$ | Modelagem e análise de fluxos de propagação de sinais/mensagens entre nós interconectados em uma rede. |
| [`rede-social/`](./rede-social/) | **Lista Duplamente Encadeada Circular** | Inserção/Remoção nas pontas: $O(1)$<br>Busca: $O(n)$ | Gerenciador de círculo social utilizando nó sentinela para eliminar casos de borda em nós nulos. |
| [`sistema-de-impressao/`](./sistema-de-impressao/) | **Filas (FIFO) & Pilhas (LIFO)** | Enfileirar/Desenfileirar: $O(1)$<br>Espaço: $O(n)$ | Escalonador de filas de impressão com estimativa de tempo e distribuição balanceada entre múltiplas impressoras. |

---

## 🛠 Como Compilar e Executar

Todos os códigos são estritamente compatíveis com os padrões **C99** e **C11**, sem dependências externas além da biblioteca padrão (`libc`).

### Flags Recomendadas do Compilador

```bash
-Wall -Wextra -pedantic -std=c99 -O2
```

- `-Wall -Wextra -pedantic`: Ativa avisos detalhados e conformidade estrita com o padrão ISO C.
- `-O2`: Habilita otimizações para máximo desempenho assintótico.
- `-std=c99`: Assegura compatibilidade entre ambientes Linux e Windows.

---

### Compilação no Linux (GCC / Clang)

Abra o terminal e navegue até a pasta do projeto desejado:

```bash
# Exemplo 1: Dicionário (Árvore AVL)
cd dicionario
gcc -Wall -Wextra -std=c99 -O2 dicionario.c -o dicionario
./dicionario <arquivo_entrada> <arquivo_saida>

# Exemplo 2: PoximDB (Árvore B)
cd ../poxim-db
gcc -Wall -Wextra -std=c99 -O2 poximdb.c -o poximdb
./poximdb <arquivo_entrada> <arquivo_saida>

# Exemplo 3: PoximDB Plus (Árvore B+)
gcc -Wall -Wextra -std=c99 -O2 poximdbplus.c -o poximdbplus
./poximdbplus <arquivo_entrada> <arquivo_saida>

# Exemplo 4: Controle de Senhas (Fila de Prioridade)
cd ../controle-de-senha
gcc -Wall -Wextra -std=c99 -O2 controle_de_senha.c -o controle_de_senha
./controle_de_senha <arquivo_entrada> <arquivo_saida>
```

---

### Compilação no Windows (GCC via MinGW / MSYS2)

No PowerShell ou CMD, com o `gcc` configurado nas variáveis de ambiente:

```powershell
# Exemplo: Loterias (Heaps & Bitsets)
cd loteria
gcc -Wall -Wextra -std=c99 -O2 loteria.c -o loteria.exe
.\loteria.exe entrada.txt saida.txt

# Exemplo: Rede Social (Lista Duplamente Encadeada Circular)
cd ..\rede-social
gcc -Wall -Wextra -std=c99 -O2 rede_social.c -o rede_social.exe
.\rede_social.exe entrada.txt saida.txt
```

---

## 💡 Exemplos Práticos de Execução

A maioria dos programas foi estruturada para operar com leitura e escrita através de argumentos de linha de comando:

```bash
./executavel <caminho_entrada> <caminho_saida>
```

### Exemplo: Teste do Módulo de Autocompletar
O diretório [`autocompletar/`](./autocompletar/) inclui amostras de teste pré-configuradas:

```bash
cd autocompletar
gcc -Wall -Wextra -std=c99 -O2 autocompletar.c -o autocompletar
./autocompletar entrada.txt saida.txt

# Verificando a conformidade com a saída esperada (Linux)
diff -u saida.txt esperada.txt
```

---

## 📊 Profiling e Análise de Desempenho

Para auditar o tempo de execução e uso de recursos das implementações, recomenda-se o uso do `gprof`:

```bash
# Compilar com suporte a instrumentação gprof
gcc -pg -O2 biblioteca.c -o biblioteca

# Executar para gerar o relatório gmon.out
./biblioteca entrada.txt saida.txt

# Gerar arquivo de análise legível
gprof biblioteca gmon.out > analise.txt
```

O diretório [`biblioteca/`](./biblioteca/) contém um exemplo de relatório de profiling (`analise.txt`) detalhando tempos cumulativos por rotina.

---

## 🏆 Padrões e Boas Práticas

- **Tratamento de I/O de Alta Performance**: Utilização de buffers estáticos expandidos (`setvbuf(..., 1<<20)`) e leitura desbloqueada (`getc_unlocked`) em problemas de grande volume de dados.
- **Zero Vazamentos de Memória**: Todas as rotinas que utilizam `malloc`/`calloc` possuem correspondente liberação recursiva ou iterativa via `free()`.
- **Portabilidade**: Código limpo com diretivas condicionais (`#if defined(__GNUC__) && !defined(_WIN32)`) para garantir compilação idêntica em Linux e Windows.
- **Estrutura Modular**: Arquivos organizados de forma independente, facilitando reutilização e testes unitários.

---

## 📄 Licença

Este repositório é disponibilizado sob a licença [MIT](https://opensource.org/licenses/MIT). Sinta-se livre para utilizar, modificar e distribuir para fins educacionais e profissionais.
