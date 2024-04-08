#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<argp.h>

const char * argp_program_version = "Gerador de arquivos auxiliares para simulações em massa.";
const char doc[] = "Gerador de arquivos auxiliares para simulações em massa.\n"
"As coordenadas das portas passadas por meio do arquivo INPUT-FILE são combinadas duas a duas usando a sintaxe de arquivos auxiliares.\n"
"\v"
"O arquivo INPUT-FILE deve seguir a seguinte sintaxe, considerando N saídas, cada uma em uma linha:\n"
"\t 1 - Cada uma das N linhas conterá dois números, separados por espaço:\n"
"\t\t 1.1 - O primeiro será a linha da porta.\n"
"\t\t 1.2 - O segundo será a coluna da porta.\n"
"\n"
"O arquivo OUTPUT-FILE armazenará o resultado (será impresso na tela quando nenhum arquivo for informado) seguindo o formato dos arquivos auxiliares descritos no --help de ./alizadeh.sh.\n"
"Todas as linhas do arquivo conterão as coordenadas de duas portas, com a excessão dos casos onde ocorreria combinação de uma porta com ela mesma. Nestes casos, a linha terá as coordenadas de uma única porta.\n";

static struct argp_option options[] = {
    {"\nArquivos:\n",0,0,OPTION_DOC,0,1},
    {"input-file", 'i', "INPUT-FILE", 0, "INPUT-FILE é o nome do arquivo que contém a correspondência entre o número de uma saída e suas coordenadas.",2},
    {"output-file", 'o', "OUTPUT-FILE", OPTION_ARG_OPTIONAL, "Indica se a saída deve ser armazenada em um arquivo, com o nome do arquivo sendo opcionalmente passado."},
    {"\nOutros:\n",0,0,OPTION_DOC,0,3},
    {0}
};

error_t parser_function(int key, char *arg, struct argp_state *state);
static struct argp argp = {options,&parser_function, NULL, doc};

typedef struct command_line {
    char nome_arquivo_entrada[51];
    char nome_arquivo_saida[51];
    int output_to_file;
} Command_line;

Command_line commands = {.nome_arquivo_entrada="",
                         .nome_arquivo_saida="saida_gerador.txt",
                         .output_to_file=0};

typedef struct saida{
    int linha;
    int coluna;
}saida;

saida *processar_entrada(FILE *entrada, int *num_saidas);
void gerar_saida(saida *vetor, int num_saidas, FILE *output_file);

int main(int argc, char **argv)
{
    if( argp_parse(&argp, argc, argv,0,0,&commands) != 0)
        return 0;

    FILE *arq_entrada = fopen(commands.nome_arquivo_entrada, "r");
    if(arq_entrada == NULL)
    {
        fprintf(stderr, "Falha na abertura do arquivo de entrada.\n");
        return 0;
    }

    FILE *arq_saida = fopen(commands.nome_arquivo_saida,"w");
    if(arq_saida == NULL)
    {
        fclose(arq_entrada);
        fprintf(stderr, "Falha na abertura (criação) do arquivo de saída.\n");
        return 0;
    }
    int num_saidas = 0;
    saida *saidas = processar_entrada(arq_entrada,&num_saidas);
    fclose(arq_entrada);

    gerar_saida(saidas,num_saidas,arq_saida);
    fclose(arq_saida);

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
        case 'i':
            strcpy(commands->nome_arquivo_entrada, arg);
            break;
        case ARGP_KEY_ARG:
            fprintf(stderr, "Nenhum argumento não-opcional é esperado.\n");
            return EINVAL;
            break;
        // case ARGP_KEY_END:
        //     break;
        default:
            return ARGP_ERR_UNKNOWN;
            break;
    }   

    return 0;
}

/**
 * Carrega as coordenadas das saídas.
 * 
 * @param entrada Arquivo de entrada já aberto
 * @param num_saidas Ponteiro para o tipo inteiro, onde será armazenado o número de saídas presentes no arquivo.
 * @return Ponteiro para o tipo saida, indicando um vetor contendo todas as portas do arquivo.
*/
saida *processar_entrada(FILE *entrada, int *num_saidas)
{
    int linha, coluna;
    int count = 0;

    saida *vetor = NULL;

    while(fscanf(entrada,"%d %d ",&linha,&coluna) != EOF)
    {

        vetor = realloc(vetor,sizeof(saida) * (count + 1));
        vetor[count].linha = linha;
        vetor[count].coluna = coluna;

        count++;
    }

    *num_saidas = count;
    return vetor;
}

/**
 * Realiza as combinações necessárias das saídas e grava os dados no arquivo passado.
 * 
 * @param vetor Ponteiro para um vetor de structs saida, contendo as coordenadas das saidas
 * @param num_saidas Inteiro indicando o comprimento de VETOR
 * @param outuput_file Arquivo onde deve ser gravado os dados
*/
void gerar_saida(saida *vetor, int num_saidas, FILE *output_file)
{
    for(int i = 0; i < num_saidas; i++)
    {
        for(int h = 0; h < num_saidas; h++)
        {
            if(i == h)
                fprintf(output_file,"%d %d.\n", vetor[i].linha, vetor[i].coluna);
            else
                fprintf(output_file,"%d %d, %d %d.\n", vetor[i].linha, vetor[i].coluna,
                                                        vetor[h].linha, vetor[h].coluna);
        }
    }
}