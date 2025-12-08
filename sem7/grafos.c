#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define max_char 50

typedef struct Cidade{
    int id;
    double x;
    double y;

}Cidade;

double calcular_distancia(Cidade c1, Cidade c2){
    double xd = c1.x - c2.x;
    double yd = c1.y - c2.y;
    return sqrt((xd * xd) + (yd * yd));
}

Cidade* carregar_arquivo(char* nome_arquivo, int* num_cidades){
    FILE* f = fopen(nome_arquivo, "r");
    if(!f){
        printf("Erro ao ler o arquivo\n");
        return NULL;
    }
    char linha[max_char];
    int dimensao = 0;
    int flag_coordenadas = 0;
    Cidade* cidades = NULL;
    int count = 0;

    //le até o final do arquivo
    while(fgets(linha, max_char, f)){
        //Leitura cabeçalho
        if(!flag_coordenadas){
            if(strstr(linha, "DIMENSION")){
                char* ptr = strchr(linha, ':');
                if(ptr){
                    dimensao = atoi(ptr + 1);
                    num_cidades = dimensao;

                    cidades = (Cidade*) malloc(dimensao* sizeof(Cidade));
                    printf("Cidades encontradas: dimensão = %d\n", dimensao);
                }
            } else if (strstr(linha, "NODE_COORD_SECTION")) {
            flag_coordenadas = 1;
            printf("Iniciando a leitura das coordenadas \n");
            
            }
        
        }else {
            if (strstr(linha, "EOF")) break;
            int id;
            double x,y;
            if(sscanf(linha, "%d %lf %lf", &id, &x, &y) == 3){
                if(count < dimensao){
                    cidades[count].id = id;
                    cidades[count].x = x;
                    cidades[count].y = y;
                    count ++;
                }
            }
        }
        

    }
    fclose(f);
    return cidades;
}

// Matriz
typedef struct Grafo_Matriz{
    double** adj;
    int num_vertices;
    int capacidade;

}Grafo_Matriz;

Grafo_Matriz* criar_grafo_matriz(int max_v) {
    Grafo_Matriz* g = (Grafo_Matriz*)malloc(sizeof(Grafo_Matriz));
    g->num_vertices = 0;
    g->capacidade = max_v;

    // Aloca um array de ponteiros (linhas)
    g->adj = (double**)malloc(max_v * sizeof(double*));
    
    //Para cada linha, aloca um array de doubles (colunas)
    for (int i = 0; i < max_v; i++) {
        g->adj[i] = (double*)malloc(max_v * sizeof(double));
        // Inicializa com 0.0
        for (int j = 0; j < max_v; j++) {
            g->adj[i][j] = 0.0;
        }
    }
    return g;
}

void inserir_grafo_matriz(Grafo_Matriz* grafo,Cidade nova_cidade, Cidade* cidades){
    if(grafo->num_vertices >= grafo->capacidade){
        printf("Matriz cheia\n");
        return;
    }
    int indice_novo_vertices = grafo->num_vertices;
    for(int i = 0;i < indice_novo_vertices ; i++){
        double distancia = calcular_distancia(nova_cidade, cidades[i]);
        grafo->adj[indice_novo_vertices][i] = distancia;
        grafo->adj[i][indice_novo_vertices] = distancia;
    }
    grafo->adj[indice_novo_vertices][indice_novo_vertices] = 0.0;
    grafo->num_vertices ++;

}

void remover_grafo_matriz(Grafo_Matriz* grafo, int id_remover){
    if(id_remover < 0 || id_remover >= grafo->num_vertices){
        return; //id não existe;
    }
    int n = grafo->num_vertices;
    for (int i = id_remover; i < n - 1; i++) {
        for (int j = 0; j < n; j++) {
            grafo->adj[i][j] = grafo->adj[i + 1][j];
        }
    }
    for (int i = id_remover; i < n - 1; i++) {
        for (int j = 0; j < n; j++) {
            grafo->adj[i][j] = grafo->adj[i][j+1];
        }
    }
    grafo->num_vertices --;
}

// Complexidade de Tempo: O(1) -> Acesso direto indexado
double busca_grafo_matriz(Grafo_Matriz* g, int u, int v) {
    if (u >= g->num_vertices || v >= g->num_vertices) return -1.0;
    return g->adj[u][v];
}

void editar_grafo_matriz(Grafo_Matriz* g, int u, int v,double distancia){
    if (u >= g->num_vertices || v >= g->num_vertices) return;
    g->adj[u][v] = distancia;
    g->adj[v][u] = distancia;

}
void free_grafo_matriz(Grafo_Matriz* grafo){
    if(grafo == NULL) return;
    for(int i = 0;i < grafo->capacidade; i++){
        free(grafo->adj[i]);
    }
    free(grafo->adj);
    free(grafo);
    print("Free executado com sucesso\n");

}

//Lista

typedef struct no_lista{
    int dest; //id do vizinho
    double peso;
    no_lista* proximo;
}no_lista;

typedef struct Grafo_lista{
    no_lista** cabeca;
    int num_vertices;
    int capacidade;
}Grafo_lista;

Grafo_lista* criar_Grafo_lista(int max_v){
    Grafo_lista* g = (Grafo_lista*)malloc(sizeof(Grafo_lista));
    g->num_vertices = 0;
    g->capacidade = max_v;

    g->cabeca = (no_lista*)malloc(max_v * sizeof(no_lista*));
    for(int i = 0; i < max_v; i++){
        g->cabeca[i] = NULL;
    }
    return g;
}
no_lista* novo_no_lista(int dest, double peso){
    no_lista* novo_no = (no_lista*)malloc(sizeof(no_lista));
    novo_no->dest = dest;
    novo_no->peso = peso;
    novo_no->proximo = NULL;
    return novo_no;
}
void inserir_vertice_lista(Grafo_lista* g, Cidade novaCidade, Cidade* lista_cidades){
    if(g->num_vertices >= g->capacidade) return;
    int u = g->num_vertices;
    g->cabeca[u] = NULL;
    for (int i = 0; i < u; i++){
        double dist = calcular_distancia(lista_cidades[i], novaCidade);

        no_lista* nodoA = novo_no_lista(i, dist);
        nodoA->proximo = g->cabeca[u];
        g->cabeca[u] = nodoA;

        no_lista* nodoB = novo_no_lista(u, dist);
        nodoB->proximo = g->cabeca[i];
        g->cabeca[i] = nodoB;

    }
    g->num_vertices ++;
}
void remover_no_lista(Grafo_lista* g, int v_idx) {
    if (v_idx < 0 || v_idx >= g->num_vertices) return;

    no_lista* curr = g->cabeca[v_idx];
    while (curr) {
        no_lista* temp = curr;
        curr = curr->proximo;
        free(temp);
    }

    
    for (int i = 0; i < g->num_vertices; i++) {
        if (i == v_idx) continue; // Pula o removido

        no_lista* prev = NULL;
        no_lista* node = g->cabeca[i];

        
        while (node) {
        
            if (node->dest == v_idx) {
                if (prev == NULL) g->cabeca[i] = node->proximo;
                else prev->proximo = node->proximo;
                
                no_lista* toFree = node;
                node = node->proximo; // Avança
                free(toFree);
            }
            // Caso 2: Aresta aponta para um ID maior que o removido -> DECREMENTAR
            // (Ex: apontava pro 6, agora aponta pro 5)
            else {
                if (node->dest > v_idx) {
                    node->dest--; 
                }
                prev = node;
                node = node->proximo;
            }
        }
    }

    // Passo C: Shift no array principal (tapar o buraco no array head)
    for (int i = v_idx; i < g->num_vertices - 1; i++) {
        g->cabeca[i] = g->cabeca[i+1];
    }
    g->cabeca[g->num_vertices - 1] = NULL; // Limpa o último
    g->num_vertices--;
}


int main() {
    int n = 0;
    Cidade* array_cidades = carregar_arquivo("qa194.tsp", &n);
    if (array_cidades) {
        printf("Cidades carregadas com sucesso\n");
        free(array_cidades);

    }
    return 0;
}