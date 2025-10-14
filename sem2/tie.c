#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define ALPHABET 26 // TAMANHO ALFABETO
typedef struct TrieNode
{
    struct Trienode* children[ALPHABET];
    bool isEndOfWord;

} TrieNode;

TrieNode* createNode(){
    TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode*));
    if(newNode){
        newNode->isEndOfWord = false;
        for (int i = 0; i < ALPHABET; i++) {
            newNode->children[i] = NULL;

        }
    }
    return newNode;
}

void insertNode(TrieNode* root, char* key){
    TrieNode* current = root;
    for (int i = 0;key[i] != '\0'; i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = createNode(); // Cria o nó se ele não existir
        }
        current = current->children[index]; // Move para o nó filho correspondente
    }
    current->isEndOfWord = true;
}

int search(TrieNode* root, char* key) {
    TrieNode* current = root;
    for (int i = 0; key[i] != '\0'; i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            return 0; // Caractere não encontrado, palavra não existe
        }
        current = current->children[index];
    }
    return current != NULL && current->isEndOfWord; // Retorna true se chegou ao fim da palavra
}


int main(){
    
}
