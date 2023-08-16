/**
 * @file funcoesSetor.c
 * @brief Este arquivo contém a implementação das funções para identificação dos setores e manipulação das estruturas de setores.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoesSetor.h"
#include "funcoesInicializacao.h"


int idSetor = 0; /*< Variável global que calcula o id dos setores **/
int numeroSetoresAdj; /*< Variável global para cálculo do número de adjacentes **/
int numeroNosSetor;/*< Variável global para cálculo do número de nos do setor **/
long int numeroConsumidores; /*< Variável global para cálculo do número de consumidores do setor **/
long int numeroConsumidoresEspeciais; /*< Variável global para cálculo do número de consumidores especiais do setor **/
int numeroSetoresAlimentador; /*< Variável global para cálculo do número de setores do alimentador **/
long int numeroSetores;
extern long int numeroAlimentadores;

/**
 * @brief Função para adição de uma estrutura NOSETOR.
 *
 * Essa função realiza a alocação de memória da estrutura NOSETOR e insere seus valores iniciais.
 * 
 * @param setor é um ponteiro para o ponteiro da estrutura do tipo NOSETOR, onde é retornado a adição do novo elemento na lista ou sua alocação inicial.
 * @param idNo é um inteiro com o valor a ser adicionado ao campo idNo do NOSETOR.
 * @return Tipo void sem retorno.
 * 
 * @note
 * @warning .
 */
void adicionaNo(NOSETOR **setor, long int idNo)
{
    NOSETOR *novoSetor;
    
    novoSetor = (NOSETOR *) malloc(sizeof(NOSETOR));
    
    if(novoSetor == NULL) exit(EXIT_FAILURE);
    
    novoSetor->idNo = idNo;
    novoSetor->prox = *setor;
    
    *setor = novoSetor;
}

/**
 * @brief Função para impressão em tela das barras adicionadas a um setor.
 *
 * Essa função realiza a impressão em tela de todas as barras que foram agrupadas em um setor percorrendo a lista ligada de barras.
 * 
 * @param setor é um ponteiro da estrutura do tipo NOSETOR com o elemento inicial da lista de barras.
 * @return Tipo void sem retorno.
 * 
 * @note
 * @warning .
 */
void imprimeSetor(NOSETOR * setor)
{
    NOSETOR * umSetor = setor;
    printf("ids:\t");
    while(umSetor != NULL)
    {
        printf("%ld\t", (umSetor->idNo));
        umSetor = umSetor->prox;
    }
    printf("\n");
}
/**
 * @brief Função para alocação e inicialização de um elemento da estrutura LISTASETORES.
 *
 * Essa função realiza a alocação de memória da estrutura LISTASETORES e insere seus valores iniciais. 
 * Também realiza a inserção do elemento da posição correta da lista de setores que está sendo construída.
 * 
 * @param listaSetores é um ponteiro para o ponteiro da estrutura do tipo LISTASETORES, onde é retornado a alocação do elemento.
 * @param setor é um ponteiro para a lista ligada de barras do setor.
 * @param noAdj é um ponteiro para a lista ligada de barras adjacentes do setor.
 * @return Tipo void sem retorno.
 * 
 * @note
 * @warning .
 */
void adicionaSetorNaLista(LISTASETORES **listaSetores, NOSETOR *setor, NOSETOR *noAdj){
    LISTASETORES *novaLista = NULL;
    LISTASETORES *aux = NULL;
    
    novaLista = (LISTASETORES *) malloc(sizeof(LISTASETORES));
    
    if(novaLista == NULL) exit(EXIT_FAILURE);
    
    novaLista->setor = setor;
    novaLista->setorAdj = noAdj;
    novaLista->idSetor = idSetor;
    novaLista->numeroAdjacentes = numeroSetoresAdj;
    novaLista->numeroNos = numeroNosSetor;
    novaLista->consumidoresEspeciais = numeroConsumidoresEspeciais;
    novaLista->consumidoresNormais = numeroConsumidores;
    novaLista->prox = NULL;
    
    numeroSetoresAlimentador++;
    aux = *listaSetores;
    
    if(aux == NULL) {
        *listaSetores = novaLista;
    }
    else {
        while(aux->prox != NULL)
            aux = aux->prox;

        aux->prox = novaLista;
    }
}

/**
 * @brief Função para impressão de todos os elementos do tipo LISTASETORES.
 *
 * Essa função imprime em tela todos os elementos da LISTASETORES com as suas barras.
 * 
 * @param listaSetores é um ponteiro da estrutura do tipo LISTASETORES para a cabeça da lista ligada.
 * @return Tipo void sem retorno.
 * 
 * @note
 * @warning .
 */
void imprimeSetores(LISTASETORES * listaSetores){
    LISTASETORES * setores = listaSetores;

    while(setores != NULL)
    {
        printf("----------------------------------------------\n");
        printf("setor: %ld numero nos %d \n", setores->idSetor, setores->numeroNos);
        imprimeSetor(setores->setor);
        printf("barras adjacentes: %d \n", setores->numeroAdjacentes);
        imprimeSetor(setores->setorAdj);
        setores = setores->prox;
    }
    printf("----------------------------------------------");
}

/**
 * @brief Função para inserir barra na fila de barras a serem analisadas.
 *
 * Essa função realiza a alocação de memória da estrutura FILABARRAS, inicializa os dados e insere o novo elemento na posição correta da fila.
 * A Fila de barras é utilizada para armazenar quais barras ainda precisam ser analisadas na detecção dos setores.
 * 
 * @param fila é um ponteiro para o ponteiro da estrutura do tipo FILABARRAS, onde é retornada a adição do novo elemento na fila ou sua alocação inicial.
 * @param idNo é um inteiro com o valor a ser adicionado ao campo idNo do FILABARRAS.
 * @return Tipo void sem retorno.
 * 
 * @note
 * @warning .
 */
void adicionaNoNaFila(FILABARRAS ** fila, long int idNo) {
    FILABARRAS *novoVertice = NULL;
    FILABARRAS *aux = NULL;
 
    novoVertice = (FILABARRAS *)malloc(sizeof(FILABARRAS));
 
    if(novoVertice == NULL) {
        printf("erro insere_fila\n");
        exit(EXIT_FAILURE);
    }
    
    novoVertice->idNo = idNo;
    novoVertice->prox = NULL;
    
    if(*fila == NULL)
        *fila = novoVertice;
    else {
        aux = *fila;
        while(aux->prox !=NULL) aux = aux->prox;
        aux->prox = novoVertice;
    }
}
/**
 * @brief Função para remoção de um elemnto da fila de barras.
 *
 * Essa função realiza a remoção de um elemento da fila de barras e atualiza os ponteiros.
 * 
 * @param fila é um ponteiro para o ponteiro da estrutura do tipo FILABARRAS, onde é feito o controle da cabeça da fila.
 * @return inteiro com o identificador contido no elemento removido.
 * 
 * @note
 * @warning .
 */
int retiraNoDaFila(FILABARRAS ** fila) {
    FILABARRAS *aux = NULL;
    
    int idNo = -1;
    
    aux = *fila;
    
    if(aux != NULL) *fila = aux->prox;
    
    idNo = aux->idNo;
    
    free(aux);
 
    return idNo;
}


/**
 * @brief Função para verificar se a fila não está vazia.
 *
 * Essa função verifica se o ponteiro passado como parâmetro é NULL. Se o ponteiro for null retorna false, caso contrário verdadeiro.
 * 
 * @param fila é um ponteiro para um elemento da estrutura do tipo FILABARRAS.
 * @return BOOL indicando se a lista contém elementos ou não.
 * 
 * @note
 * @warning .
 */
BOOL filaNaoVazia(FILABARRAS * fila) {
     if(fila == NULL)
         return false;
     return true;
}

/**
 * @brief Função para verificar se a aresta é do tipo trecho.
 *
 * Essa função verifica se a ligação entre duas barras é um trecho. Se for trecho retorna verdadeiro, caso contrário falso.
 * 
 * @param adjacente do tipo NOADJACENTE.
 * @return BOOL indicando se o parâmetro informado é trecho ou não.
 * 
 * @note
 * @warning .
 */
BOOL ligadoPorTrecho(NOADJACENTE adjacente) {
    TIPOARESTA tipoAresta = adjacente.tipoAresta;
    if(tipoAresta == trecho || tipoAresta == reguladorTensao) return true;
    return false;
}

/**
 * @brief Função para verificar se a aresta é do tipo CHAVEMANUAL.
 *
 * Essa função verifica se a ligação entre duas barras é uma CHAVEMANUAL. Se for CHAVEMANUAL retorna verdadeiro, caso contrário falso.
 * 
 * @param adjacente do tipo NOADJACENTE.
 * @return BOOL indicando se o parâmetro informado é CHAVEMANUAL ou não.
 * 
 * @note
 * @warning .
 */
BOOL ligadoPorChaveManual(NOADJACENTE adjacente) {
    TIPOARESTA tipoAresta = adjacente.tipoAresta;
    if(tipoAresta == chaveManual) return true;
    return false;
}

/**
 * @brief Função para verificar se a chave informada está fechada.
 *
 * Essa função verifica se a chave está fechada. Se for chave fechada retorna verdadeiro, caso contrário falso.
 * 
 * @param adjacente do tipo NOADJACENTE com a barra adjacente do setor.
 * @return BOOL indicando se o parâmetro informado é uma chave fechada ou não.
 * 
 * @note
 * @warning .
 */
BOOL chaveFechada(NOADJACENTE adjacente) {
    ESTADOCHAVE estado = adjacente.estadoChave;
    if(estado == normalmenteFechada) return true;
    return false;
}

/**
 * @brief Função que faz a identificação de um setor.
 *
 * Essa função a partir da barra inicial informada executa uma busca pelas barras adjacentes de forma recursiva até ter visitado todas que são ligadas por trecho.
 * 
 * @param grafo ponteiro para estrutura do tipo GRAFO que contém os dados das barras.
 * @param idNoRaiz identificador do tipo inteiro para a barra inicial do setor.
 * @param listaSetores ponteiro para ponteiro da estrutura do tipo LISTASETORES que armazena os setore já identificados.
 * @param visitado ponteiro para um vetor de BOOL que armazena se a barra já foi analisada ou não.
 * @return tipo void.
 * 
 * @note
 * @warning .
 */
void buscaSetor(GRAFO * grafo, long int idNoRaiz, LISTASETORES ** listaSetores, BOOL * visitado) {
    long int i, barraAdj, idNo;    
    
    NOSETOR * setorAtual = NULL;
    NOSETOR * barrasAdj = NULL;
    
    FILABARRAS * filaTrecho = NULL;
    FILABARRAS * filaChaveFechada = NULL;
    
    adicionaNo(&setorAtual, idNoRaiz);
    adicionaNoNaFila(&filaTrecho, idNoRaiz);
    idSetor++;
    //printf("%d \t", idSetor);
    numeroConsumidores=grafo[idNoRaiz].priorizacoes.qtdConsumidores;
    numeroConsumidoresEspeciais=grafo[idNoRaiz].priorizacoes.qtdConsumidoresEspeciais;
    numeroSetoresAdj = 0;
    numeroNosSetor = 1;
    while(filaNaoVazia(filaTrecho))
    {
        idNo = retiraNoDaFila(&filaTrecho);
        GRAFO * no = &grafo[idNo];
        grafo[idNo].idSetor = idSetor;
        for(i = 0; i<no->numeroAdjacentes; i++)
        {
            barraAdj = no->adjacentes[i].idNo;
            if(ligadoPorTrecho(no->adjacentes[i]))
            {
                if(!visitado[barraAdj])
                {
                    adicionaNoNaFila(&filaTrecho, barraAdj);
                    adicionaNo(&setorAtual, barraAdj);
                    numeroNosSetor++;
                    numeroConsumidores+=grafo[barraAdj].priorizacoes.qtdConsumidores;
                    numeroConsumidoresEspeciais+=grafo[barraAdj].priorizacoes.qtdConsumidoresEspeciais;
                }
            }
            else
            {
                numeroSetoresAdj++;
                if(!visitado[barraAdj] && chaveFechada(no->adjacentes[i]))
                {
                    adicionaNoNaFila(&filaChaveFechada, barraAdj);
                }
                adicionaNo(&barrasAdj, barraAdj);
            }
        }
        visitado[no->idNo] = true;
    }
    adicionaSetorNaLista(listaSetores, setorAtual, barrasAdj);
    
    while(filaNaoVazia(filaChaveFechada))
    {
        idNo = retiraNoDaFila(&filaChaveFechada);
        buscaSetor(grafo, idNo, listaSetores, visitado);
    }
    
    
}

/**
 * @brief Função que faz a identificação de um setor de chaves automáticas.
 *
 * Similar a função BUSCASETOR (@see buscaSetor), porém considera chaves manuais como trechos. 
 * 
 * @param grafo ponteiro para estrutura do tipo GRAFO que contém os dados das barras.
 * @param idNoRaiz identificador do tipo inteiro para a barra inicial do setor.
 * @param listaSetores ponteiro para ponteiro da estrutura do tipo LISTASETORES que armazena os setore já identificados.
 * @param visitado ponteiro para um vetor de BOOL que armazena se a barra já foi analisada ou não.
 * @return tipo void.
 * 
 * @note
 * @warning .
 */
void buscaSetorAutomatica(GRAFO * grafo, long int idNoRaiz, LISTASETORES ** listaSetores, BOOL * visitado) {
    long int i, barraAdj, idNo;    
    
    NOSETOR * setorAtual = NULL;
    NOSETOR * barrasAdj = NULL;
    
    FILABARRAS * filaTrecho = NULL;
    FILABARRAS * filaChaveFechada = NULL;
    
    adicionaNo(&setorAtual, idNoRaiz);
    adicionaNoNaFila(&filaTrecho, idNoRaiz);
    idSetor++;
    numeroSetoresAdj = 0;
    numeroNosSetor = 1;
    while(filaNaoVazia(filaTrecho))
    {
        idNo = retiraNoDaFila(&filaTrecho);
        GRAFO * no = &grafo[idNo];
        grafo[idNo].idSetor = idSetor;
        for(i = 0; i<no->numeroAdjacentes; i++)
        {
            barraAdj = no->adjacentes[i].idNo;
            //barras ligadas por chave manual ou por trecho ou por regulador de tensão, devem compor o mesmo setor
            if(ligadoPorTrecho(no->adjacentes[i]) || ligadoPorChaveManual(no->adjacentes[i]))
            {
                if(!visitado[barraAdj])
                {
                    adicionaNoNaFila(&filaTrecho, barraAdj);
                    adicionaNo(&setorAtual, barraAdj);
                    numeroNosSetor++;
                }
            }
            else//barra ligada por chave automática, pertence a um novo setor adjacente 
            {
                numeroSetoresAdj++;
                if(!visitado[barraAdj] && chaveFechada(no->adjacentes[i]))
                {
                    adicionaNoNaFila(&filaChaveFechada, barraAdj);
                }
                adicionaNo(&barrasAdj, barraAdj);
            }
        }
        visitado[no->idNo] = true;
    }
    adicionaSetorNaLista(listaSetores, setorAtual, barrasAdj);
    
    while(filaNaoVazia(filaChaveFechada))
    {
        idNo = retiraNoDaFila(&filaChaveFechada);
        buscaSetor(grafo, idNo, listaSetores, visitado);
    }
    
    
}

/**
 * @brief Função principal para identificação dos setores do SDR.
 *
 * Essa função percorre a lista de alimentadores do sistema chamando para cada alimentador a função buscaSetor (@see buscaSetor)
 * tendo como barra inicial do processo a barra do alimentador.
 * 
 * @param grafo ponteiro para estrutura do tipo GRAFO que contém os dados das barras.
 * @param alimentadores ponteiro para o vetor do tipo da estrutura DADOSALIMENTADOR com os dados dos alimentadores do SDR.
 * @param numeroBarras inteiro com a quantidade de barras no SDR.
 * @return um ponteiro para a lista de setores identificados do tipo LISTASETORES.
 * 
 * @note
 * @warning .
 */
LISTASETORES * buscaSetores(GRAFO *grafo, DADOSALIMENTADOR *alimentadores, long int numeroBarras) {
    int i, idAlim;
    LISTASETORES * lista_setores = NULL;
    
    BOOL visitado[(numeroBarras+1)];
    
    for(i=1; i<=numeroBarras; i++) 
        visitado[i] = false;
    
    for(i=1; i<=numeroAlimentadores; i++)
    {
      // printf("\n alimentador %d :",i);
        numeroSetoresAlimentador = 0;
        idAlim = alimentadores[i].barraAlimentador;
        buscaSetor(grafo, idAlim, &lista_setores, visitado);
        alimentadores[i].numeroSetores = numeroSetoresAlimentador;
    }
    numeroSetores = idSetor;
    return lista_setores;
}

/**
 * @brief Função principal para identificação dos setores de automaticas do SDR.
 *
 * Essa função percorre a lista de alimentadores do sistema chamando para cada alimentador a função buscaSetorAutomatica (@see buscaSetorAutomatica)
 * tendo como barra inicial do processo a barra do alimentador.
 * 
 * @param grafo ponteiro para estrutura do tipo GRAFO que contém os dados das barras.
 * @param alimentadores ponteiro para o vetor do tipo da estrutura DADOSALIMENTADOR com os dados dos alimentadores do SDR.
 * @param numeroBarras inteiro com a quantidade de barras no SDR.
 * @return um ponteiro para a lista de setores identificados do tipo LISTASETORES.
 * 
 * @note
 * @warning .
 */

LISTASETORES * buscaSetoresAutomatica(GRAFO *grafo, DADOSALIMENTADOR *alimentadores, long int numeroBarras) {
    int i, idAlim;
    LISTASETORES * lista_setores = NULL;
    
    BOOL visitado[(numeroBarras+1)];
    
    for(i=1; i<=numeroBarras; i++) 
        visitado[i] = false;
    
    for(i=1; i<=numeroAlimentadores; i++)
    {
        numeroSetoresAlimentador = 0;
        idAlim = alimentadores[i].barraAlimentador;
        buscaSetorAutomatica(grafo, idAlim, &lista_setores, visitado);
        alimentadores[i].numeroSetores = numeroSetoresAlimentador;
    }
    numeroSetores = idSetor;
    return lista_setores;
}

/**
 * @brief Função que verifica se uma barra é adjacente a outra.
 *
 * Essa função percorre a lista de adjacentes de uma barra verificando se outra barra informada como parâmetro está na lista.
 * 
 * @param grafoSdrParam ponteiro para estrutura do tipo GRAFO que contém os dados das barras.
 * @param idNoRaiz inteiro com o identificador da barra que deseja procurar o adjancente.
 * @param idNoAdj inteiro com o identificador da barra que seré verificada.
 * @return um BOOL indicando se a barra pertence (true) ou não (false) a lista de adjacentes.
 * 
 * @note
 * @warning .
 */
BOOL estaListaAdjacencias(GRAFO *grafoSdrParam, long int idNoRaiz, long int idNoAdj)
{
    int contador;
    for(contador =0; contador < grafoSdrParam[idNoRaiz].numeroAdjacentes; contador++)
    {
        if(grafoSdrParam[idNoRaiz].adjacentes[contador].idNo == idNoAdj)
            return true;
    }
    return false;
}

/**
 * 
 * Essa função recursiva que percorre a lista de barras do setor obtendo as RNPs. É gerada uma RNP a partir da barra inicial informada do setor com base em uma busca em profundidade.
 * 
 * @param inicioListaSetor ponteiro do tipo NOSETOR para a primeira barra da lista encadead de barras do setor.
 * @param idNo inteiro com o identificador da barra inicial para gerar a rnp do setor.
 * @param visitado ponteiro para um BOOL indicando se o nó foi visitado.
 * @param rnpParam ponteiro para o vetor do tipo RNP onde são armazenadas as barras e suas profundidades. 
 * @param indice ponteiro para inteiro que contém a posição do vetor onde será inserido o próximo elemento.
 * @param profundidade inteiro com o valor da próxima profundidade.
 * @param grafoSDRParam ponteiro para vetor da estrutura GRAFO contendo o SDR.
 * @return void.
 * 
 * @note
 * @warning .
 */
void constroiRNP(NOSETOR *inicioListaSetor, long int idNo, BOOL *visitado, RNPSETOR *rnpParam, int *indice, int profundidade, GRAFO *grafoSDRParam)
{
    //Atribui o no e a sua profundidade
        int idNoAdj;
        rnpParam->nos[indice[0]].idNo = idNo;
        rnpParam->nos[indice[0]].profundidade = profundidade;
        visitado[idNo] = true;
        indice[0]++;
        profundidade++;
        NOSETOR *noSetor = inicioListaSetor;
        while((noSetor!=NULL))
        {   
                if (estaListaAdjacencias(grafoSDRParam, idNo,  noSetor->idNo) && (visitado[noSetor->idNo]== false))
                {
                    idNoAdj= noSetor->idNo;
                    constroiRNP(inicioListaSetor, idNoAdj, visitado, rnpParam, indice, profundidade,grafoSDRParam);            
                } 
            
            noSetor = noSetor->prox;
        }
}

/**
 * 
 * Essa função percorre a lista de barras adjacentes do setor. Para cada barra adjacente constrói um RNP para o setor.
 * Faz uso da função @see constroiRNP.
 *  
 * @param setorParam ponteiro do tipo NOSETOR para a primeira barra da lista encadeada de barras do setor.
 * @param setorAdjParam ponteiro do tipo NOSETOR para a primeira barra da lista encadeada de barras adjacentes do setor.
 * @param grafoSDRParam ponteiro para vetor da estrutura GRAFO contendo o SDR.
 * @param rnps ponteiro para o vetor do tipo RNPSETOR onde são armazenadas as RNPs do setor.
 * @param numeroNos inteiro com o número de barras do setor.
 * @param numeroBarras inteiro para a quantidade de barras do SDR.
 * @return void.
 * 
 * @note
 * @warning .
 */
void percorreAdjacentesSetor(NOSETOR *setorParam, NOSETOR *setorAdjParam, GRAFO *grafoSDRParam, 
        RNPSETOR *rnps,  int numeroNos, long int numeroBarras)
{
    NOSETOR *noAdjacenteSetor = setorAdjParam;
    NOSETOR *noSetor = setorParam;
    BOOL visitado[numeroBarras+1];
    int contador;
    int contadorRNPs=0;
    //RNPSETOR *rnpSetor;
    BOOL encontrado = false;
    int idNo;
    int indiceRNP;
    int profundidade;
    while (noAdjacenteSetor != NULL)
    {
        for (contador =1; contador <=numeroBarras; contador++)
        {
            visitado[contador] = false;
        }
        
        //rnpSetor = Malloc(RNPSETOR,1);
        rnps[contadorRNPs].nos = Malloc(NORNP, (numeroNos+1));
        //Atribui o no e a sua profundidade
        rnps[contadorRNPs].nos[0].idNo = noAdjacenteSetor->idNo;
        rnps[contadorRNPs].nos[0].profundidade = 0;
        rnps[contadorRNPs].numeroNos = numeroNos+1;
        visitado[noAdjacenteSetor->idNo] = true;
        rnps[contadorRNPs].idSetorOrigem = grafoSDRParam[noAdjacenteSetor->idNo].idSetor;
        noSetor = setorParam;
        encontrado = false;
        while((noSetor!=NULL)&&(!encontrado))
        {
            int indice;
            for(indice =0; indice <grafoSDRParam[noAdjacenteSetor->idNo].numeroAdjacentes; indice++)
                if (grafoSDRParam[noAdjacenteSetor->idNo].adjacentes[indice].idNo == noSetor->idNo)
                {
                    encontrado = true;
                    idNo= noSetor->idNo;
                }
            noSetor = noSetor->prox;
        }
        //chamada da busca profundidade...
        indiceRNP = 1;
        profundidade = 1;
        constroiRNP(setorParam, idNo, visitado, &rnps[contadorRNPs], &indiceRNP, profundidade,grafoSDRParam);
        contadorRNPs++;
        noAdjacenteSetor = noAdjacenteSetor->prox;    
    }
}
/**
 * 
 * Essa função é responsável por percorrer todos os setores gerando as RNPs. 
 * Faz uso da função @see percorreAdjacentesSetor. 
 * 
 * @param listaSetores ponteiro do tipo LISTASETORES para o elemento inicial da lista de setores identificados.
 * @param grafoSDRParam ponteiro para vetor da estrutura GRAFO contendo o SDR.
 * @param rnpSetores ponteiro para ponteiro da estrutura RNPSETORES.
 * @param numeroBarras inteiro para a quantidade de barras do SDR.
 * @return void.
 * 
 * @note
 * @warning .
 */
void constroiRNPSetores(LISTASETORES *listaSetores, GRAFO *grafoSDRParam, RNPSETORES **rnpSetores, 
        long int numeroBarras)
{
    if (((* rnpSetores) = (RNPSETORES *)malloc( (numeroSetores+1) * sizeof(RNPSETORES)))==NULL)
    {
        printf("Não foi possível alocar a lista de RNP de setores");
        exit(1);
    }
    LISTASETORES * setores = listaSetores;
    int idNo;
    
    while(setores != NULL)
    {
        idNo = setores->setor->idNo;
        //O setor corresponde a um setor de alimentador
/*        if(grafoSDRParam[idNo].tipoNo == raizAlimentador)
        {
             (*rnpSetores)[setores->idSetor].rnps = Malloc(RNPSETOR, 1);
             (*rnpSetores)[setores->idSetor].idSetor = setores->idSetor;
             (*rnpSetores)[setores->idSetor].numeroRNPs = 1;
             (*rnpSetores)[setores->idSetor].rnps[0].nos = Malloc(NORNP, 1);
             //Atribui o no e a sua profundidade
             (*rnpSetores)[setores->idSetor].rnps[0].nos[0].idNo = idNo;
             (*rnpSetores)[setores->idSetor].rnps[0].nos[0].profundidade = 0;
             (*rnpSetores)[setores->idSetor].rnps[0].numeroNos = 1;
             (*rnpSetores)[setores->idSetor].rnps[0].idSetorOrigem = -1; //o setor alimentador não possui adjacente para origem de fluxo       
        }
        else //Não representa setor de alimentador
        {*/
           (*rnpSetores)[setores->idSetor].rnps = Malloc(RNPSETOR, (setores->numeroAdjacentes+1)); 
           (*rnpSetores)[setores->idSetor].idSetor = setores->idSetor;
           (*rnpSetores)[setores->idSetor].numeroRNPs = setores->numeroAdjacentes;
           percorreAdjacentesSetor(setores->setor, setores->setorAdj, grafoSDRParam, (*rnpSetores)[setores->idSetor].rnps, setores->numeroNos, numeroBarras);
        //}
   
        setores = setores->prox;
    }
}

/**
 * 
 * Essa função é responsável por imprimir na tela todas as RNPs de todos os setores. 
 * 
 * @param rnpSetoresParam ponteiro do tipo RNPSETORES para o vetor de RNP de todos os setores.
 * @return void.
 * 
 * @note
 * @warning .
 */
void imprimeRNPSetores (RNPSETORES *rnpSetoresParam)
{
    int contadorRNP;
    int contadorSetor;
    int contadorNos;
    
    for(contadorSetor = 1; contadorSetor<=numeroSetores; contadorSetor++)
    {
        printf("Identificador Setor: %ld \t ", rnpSetoresParam[contadorSetor].idSetor);
        printf("Numero de RNPs %d \n", rnpSetoresParam[contadorSetor].numeroRNPs);
        for(contadorRNP=0; contadorRNP < rnpSetoresParam[contadorSetor].numeroRNPs; contadorRNP++ )
        {
            printf("\t Identificador setor origem %ld \n", rnpSetoresParam[contadorSetor].rnps[contadorRNP].idSetorOrigem);
            printf(" \t \t Nos: ");
            for(contadorNos=0; contadorNos<rnpSetoresParam[contadorSetor].rnps[contadorRNP].numeroNos; contadorNos++)
            {
                printf("\t %ld", rnpSetoresParam[contadorSetor].rnps[contadorRNP].nos[contadorNos].idNo);
            }
            printf("\n \t \t Prof: ");
            for(contadorNos=0; contadorNos<rnpSetoresParam[contadorSetor].rnps[contadorRNP].numeroNos; contadorNos++)
            {
                printf("\t %d", rnpSetoresParam[contadorSetor].rnps[contadorRNP].nos[contadorNos].profundidade);
            }
            printf("\n");
        }
    }
}

/**
 * 
 * Essa função é responsável por gravar no arquivo rnpsetores.dad todas as RNPs de todos os setores. 
 * 
 * @param rnpSetoresParam ponteiro do tipo RNPSETORES para o vetor de RNP de todos os setores.
 * @param numeroSetores inteiro com a quantidade de setores.
 * @return void.
 * 
 * @note
 * @warning .
 */
void gravaRNPSetores (RNPSETORES *rnpSetoresParam, long int numeroSetores)
{
    int contadorRNP;
    int contadorSetor;
    int contadorNos;
    FILE *arquivo = fopen("rnpsetores.dad", "w");
    for(contadorSetor = 1; contadorSetor<=numeroSetores; contadorSetor++)
    {
        fprintf(arquivo,"Identificador\tSetor:\t%ld\n", rnpSetoresParam[contadorSetor].idSetor);
        for(contadorRNP=0; contadorRNP < rnpSetoresParam[contadorSetor].numeroRNPs; contadorRNP++ )
        {
                 
            fprintf(arquivo,"Origem\t%ld\n", rnpSetoresParam[contadorSetor].rnps[contadorRNP].idSetorOrigem);
            for(contadorNos=0; contadorNos<rnpSetoresParam[contadorSetor].rnps[contadorRNP].numeroNos; contadorNos++)
            {
                fprintf(arquivo,"%ld\t", rnpSetoresParam[contadorSetor].rnps[contadorRNP].nos[contadorNos].idNo);
            }
            fprintf(arquivo,"\n");
            for(contadorNos=0; contadorNos<rnpSetoresParam[contadorSetor].rnps[contadorRNP].numeroNos; contadorNos++)
            {
                fprintf(arquivo,"%d\t", rnpSetoresParam[contadorSetor].rnps[contadorRNP].nos[contadorNos].profundidade);
            }
            fprintf(arquivo,"\n");
        }
    }
    
    fclose(arquivo);
}

/**
 * 
 * Essa função é responsável por imprimir na tela todas as RNPs do setor informado. 
 * 
 * @param rnpSetorParam do tipo RNPSETORES com o elemento que deve ser impresso.
 * @return void.
 * 
 * @note
 * @warning .
 */
void imprimeRNPSetor (RNPSETOR rnpSetorParam)
{
    int contadorNos;
    
        
            printf("\t Identificador setor origem %ld \n", rnpSetorParam.idSetorOrigem);
            printf(" \t \t Nos: ");
            for(contadorNos=0; contadorNos<rnpSetorParam.numeroNos; contadorNos++)
            {
                printf("\t %ld", rnpSetorParam.nos[contadorNos].idNo);
            }
            printf("\n \t \t Prof: ");
            for(contadorNos=0; contadorNos<rnpSetorParam.numeroNos; contadorNos++)
            {
                printf("\t %d", rnpSetorParam.nos[contadorNos].profundidade);
            }
            printf("\n");
}

/**
 * 
 * Essa função é responsável por construir a lista de adjacências dos setores. 
 * Percorre a lista de setores identificados e a partir das barras adjacentes constrói a lista de setores vizinhos.
 * Também armazena para cada setor vizinho qual a chave do SDR por onde ocorre a ligação. 
 * 
 * @param grafoSDRParam ponteiro para o vetor do tipo GRAFO com a modelagem em grafo do SDR.
 * @param listaSetores ponteiro para o elemento inicial da lista do tipo LISTASETORES
 * @param listaChavesParam ponteiro para o vetor do tipo LISTACHAVES com a lista de chaves do SDR.
 * @param grafoSetoresParam ponteiro para ponteiro do tipo GRAFOSETORES onde será retornado a lista de adjecências dos setores.
 * @param numeroChaves inteiro com a quantidade de chaves do SDR.
 * @return void.
 * 
 * @note
 * @warning .
 */
void constroiListaAdjacenciaSetores(GRAFO *grafoSDRParam, LISTASETORES *listaSetores, 
        LISTACHAVES *listaChavesParam, GRAFOSETORES **grafoSetoresParam, long int numeroChaves)
{
    
    if (((*grafoSetoresParam) = (GRAFOSETORES *)malloc((numeroSetores+1)*sizeof (GRAFOSETORES)))==NULL)
    {
        printf("ERRO: alocacacao grafo da lista de adjacencias de setores!!!!");
        exit(1);
    }        
    
    NOSETOR *noSetorAdj;
    NOSETOR *noSetor;
    LISTASETORES * setores = listaSetores;
    int idNoSetor;
    int contador;
    BOOL encontrado;
    while(setores != NULL)
    {
         
        (*grafoSetoresParam)[setores->idSetor].idSetor = setores->idSetor;
        (*grafoSetoresParam)[setores->idSetor].setorFalta = false;
        (*grafoSetoresParam)[setores->idSetor].numeroAdjacentes = setores->numeroAdjacentes;
        (*grafoSetoresParam)[setores->idSetor].numeroConsumidores = setores->consumidoresNormais;
        (*grafoSetoresParam)[setores->idSetor].numeroConsumidoresEspeciais = setores->consumidoresEspeciais;
        (*grafoSetoresParam)[setores->idSetor].setoresAdjacentes = Malloc(long int, setores->numeroAdjacentes);
        (*grafoSetoresParam)[setores->idSetor].idChavesAdjacentes = Malloc(long int, setores->numeroAdjacentes);
        noSetorAdj = setores->setorAdj;
        contador = 0;
        while (noSetorAdj != NULL)
        {
            /* Explicação do if: Se a barra adjacente ao setor não for alimentada 
             * por nenhum alimentador/trafo ela não deve ser considerada como 
             * adjancete*/
            if(grafoSDRParam[noSetorAdj->idNo].idSetor != 0){
                (*grafoSetoresParam)[setores->idSetor].setoresAdjacentes[contador]= grafoSDRParam[noSetorAdj->idNo].idSetor;
                noSetor = setores->setor;
                encontrado = false;
                while(noSetor != NULL && !encontrado)
                {
                    if(estaListaAdjacencias(grafoSDRParam, noSetorAdj->idNo, noSetor->idNo))
                    {
                        encontrado = true;
                        idNoSetor = noSetor->idNo;
                    }
                    noSetor = noSetor->prox;
                }
                (*grafoSetoresParam)[setores->idSetor].idChavesAdjacentes[contador] = buscaChave(listaChavesParam,noSetorAdj->idNo, idNoSetor, numeroChaves);
                contador++;
            }
            else{
                (*grafoSetoresParam)[setores->idSetor].numeroAdjacentes --;
            }
            noSetorAdj = noSetorAdj->prox;
        }
        setores = setores->prox;
    }
}

/**
 * 
 * Essa função imprime na tela a lista de adjacências dos setores. 
 * 
 * @param grafoSetoresParam ponteiro do tipo GRAFOSETORES para o vetor com a lista de adjacências do grafo representado por setores.
 * @return void.
 * 
 * @note
 * @warning .
 */
void imprimeGrafoSetores(GRAFOSETORES *grafoSetoresParam)
{
    int contador;
    int contadorAdj;
    for(contador = 1; contador<=numeroSetores; contador++)
    {
        printf("Setor %ld \n", grafoSetoresParam[contador].idSetor);
        printf("\t Adjacentes");
        for(contadorAdj = 0; contadorAdj<grafoSetoresParam[contador].numeroAdjacentes; contadorAdj++)
        {
            printf("\t %ld", grafoSetoresParam[contador].setoresAdjacentes[contadorAdj]);
        }
        printf("\n \t Chaves");
        for(contadorAdj = 0; contadorAdj<grafoSetoresParam[contador].numeroAdjacentes; contadorAdj++)
        {
            printf("\t %ld", grafoSetoresParam[contador].idChavesAdjacentes[contadorAdj]);
        }
        printf("\n");    
    }
    
}

/**
 * 
 * Essa função grava no arquivo "grafosetores.dad" a lista de adjacências dos setores.
 * São impressos os setores, seus setores adjacentes e as chaves que ligam cada adjacente. 
 * 
 * @param grafoSetoresParam ponteiro do tipo GRAFOSETORES para o vetor com a lista de adjacências do grafo representado por setores.
 * @param numeroSetores inteiro com a quantidade de setores identificados
 * @param listaChavesParam ponteiro do tipo LISTACHAVES para o vetor da lista de chaves do SDR.
 * @return void.
 * 
 * @note
 * @warning .
 */
void gravaGrafoSetores(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, LISTACHAVES *listaChavesParam)
{
    int contador;
    int contadorAdj;
    FILE *arquivo = fopen("grafosetores.dad", "w");
    for(contador = 1; contador<=numeroSetores; contador++)
    {
        fprintf(arquivo,"Setor\t%ld\tConsumidores:\t%ld\tConsumidores\tEspeciais:\t%ld\n", grafoSetoresParam[contador].idSetor, grafoSetoresParam[contador].numeroConsumidores, grafoSetoresParam[contador].numeroConsumidoresEspeciais);
        fprintf(arquivo,"Adjacentes");
        for(contadorAdj = 0; contadorAdj<grafoSetoresParam[contador].numeroAdjacentes; contadorAdj++)
        {
            fprintf(arquivo,"\t%ld", grafoSetoresParam[contador].setoresAdjacentes[contadorAdj]);
        }
        fprintf(arquivo,"\nChaves");
        for(contadorAdj = 0; contadorAdj<grafoSetoresParam[contador].numeroAdjacentes; contadorAdj++)
        {
            fprintf(arquivo,"\t%ld", grafoSetoresParam[contador].idChavesAdjacentes[contadorAdj]);
        }
        fprintf(arquivo,"\nCodOp.\tChaves");
        for(contadorAdj = 0; contadorAdj<grafoSetoresParam[contador].numeroAdjacentes; contadorAdj++)
        {
            fprintf(arquivo,"\t%s", listaChavesParam[grafoSetoresParam[contador].idChavesAdjacentes[contadorAdj]].codOperacional);
        }
        fprintf(arquivo,"\n");
    }
    fclose(arquivo);
}

/**
 * 
 * Essa função faz a busca da RNP de um setor, dado o setor e o setor adjacente. 
 * 
 * @param rnpSetoresParam ponteiro do tipo RNPSETORES para o vetor com a lista RNPs do setores.
 * @param idSetor1 inteiro com identificador do setor para o qual deseja-se encontrar a RNP.
 * @param idSetor2 inteiro com o identificador do setor adjacente do idSetor1.
 * @return tipo RNPSETOR com a RNP do setor procurado.
 * 
 * @note
 * @warning .
 */
RNPSETOR buscaRNPSetor(RNPSETORES *rnpSetoresParam, long int idSetor1, long int idSetor2)
{
    int indice = 0;
    //realiza a busca pela rnp
    //printf("setor1 %ld setor2 %ld numeroRNPS %d\n", idSetor1, idSetor2,rnpSetoresParam[idSetor1].numeroRNPs);
    
    
    while (indice < rnpSetoresParam[idSetor1].numeroRNPs && rnpSetoresParam[idSetor1].rnps[indice].idSetorOrigem != idSetor2)
        indice++;
    if (indice < rnpSetoresParam[idSetor1].numeroRNPs){
        return rnpSetoresParam[idSetor1].rnps[indice];
    }
    
}

/**
 * 
 * Essa função realiza a liberação da memória alocada para armazenar as RNPs de setores. 
 * 
 * @param rnpSetores ponteiro do tipo RNPSETORES para o vetor com as RNPs dos setores.
 * @param numeroSetores inteiro com a quantidade de setores.
 * @return void.
 * 
 * @note
 * @warning .
 */
void desalocaRNPSetores(RNPSETORES *rnpSetores, long int numeroSetores)
{
    long int contadorSetor, contadorAdj;
    for(contadorSetor = 1; contadorSetor <= numeroSetores; contadorSetor++)
    {
        for(contadorAdj = 0; contadorAdj < rnpSetores[contadorSetor].numeroRNPs; contadorAdj++)
            free(rnpSetores[contadorSetor].rnps[contadorAdj].nos);
        free (rnpSetores[contadorSetor].rnps); 
    }
}

/**
 * 
 * Essa função realiza a liberação da memória alocada para armazenar a lista de adjacências de setores. 
 * 
 * @param grafoSetoresParam ponteiro do tipo GRAFOSETORES para o vetor com a lista de adjacências dos setores.
 * @param numeroSetores inteiro com a quantidade de setores.
 * @return void.
 * 
 * @note
 * @warning .
 */
void desalocaGrafoSetores(GRAFOSETORES *grafoSetoresParam, long int numeroSetores)
{
    
    long int contador;
    
    for(contador = 1; contador<=numeroSetores; contador++)
    {
         
        free(grafoSetoresParam[contador].setoresAdjacentes);
        free(grafoSetoresParam[contador].idChavesAdjacentes);
        
    }
}
