//Matheus Persch
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define ALFABETO_TAM 10


typedef struct ListaPalavras {
    char *palavra;
    struct ListaPalavras *proximo;
}ListaPalavras;


typedef struct TrieNo {
    struct TrieNo *filhos[ALFABETO_TAM];
    struct ListaPalavras *palavras; // Lista de palavras que terminam neste nó
}TrieNo;

int t9_map[] = {
    ['a'] = 2, ['b'] = 2, ['c'] = 2, 
    ['d'] = 3, ['e'] = 3, ['f'] = 3,
    ['g'] = 4, ['h'] = 4, ['i'] = 4,
    ['j'] = 5, ['k'] = 5, ['l'] = 5,
    ['m'] = 6, ['n'] = 6, ['o'] = 6,
    ['p'] = 7, ['q'] = 7, ['r'] = 7, ['s'] = 7,
    ['t'] = 8, ['u'] = 8, ['v'] = 8,
    ['w'] = 9, ['x'] = 9, ['y'] = 9, ['z'] = 9
};


TrieNo *criarNo() {
    TrieNo *no = (TrieNo *)malloc(sizeof(TrieNo));
    if (no) {
        for (int i = 0; i < ALFABETO_TAM; i++) {
            no->filhos[i] = NULL;
        }
        no->palavras = NULL;
    }
    return no;
}


void adicionarPalavra(ListaPalavras **lista, const char *palavra) {
    ListaPalavras *novoNo = (ListaPalavras *)malloc(sizeof(ListaPalavras));
    novoNo->palavra = strdup(palavra);
    novoNo->proximo = *lista;
    *lista = novoNo;
}

void inserir(TrieNo *raiz, const char *palavra_mapeada, const char *palavra_original) {
    TrieNo *atual = raiz;
    for (int i = 0; palavra_mapeada[i] != '\0'; i++) {
        char caractere = palavra_mapeada[i];
        int indice = t9_map[caractere];
        
        if (indice >= 2 && indice <= 9) {
            if (!atual->filhos[indice]) {
                atual->filhos[indice] = criarNo();
            }
            atual = atual->filhos[indice];
        } else {
            return;
        }
    }
    adicionarPalavra(&atual->palavras, palavra_original);
}
void normalizar_palavra(char *destino, const char *origem) {
    int j = 0;
    for (int i = 0; origem[i] != '\0'; i++) {
        unsigned char c1 = origem[i];
        unsigned char c2 = origem[i+1];

        
        if (c1 == 0xC3) {
            
            if (c2 >= 0xA0 && c2 <= 0xA5) { // á, à, â, ã, ä
                destino[j++] = 'a';
            } else if (c2 == 0xA7) { // ç
                destino[j++] = 'c';
            } else if (c2 >= 0xA8 && c2 <= 0xAB) { // é, è, ê, ë
                destino[j++] = 'e';
            } else if (c2 >= 0xAC && c2 <= 0xAF) { // í, ì, î, ï
                destino[j++] = 'i';
            } else if (c2 >= 0xB2 && c2 <= 0xB6) { // ó, ò, ô, õ, ö
                destino[j++] = 'o';
            } else if (c2 >= 0xB9 && c2 <= 0xBC) { // ú, ù, û, ü
                destino[j++] = 'u';
            }
            i++; // Pula o segundo byte do caractere UTF-8
        } else if (isalpha(c1)) {
            
            destino[j++] = tolower(c1);
        }
        
    }
    destino[j] = '\0'; 
}


// A função buscar permanece a mesma, pois ela opera sobre a sequência de dígitos
ListaPalavras *buscar(TrieNo *raiz, const char *sequencia) {
    TrieNo *atual = raiz;
    for (int i = 0; sequencia[i] != '\0'; i++) {
        int indice = sequencia[i] - '0';
        if (indice < 0 || indice >= ALFABETO_TAM || !atual->filhos[indice]) {
            return NULL;
        }
        atual = atual->filhos[indice];
    }
    return atual->palavras;
}


void liberarTrie(TrieNo *no) {
    if (no) {
        for (int i = 0; i < ALFABETO_TAM; i++) {
            liberarTrie(no->filhos[i]);
        }
        ListaPalavras *atual = no->palavras;
        while (atual) {
            ListaPalavras *temp = atual;
            atual = atual->proximo;
            free(temp->palavra);
            free(temp);
        }
        free(no);
    }
}

int main() {
    TrieNo *raiz = criarNo();
    FILE *arquivo = fopen("palavras.txt", "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo 'palavras.txt'");
        return 1;
    }

    char palavra_original[100];
    char palavra_normalizada[100];

    printf("Carregando dicionário...\n");
    while (fscanf(arquivo, "%99s", palavra_original) != EOF) {
        // Cria a versão "limpa" da palavra para o mapeamento T9
        normalizar_palavra(palavra_normalizada, palavra_original);
        
        // Insere na trie se a palavra normalizada não for vazia
        if (strlen(palavra_normalizada) > 0) {
            inserir(raiz, palavra_normalizada, palavra_original);
        }
    }
    fclose(arquivo);
    printf("Dicionário carregado com sucesso!\n");

    char sequencia[100];
    while (1) {
        printf("\nDigite a sequência de números ('0' ou 'sair' para terminar): ");
        scanf("%99s", sequencia);

        if (strcmp(sequencia, "sair") == 0 || strcmp(sequencia, "0") == 0) {
            break;
        }

        ListaPalavras *resultado = buscar(raiz, sequencia);
        if (resultado) {
            printf("Palavras correspondentes para '%s':\n", sequencia);
            ListaPalavras *atual = resultado;
            while (atual) {
                printf("- %s\n", atual->palavra);
                atual = atual->proximo;
            }
        } else {
            printf("Nenhuma palavra encontrada para a sequência '%s'.\n", sequencia);
        }
    }

    liberarTrie(raiz);
    return 0;
}