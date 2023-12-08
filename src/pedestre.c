/* 
   File: pedestre.c
   Author: Daniel Gonçalves
   Date: 2023-10-15
   Description: Contém funções responsáveis pela criação e administração do conjunto de pedestres.
                Além disso, contém funções responsáveis pelo processo de movimentação dos pedestres e tratamento
                de conflitos.
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include"../headers/global_declarations.h"
#include"../headers/pedestre.h"

// Equação reduzida da reta. Usada para determinar a existência de movimentação em X.
typedef struct reta{
    double m; // coeficiente angular
    double n; // coeficiente linear (onde corta o eixo y)
}reta;

typedef struct lista_ped_conflito{
    int qtd;
    int ped[8]; // considerando a vizinhança em uso (moore), 8 é a quantidade máxima de conflitos possível
}lista_ped_conflito;

typedef struct lista_vizin_valida{
    int qtd;
    celula vet[8]; // 8 é a quantidade de células na vizinhança desconsiderando a célula do pedestre
}lista_vizin_valida;


Pedestre criar_pedestre(int loc_linha, int loc_coluna);
celula determinar_menor_celula(int loc_lin, int loc_col);
celula determinar_menor_celula_valida(int loc_lin, int loc_col);
void adicionar_na_lista_vizin_valida(lista_vizin_valida *vizinhos,int lin, int col, double valor);
int resolver_movimento_em_x(Pedestre um, Pedestre dois);
int intersecao(reta r_um, reta r_dois, Pedestre p);

/**
 * Aloca uma estrutrua para o novo pedestre (cuja localização foi passada) e a inicializa.
 * 
 * @note O id do pedestre não é preenchido.
 * 
 * @param loc_linha Linha onde o pedestre deve ser inserido.
 * @param loc_coluna Coluna onde o pedestre deve ser inserido.
 * @return Pedestre (sucesso) ou NULL (fracasso).
*/ 
Pedestre criar_pedestre(int loc_linha, int loc_coluna)
{
    Pedestre novo = malloc(sizeof(struct pedestre));
    if(novo != NULL)
    {
        novo->loc_lin = novo->origin_lin = loc_linha;
        novo->loc_col = novo->origin_col = loc_coluna;
        novo->mov_lin = -1;
        novo->mov_col = -1;
        novo->estado = MOVENDO;

        grid_mapa_calor[loc_linha][loc_coluna]++;
    }

    return novo;
}

/**
 * Adiciona um novo pedestre no conjunto de pedestres.
 * 
 * @note O id do pedestre é atribuído aqui.
 * 
 * @param loc_linha Linha onde o pedestre deve ser inserido.
 * @param loc_coluna Coluna onde o pedestre deve ser inserido.
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int adicionar_pedestre_conjunto(int loc_lin, int loc_col)
{
    Pedestre novo = criar_pedestre(loc_lin, loc_col);
    if(novo == NULL)
    {
        fprintf(stderr, "Falha na criação do pedestre em (%d,%d).\n", loc_lin, loc_col);
        return 1;
    }

    pedestres.num_ped += 1;
    pedestres.vet = realloc(pedestres.vet, sizeof(struct pedestre) * pedestres.num_ped);
    if(pedestres.vet == NULL)
    {
        fprintf(stderr,"Falha na realocação do vetor de pedestres.\n");
        return 1;
    }

    novo->id = pedestres.num_ped;
    pedestres.vet[pedestres.num_ped - 1] = novo;

    return 0;
}

/**
 * Insere pedestres de forma aleatória no ambiente.
 * 
 * @note A função não trata casos onde não existe espaço para colocar todos os pedestres.
 * 
 * @param qtd Quantidade de pedestres que devem ser inseridos
 * @return Inteiro, 0 (sucesso) ou 1 (fracasso).
*/
int inserir_pedestres_aleatoriamente(int qtd)
{
    if(qtd <= 0)
        return 1;

    if(zerar_matriz_inteiros(grid_pedestres, num_lin_grid, num_col_grid))
        return 1;

    for(int q = 0; q < qtd;)
    {
        int lin = rand() % (num_lin_grid - 1) + 1;
        int col = rand() % (num_col_grid - 1) + 1;

        if(grid_pedestres[lin][col] != 0 || saidas.combined_field[lin][col] == VALOR_SAIDA 
            || saidas.combined_field[lin][col] == VALOR_PAREDE)
            continue;

        if( adicionar_pedestre_conjunto(lin,col))
            return 1;

        grid_pedestres[lin][col] = pedestres.vet[pedestres.num_ped - 1]->id;
        q++;
    }

    return 0;
}

/**
 * Desaloca o vetor de pedestres e seta a quantidade de pedestres para zero.
*/
void desalocar_pedestres()
{
    for(int p = 0; p < pedestres.num_ped; p++)
        free(pedestres.vet[p]);
        
    free(pedestres.vet);
    pedestres.vet = NULL;

    pedestres.num_ped = 0;
}

/**
 * Determina, com probabilidade de PANICO, se cada pedestre irá entrar em pânico (e ficar parado).
 * 
 * @return Inteiro, quantidade de pedestre que entraram em pânico.
*/
int panico()
{
    int qtd = 0;
    for(int i = 0; i < pedestres.num_ped; i++)
    {
        if(pedestres.vet[i]->estado == SAIU)
            continue;

        if((rand() % 100 + 1) / 100.0 <= PANICO)
        {
            pedestres.vet[i]->estado = PARADO;
            qtd++;

            if(commands.debug)
                printf("Pedestre %d em panico.\n", pedestres.vet[i]->id);
        }
    }

    return qtd;
}

/**
 * Determina a célula para a qual cada pedestre pretende se movimentar.
*/
void determinar_movimento()
{
    for(int p = 0; p < pedestres.num_ped; p++)
    {
        Pedestre atual = pedestres.vet[p];

        if(atual->estado == SAIU || atual->estado == PARADO)
            continue;

        celula destino = determinar_menor_celula_valida(atual->loc_lin, atual->loc_col);

        if(destino.loc_lin == -1 && destino.loc_col == -1)
        {  // não existem células vizinhas válidas para movimentação 
            atual->estado = PARADO;
        
            if(commands.debug)
                printf("Ped %d, encurralado.\n", atual->id);
        }
        else
        {
            // altera a intenção de movimentação
            atual->mov_lin = destino.loc_lin;
            atual->mov_col = destino.loc_col;
        }
    }
}

/**
 * Varre a vizinhança da célula indicada (onde um pedestre está) e determina se a célula com menor
 * campo de piso está ou não ocupada. Se estiver seta a qtd de células válidas para 0, indicando que o pedestre
 * não pode se mover.
 * 
 * @param loc_linha Linha onde o pedestre está.
 * @param loc_coluna Coluna onde o pedestre está.
 * @return Celula que o pedestre na posição indicada deve se mover ou {-1,-1,-1} se ele deve ficar parado.
*/
celula determinar_menor_celula(int loc_lin, int loc_col)
{
    double **piso = saidas.combined_field;
    lista_vizin_valida *vizinhos = calloc(1, sizeof(lista_vizin_valida));

    for(int j = -1; j < 2; j++)
    {
        for(int k = -1; k < 2; k++)
        {
            if(piso[loc_lin + j][loc_col + k] == VALOR_PAREDE)
                continue; // parede na célula

            if(loc_lin + j == loc_lin && loc_col + k == loc_col)
                continue; // célula do próprio pedestre

            if(j != 0 && k != 0)
            {
                if(! eh_diagonal_valida(loc_lin,loc_col,j,k,piso))
                    continue; // impossível chegar na célula
            }


            adicionar_na_lista_vizin_valida(vizinhos,loc_lin + j,loc_col + k, piso[loc_lin + j][loc_col + k]);
        }
    }

    celula destino = {-1,-1,-1};

    if(vizinhos->qtd > 0)
    {
        int iguais = 1;

        for(int i = 1; i < vizinhos->qtd; i++)
        {   
            if(vizinhos->vet[i].valor != vizinhos->vet[0].valor)
                break;

            iguais++;
        }

        int sorted_cell = rand() % iguais;

        if(grid_pedestres[vizinhos->vet[sorted_cell].loc_lin][vizinhos->vet[sorted_cell].loc_col] == 0)
            destino = vizinhos->vet[sorted_cell]; // apenas se a célula não estiver ocupada
    }

    free(vizinhos);

    return destino;
}


/**
 * Varre a vizinhança da célula indicada (onde um pedestre está) e determina as ceĺulas vizinhas em que o pedestre
 * pode se mover.
 * 
 * @param loc_linha Linha onde o pedestre está.
 * @param loc_coluna Coluna onde o pedestre está.
 * @return Celula que o pedestre na posição indicada deve se mover ou {-1,-1,-1} se ele deve ficar parado.
*/
celula determinar_menor_celula_valida(int loc_lin, int loc_col)
{
    double **piso = saidas.combined_field;
    lista_vizin_valida *vizinhos = calloc(1, sizeof(lista_vizin_valida));

    for(int j = -1; j < 2; j++)
    {
        for(int k = -1; k < 2; k++)
        {
            if(piso[loc_lin + j][loc_col + k] == VALOR_PAREDE)
                continue; // parede na célula

            if(grid_pedestres[loc_lin + j][loc_col + k] > 0)
                continue; // pedestre na célula (inclui a célula do próprio pedestre)

            if(j != 0 && k != 0)
            {
                if(! eh_diagonal_valida(loc_lin,loc_col,j,k,piso))
                    continue; // impossível chegar na célula
            }

            adicionar_na_lista_vizin_valida(vizinhos,loc_lin + j,loc_col + k, piso[loc_lin + j][loc_col + k]);
        }
    }

    celula destino = {-1,-1,-1};

    if(vizinhos->qtd > 0)
    {
        int iguais = 1;

        for(int i = 1; i < vizinhos->qtd; i++)
        {   
            if(vizinhos->vet[i].valor != vizinhos->vet[0].valor)
                break;

            iguais++;
        }

        int sorted_cell = rand() % iguais;

        destino = vizinhos->vet[sorted_cell];
    }

    free(vizinhos);

    return destino;
}

/**
 * Adiciona, de forma ordenada, a célula vizinha indicada na lista de vizinhança válida.
 * 
 * @note Utiliza isertion sort para ordenação.
 * 
 * @param vizinhos Lista de vizinhança válida
 * @param lin Linha da célula vizinha.
 * @param col Coluna da célula vizinha.
 * @param valor Valor da célula vizinha.
*/
void adicionar_na_lista_vizin_valida(lista_vizin_valida *vizinhos,int lin, int col, double valor)
{
    celula nova = {lin,col,valor};

    if(vizinhos->qtd == 0)
    {
        vizinhos->vet[0] = nova;
    }
    else
    {
        int i;
        for(i = vizinhos->qtd - 1; i >= 0; i--)
        {
            if(valor < vizinhos->vet[i].valor)
                vizinhos->vet[i + 1] = vizinhos->vet[i];
            else
            {
                vizinhos->vet[i + 1] = nova;
                break;
            }
        }

        if(i < 0)
            vizinhos->vet[0] = nova;
    }

    vizinhos->qtd++;
}

/**
 * Determina pedestres que pretendem se mover para a mesma célula e decide qual deles irá se mover.
 * Os outros permanecem parados.
 * 
 * @return Inteiro, indicando a quantidade de conflitos.
*/
int resolver_conflitos_movimento()
{
    int count = 0;
    int **mat_conflitos = alocar_matriz_int(num_lin_grid,num_col_grid); // matriz zerada
    lista_ped_conflito *conflitos = NULL;

    // verifica para onde todos os pedestres querem se mover e determina onde existe conflitos
    for(int i = 0; i < pedestres.num_ped; i++)
    {
        Pedestre atual = pedestres.vet[i];

        if(atual->estado == SAIU || atual->estado == PARADO)
            continue;

        int conteudo = mat_conflitos[atual->mov_lin][atual->mov_col]; 
        if(conteudo > 0) // novo conflito
        {
            conflitos = realloc(conflitos, sizeof(lista_ped_conflito) * (count + 1));
            conflitos[count].ped[0] = conteudo;
            conflitos[count].ped[1] = atual->id;
            conflitos[count].qtd = 2;
            /* já existia um pedestre na célula. Uma lista de pedestres em conflito para aquela célula é criada
               e preenchida com os dois pedestre em conflito.*/


            count++;

            if(commands.debug)
                printf("Conflito %d: %d %d.\n", count, conteudo, atual->id);
            mat_conflitos[atual->mov_lin][atual->mov_col] = count * -1;
            // o número negativo indica que já existe um conflito na célula.
            // Além disso, serve como index para a lista de pedestres em conflito.
            // index = (conteudo * -1) - 1;
        }
        else if(conteudo < 0) // conflito já existente
        {
            int index = (conteudo * -1) - 1;
            conflitos[index].ped[conflitos[index].qtd] = atual->id;
            conflitos[index].qtd++;

            if(commands.debug)
                printf("Conflito %d: ... %d\n", count, atual->id);
        }
        else
            mat_conflitos[atual->mov_lin][atual->mov_col] = atual->id;
            // o id do pedestre é colocado na célula que ele pretende se movimentar se ela estiver vazia.
    }

    // para cada conflito, determina o pedestre que irá se movimentar
    for(int i = 0; i < count; i++)
    {
        int sorted_num = rand() % conflitos[i].qtd;
        // sorteia um número entre 0 e ( quantidade de ped em conflito, excluso)

        for(int h = 0; h < conflitos[i].qtd; h++)
        {
            int index_ped_atual = conflitos[i].ped[h] - 1;

            if(sorted_num != h)
                pedestres.vet[index_ped_atual]->estado = PARADO;
        }

        if(commands.debug)
            printf("Conflito %d resolução: %d move.\n", i + 1, conflitos[i].ped[sorted_num]);
    }

    desalocar_matriz_int(mat_conflitos,num_lin_grid);
    free(conflitos);

    return count;
}

/**
 * Busca por pedestres realizando movimentação em X e resolve os conflitos.
 * 
 * @note Varre a grid de pedestre no tempo atual em busca de pedestres adjascentes uns aos outros.
 * @note Quando encontrados, envia os pedestres para a função que verifica e lida com movimentações em X.
*/
void varredura_movimento_em_x()
{
    for(int i = 1; i < num_lin_grid - 1; i++) // evita os limites da grid pois pedestres não estarão lá
    {
        for(int h = 1; h < num_col_grid - 1; h++)
        {
            int cel_atual = grid_pedestres[i][h];
            if(cel_atual > 0)
            {
                if(pedestres.vet[cel_atual - 1]->estado != MOVENDO)
                    continue;

                // pedestres acima da célula atual já terão sido verificados

                // o possível pedestre na célula à esquerda da atual já terá sido verificado
                int cel_adj = grid_pedestres[i][h + 1];
                if(cel_adj > 0)
                    resolver_movimento_em_x(pedestres.vet[cel_atual- 1], pedestres.vet[cel_adj - 1]);

                for(int k = -1; k < 2; k++) // linha inferior
                {
                    int cel_adj = grid_pedestres[i + 1][h + k];
                    if(cel_adj > 0)
                        resolver_movimento_em_x(pedestres.vet[cel_atual- 1], pedestres.vet[cel_adj - 1]);
                }
            }
        }
    }
}

/**
 * Verifica se os dois pedestres indicados estão realizando movimentação em X e decide qual irá se mover, se for o caso.
 * 
 * @param um Pedestre.
 * @param dois Pedestre.
 * @return Inteiro, 0 (não ocorrência de movimentação em X), 1 (existência de movimentação em X) ou 2 (fracasso).
*/
int resolver_movimento_em_x(Pedestre um, Pedestre dois)
{
    if(um == NULL || dois == NULL)
        return 2;

    if(um->estado != MOVENDO || dois->estado != MOVENDO)
        return 0;

    reta r_um, r_dois;
    // Cada reta contém o segmento de reta delimitado pelo ponto de origem e destino de um dos pedestres. 

    if(um->mov_col == um->loc_col || dois->mov_col == dois->loc_col)
        return 0; // coeficiente angular infinito (divisão por zero), ocasionando uma reta na vertical. Impossível ocorrer movimentos em X.

    if(um->mov_lin == um->loc_lin || dois->mov_lin == dois->loc_lin)
        return 0; // uma das retas está na horizontal.Não ocorre movimentações em X nesta situação.

    // m = (yf - yi) / (xf - xi)
    r_um.m = (um->mov_lin - um->loc_lin) / (um->mov_col - um->loc_col);
    r_dois.m = (dois->mov_lin - dois->loc_lin) / (dois->mov_col - dois->loc_col);

    // n = yi - m * xi
    r_um.n = um->loc_lin - r_um.m * um->loc_col;
    r_dois.n = dois->loc_lin - r_dois.m * dois->loc_col;

    // if(commands.debug)
    // {
    //     printf("Um: %d %d --> %d %d\n", um->loc_lin, um->loc_col, um->mov_lin, um->mov_col);
    //     printf("Dois: %d %d --> %d %d\n", dois->loc_lin, dois->loc_col, dois->mov_lin, dois->mov_col);
    // }

    if(intersecao(r_um,r_dois,um) == 1)
    {

        int sorted_num = rand() % 100;

        if(sorted_num < 50)
            dois->estado = PARADO;
        else
            um->estado = PARADO;

        if(commands.debug)
            printf("Movimento em X entre %d e %d: %d move.\n", um->id, dois->id, sorted_num < 50 ? um->id : dois->id);

        return 1;
    }

    return 0;
}

/**
 * Determina se as duas retas passadas se interceptam no segmento determinado pela origme e destino do pedestre.
 * 
 * @param r_um Primeira reta.
 * @param r_dois Segunda reta.
 * @param p Pedestre
 * @return Inteiro, 0 (não se inerceptam), 1 (se interceptam no intervalo) ou 2 (se interceptam na ponta do intervalo).
*/
int intersecao(reta r_um, reta r_dois, Pedestre p)
{
    if(r_um.m == r_dois.m) // retas paralelas ou iguais
        return 0;

     /*
        (1): y = m1 * x + n1
        (2): y = m2 * x + n2

        m1 * x + n1 = m2 * x + n2
        m1*x - m2*x = n2 - n1
        x(m1 - m2) = n2 - n1
        x = (n2 - n1) / (m1 - m2)  
    */

    double intersect_x = (r_dois.n - r_um.n) / (r_um.m - r_dois.m);
    double intersect_y = r_um.m * intersect_x + r_um.n;

    if(intersect_x == p->mov_col && intersect_y == p->mov_lin)
        return 2; // os pedestre se movem para o mesmo local

    if(intersect_x > fmin(p->loc_col, p->mov_col) && intersect_x < fmax(p->loc_col, p->mov_col)
        && intersect_y > fmin(p->loc_lin, p->mov_lin) && intersect_y < fmax(p->loc_lin, p->mov_lin))
        return 1;  // movimetanção em x
}

/**
 * Pedestres que estejam no estado MOVENDO mudam sua célula atual para a célula que pretendiam se mover
 * (As coordenadas de destino são copiadas para as coordenadas atuais).
 * 
*/
void confirmar_movimentacao()
{
    for(int i = 0; i < pedestres.num_ped; i++)
    {
        Pedestre atual = pedestres.vet[i];
        if(atual->estado == MOVENDO)
        {
            atual->loc_lin = atual->mov_lin;
            atual->loc_col = atual->mov_col;

            if(saidas.combined_field[atual->loc_lin][atual->loc_col] == VALOR_SAIDA)
            {
                atual->estado = SAIU;
                grid_mapa_calor[atual->loc_lin][atual->loc_col]++;
            }
        }
    }
}

/**
 * Verifica se todos os pedestres saíram do ambiente.
 * 
 * @return Inteiro, 0 (todos os pedestres saíram) ou 1 (ainda há pedestres no ambiente).
*/
int ambiente_vazio()
{
    for(int i = 0; i < pedestres.num_ped; i++)
    {
        Pedestre atual = pedestres.vet[i];
        if(atual->estado != SAIU)
            return 1;
    }

    return 0;
}

/**
 * Zera a grid de pedestres e atualza com as localizações atuais de todos os pedestres.
*/
void atualizar_grid_pedestres()
{
    zerar_matriz_inteiros(grid_pedestres, num_lin_grid, num_col_grid);

    for(int p = 0; p < pedestres.num_ped; p++)
    {
        Pedestre atual = pedestres.vet[p];

        if(atual->estado == SAIU)
            continue;

        grid_pedestres[atual->loc_lin][atual->loc_col] = atual->id;
        grid_mapa_calor[atual->loc_lin][atual->loc_col]++;
    }
}

/**
 * Reseta todos os pedestres que estão em estado PARADO para estado MOVENDO.
*/
void resetar_estado_pedestres()
{
    for(int p = 0; p < pedestres.num_ped; p++)
    {
        if(pedestres.vet[p]->estado != SAIU)
           pedestres.vet[p]->estado = MOVENDO;

    }
}

/**
 * Reinicia o estado da estrutura do pedestre para seu original (assim que foi criado).
*/
void reiniciar_pedestre()
{
    zerar_matriz_inteiros(grid_pedestres, num_lin_grid, num_col_grid);
    
    for(int p = 0; p < pedestres.num_ped; p++)
    {
        pedestres.vet[p]->loc_lin = pedestres.vet[p]->origin_lin;
        pedestres.vet[p]->loc_col = pedestres.vet[p]->origin_col;
        pedestres.vet[p]->estado = MOVENDO;
        grid_pedestres[pedestres.vet[p]->loc_lin][pedestres.vet[p]->loc_col] = pedestres.vet[p]->id;
    }
}