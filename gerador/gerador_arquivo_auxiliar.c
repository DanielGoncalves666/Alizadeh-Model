#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<argp.h>

const char * argp_program_version = "Gerador de arquivos auxiliares para simulações em massa.";
const char doc[] = "Gerador de arquivos auxiliares para simulações em massa.\n"
"As coordenadas das portas passadas por meio do arquivo INPUT-FILE são combinadas de acordo com o modo especificado pela opção --mode.\n"
"\v"
"O arquivo OUTPUT-FILE armazenará o resultado (será impresso na tela quando nenhum arquivo for informado) seguindo o formato dos arquivos auxiliares descritos no --help de ./alizadeh.sh.\n"
"\n"
"A opção --mode deve receber um inteiro, indicando a organização correspondente dos dados no arquivo de entrada e como os mesmos devem ser combinados:\n"
"\t * 1 (one-by-one): o arquivo de entrada deve conter N linhas, cada uma indicando as coordenadas de uma saída. As coordenadas (linha e coluna) devem ser separadas por espaço.\n" 
"\t\t Cada uma das saídas do arquivo de entrada será combinada com outra, em uma combinação um a um, de modo que cada linha conterá as coordenadas de duas portas, com excessão "
"dos casos onde ocorreriam combinação de uma porta com ela mesma. Nestes casos, a linha terá as coordenadas de uma única porta.\n"
"\t * 2 (expansion): o arquivo de entrada deve conter 2 linhas, cada uma contendo M pares de coordenadas, separados por vírgula.\n"
"\t\t O primeiro par de coordenadas de cada linha indicará a saída original, a qual será gradativamente expandida seguindo a ordem das coordenadas do restante da linha.\n"
"\t\t Cada saída de tamanho T gerada à partir da primeira linha será combinada com todas as saídas expandidas à partir da segunda linha.\n"
"\n"
"Por padrão, --mode é a opção 1.\n";

static struct argp_option options[] = {
    {"\nArquivos:\n",0,0,OPTION_DOC,0,1},
    {"input-file", 'i', "INPUT-FILE", 0, "INPUT-FILE é o nome do arquivo que contém a correspondência entre o número de uma saída e suas coordenadas.",2},
    {"output-file", 'o', "OUTPUT-FILE", OPTION_ARG_OPTIONAL, "Indica se a saída deve ser armazenada em um arquivo, com o nome do arquivo sendo opcionalmente passado."},
    {"mode", 't', "MODE", 0, "Indica o padrão dos dados no arquivo de entrada e como os dados de entrada devem ser combinados para se gerar a saída."},
    {"\nOutros:\n",0,0,OPTION_DOC,0,3},
    {0}
};

error_t parser_function(int key, char *arg, struct argp_state *state);
static struct argp argp = {options,&parser_function, NULL, doc};

typedef struct command_line {
    char nome_arquivo_entrada[151];
    char nome_arquivo_saida[151];
    int mode;
    int output_to_file;
} Command_line;

Command_line commands = {.nome_arquivo_entrada="",
                         .nome_arquivo_saida="saida_gerador.txt",
                         .mode=1,
                         .output_to_file=0};

typedef struct saida{
    int linha;
    int coluna;
}saida;

saida *one_by_one_input_processing(FILE *entrada, int *num_saidas);
void expansion_input_processing(FILE *entrada, saida **saidaA, saida **saidaB, int *tamanhoA, int *tamanhoB);
void one_by_one_output_generation(saida *vetor, int num_saidas, FILE *output_file);
void expansion_output_generation(saida *saidaA, saida *saidaB, int tamanhoA, int tamanhoB, FILE *output_file);

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
    
    switch(commands.mode)
    {
        case 1:
            int num_saidas = 0;
            saida *saidas = one_by_one_input_processing(arq_entrada,&num_saidas);
            one_by_one_output_generation(saidas,num_saidas,arq_saida);

            break;
        case 2:
            saida *saidaA = NULL;
            saida *saidaB = NULL;
            int tam_A = 0, tam_B = 0;

            expansion_input_processing(arq_entrada, &saidaA, &saidaB, &tam_A, &tam_B);
            expansion_output_generation(saidaA, saidaB, tam_A, tam_B, arq_saida);
            break;
        default:
            break;
    }

    
    fclose(arq_entrada);
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
        case 't':
            if(arg != NULL)
                commands->mode = atoi(arg);
            
            if(commands->mode != 1 && commands->mode != 2)
            {
                fprintf(stderr, "invalid mode.\n");
                return EINVAL;
            }
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
 * Carrega as coordenadas das saídas do modo one-by-one.
 * 
 * @param entrada Arquivo de entrada já aberto
 * @param num_saidas Ponteiro para o tipo inteiro, onde será armazenado o número de saídas presentes no arquivo.
 * @return Ponteiro para o tipo saida, indicando um vetor contendo todas as portas do arquivo.
*/
saida *one_by_one_input_processing(FILE *entrada, int *num_saidas)
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
 * Carrega as coordenadas das saídas do modo expansion.
 * 
 * @param entrada Arquivo de entrada já aberto
 * @param saidaA Ponteiro para ponteiro do tipo saida, indicando onde serão armazenadas as coordenadas referentes à primeira saída.
 * @param saidaB Ponteiro para ponteiro do tipo saida, indicando onde serão armazenadas as coordenadas referentes à segunda saída.
 * @param tamanhoA ponteiro para inteiro, indicando onde o tamanho da saída A deve ser armazenado.
 * @param tamanhoB ponteiro para inteiro, indicando onde o tamanho da saída B deve ser armazenado.
*/
void expansion_input_processing(FILE *entrada, saida **saidaA, saida **saidaB, int *tamanhoA, int *tamanhoB)
{
    int linha, coluna;
    char caracter;
    int count = 0;

    saida **atual = saidaA;
    while(fscanf(entrada,"%d %d%c ",&linha,&coluna, &caracter) != EOF)
    {

        *atual = realloc(*atual,sizeof(saida) * (count + 1));
        (*atual)[count].linha = linha;
        (*atual)[count].coluna = coluna;

        count++;
        
        if(caracter == '\n')
        {
            atual = saidaB;
            *tamanhoA = count;
            count = 0;
        }
    }
    *tamanhoB = count;
}

/**
 * Realiza as combinações necessárias das saídas no modo one-by-one e grava os dados no arquivo passado.
 * 
 * @param vetor Ponteiro para um vetor de structs saida, contendo as coordenadas das saidas
 * @param num_saidas Inteiro indicando o comprimento de VETOR
 * @param outuput_file Arquivo onde deve ser gravado os dados
*/
void one_by_one_output_generation(saida *vetor, int num_saidas, FILE *output_file)
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

    free(vetor);
}

/**
 * Realiza as combinações necessárias das saídas no modo expansion e grava os dados no arquivo passado.
 * 
 * @param saidaA Ponteiro para o tipo saida, indicando onde estão armazenadas as coordenadas referentes à primeira saída.
 * @param saidaB Ponteiro para o tipo saida, indicando onde estão armazenadas as coordenadas referentes à segunda saída.
 * @param tamanhoA inteiro, indicando o tamanho máximo da saída A.
 * @param tamanhoB inteiro, indicando o tamanho máximo da saída B.
 * @param outuput_file Arquivo onde deve ser gravado os dados
*/
void expansion_output_generation(saida *saidaA, saida *saidaB, int tamanhoA, int tamanhoB, FILE *output_file)
{
    char aux; // indica o próximo caractere a ser escrito depois de um par de coordenadas

    for(int i = 0; i < tamanhoA; i++)
    {
        for(int h = 0; h < tamanhoB; h++)
        {
            // escreve a primeira saída
            aux = '+'; // agregador de células para uma mesma saída
            for(int j = 0; j <= i; j++)
            {
                if(j == i)
                    aux = ','; // fim da primeira saída (separador de saídas)

                fprintf(output_file, "%d %d%c ", saidaA[j].linha, saidaA[j].coluna, aux);
            }

            // escreve a segunda saída
            aux = '+';
            for(int j = 0; j <= h; j++)
            {
                if(j == h)
                    aux = '.'; // fim da segunda saída e fim do conjunto

                fprintf(output_file, "%d %d%c ", saidaB[j].linha, saidaB[j].coluna, aux);
            }

            fprintf(output_file, "\n");
        }
    }

    free(saidaA);
    free(saidaB);
}