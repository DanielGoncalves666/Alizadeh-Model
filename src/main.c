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
#include<math.h>

#include"../headers/global_declarations.h"
#include"../headers/inicializacao.h"
#include"../headers/saida.h"
#include"../headers/pedestre.h"
#include"../headers/impressao.h"

const char * argp_program_version = "Modelo Alizadeh, sem movimentação em X ou através de obstáculos.";

const char doc[] = "Alizadeh - Simula uma evacuação de pedestres por meio do modelo de (Alizadeh,2011), com pequenas alterações."
"\v"
"O arquivo passado por --auxiliary-file deve conter, em cada uma de suas linhas, as localizações das saídas para um único conjunto de simulações.\n"
"\n"
"--output-type indica quais e como os dados gerados pela simulação devem ser enviados para a saída. As seguintes opções são possíveis:\n"
"\t 1 - Impressão visual do ambiente.\n"
"\t 2 - Quantidade de passos de tempo até o fim das simulações.\n"
"\t 3 - Impressão de mapas de calor das células do ambiente.\n"
"\t 4 - Variação da distribuição de pedestres para duas saídas, no início da simulação.\n"
"A opção 1 é a padrão.\n"
"A opção 4 gera números entre 0 e 1, onde valores próximos à zero indicam uam distribuição uniforme entre as duas saídas."
"Já valores próximos a 1 indicam uma distribuição de pedestres desigual.\n"
"\n"
"--input-method indica como o ambiente informado em --input-file deve ser carregado ou se o ambiente deve ser gerado.\n"
"\tAmbiente carregado de um arquivo:\n"
"\t\t1 - Apenas a estrutura do ambiente (portas substituídas por paredes).\n"
"\t\t2 - Estrutura e portas.\n"
"\t\t3 - Estrutura e pedestres.\n"
"\t\t4 - Estrutura, portas e pedestres.\n"
"\tAmbiente criado automaticamente:\n"
"\t\t5 - Ambiente será criado considerando quantidade de linhas e colunas passadas pelas opções --lin e --col.\n"
"Opções que não carregam portas do arquivo de entrada devem recebê-las via --auxiliary-file.\n"
"O método 4 é o padrão.\n"
"Para os métodos 1,3 e 5, --auxiliary-file é obrigatório.\n"
"Para o método 5, --lin e --col são obrigatórios.\n"
"\n"
"As variáveis de simulação não são obrigatórias.\n"
"--alfa tem valor padrão de 0.\n"
"--input-file tem valor padrão de \"sala_padrao.txt\".\n"
"--simu e --ped tem valor padrão de 1.\n"
"--seed tem valor padrão de 0.\n\n"
"Toggle Options são opções que podem ser ativadas e também não são obrigatórias.\n"
"--na-sala quando ativado obriga os pedestres a ficarem um passo de tempo na saída do ambiente antes de serem removidos.\n"
"--sempre-menor quando ativado obriga os pedestres a só se moverem para a menor célula de sua vizinhança. Se esta estiver ocupada, o pedestre irá esperar ela ser desocupada.\n"
"--evitar-mov-cantos quando ativado impede que pedestres se movimentem através dos cantos de paredes/obstáculos. Um único movimento se torna necessariamente em 3 movimentos.\n"
"--permitir-mov-x quando ativado permite que os pedestres ignorem a restrição que impede movimentações em X.\n"
"--debug ativa mensagens de debug.\n"
"--status ativa mensagens que indicam o progessão de simulações.\n"
"--detalhes inclui um cabeçalho contendo as correspondentes saídas de cada conjunto de simulação.\n"
"\n"
"Opções desnecessárias para determinados --input-method são ignoradas.\n";

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
    {"alfa", 1002, "ALFA", 0, "Coeficiente de evitação de multidões", 8},
    {"ped", 'p', "PEDESTRES", 0, "Número de pedestres a serem inseridos no ambiente de forma aleatória."},
    {"simu", 's', "SIMULACOES", 0, "Número de simulações a serem realizadas por conjunto de saídas."},
    {"seed", 'e', "SEED", 0, "Semente inicial para geração de números pseudo-aleatórios."},

    {"\nToggle Options:\n",0,0,OPTION_DOC,0,9},
    {"debug", 'd',0,0, "Indica se mensagens de debug devem ser impressas na saída padrão.",10},
    {"status", 1004, 0,0, "Indica se mensagens de status durante a execução de simulações devem ser impressas em stdout."},
    {"detalhes", 1005, 0,0, "Indica se o output deve conter informações sobre as saídas."},
    {"na-saida", 1000,0,0, "Indica que o pedestre deve permanecer por um passo de tempo quando chega na saída (invés de ser retirado imediatamente)."},
    {"sempre-menor", 1001, 0,0, "Indica que a movimentação dos pedestres é sempre para a menor célula, com o pedestre ficando parado se ela estiver ocupada."},
    {"evitar-mov-cantos",1003,0,0, "Indica que a movimentação através de cantos de paredes/obstáculos deve ser impedida."},
    {"permitir-mov-x",1006,0,0, "Permite que os pedestres se movimentem em X."},

    {"\nOutros:\n",0,0,OPTION_DOC,0,11},
    {0}
};

void obter_comando_completo(char *comando_completo, int key, char *arg);
double determinar_delta();
error_t parser_function(int key, char *arg, struct argp_state *state);

static struct argp argp = {options,&parser_function, NULL, doc};

int main(int argc, char **argv)
{
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

    if( alocar_combined_fields())
        return 0;

    imprimir_comando(commands, arquivo_saida);

    int num_conjunto_simulacoes = -1;
    if(commands.status && arquivo_auxiliar)
    {
        num_conjunto_simulacoes = extrair_numero_linhas(arquivo_auxiliar);
        if(num_conjunto_simulacoes == -1)
            return 0;
    }

    int conjunto_index = 0;
    do
    {
        if(commands.input_method == 1 || commands.input_method == 3 || commands.input_method == 5)
        {
            if( !extrair_saidas(arquivo_auxiliar))
                break; // caso ocorra algum erro, ou se todos os conjuntos de saídas já tiverem sido processados.
        }

        if(commands.detalhes)
            imprimir_cabecalho(arquivo_saida);

        int retorno = calcular_pisos_estaticos();
        if( retorno == 1) return 0;
        else if(retorno == -1)
        {
            if(commands.output_type != 2 && commands.output_type != 4)
                fprintf(arquivo_saida, "Pelo menos uma das saídas do conjunto não é acessível.\n");
            else
            {
                // imprime um placeholder quando existe uma saída inválida no conjunto
                for(int vezes = 0; vezes < numero_simulacoes; vezes++)
                {
                    fprintf(arquivo_saida,"-1 ");
                }
                fprintf(arquivo_saida, "\n");
            }

            if(commands.input_method == 1 || commands.input_method == 3 || commands.input_method == 5)
                desalocar_saidas();

            if(commands.status)
            {
                imprimir_status(conjunto_index, num_conjunto_simulacoes);
                conjunto_index++;
            }

            continue;
        }

        if((commands.output_type == 2 || commands.output_type == 4) && saidas.num_saidas == 1)
            fprintf(arquivo_saida,"#1 "); // Serve para indicar casos onde a saída foi combinada com ela mesma no exemplo 1 do artigo do Alizadeh.
                                          // Esses casos terão os maiores valores e deverão ser tratados de forma específica.

        for(int i = 0; i < numero_simulacoes; i++, seed++)
        {
            srand(seed);

            //debug
            if(commands.debug)
            {
                printf("Piso estático.\n");
                imprimir_piso(saidas.static_combined_field);
            }

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
                if(commands.output_type == 1)
                    fprintf(arquivo_saida,"Passo %d\n",num_passos_tempo);

                //debug
                if(commands.debug)
                    printf("\nPiso Dinâmico - Passo %d.\n", num_passos_tempo);

                if(calcular_piso_geral())
                    return 0;

                //debug
                if(commands.debug)
                    imprimir_piso(saidas.dynamic_combined_field);    

                if(commands.output_type == 4)
                    break; // é necessário apenas o cálculo dos campos de piso estático e dinâmico do primeiro passo de tempo

                determinar_movimento();
                panico();

                if(!commands.permitir_mov_x)
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

            if(commands.output_type == 2) // passos de tempo
                fprintf(arquivo_saida,"%d ", num_passos_tempo);

            if(commands.output_type == 4)
            {
                double delta = 1;
                if(saidas.num_saidas == 2) // caso o número de saídas não seja 2, sempre imprime 1
                    delta = determinar_delta();

                fprintf(arquivo_saida,"%.3lf ", delta);
            }
        }

        if(commands.input_method == 1 || commands.input_method == 3 || commands.input_method == 5)
            desalocar_saidas();

        if(commands.output_type == 2 || commands.output_type == 4)
            fprintf(arquivo_saida, "\n");

        if(commands.output_type == 3) // mapa de calor
            imprimir_mapa_calor(arquivo_saida);        

        // esses modos apresentam apenas um conjunto de saídas (as saídas do próprio ambiente)
        if(commands.input_method == 2 || commands.input_method == 4)
            break;

        if(commands.status)
        {
            imprimir_status(conjunto_index, num_conjunto_simulacoes);
            conjunto_index++;
        }

    }while(1);

    finalizar_programa(arquivo_saida, arquivo_auxiliar);

    return 0;
}

/**
 * Determina o valor de delta (estático ou do primeiro passo de tempo). 
 * 
 * @note O delta é um valor entre 0 e 1 (inclusos) que aponta a distribuição dos pedestres entre as duas saídas existentes. Valores próximos a zero apontam 
 * um equilíbrio na distribuição entre as duas saídas, enquanto que valores próximos a 1 indicam uma preferência maior dos pedestres em relação a uma única saída.
 * 
 * @return double, indicando o delta obtido.
*/
double determinar_delta()
{
    double **saidaA = commands.alfa == 0 ? saidas.vet_saidas[0]->estatico : saidas.vet_saidas[0]->field;
    double **saidaB = commands.alfa == 0 ? saidas.vet_saidas[1]->estatico : saidas.vet_saidas[1]->field;

    int Na = 0; // número de células onde o campo de piso da saídaB é menor ou igual ao da saídaA
    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            if(saidaA[i][h] == VALOR_PAREDE || saidaA[i][h] == VALOR_SAIDA || saidaB[i][h] == VALOR_SAIDA)
                continue;

            if(grid_pedestres[i][h] != 0 && saidaB[i][h] <= saidaA[i][h] + TOLERANCIA)
                Na++;
        }
    }

    double delta = 1.0 - (fmin(Na, pedestres.num_ped - Na) / fmax(Na, pedestres.num_ped - Na));

    return delta;
}

error_t parser_function(int key, char *arg, struct argp_state *state)
{
    struct command_line *commands = state->input;

    obter_comando_completo(commands->comando_completo, key, arg);

    switch(key)
    {
        case 'o':
            if(arg != NULL)
                strcpy(commands->nome_arquivo_saida, arg);

            commands->output_to_file = 1;
            break;
        case 'O':
            commands->output_type = atoi(arg);
            if(commands->output_type <= 0 || commands->output_type > 4)
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
            seed = atoi(arg);
            if(seed < 0)
            {
                fprintf(stderr, "Seed inválida.\n");
                return EIO;
            }
            break;
        case 'd':
            commands->debug = 1;
            break;
        case 1000:
            commands->na_saida = 1;
            break;
        case 1001:
            commands->sempre_menor = 1;
            break;
        case 1002:
            commands->alfa = atof(arg); 
            break;
        case 1003:
            commands->evitar_mov_cantos = 1;
            break;
        case 1004:
            commands->status = 1;
            break;
        case 1005:
            commands->detalhes = 1;
            break;
        case 1006:
            commands->permitir_mov_x = 1;
            break;
        case ARGP_KEY_ARG:
            fprintf(stderr, "Nenhum argumento não-opcional é esperado, mas %s foi inserido.\n", arg);
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

            if(commands->output_type == 4)
                numero_simulacoes = 1;

            break;
        default:
            return ARGP_ERR_UNKNOWN;
            break;
    }

    return 0;
}

/**
 * Realiza o armazenamento das partes do comando entrado via terminal em uma string. 
 * 
 * @param comando_completo String onde as partes do comando serão armazenadas.
 * @param key Inteiro que indica qual opção está sendo recebida.
 * @param arg String, contendo o argumento da opção recebida, se existir.
*/
void obter_comando_completo(char *comando_completo, int key, char *arg)
{
    char aux[100];

    switch(key)
    {
        case 1000:
            sprintf(aux, " --na-saida");
            break;
        case 1001:
            sprintf(aux, " --sempre-menor");
            break;
        case 1002:
            sprintf(aux, " --alfa=%s",arg);
            break;
        case 1003:
            sprintf(aux, " --evitar-mov-cantos");
            break;
        case 1004:
            sprintf(aux, " --status");
            break;
        case 1005:
            sprintf(aux, " --detalhes");
            break;
        case 1006:
            sprintf(aux, " --permitir-mov-x");
            break;
        case 'd':
            sprintf(aux, " --debug");
            break;
        case 'o':
        case 'O':
        case 'i':
        case 'm':
        case 'a':
        case 'l':
        case 'c':
        case 'p':
        case 's':
        case 'e':
            if(arg == NULL)
                sprintf(aux, " -%c",key);
            else
                sprintf(aux, " -%c%s",key, arg);

            break;
        default:
            return;
    }

    strcat(comando_completo, aux);
}