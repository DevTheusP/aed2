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

//Definição das funções da arvore b
void inserir_na_arvore_b(FILE *arquivo_idx, int chave, long offset_dado);
void dividir_filho(FILE *arquivo_idx, BtreeNode *no_pai, int indice_filho, BtreeNode *filho_cheio);
void inserir_nao_cheio(FILE *arquivo_idx, BtreeNode *no, int chave, long offset_dado);

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
// Função para apendar um registro no final de um arquivo .dat
// Vou usar na hora de criar registros
// Retorna o offset (posição) onde o registro foi escrito, ou -1 em caso de erro.
long apendar_registro(void *registro, size_t tamanho_struct, FILE *arquivo_dat) {
    // Move o "ponteiro" para o final do arquivo
    if (fseek(arquivo_dat, 0, SEEK_END) != 0) {
        perror("Erro ao buscar final do arquivo para apendar");
        return -1;
    }
    
    // Pega a posição atual, que é onde o novo registro vai começa
    long offset = ftell(arquivo_dat);
    if (offset == -1) {
        perror("Erro ao obter offset atual");
        return -1;
    }

    // Dale no novo registro
    if (fwrite(registro, tamanho_struct, 1, arquivo_dat) != 1) {
        perror("Erro ao apendar registro no arquivo .dat");
        return -1;
    }
    
    // Retorna a posição onde o registro foi escrito
    return offset;
}

int ler_cabecalho_idx(BTreeHeader *header_buffer, FILE *arquivo_idx){
    if(fseek(arquivo_idx,0,SEEK_SET) != 0){
        perror("Erro ao buscar início do arquivo de índice (heaer)");
        return 0;
    }
    if(fread(header_buffer,sizeof(BTreeHeader),1,arquivo_idx)!= 1){
        // cabeçaho não existe ainda
        return 0;
    }
    return 1;
}
// Escreve o cabeçalho (sempre no offset 0) em um arquivo de índice
int escrever_cabecalho_idx(BTreeHeader *header, FILE *arquivo_idx) {
    if (fseek(arquivo_idx, 0, SEEK_SET) != 0) {
        perror("Erro ao buscar início do arquivo de índice (header)");
        return 0;
    }
    if (fwrite(header, sizeof(BTreeHeader), 1, arquivo_idx) != 1) {
        perror("Erro ao escrever cabeçalho no arquivo de índice");
        return 0;
    }
    return 1;
}
// Lê um BtreeNode de um arquivo de índice a partir de um offset específico
int ler_no_idx(BtreeNode *no_buffer, long offset, FILE *arquivo_idx) {
    if (offset <= 0) {
        fprintf(stderr, "Erro: Tentativa de ler nó com offset inválido (%ld)\n", offset);
        return 0;
    }
    if (fseek(arquivo_idx, offset, SEEK_SET) != 0) {
        perror("Erro ao buscar offset do nó para leitura");
        return 0;
    }
    if (fread(no_buffer, sizeof(BtreeNode), 1, arquivo_idx) != 1) {
        perror("Erro ao ler nó do arquivo de índice");
        return 0;
    }
    return 1;
}
// Escreve um BtreeNode em um arquivo de índice.
// O offset de escrita é pego de 'no_da_arvore->meu_offset'.
int escrever_no_idx(BtreeNode *no, FILE *arquivo_idx) {
    if (no->meu_offset <= 0) {
        fprintf(stderr, "Erro: Tentativa de escrever nó com offset inválido (%ld)\n", no->meu_offset);
        return 0;
    }
    if (fseek(arquivo_idx, no->meu_offset, SEEK_SET) != 0) {
        perror("Erro ao buscar offset do nó para escrita");
        return 0;
    }
    if (fwrite(no, sizeof(BtreeNode), 1, arquivo_idx) != 1) {
        perror("Erro ao escrever nó no arquivo de índice");
        return 0;
    }
    return 1;
}
// Descobre o próximo offset livre no arquivo de índice pra dale um novo nó
// O offset 0 é o Header. Os nós começam depois dele
long obter_proximo_offset_livre(FILE *arquivo_idx) {
    if (fseek(arquivo_idx, 0, SEEK_END) != 0) {
        perror("Erro ao buscar final do arquivo de índice");
        return -1;
    }
    
    long offset = ftell(arquivo_idx);
    if (offset == -1) {
        perror("Erro ao obter offset final");
        return -1;
    }

    // Se o arquivo estiver totalmente vazio (offset 0), o primeiro nó
    // deve começar DEPOIS do cabeçalho se não vai dar merda
    if (offset < sizeof(BTreeHeader)) {
        offset = sizeof(BTreeHeader);
    }
    
    return offset;
}
// função pra dale a partida no programa.
void inicializar_arquivos() {
    
    alunos_dat = fopen("alunos.dat", "r+b");
    alunos_idx = fopen("alunos.idx", "r+b");

    if (alunos_dat == NULL || alunos_idx == NULL) {
        printf("Arquivos de Alunos não encontrados. Criando novos...\n");
        if (alunos_dat) fclose(alunos_dat);
        if (alunos_idx) fclose(alunos_idx);

        alunos_dat = fopen("alunos.dat", "w+b");
        alunos_idx = fopen("alunos.idx", "w+b");
        
        if (alunos_dat == NULL || alunos_idx == NULL) {
            perror("Erro crítico ao criar arquivos de alunos");
            exit(1);
        }
        
        // cria o cabecalho e a raiz dos aluno
        BTreeHeader header_aluno;
        BtreeNode raiz_aluno;
        
        // aloca a primeira raiz
        long offset_raiz_aluno = obter_proximo_offset_livre(alunos_idx);
        raiz_aluno.meu_offset = offset_raiz_aluno;
        raiz_aluno.eh_folha = 1;
        raiz_aluno.num_chaves = 0;
        
        // colocar o cabeçalho pra apontar pra raiz pra não dar merda
        header_aluno.offset_raiz = offset_raiz_aluno;
        header_aluno.proximo_id = 0; 
        
        // 4. Escrever no disco
        escrever_cabecalho_idx(&header_aluno, alunos_idx);
        escrever_no_idx(&raiz_aluno, alunos_idx);
    }

    disciplinas_dat = fopen("disciplinas.dat", "r+b");
    disciplinas_idx = fopen("disciplinas.idx", "r+b");

    if (disciplinas_dat == NULL || disciplinas_idx == NULL) {
        printf("Arquivos de Disciplinas não encontrados. Criando novos...\n");
        if (disciplinas_dat) fclose(disciplinas_dat);
        if (disciplinas_idx) fclose(disciplinas_idx);

        disciplinas_dat = fopen("disciplinas.dat", "w+b");
        disciplinas_idx = fopen("disciplinas.idx", "w+b");
        
        if (disciplinas_dat == NULL || disciplinas_idx == NULL) {
            perror("Erro crítico ao criar arquivos de disciplinas");
            exit(1);
        }

        BTreeHeader header_disc;
        BtreeNode raiz_disc;
        long offset_raiz_disc = obter_proximo_offset_livre(disciplinas_idx);
        raiz_disc.meu_offset = offset_raiz_disc;
        raiz_disc.eh_folha = 1;
        raiz_disc.num_chaves = 0;
        
        header_disc.offset_raiz = offset_raiz_disc;
        header_disc.proximo_id = 0; 
        
        escrever_cabecalho_idx(&header_disc, disciplinas_idx);
        escrever_no_idx(&raiz_disc, disciplinas_idx);
    }

    
    matriculas_dat = fopen("matriculas.dat", "r+b");
    matriculas_idx = fopen("matriculas.idx", "r+b");

    if (matriculas_dat == NULL || matriculas_idx == NULL) {
        printf("Arquivos de Matrículas não encontrados. Criando novos...\n");
        if (matriculas_dat) fclose(matriculas_dat);
        if (matriculas_idx) fclose(matriculas_idx);

        matriculas_dat = fopen("matriculas.dat", "w+b");
        matriculas_idx = fopen("matriculas.idx", "w+b");
        
        if (matriculas_dat == NULL || matriculas_idx == NULL) {
            perror("Erro crítico ao criar arquivos de matrículas");
            exit(1);
        }
        
        BTreeHeader header_mat;
        BtreeNode raiz_mat;
        long offset_raiz_mat = obter_proximo_offset_livre(matriculas_idx);
        
        raiz_mat.meu_offset = offset_raiz_mat;
        raiz_mat.eh_folha = 1;
        raiz_mat.num_chaves = 0;
        
        header_mat.offset_raiz = offset_raiz_mat;
        // Usado para gerar a Primary key sequencial (id_matricula), não usa nos alunos e disciplinas
        header_mat.proximo_id = 1; 
        
        escrever_cabecalho_idx(&header_mat, matriculas_idx);
        escrever_no_idx(&raiz_mat, matriculas_idx);
    }
    
    printf("Arquivos abertos e inicializados com sucesso.\n");
}

void fechar_arquivos() {
    if (alunos_dat) fclose(alunos_dat);
    if (alunos_idx) fclose(alunos_idx);
    if (disciplinas_dat) fclose(disciplinas_dat);
    if (disciplinas_idx) fclose(disciplinas_idx);
    if (matriculas_dat) fclose(matriculas_dat);
    if (matriculas_idx) fclose(matriculas_idx);
    printf("Arquivos fechados.\n");
}

// busca binaria por recursividade #paia
// Ela navega pelos nós no arquivo .idx.
// Retorna o offset do DADO no arquivo .dat se encontrar.
// Retorna -1 se não encontrar.
long buscar_no_recursivo(FILE *arquivo_idx, long offset_no_atual, int chave) {
    
    // 1. Carregar o nó atual da memória
    BtreeNode no_atual;
    if (offset_no_atual <= 0) {
        return -1; // Offset inválido
    }
    if (!ler_no_idx(&no_atual, offset_no_atual, arquivo_idx)) {
        fprintf(stderr, "Erro: Falha ao ler nó durante a busca (offset: %ld)\n", offset_no_atual);
        return -1;
    }

    // acharchave dentro do nó atual busca binaria
    // achar 'i' tal que:
    // - se chave == chaves[i], achamo dale.
    // - se chave < chaves[i], desce por offsets_filhos[i].
    // - se chave > chaves[i], continua a busca.
    // No final, se não encontrar, 'l' será o índice do filho para descer.
    
    int l = 0; // left
    int r = no_atual.num_chaves - 1; // right
    int mid;

    while (l <= r) {
        mid = l + (r - l) / 2; // Evita overflow de (l+r)
        
        if (no_atual.chaves[mid] == chave) {
            //achou dale, retorna o offset do dado no arquivo.dat
            return no_atual.offsets_dados[mid];
        }
        
        if (no_atual.chaves[mid] < chave) {
            l = mid + 1; // procura na direita
        } else {
            r = mid - 1; // procura na esquerda
        }
    }
    
    // se passar por aqui nao achou a chave nesse nó
    // O 'while' terminou (l > r).
    // O índice 'l' é a posição correta do filho
    // por onde devem descer (o primeiro elemento > chave).
    
    // verificar se esta em uma folha
    if (no_atual.eh_folha) {
        //se chegou aqui não achou e é folha its over 
        return -1;
    }

    // se chegou aqui não é folha e o lugar certo pra descer é pelo l
    long offset_filho = no_atual.offsets_filhos[l];
    
    // tomale recursao no filho
    return buscar_no_recursivo(arquivo_idx, offset_filho, chave);
}

// funcao da busca total, le o cabecalho e depois dale a busca
long buscar_na_arvore_b(FILE *arquivo_idx, int chave) {
    // le o cabecalho no arquivo de indice
    BTreeHeader header;
    if (!ler_cabecalho_idx(&header, arquivo_idx)) {
        fprintf(stderr, "Erro: Não foi possível ler o cabeçalho do arquivo de índice.\n");
        return -1; // não encontrou
    }

    //dale busca recursiva
    return buscar_no_recursivo(arquivo_idx, header.offset_raiz, chave);
}
//função para ler um aluno pela matricula
//retorna um ponteiro para um Aluno alocado dinamicamente
//retorna NULL se não encontrar nada ou se o aluno estiver inativo
Aluno* ler_aluno(int matricula){
    // Usar a arvore B para encontrar o offset do dado no .dat
    long offset_dado = buscar_na_arvore_b(alunos_idx, matricula);
    if (offset_dado == -1) {
            //não encontrado no índice
        printf("Aluno com matrícula %d não encontrado no índice.\n", matricula);
        return NULL;
    }
    // aloca memória para o resultado
    Aluno *aluno_buffer = (Aluno*)malloc(sizeof(Aluno));
    if(aluno_buffer == NULL) {
        perror("Erro: Falha ao alocar memória para ler aluno");
        return NULL;
    }

    if(!ler_registro(aluno_buffer, sizeof(Aluno), offset_dado, alunos_dat)) {
        fprintf(stderr, "Erro: Índice encontrou o offset, mas a leitura do .dat falhou!");
        free(aluno_buffer);
        return NULL;
    }
    if (aluno_buffer->ativo == 0) {
        printf("Aluno com matrícula %d foi encontrado, mas está marcado como removido.\n", matricula);
        free(aluno_buffer);
        return NULL;
    }

    return aluno_buffer;
}  
void criar_aluno(int matricula, char *nome) {
    // primeiro checar se a matrícula já existe
    if(buscar_na_arvore_b(alunos_idx, matricula) != -1) {
        printf("Erro: Aluno com matrícula %d já existe.\n", matricula);
        return;
    }
    // criar o registro do aluno na memória
    Aluno novo_aluno;
    novo_aluno.matricula = matricula;
    strncpy(novo_aluno.nome_aluno, nome, TAM_NOME_ALUNO);
    novo_aluno.nome_aluno[TAM_NOME_ALUNO - 1] = '\0';
    novo_aluno.ativo = 1;

    // apendar o registro no arquivo .dat
    long offset_dado = apendar_registro(&novo_aluno, sizeof(Aluno), alunos_dat);

    if(offset_dado == -1) {
        fprintf(stderr, "Erro: falha ao salvar novo aluno no .dat\n");
        return;
    }
    // inserir a chave e o ofsset nbo .idx
    inserir_na_arvore_b(alunos_idx, matricula, offset_dado);

    printf("Aluno %d - %s criado com sucesso.\n", matricula, nome);
}

Disciplina* ler_disciplina(int codigo) {
    long offset_dado = buscar_na_arvore_b(disciplinas_idx, codigo);
    if(offset_dado == -1){
        printf("Disciplina com código %d não encontrada no índice.\n", codigo);
        return NULL;
    }
    Disciplina *disciplina_buffer = (Disciplina*)malloc(sizeof(Disciplina));
    if(disciplina_buffer == NULL) {
        perror("Erro: Falha ao alocar memória para ler disciplina");
        return NULL;
    }
    if(!ler_registro(disciplina_buffer, sizeof(Disciplina), offset_dado, disciplinas_dat)) {
        fprintf(stderr, "Erro: índice encontrou o offset, mas a leitura do .dat falhou (offset: %ld)\n", offset_dado);
        free(disciplina_buffer);
        return NULL;
    }
    if (disciplina_buffer->ativo == 0) {
        printf("Disciplina com código %d foi encontrada, mas está marcada como removida.\n", codigo);
        free(disciplina_buffer);
        return NULL;
    }
    return disciplina_buffer;
}

void criar_disciplina(int codigo, char *nome) {
    if(buscar_na_arvore_b(disciplinas_idx, codigo) != -1){
        printf("Erro: Disciplina com código %d já existe.\n", codigo);
        return;
    }
    Disciplina nova_disciplina;
    nova_disciplina.codigo_disciplina = codigo;
    strncpy(nova_disciplina.nome_disciplina, nome, TAM_NOME_DISCIPLINA);
    nova_disciplina.nome_disciplina[TAM_NOME_DISCIPLINA - 1] = '\0';
    nova_disciplina.ativo = 1;

    long offset_dado = apendar_registro(&nova_disciplina, sizeof(Disciplina), disciplinas_dat);
    if(offset_dado == -1){
        fprintf(stderr, "Erro: Falha ao salvar nova disciplina no .dat\n");
        return;
    }

    inserir_na_arvore_b(disciplinas_idx, codigo, offset_dado);

    printf("Disciplina %d - %s criada com sucesso.\n", codigo, nome);

}

void inserir_na_arvore_b(FILE *arquivo_idx, int chave, long offset_dado) {
    // Ler o cabeçalho para achar a raiz
    BTreeHeader header;
    ler_cabecalho_idx(&header, arquivo_idx);
    //ler o nó raiz
    BtreeNode raiz;
    if(!ler_no_idx(&raiz, header.offset_raiz, arquivo_idx)) {
        fprintf(stderr, "Erro fatal: Não foi possível ler o nó raiz.\n");
        return;
    }

    // verifica se a raiz ta cheia T= 3 -> MAX CHAVES = 5)
    if(raiz.num_chaves == MAX_CHAVES){
        // raiz ta cheia
        //precisa criar uma raiz nova e dividir a antiga

        long offset_nova_raiz = obter_proximo_offset_livre(arquivo_idx);
        BtreeNode nova_raiz;
        nova_raiz.meu_offset = offset_nova_raiz;
        nova_raiz.eh_folha = 0; // nova raiz nunca é uma folha
        nova_raiz.num_chaves = 0; // starta vazia

        // a raiz antiga vira o primeiro e unico filho da nova raiz
        nova_raiz.offsets_filhos[0] = header.offset_raiz;

        //dividir a raiz antiga
        //funcao dividir filho vai 'promover' a chave do meio da raiz antiga para nova raiz
        dividir_filho(arquivo_idx, &nova_raiz, 0 , &raiz);

        //finalmente ta pronto
        // dale inserir
        // a chave nao vai pra nova mas pra um dos filhos dela
        inserir_nao_cheio(arquivo_idx, &nova_raiz, chave, offset_dado);

        // atualiza o cabecalho no disco pra aponta pra nova raiz
        header.offset_raiz = offset_nova_raiz;
        escrever_cabecalho_idx(&nova_raiz, arquivo_idx);
        //atualiza a nova raiz no disco
        escrever_no_idx(&nova_raiz, arquivo_idx);


    }else{
        // a raiz nao ta cheia
        // so dale
        inserir_nao_cheio(arquivo_idx, &raiz, chave, offset_dado);
        
    }
}

Matricula* ler_matricula(int id_matricula) {
    long offset_matricula = buscar_na_arvore_b(matriculas_idx, id_matricula);
    if(offset_matricula == -1){
        printf("Matricula com código %d não encontrada no índice.\n", id_matricula);
        return NULL;
    }
    Matricula *matricula_buffer = (Matricula*)malloc(sizeof(Matricula));
    if(matricula_buffer == NULL) {
        perror("Erro: Falha ao alocar memória para ler matricula");
        return NULL;
    }
    if(!ler_registro(matricula_buffer, sizeof(Matricula), offset_matricula, matriculas_dat)) {
        fprintf(stderr, "Erro: índice encontrou o offset, mas a leitura do .dat falhou (offset: %ld)\n", offset_matricula);
        free(matricula_buffer);
        return NULL;
    }
    // 4. Checar se o registro está marcado como 'ativo'
    if (matricula_buffer->ativo == 0) {
        printf("Matrícula com ID %d foi encontrada, mas está marcada como removida.\n", id_matricula);
        free(matricula_buffer);
        return NULL;
    }
    return matricula_buffer;

}
void criar_matricula(int matricula_aluno, int codigo_disciplina, float media_final) {
    // verifica se o aluno existe
    if (buscar_na_arvore_b(alunos_idx, matricula_aluno) == -1) {
        printf("Erro: Não foi possível criar matrícula. Aluno %d não encontrado.\n", matricula_aluno);
        return;
    }

    // verifica se a disciplina existe
    if (buscar_na_arvore_b(disciplinas_idx, codigo_disciplina) == -1) {
        printf("Erro: Não foi possível criar matrícula. Disciplina %d não encontrada.\n", codigo_disciplina);
        return;
    }
    // gera o proximo id da matricula sequencial
    BTreeHeader header_mat;
    if (!ler_cabecalho_idx(&header_mat, matriculas_idx)) {
        fprintf(stderr, "Erro: Falha ao ler cabeçalho de matrículas.\n");
        return;
    }

    int novo_id_matricula = header_mat.proximo_id;

    // cria o registro de matricula em memoria
    Matricula nova_matricula;
    nova_matricula.id_matricula = novo_id_matricula;
    nova_matricula.matricula_aluno = matricula_aluno;
    nova_matricula.codigo_disciplina = codigo_disciplina;
    nova_matricula.media_final = media_final;
    nova_matricula.ativo = 1;

    // coloca no .dat
    long offset_dado = apendar_registro(&nova_matricula, sizeof(Matricula), matriculas_dat);
    
    if (offset_dado == -1) {
        fprintf(stderr, "Erro: Falha ao salvar nova matrícula no .dat\n");
        return;
    }

    // insere a chave e o offset no .idx
    inserir_na_arvore_b(matriculas_idx, novo_id_matricula, offset_dado);

    // atualiza o proximo id sequencial no header
    header_mat.proximo_id++;
    escrever_cabecalho_idx(&header_mat, matriculas_idx);
    
    printf("Matrícula ID %d criada com sucesso (Aluno: %d, Disciplina: %d).\n", 
           novo_id_matricula, matricula_aluno, codigo_disciplina);
}

void dividir_filho(FILE *arquivo_idx, BtreeNode *no_pai, int indice_filho, BtreeNode *filho_cheio) {
    long offset_novo_irmao = obter_proximo_offset_livre(arquivo_idx);
    BtreeNode novo_irmao;
    novo_irmao.meu_offset = offset_novo_irmao;
    novo_irmao.eh_folha = filho_cheio->eh_folha;
    novo_irmao.num_chaves = T - 1;

    for(int j = 0; j < T - 1; j++){
        novo_irmao.chaves[j] = filho_cheio->chaves[j + T];
        novo_irmao.offsets_dados[j] = filho_cheio->offsets_dados[j + T];
    }

    if(!filho_cheio->eh_folha){
        for(int  j = 0; j < T; j++){
            novo_irmao.offsets_filhos[j] = filho_cheio->offsets_filhos[j + T];
        }
    }
    filho_cheio->num_chaves = T -1;

    for(int j = no_pai->num_chaves  -1; j > indice_filho - 1; j--){
        no_pai->chaves[j+1] = no_pai->chaves[j];
        no_pai->offsets_filhos[j+ 1] = no_pai->offsets_filhos[j];
    }
    // (Mover chaves)
    for (int j = no_pai->num_chaves - 1; j > indice_filho - 1; j--) {
        no_pai->chaves[j + 1] = no_pai->chaves[j];
        no_pai->offsets_dados[j + 1] = no_pai->offsets_dados[j];
    }
    // sobe a chave do meio do filho pro pai
    no_pai->chaves[indice_filho] = filho_cheio->chaves[T - 1]; // Chave do meio (índice 2)
    no_pai->offsets_dados[indice_filho] = filho_cheio->offsets_dados[T - 1];
    
    // aponta o no pai pro novo irmao
    no_pai->offsets_filhos[indice_filho + 1] = novo_irmao.meu_offset;
    no_pai->num_chaves++;

    // salva no disco
    escrever_no_idx(no_pai, arquivo_idx);
    escrever_no_idx(filho_cheio, arquivo_idx);
    escrever_no_idx(&novo_irmao, arquivo_idx);
}

void inserir_nao_cheio(FILE *arquivo_idx, BtreeNode *no, int chave, long offset_dado){
    int i = no->num_chaves -1;
    if (no->eh_folha){
        while(i >= 0 && chave < no->chaves[i]){
            no->chaves[i + 1] = no->chaves[i];
            no->offsets_dados[i+1] = no->offsets_dados[i];
            i--;
        }
        no->chaves[i+1] = chave;
        no->offsets_dados[i+1] = offset_dado;
        no->num_chaves++;

        escrever_no_idx(no, arquivo_idx);
    }else{
        // Se o nó não for folha
        // acha o nó pra descer
        while (i >= 0 && chave < no->chaves[i]) {
            i--;
        }
        int indice_filho = i + 1; // O filho correto é o [i+1]

        // carrega o filho no disco
        BtreeNode filho;
        if (!ler_no_idx(&filho, no->offsets_filhos[indice_filho], arquivo_idx)) {
            fprintf(stderr, "Erro: Falha ao ler nó filho durante inserção.\n");
            return;
        }

        // checa se o filho ta cheio
        if (filho.num_chaves == MAX_CHAVES) {
            // Se o filho está cheio, ja divide antes de descer
            dividir_filho(arquivo_idx, no, indice_filho, &filho);
            // Depois de splitar o pai ganha a mediana do filho pos split
            // temos que checar para qual dos filhos vai a nova chave
            if (chave > no->chaves[indice_filho]) {
                indice_filho++; // A chave vai para o novo irmão da direita
            }
        }
        
        // le o filho certo pra inserir
        if (!ler_no_idx(&filho, no->offsets_filhos[indice_filho], arquivo_idx)) {
            fprintf(stderr, "Erro: Falha ao ler nó filho após split.\n");
            return;
        }

        // agora chama recursivamente e dale
        inserir_nao_cheio(arquivo_idx, &filho, chave, offset_dado);
    }
}

// pega emprestado da esquerda quando for deletar e quebrar a regra
void redistribuir_da_esquerda(FILE *arquivo_idx, BtreeNode *no_pai, int indice_filho);

// mesma coisa so que pra direita
void redistribuir_da_direita(FILE *arquivo_idx, BtreeNode *no_pai, int indice_filho);

// funde o filho com o irmão e baixa o pai
void fundir_filho(FILE *arquivo_idx, BtreeNode *no_pai, int indice_filho);


// função para garantir que o as t-1 chaves
void preencher_filho(FILE *arquivo_idx, BtreeNode *no_pai, int indice_filho) {
    
    // Tenta redistribuir do irmão da esquerda, se ele existir e tiver mais que T-1 chaves
    if (indice_filho != 0 && no_pai->offsets_filhos[indice_filho - 1] != 0) {
        
        BtreeNode irmao_esq;
        ler_no_idx(&irmao_esq, no_pai->offsets_filhos[indice_filho - 1], arquivo_idx);

        if (irmao_esq.num_chaves >= T) { 
            redistribuir_da_esquerda(arquivo_idx, no_pai, indice_filho);
            return;
        }
    }

    // Tenta redistribuir do irmão da direita, se ele existir e tiver mais que T-1 chaves
    
    if (indice_filho != no_pai->num_chaves && no_pai->offsets_filhos[indice_filho + 1] != 0) {
        
        BtreeNode irmao_dir;
        ler_no_idx(&irmao_dir, no_pai->offsets_filhos[indice_filho + 1], arquivo_idx);

        if (irmao_dir.num_chaves >= T) { 
            redistribuir_da_direita(arquivo_idx, no_pai, indice_filho);
            return;
        }
    }

    // se chegou aqui os irmãos estão no limite
    if (indice_filho != no_pai->num_chaves) {
        // Funde o filho [i] com o irmão da direita [i+1]
        fundir_filho(arquivo_idx, no_pai, indice_filho);
    } else {
        // Funde o filho [i] (que é o último) com o irmão da esquerda [i-1]
        fundir_filho(arquivo_idx, no_pai, indice_filho - 1);
    }
}
// Retorna a menor chave no nó da folha da subárvore do 'no' (o sucessor).
int obter_sucessor(FILE *arquivo_idx, long offset_no) {
    BtreeNode no;
    ler_no_idx(&no, offset_no, arquivo_idx);

    // Navega sempre para o filho mais à esquerda até chegar na folha
    while (!no.eh_folha) {
        offset_no = no.offsets_filhos[0];
        ler_no_idx(&no, offset_no, arquivo_idx);
    }

    // A menor chave é a primeira no nó folha
    return no.chaves[0];
}
// Retorna a maior chave no nó da folha da subárvore do 'no' (o antecessor).
int obter_antecessor(FILE *arquivo_idx, long offset_no) {
    BtreeNode no;
    ler_no_idx(&no, offset_no, arquivo_idx);

    // Navega sempre para o filho mais à direita até chegar na folha
    while (!no.eh_folha) {
        offset_no = no.offsets_filhos[no.num_chaves];
        ler_no_idx(&no, offset_no, arquivo_idx);
    }

    // A maior chave é a última no nó folha
    return no.chaves[no.num_chaves - 1];
}
void remover_recursivo(FILE *arquivo_idx, BtreeNode *no, int chave) {
    int i = 0;
    // acha por onde descer
    while (i < no->num_chaves && chave > no->chaves[i]) {
        i++;
    }

    // caso 1 e 2, a chave está aqui no indice i
    if (i < no->num_chaves && chave == no->chaves[i]) {
        
        if (no->eh_folha) {
            // Caso 1: Nó Folha - Remover e deslocar o resto para a esquerda
            // Remove a chave e o offset_dado
            for (int j = i + 1; j < no->num_chaves; j++) {
                no->chaves[j - 1] = no->chaves[j];
                no->offsets_dados[j - 1] = no->offsets_dados[j];
            }
            no->num_chaves--;
            escrever_no_idx(no, arquivo_idx); 
            return;
            
        } else {
            // Caso 2: Nó Interno - Substituir e descer
            // Tenta usar o Antecessor (filho da esquerda)
            BtreeNode filho_esq;
            ler_no_idx(&filho_esq, no->offsets_filhos[i], arquivo_idx);
            
            if (filho_esq.num_chaves >= T) {
                // Se o filho da esquerda tem chaves suficientes, pegue o antecessor (a maior chave)
                int antecessor = obter_antecessor(arquivo_idx, no->offsets_filhos[i]);
                // Substitui a chave a ser removida (chaves[i]) pelo antecessor
                no->chaves[i] = antecessor;
                
                // nao atualiza o offset de dados agora
                escrever_no_idx(no, arquivo_idx);

                //agora remove recursivo o nó que foi promovido da folha
                remover_recursivo(arquivo_idx, &filho_esq, antecessor);
                return;
                
            } else {
                // Tenta usar o Sucessor (filho da direita)
                BtreeNode filho_dir;
                ler_no_idx(&filho_dir, no->offsets_filhos[i+1], arquivo_idx);
                
                if (filho_dir.num_chaves >= T) {
                    // Se o filho da direita tem chaves suficientes, pegue o sucessor (a menor chave)
                    int sucessor = obter_sucessor(arquivo_idx, no->offsets_filhos[i+1]);
                    no->chaves[i] = sucessor;
                    escrever_no_idx(no, arquivo_idx);
                    remover_recursivo(arquivo_idx, &filho_dir, sucessor);
                    return;

                } else {
                    // Os dois filhos (antecessor e sucessor) estão no limite (T-1 chaves)
                    // Fundir o filho da esquerda [i] com o da direita [i+1]
                    fundir_filho(arquivo_idx, no, i); 
                    
                    // O nó fundido agora contém a chave k pra remover
                    // chama a função no nó esquerdo
                    remover_recursivo(arquivo_idx, &filho_esq, chave); 
                    return;
                }
            }
        }
    } 
    
    // a chave não ta nesse nó vai pra ca
    else {
        // Encontramos o filho para onde descer (o índice 'i' já aponta para o filho correto)
        long offset_filho = no->offsets_filhos[i];
        
        BtreeNode filho;
        ler_no_idx(&filho, offset_filho, arquivo_idx);
        
        // se o filho tem apenas t-1 preenche ou funde
        if (filho.num_chaves == T - 1) {
            preencher_filho(arquivo_idx, no, i);
            
            // O nó pai (no) e possivelmente os filhos mudaram após o preenchimento/fusão.
            // recarrega o nó pai, ele pode ter perdido ou ganhado chaves.
            // E recalcula o filho correto para descer, pois o pai mudou.
            
            // Recarrega o nó pai (pois o preenchimento/fusão o modificou)
            if (!ler_no_idx(no, no->meu_offset, arquivo_idx)) {
                fprintf(stderr, "Erro ao recarregar o nó pai após preenchimento.\n");
                return;
            }

            // Recalcula o índice 'i' para descer (pois o pai pode ter recebido/perdido chaves)
            i = 0;
            while (i < no->num_chaves && chave > no->chaves[i]) {
                i++;
            }
            // Recarrega o filho correto para descer
            ler_no_idx(&filho, no->offsets_filhos[i], arquivo_idx); 
        }

        // Agora, o filho tem pelo menos T chaves (ou é um nó fundido que dá pra deletar).
        remover_recursivo(arquivo_idx, &filho, chave);
    }
}

//funcao principal pra remover
void remover_da_arvore_b(FILE *arquivo_idx, int chave) {
    BTreeHeader header;
    ler_cabecalho_idx(&header, arquivo_idx);

    BtreeNode raiz;
    if (!ler_no_idx(&raiz, header.offset_raiz, arquivo_idx)) {
        fprintf(stderr, "Erro fatal ao ler raiz para deleção.\n");
        return;
    }

    remover_recursivo(arquivo_idx, &raiz, chave);

    // Verifica se a raiz ficou vazia (após fusão)
    if (raiz.num_chaves == 0) {
        // A árvore diminui de altura
        if (raiz.eh_folha) {
            // Raiz vazia e folha (árvore está vazia), mantém a raiz vazia
            header.offset_raiz = raiz.meu_offset;
        } else {
            // Raiz vazia e não folha (tem um filho), o filho se torna a nova raiz
            header.offset_raiz = raiz.offsets_filhos[0];
        }
        escrever_cabecalho_idx(&header, arquivo_idx);
    }
}

// falta fazer fundir filho e redistribuir esquerda e direita