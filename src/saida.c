/* 
   File: saida.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém funções dedicadas à criação e administração de saídas, além do cálculo do campo de piso.
*/

#include<stdio.h>
#include<stdlib.h>
#include"../headers/global_declarations.h"
#include"../headers/saida.h"

const double regra[3][3] = {{VALOR_DIAGONAL, 1.0, VALOR_DIAGONAL},
                            {     1.0,       0.0,       1.0     },
                            {VALOR_DIAGONAL, 1.0, VALOR_DIAGONAL}};
/* Considerando que a célula central tenha valor X, 'regra' indica o valor a ser somado para
   determinar o valor dos vizinhos. */

Saida criar_saida(int loc_linha, int loc_coluna);
int determinar_piso_saida(Saida s);

/**
 * Cria uma estrutura Saida correspondente à localização passada (se válida) e a inicializa
 * 
 * @param loc_linha Linha da saída
 * @param loc_coluna Coluna da saída
 * @return Estrutura Saida ou NULL (fracasso).
*/
Saida criar_saida(int loc_linha, int loc_coluna)
{
    if(loc_linha < 0 || loc_linha >= num_lin_grid || loc_coluna < 0 || loc_coluna >= num_col_grid)
        return NULL;
    
    Saida nova = malloc(sizeof(struct saida));
    if(nova != NULL)
    {
        nova->loc_lin = loc_linha;
        nova->loc_col = loc_coluna;

        nova->field = alocar_matriz_double(num_lin_grid, num_col_grid);
    }

    return nova;
}

/**
 * Adiciona uma nova saida no conjunto de saidas.
 * 
 * @param loc_linha Linha da saída.
 * @param loc_coluna Coluna da saída.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int adicionar_saida_conjunto(int loc_linha, int loc_coluna)
{
    Saida nova = criar_saida(loc_linha, loc_coluna);
    if(nova == NULL)
    {
        fprintf(stderr,"Falha na criação da Saida em (%d,%d).\n",loc_linha, loc_coluna);
        return 1;
    }

    saidas.num_saidas += 1;
    saidas.vet_saidas = realloc(saidas.vet_saidas, sizeof(Saida) * saidas.num_saidas);
    if(saidas.vet_saidas == NULL)
    {
        fprintf(stderr, "Falha na realocação do vetor de saídas.\n");
        return 1;
    }    

    saidas.vet_saidas[saidas.num_saidas - 1] = nova;

    return 0;
}

/**
 * Desaloca as estruturas para as saídas individuais, a matriz da saída combinada e zera a quantidade de saídas.
*/
void desalocar_saidas()
{
    for(int s = 0; s < saidas.num_saidas; s++)
        free(saidas.vet_saidas[s]->field);
    free(saidas.vet_saidas);
    saidas.vet_saidas = NULL;

    desalocar_matriz_double(saidas.combined_field, num_lin_grid);
    saidas.combined_field = NULL;

    saidas.num_saidas = 0;
}

/**
 * Calcula o campo de piso referente à saida dada como argumento
 * 
 * @param s Saida cujo piso será calculado.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int determinar_piso_saida(Saida s)
{
    if(s == NULL)
    {
        fprintf(stderr, "NULL como entrada em 'determinar_piso_saida'.\n");
        return 1;
    }

    double **mat = s->field;

    // adiciona as paredes no piso da saida (outras saidas também são consideradas como paredes)
    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            double conteudo = grid_esqueleto[i][h];
            if(conteudo == VALOR_PAREDE)
                mat[i][h] = VALOR_PAREDE;
            else
                mat[i][h] = 0.0;
        }
    }
    mat[s->loc_lin][s->loc_col] = VALOR_SAIDA; // Adiciona a saida

    double **aux = alocar_matriz_double(num_lin_grid,num_col_grid);
    // matriz para armazenar as alterações para o tempo t + 1 do autômato
    if(aux == NULL)
    {
        fprintf(stderr,"Falha na alocação da matriz auxiliar para cálculo do piso.\n");
        return 1;
    }

    copiar_matriz_double(aux, mat); // copia o piso base para a matriz auxiliar

    int qtd_mudancas;
    do
    {
        qtd_mudancas = 0;
        for(int i = 0; i < num_lin_grid; i++)
        {
            for(int h = 0; h < num_col_grid; h++)
            {
                double atual = mat[i][h]; // valor atual de piso na posição dada

                if(atual == VALOR_PAREDE || atual == 0.0) 
                    continue; // cálculo de piso dos vizinhos ocorre apenas para células com valor de piso já dado

                for(int j = -1; j < 2; j++)
                {
                    if(i + j < 0 || i + j >= num_lin_grid)
                        continue;

                    for(int k = -1; k < 2; k++)
                    {
                        if(h + k < 0 || h + k >= num_col_grid)
                            continue;

                        if(mat[i + j][h + k] == VALOR_PAREDE || mat[i + j][h + k] == VALOR_SAIDA)
                            continue;

                        if(j != 0 && k != 0)
                        {
                            if(! eh_diagonal_valida(i,h,j,k,mat))
                                continue;
                        }

                        double novo_valor = mat[i][h] + regra[1 + j][1 + k];
                        if(aux[i + j][h + k] == 0.0)
                        {    
                            aux[i + j][h + k] = novo_valor;
                            qtd_mudancas++;
                        }
                        else if(novo_valor < aux[i + j][h + k])
                        {
                            aux[i + j][h + k] = novo_valor;
                            qtd_mudancas++;
                        }
                    }
                }
            }
        }
        copiar_matriz_double(mat,aux);
    }
    while(qtd_mudancas != 0);

    return 0;
}

/**
 * Determina o piso geral do ambiente por meio da fusão dos pisos para cada saída.
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (falha).
*/
int determinar_piso_geral()
{
    if(saidas.num_saidas <= 0 || saidas.vet_saidas == NULL)
    {
        fprintf(stderr,"O número de saídas (%d) é inválido ou o vetor de saidas é NULL.\n", saidas.num_saidas);
        return 1;
    }

    for(int q = 0; q < saidas.num_saidas; q++)
    {
        if( determinar_piso_saida(saidas.vet_saidas[q]))
            return 1;
    }

    saidas.combined_field = alocar_matriz_double(num_lin_grid, num_col_grid);
    if(saidas.combined_field == NULL)
    {
        fprintf(stderr,"Falha na alocação da matriz de piso combinado.\n");
        return 1;
    }

    copiar_matriz_double(saidas.combined_field, saidas.vet_saidas[0]->field); // copia o piso da primeira porta
    
    for(int q = 1; q < saidas.num_saidas; q++)
    {
        for(int i = 0; i < num_lin_grid; i++)
        {
            for(int h = 0; h < num_col_grid; h++)
            {
                if(saidas.combined_field[i][h] > saidas.vet_saidas[q]->field[i][h])
                    saidas.combined_field[i][h] = saidas.vet_saidas[q]->field[i][h];
            }
        }
    }

    return 0;
}