/* 
   File: celula.c
   Author: Daniel Gonçalves
   Date: 2023-12-09
   Description: Contém funções que realizam operações sobre conjuntos de células.
*/

#include<stdio.h>
#include<stdlib.h>
#include"../headers/global_declarations.h"
#include"../headers/celula.h"

int particao(celula *vet, int ini, int fim);
void troca(celula *vet, int a, int b);

/**
 * Ordena o vetor de células passado de acordo com o valor do piso, em ordem crescente.
 * 
 * @note Utiliza Quicksort
 * 
 * @param vet vetor de células
*/
void ordenar_vetor_celulas(celula *vet, int ini, int fim)
{
    if(ini >= fim)
        return;

    int pivo = particao(vet, ini, fim);
    ordenar_vetor_celulas(vet,ini,pivo - 1);
    ordenar_vetor_celulas(vet, pivo + 1, fim);
}

/**
 * Função particao do algoritmo Quicksort
 * 
 * @param vet vetor de células
 * @param ini início do intervalo
 * @param fim fim do intervalo
 * 
 * @return inteiro, indicando a posição do pivô
*/
int particao(celula *vet, int ini, int fim)
{
    int pivo = ini;
    for(int i = pivo + 1; i <= fim; i++)
    {
        if(vet[i].valor < vet[ini].valor)
        {
            pivo = pivo + 1;
            troca(vet,pivo,i);
        }
    }
    troca(vet,ini,pivo);
    return pivo;
}

/**
 * Função de troca do algoritmo Quicksort
 * 
 * @param vet vetor de células
 * @param a primeira célula.
 * @param b segunda célula
*/
void troca(celula *vet, int a, int b)
{
    celula buffer = vet[a];
    vet[a] = vet[b];
    vet[b] = buffer;
}

/**
 * Realiza uma busca binária pelo valor passado no vetor de células.
 * 
 * @param vet vetor de células
 * @param N número de células
 * @param valor valor procurado
 * @param qtd_iguais quantidade de células iguais à VALOR
 * 
 * @return número de células menores que VALOR, ou -1, caso não exista nenhuma célula menor que VALOR
 * 
*/
int busca_binaria_celulas(celula *vet, int N, double valor, int *qtd_iguais)
{
    int esq = 0;
    int dir = N - 1;
    int meio;

    int qtd_menor = -1;
    *qtd_iguais = 0;

    if(valor < vet[0].valor)
        return qtd_menor;

    if(valor > vet[N - 1].valor)
        return N;

    while(esq <= dir)
    {
        meio = (esq + dir) / 2;

        if(vet[meio].valor == valor)
        {
            int i = meio;

            // verifica se existe VALOR à direita
            while(i >= 0 && vet[i].valor == valor)
            {
                i--;
                (*qtd_iguais)++;
            }
            qtd_menor = i + 1; // menor índice onde existe VALOR (equivalente à quantidade de células menores que VALOR)

            i = meio + 1;
            // verifica se existe VALOR à esquerda
            while(i < N && vet[i].valor == valor)
            {
                i++;
                (*qtd_iguais)++;
            }

            break;
        }
        else
        {
            if(vet[meio].valor > valor)
                dir = meio - 1;
            else
                esq = meio + 1;
        }
    }

    // caso não exista célula ocupada com o campo de piso VALOR
    if(qtd_menor == -1)
    {
        // pelo próprio algoritmo quicksort temos que VALOR será maior que vet[esq - 1] e menor que vet[esq + 1].
        // Às vezes VALOR será menor que vet[esq] e às vezes maior.

        // Desta forma, temos garantido que as células entre 0 e esq - 1 (totalizando 'esq') são menores que VALOR
        qtd_menor = esq;

        //devemos então verificar se vet[esq] é menor que VALOR
        if(vet[esq].valor < valor)
            qtd_menor++;
    }

    return qtd_menor;
}