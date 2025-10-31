#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int idOriginal;
    int valor;
    int peso;
    double razaoValorPeso; // A métrica para a escolha gulosa
} ItemInfo;

// Função de comparação para qsort, para ordenar os itens
// pela razão valor/peso em ordem decrescente.
int compararItens(const void *a, const void *b) {
    ItemInfo *itemA = (ItemInfo *)a;
    ItemInfo *itemB = (ItemInfo *)b;
    
    // Queremos ordem decrescente, por isso comparamos B com A
    if (itemB->razaoValorPeso > itemA->razaoValorPeso) {
        return 1;
    } else if (itemB->razaoValorPeso < itemA->razaoValorPeso) {
        return -1;
    }
    return 0;
}


void cestoGuloso(int capacidadeMochila, int pesos[], int valores[], int numeroDeItens) {
    ItemInfo *listaDeItens = (ItemInfo *)malloc(numeroDeItens * sizeof(ItemInfo));

    
    for (int i = 0; i < numeroDeItens; i++) {
        listaDeItens[i].idOriginal = i;
        listaDeItens[i].valor = valores[i];
        listaDeItens[i].peso = pesos[i];
        if (pesos[i] > 0) {
            listaDeItens[i].razaoValorPeso = (double)valores[i] / pesos[i];
        } else {
            listaDeItens[i].razaoValorPeso = 0; 
        }
    }

    
    qsort(listaDeItens, numeroDeItens, sizeof(ItemInfo), compararItens);

    int pesoAtualNaMochila = 0;
    double valorTotalNaMochila = 0.0;

    printf("Solução Gulosa (Maior Razão Valor/Peso):\n");
    printf("Itens escolhidos (índice, peso, valor):\n");

    
    for (int i = 0; i < numeroDeItens; i++) {
        if (pesoAtualNaMochila + listaDeItens[i].peso <= capacidadeMochila) {
            pesoAtualNaMochila += listaDeItens[i].peso;
            valorTotalNaMochila += listaDeItens[i].valor;
            printf("  - Item %d (Peso: %d, Valor: %d)\n", listaDeItens[i].idOriginal, listaDeItens[i].peso, listaDeItens[i].valor);
        }
    }

    printf("Valor máximo na mochila = %.2f\n", valorTotalNaMochila);
    printf("Peso total na mochila = %d\n", pesoAtualNaMochila);

    free(listaDeItens);
}

int main() {
    int valoresItens[] = {60, 100, 120};
    int pesosItens[] = {10, 20, 30};
    int capacidadeTotalMochila = 50;
    int totalDeItens = sizeof(valoresItens) / sizeof(valoresItens[0]);

    cestoGuloso(capacidadeTotalMochila, pesosItens, valoresItens, totalDeItens);

    return 0;
}