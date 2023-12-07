/* 
   File: main.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém a função main do projeto, além das estruturas e funções necessárias para o uso do argp para
                tratamento de argumentos via linha de comando.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<argp.h>
#include<unistd.h>

#include"../headers/global_declarations.h"
#include"../headers/inicializacao.h"
#include"../headers/saida.h"
#include"../headers/pedestre.h"
#include"../headers/impressao.h"

const char * argp_program_version = "Varas Original, sem movimentação em X ou através de obstáculos.";

const char doc[] = "Varas - Simula uma evacuação de pedestres por meio do modelo de (Varas,2007)."
"\v"
"O arquivo passado por --auxiliary-file deve conter, em cada uma de suas linhas, as localizações das saídas para um único conjunto de simulações.\n"
"Um número indeterminado de portas é aceito para um dado conjunto de simulações, com o conteúdo do arquivo devendo seguir o seguinte padrão:\n"
"LIN_PORTA_1 COL_PORTA_1, LIN_PORTA_2 COL_PORTA_2 ...\n"
"\tDepois do último par deve vir um ponto final.\n"
"OBS: Saídas repetidas são aceitas.\n"
"\n"
"--output-type indica quais e como os dados gerados pela simulação devem ser enviados para a saída. As seguintes opções são possíveis:\n"
"\t 1 - Impressão visual do ambiente.\n"
"\t 2 - Quantidade de passos de tempo até o fim das simulações.\n"
"\t 3 - Impressão de mapas de calor das células do ambiente.\n"
"A opção 1 é a padrão.\n"
"\n"
"--input-method indica como o ambiente informado em --input-file deve ser carregado ou se o ambiente deve ser gerado.\n"
"\tAmbiente carregado de um arquivo:\n"
"\t\t1 - Apenas a estrutura do ambiente (portas substituídas por paredes).\n"
"\t\t2 - Estrutura e portas.\n"
"\t\t3 - Estrutura e pedestres."
"\t\t4 - Estrutura, portas e pedestres.\n"
"\tAmbiente criado automaticamente:\n"
"\t\t5 - Ambiente será criado considerando quantidade de linhas e colunas passadas pelas opções --lin e --col,"
"Opções que não carregam portas do arquivo de entrada devem recebê-las via --auxiliary-file.\n"
"O método 4 é o padrão.\n"
"Para os métodos 1,3 e 5, --auxiliary-file é obrigatório.\n"
"Para o método 5, --lin e --col são obrigatórios.\n"
"\n"
"O restante das opções (--simu, --ped, --seed), são sempre opcionais.\n"
"--input-file tem valor padrão de \"sala_padrao.txt\".\n"
"--simu e --ped tem valor padrão de 1.\n"
"--seed tem valor padrão de 0.\n"
"\n"
"Opções desnecessárias para determinados modos são ignoradas.\n";

static struct argp_option options[] = {
    {"\nArquivos:\n",0,0,OPTION_DOC,0,1},
    {"input-file", 'i', "INPUT-FILE", 0, "INPUT-FILE é o nome do arquivo que contém o ambiente pre-definido a ser carregado.",2},
    {"output-file", 'o', "OUTPUT-FILE", OPTION_ARG_OPTIONAL, "Indica se a saída deve ser armazenada em um arquivo, com o nome do arquivo sendo opcionalmente passado."},
    {"auxiliary-file", 'a', "ARQUIVO_AUXILIAR",0, "Contém informações auxiliares para a realização das simulações. Ex: localização das saídas."},

    {"\nModos de operação:\n",0,0,OPTION_DOC,0,3},    
    {"input-method", 'm', "METHOD",0, "Indica como tratar INPUT_FILE.",4},
    {"output-type", 'O', "TYPE", 0, "Indica o tipo de saída que deve ser gerada pelas simulações."},
    
    {"\nDimensões do ambiente:\n",0,0,OPTION_DOC,0,5},
    {"lin", 'l', "LINHAS", 0, "LINHAS indica a quantidade de linhas que o ambiente criado deve ter.",6},
    {"col", 'c', "COLUNAS", 0, "COLUNAS indica a quantidade de colunas que o ambiente criado deve ter."},

    {"\nVariáveis de simulação:\n",0,0,OPTION_DOC,0,7},
    {"ped", 'p', "PEDESTRES", 0, "Número de pedestres a serem inseridos no ambiente de forma aleatória.",8},
    {"simu", 's', "SIMULACOES", 0, "Número de simulações a serem realizadas por conjunto de saídas."},
    {"seed", 'e', "SEED", 0, "Semente inicial para geração de números pseudo-aleatórios."},
    {"debug", 'd',0,0, "Indica se mensagens de debug devem ser impressas na saída padrão."},
    {"\nOutros:\n",0,0,OPTION_DOC,0,9},
    {0}
};


error_t parser_function(int key, char *arg, struct argp_state *state);

static struct argp argp = {options,&parser_function, NULL, doc};

int main(int argc, char **argv){

    if( argp_parse(&argp, argc, argv,0,0,&commands) != 0)
        return 0;

    FILE *arquivo_auxiliar = NULL;
    if(abrir_arquivo_auxiliar(commands, &arquivo_auxiliar))
        return 0;
    
    FILE *arquivo_saida = NULL;
    if(abrir_arquivo_output(commands, &arquivo_saida))
    {
        if(arquivo_auxiliar != NULL)
            fclose(arquivo_auxiliar);
        return 0;
    }

    if(commands.input_method != 5)
    {
        if( carregar_ambiente(commands.nome_arquivo_entrada, commands.input_method))
            return 0;
    }
    else
    {
        if( alocar_grids())
            return 0;

        if( gerar_ambiente())
            return 0;
    }

    imprimir_informacoes_gerais(commands, arquivo_saida);

    do
    {
        if(commands.input_method == 1 || commands.input_method == 3 || commands.input_method == 5)
        {
            if( !extrair_saidas(arquivo_auxiliar))
                break; // caso ocorra algum erro, ou se todos os conjuntos de saídas já tiverem sido processados.
        }

        // fprintf(arquivo_saida, "Conjunto de saídas: %d %d", saidas.vet_saidas[0]->loc_lin, saidas.vet_saidas[0]->loc_col);
        // for(int s = 1; s < saidas.num_saidas; s++)
        //     fprintf(arquivo_saida, ", %d %d", saidas.vet_saidas[s]->loc_lin, saidas.vet_saidas[s]->loc_col);
        // fprintf(arquivo_saida, ".\n");

        int seed = original_seed;
        for(int i = 0; i < numero_simulacoes; i++, seed++)
        {
            srand(seed);
            if( determinar_piso_geral())
                return 0;

            if(commands.debug)
                imprimir_piso(saidas.combined_field);

            if(commands.input_method != 3 && commands.input_method != 4)
            {
                if( inserir_pedestres_aleatoriamente(numero_pedestres))
                    return 0;
            }
            
            if(commands.output_type == 1)
            {
                fprintf(arquivo_saida,"Simulação %d\n",i);
                if(!commands.output_to_file)
                    printf("\e[1;1H\e[2J");
                imprimir_grid_pedestres(arquivo_saida);
            }

            int num_passos_tempo = 0;
            while(ambiente_vazio())
            {
                if(commands.debug)
                    printf("\nPasso %d.\n", num_passos_tempo);

                determinar_movimento();
                panico();
                varredura_movimento_em_x();
                resolver_conflitos_movimento();
                confirmar_movimentacao();
                atualizar_grid_pedestres();
                resetar_estado_pedestres();
                
                if(commands.output_type == 1)
                {
                    if(!commands.output_to_file)
                    {
                        sleep(1);
                        printf("\e[1;1H\e[2J");
                    }
                    imprimir_grid_pedestres(arquivo_saida);
                }

                num_passos_tempo++;
            }

            if(commands.input_method == 3 || commands.input_method == 4)
                reiniciar_pedestre();
            else
                desalocar_pedestres();

            if(commands.output_type == 2) // passos e tempo
                fprintf(arquivo_saida,"%d ", num_passos_tempo);
        }

        if(commands.input_method == 1 || commands.input_method == 3 || commands.input_method == 5)
            desalocar_saidas();

        if(commands.output_type == 2)
            fprintf(arquivo_saida, "\n");

        if(commands.output_type == 3) // mapa de calor
            imprimir_mapa_calor(arquivo_saida);        

        // esses modos apresentam apenas um conjunto de saídas (as saídas do próprio ambiente)
        if(commands.input_method == 2 || commands.input_method == 4)
            break;

    }while(1);

    finalizar_programa(arquivo_saida, arquivo_auxiliar);

    return 0;
}

error_t parser_function(int key, char *arg, struct argp_state *state)
{
    struct command_line *commands = state->input;

    switch(key)
    {
        case 'o':
            if(arg != NULL)
                strcpy(commands->nome_arquivo_saida, arg);

            commands->output_to_file = 1;
            break;
        case 'O':
            commands->output_type = atoi(arg);
            if(commands->output_type <= 0 || commands->output_type > 3)
            {
                fprintf(stderr, "Nenhuma forma de saída corresponde ao output-type passado.\n");
                return EIO;
            }

            break;
        case 'i':
            strcpy(commands->nome_arquivo_entrada, arg);
            break;
        case 'm':
            commands->input_method = atoi(arg);
            if(commands->input_method <= 0 || commands->input_method > 5)
            {
                fprintf(stderr, "Nenhum tratamento corresponde ao input_method passado.\n");
                return EIO;
            }

            break;
        case 'a':
            strcpy(commands->nome_arquivo_auxiliar, arg);
            break;
        case 'l':
            num_lin_grid = atoi(arg);
            if(num_lin_grid <= 0)
            {
                fprintf(stderr,"Número de linhas inválido.\n");
                return EIO;
            }
            break;
        case 'c':
            num_col_grid = atoi(arg);
            if(num_col_grid <= 0)
            {
                fprintf(stderr,"Número de colunas inválido.\n");
                return EIO;
            }
            break;
        case 'p':
            numero_pedestres = atoi(arg);
            if(numero_pedestres <= 0)
            {
                fprintf(stderr, "Número de pedestres inválido.\n");
                return EIO;
            }
            break;
        case 's':
            numero_simulacoes = atoi(arg);
            if(numero_simulacoes <= 0)
            {
                fprintf(stderr, "Número de simulações inválido.\n");
                return EIO;
            }
            break;
        case 'e':
            original_seed = atoi(arg);
            if(original_seed < 0)
            {
                fprintf(stderr, "Seed inválida.\n");
                return EIO;
            }
            break;
        case 'd':
            commands->debug = 1;
            break;
        case ARGP_KEY_ARG:
            fprintf(stderr, "Nenhum argumento não-opcional é esperado.\n");
            return EINVAL;
            break;
        case ARGP_KEY_END:
            if( commands->input_method == 1 || commands->input_method == 3 || commands->input_method == 5) // sem portas
            {
                if( strcmp(commands->nome_arquivo_auxiliar,"") == 0)
                {
                    fprintf(stderr, "Arquivo auxiliar com a localização das saídas é esperado.\n");
                    return EIO;
                }
            }
            else
            {
                if( strcmp(commands->nome_arquivo_auxiliar,"") != 0)
                    strcpy(commands->nome_arquivo_auxiliar,"");
            }

            if(commands->input_method == 5)
            {
                if(num_lin_grid == 0 || num_col_grid == 0)
                {
                    fprintf(stderr, "Dimensões do ambiente não foram informadas.\n");
                    return EIO;
                }
            }

            break;
        default:
            return ARGP_ERR_UNKNOWN;
            break;
    }

    return 0;
}