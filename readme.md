# Implementação do Modelo de Alizadeh para evacuação de pedestres

Esta implementação toma como base a implementação do modelo de Varas, disponível [aqui](https://github.com/DanielGoncalves666/Reimplementacao-Modelo-Varas) e adiciona alguns mecanismos que impactam na movimentação dos pedestres.

## Terminologia

### Simulação

Entende-se como uma única execução do processo de evacuação, compreendo o momento em que os pedestres estão parados até o momento em que a sala estiver vazia.

### Conjunto de Simulação

Endende-se como um conjunto de simulações de mesmos parâmetros, com excessão do parâmetro **seed**.

## Compilar e Executar

Em um terminal execute `./alizadeh.sh`, seguido dos argumentos necessários.

## Arquivos de Entrada e Saída

### Arquivos de Ambiente

Os arquivos de ambiente contém, em sua primeira linha, as dimensões do ambiente (linhas e colunas, respectivamente). Em seguida o ambiente é desenhado, devendo ser das dimensões especificadas na primeira linha.
Os seguintes símbolos são usados:

|Símbolo   | Significado          |
|    ---   |        ---           |
| \#       | Paredes e obstáculos |
| p ou P   | Pedestres            |
| _        | Saídas               |
| .        | Vazio                |

Arquivos de ambiente devem ser inseridos no diretório `ambientes/`.

### Arquivos Auxiliares

Os arquivos auxiliares contêm as coordenadas em que saídas devem ser inseridas no ambiente, desde que o modo correto
tenha sido escolhido. Cada linha contém um conjunto de saídas que será usado pelo número de simulações determinadas,
sendo substituído pelo próximo conjunto (se existir) assim que elas acabem. A seguinte sintaxe é usada:

1 - Vírgulas são usadas para separar diferentes saídas.

2 - O símbolo de adição ( + ) é usado para indicar que o próximo par de coordenadas faz parte da mesma saída.

3 - Um ponto final indica o fim da lista de coordenadas para uma dada simulação.

Exemplos:

```text
linha1 coluna1, linha2 coluna2, ... , linhaN colunaN.
linha1_1 coluna1_1+ linha1_2 coluna1_2, ...
```

Um número indeterminado de portas é aceito para um dado conjunto de simulações.
Saídas repetidas são aceitas (e tratadas como duas saídas distintas).

Arquivos auxiliares devem ser inseridos no diretório `saidas/`.

#### Gerador de Arquivo Auxiliar

No diretório `gerador/` há um programa que, dado um arquivo com coordenadas de saídas em um ambiente, realiza combinações entre elas.
Utilize o argumento `--help` para receber ajuda sobre seu funcionamento.

### Arquivos de Saída

Os arquivos de saída contém os resultados das simulações e são salvos no diretório `output/`.

## Observações

Esta implementação do modelo adiciona mecanismos que impedem movimentação cruzada entre dois pedestres e que
impendem a movimentação de pedestres através de obstáculos colocados nas diagonais.

## Manual de Uso

```bash
Usage: alizadeh.exe [OPTION...]
Alizadeh - Simula uma evacuação de pedestres por meio do modelo de
(Alizadeh,2011), com pequenas alterações.

  
Arquivos:

  -a, --auxiliary-file=ARQUIVO_AUXILIAR
                             Contém informações auxiliares para a
                             realização das simulações. Ex: localização
                             das saídas.
  -i, --input-file=INPUT-FILE   INPUT-FILE é o nome do arquivo que contém o
                             ambiente pre-definido a ser carregado.
  -o, --output-file[=OUTPUT-FILE]
                             Indica se a saída deve ser armazenada em um
                             arquivo, com o nome do arquivo sendo opcionalmente
                             passado.
  
Modos de operação:

  -m, --input-method=METHOD  Indica como tratar INPUT_FILE.
  -O, --output-type=TYPE     Indica o tipo de saída que deve ser gerada pelas
                             simulações.
  
Dimensões do ambiente:

  -c, --col=COLUNAS          COLUNAS indica a quantidade de colunas que o
                             ambiente criado deve ter.
  -l, --lin=LINHAS           LINHAS indica a quantidade de linhas que o
                             ambiente criado deve ter.
  
Variáveis de simulação:

      --alfa=ALFA            Coeficiente de evitação de multidões
  -e, --seed=SEED            Semente inicial para geração de números
                             pseudo-aleatórios.
  -p, --ped=PEDESTRES        Número de pedestres a serem inseridos no ambiente
                             de forma aleatória.
  -s, --simu=SIMULACOES      Número de simulações a serem realizadas por
                             conjunto de saídas.
  
Toggle Options:

      --detalhes             Indica se o output deve conter informações sobre
                             as saídas.
  -d, --debug                Indica se mensagens de debug devem ser impressas
                             na saída padrão.
      --evitar-mov-cantos    Indica que a movimentação através de cantos de
                             paredes/obstáculos deve ser impedida.
      --na-saida             Indica que o pedestre deve permanecer por um passo
                             de tempo quando chega na saída (invés de ser
                             retirado imediatamente).
      --permitir-mov-x       Permite que os pedestres se movimentem em X.
      --sempre-menor         Indica que a movimentação dos pedestres é
                             sempre para a menor célula, com o pedestre
                             ficando parado se ela estiver ocupada.
      --status               Indica se mensagens de status durante a execução
                             de simulações devem ser impressas em stdout.
  
Outros:

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

O arquivo passado por --auxiliary-file deve conter, em cada uma de suas linhas,
as localizações das saídas para um único conjunto de simulações.

--output-type indica quais e como os dados gerados pela simulação devem ser
enviados para a saída. As seguintes opções são possíveis:
         1 - Impressão visual do ambiente.
         2 - Quantidade de passos de tempo até o fim das simulações.
         3 - Impressão de mapas de calor das células do ambiente.
         4 - Variação da distribuição de pedestres para duas saídas, no início
da simulação.
A opção 1 é a padrão.
A opção 4 gera números entre 0 e 1, onde valores próximos à zero indicam
uam distribuição uniforme entre as duas saídas.Já valores próximos a 1
indicam uma distribuição de pedestres desigual.

--input-method indica como o ambiente informado em --input-file deve ser
carregado ou se o ambiente deve ser gerado.
        Ambiente carregado de um arquivo:
                1 - Apenas a estrutura do ambiente (portas substituídas por paredes).
                2 - Estrutura e portas.
                3 - Estrutura e pedestres.
                4 - Estrutura, portas e pedestres.
        Ambiente criado automaticamente:
                5 - Ambiente será criado considerando quantidade de linhas e colunas
passadas pelas opções --lin e --col.
Opções que não carregam portas do arquivo de entrada devem recebê-las via
--auxiliary-file.
O método 4 é o padrão.
Para os métodos 1,3 e 5, --auxiliary-file é obrigatório.
Para o método 5, --lin e --col são obrigatórios.

As variáveis de simulação não são obrigatórias.
--alfa tem valor padrão de 0.
--input-file tem valor padrão de "sala_padrao.txt".
--simu e --ped tem valor padrão de 1.
--seed tem valor padrão de 0.

Toggle Options são opções que podem ser ativadas e também não são
obrigatórias.
--na-sala quando ativado obriga os pedestres a ficarem um passo de tempo na
saída do ambiente antes de serem removidos.
--sempre-menor quando ativado obriga os pedestres a só se moverem para a menor
célula de sua vizinhança. Se esta estiver ocupada, o pedestre irá esperar
ela ser desocupada.
--evitar-mov-cantos quando ativado impede que pedestres se movimentem através
dos cantos de paredes/obstáculos. Um único movimento se torna necessariamente
em 3 movimentos.
--permitir-mov-x quando ativado permite que os pedestres ignorem a restrição
que impede movimentações em X.
--debug ativa mensagens de debug.
--status ativa mensagens que indicam o progessão de simulações.
--detalhes inclui um cabeçalho contendo as correspondentes saídas de cada
conjunto de simulação.

Opções desnecessárias para determinados --input-method são ignoradas.
```

## Geração de Gráficos

Na pasta graphics há um programa para geração dos gráficos presentes no artigo de Alizadeh. O execute com a opção `--help` para receber ajuda sobre seu funcionamento.
