#ifndef INICIALIZACAO
#define INICIALIZACAO

int abrir_arquivo_auxiliar(struct command_line commands, FILE **arquivo_auxiliar);
int abrir_arquivo_output(struct command_line commands, FILE **arquivo_saida);
int alocar_grids();
int carregar_ambiente(char *nome, int modo);
int gerar_ambiente();
int extrair_numero_linhas(FILE *arquivo_auxiliar);
int extrair_saidas(FILE *arquivo_auxiliar);
void finalizar_programa(FILE *arquivo_saida, FILE *arquivo_auxiliar);

#endif