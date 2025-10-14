#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- DEFINIÇÕES GLOBAIS ---
#define ALPHABET_SIZE 256         // Tamanho para cobrir ASCII estendido/UTF-8 byte
#define MAX_WORD_LEN 100          // Comprimento máximo da palavra
#define LEXICON_FILENAME "lexico_v3.0.txt" // Nome do arquivo de entrada
#define OUTPUT_FILENAME "OpLexicon_v3.0_updated.txt" // Nome do arquivo de saída
#define SENTINEL_VALUE -999.0     // Valor para indicar que uma palavra não existe

// --- ESTRUTURA DA TRIE ---
typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    float polarity_value;       // Valor numérico da polaridade
    int isEndOfWord;            // Flag para indicar que uma palavra termina aqui
} TrieNode;

// --- FUNÇÕES DA TRIE ---

/**
 * Cria e inicializa um novo nó da Trie.
 * @return Ponteiro para o novo nó.
 */
TrieNode* createNode() {
    TrieNode *pNode = (TrieNode *)malloc(sizeof(TrieNode));

    if (pNode) {
        pNode->isEndOfWord = 0;
        pNode->polarity_value = SENTINEL_VALUE;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            pNode->children[i] = NULL;
        }
    }
    return pNode;
}

void insertWord(TrieNode *root, const char *word, float polarity_val) {
    TrieNode *pCrawl = root;
    int length = strlen(word);

    for (int i = 0; i < length; i++) {
        // Ignora o caractere de nova linha (se o parser não o remover)
        if (word[i] == '\n') continue;
        
        
        unsigned char index = (unsigned char)word[i];

        if (!pCrawl->children[index]) {
            pCrawl->children[index] = createNode();
        }

        pCrawl = pCrawl->children[index];
    }
    
    if (length > 0) {
        // Marca o nó final e armazena a polaridade
        pCrawl->isEndOfWord = 1;
        pCrawl->polarity_value = polarity_val;
    }
}

float searchPolarity(TrieNode *root, const char *word) {
    TrieNode *pCrawl = root;
    int length = strlen(word);

    for (int i = 0; i < length; i++) {
        unsigned char index = (unsigned char)word[i];

        if (!pCrawl->children[index]) {
            return SENTINEL_VALUE; 
        }

        pCrawl = pCrawl->children[index];
    }

    // Retorna a polaridade se for o final de uma palavra
    return (pCrawl != NULL && pCrawl->isEndOfWord) ? pCrawl->polarity_value : SENTINEL_VALUE;
}


int updatePolarity(TrieNode *root, const char *word, float new_polarity_val) {
    TrieNode *pCrawl = root;
    int length = strlen(word);

    for (int i = 0; i < length; i++) {
        unsigned char index = (unsigned char)word[i];
        if (!pCrawl->children[index]) {
            return 0; // Palavra não existe
        }
        pCrawl = pCrawl->children[index];
    }

    if (pCrawl != NULL && pCrawl->isEndOfWord) {
        pCrawl->polarity_value = new_polarity_val;
        return 1; // Polaridade atualizada com sucesso
    }

    return 0; // Palavra não existe ou não é uma palavra completa
}

void destroyTrie(TrieNode *root) {
    if (!root) return;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            destroyTrie(root->children[i]);
        }
    }
    free(root);
}

void loadLexicon(TrieNode *root, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return;
    }

    char line[512]; // Aumenta o buffer para linhas potencialmente mais longas
    int count = 0;
    
    // A função strdup é da string.h e pode não estar disponível em todos os compiladores C antigos.
    // Usaremos uma solução mais robusta para alocação.
    
    while (fgets(line, sizeof(line), fp)) {
        
        // Aloca espaço para uma cópia mutável da linha
        char *temp_line = (char *)malloc(strlen(line) + 1);
        if (!temp_line) {
            perror("ERRO de alocação de memória (strdup)");
            break;
        }
        strcpy(temp_line, line);

        char *word_str = NULL;
        char *polarity_str = NULL;
        float polarity_val = 0.0;
        int col_index = 0;

        // O delimitador é a VÍRGULA (',') para este formato
        char *token = strtok(temp_line, ","); 
        
        while(token != NULL) {
            if (col_index == 0) {
                // Primeira coluna: Palavra
                word_str = token;
            } else if (col_index == 2) {
                // Terceira coluna: Polaridade
                polarity_str = token;
                break; // Otimização: para o loop após encontrar a polaridade
            }
            
            token = strtok(NULL, ",");
            col_index++;
        }
        
        // Verifica se a palavra e a polaridade foram encontradas
        if (word_str && polarity_str) {
            
            // Remove espaços em branco do início/fim (trimming básico)
            while(isspace((unsigned char)*word_str)) word_str++;
            
            // Remove o caractere de nova linha ou espaços do final da polaridade
            int len = strlen(polarity_str);
            while(len > 0 && isspace((unsigned char)polarity_str[len-1])) {
                polarity_str[--len] = '\0';
            }

            // A polaridade pode ser um float ou um inteiro, sscanf resolve isso.
            if (sscanf(polarity_str, "%f", &polarity_val) == 1) {
                insertWord(root, word_str, polarity_val);
                count++;
            }
        }
        
        free(temp_line);
    }

    fclose(fp);
    printf("Lexico carregado. %d palavras/entradas inseridas na Trie.\n", count);
}

/**
 * Função recursiva (DFS) para salvar a Trie no formato 'palavra, polaridade\n'.
 * Nota: Salvaremos apenas a palavra e a polaridade, no novo formato simples.
 * Complexidade O(S), onde S é a soma dos comprimentos de todas as palavras.
 * @param root Nó atual.
 * @param currentWord Palavra sendo construída (prefixo).
 * @param level Nível de profundidade (para indexar currentWord).
 * @param fp Ponteiro do arquivo de saída.
 */
void saveTrieToFileDFS(TrieNode *root, char *currentWord, int level, FILE *fp) {
    if (root->isEndOfWord) {
        currentWord[level] = '\0'; // Finaliza a palavra
        // Salva no formato: palavra,polaridade
        fprintf(fp, "%s,%.2f\n", currentWord, root->polarity_value);
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            currentWord[level] = (char)i; // Adiciona o caractere ao prefixo
            saveTrieToFileDFS(root->children[i], currentWord, level + 1, fp);
        }
    }
}

// --- FUNÇÃO PRINCIPAL ---
int main() {
    TrieNode *root = createNode();

    // 1. Carregamento do Léxico
    loadLexicon(root, LEXICON_FILENAME);

    char choice;
    char word[MAX_WORD_LEN];
    float polaridade;

    do {
        printf("\n--- Menu OpLexicon (Trie) ---\n");
        printf("1. Buscar Polaridade (O(L))\n");
        printf("2. Editar Polaridade (O(L))\n");
        printf("3. Salvar Léxico Atualizado (O(S))\n");
        printf("0. Sair\n");
        printf("Escolha a operação: ");

        // Limpar o buffer de entrada antes de ler a escolha
        int c;
        // Leitura de um único caractere para a escolha e limpeza do restante da linha
        while ((c = getchar()) != '\n' && c != EOF); 
        // Em um loop robusto, a leitura do menu deveria ser mais cuidadosa para evitar problemas de buffer.
        if (scanf(" %c", &choice) != 1) continue; 


        // Limpar o buffer novamente para entradas seguintes (como a palavra)
        while ((c = getchar()) != '\n' && c != EOF);

        switch (choice) {
            case '1':
                // Busca de polaridade
                printf("Digite a palavra para buscar: ");
                // Uso de fgets para ler a linha completa e evitar buffer issues com scanf
                if (fgets(word, MAX_WORD_LEN, stdin)) {
                    // Remover o '\n' final de fgets
                    word[strcspn(word, "\n")] = 0;
                    polaridade = searchPolarity(root, word);
                    if (polaridade != SENTINEL_VALUE) {
                        printf("Polaridade de '%s': %.2f\n", word, polaridade);
                    } else {
                        printf("Palavra '%s' não encontrada no léxico.\n", word);
                    }
                }
                break;

            case '2':
                // Edição de polaridade
                printf("Digite a palavra para editar: ");
                if (fgets(word, MAX_WORD_LEN, stdin)) {
                     // Remover o '\n' final de fgets
                    word[strcspn(word, "\n")] = 0;

                    printf("Digite a NOVA polaridade (float, ex: 1.0, -0.5): ");
                    
                    if (scanf("%f", &polaridade) != 1) {
                        printf("Erro ao ler a polaridade. Voltando ao menu.\n");
                        // Limpar o buffer de entrada do float
                        while (getchar() != '\n'); 
                        break;
                    }

                    if (updatePolarity(root, word, polaridade)) {
                        printf("Polaridade de '%s' atualizada para %.2f.\n", word, polaridade);
                    } else {
                        printf("ERRO: Palavra '%s' não encontrada para edição.\n", word);
                    }
                }
                 // Limpar o buffer de entrada após a leitura do float
                while ((c = getchar()) != '\n' && c != EOF); 
                break;

            case '3':
                // Salvamento de arquivo (Serialização)
                printf("Salvando léxico atualizado em '%s'...\n", OUTPUT_FILENAME);
                FILE *fp_out = fopen(OUTPUT_FILENAME, "w");
                if (fp_out) {
                    char currentWord[MAX_WORD_LEN] = {0};
                    saveTrieToFileDFS(root, currentWord, 0, fp_out);
                    fclose(fp_out);
                    printf("Salvamento concluído. Verifique o arquivo '%s'.\n", OUTPUT_FILENAME);
                } else {
                    perror("ERRO ao abrir o arquivo de saída");
                }
                break;

            case '0':
                printf("Saindo do programa.\n");
                break;

            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    } while (choice != '0');

    // Liberação de memória
    destroyTrie(root);

    return 0;
}