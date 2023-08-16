/**
 * @file data_structures.h
 * @brief Este arquivo contém a definição das estruturas de dados utilizadas no projeto smartgrids para
 * reconfiguração/restabelecimento de sistemas de distribuição de energia elétrica radiais (SDR).
 * Os tipos que estão sendo definidos para a aplicação tem como padrão o nome com todas as letras em caixa alta.
 * Para compor o nome das variáveis com nome composto o padrão utizado é o primeiro "nome" em minúsculo e os demais com a primeira letra em maiúsculo e as demais em minúsculo.
 * 
 */

#ifndef DATA_STRUCTURES_H_INCLUDED
#define DATA_STRUCTURES_H_INCLUDED

/**
 * LEANDRO: Define o valor da precisão a ser considerada na comparação entre valores de carregamento de rede, queda de tensão e carregamento de subestação. Em outras palavras, define o
 número de casas decimais a serem consideradas nestas comparações.
 Para considerar apenas duas casas decimais, utilize 100. Para considerar três casas decimais utilize 1000, e assim por diante.
 */
//#define precisaoComparacao 100

/**
 Define versão simplicada para o comando malloc da linguagem c
 */
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
/**
 Define a constante para o valor máximo do tipo inteiro
 */
#define MAXINT    100007
/**
 Define a constante para o valor máximo do tipo float
 */
//#define MAXFLOAT  100007.999999
#define fatorPonderacaoManobras  10.0

/**
 * Define o passo de ajuste pelo regulador de tensão
 */
#define passoRegulador 0.00625

/**
 * Específica a métrica de clock para calcular o tempo de execução em segundos.
 */
#ifdef CLOCKS_PER_SEC
 static const double CLOCK_TICKS_PER_SECOND = (double)CLOCKS_PER_SEC;
 #elif defined(CLK_TCK)
 static const double CLOCK_TICKS_PER_SECOND = (double)CLK_TCK;
 #else
 static const double CLOCK_TICKS_PER_SECOND = 1000000.0;
 #endif

 /**
  * Define os limites superiores para as restrições Carregamento de Rede, 
  * Carregamento de Trafo e Queda de Tensão.
  */
#define maxCarregamentoRede 100.00
#define maxCarregamentoTrafo 100.00
#define maxQuedaTensao 10.0
/*
  * Define o tempo (em HORAS) necessário para operação de chaves Manuais e Automáticas
  * A fonte destas informações é o seguinte artigo:
  * Zidan, A.; El-Saadany, E. F. A cooperative multiagent framework for self-healing mechanisms in distribution systems. IEEE Transactions on Smart Grid, v. 3, n. 3, p. 1525–1539, 2012.
  */
#define tempoOperacaoChaveManual 0.4166666666667 //Por Leandro: tempo, em HORAS, necessário para operação de uma chave do tipo manual. Enquivale a 25 MINUTOS (FONTE: Zidan, A.; El-Saadany, E. F. A cooperative multiagent framework for self-healing mechanisms in distribution systems. IEEE Transactions on Smart Grid, v. 3, n. 3, p. 1525–1539, 2012.)
#define tempoOperacaoChaveAutomatica 0.0138888888889 //Por Leandro: tempo, em HORAS, necessário para operação de uma chave do tipo automática. Equivale a 50 SEGUNDOS (FONTE: Zidan, A.; El-Saadany, E. F. A cooperative multiagent framework for self-healing mechanisms in distribution systems. IEEE Transactions on Smart Grid, v. 3, n. 3, p. 1525–1539, 2012.)

 /**
  * Por Leandro: Define o tempo (em HORAS) estimado para Restauração da Causa de UMA falha. O tempo total estimada a recuperação de várias faltas simultâneas
  * será calculado a partir dessa constante na biblioteca "funcoesHeuristica.c"
  */
 #define TEMPO_DE_RESTAURACAO_DA_FALHA 4

/**
 * 
 /**
  * Define os limites superiores relaxados para as restrições Carregamento de Trafo e Queda de Tensão.
  */
#define maxCarregamentoRedeR 100.00
#define maxCarregamentoTrafoR 150.00 //restrição relaxada
#define maxQuedaTensaoR 20.0 //restrição relaxada

 
/**
 * @brief A enumeração BOOLEAN é utilizada para definir o tipo BOOL.
 *
 */
enum BOOLEAN {
	true = 1, /**< Valor verdadeiro para os testes booleanos. */
	false = 0 /**< Valor falso para os testes booleanos. */
};
/**
 * @brief O tipo bool simula o uso de variáveis booleanas presente na linguagem C++.
 *
 * As variáveis deste tipo serão utilizadas nos testes lógicos.
 * Além disso, este tipo será utilizado para as comparações condicionais.
 */
typedef enum BOOLEAN BOOL;

/**
 * @brief A enumeração TIPONO define a classificação dos nós que representam barras do SDR.
 *
 * Variáveis do TIPONO podem possuir 4 classificações.
 * Essa classificações são sem carga, com carga, capacitor e raiz de alimentador.
 * Os dados de entrada contém um valor inteiro especificado para cada uma das classificações com valores inteiros de 0 a 3
 */
typedef enum  {
	semCarga, /**< Indica as barras que não possuem carga. */
	comCarga, /**< Indica as barras com carga. */
	capacitor, /**< Indica as barras que representam capacitores. */
	raizAlimentador /**< Indica que a barra é raiz de alimentador. */
} TIPONO;

/**
 * @brief A enumeração TIPOFASES define as fases as quais estão conectadas a um nó do SDR.
 *
 * Variáveis do TIPOFASES podem possuir 7 classificações.
 * Essa classificações são A, B, C, AB, AC, BC, ABC.
 * Os dados de entrada contém um valor inteiro especificado para cada uma das classificações, de três caracteres, indicando, respectivamente, se as fases A, B e C estão conectadas ao nó (valor igual a 1), ou não (valor igual a 0).
 * Ex: Sequência 100 (A), 101 (AC), 111 (ABC).
 */

typedef enum  {
  A,
  B,
  C,
  AB,
  AC,
  BC,
  ABC
} TIPOFASES;

/**
 * @brief A enumeração SEQUENCIAMANOBRASALIVIO define a sequência na qual deseja-se operar as
 * manobras que ocorrem em pares para transferência de cargas entre alimentadores visando
 * o alívio de alimentadores. Manobras de alívio em alimentadores são importantes porque podem
 * permitir a reconexão de mais consumidores saudáveis desligados
 *
 * Variáveis do SEQUENCIAMANOBRASALIVIO podem possuir 2 classificações.
 * Essa classificações são:
 * "abreFecha" - primeiro desliga o agrupamento de setores a ser transferido e depois reconecta-o ao novo alimentador
 * "fechaAbre" - primeiro conecta no novo alimentador o agrupamento de setores a ser transferido e depois desliga-o do alimentador de origem
 */
typedef enum  {
	abreFecha,
	fechaAbre,
} SEQUENCIAMANOBRASALIVIO;


/**
 * @brief A enumeração TIPOARESTA define a classificação das aresta do SDR.
 *
 * Variáveis do TIPOARESTA podem possuir 4 classificações.
 * Essa classificações são chave manual, chave automática, trecho e regulador de Tensão.
 * Os dados de entrada contém um valor inteiro especificado para cada uma das classificações com valores inteiros de 0 a 3
 */
typedef enum  {
	chaveManual, /**< Indica que o tipo da chave é manual. */
	chaveAutomatica, /**< Indica que o tipo da chave é automática. */
	trecho, /**< Indica que a aresta é um trecho. */
	reguladorTensao /**< Indica que a aresta é um regulador de tensão. */
} TIPOARESTA;

/**
 * @brief A enumeração ESTADOCHAVE define qual o estado da chave no inicio da execução do programa no SDR.
 * Variáveis do tipo ESTADOCHAVE podem possuir 3 valores.
 * Os valores possíveis são normalmente Aberta, normalmente Fechada e outros tipos de aresta. Para as arestas que não representam chaves é utilizado o terceiro valor.
  * Os dados de entrada contém um valor inteiro especificado para cada uma das categorias com valores inteiros de 0 a 2
 */
typedef enum  {
	normalmenteAberta, /**< Indica que o estado da chave é normalmente aberto. */
        normalmenteFechada, /**< Indica que o estado da chave é normalmente fechado. */
        outrasArestas /**< Valor utilizado para as arestas que não representam chaves. */
} ESTADOCHAVE;

/**
 * @brief A enumeração CONDICAO define a condição de operação das arestas do SDR.
 *
 * Variáveis do tipo CONDICAO podem possuir 2 valores.
 * Os valores possíveis são em operação e com restrição. Para as arestas do tipo trecho a condição possui valor em operação em todas as situações.
 * Para os demais tipos de aresta esse valor indica a condição operacional de cada uma e pode variar a cada execução do software.
 * Os dados de entrada contém um valor inteiro especificado para cada uma das categorias com valores inteiros de 0 ou 1
 */
typedef enum  {
	emOperacao, /**< Indica que a aresta está em condição de operação, trechos sempre estão em operação. */
	comRestricao /**< Indica que a aresta possui alguma restrição em sua operação. */
} CONDICAO;


/**
 * @brief A enumeração SUBTIPOARESTA define a subdivisão da classificação das arestas do grafo que representa o SDR.
 *
 * Variáveis do SUBTIPOARESTA podem possuir 5 categorias.
 * As categorias são chave fusível, chave seca, chave operável com carga, chave operável com curto circuíto e outros tipos de aresta.
 * Os dados de entrada contém um valor inteiro especificado para cada uma das categorias com valores inteiros de 0 a 4
 */
typedef enum  {
	chaveFusivel, /**< Indica que a chave é do tipo fusível. */
	chaveSeca, /**< Indica que a chave é do tipo seca. */
	chaveCarga, /**< Indica que a chave opera com carga. */
	chaveCurtoCircuito, /**< Indica que a chave opera em curti circuito. */
	outrosSubTipo /**< Utilizada para as arestas que não são chaves. */
} SUBTIPOARESTA;

/**
 * @brief A enumeração TIPOREGULADOR especifica se o regulador de tensão permite ou não fluxo reverso.
 *
 * Variáveis do TIPOREGULADOR podem ter 2 valores.
 * A categoria comFluxoReverso indica que o regulador permite a operação com fluxo reverso e a
 * categoris SemFluxoReverso indica que o regulador não permite a operação com fluxo reverso.
 * Os dados de entrada contém um valor inteiro especificado para cada uma das categorias com valores inteiros de 0 e 1
 */
typedef enum  {
	comFluxoReverso, /**< Indica que o regulador de tensão opera com fluxo reverso. */
	semFluxoReverso /**< Indica que o regulador de tensão não opera com fluxo reverso. */
} TIPOREGULADOR;



/**
 * @brief Armazena as informações de cada uma das arestas do grafo que representa o SDR.
 *
 * Esta estrutura é utlizada para armazenar as informações de cada uma das arestas, na representação por grafos
 * do sistema de distribuição. Uma aresta do SDR pode ser uma chave, manual ou automática, um trecho ou um regulador de tensão.
 * Cada posição da lista de adjacentes de um nó do grafo será especificada pelo tipo criado a partir dessa estrutura.
 */
typedef struct {
  long int idNo;/**< Inteiro que identifica cada nó do SDR. */
  char idAresta[15]; /**< Número operacional das chaves e reguladores de tensão. No caso de trechos é um número sequencial */
  ESTADOCHAVE estadoChave; /**< Estado da chave no momento inicial do processo, aberta ou fechada. */ 
  TIPOARESTA tipoAresta; /**< Tipo da aresta no SDR. */
  CONDICAO condicao; /**< Condição operacional da aresta. */
  SUBTIPOARESTA subTipoAresta; /**< Subtipo da aresta no SDR. */
  double resistencia; /**< Valor do tipo double indicando a resistência da aresta. */
  double reatancia; /**< Valor do tipo double indicando a reatância da aresta. */
  double ampacidade; /**< Valor do tipo double indicando a ampacidade da aresta. */
} NOADJACENTE;

/**
 * @brief Armazena os valores de P e Q das barras do SDR.
 *
 * Nos casos onde a barra for sem carga esses campos possuem valor 0.
 */
typedef struct {
  double p; /**< Valor do tipo double indicando o valor P da barra. */
  double q; /**< Valor do tipo double indicando o valor Q da barra. */
} PQ;

typedef struct {
  double p[3];
  double q[3];
} PQTRIFASICO;


/**
 * Por Leandro
 * @brief A enumeração TIPOCONSUMIDOR define a classificação dos consumidores em cada barraa do SDR.
 *
 */
typedef enum  {
	semPrioridade, //0
	prioridadeBaixa, //1
	prioridadeIntermediaria, //2
	prioridadeAlta //3
} TIPOCONSUMIDOR;

/**
 * @brief Armazena os dados necessários para calcular a priorização.
 *
 * Esta estrutura contém os valores que serão utilizados para calcular a priorização de religação dos setores.
 *
 */

typedef struct {
  double eusdGrupoA; /**< Valor de EUSD do grupo A. */
  double eusdGrupoB; /**< Valor de EUSD do grupo B. */
  int qtdConsumidores; /**< Quantidade de consumidores. */
  int qtdConsumidoresEspeciais; /**< Quantidade de consumidores especiais. */
  TIPOCONSUMIDOR prioridadeConsumidor; /**<Por Leandro: Define o tipo do consumidor conectado à barra. */
} PRIORIZACAO;

/**
 * @brief Armazena os dados necessários para calcular a priorização.
 *
 * Esta estrutura contém os valores que serão utilizados para calcular a priorização de religação dos setores.
 *
 */
typedef struct {
  TIPOCONSUMIDOR prioridadeConsumidor; /**<Por Leandro: Define o tipo do consumidor conectado à barra. */
} PRIORIDADES;

/**
 * @brief Armazena os dados necessários para calcular as compensações.
 *
 * Esta estrutura contém os valores que serão utilizados para calcular as compensações
 * pagas na forma de multas para a Aneel com base nos indicadores estabelecidos.
 * Valores necessários para o cálculo da compensação esperada no trecho a ser priorizado.
 * Os valores de DIC e DMIC são compensações diretas do trecho desligado e são proporcionais ao tempo esperado para inspeção e recomposição.
 * Já o valor do FIC foi inserido para possibilitar o cálculo das possíveis compensações que ocorrerão caso se tome a decisão de desligar trechos "sãos" visando o corte de carga para
 * restabelecimento de outros trechos do sistema.
 */
typedef struct {
  double metaDicMensal; /**< Meta de DIC mensal. */
  double metaDicTrimestral; /**< Meta de DIC trimestral. */
  double metaDicAnual; /**< Meta de DIC anual. */
  double metaDmicMensal; /**< Meta de DMIC mensal. */
  double metaFicMensal; /**< Meta de FIC mensal. */
  double metaFicTrimestral; /**< Meta de FIC trimestral. */
  double metaFicAnual; /**< Meta de FIC anual. */
  double resulDicAcumAno; /**< Resultado de DIC acumulado no Ano. */
  double resulDicAcumTrimestre; /**< Resultado de DIC acumulado no Trimestre. */
  double resulDicAcumMes; /**< Resultado de DIC acumulado no mês. */
  double resulFicAcumAno; /**< Resultado de FIC acumulado no Ano. */
  double resulFicAcumTrimestre; /**< Resultado de FIC acumulado no trimestre. */
  double resulFicAcumMes; /**< Resultado de FIC acumulado no mês. */
  double resulDmicMes; /**< Resultado de DMIC acumulado no mês. */
} COMPENSACAO;

/**
 * @brief Armazena as informações de cada um dos nós/barras do SDR.
 *
 * Esta estrutura é utilizada para armazenar as informações de cada um dos nós, na representação por grafos
 * do sistema de distribuição. Cada nó do grafo refere-se a uma barra do SDR. Os nós podem ser barras sem carga ou com carga, capacitor ou raiz de alimentador.
 * Cada posição da lista de adjacentes de um nó do grafo será especificada pelo tipo criado a partir dessa estrutura.
 */
typedef struct {
  long int idNo; /**< Valor inteiro sequencial atribuído a cada nó/barra do SDR. */
  TIPONO tipoNo; /**< Classificação do tipo de cada nó do SDR. */
  TIPOFASES tipoFases; /**< Classificação das fases conectadas a cada nó do SDR. */
  PQTRIFASICO pqTrifasico; /**< Classificação das fases conectadas a cada nó do SDR. */
  int numeroAdjacentes; /**< Indica o tamanho da lista de adjacentes do nó. */
  NOADJACENTE *adjacentes; /**< Lista dos nós adjacentes. */
  PQ valorPQ; /**< Array com 24 posições, uma para cada hora do dia contendo os valores de P e Q. */
  PRIORIZACAO priorizacoes; /**< Contém os valores para cálculo das priorizações. */
  COMPENSACAO compensacoes; /**< Contém os valores para verificação de compensações do trecho. */
  long int idSetor; /**< Identificador do setor, será utilizado no próximo de identificação dos setores. */
} GRAFO;


/**
 * @brief Armazena as informações de cada trafo que compõem o SDR.
 *
 * Esta estrutura é utilizada para armazenar as informações dos trafos do SDR.
 * Esses dados são utilizados no cálculo das funções objetivo do problema e para validação de factibilidade da solução
 * com base nas restrições operacionais.
 */

typedef enum{
  YYat,
  YatY,
  YatYat,
  YY,
  DD,
  DY,
  DYat,
  YatD,
  YD
} TIPOSTRAFOS;

typedef struct {
  int idTrafo; /**< Valor inteiro sequencial atribuído a cada trafo do SDR. */
  int idSubEstacao; /**< Valor inteiro sequencial atribuído a cada subestação do SDR. */
  double capacidade; /**< Indica a capacidade de potência em MVA do trafo. */
  __complex__ double tensaoReal; /**< Indica a tensão suportada pelo trafo. */
  double impedancia; /**< Indica a impedância do trafo. */
  TIPOSTRAFOS tipoTransformador;
  int numeroAlimentadores; /**< Por Leandro: Valor inteiro que quantifica o número de alimentadores conectados a um trafo. */
  int *alimentador; /**< Por Leandro: Vetor que armazena o índice dos Alimentadores conectados ao um trafo. */
} DADOSTRAFO;



/**
 * @brief Armazena as informações de cada alimentador que compõem o SDR.
 *
 * Esta estrutura é utilizada para armazenar a relação dos alimentadores com os trafos do SDR.
 * Esses dados são utilizados no cálculo das funções objetivo do problema e para validação de factibilidade da solução
 * com base nas restrições operacionais.
 */
typedef struct {
  char idAlimentador[15]; /**< Código operacional do alimentador no SDR. */
  long int barraAlimentador; /**< Valor inteiro da barra do alimentador, idNo que corresponde a este. */
  int idTrafo; /**< Identificador do trafo ao qual o alimentador estão ligado.*/
  int numeroSetores; /**< Armazena quantos setores compoem o alimentador. Será utilizado para definir o tamanho da RNP*/
} DADOSALIMENTADOR;

/**
 * @brief Armazena as informações dos reguladores de tensão presentes no SDR.
 *
 * Esta estrutura é utilizada para armazenar os dados de operação dos reguladores de tensão presentes no SDR.
 * Esses dados são utilizados no cálculo das funções objetivo do problema e para validação de factibilidade da solução
 * com base nas restrições operacionais.
 */
typedef struct {
  char idRegulador[30]; /**< Numero Operacional do regulador de tensao. */
  TIPOREGULADOR tipoRegulador; /**< Caracteriza se o regulador de tensão permite ou não fluxo reverso. */
  double ampacidade; /**< Ampacidade suportada pelo regulador de tensão.*/
  int numeroTaps; /**< Porcentagem de correção realizada pelo regulador.*/
} DADOSREGULADOR;

 /**
  * @brief Estrutura de dados para montagem de uma lista ligada de inteiros.
  * 
  * Esss estrutura de dados é utilizada no processo de identificação dos setores do SDR.
  * Setor é o agrupamento de barras entre chaves normalmente abertas ou fechadas. 
  * A lista é utilizada para armazenar identificador de barras do setor. 
  * 
  */
typedef struct NoLista {
    long int idNo; /**< identificador do elemento.*/
    struct NoLista * prox; /**< Ponteiro do tipo NOSETOR para o próximo elemento.*/
} NOSETOR;

/**
 * @brief Estrutura de dados do tipo lista ligada de setores que compõem a área fornecida por um alimentador do SDR.
 *
 * A estrutura de dados contém a lista ligada de barras do setor, a lista ligada de barras adjacentes do setor e os dados do setor.
 * É utilizada no processo de identificação dos agrupamentos de setores do sistema de distribuição e da topologia atual do sistema. 
 */
typedef struct ListaSetores{ 
    NOSETOR * setor;  /**< Ponteiro do tipo NOSETOR para a lista de barras do setor.*/
    NOSETOR * setorAdj;/**< Ponteiro do tipo NOSETOR para a lista de barras adjacentes.*/
    long int consumidoresEspeciais; /**< Armazena o número de consumidores no setor.*/
    long int consumidoresNormais; /**< Armazena o número de consumidores especiais no setor.*/
    long int idSetor; /**< Armazena o identificador do setor.*/
    int numeroAdjacentes; /**< Armazena o número de setores vizinhos.*/
    int numeroNos; /**< Contém o número de barras do setor.*/
    struct ListaSetores * prox; /**< Ponteiro do tipo LISTASETORES para o próximo elemento.*/
} LISTASETORES;
/**
 * @brief Estrutura de dados do tipo lista ligada para armazenar a fila de barras adjacentes.
 *
 * A estrutura de dados contém a lista ligada de barras adjacentes de um setor por chave fechada.
 * É utilizada no processo de identificação dos setores do sistema de distribuição e da topologia atual do sistema. 
 * Define quais serão os processos setores a serem identificados.
 */
typedef struct Fila {
    long int idNo; /**< Identificador da barra.*/
    struct Fila * prox; /**< Ponteiro do tipo FILABARRAS para o próximo elemento.*/
} FILABARRAS;

/*Estruturas da RNP*/

/**
 * @brief Armazena as informações que compõem as colunas da matriz PI.
 *
 * Esta estrutura é utilizada para armazenar os dados necessários para rapidamente identificar onde
 * um nó se encontra no indivíduo.
 */
typedef struct 
{
  long int idConfiguracao; /**< Inteiro que identifica o índividuo onde o ocorreu modificação no nó. */
  int idRNP; /**< Inteiro que identifica o RNP que contém o nó. */
  int posicao; /**< Inteiro que identifica a posição do nó na RNP. */
}COLUNAPI;


/**
 * @brief Represenra a matriz PI da RNP.
 *
 * Esta estrutura é utilizada para armazenar os dados de cada nó para rapidamente identifica-lo no momento
 * da aplicação dos operadores da RNP.
 */

typedef struct
{
  long int idNo; /**< Identificador do nó. */
  int maximoColunas; /**< Número máximo de colunas disponíveis da matriz. */
  int numeroColunas; /**< Número de colunas que estão ocupadas da matriz. */
  COLUNAPI *colunas; /**< Colunas de matriz PI. Cada coluna é alocada a medida que precisa ser utilizada. */
} MATRIZPI;


/**
 * @brief Armazena os nós utilizadas para obter um indivíduo.
 *
 * Esta estrutura contém os nós utilizadas para produzir um novo indivíduo à partir 
 * de outro indivíduo por meio dos operadores PAO ou CAO. Essa estrutura é um campo do vetor Pi da RNP. 
 * A informação contida nessa estrutura é utilizada para definir a sequência de chaveamento da solução final.
 */

typedef struct 
{ 
	long int p; /**< Valor do nó de poda p utilizado por PAO ou CAO.*/ 
	long int a; /**< Valor do nó de enxerto a utilizado por PAO ou CAO.*/
	long int r; /**< Valor do novo nó raiz r utilizado por CAO.*/
        //int iadj;  /**< Indice do nó a na lista de adjacências de p ou r.*/
} NOSPRA;

/**
 * @brief Por Leandro: A enumeração OPERADOR define os operadores possíveis de serem aplicados para a geração de um novo indivíduo.
 *
 * Variáveis do tipo OPERADOR podem possuir 6 classificações.
 * Essa classificações são Initial Individual Operator (IIP), Exahustive Search Operator (ESO), Load Reconector Operator (LRO), Preserve Ancestor Operator (PAO) e Chance Ancestor Operator (CAO) e Load Shedding Operator (LSO).
 * Os dados de entrada contém um valor inteiro especificado para cada uma das classificações com valores inteiros de 0 a 5
 */
typedef enum  {
	IIO, /**< Refere-se a função/operador que faz a obtenção da configuração conceitualmente início do problema, ie., aquela na qual o(s) setor(es) em falta encontra(m)-se isolado(s) e os setores saudáveis à jusante encontram-se desligados*/
	ESO, /**< Refere-se a função/operador que faz a obtenção das configurações por meio da heuristica que busca todas as opções de reconexão dos agrupamentos de setores saudáveis, a qual é comumente chamada de Busca Exaustiva*/
	LRO, /**< Refere-se ao operado que faz a geração de um novo indivíduo por meio da reconexão de setores saudáveis desligados possíveis de serem reconectados*/
	PAO, /**< Refere-se ao operador PAO da RNP que faz a geração de um novo indivíduo por meio da poda e transferência de setores por meio do mesmo nó no qual houve a poda*/
	CAO, /**< Refere-se ao operador CAO da RNP que faz a geração de um novo indivíduo por meio da poda e transferência de setores por meio de um nó diferente daquel no qual houve a poda*/
	LSO  /**< Refere-se ao operado que faz a geração de um novo indivíduo por meio do corte de setores em serviço */
} OPERADOR;


/**
 * @brief esta estrutura é utilizada para armazenar as informações
 * de uma configuração intermediária, a saber:
 * - o identificador da configuração
 * - o identificador da configuração anterior/ancestral
 * - o vetor com o identificador das chaves que devem ser abertas
 * - o vetor com o identificador das chaves que devem ser fechadas
 * - o número de pares de chaves
 */
typedef struct
{
	long int idConfiguracao; /**< Indice da configuração intermediária.*/
	long int idAncestral; /**< Indice da configuração intermediária anterior/ancestral.*/
	long int *idChaveAberta; /**< Vetor com o identificador das chaves que foram abertas para obter a configuração intermediária a partir da sua ancestral.*/
	long int *idChaveFechada; /**< Vetor com o identificador das chaves que foram fechadas para obter a configuração intermediária a partir da sua ancestral.*/
	NOSPRA *nos; /**< Conjuntos de nós utilizados para obter a configuração intermediária a partir da sua ancestral.*/
	int numeroParesManobras; /**< Informa o número de pares de manobras a serem executados. Em outras palavras, é o tamanho dos vetores "*idChaveAberta" e "*idChaveFechada"*/
} 	CHAVESCONFIGURACAO;

/**
 * @brief Por Leandro: esta estrutura é utilizada para armazenar o tamanho e as informações
 * das configurações intermediárias que compõem uma sequência de chavamento .
 */
typedef struct
{
	CHAVESCONFIGURACAO *configuracao; /**< Vetor com o indentificador das configurações intermediárias*/
	int tamanho; /**< Número de configurações intermediárias, ou de grupos de chaves*/
} 	SEQUENCIACHAVEAMENTO;

/**
 * @brief Armazena o ancestral do indivíduo.
 *
 * Esta estrutura armazena o id o indivíduo que foi utilizado para construir a nova solução por meio dos operadores
 * PAO e CAO. Além disso, armazena os nós (p, r, a) que foram escolhidos para obter essa solução.
 */
typedef struct 
{ 
	long int idAncestral; /**< Indice do indivíduo ancestral.*/
	long int *idChaveAberta; /**< Vetor com o identificador das chaves que foram abertas para obter a configuração.*/
	BOOL *estadoInicialChaveAberta; /**< Vetor que indica para cada chave aberta se esta voltou aos seu estado inicial.*/
	long int *idChaveFechada; /**< Vetor com o identificador das chaves que foram fechadas para obter a configuração.*/
	BOOL *estadoInicialChaveFechada; /**< Vetor que indica para cada chave fechada se esta voltou aos seu estado inicial.*/
	int casoManobra; /**< indica se a manobra fez com que alguma das chaves retornasse ao estado inicial, ver tese Augusto, valores possíveis
					  * caso 1: as duas chaves do par de manobra tem estado diferente da configuracao inicial
					  * caso 2: as duas chaves do par de manobra retornaram ao estado da condiguracao inicial
					  * caso 3: um das chaves retornou ao estado da configuração inicial e a outra possui estado diferente da configuração inicial */
	int numeroManobras; /**< Número PARES de manobras necessário para obter a configuração a partir do ancestral.*/
	NOSPRA *nos; /**< Conjuntos de nós utilizados.*/ 
	BOOL sequenciaVerificada; /**< Por Leandro: informa se a sequência de manobras do  indivíduo em questão foi verificada pelo procedimento de correção de sequência de chaveamento*/
//	SEQUENCIACHAVEAMENTO sequenciaChaveamento; /**< Por Leandro: armazena a sequência de chaveamento do indivíduo quando esta foi gerada pelas rotinas que removem chaves repetidas e não pelo processo evolutivo*/
//	BOOL sequenciaCorrigida; /**< Por Leandro: se "false" informa que a sequência de manobras do indivíduo em questão não é proviniente do processo de remoção de chaves que retornaram ao seu estado inicial, e a sequência de chaveamento pode ser, portante, obtida acessando normalmente o "vetorPi". Se "true, informa que a sequência de chaveamento do indivíduo foi obtida pelo procedimento de remoção de chaves que retornaram ao seu estado inicial e que a mesma está salva numa variável específica*/
//	OPERADOR operador; /**< Por Leandro: indica o operador que foi utilizado para a geração do indivíduo. Isto é necessário para o processamento de sequência de chaveamento ao longo do processo evolutivo.*/
} VETORPI;

/**
 * @brief Armazena os dados de cada nó na RNP.
 *
 * Esta estrutura é utilizada para armazenar nó e a sua profundidade para compor o vetor da RNP
 */
typedef struct
{
 long int idNo; /**< Identificador do nó.*/
 int profundidade; /**< Profundidade do nó em relação a raiz.*/
} NORNP;

/**
 * Por Leandro:
 * @brief Esta estrutura armazena a potência ATIVA não suprida por nível de prioridade de consumidor.
 */
typedef struct{
 double consumidoresSemPrioridade;
 double consumidoresPrioridadeBaixa;
 double consumidoresPrioridadeIntermediaria;
 double consumidoresPrioridadeAlta;
}NIVEISDEPRIORIDADEATENDIMENTO;

/**
 * @brief Estrutura dos objetivos.
 *
 * Estrutura para armazenar o valor de cada objetivo do indivíduo. É um campo do indivíduo.
 */
typedef struct{
  double demandaAlimentador; /**< Armazena a demanda de carga do alimentador .*/
  double maiorCarregamentoRede; /**< Armazena o valor do maior carregamento de rede do alimentador .*/
  double quedaMaxima; /**< Armazena o valo da maior queda de tensão do alimentador*/
  double perdasResistivas; /**< Armazena as perdas resistivas do alimentador.*/
  __complex__ double potenciaAlimentador; /**< Armazena a potência utilizada do alimentador.*/
  double menorTensao; /**< Armazena a menor tensão do alimentador.*/
  long int piorBarra; /**< Armazena a barra com piores indicadores.*/
  long int noMenorTensao; /**< Por Leandro: variável para armazenar o setor de uma RNP (alimentador) no qual encontra-se a barra com o menor valor de tensão nesta RNP.*/
  long int noMaiorCarregamentoRede; /**< Por Leandro: variável para armazenar no setor da RNP no qual encontra-se o trecho desta RNP com o maior carregamento de rede.*/
  double sobrecargaRede; /**< Por Leandro: variável para armazenar em Amperes o valor da sobrecarga de rede do trecho com o maior carregamento percentual no ALIMENTADOR (o quanto, em A, é o excedente ao valor limite superior).*/
  NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida;
}OBJETIVOSRNP;

/**
 * @brief Vetor da RNP.
 *
 * Esta estrutura é utilizada para armazenar uma árvore codificada na RNP.
 */
 typedef struct
 {
  int numeroNos; /**< Quantidade de setores do alimentador.*/
  NORNP *nos; /**< Vetor com os setores e as profundidades.*/
  OBJETIVOSRNP fitnessRNP; /**< Armazena os dados de fitness do alimentador.*/
}RNP;
/**
 * @brief EStruura os valores de availiação.
 *
 * Esta estrutura é utilizada para armazenar para uma árvore codificada na RNP os valores de fitness para ela.
 */

typedef struct
 {
  int seca; /**< manobras de chave seca(faca).*/
  int comCargaManual; /**< manobras de chave oleo e gas manual.*/
  int comCargaAutomatica; /**< manobras de chave com carga/gas automatica.*/
  int curtoManual; /**< manobras de chave com protecao manual.*/
  int curtoAutomatica; /**< manobras de chave com protecao a.*/
}MANOBRAS;

/*Estruturas do AE e do Problema*/

/**
 * @brief Estrutura dos objetivos.
 *
 * Estrutura para armazenar o valor de cada objetivo do indivíduo..
 */
typedef struct{
  double maiorCarregamentoRede; /**< Valor do maior carregamento de rede .*/
  double perdasResistivas; /**< Total das perdas resistivas da rede.*/
  double maiorCarregamentoTrafo; /**< Valor do maior carregamento de trafo.*/
  double maiorDemandaAlimentador; /**<Valor da maior demanda de alimentador da rede. */
  double menorTensao; /**< Valor da menor tensão da rede.*/
  int    manobrasManuais; /**< Número de manobras manuais para obter a configuração.*/
  int    manobrasAutomaticas; /**< Número de manobras automaticas para obter a configuração.*/
  int    manobrasRestabelecimento; /**<Número de manobras relizadas após o processo de BE e Evolutivo (quando necessário).*/
  int    manobrasAlivio; /**<Número de manobras realizadas para alívio de carga.*/
  int    manobrasAposChaveamento; /**<Número de manobras realizadas após o processo da Sequência de Chaveamento.*/
  double quedaMaxima; /**< Valor da maior queda de tensão do sistema*/
  __complex__ double *potenciaTrafo; /**< Armazena o carregamento de cada um dos trafos no SDR.*/
  double ponderacao; /**< Valor da função de ponderação dos objetivos*/
  int rank; /**< Valor da classificação da solução calculado de acordo com a formulação do SPEA2*/
  int fronteira; /**<Valor da fronteira de dominância*/
  long int consumidoresSemFornecimento; /**<Número total de consumidores sem fornecimento*/
  long int consumidoresEspeciaisSemFornecimento; /**<Número total de consumidores especiais sem fornecimento*/
  long int consumidoresEspeciaisTransferidos; /**< Armazena a quantide de consumidores especiais transferidos pelo PAO ou CAO.*/
  long int consumidoresDesligadosEmCorteDeCarga; /**<Número total de consumidores desligados por necessidade de Corte de Carga.*/
  MANOBRAS contadorManobrasTipo; /**<EStrutura de dados contendo os valores de manobras para cada subtipo de chave*/
  long int noMenorTensao; /**< Por Leandro: variável para armazenar o setor da Região Problema no qual encontra-se a barra com o menor valor de tensão dentro da Região Problema.*/
  long int noMaiorCarregamentoRede; /**< Por Leandro: variável para armazenar o setor da Região Problema no qual encontra-se o trecho desta região com o maior carregamento de rede.*/
  long int idTrafoMaiorCarregamento; /**< Por Leandro: variável para armazenar o trafo mais carregado da rede.*/
  double sobrecargaRede; /**< Por Leandro: variável para armazenar em AMPERES o valor da sobrecarga de rede do trecho com o maior carregamento percentual no ALIMENTADOR (o quanto, em A, é o excedente ao valor limite superior).*/
  double sobrecargaTrafo; /**< Por Leandro: variável para armazenar em AMPERES o valor da sobrecarga de trafo do trafo com o maior carregamento percentual dentre os trafos da região do problema (I.E, a quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo com o maior carregamento dentre os trafos da região do problema).*/
  double tempo; /**< Por Leandro: variável para armazenar o tempo total ESTIMADO necessario para operar todas as chaves para obter a configuração. A unidade de tempo é a mesma das variáveis globais "tempoOperacaoChaveManual" e "tempoOperacaoChaveAutomatica"*/
  double tempoBruto; /**< Por Leandro: esta variável também armazena o tempo total ESTIMADO necessario para operar todas as chaves para obter a configuração. Mas, diferentemente da variável "tempo", esta variável não irá descontar o tempo quando há operações que retornam um chave ao seu estado anterior. Uma vez que este tempo é utilizado na determinação da Energia Não Suprida, a consideração das manobras repetidas e a minização da mesma irá priorizar configurações com sequência de chavemento com menos manobras repetidas, isto é, melhores. A unidade de tempo é a mesma das variáveis globais "tempoOperacaoChaveManual" e "tempoOperacaoChaveAutomatica"*/
  double potenciaTotalNaoSuprida; /**< Por Leandro: potência ATIVA TOTAL não suprida (somatório da potência ativa de todas as cargas saudáveis e reconectáveis desligadas*/
  double energiaTotalNaoSuprida;/**< Por Leandro: ENERGIA (ativa) não suprida (Somatório do Produto entre potência ativa não suprida e tempo após cada manobra*/
  NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida;  /**< Por Leandro: potência ATIVA não suprida para cada nível de prioridade*/
  NIVEISDEPRIORIDADEATENDIMENTO energiaNaoSuprida; /**< Por Leandro: energia ATIVA não suprida para cada nível de prioridade*/
}OBJETIVOS;

/**
 * @brief Estrutura dos dados elétricos da configuração.
 *
 * Estrutura para armazenar os dados elétricos da configuração que serão utilizados para calcular o fluxo de carga. É um campo do indivíduo.
 * Cristhian: adicionada a dimensão tripla para considerar os dados nas três fases.
 * Cristhian: adicionada a variável do tipo TIPOFASES para considerar  as fases conectadas ao nó
 */
typedef struct{
  __complex__ double *corrente[3]; /**<Armazena os valores de corrente em cada trecho do SDR.*/
  __complex__ double *iJusante[3];/**<Armazena os valores de corrente jusante do SDR.*/
  __complex__ double *vBarra[3];/**<Armazena os valores cálculados de potência das barras.*/
  __complex__ double *potencia[3];/**<Armazena os valores de potência.*/
  TIPOFASES *tipoFases;
}DADOSELETRICOS;

/**
 * @brief Estrutura do indivíduo.
 *
 * Esta estrutura contém os campos que representam um indivíduo no algoritmo evolutivo.
 * É utilizada para compor o vetor de soluções que representam uma população
 */

typedef struct{
 long int idConfiguracao; /**< identificador do indivíduo.*/
 int numeroRNP; /**< Quantidade de RNPs (árvores) do indivíduo.*/
 int numeroRNPFicticia; /**< Por Leandro> Quantidade de RNPs Fictícias do indivíduo.*/
 DADOSELETRICOS dadosEletricos; /**< Armazena os dados elétricos utilizados no cálculo do fluxo de carga.*/
 OBJETIVOS objetivo; /**< Valores do indivíduo para os objetivos.*/
 RNP *rnp; /**< Vetor de ponteiros para RNPs que compõem a solução.*/
 RNP *rnpFicticia; /**< Por Leandro: Vetor de ponteiros para as RNPs Fictícias que armazenarão os nós cortados em cada solução.*/
 int *idRnpFalta; /**< Vetor de dinâmico de inteiros com os índices das RNPs utilizadas no restabelecimento.*/
 int numeroRNPsFalta; /**< Número de RNPs que tiveram setores manobrados em decorrência da falta*/
 //long int *ranqueamentoRnpsFicticias; /**< Salva um ranqueamento das RNPs Fictícias em ordem decrescente do valor de potência não suprida considerando níveis de prioridade de atendimento. Este será utilizado para guiar a aplicação do operador LRO, de reconexão de carga*/
 NIVEISDEPRIORIDADEATENDIMENTO potenciaTransferida;  /**< Por Leandro: potência ATIVA tranferida para cada nível de prioridade (tranferida entre alimentadores) para ober uma nova configuração a partir de seu ancestral. Nestas variáveis não são acumuladas a potência transferida para obtenção de indivíduos ancestrais*/
}CONFIGURACAO;


/**
 * @brief Adaptação da Estrutura da RNP para armazenar a RNP de setor.
 *
 * Esta estrutura é utilizada para armazenar a RNP de um setor dada a origem do fluxo de energia.
 */
typedef struct
 {
  int numeroNos; /**< Número de nós do setor.*/
  NORNP *nos; /**< Vetor de RNP do setor.*/
  long int idSetorOrigem; /**< Identificador do setor raiz.*/
}RNPSETOR;

/**
 * @brief Armazena a lista de RNPs de cada setor do SDR.
 *
 * Esta estrutura é utilizada para armazenar a lista de RNPs de cada setor identificado do SDR.
 */

typedef struct{
 long int idSetor; /**< Identificador do setor.*/
 int numeroRNPs; /**< Número de setores adjacentes.*/
 RNPSETOR *rnps; /**< Vetor de RNPs.*/
}RNPSETORES;

/**
 * @brief Armazena o grafo de setores (lista de adjacências de setores).
 *
 * Esta estrutura é utilizada para armazenar a relação de adjacências dos setores do SDR
 * na forma de grafo. Essa lista é consultada pelos operadores da RNP na escolha dos nós.
 */
typedef struct{
 long int idSetor;/**< Inteiro que identifica o setor. */
 long int numeroConsumidores; /**<Número TOTAL de consumidores do setor*/
 long int numeroConsumidoresEspeciais;/**<Número de consumidores especiais do setor*/
 int numeroAdjacentes;/**< Número de setores adjacentes. */
 long int *setoresAdjacentes; /**< Lista de setores adjacentes. */
 long int *idChavesAdjacentes; /**< Identificador da chave que liga os setores adjacentes na lista de chaves. */
 int classificacao; /**< Indica a classificação do Setor - 0:SAR - 1:Folha S/ NA - 2:Folha C/ NA - 3:CAR. */
 BOOL setorFalta; /**<Indica se o setor está em falta ou sem possibilidade de restabelecimento*/
 BOOL setorJusanteFalta; /* <Variável booleana para identificar os setores que estavam à jusante do setor em falta e que foram, portanto, diretamente afetados (desligados)*/
}GRAFOSETORES;


/**
 * @brief Armazena os valores de ampacidade para cada par de barra.
 *
 * Esta estrutura é utilizada para armazenar a célula de valores de máximo de corrente para cálculo do fluxo de carga.
 */

typedef struct{
 long int idNo; /**< Identificador da barra do SDR. */
 double valor; /**< Valor double do máximo de corrente para o trecho. */
}CELULACORRENTE;

/**
 * @brief Armazena o máximo de corrente das linhas.
 *
 * Esta estrutura é utilizada para armazenar a matriz de valores máximos de corrente para cálculo do fluxo de carga.
 */

typedef struct{
 long int idNo; /**< Inteiro que identifica cada o barra destino. */
 int numeroAdjacentes; /**< Inteiro que identifica a quantidade de nós adjacentes. */
 CELULACORRENTE *noAdjacentes; /**< Armazena os valores complexos para cada nó adjacente. */
}MATRIZMAXCORRENTE;

/**
 * @brief Armazena os valores complexos para cada para de barra.
 *
 * Esta estrutura é utilizada para armazenar a célula de valores complexos para cálculo do fluxo de carga.
 * Cristhian: adicionada dimensão 3x3 da matriz impedância para casos trifásicos 
 */

typedef struct{
 long int idNo; /**< Inteiro que identifica cada barra destino no SDR. */
 __complex__ double valor[3][3]; /**< Valor complexo com a impedância. */
}CELULACOMPLEXA;

/**
 * @brief Armazena a matriz de valores complexos.
 *
 * Esta estrutura é utilizada para armazenar a matriz de de valores complexos para cálculo do fluxo de carga.
 */

typedef struct{
 long int idNo; /**< Inteiro que identifica cada barra de origem. */
 int numeroAdjacentes; /**< Inteiro que identifica a quantidade de barras adjacentes. */
 CELULACOMPLEXA *noAdjacentes; /**< Armazena os valores complexos de impedância para cada barra adjacente. */
}MATRIZCOMPLEXA;

/**
 * @brief Armazena as informações de cada chave do SDR.
 *
 * Esta estrutura é utlizada para armazenar as informações de cada chave do SDR com a inserção do identificador interno.
 */
typedef struct {
  long int idNoDe;/**< Inteiro que identifica cada barra origem da chave no SDR. */
  long int idNoPara;/**< Inteiro que identifica cada barra destino da chave no SDR. */
  long int identificador; /**< Identificador interno único para cada chave. */
  char codOperacional[15]; /**< Número operacional das chaves. */
  TIPOARESTA tipoChave; /**< Tipo da chave no SDR. */
  ESTADOCHAVE estadoChave; /**< Estado da chave no momento inicial do processo, aberta ou fechada. */ 
  CONDICAO condicao; /**< Condição operacional da chave. */
  SUBTIPOARESTA subTipoChave; /**< Subtipo da chave no SDR. */
} LISTACHAVES;



/**
 * @brief Armazena as informações das opções para restabelecimento das partes sãs.
 *
 * Esta estrutura é utilizada para armazenar todas as opções de restabelecimento que existem para um determinado trecho são afetado pela falta.
 */
typedef struct {
  long int *noR;/**< Array de Inteiro para armazenar os setores raiz. */
  long int *noA;/**< Array de Inteiro para armazenar os setores destino. */
  long int *noP;/**< Array de Inteiro para armazenar os setores de corte. */
  int *rnpA; /**< Array de Inteiro para armazenar os alimentadores de destino. */
  int *rnpAFicticia; /**< Array de Inteiro para armazenar os alimentadores de destino. */
  long int numeroNos; /**< Número de opções possíveis para o restabelecimento. */ 
} NOSRESTABELECIMENTO;

/**
 * @brief Estrutura para criação das tabelas do algoritmo AEMT.
 *
 * Esta estrutura é utilizada para armazenar as subpopulações, chamadas de tabelas referentes a cada objetivo. As tabelas armazenam
 * o identificador do indivíduo e o valor do objetivo referente a tabela.
 */
typedef struct {
  long int idConfiguracao; /**<Identificador da configuração*/
  double valorObjetivo; /**<Valor do objetivo da configuração para a tabela*/
  double distanciaMultidao; /**<Valor da distância de multidão (Crowding de distance)*/
} TABELA;

/**
 * @brief Estrutura para criação das tabelas do algoritmo AEMT.
 *
 * Esta estrutura é utilizada para armazenar as subpopulações, chamadas de tabelas referentes a cada objetivo. As tabelas armazenam
 * o identificador do indivíduo e o valor do objetivo referente a tabela.
 */
typedef struct {
  long int numeroIndividuos; /**<Quantidade de indivíduos inseridos na tabela*/
  long int tamanhoMaximo; /**<Quantidade máxima de indivíduos que podem ser inseridos na tabela*/
  TABELA *tabela; /**<Ponteiro do tipo tabela para armazenar o vetor com os individuos da tabela*/
  double torneioValorReferencia; //Valor indicado para computar a quantidade de inclusoes nas demais tabelas de individuos gerados a partir da tabela atual (utilizado no torneio de tabelas)
} VETORTABELA;


/**
 * @brief Estrutura de dados para definir a sequência de chaveamento.
 *
 * Esta estrutura é utilizada para armazenar as configurações intermediárias e subsequências de chaveamento a partir da configuração resultante do MRAN.
 */
typedef struct lChav{
    CONFIGURACAO *configuracao; /**< Ponteiro do tipo CONFIGURACAO que armazena a estrutura do SDR no formato da RNP*/
    long int *idChaves; /**< Vetor com o identificador das chaves na ordem que devem ser manobradas*/
    long int consumidoresCorteCarga; /**< Quantidade de consumidores sem fornecimento devido a corte de carga ou não terem sido ligados ainda*/
    ESTADOCHAVE *estadoFinal; /**< Vetor indicando qual é o estado final da chave, se a chave deve ser aberta ou fechada*/
    BOOL *reguladorTensao; /**< Vetor indicando se existe regulador de tensão no trecho manobrado*/
    long int *chavesRestabFechar; /**< Vetor com o identificador das chaves que ainda precisam ser manobradas para restabelecer as partes sãs*/ 
    int *paresManobras; /**< Vetor com o índice da lista de pares de manobras que ainda estão foram inseridas na lista de manobras*/
    struct lChav *anterior; /**<Ponteiro para a estrutura de chaveamento anterior*/
    struct lChav *proximo; /**< Ponteiro para a próxima estrutura de chaveamento*/
    int numeroChaveamentos; /**< Número de chaveamentos que foram inseridos no vetor*/ //quantos chaveamentos foram realizados no total
    int numChavesRestab; /**< Quantidade de chaves para restabelecimento dos setores sãos que ainda não foram manobradas*/ //quantas chaves do restabelecimento ainda precisam ser fechadas
    int numPares; /**< Quantidade de pares de manobras que ainda precisam ser manobradas*/ //quantos pares de manobras ainda precisam ser aplicados.
}ELEMENTO;
/**
 * @brief Estrutura de dados para armazenar a lista encadeada utilizada na sequência de chaveamento.
 *
 * Esta estrutura é utilizada para gerenciar a lista encadeada da sequência de chaveamento.
 */
typedef struct lista{
    ELEMENTO *inicio; /**< Ponteiro do tipo ELEMENTO que indica o inicio da lista*/
    ELEMENTO *fim; /**< Ponteiro do tipo ELEMENTO que indica o fim da lista*/
    long int tamanho; /**< Número de elementos inseridos na lista*/
}LISTA;


/**
 * @brief Estrutura de dados para armazenar os indivíduos presentes numa fronteira de Pareto
 */
typedef struct{
	long int idConfiguracao;
	  double valorObjetivo1;
	  double valorObjetivo2;
 }INDIVIDUOSNAODOMINADOS;

 /**
  * @brief Estrutura de dados para armazenar fronteiras de Pareto
  *
  *@param nivelDominancia - nível de dominância da fronteira. Por exemplo, para a fronteira formada por solução não dominadas, "nivelDominancia" é zero
  *@param *individuos - vetor que armazena os indivíduos presentes na fronteira com nível de dominância "nivelDominancia"
  *@param numeroIndividuos - número de indivíduos presentes nesta fronteira de Pareto
  */
typedef struct
{
	int nivelDominancia;
	INDIVIDUOSNAODOMINADOS *individuos;
	int numeroIndividuos;
}FRONTEIRAS;

#endif // GERADORRNP_H_INCLUDED
