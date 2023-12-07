/* 
   File: impressao.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém funções dedicadas à impressão de dados relacionados às simulações
*/

#include<stdio.h>
#include<string.h>
#include"../headers/global_declarations.h"
#include"../headers/impressao.h"

/**
 * Imprime um cabeçalho contendo informações inseridas via linha de commando no arquivo de saída.
 * 
 * @param commands Estrutrura contendo parte dos dados de entrada.
 * @param arquivo_saida
*/
void imprimir_informacoes_gerais(struct command_line commands, FILE *arquivo_saida)
{
	if(strcmp(commands.nome_arquivo_entrada,"") != 0)
		fprintf(arquivo_saida, "Arquivo de entrada: %s.\n", commands.nome_arquivo_entrada);

	if(strcmp(commands.nome_arquivo_auxiliar, "") != 0) 
		fprintf(arquivo_saida, "Arquivo auxiliar: %s.\n", commands.nome_arquivo_auxiliar);
	
	fprintf(arquivo_saida, "Quantidade de simulações: %d.\n", numero_simulacoes);
	fprintf(arquivo_saida, "Seed inicial utilizada: %d.\n", original_seed);

	if(commands.input_method == 1 || commands.input_method == 2|| commands.input_method == 5)
		fprintf(arquivo_saida, "Quantidade de pedestres: %d.\n", numero_pedestres);
	
	if(commands.input_method == 5)
		fprintf(arquivo_saida, "Dimensões do ambiente: %d x %d.\n", num_lin_grid, num_col_grid);

	fprintf(arquivo_saida,"--------------------------------------------------------------\n\n");
}

/**
 * Imprime a grid do mapa de calor.
 * 
 * O valor de cada posição é dividido pelo número de simulações. Deste modo, o mapa de calor é um mapa de médias.
 * 
 * @param arquivo_saida
*/
void imprimir_mapa_calor(FILE *arquivo_saida)
{
    for(int i = 0; i < num_lin_grid; i++){
		for(int h = 0; h < num_col_grid; h++)
            fprintf(arquivo_saida, "%7.2lf ", (double) grid_mapa_calor[i][h] / (double) numero_simulacoes);

		fprintf(arquivo_saida,"\n");
	}
    fprintf(arquivo_saida,"\n");
}

/**
 * Imprime a grid de pedestres
 * 
 * @param arquivo_saida
*/
void imprimir_grid_pedestres(FILE *arquivo_saida)
{
	for(int i = 0; i < num_lin_grid; i++){
		for(int h = 0; h < num_col_grid; h++)
		{
			if(saidas.combined_field[i][h] == VALOR_SAIDA)
				fprintf(arquivo_saida,"🚪");
			else if(saidas.combined_field[i][h] == VALOR_PAREDE)
				fprintf(arquivo_saida,"🧱"); // imprime parede
			else if(grid_pedestres[i][h] == 0)
				fprintf(arquivo_saida,"⬛"); // célula vazia
			else
				fprintf(arquivo_saida,"👤");
		}
		fprintf(arquivo_saida,"\n");
	}
    fprintf(arquivo_saida,"\n");
}

/**
 * ## imprimir_piso
 * 
 * #### Entrada
 * Matriz de Double.
 * 
 * #### Descrição
 * Imprime o conteúdo da matriz
 * 
 * #### Saída
 * 1, em sucesso, 0, em falha
*/

/**
 * Imprime o conteúdo da matriz de campo de piso.
 * 
 * @param mat Matriz de Doubles.
*/
void imprimir_piso(double **mat)
{
	for(int i=0; i < num_lin_grid; i++){
		for(int h=0; h < num_col_grid; h++){
			if(mat[i][h] >= 1000.0)
				printf("%.0lf\t", mat[i][h]);
			else
				printf("%5.1lf\t", mat[i][h]);
		}
		printf("\n\n");
	}
	printf("\n");
}