/**
 * @file funcoesLeituraDados.c
 * @author Telma Woerle de Lima Soares
 * @date 12 Sep 2013
 * @brief Contém a implementação das funções utilizadas para a leitura dos dados no novo formato especificado.
 * Este arquivo implementa as funções definidas na biblioteca funcoesLeituraDados.h
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoesLeituraDados.h"
#include "funcoesRNP.h"

long int numeroAlimentadores; /**< Variável global para armazenar o número de alimentadores do SDR. */
long int numeroReguladores; /**< Variável global para armazenar o número de reguladores de tensão do SDR. */
extern __complex__ double ij; /**< Variável global para converter os valores para número complexo. */

/**
 * @brief Função principal para a leitura do arquivo dados_barras_linhas_trafos.dad.
 *
 * Essa função realiza a leitura do arquivo dados_barras_linhas_trafos.dad por meio das 
 * funções leituraBarras, leiturasLinhas e leituraTrafos. Essa função principal é responsável por 
 * localizar os marcadores DBAR, DLIM e DTRAFO. A função assume que o nome do arquivo é padrão da forma 
 * dados_barras_linhas_trafos.dad. Além disso o arquivo possui como separação dos dados os marcadores especificados.
 * Ao localizar o marcador o fluxo de controle é desviado para a função correspondente para a leitura dos dados referentes ao marcador.
 * Recebe como parâmetros de entrada e saída um ponteiro para ponteiro do tipo GRAFO @p **grafoSDRParam e
 * um ponteiro para ponteiro do tipo DADOSTRAFO @p  **dadosTrafoSDRParam.
 * A função retorna @c BOOL indicando se a leitura foi bem sucedida.
 * Para utilizar a função:
 * @code
 * GRAFO *grafoExemplo;
 * DADOSTRAFO *trafoExemplo;
 * BOOL leitura = leituraBarrasLinhasTrafos( &grafoExemplo, &trafoExemplo);
 * if (leitura)
 *      printf("leitura concluida\n");
 * @endcode
 * 
 * @param grafoSDRParam
 * @param dadosTrafoSDRParam
 * @param numeroBarras
 * @param numeroTrafos
 * @param numeroChaves
 * @return  * @return Retorna o tipo booleano definido indicando que a leitura foi bem sucedida.
 * @see leituraBarras
 * @see leituraLinhas
 * @see leituraTrafos
 * @note
 * @warning .
 */

BOOL leituraBarrasLinhasTrafos(GRAFO **grafoSDRParam, DADOSTRAFO **dadosTrafoSDRParam, long int *numeroBarras, long int *numeroTrafos, long int *numeroChaves)
{
    FILE *arquivo; /* Variável ponteiro para a manipulação do arquivo. */
    char blocoLeitura[100]; /* Variável para realizar a leitura de caracteres do arquivo. */
    char marcador[10]; /* Variável para ler o marcador que diferencia cada trecho do arquivo. */
    BOOL continuaLeitura =  true;
    // Abertura do arquivo dados_barras_linhas_trafos.dad para leitura e associação ao ponteiro de arquivo
    arquivo = fopen("dados_barras_linhas_trafos.dad","r");
    
    //Verifica se o arquivo foi aberto com sucesso, caso ocorra um erro retorna aviso ao usuário e encerra o programa.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo dados_barras_linhas_trafos.dad !!!\n");
            exit(1);
    }
    
    //Realiza a leitura até que seja encontrado o fim do arquivo.
    while(continuaLeitura && !feof(arquivo))
    {
        //Leitura de um bloco de caracteres do arquivo.
        fgets(blocoLeitura,100,arquivo);
        //Extrai da string lida a primeira palavra na variável marcador.
        sscanf(blocoLeitura,"%s",marcador);
        //Converte a string da variável marcador para caracteres em UPPERCASE.
        //strupr(marcador);
        //Realiza a comparação da string armazenada na variável marcador com os marcadores definidos. Caso a string não correponda a nenhum marcador encerra o programa.
      //  printf("Marcador %s \n", marcador);
        if (!strcmp(marcador, "DBAR"))
        {
            leituraBarras(arquivo,grafoSDRParam, numeroBarras); //Direciona o fluxo para a função leituraBarras que realiza a leitura no formato do trecho de barras
        }
        else
        if (!strcmp(marcador, "DLIN"))
        {
            leituraLinhas(arquivo,*grafoSDRParam, numeroChaves); //Direciona o fluxo para a função leituraLinhas que realiza a leitura no formato do trecho de linhas
        }
        else
        if (!strcmp(marcador, "DTRAFO"))
        {
        //    leituraTrafos(arquivo,dadosTrafoSDRParam, numeroTrafos); //Direciona o fluxo para a função leituraTrafos que realiza a leitura no formato do trecho de trafos
            continuaLeitura = false;
        }
        else
        {
            printf("Erro ao ler arquivo dados_barras_linhas_trafos.dad !!! \n Formato Invalido!!!\n");
            exit(1);
        }    
        
    }
    //Fecha o arquivo
    fclose(arquivo);
    return(true);
}

/**
 * @brief Função auxiliar para a leitura do arquivo dados_barras_linhas_trafos.dad, referente ao trecho relativo as informações das barras. 
 *
 * Essa função realiza a leitura do trecho do arquivo dados_barras_linhas_trafos.dad relativo ao marcador DBAR 
 * que contém os dados relativos as barras do sistema. Também é responsável pela alocação de memória necessária para armazenar o SDR
 * na forma de um grafo. Recebe como parâmetro de entrada e saída um ponteiro para ponteiro do tipo GRAFO., @p **grafoSDRParam .
 * A função retorna @c void.
 * 
 * @param arquivo ponteiro para o arquivo onde está sendo realizada a leitura.
 * @param grafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do GRAFO, onde é retornado o SDR na forma de um grafo.
 * @param numeroBarras
 * @return void.
 * @see leituraBarrasLinhasTrafos
 * @note Para realizar a leitura dos dados de P e Q, que são constituídos de 24 pares, utilizou-se a string de controle %n. 
 * Essa string de controle retorna a quantidade de caracteres lidos. Com isso é possível no sscanf fazer a alteração do ponteiro
 *  e utilizar uma estrutura de repetição para realizar a leitura desses dados. 
 * @warning Como se trata de uma função auxiliar essa não deve ser chamada diretamente por outras partes do programa.
 */
/**
 * 
 * @param arquivo
 * @param grafoSDRParam
 * @param numeroBarras
 */
void leituraBarras(FILE *arquivo, GRAFO **grafoSDRParam, long int *numeroBarras)
{
    char blocoLeitura[2000]; /* Variável para realizar a leitura do bloco de caracteres do arquivo. */
    char *dados; /* Variável do tipo ponteiro para char, utilizada para alterar o ponteiro da string lida do arquivo de forma a realizar o loop no sscanf. */
    
    long int contador; /* Variáveis contadores para percorrer o arquivo e a string de leitura. */
    long int numeroNos; /* Variável para ler o número de nós que o arquivo contém. */
    PQ valorPQ; /* Variável array do tipo PQ auxiliar para extrair os valores de P e Q da string lida. */
    long int identificadorNo; /* Variável auxiliar para leitura do identificador do nó na string lida do arquivo. */
    int tipoDoNo; /* Variável auxiliar para leitura do tipo do nó na string lida do arquivo. */
    PRIORIZACAO dadosPriorizacao; /* Variável auxiliar para leitura dos dados de priorizações da barra na string lida do arquivo. */
    COMPENSACAO dadosCompensacao; /* Variável auxiliar para leitura dos dados de compensações na string lida do arquivo. */   
    
    //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura, 2000, arquivo);
    //identificação da variável numero de nós da string lida do arquivo
    sscanf(blocoLeitura, "%ld",&numeroNos);
    //atribui o valor lido da string a variável global que armazena o número de barras do SDR
    numeroBarras[0] = numeroNos;
    
    //realiza a alocação da estrutura em grafo para armazenar o SDR e verifica se o processo foi bem sucedido
    //caso não seja possível realizar a alocação o programa é encerrado
    if (((*grafoSDRParam) = (GRAFO *)malloc( (numeroNos+1) * sizeof(GRAFO)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para o grafo do SDR !!!!");
        exit(1); 
    }
    //realiza a leitura e inserção na estrutura do grafo dos dados relativos as barras do SDR
    for (contador = 1; contador<=numeroNos; contador++)
    {
        fgets(blocoLeitura, 2000, arquivo);
        //atribui a string lida do arquivo a variável auxiliar dados, do tipo ponteiro para permitir a alteração do ponteiro da string
        dados = blocoLeitura;
        //faz a leitura na string dos dados identificador do nó(barra) e do tipo do nó, o terceiro valor retorna a quantidade de caracteres lidos
        sscanf(dados, "%ld %d %lf %lf %lf %lf %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&identificadorNo,&tipoDoNo,&valorPQ.p,&valorPQ.q,&dadosPriorizacao.eusdGrupoA,&dadosPriorizacao.eusdGrupoB,&dadosPriorizacao.qtdConsumidores,&dadosPriorizacao.qtdConsumidoresEspeciais,&dadosCompensacao.metaDicMensal,&dadosCompensacao.metaDicTrimestral,&dadosCompensacao.metaDicAnual,&dadosCompensacao.metaDmicMensal,&dadosCompensacao.metaFicMensal, &dadosCompensacao.metaFicTrimestral, &dadosCompensacao.metaFicAnual, &dadosCompensacao.resulDicAcumAno, &dadosCompensacao.resulDicAcumTrimestre, &dadosCompensacao.resulDicAcumMes, &dadosCompensacao.resulFicAcumAno, &dadosCompensacao.resulFicAcumTrimestre, &dadosCompensacao.resulFicAcumMes, &dadosCompensacao.resulDmicMes);           

        //printf("barra %ld consumidor %d consumidorEspecial %d \t", identificadorNo, dadosPriorizacao.qtdConsumidores, dadosPriorizacao.qtdConsumidoresEspeciais);
        //atribui os dados lidos nas variáveis auxiliares para a estrutura que armazena o grafo que representa do SDR
        //o formato (*grafoSDRParam) é utilizado pois o parâmetro trata-se de um ponteiro para ponteiro
        (*grafoSDRParam)[contador].idNo = identificadorNo;
        (*grafoSDRParam)[contador].tipoNo = tipoDoNo;
        (*grafoSDRParam)[contador].numeroAdjacentes = 0;
        (*grafoSDRParam)[contador].compensacoes.metaDicAnual = dadosCompensacao.metaDicAnual;
        (*grafoSDRParam)[contador].compensacoes.metaDicMensal = dadosCompensacao.metaDicMensal;
        (*grafoSDRParam)[contador].compensacoes.metaDicTrimestral = dadosCompensacao.metaDicTrimestral;
        (*grafoSDRParam)[contador].compensacoes.metaDmicMensal = dadosCompensacao.metaDmicMensal;
        (*grafoSDRParam)[contador].compensacoes.metaFicAnual = dadosCompensacao.metaFicAnual;
        (*grafoSDRParam)[contador].compensacoes.metaFicMensal = dadosCompensacao.metaFicMensal;
        (*grafoSDRParam)[contador].compensacoes.metaFicTrimestral = dadosCompensacao.metaFicTrimestral;
        (*grafoSDRParam)[contador].compensacoes.resulDicAcumAno = dadosCompensacao.resulDicAcumAno;
        (*grafoSDRParam)[contador].compensacoes.resulDicAcumMes = dadosCompensacao.resulDicAcumMes;
        (*grafoSDRParam)[contador].compensacoes.resulDicAcumTrimestre = dadosCompensacao.resulDicAcumTrimestre;
        (*grafoSDRParam)[contador].compensacoes.resulDmicMes = dadosCompensacao.resulDmicMes;
        (*grafoSDRParam)[contador].compensacoes.resulFicAcumAno = dadosCompensacao.resulFicAcumAno;
        (*grafoSDRParam)[contador].compensacoes.resulFicAcumMes = dadosCompensacao.resulFicAcumMes;
        (*grafoSDRParam)[contador].compensacoes.resulFicAcumTrimestre = dadosCompensacao.resulFicAcumTrimestre;
        (*grafoSDRParam)[contador].priorizacoes.eusdGrupoA = dadosPriorizacao.eusdGrupoA;
        (*grafoSDRParam)[contador].priorizacoes.eusdGrupoB = dadosPriorizacao.eusdGrupoB;
        (*grafoSDRParam)[contador].priorizacoes.qtdConsumidores = dadosPriorizacao.qtdConsumidores;
        (*grafoSDRParam)[contador].priorizacoes.qtdConsumidoresEspeciais = dadosPriorizacao.qtdConsumidoresEspeciais;
        (*grafoSDRParam)[contador].valorPQ.p = valorPQ.p;
        (*grafoSDRParam)[contador].valorPQ.q = valorPQ.q;
        (*grafoSDRParam)[contador].idSetor = 0;
        //alocação do espaço para armazenar a lista de adjacentes que será lida na função leituralinhas
        (*grafoSDRParam)[contador].adjacentes = Malloc(NOADJACENTE,11);
    }
 }

/**
 * @brief Função auxiliar para a leitura do arquivo dados_barras_linhas_trafos.dad, referente ao trecho relativo as informações das linhas. 
 *
 * Essa função realiza a leitura do trecho do arquivo dados_barras_linhas_trafos.dad relativo ao marcador DLIN 
 * que contém os dados relativos as linhas do sistema. Nessa função é construída a lista de adjacentes de cada nó da representação do SDR na 
 * forma de grafos. Recebe como parâmetro de entrada e saída um ponteiro do tipo GRAFO., @p *grafoSDRParam .
 * A função retorna @c void.
 * 
 * @param arquivo ponteiro para o arquivo onde está sendo realizada a leitura.
 * @param grafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do GRAFO, onde é retornado o SDR na forma de um grafo.
 * @param numeroChaves
 * @return void
 * @see leituraBarrasLinhasTrafos
 * @note
 * @warning Como se trata de uma função auxiliar essa não deve ser chamada diretamente por outras partes do programa.
 */

void leituraLinhas(FILE *arquivo, GRAFO *grafoSDRParam, long int *numeroChaves)
{
    char blocoLeitura[1000]; //Variável para leitura do bloco de caracteres do arquivo
    long int barraDe, barraPara; //Variáveis para ler o par barra de origem e barra de destino
    char identificadorAresta[20]; //Variável para ler o indentificador da aresta
    int dadosCondicao; //Variável para ler a condição operacional da aresta (linha)
    int estadoChave; //Variável para ler o estado das linhas (chave aberta, chave fechada ou outros)
    int dadosTipoAresta; //Variável para ler os dados do tipo da aresta
    int numeroArestas; //Variável para ler o número de arestas do sistema
    int dadosSubTipoAresta; //Variável para ler o subtipo das arestas
    int contador; //Variável contador 
    double resistencia; //Variável para leitura da resistência da linha
    double reatancia; //Variável para leitura da reatância da linha
    double ampacidade; //Variável para leitura da ampacidade da linha
    numeroChaves[0] =0;
    //Leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura, 1000, arquivo);
    //Leitura do número de arestas da string blocoLeitura
    sscanf(blocoLeitura, "%d",&numeroArestas);
    //Leitura de cada linha do arquivo relativo a cada uma das arestas do SDR
    for(contador = 0; contador<numeroArestas; contador++)
    {
         //Leitura do bloco de caracteres do arquivo
        fgets(blocoLeitura, 1000, arquivo);
         //Leitura de cad um dos dados das arestas do SDR a partir da string blocoLeitura
        sscanf(blocoLeitura, "%ld %ld %s %d %d %d %d %lf %lf %lf",&barraDe, &barraPara, identificadorAresta, &dadosTipoAresta, &estadoChave, &dadosCondicao, &dadosSubTipoAresta, &resistencia, &reatancia, &ampacidade);
        //Insere os dados das linhas no formato da lista de adjacências do grafo
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].idNo = barraPara;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].condicao = dadosCondicao;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].estadoChave = estadoChave;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].tipoAresta =  dadosTipoAresta;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].subTipoAresta = dadosSubTipoAresta;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].resistencia = resistencia;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].reatancia = reatancia;
        grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].ampacidade = ampacidade;
        // como o parâmetro identificador da Aresta é uma string para inserir na lista de adjacencias utiliza o comando de cópia de strings
        strcpy(grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].idAresta, identificadorAresta);
        //faz a contagem do número de chaves do SDR
        if (grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].tipoAresta == chaveManual || grafoSDRParam[barraDe].adjacentes[grafoSDRParam[barraDe].numeroAdjacentes].tipoAresta == chaveAutomatica)
            numeroChaves[0]++;
        //Atualiza o número de adjacentes da barra
        grafoSDRParam[barraDe].numeroAdjacentes++;
        //Processo similar ao anterior só inverte as barras De e Para
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].idNo = barraDe;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].condicao = dadosCondicao;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].estadoChave = estadoChave;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].tipoAresta =  dadosTipoAresta;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].subTipoAresta = dadosSubTipoAresta;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].resistencia = resistencia;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].reatancia = reatancia;
        grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].ampacidade = ampacidade;
        strcpy(grafoSDRParam[barraPara].adjacentes[grafoSDRParam[barraPara].numeroAdjacentes].idAresta, identificadorAresta);
        grafoSDRParam[barraPara].numeroAdjacentes++;
    }
}

/**
 * @brief Função para a leitura do arquivo dadosTrafo.dad, relativo as informações dos trafos. 
 *
 * Essa função realiza a leitura dos dados relativos aos trafos do sistema. Nessa função é alocada a estrutura que armazena os dados dos trafos.
 * Para isso ao função recebe como parâmetro de entrada e saída e um ponteiro para o ponteiro do tipo DADOSTRAFO, @p **dadosTrafoSDRParam .
 * A função retorna @c void.
 * 
 * @param dadosTrafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do DADOSTRAFO, onde os dados do trafo são retornados.
 * @param numeroTrafos inteiro contendo o número total de trafos do SDR.
 * @return void
  * @note
 */

void leituraTrafos(DADOSTRAFO **dadosTrafoSDRParam, long int *numeroTrafos)
{
    char blocoLeitura[300];  //Variável para leitura do bloco de caracteres do arquivo
    long int tamTrafos; //Variável para ler o número de trafos do SDR
    long int contador; //Variável contador para controlar a leitura dos dados de trafos
    int identificadorTrafo; //Variável para o identificador do trafo
    int identificadorSubEstacao; //Variável para o identificador da subestação
    double capacidade; //Variável para leitura da capacidade do trafo
    double tensaoReal; //Variável para leitura da tensão real do trafo
    double impedancia;
    FILE *arquivo;
    arquivo = fopen("dadosTrafo.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo dadosTrafo.dad");
        exit(1);
    }
     //Leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura, 300, arquivo);
     //Leitura do número de trafos do SDR
    sscanf(blocoLeitura, "%ld",&tamTrafos);
    //Atribui o número de trafos lido do arquivo para a variável global numeroTrafos
    numeroTrafos[0] = tamTrafos;
    //Aloca a memória para armazenar os dados dos trafos do SDR e verifica se o processo foi concluído com sucesso. 
    //Caso não seja possível alocar a memória encerra o programa.
    if (((*dadosTrafoSDRParam) = (DADOSTRAFO *)malloc( (tamTrafos+1) * sizeof(DADOSTRAFO)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para os dados de trafos do SDR !!!!");
        exit(1); 
    }

    //Leitura dos dados de cada trafo
    for(contador = 1; contador <= numeroTrafos[0]; contador++)
    {
         //Leitura do bloco de caracteres do arquivo
        fgets(blocoLeitura, 300, arquivo);
         //Leitura de cada informação dos dados dos trafos na string blocoLeitura
        sscanf(blocoLeitura,"%d %d %lf %lf %lf", &identificadorTrafo, &identificadorSubEstacao, &capacidade, &tensaoReal, &impedancia);
        //Inserção na estrutura de dados dos trafos das informações lidas.
        (*dadosTrafoSDRParam)[contador].idTrafo = identificadorTrafo;
        (*dadosTrafoSDRParam)[contador].idSubEstacao = identificadorSubEstacao;
        (*dadosTrafoSDRParam)[contador].capacidade = capacidade;
        (*dadosTrafoSDRParam)[contador].tensaoReal = tensaoReal;
        (*dadosTrafoSDRParam)[contador].impedancia = (impedancia/100)*(tensaoReal*tensaoReal)/capacidade;
    }
}

/**
 * @brief Função para a leitura do arquivo dadosTrafoTrifasicos.dad, relativo as informações trifasicas dos trafos. 
 *
 * Essa função realiza a leitura dos dados trifasicos relativos aos trafos do sistema. Nessa função é alocada a estrutura que armazena os dados dos trafos.
 * Para isso ao função recebe como parâmetro de entrada e saída e um ponteiro para o ponteiro do tipo DADOSTRAFO, @p **dadosTrafoSDRParam .
 * A função retorna @c void.
 * 
 * @param dadosTrafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do DADOSTRAFO, onde os dados do trafo são retornados.
 * @param numeroTrafos inteiro contendo o número total de trafos do SDR.
 * @return void
  * @note
 */

void leituraTrafosTrifasicos(DADOSTRAFO **dadosTrafoSDRParam, long int *numeroTrafos)
{
    char blocoLeitura[300];  //Variável para leitura do bloco de caracteres do arquivo
    long int tamTrafos; //Variável para ler o número de trafos do SDR
    long int contador; //Variável contador para controlar a leitura dos dados de trafos
    int identificadorTrafo; //Variável para o identificador do trafo
    int identificadorSubEstacao; //Variável para o identificador da subestação
    char tipoTrafo[10];
    FILE *arquivo;
    arquivo = fopen("dadosTrafosTrifasicos.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo dadosTrafosTrifasicos.dad");
        exit(1);
    }
    //Leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura, 300, arquivo);
     //Leitura do número de trafos do SDR
    sscanf(blocoLeitura, "%ld",&tamTrafos);
    //Leitura dos dados trifasicos de cada trafo
    for(contador = 1; contador <= numeroTrafos[0]; contador++)
    {
         //Leitura do bloco de caracteres do arquivo
        fgets(blocoLeitura, 300, arquivo);
         //Leitura de cada informação dos dados dos trafos na string blocoLeitura
        sscanf(blocoLeitura,"%d %d %s", &identificadorTrafo, &identificadorSubEstacao, tipoTrafo);
        //Inserção na estrutura de dados dos trafos das informações lidas.
        (*dadosTrafoSDRParam)[contador].idTrafo = identificadorTrafo;
        (*dadosTrafoSDRParam)[contador].idSubEstacao = identificadorSubEstacao;

        TIPOSTRAFOS enumTipo;
        //Define o TIPOTRAFO da barra baseado na string lida no arquivo
        if     (strcmp(tipoTrafo, "YYat"    ) == 0)   enumTipo = YYat;
        else if(strcmp(tipoTrafo, "YatY"    ) == 0)   enumTipo = YatY;
        else if(strcmp(tipoTrafo, "YatYat"  ) == 0)   enumTipo = YatYat;
        else if(strcmp(tipoTrafo, "YY"      ) == 0)   enumTipo = YY;
        else if(strcmp(tipoTrafo, "DD"      ) == 0)   enumTipo = DD;
        else if(strcmp(tipoTrafo, "DY"      ) == 0)   enumTipo = DY;
        else if(strcmp(tipoTrafo, "DYat"    ) == 0)   enumTipo = DYat;
        else if(strcmp(tipoTrafo, "YatD"    ) == 0)   enumTipo = YatD;
        else if(strcmp(tipoTrafo, "YD"      ) == 0)   enumTipo = YD;

        (*dadosTrafoSDRParam)[contador].tipoTransformador = enumTipo;
    }
}


/**
 * @brief Função para imprimir os dados lidos sobre o SDR representado na forma de um grafo.
 *
 * Essa função recebe como parâmetro uma variavél do tipo GRAFO e realiza a impressão
 * na tela dos principais dados contidos na estrutura informada. É utilizada para validar o processo executado pela função de leitura.
 
 * Exemplo:
 * @code
 * GRAFO *grafoExemplo;
 * DADOSTRAFO *trafoExemplo;
 * BOOL leitura = leituraBarrasLinhasTrafos( &grafoExemplo, &trafoExemplo);
 * imprimeGrafo(grafoExemplo);
 * @endcode
 * 
 * @param grafoSDRParam
 * @param numeroBarras
 */

void imprimeGrafo(GRAFO *grafoSDRParam, long int numeroBarras)
{
    int contador, contadorAdj; //Variáveis contadores para imprimir os dados do grafo do SDR
    
    //Loop para percorrer cada barra do SDR
    for(contador =1; contador <= numeroBarras; contador++)
    {
        //Imprime os dados das barras
        printf ("BARRA ID: %ld \t", grafoSDRParam[contador].idNo);
        printf ("\t TIPO DA BARRA: %d \t",grafoSDRParam[contador].tipoNo);
        printf ("\t SETOR DA BARRA: %ld \n",grafoSDRParam[contador].idSetor);
        printf ("\t P Q \n");
        printf("\t");
        printf("%.2lf  %.2lf", grafoSDRParam[contador].valorPQ.p, grafoSDRParam[contador].valorPQ.q);

        printf("\n");
        printf("\t Dados para Priorizacoes: \n");
        printf("\t \t EUSD Grupo A:                   %.2lf\n", grafoSDRParam[contador].priorizacoes.eusdGrupoA);
        printf("\t \t EUSD Grupo B:                   %.2lf\n", grafoSDRParam[contador].priorizacoes.eusdGrupoB);
        printf("\t \t Numero Consumidores:           %10d\n", grafoSDRParam[contador].priorizacoes.qtdConsumidores);
        printf("\t \t Numero Consumidores Especiais: %10d\n", grafoSDRParam[contador].priorizacoes.qtdConsumidoresEspeciais);
        printf("\n");
       /* printf("\t Dados Compensacoes: \n");
        printf("\t \t Meta DIC Mensal:          %.2lf\n", grafoSDRParam[contador].compensacoes.metaDicMensal);
        printf("\t \t Meta DIC Trimestral:      %.2lf\n", grafoSDRParam[contador].compensacoes.metaDicTrimestral);
        printf("\t \t Meta DIC Anual:           %.2lf\n", grafoSDRParam[contador].compensacoes.metaDicAnual);
        printf("\t \t Meta DMIC Mensal:         %.2lf\n", grafoSDRParam[contador].compensacoes.metaDmicMensal);
        printf("\t \t Meta FIC Mensal:          %.2lf\n", grafoSDRParam[contador].compensacoes.metaFicMensal);
        printf("\t \t Meta FIC Trimestral:      %.2lf\n", grafoSDRParam[contador].compensacoes.metaFicTrimestral);
        printf("\t \t Meta FIC Anual:           %.2lf\n", grafoSDRParam[contador].compensacoes.metaFicAnual);
        printf("\t \t Acumulado DIC Mensal:     %.2lf\n", grafoSDRParam[contador].compensacoes.resulDicAcumMes);
        printf("\t \t Acumulado DIC Trimestral: %.2lf\n", grafoSDRParam[contador].compensacoes.resulDicAcumTrimestre);
        printf("\t \t Acumulado DIC Anual:      %.2lf\n", grafoSDRParam[contador].compensacoes.resulDicAcumAno);
        printf("\t \t Acumulado DMIC Mensal:    %.2lf\n", grafoSDRParam[contador].compensacoes.resulDmicMes);
        printf("\t \t Acumulado FIC Mensal:     %.2lf\n", grafoSDRParam[contador].compensacoes.resulFicAcumMes);
        printf("\t \t Acumulado FIC Trimestral: %.2lf\n", grafoSDRParam[contador].compensacoes.resulFicAcumTrimestre);
        printf("\t \t Acumulado FIC Anual:      %.2lf\n", grafoSDRParam[contador].compensacoes.resulFicAcumAno);
        printf("\t Numero Adjacentes: %d \n", grafoSDRParam[contador].numeroAdjacentes);
        printf("\t");*/
        //Imprime cada barra da lista de adjacentes
        for(contadorAdj = 0; contadorAdj<grafoSDRParam[contador].numeroAdjacentes; contadorAdj++)
        {
            printf("\t Barra Adjacente %ld Estado chave %d", grafoSDRParam[contador].adjacentes[contadorAdj].idNo,grafoSDRParam[contador].adjacentes[contadorAdj].estadoChave);
        }
        printf("\n");
    }
    

}

/**
 * @brief Função para imprimir os dados lidos sobre os trafos do SDR.
 *
 * Essa função recebe como parâmetro uma variavél do tipo DADOSTRAFO e realiza a impressão
 * na tela dos dados contidos na estrutura  informada. É utilizada para validar o processo executado pela função de leitura.
 
 * Exemplo:
 * @code
 * GRAFO *grafoExemplo;
 * DADOSTRAFO *trafoExemplo;
 * BOOL leitura = leituraBarrasLinhasTrafos( &grafoExemplo, &trafoExemplo);
 * imprimeTrafos(trafoExemplo);
 * @endcode
 * 
 * @param dadosTrafoSDRParam é um ponteiro da estrutura do tipo do DADOSTRAFO.
 * @param numeroTrafos
 */
void imprimeTrafos(DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos)
{
    int contador;
    //Loop para imprimir os dados de cada um dos trafos
    for(contador = 1; contador<=numeroTrafos; contador++)
    {
        printf("Identificador Trafo %d \n", dadosTrafoSDRParam[contador].idTrafo);
        printf("\t Identificador SubEstacao %d \n", dadosTrafoSDRParam[contador].idSubEstacao);
        printf("\t Capacidade %lf \n",dadosTrafoSDRParam[contador].capacidade);
        printf("\t Tensao Real %lf \n",__real__ dadosTrafoSDRParam[contador].tensaoReal);
    }    

}

/**
 * @brief Função para imprimir os dados lidos sobre os trafos do SDR.
 *
 * Essa função recebe como parâmetro uma variavél do tipo DADOSREGULADOR e realiza a impressão
 * na tela dos dados contidos na estrutura  informada. É utilizada para validar o processo executado pela função de leitura.
 * 
 * @param dadosReguladorParam
 * @param numeroReguladoresParam
 */
void imprimeReguladores(DADOSREGULADOR *dadosReguladorParam, long int numeroReguladoresParam)
{
    int contador;
    //Loop para imprimir os dados de cada um dos trafos
    for(contador = 0; contador<numeroReguladoresParam; contador++)
    {
        printf("Identificador Regulador %s \n", dadosReguladorParam[contador].idRegulador);
        printf("\t Ampacidade %lf \n", dadosReguladorParam[contador].ampacidade);
        printf("\t Taps %d \n",dadosReguladorParam[contador].numeroTaps);
        printf("\t Tipo %d \n",dadosReguladorParam[contador].tipoRegulador);
    }    

}

/**
 * @brief Função principal para a leitura do arquivo dados_alimentadores.dad.
 *
 * Essa função realiza a leitura do arquivo dados_alimentadores.dad. A função assume que o nome do arquivo é padrão da forma 
 * dados_alimentadores.dad. O arquivo inicia com o marcador DALIM que é verificado pela função.
 * Localizado o marcador a função dá procedimento a leitura dos dados do arquivo armazenando-os na estrutura de dados definida.
 * Tem como parâmentro de entrada e saída um ponteiro para ponteiro da estrutura do tipo DADOS ALIMNETADORES @p **dadosAlimentadoresSDRParam.
 * A função retorna @c BOOL indicando se a leitura foi bem sucedida.
 * Para utilizar a função:
 * @code
 * DADOSALIMENTADORES *alimentadorExemplo;
 * BOOL leitura = leituraDadosAlimentadores( &alimentadorExemplo);
 * if (leitura)
 *      printf("leitura concluida\n");
 * @endcode
 * 
 * @param dadosAlimentadorSDRParam é um ponteiro para o ponteiro da estrutura do tipo do DADOSALIMENTADORES, onde é retornado os dados dos alimentadores.
 * @return Retorna o tipo booleano definido indicando que a leitura foi bem sucedida.
 */
BOOL leituraDadosAlimentadores(DADOSALIMENTADOR **dadosAlimentadorSDRParam)
{
    FILE *arquivo;  //Variável ponteiro do tipo FILE para ler e manipular o arquivo.
    char blocoLeitura[100]; //Variável para realizar a leitura do bloco de caracteres do arquivo.
    char marcador[10]; //Variável para leitura e verificação do marcador de formato do arquivo.
    long int contador; //Variável contador para realizar a leitura das linhas de dados do arquivo.
    long int numAlim; // Variável para leitura do número de dados de alimentadores contidos no arquivo.
    char idAlimentador[15]; //Variável para leitura do identificador do alimentador.
    long int idBarraAlimentador; //Variável para leitura do identificador da barra do alimentador.
    int idTrafoAlimentador; //Variável para leitura do identificador do trafo onde o alimentador está ligado.
    
    //Abertura e associação do arquivo dados_alimentadores.dad á variáve arquivo
    arquivo = fopen("dados_alimentadores.dad","r");
    //Verifica se foi possível abrir o arquivo e encerra o programa se ocorreu um erro.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo dados_alimentadores.dad !!!\n");
            exit(1);
    }
    
    //Leitura do bloco de dados do arquivo
    fgets(blocoLeitura,100,arquivo);
    
    //Leitura a partir da string blocoLeitura do marcador do arquivo
    sscanf(blocoLeitura,"%s",marcador);
    
    //Verifica se o arquivo contém o marcador especificado
    if (!strcmp(marcador, "DALIM"))
    {
       //Leitura do bloco de dados do arquivo 
       fgets(blocoLeitura,100,arquivo);
       //Leitura a parti da variável blocoLeitura do número de alimentadores
       sscanf(blocoLeitura,"%ld",&numAlim);
       //Atribui o número de alimentadores a variável global
       numeroAlimentadores = numAlim;
       //printf("numeroAlimentadores %ld \n", numAlim);
       //Aloca a memória para a estrutura de dados dos alimentadores e verifica se a alocação ocorreu com sucesso. Caso contrário o programa é encerrado.
       if (((*dadosAlimentadorSDRParam) = (DADOSALIMENTADOR *)malloc( (numAlim+1) * sizeof(DADOSALIMENTADOR)))==NULL)
        {
            printf("Erro -- Nao foi possivel alocar espaco de memoria para os dados dos alimentadores do SDR !!!!");
            exit(1); 
        }
       //Loop para leitura dos dados dos alimentadores
       for(contador=1; contador<=numAlim; contador++)
       {
           //Leitura do bloco de dados do arquivo
           fgets(blocoLeitura, 100, arquivo);
           //Leitura a partir da string blocoLeitura dos dados dos alimentadores
           sscanf(blocoLeitura,"%s %ld %d", idAlimentador,&idBarraAlimentador,&idTrafoAlimentador);
           
           //Insere os dados dos alimentadores na estrutura
           (*dadosAlimentadorSDRParam)[contador].barraAlimentador = idBarraAlimentador;
           (*dadosAlimentadorSDRParam)[contador].idTrafo = idTrafoAlimentador;
           strcpy((*dadosAlimentadorSDRParam)[contador].idAlimentador, idAlimentador);
       }
    }
    else
    {
        printf("Erro ao ler arquivo dados_alimentadores.dad !!! \n Formato Invalido!!!\n");
        exit(1);
    }
    
    //fecha o arquivo lido.
    fclose(arquivo);
    return(true);
}

/**
 * @brief Função principal para a leitura do arquivo dados_reguladores_tensao.dad.
 *
 * Essa função realiza a leitura do arquivo dados_reguladores_tensao.dad. A função assume que o nome do arquivo é padrão da forma 
 * dados_reguladores_tensao.dad. O arquivo inicia com o marcador DBRDT que é verificado pela função.
 * Localizado o marcador a função dá procedimento a leitura dos dados do arquivo armazenando-os na estrutura de dados definida.
 * Tem como parâmentro de entrada e saída um ponteiro para ponteiro da estrutura do tipo DADOSREGULADOR @p **dadosReguladoresSDRParam.
 * A função retorna @c BOOL indicando se a leitura foi bem sucedida.
 * Para utilizar a função:
 * @code
 * DADOSREGULADOR *reguladorExemplo;
 * BOOL leitura = leituraDadosReguladoresTensao( &reguladorExemplo);
 * if (leitura)
 *      printf("leitura concluida\n");
 * @endcode
 * 
 * @param dadosReguladorSDRParam é um ponteiro para o ponteiro da estrutura do tipo do DADOSREGULADOR, onde é retornado os dados dos reguladores de tensão.
 * @return Retorna o tipo booleano definido indicando que a leitura foi bem sucedida.
 */
BOOL leituraDadosReguladoresTensao(DADOSREGULADOR **dadosReguladorSDRParam)
{
    FILE *arquivo; //Variável ponteiro do tipo FILE para ler e manipular o arquivo.
    char blocoLeitura[100]; //Variável para realizar a leitura do bloco de caracteres do arquivo.
    char marcador[10]; //Variável para leitura e verificação do marcador de formato do arquivo.
    int contador; //Variável contador para controlar a leitura do arquivo
    long int numRegulador; //Variável para armazenar o número de reguladores de tensão
    char idRegulador[30]; //Variável para o identificador do regulador de tensão
    int tipoRegulador; //Variável para fazer a leitura do tipo do regulador
    double ampacidade; //Variável para fazer a leitura da ampacidade
    int numeroTaps; //Variável para fazer a leitura da correção do regulador
    
    //Abertura do arquivo dados_reguladores_tensao.dad e associação a variável arquivo
    arquivo = fopen("dados_reguladores_tensao.dad","r");
    //Verifica se a abertura do arquivo ocorreu com sucesso. Caso contrário encerra o programa.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo dados_reguladores_tensao.dad !!!\n");
            exit(1);
    }
    
    //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do marcador 
    sscanf(blocoLeitura,"%s",marcador);
    //verifica se o conteúdo da variável marcador corresponde ao marcador do arquivo. Caso contrário o programa é encerrado
    if (!strcmp(marcador, "DBRT"))
    {
       //leitura do bloco de caracteres do arquivo
       fgets(blocoLeitura,100,arquivo);
       //leitura do numero de reguladores da string blocoLeitura
       sscanf(blocoLeitura,"%ld",&numRegulador);
       //Atribui o número de reguladores para a variável global
       numeroReguladores = numRegulador;
       //Aloca a memória para armazenar os dados dos reguladores de tensão e verifica se a alocação foi vem sucedida.
       if (((*dadosReguladorSDRParam) = (DADOSREGULADOR *)malloc( numRegulador * sizeof(DADOSREGULADOR)))==NULL)
        {
            printf("Erro -- Nao foi possivel alocar espaco de memoria para os dados dos alimentadores do SDR !!!!");
            exit(1); 
        }
       //Leitura dos dados dos alimentadores
       for(contador=0; contador<numRegulador; contador++)
       { 
           //leitura do bloco de caracteres do arquivo
           fgets(blocoLeitura, 100, arquivo);
           sscanf(blocoLeitura, "%s %d %lf %d", idRegulador,&tipoRegulador,&ampacidade,&numeroTaps); 
           //Atribui os dados lidas para a estrutura com os dados dos reguladores de tensão
           strcpy((*dadosReguladorSDRParam)[contador].idRegulador, idRegulador);
           (*dadosReguladorSDRParam)[contador].tipoRegulador = tipoRegulador;
           (*dadosReguladorSDRParam)[contador].ampacidade = ampacidade;
           (*dadosReguladorSDRParam)[contador].numeroTaps = numeroTaps;
       }
    }
    else
    {
        printf("Erro ao ler arquivo dados_reguladores_tensao.dad !!! \n Formato Invalido!!!\n");
        exit(1);
    }
    
    //fecha o arquivo
    fclose(arquivo);
    return(true);
}

/**
 *
 * Essa função realiza a leitura do arquivo parametros.dad. A função assume que o nome do arquivo é padrão da forma 
 * parametros.dad. O arquivo deve conter os parâmetros necessário o para a execução da metodologia
 * 
 * @param numeroGeracoes ponteiro para inteiro onde será retornado o número máximo de avalições do algoritmo evolutivo
 * @param tamanhoTabelas ponteiro para um vetor de inteiros com a quantidade de individuos para cada subpopulacao
 * @param SBase inteiro onde será retornada a únidade para calculo das cargas
 * @param tamanhoPopulacaoInicial ponteiro para inteiro onde será retornado a quantidade de soluções que devem ser geradas antes do algoritmo evolutifo 
 */
void leituraParametros(long int *numeroGeracoes, int *tamanhoTabelas, int *SBase, int *tamanhoPopulacaoInicial)
{
    
    FILE *arquivo; //Variável ponteiro do tipo FILE para ler e manipular o arquivo.
    char blocoLeitura[100]; //Variável para realizar a leitura do bloco de caracteres do arquivo.
    //Abertura do arquivo dados_reguladores_tensao.dad e associação a variável arquivo
    arquivo = fopen("parametros.dad","r");
    //Verifica se a abertura do arquivo ocorreu com sucesso. Caso contrário encerra o programa.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo parametros.dad !!!\n");
            exit(1);
    }
    
    //leitura do bloco de caracteres do arquivo
   // fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de gerações 
   // sscanf(blocoLeitura,"%d",&seed[0]);
    
    //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de gerações 
    sscanf(blocoLeitura,"%ld",&numeroGeracoes[0]);
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do tamanho da população inicial 
    sscanf(blocoLeitura,"%d",&tamanhoPopulacaoInicial[0]);
    //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do tamanho de cada tabela 
    sscanf(blocoLeitura,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&tamanhoTabelas[0], &tamanhoTabelas[1],&tamanhoTabelas[2],&tamanhoTabelas[3],&tamanhoTabelas[4],&tamanhoTabelas[5], &tamanhoTabelas[6], &tamanhoTabelas[7], &tamanhoTabelas[8], &tamanhoTabelas[9], &tamanhoTabelas[10], &tamanhoTabelas[11], &tamanhoTabelas[12], &tamanhoTabelas[13], &tamanhoTabelas[14], &tamanhoTabelas[15], &tamanhoTabelas[16], &tamanhoTabelas[17], &tamanhoTabelas[18], &tamanhoTabelas[19], &tamanhoTabelas[20], &tamanhoTabelas[21], &tamanhoTabelas[22], &tamanhoTabelas[23], &tamanhoTabelas[24], &tamanhoTabelas[25], &tamanhoTabelas[26], &tamanhoTabelas[27], &tamanhoTabelas[28]);
   //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de gerações 
    sscanf(blocoLeitura,"%d",&SBase[0]);
    
    
}

/**
 * Por Leandro: consiste na função "leituraParametros()" modificada para que
 * 1) seja lida a sequência desejada de operação das chaves em pares, que transferem cargas entre alimentadores
 * 	Se 01, então é Abre-Fecha
 * 	Se 10, então é Fecha-Abre
 *
 * 2) seja lido o número desejado de soluções a serem selecionadas na fronteira de Pareto obtida pelo AEMO, ao término do processo evolutivo
 *
 * 3) para que deixe de ser lido o "tamanhoPopulacaoInicial", pois tal informação deixou de ser usada.
 *
 * 4) passe a ser lido um novo parâmentro de entrada: "maximoGeracoesSemAtualizacaoPopulacao". Ele informa o número de máximo de gerações
 * permito sem que haja atualização da população. Caso este número seja atingido antes da execução de todas de todas as geraçẽos, o processo
 * evolutivo será encerrado.
 *
 * Essa função realiza a leitura do arquivo parametros.dad. A função assume que o nome do arquivo é padrão da forma
 * parametros.dad. O arquivo deve conter os parâmetros necessário o para a execução da metodologia
 *
 * @param numeroGeracoes ponteiro para inteiro onde será retornado o número máximo de avalições do algoritmo evolutivo
 * @param tamanhoTabelas ponteiro para um vetor de inteiros com a quantidade de individuos para cada subpopulacao
 * @param SBase inteiro onde será retornada a únidade para calculo das cargas
 * @param tamanhoPopulacaoInicial ponteiro para inteiro onde será retornado a quantidade de soluções que devem ser geradas antes do algoritmo evolutifo
 * @param sequenciaManobrasTranferenciaCargaParam ponteiro para a variável que salva a sequência desejada para operação das manobras que ocorrem em pares
 * @param numeroDesejadoSolucoesFinaisParam ponteiro para o inteiro que armazena o número de soluções que se dejesa informar após a execução do processo evolutivo, as quais serão selecionada na Fronteira de Pareto obtida
 *
 */
void leituraParametrosModificada(long int *numeroGeracoes, int *tamanhoTabelas, int *SBase, long int *maximoGeracoesSemAtualizacaoPopulacao, SEQUENCIAMANOBRASALIVIO *sequenciaManobrasTranferenciaCargaParam, int *numeroDesejadoSolucoesFinaisParam)
{

    FILE *arquivo; //Variável ponteiro do tipo FILE para ler e manipular o arquivo.
    char blocoLeitura[100]; //Variável para realizar a leitura do bloco de caracteres do arquivo.
    int sequencia;
    //Abertura do arquivo dados_reguladores_tensao.dad e associação a variável arquivo
    arquivo = fopen("parametros.dad","r");
    //Verifica se a abertura do arquivo ocorreu com sucesso. Caso contrário encerra o programa.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo parametros.dad !!!\n");
            exit(1);
    }

    //leitura do bloco de caracteres do arquivo
   // fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de gerações
   // sscanf(blocoLeitura,"%d",&seed[0]);

    //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de gerações
    sscanf(blocoLeitura,"%ld",&numeroGeracoes[0]);
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do tamanho da população inicial
    sscanf(blocoLeitura,"%ld",&maximoGeracoesSemAtualizacaoPopulacao[0]);
    //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do tamanho de cada tabela
    sscanf(blocoLeitura,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&tamanhoTabelas[0], &tamanhoTabelas[1],&tamanhoTabelas[2],&tamanhoTabelas[3],&tamanhoTabelas[4],&tamanhoTabelas[5], &tamanhoTabelas[6], &tamanhoTabelas[7], &tamanhoTabelas[8], &tamanhoTabelas[9], &tamanhoTabelas[10], &tamanhoTabelas[11], &tamanhoTabelas[12], &tamanhoTabelas[13], &tamanhoTabelas[14], &tamanhoTabelas[15], &tamanhoTabelas[16], &tamanhoTabelas[17], &tamanhoTabelas[18], &tamanhoTabelas[19], &tamanhoTabelas[20], &tamanhoTabelas[21], &tamanhoTabelas[22], &tamanhoTabelas[23], &tamanhoTabelas[24], &tamanhoTabelas[25]);
   //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de gerações
    sscanf(blocoLeitura,"%d",&SBase[0]);
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura da sequência para operação das manobras em pares
    sscanf(blocoLeitura,"%d", &sequencia);
    if(sequencia == 01)
    	sequenciaManobrasTranferenciaCargaParam[0] = abreFecha;
    else{
        if(sequencia == 10)
        	sequenciaManobrasTranferenciaCargaParam[0] = fechaAbre;
        else{
            printf("ATENÇÃO\n O parâmetro que informa a sequência na qual deseja-se operar as manobras para transferência de carga aceita SOMENTE os seguintes valores:\n 01 para Abre-Fecha\n 10 para Fecha-Abre\n");
            exit(1);
        }
    }
    //leitura da string blocoLeitura do numero de soluções finais desejado.
    fgets(blocoLeitura,100,arquivo);
    sscanf(blocoLeitura,"%d",&numeroDesejadoSolucoesFinaisParam[0]);
}
/**
 * Essa função realiza a leitura do arquivo dadosEntrada.dad. A função assume que o nome do arquivo é padrão da forma 
 * dadosEntrada.dad. O arquivo inicia com a quantidade de setores em falta.
 * A seguir o identificador de cada setor é listado, sendo um setor por linha do arquivo.  
 * @param numeroSetorFalta ponteiro para inteiro onde será retornado a quantidade de setores em falta.
 * @param setoresFalta é um ponteiro para o ponteirdo do vetor de inteiros com os identificadores dos setores em falta.
 */
void leituraDadosEntrada(int *numeroSetorFalta, long int **setoresFalta)
{
    
    FILE *arquivo; //Variável ponteiro do tipo FILE para ler e manipular o arquivo.
    char blocoLeitura[100]; //Variável para realizar a leitura do bloco de caracteres do arquivo.
    int contador;
    long int setor;
    
    //Abertura do arquivo dados_reguladores_tensao.dad e associação a variável arquivo
    arquivo = fopen("dadosEntrada.dad","r");
    //Verifica se a abertura do arquivo ocorreu com sucesso. Caso contrário encerra o programa.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo dadoEntrada.dad !!!\n");
            exit(1);
    }

 //leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do numero de setores em falta 
    sscanf(blocoLeitura,"%d",&numeroSetorFalta[0]);
    if (((*setoresFalta) = (long int *)malloc( (numeroSetorFalta[0]+1) * sizeof(long int)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para o grafo do SDR !!!!");
        exit(1); 
    }
    for(contador =0; contador < numeroSetorFalta[0]; contador++)
    {
        //leitura do bloco de caracteres do arquivo
        fgets(blocoLeitura,100,arquivo);
        //leitura da string blocoLeitura do numero de gerações 
        sscanf(blocoLeitura,"%ld",&setor);
        (*setoresFalta)[contador] = setor;
    }
}

void leituraIndividuoInicial(long int numeroSetores, long int numeroAlimentadores, CONFIGURACAO *individuo)
{
    
    long int auxiliar[numeroSetores];
    FILE *arquivo;
    long int contaSetores, contador;
    int indice;
    char buffer[5000];
    char *data;
    int tamanho;
    arquivo = fopen("individuo0.dad","r+" );
    if (arquivo != NULL) {
        fgets(buffer, 5000, arquivo);
        for (indice = 0; indice < numeroAlimentadores; indice++) {
            fgets(buffer, 5000, arquivo);
            data = buffer;
            contaSetores = 0;
            while (1 == sscanf(data, " %ld%n", &auxiliar[contaSetores], &tamanho)) {
                contaSetores++;
                data += tamanho;
                //printf("read: %5d; sum = %5d; offset = %5d\n", n, sum, offset);
            }
            alocaRNP(contaSetores, &individuo->rnp[indice]);
            for(contador =0; contador < contaSetores; contador++)
            {
                individuo->rnp[indice].nos[contador].idNo = auxiliar[contador];
            }
            fgets(buffer, 5000, arquivo);
            data = buffer;
            contaSetores = 0;
            while (1 == sscanf(data, " %ld%n", &auxiliar[contaSetores], &tamanho)) {
                contaSetores++;
                data += tamanho;
                //printf("read: %5d; sum = %5d; offset = %5d\n", n, sum, offset);
            }
            
            for(contador =0; contador < contaSetores; contador++)
            {
                individuo->rnp[indice].nos[contador].profundidade = auxiliar[contador];
            }
            fgets(buffer, 5000, arquivo);
        }
    }
    else
    {
        printf("Nao foi possivel ler individuo inicial");
        exit(1);
    }
}

void leituraListaAdjacenciaSetores(GRAFOSETORES **grafoSetoresParam, long int *numeroSetores)
{
    FILE *arquivo;
    long int contadorAdj, contador;
    int adjacentes;
    long int setor, consumidores, consumidoresEspeciais, auxiliar;
    char buffer[5000];
    char *data;
    int tamanho;
    arquivo = fopen("grafosetores.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo grafosetores.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "numero	setores	%ld", numeroSetores);
    
    if (((*grafoSetoresParam) = (GRAFOSETORES *)malloc((numeroSetores[0]+1)*sizeof (GRAFOSETORES)))==NULL)
    {
        printf("ERRO: alocacacao grafo da lista de adjacencias de setores!!!!");
        exit(1);
    }        
    for(contador = 1; contador<=numeroSetores[0]; contador++)
    {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "Setor %ld numero adjacentes %d\n",&setor, &adjacentes);
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "Consumidores: %ld Consumidores Especiais: %ld",&consumidores, &consumidoresEspeciais);
        (*grafoSetoresParam)[setor].idSetor = setor;
        (*grafoSetoresParam)[setor].setorFalta = false;
        (*grafoSetoresParam)[setor].numeroAdjacentes = adjacentes;
        (*grafoSetoresParam)[setor].numeroConsumidores = consumidores;
        (*grafoSetoresParam)[setor].numeroConsumidoresEspeciais = consumidoresEspeciais;
        (*grafoSetoresParam)[setor].setoresAdjacentes = Malloc(long int, adjacentes);
        (*grafoSetoresParam)[setor].idChavesAdjacentes = Malloc(long int, adjacentes);
        
        fgets(buffer, 5000, arquivo);
        data = buffer;
        sscanf(data,"Adjacentes %ld%n",&auxiliar,&tamanho);
        for(contadorAdj = 0; contadorAdj<adjacentes; contadorAdj++)
        {
            (*grafoSetoresParam)[setor].setoresAdjacentes[contadorAdj]= auxiliar;
            data+=tamanho;
            sscanf(data,"%ld%n",&auxiliar,&tamanho);
        }
        
        fgets(buffer, 5000, arquivo);
        data = buffer;
        sscanf(data,"Chaves %ld%n",&auxiliar,&tamanho);
        for(contadorAdj = 0; contadorAdj<adjacentes; contadorAdj++)
        {
            (*grafoSetoresParam)[setor].idChavesAdjacentes[contadorAdj]= auxiliar;
            data+=tamanho;
            sscanf(data,"%ld%n",&auxiliar,&tamanho);
        }
        fgets(buffer, 5000, arquivo);
    }
    fclose(arquivo);
}

void leituraRNPSetores(RNPSETORES **rnpSetores, long int numeroSetores)
{
   long int idNo, idAdj, auxiliar;
   char *data;
   FILE *arquivo;
   int numeroRnp;
   char buffer[30000];
   int numeroNos;
   long int contador;
   int contadorAdj, contadorNos, tamanho;
    if (((* rnpSetores) = (RNPSETORES *)malloc( (numeroSetores+3) * sizeof(RNPSETORES)))==NULL)
    {
        printf("Não foi possível alocar a lista de RNP de setores");
        exit(1);
    }
   
    arquivo = fopen("rnpsetores.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo rnpsetores.dad");
        exit(1);
    }
    for (contador = 0; contador < numeroSetores; contador++) {
        fgets(buffer, 30000, arquivo);
        sscanf(buffer, "Identificador	Setor: %ld RNPs %d", &idNo, &numeroRnp);
        (*rnpSetores)[idNo].rnps = Malloc(RNPSETOR, (numeroRnp + 1));
        (*rnpSetores)[idNo].idSetor = idNo;
        (*rnpSetores)[idNo].numeroRNPs = numeroRnp;
        
        for(contadorAdj = 0; contadorAdj < numeroRnp; contadorAdj++)
        {
            fgets(buffer, 30000, arquivo);
            sscanf(buffer, "Origem %ld numero	nos %d", &idAdj, &numeroNos);
            (*rnpSetores)[idNo].rnps[contadorAdj].nos = Malloc(NORNP, (numeroNos));
            (*rnpSetores)[idNo].rnps[contadorAdj].numeroNos = numeroNos;
            (*rnpSetores)[idNo].rnps[contadorAdj].idSetorOrigem = idAdj;
            fgets(buffer, 30000, arquivo);
            data = buffer;
            for(contadorNos = 0; contadorNos < numeroNos; contadorNos++)
            {
                sscanf(data,"%ld%n",&auxiliar,&tamanho);
                (*rnpSetores)[idNo].rnps[contadorAdj].nos[contadorNos].idNo = auxiliar;
                data+=tamanho;
            }
            fgets(buffer, 30000, arquivo);
            data = buffer;
            for(contadorNos = 0; contadorNos < numeroNos; contadorNos++)
            {
                sscanf(data,"%ld%n",&auxiliar,&tamanho);
                (*rnpSetores)[idNo].rnps[contadorAdj].nos[contadorNos].profundidade = auxiliar;
                data+=tamanho;
            }
        }
        (*rnpSetores)[idNo].rnps[numeroRnp].nos = Malloc(NORNP, (4));
    }
    fclose(arquivo);
        
    //rnp de setores para armazenar a barra de subestação para o calculo do fluxo em anel
    (*rnpSetores)[0].rnps = Malloc(RNPSETOR, (1));
    (*rnpSetores)[0].idSetor = 0;
    (*rnpSetores)[0].numeroRNPs = 1;

    (*rnpSetores)[numeroSetores+1].rnps = Malloc(RNPSETOR, (3));
    (*rnpSetores)[numeroSetores+1].idSetor = numeroSetores+1;
    (*rnpSetores)[numeroSetores+1].numeroRNPs = 3;
    (*rnpSetores)[numeroSetores+2].rnps = Malloc(RNPSETOR, (3));
    (*rnpSetores)[numeroSetores+2].idSetor = numeroSetores+2;
    (*rnpSetores)[numeroSetores+2].numeroRNPs = 3;
}


void leituraListaChaves(LISTACHAVES **listaChavesParam, long int *numeroChaves, ESTADOCHAVE **estadoInicial)
{
    FILE *arquivo;
    long int contador;
    long int barraOrigem, barraDestino, codigo;
    char buffer[5000];
    char *data;
    char codOperacional[100];
    int condicao, tipoChave, subTipoChave, estado;
    arquivo = fopen("chavesCompleto.dad","r+" );
    int contadorCCM = 0, contadorCCR = 0;

    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo chavesCompleto.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", numeroChaves);
    
   if (((*listaChavesParam) = (LISTACHAVES *)malloc((numeroChaves[0]+1) * sizeof(LISTACHAVES)))==NULL)
    {
        printf("ERRO ao alocar memoria para lista de chaves!!!");
        exit(1);

    } 
    
    if (((*estadoInicial) = (ESTADOCHAVE *)malloc((numeroChaves[0]+1) * sizeof(ESTADOCHAVE)))==NULL)
    {
        printf("ERRO ao alocar memoria para lista de chaves!!!");
        exit(1);
    }
    for (contador = 1; contador <= numeroChaves[0]; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %s %d %ld %ld %d %d %d", &codigo, codOperacional, &estado, &barraOrigem, &barraDestino, &tipoChave, &subTipoChave, &condicao);
        (*listaChavesParam)[codigo].idNoDe = barraOrigem;
        (*listaChavesParam)[codigo].idNoPara = barraDestino;
        (*listaChavesParam)[codigo].condicao = condicao;
        (*listaChavesParam)[codigo].estadoChave = estado;
        (*listaChavesParam)[codigo].identificador =  codigo;
        (*listaChavesParam)[codigo].subTipoChave = subTipoChave;
        (*listaChavesParam)[codigo].tipoChave = tipoChave;
        strcpy((*listaChavesParam)[codigo].codOperacional,codOperacional);
        (*estadoInicial)[codigo] = estado;

//        if((*listaChavesParam)[codigo].tipoChave == 0)
//        	contadorCCM++;
//        if((*listaChavesParam)[codigo].tipoChave == 1)
//			contadorCCR++;


    }

//    printf("CCM: %d\n CCR: %d", contadorCCM, contadorCCR);
//    printf("\n");
//    printf("\n");
//    printf("\n");

    fclose(arquivo);
}


void leituraSetorBarra(GRAFO *grafoSDRParam)
{
    FILE *arquivo;
    long int contador, nBarras;
    long int barra, setor;
    char buffer[5000];
    
    arquivo = fopen("barras.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo barras.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "numero barras %ld", &nBarras);
    
    for (contador = 1; contador <= nBarras; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "BarraId %ld SetorId %ld", &barra,  &setor);
        grafoSDRParam[barra].idSetor = setor;
    }
    fclose(arquivo);
}

void leituraBarrasSimplicado(GRAFO **grafoSDRParam, long int *numeroBarras)
{
    FILE *arquivo;
    int tipo;
    double p, q;
    long int contador;
    long int barra, setor;
    char buffer[5000];

    arquivo = fopen("barras.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo barras.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", numeroBarras);

     //realiza a alocação da estrutura em grafo para armazenar o SDR e verifica se o processo foi bem sucedido
    //caso não seja possível realizar a alocação o programa é encerrado
    if (((*grafoSDRParam) = (GRAFO *)malloc( (numeroBarras[0]+1) * sizeof(GRAFO)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para o grafo do SDR !!!!");
        exit(1);
    }
    for (contador = 1; contador <= numeroBarras[0]; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %ld %d %lf %lf", &barra,  &setor, &tipo, &p, &q);
        (*grafoSDRParam)[barra].idNo = barra;
        (*grafoSDRParam)[barra].idSetor = setor;
        (*grafoSDRParam)[barra].tipoNo = tipo;
        (*grafoSDRParam)[barra].valorPQ.p = p;
        (*grafoSDRParam)[barra].valorPQ.q = q;

    }
    fclose(arquivo);
}

/*
 * Por Cristhian: Função modificada para que leia as informações relevantes para o FP.
 */
void leituraBarrasSimplicadoModificada(GRAFO **grafoSDRParam, long int *numeroBarras)
{
    FILE *arquivo;
    int tipo;
    double p, q;
    long int contador;
    long int barra, setor;
    char buffer[5000];
    int ignorar = 0, contadorBarras = 0;

    arquivo = fopen("barras_m.dad","r+" );
    
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo barras_m.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", numeroBarras);
    
     //realiza a alocação da estrutura em grafo para armazenar o SDR e verifica se o processo foi bem sucedido
    //caso não seja possível realizar a alocação o programa é encerrado
    if (((*grafoSDRParam) = (GRAFO *)malloc( (numeroBarras[0]+1) * sizeof(GRAFO)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para o grafo do SDR !!!!");
        exit(1); 
    }    
    for (contador = 1; contador <= numeroBarras[0]; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %ld %d %lf %lf %d", &barra,  &setor, &tipo, &p, &q, &ignorar);
        (*grafoSDRParam)[barra].idNo = barra;
        (*grafoSDRParam)[barra].idSetor = setor;
        (*grafoSDRParam)[barra].tipoNo = tipo;
        (*grafoSDRParam)[barra].valorPQ.p = p;
        (*grafoSDRParam)[barra].valorPQ.q = q;
    }
    fclose(arquivo);
}

/*
 * Por Cristhian: Função modificada para que leia as informações relevantes para o FP.
 */
void leituraBarrasTrifasicas(GRAFO **grafoSDRParam, long int *numeroBarras)
{
    FILE *arquivo;
    double pA, qA, pB, qB, pC, qC;
    long int contador;
    long int barra, setor;
    int fases;
    char buffer[5000];
    int contadorBarras = 0;

    arquivo = fopen("barrasTrifasicas.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo barrasTrifasicas.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", numeroBarras);

    for (contador = 1; contador <= numeroBarras[0]; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %ld %d %lf %lf %lf %lf %lf %lf" , &barra,  &setor, &fases, &pA, &qA, &pB, &qB, &pC, &qC);
        (*grafoSDRParam)[barra].idNo = barra;
        (*grafoSDRParam)[barra].idSetor = setor;

        //Verifica quais fases conectam-se ao nó e atribui o objeto TIPOFASES correspondente
        TIPOFASES enumFases;

        switch(fases){
            case 100:
                enumFases = A;
            break;
            //Esse valor é equivalente a 010
            case 10:
                enumFases = B;
            break;
            case 001:
                enumFases = C;
            break;
            case 110:
                enumFases= AB;
            break;
            case 101:
                enumFases = AC;
            break;
            //Esse valor é equivalente a 011
            case 11:
                enumFases = BC;
            break;
            case 111:
                enumFases = ABC;
            break;
            default:
                printf("Erro na leitura das fases - Valor invalido na barra %ld do setor %ld\n",barra,setor);
            break;
        }
        (*grafoSDRParam)[barra].tipoFases = enumFases;
        //Salva os dados de demanda de potência por fase, para cada barra.
        (*grafoSDRParam)[barra].pqTrifasico.p[0] = pA;
        (*grafoSDRParam)[barra].pqTrifasico.p[1] = pB;
        (*grafoSDRParam)[barra].pqTrifasico.p[2] = pC;

        (*grafoSDRParam)[barra].pqTrifasico.q[0] = qA;
        (*grafoSDRParam)[barra].pqTrifasico.q[1] = qB;
        (*grafoSDRParam)[barra].pqTrifasico.q[2] = qC;
    }

    fclose(arquivo);
}


void leituraMatrizImpedanciaCorrente(MATRIZCOMPLEXA **ZParam,  MATRIZMAXCORRENTE **maximoCorrenteParam, long int numeroBarrasParam, 
        DADOSALIMENTADOR *dadosAlimentador, long int numeroAlimentadores)
{
    FILE *arquivo;
    char buffer[5000];
    long int indiceBarras, barra, barraAdj;
    int indiceAdjacente, numeroAdjacentes, indiceAlimentador;
    double resistencias[6], reatancias[6],ampacidade;
    BOOL barraAlimentador;
    int iterador,iteradorAuxiliar,contador=0;
    arquivo = fopen("matrizImpedanciaCorrenteTrifasicos.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo matrizImpedanciaCorrenteTrifasicos.dad");
        exit(1);
    }

    if (((*maximoCorrenteParam) = (MATRIZMAXCORRENTE *) malloc((numeroBarrasParam + 4) * sizeof (MATRIZMAXCORRENTE))) == NULL) {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para a matriz de máximo de corrente !!!!");
        exit(1);
    }

    //aloca a memória para a matriz Z 
    if (((*ZParam) = (MATRIZCOMPLEXA *) malloc((numeroBarrasParam + 4) * sizeof (MATRIZCOMPLEXA))) == NULL) {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para a matriz Z !!!!");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);

    (*ZParam)[0].idNo = 0;
    (*ZParam)[0].numeroAdjacentes = 1;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[0].noAdjacentes = (CELULACOMPLEXA *) malloc(1 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[0].idNo = 0;
    (*maximoCorrenteParam)[0].numeroAdjacentes = 1;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[0].noAdjacentes = (CELULACORRENTE *) malloc(1 * sizeof (CELULACORRENTE));

    //barra ficticia para o regulador do alimentador 1 no fluxo em anel
    (*ZParam)[numeroBarrasParam+1].idNo = numeroBarrasParam+1;
    (*ZParam)[numeroBarrasParam+1].numeroAdjacentes = 2;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[numeroBarrasParam+1].noAdjacentes = (CELULACOMPLEXA *) malloc(2 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[numeroBarrasParam+1].idNo = numeroBarrasParam+1;
    (*maximoCorrenteParam)[numeroBarrasParam+1].numeroAdjacentes = 2;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[numeroBarrasParam+1].noAdjacentes = (CELULACORRENTE *) malloc(2 * sizeof (CELULACORRENTE));

    //barra ficticia para o regulador do alimentador 2 no fluxo em anel
    (*ZParam)[numeroBarrasParam+2].idNo = numeroBarrasParam+2;
    (*ZParam)[numeroBarrasParam+2].numeroAdjacentes = 2;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[numeroBarrasParam+2].noAdjacentes = (CELULACOMPLEXA *) malloc(2 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[numeroBarrasParam+2].idNo = numeroBarrasParam+2;
    (*maximoCorrenteParam)[numeroBarrasParam+2].numeroAdjacentes = 2;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[numeroBarrasParam+2].noAdjacentes = (CELULACORRENTE *) malloc(2 * sizeof (CELULACORRENTE));


    //barra ficticia para ligar a subestacao ao modelo do anel
    (*ZParam)[numeroBarrasParam+3].idNo = numeroBarrasParam+3;
    (*ZParam)[numeroBarrasParam+3].numeroAdjacentes = 3;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[numeroBarrasParam+3].noAdjacentes = (CELULACOMPLEXA *) malloc(3 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[numeroBarrasParam+3].idNo = numeroBarrasParam+3;
    (*maximoCorrenteParam)[numeroBarrasParam+3].numeroAdjacentes = 3;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[numeroBarrasParam+3].noAdjacentes = (CELULACORRENTE *) malloc(3 * sizeof (CELULACORRENTE));


    for(indiceBarras = 1; indiceBarras <= numeroBarrasParam; indiceBarras++)
    {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %d", &barra, &numeroAdjacentes);
        //verifica se a barra é de um alimentador
        barraAlimentador = false;
        indiceAlimentador = 1;
        while ((indiceAlimentador <= numeroAlimentadores) && !barraAlimentador) {
            if (dadosAlimentador[indiceAlimentador].barraAlimentador == barra)
            {
                barraAlimentador = true;
            }
            indiceAlimentador++;
        }
        (*ZParam)[barra].idNo = barra;
        (*ZParam)[barra].numeroAdjacentes = numeroAdjacentes;
        (*maximoCorrenteParam)[barra].idNo = barra;
        (*maximoCorrenteParam)[barra].numeroAdjacentes = numeroAdjacentes;
        if (barraAlimentador) {
            //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
            (*ZParam)[barra].noAdjacentes = (CELULACOMPLEXA *) malloc((numeroAdjacentes + 1) * sizeof (CELULACOMPLEXA));


            //aloca o espaço para os dados dos adjacentes.
            (*maximoCorrenteParam)[barra].noAdjacentes = (CELULACORRENTE *) malloc((numeroAdjacentes + 1) * sizeof (CELULACORRENTE));
        } else {
            //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
            (*ZParam)[barra].noAdjacentes = (CELULACOMPLEXA *) malloc(numeroAdjacentes * sizeof (CELULACOMPLEXA));

            //aloca o espaço para os dados dos adjacentes.
            (*maximoCorrenteParam)[barra].noAdjacentes = (CELULACORRENTE *) malloc(numeroAdjacentes * sizeof (CELULACORRENTE));
        }
        for(indiceAdjacente = 0; indiceAdjacente < numeroAdjacentes; indiceAdjacente++)
        {
            fgets(buffer, 5000, arquivo);
            sscanf(buffer, "%ld %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &barraAdj, &ampacidade, &resistencias[0],&reatancias[0],&resistencias[1],&reatancias[1],&resistencias[2],&reatancias[2],&resistencias[3],&reatancias[3],&resistencias[4],&reatancias[4],&resistencias[5],&reatancias[5]);

            (*ZParam)[barra].noAdjacentes[indiceAdjacente].idNo = barraAdj;
            //Adicionando os elementos da matriz impedancia da triangular superior
            for(iterador=0;iterador<3;iterador++){
                for(iteradorAuxiliar=iterador;iteradorAuxiliar<3;iteradorAuxiliar++){
                    (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[iterador][iteradorAuxiliar] = resistencias[contador] + ij*reatancias[contador];
                    contador++;
                }
            }
            //Adicionando os elementos da matriz impedancia da triangular inferior
            (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[iterador][iteradorAuxiliar] = resistencias[1] + ij*reatancias[1];
            (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[iterador][iteradorAuxiliar] = resistencias[2] + ij*reatancias[2];
            (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[iterador][iteradorAuxiliar] = resistencias[4] + ij*reatancias[4];

            (*maximoCorrenteParam)[barra].noAdjacentes[indiceAdjacente].valor = ampacidade;
        }
    }
    fclose(arquivo);

}

void leituraVetorTaps(int *tapsParam, DADOSREGULADOR *dadosReguladorParam, long int numeroReguladoresParam, long int numeroBarrasParam) {
    long int contador, nLinhas, indiceNo1, indiceNo2, indiceRegulador;
    char idRegulador[30];
    FILE *arquivo;
    char buffer[5000];
    arquivo = fopen("barrasRegulador.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo matrizImpedanciaCorrente.dad");
        exit(1);
    }

    for (indiceNo1 = 1; indiceNo1 <= numeroBarrasParam+4; indiceNo1++) {
        tapsParam[indiceNo1] = 0;
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", &nLinhas);

    for (contador = 0; contador < nLinhas; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%s %ld %ld", idRegulador, &indiceNo1, &indiceNo2);
        for (indiceRegulador = 0; indiceRegulador < numeroReguladoresParam; indiceRegulador++) {
            if (strcmp(idRegulador, dadosReguladorParam[indiceRegulador].idRegulador) == 0) {
                //printf("regulador %s barra1 %ld barra2 %ld\n", idRegulador, grafoSDRParam[indiceNo1].idNo, grafoSDRParam[indiceNo1].adjacentes[indiceNo2].idNo);
                tapsParam[indiceNo1] = indiceRegulador;
                tapsParam[indiceNo2] = indiceRegulador;
            }
        }
    }

        fclose(arquivo);
}


/* @brief Por Leandro: faz a leitura somente do SBase a fim de evitar a adição de passagem deste parâmentro entre várias funções
 * até o seu uso na função "operadorLSOHeuristica()"
 *
 * @param SBase variável a ser lida
 * @return
*/
void leituraSBase(int *SBase)
{

    FILE *arquivo; //Variável ponteiro do tipo FILE para ler e manipular o arquivo.
    char blocoLeitura[100]; //Variável para realizar a leitura do bloco de caracteres do arquivo.
    //Abertura do arquivo dados_reguladores_tensao.dad e associação a variável arquivo
    arquivo = fopen("parametros.dad","r");
    //Verifica se a abertura do arquivo ocorreu com sucesso. Caso contrário encerra o programa.
    if(arquivo == NULL)
    {
            printf("Erro ao abrir arquivo parametros.dad !!!\n");
            exit(1);
    }
    fgets(blocoLeitura,100,arquivo);
    fgets(blocoLeitura,100,arquivo);
    fgets(blocoLeitura,100,arquivo);
    fgets(blocoLeitura,100,arquivo);
    //leitura da string blocoLeitura do Sbase
    fgets(blocoLeitura,100,arquivo);
    sscanf(blocoLeitura,"%d",&SBase[0]);
    fclose(arquivo);

}
