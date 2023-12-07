/* 
   File: inicializacao.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Cria e carrega a estrutura do ambiente especificado, suas saídas e pedestres (se for o caso).
                Alternativamente, pode criar um ambiente retangular do zero, e inserir saídas e pedestres de forma 
                manual.

*/


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include"../headers/global_declarations.h"
#include"../headers/inicializacao.h"
#include"../headers/saida.h"
#include"../headers/pedestre.h"

const char *path_input = "ambientes/";
const char *path_saidas = "saidas/";
const char *path_output = "output/";

/**
 * Abre o arquivo auxiliar em modo leitura.
 * 
 * @param commands Estrutrura contendo parte dos dados de entrada.
 * @param arquivo_auxiliar Ponteiro para estrutura FILE que conterá o arquivo auxiliar.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int abrir_arquivo_auxiliar(struct command_line commands, FILE **arquivo_auxiliar)
{
    char complete_path[100] = "";
    
    if( commands.input_method == 1 || commands.input_method == 3 || commands.input_method == 5)
    {
        sprintf(complete_path,"%s%s",path_saidas,commands.nome_arquivo_auxiliar);

        *arquivo_auxiliar = fopen(complete_path,"r");
        if(*arquivo_auxiliar == NULL)
        {
            fprintf(stderr, "Não foi possível abrir o arquivo auxiliar.\n");
            return 1;
        }
    }

    return 0;
}

/**
 * Abre o arquivo de saída em modo leitura.
 * 
 * @note Se nenhum arquivo de saída tiver sido informado ou a opção de geração de nome automático escolhida,
 * seta a saída para stdout. 
 * 
 * @param commands Estrutrura contendo parte dos dados de entrada.
 * @param arquivo_auxiliar Ponteiro para estrutura FILE que conterá o arquivo de saida.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int abrir_arquivo_output(struct command_line commands, FILE **arquivo_saida)
{
    char complete_path[300] = "";
    
    time_t timer;
    time(&timer);
    struct tm *agora = gmtime(&timer);
    
    if(commands.output_to_file)
    {
        // se nenhum nome tiver sido passado
        if(strcmp(commands.nome_arquivo_saida, "") == 0)
        {
            char *output_type_name;
            if(commands.output_type == 1)
                output_type_name = "visual";
            else if(commands.output_type == 2)
                output_type_name = "tempo_discreto";
            else
                output_type_name = "mapa_calor";

            int horas = agora->tm_hour - 3 < 0 ? 24 + (agora->tm_hour - 3): agora->tm_hour - 3;

            sprintf(complete_path,"%s%s-%s-%02d %02d %d-%02d:%02d:%02d.txt", path_output, output_type_name, 
                    commands.nome_arquivo_entrada, agora->tm_mday, agora->tm_mon + 1, 
                    agora->tm_year + 1900, horas, agora->tm_min, agora->tm_sec);
        }
        else
            sprintf(complete_path,"%s%s",path_output,commands.nome_arquivo_saida);


        *arquivo_saida = fopen(complete_path,"w");
        if(*arquivo_saida == NULL)
        {
            fprintf(stderr, "Não foi possível criar o arquivo de output.\n");
            return 1;
        }
    }
    else
        *arquivo_saida = stdout;

    return 0;
}

/**
 * Aloca as matriz de inteiros das grids.
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int alocar_grids()
{
    grid_esqueleto = alocar_matriz_int(num_lin_grid, num_col_grid);
    grid_pedestres = alocar_matriz_int(num_lin_grid, num_col_grid);
    grid_mapa_calor = alocar_matriz_int(num_lin_grid, num_col_grid);
    if(grid_esqueleto == NULL || grid_pedestres == NULL || grid_mapa_calor == NULL)
    {
        fprintf(stderr,"Falha na alocação das grids com dimensões %d x %d.\n", num_lin_grid, num_col_grid);
        return 1;
    }

    return 0;
}

/**
 * Carrega o ambiente armazenado no arquivo de entrada indicado por NOME.
 * 
 * @note O ambiente é carregado considerando 1 de três modos:
 * @note 1: apenas paredes.
 * @note 2: paredes e portas.
 * @note 3: paredes, portas e pedestres.
 * 
 * @param nome Nome do arquivo de entrada, o qual contém o ambiente.
 * @param modo Modo de carregamento do ambiente.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int carregar_ambiente(char *nome, int modo)
{
    char complete_path[100] = "";
    sprintf(complete_path,"%s%s",path_input,nome);

    FILE *arquivo = fopen(complete_path, "r");
    if(arquivo == NULL)
    {
        fprintf(stderr,"Não foi possível abrir o arquivo %s.\n",nome);
        return 1;
    }

    if( fscanf(arquivo,"%d %d", &num_lin_grid, &num_col_grid) != 2)
    {
        fprintf(stderr, "Dimensões do mapa não encontradas no arquivo.\n");
        return 1;
    }

    if( alocar_grids())
        return 1;

    char lido = '\0';
    fscanf(arquivo,"%c",&lido);// elimina o '\n' depois das dimensões
    for(int i = 0; i < num_lin_grid; i++)
    {
        int h = 0;
        for(; h <= num_col_grid; h++)
        {
            int retorno = fscanf(arquivo,"%c",&lido);
            if(retorno == EOF)
                break;

            if(h == num_col_grid && lido != '\n') // nessa posição espera-se uma quebra de linha.
            {
                fprintf(stderr,"Linha %d possui mais colunas que a dimensão passada.\n", i);
                return 1;
            }

            switch(lido)
            {
                case '#':
                    grid_esqueleto[i][h] = VALOR_PAREDE;
                    break;
                case '_':
                    if(modo != 1 && modo != 3)
                    {
                        if( adicionar_saida_conjunto(i,h))
                            return 1;
                     
                        grid_esqueleto[i][h] = VALOR_PAREDE;
                    }
                    else
                        grid_esqueleto[i][h] = VALOR_PAREDE;
                        // na existência de saídas no meio do ambiente uma parede seria colocada no lugar.

                    break;
                case '.':
                    grid_esqueleto[i][h] = 0;
                    break;
                case 'p':
                case 'P':
                    if(modo == 3 || modo == 4)
                    {
                        if( adicionar_pedestre_conjunto(i,h))
                            return 1;
                        grid_pedestres[i][h] = pedestres.vet[pedestres.num_ped - 1]->id;
                    }
                    else
                        grid_esqueleto[i][h] = 0;

                    break;
                case '\n':
                    goto fora;
                default:
                    fprintf(stderr,"Símbolo desconhecido no desenho do ambiente: %c\n", lido);
                    return 1;
            }
        }

        fora:
        if( h < num_col_grid)
        {
            fprintf(stderr,"Linha %d possui menos colunas que a dimensão passada.\n", i);
            return 1;
        }
    }

    fclose(arquivo);

    return 0;
}

/**
 * Gera um ambiente retangular, com tamanho especificado nas variávies globais num_lin_grid e num_col_grid.
 * As bordas recebem paredes. 
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int gerar_ambiente()
{
    if(grid_esqueleto == NULL)
        return 1;

    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            if(i > 0 && i < num_lin_grid - 1 && h > 0 && h < num_col_grid - 1)
                grid_esqueleto[i][h] = 0;
            else
                grid_esqueleto[i][h] = VALOR_PAREDE;
        }
    }

    return 0;
}

/**
 * Lê uma única linha do arquivo passado, extrai as saídas e as adiciona no ambiente. 
 * 
 * @param arquivo_auxiliar Contém as localizações das portas.
 * @return Inteiro, quantidade de saídas extraídas (sucesso) ou 0 (fracasso).
*/
int extrair_saidas(FILE *arquivo_auxiliar)
{
    int lin = 0;
    int col = 0;

    int qtd = 0;
    char caracter = '\0';

    while(1)
    {
        int retorno = fscanf(arquivo_auxiliar,"%d %d %c ",&lin,&col,&caracter);

        if( retorno == EOF)
            return 0; 

        if( retorno != 3)
        {
            fprintf(stderr, "Falha ao ler o arquivo auxiliar. Verifique se o padrão está sendo seguido.\n");
            return 0;
        }

        qtd++;

        if(caracter == ',')
        {
            if( adicionar_saida_conjunto(lin,col))
                return 0;
        }
        else if(caracter == '.')
        {
            if( adicionar_saida_conjunto(lin,col))
                return 0;
            break;
        }
        else
        {
            fprintf(stderr, "Falha ao ler o arquivo auxiliar. Símbolo desconhecido.\n");
            return 0;
        }
    }

    return qtd;
}

 /**
  * Fecha arquivos abertos e desaloca memória.
  * 
  * @param arquivo_saida
  * @param arquivo_auxiliar
 */
void finalizar_programa(FILE *arquivo_saida, FILE *arquivo_auxiliar)
{
    if(arquivo_auxiliar != NULL)
        fclose(arquivo_auxiliar);

    if(arquivo_saida != NULL && arquivo_saida != stdout)
        fclose(arquivo_saida);

    desalocar_pedestres();
    desalocar_saidas();

    desalocar_matriz_int(grid_esqueleto,num_lin_grid);
    desalocar_matriz_int(grid_pedestres,num_lin_grid);
    desalocar_matriz_int(grid_mapa_calor,num_lin_grid);
}