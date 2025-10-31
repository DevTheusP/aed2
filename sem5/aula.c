#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const char *nome;
    double peso;
    int valor;
} Item;
int compare(const void* a, const void* b) {
   return (*(int*)a - *(int*)b);
}
int main(void) {
    Item itens[] = {
        {"Casca de Carapanaúba", 1.8, 16},
        {"Yãkoana", 4.0, 34},
        {"Tucumã", 2.5, 15},
        {"Jenipapo", 4.5, 28},
        {"Fibra", 1.0, 10}
    };
    int n = sizeof(itens)/sizeof(itens[0]);
    double capacidade = 7.0;

    double razao = (double)malloc(sizeof(double)*n);
    for(int i = 0; i < n; i++){
        razao[i] = (double)itens[i].valor / itens[i].peso;
        printf("\n%lf", razao[i]);
    }
    qsort(razao,n,sizeof(razao[0]),compare);
    printf("\nDepois de ordenar");
    for(int i = 0; i < n; i++){
        printf("\n%lf", razao[i]);
    }
    


    /*int total = 1 << n;
    double melhor_peso = 0.0;
    int melhor_valor = 0;
    int melhor_mask = 0;

    for (int mask = 0; mask < total; mask++) {
        double peso = 0.0;
        int valor = 0;
        for (int i = 0; i < n; i++) {
            if (mask & (1 << i)) {
                peso += itens[i].peso;
                valor += itens[i].valor;
            }
        }
        if (peso <= capacidade && valor > melhor_valor) {
            melhor_valor = valor;
            melhor_peso = peso;
            melhor_mask = mask;
        }
    }

    printf("=== Solução Ótima (força bruta) ===\n");
    printf("Itens escolhidos:\n");
    for (int i = 0; i < n; ++i) {
        if (melhor_mask & (1 << i))
            printf(" - %s (peso %.1f, valor %d)\n", itens[i].nome, itens[i].peso, itens[i].valor);
    }
    printf("Peso total: %.1f kg\n", melhor_peso);
    printf("Valor total: %d\n", melhor_valor);*/

    return 0;
}
