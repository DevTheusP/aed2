#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define TAM_NOME_ALUNO 100
#define TAM_NOME_DISCIPLINA 100

// ESTRUTURAS DE DADOS

typedef struct Aluno{

    int matricula;
    char nome_aluno[TAM_NOME_ALUNO];
    int ativo;

} Aluno;

typedef struct Disciplina{

    int codigo_disciplina;
    char nome_disciplina[TAM_NOME_DISCIPLINA];
    int ativo;

} Disciplina;

typedef struct Matricula{

    int id_matricula;
    int matricula_aluno;
    int codigo_disciplina;
    float media_final;
    int ativo;

} Matricula;

// ESTRUTURAS DA ARVORE B

// ORDEM DA ARVORE B -> T = 3 SIGNIGICA QUE CADA NÓ RTEM NO MÁXIMO 5 CHAVES (2*T - 1)

#define T 3
#define MAX_CHAVES (2*T -1)
#define MAX_FILHOS (2 * T)

// NÓ DA ÁRVORE QUE VAI SER GUARDADO NO DISCO
typedef struct BtreeNode {
    int num_chaves;
    int chaves[MAX_CHAVES];
    
    //OFFSETS_DADOS APONTA PARA O REGISTRO NO ARQUIVO .DAT
    // É UM ARRAY DE "PONTEIRO PARA DADOS";
    // CADA POSIÇÃO ARMAZENA A POSIÇÃO EXATA EM BYTES DO REGISTRO DE DADOS COMPLETO NO ARQUIVO .DAT
    // SE CHAVES[0] = 1001, OFFSETS_DADOS[0] = 512 SIGNIFICA QUE O REGISTRO DO ALUNO 1001 COMEÇA NO BYTE 512 DO ARQUIVO .DAT
    long offsets_dados[MAX_CHAVES];

    //OFFSETS_FILHOS APONTA PARA OUTROS NÓS NO ARQUIVO .IDX
    // COMO SE FOSSE UM "ARRAY DE PONTEIROS PARA NÓS FILHOS";
    // CADA POSIÇÃO ARMAZENA A POSIÇÃO EXATA EM BYTES DE OUTRO BTREENODE DENTRO DO MESMO ARQIUVO DE INDICE .IDX
    // SÓ É USADO SE O NÓ NÃO FOR FOLHA POR MOTIVOS OBVIOS
    // UM NÓ COM K CHAVES TEM K+1 FILHOS
    // OFFSETS_FILHOS[0] APONTA PARA O NO FILHO QUE CONTEM TODAS AS CHAVES MENORES QUE CHAVES[0]
    //OFFSETS_FILHOS[1] APONTA PARA O NÓ FILHO QUE CONTEM TODAS AS CHAVES ENTRE CHAVES[0] E CHAVES[1]
    // offsets_filhos[num_chaves] aponta para o nó filho que contém todas as chaves maiores que chaves[num_chaves - 1].
    long offsets_filhos[MAX_FILHOS];

    int eh_folha;

    // meu_offset aponta para si mesmo, vai ser util depois;
    long meu_offset;
} BtreeNode;


// Cabeçalho do arquivo de índice (.idx)
// Ficará sempre nos primeiros bytes (posição 0) do arquivo .idx
typedef struct BTreeHeader{
    long offset_raiz; // Offset (posição) do nó raiz no arquivo .idx
    int proximo_id; // (Usado para gerar PK sequencial de Matricula)
} BTreeHeader;

// --- Ponteiros de Arquivo Globais ---
FILE *alunos_dat;
FILE *alunos_idx;
FILE *disciplinas_dat;
FILE *disciplinas_idx;
FILE *matriculas_dat;
FILE *matriculas_idx;

// GERENCIADOR DE ARQUIVOS .DAT

// Função para ler um registro dado um offset
// Retorna 1 em sucesso, 0 em falha (ex: offset inválido)
int ler_registro(void *buffer, size_t tamanho_struct, long offset, FILE *arquivo_dat) {
    if (fseek(arquivo_dat, offset, SEEK_SET) != 0) {
        perror("Erro ao buscar offset para leitura");
        return 0;
    }
    
    if (fread(buffer, tamanho_struct, 1, arquivo_dat) != 1) {
        // Pode não ser um erro 
        return 0;
    }
    return 1;
}

// Função para escrever um registro em um arquivo .dat dado um offset
// vou usar na hora de atualizar um registro
// Retorna 1 em sucesso, 0 em falha.
int escrever_registro(void *registro, size_t tamanho_struct, long offset, FILE *arquivo_dat) {
    if (fseek(arquivo_dat, offset, SEEK_SET) != 0) {
        perror("Erro ao buscar offset para escrita");
        return 0;
    }
    
    if (fwrite(registro, tamanho_struct, 1, arquivo_dat) != 1) {
        perror("Erro ao escrever registro no arquivo .dat");
        return 0;
    }
    return 1;
}