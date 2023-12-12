/* 
   File: saida.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém funções dedicadas à criação e administração de saídas, além do cálculo do campo de piso.
*/

#include<stdio.h>
#include<stdlib.h>
#include"../headers/global_declarations.h"
#include"../headers/saida.h"
#include"../headers/celula.h"
#include"../headers/impressao.h"

const double regra[3][3] = {{VALOR_DIAGONAL, 1.0, VALOR_DIAGONAL},
                            {     1.0,       0.0,       1.0     },
                            {VALOR_DIAGONAL, 1.0, VALOR_DIAGONAL}};
/* Considerando que a célula central tenha valor X, 'regra' indica o valor a ser somado para
   determinar o valor dos vizinhos. */

Saida criar_saida(int loc_linha, int loc_coluna);
int determinar_piso_estatico(Saida s);
int determinar_piso_dinamico(Saida s);
int determinar_piso_final(Saida s, double alfa);
int combinar_pisos(double ***piso_combinado, int tipo);

/**
 * Cria uma estrutura Saida correspondente à localização passada (se válida) e a inicializa
 * 
 * @param loc_linha Linha da saída
 * @param loc_coluna Coluna da saída
 * @return Estrutura Saida ou NULL (fracasso).
*/
Saida criar_saida(int loc_linha, int loc_coluna)
{
    if(loc_linha < 0 || loc_linha >= num_lin_grid || loc_coluna < 0 || loc_coluna >= num_col_grid)
        return NULL;
    
    Saida nova = malloc(sizeof(struct saida));
    if(nova != NULL)
    {
        nova->loc_lin = loc_linha;
        nova->loc_col = loc_coluna;

        nova->estatico = alocar_matriz_double(num_lin_grid, num_col_grid);
        nova->dinamico = alocar_matriz_double(num_lin_grid, num_col_grid);
        nova->field = alocar_matriz_double(num_lin_grid, num_col_grid);
    }

    return nova;
}

/**
 * Adiciona uma nova saida no conjunto de saidas.
 * 
 * @param loc_linha Linha da saída.
 * @param loc_coluna Coluna da saída.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int adicionar_saida_conjunto(int loc_linha, int loc_coluna)
{
    Saida nova = criar_saida(loc_linha, loc_coluna);
    if(nova == NULL)
    {
        fprintf(stderr,"Falha na criação da Saida em (%d,%d).\n",loc_linha, loc_coluna);
        return 1;
    }

    saidas.num_saidas += 1;
    saidas.vet_saidas = realloc(saidas.vet_saidas, sizeof(Saida) * saidas.num_saidas);
    if(saidas.vet_saidas == NULL)
    {
        fprintf(stderr, "Falha na realocação do vetor de saídas.\n");
        return 1;
    }    

    saidas.vet_saidas[saidas.num_saidas - 1] = nova;

    return 0;
}

/**
 * Desaloca as estruturas para as saídas individuais, a matriz da saída combinada e zera a quantidade de saídas.
*/
void desalocar_saidas()
{
    for(int s = 0; s < saidas.num_saidas; s++)
    {
        free(saidas.vet_saidas[s]->estatico);
        free(saidas.vet_saidas[s]->dinamico);
        free(saidas.vet_saidas[s]->field);
    }

    free(saidas.vet_saidas);
    saidas.vet_saidas = NULL;

    desalocar_matriz_double(saidas.static_combined_field, num_lin_grid);
    desalocar_matriz_double(saidas.dynamic_combined_field,num_lin_grid);
    saidas.static_combined_field = NULL;
    saidas.dynamic_combined_field = NULL;

    saidas.num_saidas = 0;
}

/**
 * Calcula o campo de piso estático referente à saida dada como argumento
 * 
 * @param s Saida cujo piso será calculado.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int determinar_piso_estatico(Saida s)
{
    if(s == NULL)
    {
        fprintf(stderr, "NULL como entrada em 'determinar_piso_estatico'.\n");
        return 1;
    }

    double **mat = s->estatico;

    // adiciona as paredes no piso da saida (outras saidas também são consideradas como paredes)
    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            double conteudo = grid_esqueleto[i][h];
            if(conteudo == VALOR_PAREDE)
                mat[i][h] = VALOR_PAREDE;
            else
                mat[i][h] = 0.0;
        }
    }
    mat[s->loc_lin][s->loc_col] = VALOR_SAIDA; // Adiciona a saida

    double **aux = alocar_matriz_double(num_lin_grid,num_col_grid);
    // matriz para armazenar as alterações para o tempo t + 1 do autômato
    if(aux == NULL)
    {
        fprintf(stderr,"Falha na alocação da matriz auxiliar para cálculo do piso.\n");
        return 1;
    }

    copiar_matriz_double(aux, mat); // copia o piso base para a matriz auxiliar

    int qtd_mudancas;
    do
    {
        qtd_mudancas = 0;
        for(int i = 0; i < num_lin_grid; i++)
        {
            for(int h = 0; h < num_col_grid; h++)
            {
                double atual = mat[i][h]; // valor atual de piso na posição dada

                if(atual == VALOR_PAREDE || atual == 0.0) 
                    continue; // cálculo de piso dos vizinhos ocorre apenas para células com valor de piso já dado

                for(int j = -1; j < 2; j++)
                {
                    if(i + j < 0 || i + j >= num_lin_grid)
                        continue;

                    for(int k = -1; k < 2; k++)
                    {
                        if(h + k < 0 || h + k >= num_col_grid)
                            continue;

                        if(mat[i + j][h + k] == VALOR_PAREDE || mat[i + j][h + k] == VALOR_SAIDA)
                            continue;

                        if(j != 0 && k != 0)
                        {
                            if(! eh_diagonal_valida(i,h,j,k,mat))
                                continue;
                        }

                        double novo_valor = mat[i][h] + regra[1 + j][1 + k];
                        if(aux[i + j][h + k] == 0.0)
                        {    
                            aux[i + j][h + k] = novo_valor;
                            qtd_mudancas++;
                        }
                        else if(novo_valor < aux[i + j][h + k])
                        {
                            aux[i + j][h + k] = novo_valor;
                            qtd_mudancas++;
                        }
                    }
                }
            }
        }
        copiar_matriz_double(mat,aux);
    }
    while(qtd_mudancas != 0);

    return 0;
}

/**
 * Determina o piso estático de todas as saídas.
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (falha).
*/
int calcular_pisos_estaticos()
{
    if(saidas.num_saidas <= 0 || saidas.vet_saidas == NULL)
    {
        fprintf(stderr,"O número de saídas (%d) é inválido ou o vetor de saidas é NULL.\n", saidas.num_saidas);
        return 1;
    }

    for(int q = 0; q < saidas.num_saidas; q++)
    {
        if( determinar_piso_estatico(saidas.vet_saidas[q]))
            return 1;
    }

    if( combinar_pisos(&(saidas.static_combined_field), 1))
        return 1;

    return 0;
}

/**
 * Calcula o campo de piso dinâmico referente à saida dada como argumento
 * 
 * @param s Saida cujo peso dinâmico será calculado
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int determinar_piso_dinamico(Saida s)
{
    if(s == NULL)
    {
        fprintf(stderr, "NULL como entrada em 'determinar_piso_dinamico'.\n");
        return 1;
    }

    double **mat = s->dinamico;

    // Adiciona um indicador das posições que são ocupadas por paredes, obstáculos ou saídas
    // Indica quais células o piso dinâmico não deve ser calculado
    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            double conteudo = grid_esqueleto[i][h];
            if(conteudo == VALOR_PAREDE)
                mat[i][h] = -1;
            else
                mat[i][h] = 0.0;
        }
    }

    celula *celulas_ocupadas = malloc(sizeof(celula) * pedestres.num_ped);
    int index = -1;
    for(int p = 0; p < pedestres.num_ped; p++)
    {
        if(pedestres.vet[p]->estado == SAIU)
            continue;

        int linha = pedestres.vet[p]->loc_lin;
        int coluna = pedestres.vet[p]->loc_col;

        index++;

        celulas_ocupadas[index].loc_lin = linha;
        celulas_ocupadas[index].loc_col = coluna;
        celulas_ocupadas[index].valor = s->estatico[linha][coluna];
    }

    ordenar_vetor_celulas(celulas_ocupadas,0, index);

    // for(int p = 0; p <= index; p++)
    //     printf("%.2lf ", celulas_ocupadas[p].valor);
    // printf("\n\n");
    
    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            if(mat[i][h] == -1)
                continue;

            double peso_estatico_celula = s->estatico[i][h]; // peso da célula sob análise

            int qtd_pedestres_igual = 0; // qtd de pedestres que ocupam células com o mesmo campo de piso
            int qtd_pedestres_menor = busca_binaria_celulas(celulas_ocupadas, index + 1, 
                                                            peso_estatico_celula, &qtd_pedestres_igual); 
                                                            // qtd de pedestres que ocupam células com campo de piso menor

            if(qtd_pedestres_menor == -1)
                qtd_pedestres_menor += 1;
            
            //printf("v %.1lf m %d i %d\n", peso_estatico_celula, qtd_pedestres_menor, qtd_pedestres_igual);

            mat[i][h] = qtd_pedestres_menor + qtd_pedestres_igual; // dividido pela largura da porta, que por enquanto é sempre 1
            /*  A equação descrita no artigo é 
                    mat[i][h] = qtd_pedestres_menor + (qtd_pedestres_igual / 2.0)
                Porém, seu uso não corresponde com os valores de campo de piso mostrados no próprio artigo. Para que isso ocorra
                é necessário remover a divisão por 2.
            */

        }
    }

    imprimir_piso(mat);


    free(celulas_ocupadas);


    return 0;
}


/**
 * Determina o piso final da porta passada, por meio da combinação dos pesos estáticos e dinâmicos.
 * 
 * @param s Saida cujo piso final será calculado
 * @param alfa coeficiente de evitação de multidões
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int determinar_piso_final(Saida s, double alfa)
{
    if(s == NULL)
    {
        fprintf(stderr, "NULL como entrada em 'determinar_piso_final'.\n");
        return 1;
    }

    for(int i = 0; i < num_lin_grid; i++)
    {
        for(int h = 0; h < num_col_grid; h++)
        {
            if(s->dinamico[i][h] == -1)
                s->field[i][h] = s->estatico[i][h]; // células que devem ser ignoradas no peso dinâmico recebem valores do estático
            else
                s->field[i][h] = s->estatico[i][h] + alfa * s->dinamico[i][h];
        }
    }

    return 0;
}

/**
 * Determina o piso geral do ambiente por meio da fusão dos pisos para cada saída.
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (falha).
*/
int calcular_piso_geral()
{
    if(saidas.num_saidas <= 0 || saidas.vet_saidas == NULL)
    {
        fprintf(stderr,"O número de saídas (%d) é inválido ou o vetor de saidas é NULL.\n", saidas.num_saidas);
        return 1;
    }

    for(int q = 0; q < saidas.num_saidas; q++)
    {
        if( determinar_piso_dinamico(saidas.vet_saidas[q]))
            return 1;

        if( determinar_piso_final(saidas.vet_saidas[q], commands.alfa))
            return 1;
    }

    if( combinar_pisos(&(saidas.dynamic_combined_field), 2))
        return 1;

    return 0;
}

/**
 * 
 * @param piso_combinado ponteiro para a matriz onde a combinação deve ocorrer
 * @param tipo combinação de pisos estáticos (1) ou dinâmicos (2)
 * 
 * @return Inteiro, 0 (sucesso) ou 1 (falha).
*/
int combinar_pisos(double ***piso_combinado, int tipo)
{

    *piso_combinado = alocar_matriz_double(num_lin_grid, num_col_grid);
    if(*piso_combinado == NULL)
    {
        fprintf(stderr,"Falha na alocação da matriz de piso combinado.\n");
        return 1;
    }

    double **fonte = tipo == 1? saidas.vet_saidas[0]->estatico : saidas.vet_saidas[0]->field;
    copiar_matriz_double(*piso_combinado, fonte); // copia o piso da primeira porta
    
    for(int q = 1; q < saidas.num_saidas; q++)
    {
        fonte =  tipo == 1? saidas.vet_saidas[q]->estatico : saidas.vet_saidas[q]->field;
        for(int i = 0; i < num_lin_grid; i++)
        {
            for(int h = 0; h < num_col_grid; h++)
            {
                if((*piso_combinado)[i][h] > fonte[i][h])
                    (*piso_combinado)[i][h] = fonte[i][h];
            }
        }
    }

    return 0;
}