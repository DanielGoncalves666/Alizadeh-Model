/* 
   File: impressao.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém funções dedicadas à impressão de dados relacionados às simulações
*/

#include<stdio.h>
#include<string.h>
#include<time.h>
#include"../headers/global_declarations.h"
#include"../headers/impressao.h"

/**
 * Imprime o comando recebido via terminal.
 * 
 * @param commands Estrutrura contendo parte dos dados de entrada.
 * @param arquivo_saida Arquivo onde os dados devem ser armazenados
*/
void imprimir_comando(struct command_line commands, FILE *arquivo_saida)
{
	fprintf(arquivo_saida, "./alizadeh.sh%s", commands.comando_completo);
	fprintf(arquivo_saida,"\n--------------------------------------------------------------\n\n");
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
			if(grid_pedestres[i][h] != 0)
				fprintf(arquivo_saida,"👤");
			else if(saidas.static_combined_field[i][h] == VALOR_SAIDA)
				fprintf(arquivo_saida,"🚪");
			else if(saidas.static_combined_field[i][h] == VALOR_PAREDE)
				fprintf(arquivo_saida,"🧱"); // imprime parede
			else if(grid_pedestres[i][h] == 0)
				fprintf(arquivo_saida,"⬛"); // célula vazia
		}
		fprintf(arquivo_saida,"\n");
	}
    fprintf(arquivo_saida,"\n");
}

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

/**
 * Imprime um cabeçalho contendo as correspondentes saídas de cada grupo de simulação.
 * 
 * @param arquivo_saida O arquivo onde as informações devem ser impressas
*/
void imprimir_cabecalho(FILE *arquivo_saida)
{
	char separador = ',';
    char agregador = '+';

	fprintf(arquivo_saida, "Conjunto de saídas:");
	for(int s = 0; s < saidas.num_saidas; s++)
	{
		if(s == saidas.num_saidas - 1)
			separador = '.';
		
		int largura_saida = saidas.vet_saidas[s]->largura;
		for(int c = 0; c < largura_saida; c++)
		{ 
			celula cel = saidas.vet_saidas[s]->loc[c];
			fprintf(arquivo_saida, " %d %d%c", cel.loc_lin, cel.loc_col, 
									c == largura_saida - 1 ? separador : agregador);
		}

	}

	fprintf(arquivo_saida, "\n");
}

/**
 * Imprime a mensagem de status da execução do programa.
 * 
 * @param conjunto_index Indica qual o conjunto de simulações finalizado.
 * @param numero_conjuntos Indica o número total de conjuntos de simulações
*/
void imprimir_status(int conjunto_index, int numero_conjuntos)
{
	char dataHora[51];
            
	time_t atual = time(NULL);
	struct tm * timeinfo = localtime(&atual);
	
	strftime(dataHora,50,"%F %Z %T",timeinfo);
	fprintf(stdout, "Conjunto de simulação %5d/%d finalizado às %s.\n", conjunto_index + 1, numero_conjuntos, dataHora);
}