//MATHEUS PERSCH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define HASH_P 31
#define INITIAL_TABLE_SIZE 17


typedef enum {
    NEVER_OCCUPIED, // Posição nunca usada
    OCCUPIED,       // Posição atualmente em uso
    REMOVED         // Posição continha um dado que foi removido
} State;\


typedef struct {
    char* key;   // A palavra (chave)
    char* data;  // Dados associados (polaridade, etc.)
    State state; // O estado da entrada
} HashEntry;

// Estrutura principal da Tabela Hash
typedef struct {
    HashEntry* entries; // O vetor de entradas
    int size;           // Tamanho 'm' do vetor
    int count;          // Número de elementos atualmente na tabela
} HashTable;


unsigned int hash1(const char* key, int m) {
    unsigned long long hash_value = 0;
    unsigned long long p_pow = 53;
    
    for (int i = 0; key[i] != '\0'; i++) {
        hash_value = (hash_value + (unsigned char)key[i] * p_pow) % m;
        p_pow = (p_pow * HASH_P) % m;
    }
    return hash_value;
}

unsigned int hash2(const char* key, int m) {
    unsigned long long hash_value = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash_value += (unsigned char)key[i];
    }
    // Garante que o resultado nunca seja 0
    return 1 + (hash_value % (m - 1));
}

void rehash(HashTable** ht);
void insert(HashTable** ht_ptr, const char* key, const char* data);

HashTable* create_table(int size) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        perror("Falha ao alocar memória para a tabela hash");
        exit(EXIT_FAILURE);
    }
    
    ht->size = size;
    ht->count = 0;
    ht->entries = (HashEntry*)malloc(sizeof(HashEntry) * ht->size);
    if (!ht->entries) {
        perror("Falha ao alocar memória para as entradas da tabela");
        free(ht);
        exit(EXIT_FAILURE);
    }

    
    for (int i = 0; i < ht->size; i++) {
        ht->entries[i].key = NULL;
        ht->entries[i].data = NULL;
        ht->entries[i].state = NEVER_OCCUPIED;
    }
    
    return ht;
}


void free_table(HashTable* ht) {
    if (!ht){ 
        return;
    };
    
    for (int i = 0; i < ht->size; i++) {
        free(ht->entries[i].key); 
        free(ht->entries[i].data); 
    }
    free(ht->entries);
    free(ht);
}

HashEntry* search(HashTable* ht, const char* key) {
    if (!ht) return NULL;
    
    unsigned int h1 = hash1(key, ht->size);
    unsigned int h2 = hash2(key, ht->size);

    
    for (int t = 0; t < ht->size; t++) {
        unsigned int index = (h1 + t * h2) % ht->size;

        
        if (ht->entries[index].state == NEVER_OCCUPIED) {
            return NULL;
        }
        
        
        if (ht->entries[index].state == OCCUPIED && strcmp(ht->entries[index].key, key) == 0) {
            return &ht->entries[index];
        }
        
    }

    return NULL; 
}


void insert(HashTable** ht_ptr, const char* key, const char* data) {
    if ((*ht_ptr)->count == (*ht_ptr)->size) {
        printf("Tabela cheia! Realizando re-hash...\n");
        rehash(ht_ptr);
    }
    
    HashTable* ht = *ht_ptr;
    unsigned int h1 = hash1(key, ht->size);
    unsigned int h2 = hash2(key, ht->size);

    for (int t = 0; t < ht->size; t++) {
        unsigned int index = (h1 + t * h2) % ht->size;

        
        if (ht->entries[index].state == NEVER_OCCUPIED || ht->entries[index].state == REMOVED) {
            ht->entries[index].key = strdup(key);   
            ht->entries[index].data = strdup(data);
            ht->entries[index].state = OCCUPIED;
            ht->count++;
            return;
        }
        
        else if (ht->entries[index].state == OCCUPIED && strcmp(ht->entries[index].key, key) == 0) {
            free(ht->entries[index].data); 
            ht->entries[index].data = strdup(data); 
            printf("Chave '%s' já existe. Dados atualizados.\n", key);
            return;
        }
    }
}


int remove_entry(HashTable* ht, const char* key) {
    HashEntry* entry = search(ht, key);

    if (entry != NULL) {
        
        entry->state = REMOVED;
        ht->count--;
        return 1; 
    }
    
    return 0; 
}


void rehash(HashTable** ht_ptr) {
    HashTable* old_ht = *ht_ptr;
    int new_size = old_ht->size * 2;
    
    
    HashTable* new_ht = create_table(new_size);

    printf("Redimensionando a tabela de %d para %d posições.\n", old_ht->size, new_size);

    
    for (int i = 0; i < old_ht->size; i++) {
        if (old_ht->entries[i].state == OCCUPIED) {
            
            insert(&new_ht, old_ht->entries[i].key, old_ht->entries[i].data);
        }
    }
    
    
    free_table(old_ht);
    
    
    *ht_ptr = new_ht;
}


void print_table(HashTable* ht) {
    printf("\n--- Estado da Tabela Hash (Tamanho: %d, Ocupação: %d) ---\n", ht->size, ht->count);
    for (int i = 0; i < ht->size; i++) {
        printf("Índice [%2d]: ", i);
        if (ht->entries[i].state == OCCUPIED) {
            printf("OCUPADO  | Chave: '%s', Dados: '%s'\n", ht->entries[i].key, ht->entries[i].data);
        } else if (ht->entries[i].state == REMOVED) {
            printf("REMOVIDO | (Chave anterior: '%s')\n", ht->entries[i].key);
        } else {
            printf("LIVRE\n");
        }
    }
    printf("----------------------------------------------------------\n");
}


void load_from_file(HashTable** ht_ptr, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        
        line[strcspn(line, "\r\n")] = 0;

        char* key = strtok(line, ",");
        char* data = strtok(NULL, ""); 

        if (key && data) {
            insert(ht_ptr, key, data);
        }
    }

    fclose(file);
}


int main() {
    HashTable* ht = create_table(INITIAL_TABLE_SIZE);
    
    printf("Carregando dados do arquivo 'palavras.txt'...\n");
    load_from_file(&ht, "palavras.txt");
    print_table(ht);

    int choice;
    char key_buffer[256];
    char data_buffer[512];

    do {
        printf("\nMenu de Operações:\n");
        printf("1. Inserir um elemento\n");
        printf("2. Buscar um elemento\n");
        printf("3. Remover um elemento\n");
        printf("4. Mostrar tabela\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &choice);
        
        
        while(getchar() != '\n');

        switch (choice) {
            case 1:
                printf("Digite a chave (palavra): ");
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\r\n")] = 0;

                printf("Digite os dados associados: ");
                fgets(data_buffer, sizeof(data_buffer), stdin);
                data_buffer[strcspn(data_buffer, "\r\n")] = 0;
                
                insert(&ht, key_buffer, data_buffer);
                printf("'%s' inserido.\n", key_buffer);
                break;
            case 2:
                printf("Digite a chave para buscar: ");
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\r\n")] = 0;
                
                HashEntry* found = search(ht, key_buffer);
                if (found) {
                    printf("Encontrado! Chave: '%s', Dados: '%s'\n", found->key, found->data);
                } else {
                    printf("Chave '%s' não encontrada.\n", key_buffer);
                }
                break;
            case 3:
                printf("Digite a chave para remover: ");
                fgets(key_buffer, sizeof(key_buffer), stdin);
                key_buffer[strcspn(key_buffer, "\r\n")] = 0;

                if (remove_entry(ht, key_buffer)) {
                    printf("Chave '%s' removida com sucesso.\n", key_buffer);
                } else {
                    printf("Falha ao remover. Chave '%s' não encontrada.\n", key_buffer);
                }
                break;
            case 4:
                print_table(ht);
                break;
            case 0:
                printf("Encerrando o programa.\n");
                break;
            default:
                printf("Opção inválida!\n");
                break;
        }

    } while (choice != 0);

    free_table(ht);
    return 0;
}