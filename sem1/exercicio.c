//Nome: Matheus Persch

//FALTOU IMPLEMENTAR A BUSCA E CONSEQUENTEMENTE A MARCAÇÃO NO TABULEIRO;
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX 100
#define TENTAVIAS 100


typedef struct{
    int valido;
    char letra;
}Celula;
typedef struct{
    char palavra[MAX];
    int dir[2];
    int x;
    int y;
    int tamanho;
}Palavra;

void inicializaMatriz(Celula **matriz, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            matriz[i][j].valido = 1;
            matriz[i][j].letra = ' ';
        }
    }
}
void imprimeMatriz(Celula **matriz, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(matriz[i][j].letra == ' '){
                matriz[i][j].letra = 'A' + rand() % 26;
            }
            printf("%c ", matriz[i][j].letra);
        }
        printf("\n");
    }
}
Palavra *tentaInserir(Celula **matriz, int n, char *palavra){
    int len = strlen(palavra);
    int direcoes[8][2] = {
        {0, 1},   // direita
        {1, 0},   // baixo
        {1, 1},   // diagonal direita baixo
        {0, -1},  // esquerda
        {-1, 0},  // cima
        {-1, -1}, // diagonal esquerda cima
        {1, -1},  // diagonal esquerda baixo
        {-1, 1}   // diagonal direita cima
    };
    for(int tentativa = 0; tentativa < TENTAVIAS; tentativa++){
        int dir = rand() % 8;
        int x = rand() % n;
        int y = rand() % n;
        int dx = direcoes[dir][0];
        int dy = direcoes[dir][1];
        int i;
        for(i = 0; i < len; i++){
            int nx = x + i * dx;
            int ny = y + i * dy;
            if(nx < 0 || nx >= n || ny < 0 || ny >= n) break;
            if(matriz[nx][ny].valido == 0) break;
            if(matriz[nx][ny].letra != ' ' && matriz[nx][ny].letra != palavra[i]) break;
        }
        if(i == len){
            for(i = 0; i < len; i++){
                int nx = x + i * dx;
                int ny = y + i * dy;
                matriz[nx][ny].letra = palavra[i];
                matriz[nx][ny].valido = 0;
            }
            Palavra *pal =(Palavra *)malloc(sizeof(Palavra*));
            strcpy(pal->palavra,palavra);
            pal->tamanho = strlen(palavra);
            pal->dir[0] = dx;
            pal->dir[1] = dy;
            pal->x = x;
            pal->y = y;
            return pal;
        }
        
    }
    printf("\nNão foi possível inserir a palavra: %s\n", palavra);
    return NULL;
}
void buscarPalavra(Celula **matriz,int n,Palavra **palavras){
    printf("\nDigite a palavra que você quer buscar:");
    getchar();
    char palavra[MAX];
    fgets(palavra, MAX, stdin);
    palavra[strcspn(palavra, "\n")] = 0;
    for(int i = 0; i < sizeof(palavras) / sizeof(palavras[0]); i++){
        if(strcmp(palavras[i]->palavra,palavra)){
        }
    }
}

int main(){
    srand(time(NULL));
    int n;
    printf("Digite o tamanho da Matriz NxN: ");
    scanf("%d", &n);
    printf("\nDigite o número de palavras(max 10): ");
    int numpal;
    scanf("%d",&numpal);
    FILE *f = fopen("palavras.txt","r");
    if(f == NULL){
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }
    char palavras[10][MAX];
    for(int i = 0; i < numpal; i++){
        fgets(palavras[i], MAX, f);
        palavras[i][strcspn(palavras[i], "\n")] = 0;
    }
    fclose(f);
    Celula **matriz = (Celula **)malloc(n * sizeof(Celula *));
    for(int i = 0; i < n; i++){
        matriz[i] = (Celula *)malloc(n * sizeof(Celula));
    }
    inicializaMatriz(matriz, n);
    Palavra **pal = (Palavra **)malloc(numpal * sizeof(Palavra*));
    for(int i = 0; i < numpal; i++){
        Palavra *temp = tentaInserir(matriz, n, palavras[i]);
        if(temp != NULL){
            pal[i] = temp;
            printf("\n%s alocado com sucesso\n", pal[i]->palavra);
        }
        
    }
    imprimeMatriz(matriz, n);
    int escolha = 10;
    while(escolha != 0){
        printf("\nDigite 1 para buscar palavra, 2 para mostrar o tabuleiro e 0 para sair: ");
        scanf("%d",&escolha);
        if(escolha == 1){
            buscarPalavra(matriz,n,pal);
        }else if(escolha == 2){
            imprimeMatriz(matriz, n);
        }else if(escolha == 0){
            printf("\nSaindo do programa....");
        }else{
            printf("\nEscolha incorreta , tente novamente.");
        }
    }
    free(matriz);
    free(pal);

}
