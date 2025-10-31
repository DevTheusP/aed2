#include <stdio.h>
#include <stdlib.h>


int max(int a, int b) {
    return (a > b) ? a : b;
}


void cestoOtimo(int capacidadeMochila, int pesos[], int valores[], int numeroDeItens) {
    // Aloca a matriz para a programação dinâmica (tabelaDP).
    // As linhas representam os itens e as colunas a capacidade.
    int **tabelaDP = (int **)malloc((numeroDeItens + 1) * sizeof(int *));
    for (int i = 0; i <= numeroDeItens; i++) {
        tabelaDP[i] = (int *)malloc((capacidadeMochila + 1) * sizeof(int));
    }

    // Constrói a tabelaDP de forma iterativa
    for (int indiceItem = 0; indiceItem <= numeroDeItens; indiceItem++) {
        for (int capacidadeAtual = 0; capacidadeAtual <= capacidadeMochila; capacidadeAtual++) {
            if (indiceItem == 0 || capacidadeAtual == 0) {
                // Caso base: se não há itens ou a mochila não tem capacidade, o valor é 0.
                tabelaDP[indiceItem][capacidadeAtual] = 0;
            } else if (pesos[indiceItem - 1] <= capacidadeAtual) {
                // Se o item atual cabe na capacidade sendo avaliada,
                // decidimos se vale a pena incluí-lo.
                int valorComItem = valores[indiceItem - 1] + tabelaDP[indiceItem - 1][capacidadeAtual - pesos[indiceItem - 1]];
                int valorSemItem = tabelaDP[indiceItem - 1][capacidadeAtual];
                tabelaDP[indiceItem][capacidadeAtual] = max(valorComItem, valorSemItem);
            } else {
                // Se o item atual é mais pesado que a capacidade,
                // não podemos incluí-lo. O valor é o mesmo da solução anterior.
                tabelaDP[indiceItem][capacidadeAtual] = tabelaDP[indiceItem - 1][capacidadeAtual];
            }
        }
    }

    printf("Solução Ótima (Programação Dinâmica):\n");
    printf("Valor máximo na mochila = %d\n", tabelaDP[numeroDeItens][capacidadeMochila]);

    // O código abaixo reconstrói o caminho para descobrir quais itens foram escolhidos
    printf("Itens escolhidos (índice, peso, valor):\n");
    int valorResultado = tabelaDP[numeroDeItens][capacidadeMochila];
    int capacidadeRestante = capacidadeMochila;

    for (int indiceItem = numeroDeItens; indiceItem > 0 && valorResultado > 0; indiceItem--) {
        // Se o valor mudou em relação à linha anterior, significa que o item atual foi incluído
        if (valorResultado != tabelaDP[indiceItem - 1][capacidadeRestante]) {
            printf("  - Item %d (Peso: %d, Valor: %d)\n", indiceItem - 1, pesos[indiceItem - 1], valores[indiceItem - 1]);
            
            // Atualiza o valor e a capacidade para continuar a busca
            valorResultado -= valores[indiceItem - 1];
            capacidadeRestante -= pesos[indiceItem - 1];
        }
    }
    printf("\n");

    // Libera a memória alocada dinamicamente
    for (int i = 0; i <= numeroDeItens; i++) {
        free(tabelaDP[i]);
    }
    free(tabelaDP);
}

int main() {
    int valoresItens[] = {60, 100, 120};
    int pesosItens[] = {10, 20, 30};
    int capacidadeTotalMochila = 50;
    int totalDeItens = sizeof(valoresItens) / sizeof(valoresItens[0]);

    cestoOtimo(capacidadeTotalMochila, pesosItens, valoresItens, totalDeItens);

    return 0;
}