/* 
   File: global_declarations.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém declarações e definições de variáveis globais e estruturas que serão usadas por todo o programa.
                Além disso, contém funções utilitárias.
*/

#include<stdio.h>
#include<stdlib.h>
#include"../headers/global_declarations.h"

int num_lin_grid = 0;
int num_col_grid = 0;
int numero_simulacoes = 1; // uma única simulação por padrão
int numero_pedestres = 1;
int original_seed = 0;

Grid grid_esqueleto = NULL; // grid contendo paredes e saidas
Grid grid_pedestres = NULL; // grid contendo apenas a localização dos pedestres
Grid grid_mapa_calor = NULL; // armazena a quantidade de vezes que um pedestre esteve em uma dada célula
Conjunto_saidas saidas = {NULL, NULL, 0};
Conjunto_pedestres pedestres = {NULL,0};
Command_line commands = {"sala_padrao.txt","","",1,0,3,0,0};

/**
 * Aloca de forma dinâmica uma matriz de inteiros de dimensão NUM_LIN x NUM_COL
 *
 * @param num_lin Número de linhas.
 * @param num_col Número de colunas.
 * @return Matriz de inteiros zerada ou NULL.
 */
int **alocar_matriz_int(int num_lin, int num_col)
{
    if(num_lin <= 0 || num_col <= 0)
    {
        fprintf(stderr,"Dimensões do matriz nulas ou negativas.\n");
        return NULL;
    }

    int **novo = malloc(sizeof(int *) * num_lin);
    if( novo == NULL )
        return NULL;
    
    for(int i = 0; i < num_lin; i++)
    {
        novo[i] = calloc(num_col, sizeof(int));
        if(novo[i] == NULL)
            return NULL;
    }

    return novo;
}

/**
 * Zera a matriz de inteiros indicada.
 *
 * @param mat Matriz de inteiros
 * @param num_lin Número de linhas.
 * @param num_col Número de colunas.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
 */
int zerar_matriz_inteiros(int **mat, int num_lin, int num_col)
{
    if(mat == NULL)
        return 1;

    for(int i = 0; i < num_lin; i++)
    {
        if(mat[i] == NULL)
            return 1;

        for(int h = 0; h < num_col; h++)
            mat[i][h] = 0;
    }

    return 0;
}

/**
 * Desaloca a matriz de inteiros indicada.
 *
 * @param mat Matriz de inteiros
 * @param num_lin Número de linhas.
 */
void desalocar_matriz_int(int **mat, int lin)
{
    if(mat != NULL)
    {
        for(int i = 0; i < lin; i++)
        {
            free(mat[i]);
        }
        free(mat);
    }
}

/**
 * Aloca de forma dinâmica uma matriz de doubles de dimensão NUM_LIN x NUM_COL.
 *
 * @param num_lin Número de linhas.
 * @param num_col Número de colunas.
 * @return Matriz de doubles zerado ou NULL.
 */
double **alocar_matriz_double(int num_lin, int num_col)
{
    if(num_lin <= 0 || num_col <= 0)
    {
        fprintf(stderr,"Dimensões do matriz nulas ou negativas.\n");
        return NULL;
    }

    double **novo = malloc(sizeof(double *) * num_lin);
    if( novo == NULL )
        return NULL;
    
    for(int i = 0; i < num_lin; i++)
        novo[i] = calloc(num_col, sizeof(double));

    return novo;
}

/**
 * Desaloca a matriz de doubles indicada
 *
 * @param mat Matriz de doubles.
 * @param num_lin Número de linhas.
 */
void desalocar_matriz_double(double **mat, int lin)
{
    if(mat != NULL)
    {
        for(int i = 0; i < lin; i++)
        {
            free(mat[i]);
        }
        free(mat);
    }
}

/**
 * Copia o conteúdo da matriz SRC para a matriz DEST.
 *
 * Comportamento indefinido ocorrerá se as matrizes não forem de mesmo tamanho.
 * 
 * @param dest Matriz de doubles que será o destino.
 * @param src Matriz de doubles que será a fonte.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
 */
int copiar_matriz_double(double **dest, double **src)
{
    if(dest == NULL || src == NULL)
        return 0;

    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            dest[i][h] = src[i][h];
        }
    }
}

/**
 * Verifica se a diagonal indicada por [loc_lin,loc_col] e [j,k] pode ser atravessada (mão há obstáculos dos lados).
 *
 * @param loc_lin Linha da célula base.
 * @param loc_col Coluna da célula base.
 * @param j Modificador de linha. Quando somado à LOC_LIN indica a linha da célula alvo.
 * @param k Modificador de coluna. Quando somado à LOC_COL indica a coluna da célula alvo.
 * @param mat Matriz de doubles. Indica o campo de piso.
 * @return Inteiro, 0 (diagonal inválida) ou 1 (diagonal válida).
 */
int eh_diagonal_valida(int loc_lin, int loc_col, int j, int k, double **mat)
{
    if(j == -1 && k == -1)
    {
        if(loc_lin - 1 >= 0 && mat[loc_lin - 1][loc_col] == VALOR_PAREDE)
        {
            if(loc_col - 1 >= 0 && mat[loc_lin][loc_col - 1] == VALOR_PAREDE)
                return 0;
        }
    }
    else if(j == -1 && k == 1)
    {
        if(loc_lin - 1 >= 0 && mat[loc_lin - 1][loc_col] == VALOR_PAREDE)
        {
            if(loc_col + 1 < num_col_grid && mat[loc_lin][loc_col + 1] == VALOR_PAREDE)
                return 0;
        }
    }
    else if(j == 1 && k == -1)
    {
        if(loc_lin + 1 < num_lin_grid && mat[loc_lin + 1][loc_col] == VALOR_PAREDE)
        {
            if(loc_col - 1 >= 0 && mat[loc_lin][loc_col - 1] == VALOR_PAREDE)
                return 0;
        }
    }
    else if(j == 1 && k == 1)
    {
        if(loc_lin + 1 < num_lin_grid && mat[loc_lin + 1][loc_col] == VALOR_PAREDE)
        {
            if(loc_col + 1 < num_col_grid && mat[loc_lin][loc_col + 1] == VALOR_PAREDE)
                return 0;
        }
    }

    return 1;
}