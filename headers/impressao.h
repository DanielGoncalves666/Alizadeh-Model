#ifndef IMPRESSAO__H
#define IMPRESSAO__H

void imprimir_informacoes_gerais(struct command_line commands, FILE *arquivo_saida);
void imprimir_mapa_calor(FILE *arquivo_saida);
void imprimir_grid_pedestres(FILE *arquivo_saida);
void imprimir_piso(double **mat);

#endif