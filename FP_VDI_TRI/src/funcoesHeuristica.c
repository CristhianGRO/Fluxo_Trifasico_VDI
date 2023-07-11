/**
 * @file funcoesHeuristica.c
 * @brief Implementa a versão das operações da representação nó-profundidade com a inserção de heurísticas do problema de restabelecimento.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <complex.h>
#include <string.h>
#include "data_structures.h"
#include "funcoesRNP.h"
#include "funcoesSetor.h"
#include "funcoesProblema.h"
#include "funcoesChaveamento.h"
#include "funcoesHeuristica.h"
#include "funcoesAuxiliares.h"
#include "aemt.h"
#include "funcoesLeituraDados.h"

long int numeroExecucoesFluxoCarga; /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/
int tempoDeRestauracaoDaFalha; /**Por Leandro: salvará o Tempo total estimado necessário para a recupeção de todas as falhas. Em outras palavras, o tempo no qual a rede operará na configuração obtida através de plano de restabelecimento obtido e informado*/

extern int maximoTentativas; /**define o número máximo de tentativas na escolha dos nós para os operadores PAO e CAO*/
extern __complex__ double ij;/** constante para converter o valor double para a parte imaginária de um número completo*/
/**
 * Método que realiza a aplicação do operador PAO da RNP. Esse operador consiste na poda de uma subárvore de uma RNP
 * com o enxerto em outra RNP. Para isso, escolhe de forma aleatória uma rnp do vetor de alimentadores manobrados,
 * depois é selecionada, também de forma aleatória uma posição desso vetor da rnp e o nó contido nessa posição é chamado
 * de nó de poda (p). A seguir é escolhido de forma aleatória um nó da lista de adjacêntes de p no grafo e se este estiver
 * em outra rnp ele é chamada de nó adjacente (a). Definidos os nós é feita a transferência da subárvore entre as duas rnps.
 * 
 * Tem como parâmetros o vetor de configurações, a geração que está sendo executada, o identificador da configuração que irá
 * dar origem a nova solução, o grafo de setores, o vetor de matriz pi e a vetor pi.
  * 
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 */
/*void operadorPAOHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam, 
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam, 
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB, 
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, 
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, 
        long int *numChaveFechada)
{
    COLUNAPI *colunaNoA;
    long int noA, noP;
    int indiceL, indiceP, indiceA, indice, contador; 
    long int noRaizP;
    int rnpP, rnpA, indiceRnpP;
    long int idChaveAberta, idChaveFechada;
    int profundidadeA;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados;
    
    consumidoresManobrados = 0;
    //Determinação dos nós p,e a, para aplicação do operador
    //Executa até obter um nó a
    while (noA == 0) {
        //o no P sorteado nao pode ser a raiz da árvore
       // printf("idConfiguracao %ld, numeroRNPs %d \n",idConfiguracaoSelecionadaParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
        indiceRnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta - 1));
        rnpP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indiceRnpP];
        //verifica se a árvore possui mais de um nó para poder aplicar o operador
        if (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos > 1) {
            indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos - 1));
            noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
            //determinação do no A adjacente a R
           // printf("rnpP %d noP %ld \n",rnpP, noP);
            colunaNoA = determinaNoAdjacente(maximoTentativas, noP, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP, grafoSetoresParam, matrizPiParam, vetorPiParam);
            if (colunaNoA != NULL) { //se obteve um nó a válido recupera os dados dele.
                rnpA = colunaNoA[0].idRNP;
                indiceA = colunaNoA[0].posicao;
                noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;
                profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].profundidade;
            }
        }
    }
   //calcula o tamanho da subárvore podada
    tamanhoSubarvore = indiceL-indiceP+1;
    //localiza quais chaves serão aberta e fechada
    indice = indiceP-1;
    while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
    {
        indice--;
    }
    noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
    for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
    {
        if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
            idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
        else
            if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noA)
                idChaveFechada = grafoSetoresParam[noP].idChavesAdjacentes[indice];
    }
    //realiza a alocação das que serão alteradas RNPs 
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
    //constroi a rnp de origem nova
    constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
    //constroi a rnp de destino nova
   // constroiRNPDestinoPAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);
    constroiRNPDestino(popConfiguracaoParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceP, matrizPiParam, 
            idConfiguracaoSelecionadaParam, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], idNovaConfiguracaoParam); 
    //calcula o número de manobras
    numeroManobrasModificada(popConfiguracaoParam,
        idChaveAberta, idChaveFechada, listaChavesParam, idConfiguracaoSelecionadaParam,
        estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
    //salva os setores utilizados na manobra, nesse caso os setore P e R são os mesmos.
    nosPRA[0].a = noA;
    nosPRA[0].p = noP;
    nosPRA[0].r = noP;
    //insere no vetor pi o ancestral do novo individuo obtido
    atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam, 
            &idChaveAberta, &idChaveFechada,1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA);
    //copia o ponteiro para as rnps que não foram alteradas
    for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
    {
        if (indice != rnpP && indice != rnpA)
        {
            popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
        }
    }
    copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
    insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);
    avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, rnpP, idNovaConfiguracaoParam,
            dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP, 
            indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
            ZParam, maximoCorrenteParam, numeroBarrasParam, false);
    
    //Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
    for(contador = 0; contador < tamanhoSubarvore; contador++)
    {
        if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA)
        {
            consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
        }
        else
        {
            consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
        }
    }
    popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;

}*/

/**
 * Método que realiza a aplicação do operador CAO da RNP. Esse operador consiste na poda de uma subárvore de uma RNP
 * com o enxerto em outra RNP, a partir de outro nó da subárvore podada. Para isso, escolhe de forma aleatória uma rnp do vetor de alimentadore manobrados,
 * depois é selecionada, também de forma aleatória uma posição desso vetor da rnp e o nó contido nessa posição é chamado
 * de nó de poda (p). No próximo passo escolhe um nó aleatório da subárvore podada para ser a nova raiz (r). A seguir é escolhido de forma aleatória um nó da 
 * lista de adjacentes de r no grafo e se este estiver
 * em outra rnp ele é chamada de nó adjacente (a). Definidos os nós é feita a transferência da subárvore entre as duas rnps.
 * 
 * Tem como parâmetros o vetor de configurações, a geração que está sendo executada, o identificador da configuração que irá
 * dar origem a nova solução, o grafo de setores, o vetor de matriz pi e a vetor pi.
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 */
/*void operadorCAOHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam, 
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam, 
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB, 
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam,
        long int *numChaveAberta, long int *numChaveFechada)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noRaizP;
    int indiceL, indiceP, indiceR, indiceA, indice, contador;
    long int idChaveAberta, idChaveFechada;
    int rnpP, rnpA, indiceRnpP;
    int profundidadeA;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados;
    
    consumidoresManobrados = 0;
    
    //Determinação dos nós p,r e a, para aplicação do operador
    //Executa até obter um nó a
    while (noA == 0) {
        //o no P sorteado nao pode ser a raiz da árvore
        indiceRnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta - 1));
        rnpP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indiceRnpP];
        //verifica se a árvore possui mais de um nó para poder aplicar o operador
        if (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos > 1) {
            indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos - 1));
            noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
            //determinação do no R
            if (indiceL == indiceP) //nesse caso não existe outro nó para religar a subárvore
                indiceR = indiceP;
            else
                indiceR = inteiroAleatorio(indiceP, indiceL);
           // printf("indiceR %d indiceP %d indiceL %d \n", indiceR, indiceP, indiceL);
            noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceR].idNo;
            //determinação do no A adjacente a R
            colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP, grafoSetoresParam, matrizPiParam, vetorPiParam);
            if (colunaNoA != NULL) {
                //recupera os dados do nó a selecionado
                rnpA = colunaNoA[0].idRNP;
                indiceA = colunaNoA[0].posicao;
                noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;
                profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].profundidade;
            }
        }
    }
   //localiza quais chaves serão aberta e fechada
    indice = indiceP-1;
    while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
    {
        indice--;
    }
    noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
    for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
    {
        if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
            idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
         }
    
    for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
    {
            if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
                idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
    }
    
    //printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
    //calcula o tamanho da subárvore
    tamanhoSubarvore = indiceL-indiceP+1;
    //faz a alocação das novas RNPs
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
    //Obtém a nova rnp de origem, sem a subárvore podada
    constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
    //Obtém a nova rnp de destino, inserindo a subárvore podada
    constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);
    
   //copia os ponteiros das rnps não alteradas
    for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
    {
        if (indice != rnpP && indice != rnpA)
        {
            popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
        }
    }
    copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
    insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);
    //imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
    numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
      idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
    //salva os setores utilizados na manobra
    nosPRA[0].a = noA;
    nosPRA[0].p = noP;
    nosPRA[0].r = noR;
    atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
            &idChaveAberta, &idChaveFechada,1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA);
    avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, rnpP, idNovaConfiguracaoParam,
            dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP, 
            indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
            ZParam, maximoCorrenteParam, numeroBarrasParam, false);
    
    //Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
    for(contador = 0; contador < tamanhoSubarvore; contador++)
    {
        //printf("\nQtdAntes: %ld - idAnt: %ld - idNova: %ld - idNo: %ld - Config(0): %d - Para: %d - a: %ld - p: %ld - Valor: %ld - ", popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoSelecionadaParam, idNovaConfiguracaoParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo, matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP, rnpA, noA, noP, grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais);
        if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA)
        {
            //printf("igual - ");
            consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
            //popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
        }
        else
        {
            //printf("diferente - ");
            consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
            //popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
        }
    }
    popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;
    //printf("\nQtdDepois: %ld - Valor: %ld", popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos, consumidoresManobrados);
}*/

/**
 * Por Leandro: consiste na função "processoEvolutivoHeuristica()" modificada para:
 * 1) realizar todas as adequações necessárias a obtenção da metodologia proposta.
 * 2) para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRAALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 * 3) para que as variáveis "configuracoesParam" e "vetorPiParam" sejam passados referência e não mais por valor. Isto se deve ao fato de que, em determinada circunstância,
 * pode ser necessário alocar mais memória para estas duas variáveis. Caso os parâmentros continuassem sendo passados por valor, ao concluir a execução desta função em questão,
 * as novas posições alocadas e valores salvos seriam desconsiderados
 * 4) efetuar a passagem de "numeroTotalManobrasRestauracaoCompletaSemAlivioParam"
 * 5) deixar de efetuar a passagem de "numeroMinimoManobrasManuais"
 *
 *
 * Descrição da função "processoEvolutivoHeuristica()":
 *  Esta é uma das funções principais do MRAN responsável por implementar o algoritmo evolutivo multiobjetivo em tabelas (subpopulações).
 * O principal parâmetro desse método são as soluções geradas pela heurística de busca local exaustiva para restabelecimento.
 * Essas soluções são inseridas nas subpopulações e inicia o loop iterativo de geração de novas soluções. 
 * Esse loop tem como critério de parada o número máximo de soluções a serem geradas passado no parâmetro maximoGeracoes.
 * A cada loop uma nova solução é obtida a partir de uma solução atual selecionada por torneio por meio da aplicação ou do operador PAO ou do operador CAO.
 * A nova solução obtida é depois inserida ou não (caso seja pior) em uma ou mais subpopulações definidas. 
 * @param dadosTrafoParam
 * @param dadosAlimentadorParam
 * @param configuracoesParam
 * @param populacaoParam
 * @param grafoSetoresParam
 * @param matrizSetoresParam
 * @param listaChavesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param maximoCorrenteParam
 * @param ZParam
 * @param numeroBarras
 * @param numeroTrafos
 * @param estadoInicial
 * @param maximoGeracoes
 * @param numeroTabelas
 * @param taxaOperadorPAO
 * @param idConfiguracao
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param numChaveAberta
 * @param numChaveFechada
 * @param numeroIndividuos
 *@param numeroTotalManobrasRestauracaoCompletaSemAlivioParam
 *@param grafoSDRParam
 *@param idPrimeiraConfiguracaoParam
 *@param sequenciaManobrasAlivioParam
 *@param numeroPosicoesAlocadasParam
 *@param numeroSetoresParam
 *@param numeroSetorFaltaParam
 */
void processoEvolutivoHeuristicaModificada(DADOSTRAFO *dadosTrafoParam,
        DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO **configuracoesParam, VETORTABELA *populacaoParam, GRAFOSETORES *grafoSetoresParam,
        RNPSETORES *matrizSetoresParam, LISTACHAVES *listaChavesParam, MATRIZPI *matrizPiParam,
        VETORPI **vetorPiParam, MATRIZMAXCORRENTE *maximoCorrenteParam, MATRIZCOMPLEXA *ZParam,
        long int numeroBarras, int numeroTrafos, ESTADOCHAVE *estadoInicial,
        long int maximoGeracoes, int numeroTabelas,double taxaOperadorPAOParam,
        long int idConfiguracao, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, 
        long int *numChaveAberta, long int *numChaveFechada,  long int maximoGeracoesSemAtualizacaoPopulacaoParam, int numeroTotalManobrasRestauracaoCompletaSemAlivioParam,
		GRAFO *grafoSDRParam, long int idPrimeiraConfiguracaoParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam,
		long int *numeroPosicoesAlocadasParam, long int numeroSetoresParam, int numeroSetorFaltaParam, long int *numeroConfiguracoesGeradasParam, long int *geracoesExecutadas) {
    double probabilidadeAplicacao;
	long int geracao, idConfiguracaoSelecionada, idConfiguracaoAuxiliar, indiceConfiguracao, contadorGeracoesSemAtualizacaoPopulacao;
    int  sucessoAplicacaoOperador,  operadorAplicado; //Por Leandro
    BOOL LROAplicado, flagManobrasAlivio, sequenciaChaveamentoFactivel, flagAtualizacaoPopulacao; //Por Leandro

    /* PROCESSO EVOLUTIVO*/
    geracao = 0;
    contadorGeracoesSemAtualizacaoPopulacao = 0; //Conta o número de gerações sem haver a inserção de indivíduos nas tabelas
    while (geracao < maximoGeracoes &&  contadorGeracoesSemAtualizacaoPopulacao < maximoGeracoesSemAtualizacaoPopulacaoParam) {

    	flagAtualizacaoPopulacao = false;
    	operadorAplicado = 0; // a variável "operadorAplicado" assume: 0 - se ainda não foi aplicado nenhum operador; -1 - se houve uma tentativa fracassada de aplicar um operador; 1 - se um operador foi aplicadao
      	while(operadorAplicado != 1){

			idConfiguracaoSelecionada = selecionaConfiguracao(populacaoParam, numeroTabelas);

			LROAplicado = false;
			if((*configuracoesParam)[idConfiguracaoSelecionada].objetivo.potenciaTotalNaoSuprida > 0){ //Se há alguma carga (potência ativa) fora de serviço
				operadorAplicado = operadorLROHeuristica((*configuracoesParam), idConfiguracao, idConfiguracaoSelecionada,
											grafoSetoresParam, matrizPiParam, (*vetorPiParam), indiceRegulador, dadosReguladorSDR,
											dadosAlimentadorParam, dadosTrafoParam, numeroTrafos,
											matrizSetoresParam, ZParam, maximoCorrenteParam, numeroBarras,estadoInicial,
											listaChavesParam, numChaveAberta, numChaveFechada, grafoSDRParam,
											false, -1, -1, -1, -1, -1, sequenciaManobrasAlivioParam);
				LROAplicado = true;
			}


			// O PAO e o CAO serão aplicados em três situações: (i) quando não houver cargas a serem reconectadas; (ii) quando, por algum motivo, o LRO não foi aplicado;
			// (iii) quando houverem cargas a serem reconectadas e chaves NAs permetindo-o, mas a aplicação do LRO resultou num indivíduo infactível relaxada
			if(((*configuracoesParam)[idConfiguracaoSelecionada].objetivo.potenciaTotalNaoSuprida == 0) || (operadorAplicado == -1) || (verificaFactibilidade((*configuracoesParam)[idConfiguracao], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR) == false)){
				if((operadorAplicado == 1) && (verificaFactibilidade((*configuracoesParam)[idConfiguracao], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR) == false)){
					idConfiguracao++;
			    	//Verificar se é necessário ALOCAR mais posições nas variáveis "configuracoesParam" e "vetorPiParam" (isto faz-se necessário se for gerado mais de um indivíduo por geração
			      	if(idConfiguracao >= numeroPosicoesAlocadasParam[0] - 100){ //Se for verdade, então a grande maioria das posições alocadas em "configuracoesParam" e "vetorPiParam" já foram utilizadas. Logo, é necessário alocar mais posições
			    		realocaVetorPi(numeroPosicoesAlocadasParam[0], (numeroPosicoesAlocadasParam[0] + (maximoGeracoes - geracao) + 1000), &(*vetorPiParam));
			    		realocaIndividuo(numeroPosicoesAlocadasParam[0], (numeroPosicoesAlocadasParam[0] + (maximoGeracoes - geracao) + 1000), &(*configuracoesParam), idPrimeiraConfiguracaoParam, numeroTrafos);
			    		numeroPosicoesAlocadasParam[0] = numeroPosicoesAlocadasParam[0] + (maximoGeracoes - geracao) + 1000;
			    	}
				}

				operadorAplicado = 0;
				LROAplicado = false;
				probabilidadeAplicacao = (double) rand() / (double) RAND_MAX;
				if (probabilidadeAplicacao <= taxaOperadorPAOParam) //aplica o operador PAO
				{
					operadorAplicado = operadorPAOHeuristicaMelhorada((*configuracoesParam), idConfiguracao, idConfiguracaoSelecionada,
													 grafoSetoresParam, matrizPiParam, (*vetorPiParam), indiceRegulador, dadosReguladorSDR,
													 dadosAlimentadorParam, dadosTrafoParam, numeroTrafos,
													 matrizSetoresParam, ZParam, maximoCorrenteParam, numeroBarras,
													 estadoInicial, listaChavesParam, numChaveAberta, numChaveFechada, grafoSDRParam, sequenciaManobrasAlivioParam); //Por Leandro: a função "operadorPAOHeuristicaMelhorada()" passou a ser chamada no lugar da função "operadorPAOHeuristica()"
				}
				else //aplica o operador CAO
				{
					operadorAplicado = operadorCAOHeuristicaMelhorada((*configuracoesParam), idConfiguracao, idConfiguracaoSelecionada,
													grafoSetoresParam, matrizPiParam, (*vetorPiParam), indiceRegulador, dadosReguladorSDR,
													dadosAlimentadorParam, dadosTrafoParam, numeroTrafos,
													matrizSetoresParam, ZParam, maximoCorrenteParam, numeroBarras,
													estadoInicial,listaChavesParam, numChaveAberta, numChaveFechada, grafoSDRParam, sequenciaManobrasAlivioParam); //Por Leandro: a função "operadorCAOHeuristicaMelhorada()" passou a ser chamada no lugar da função "operadorCAOHeuristica()"
				}
			}
      	}

		// Verifica se alguma chave retornou ao seu estado inicial e tenta corrigir a sequência de chaveamento, isto é, excluir tal(is) chave(s)
		idConfiguracaoAuxiliar = idConfiguracao;
		if(verificaFactibilidade((*configuracoesParam)[idConfiguracao], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR)){
			sequenciaChaveamentoFactivel = true;
			verificaSequenciaChaveamento(grafoSDRParam, numeroBarras, dadosAlimentadorParam, dadosTrafoParam, maximoCorrenteParam,
					ZParam, matrizSetoresParam, grafoSetoresParam, numeroSetores, numeroSetorFaltaParam, numeroTrafos, numeroAlimentadores,
					indiceRegulador, dadosReguladorSDR, (*vetorPiParam), (*configuracoesParam), matrizPiParam, &idConfiguracaoAuxiliar, listaChavesParam,
					estadoInicial, sequenciaManobrasAlivioParam, &numeroPosicoesAlocadasParam[0], idPrimeiraConfiguracaoParam, &sequenciaChaveamentoFactivel);
		}
		(*vetorPiParam)[idConfiguracao].sequenciaVerificada = true;

		//Insere o indivíduo "idConfiguracao" gerado por um dos operadores de reprodução
		flagManobrasAlivio = determinaFlagManobrasAlivio((*configuracoesParam), matrizPiParam, (*vetorPiParam), idConfiguracao);
		insereTabelasModificadaV2(populacaoParam, (*configuracoesParam), idConfiguracao, idPrimeiraConfiguracaoParam, numeroTotalManobrasRestauracaoCompletaSemAlivioParam, sequenciaManobrasAlivioParam, flagManobrasAlivio, (*vetorPiParam), &flagAtualizacaoPopulacao);
		//Insere os indivíduos que foram gerados pelo procedimento de verificação da Sequência de Chaveamento de "idConfiguracao". Estes, por terem sido gerados por tal procedimento, já possem garantida a factibilidade de sua sequência de manobras
		for(indiceConfiguracao = idConfiguracao + 1; indiceConfiguracao <= idConfiguracaoAuxiliar; indiceConfiguracao++){
			flagManobrasAlivio = determinaFlagManobrasAlivio((*configuracoesParam), matrizPiParam, (*vetorPiParam), indiceConfiguracao);
			insereTabelasModificadaV2(populacaoParam, (*configuracoesParam), indiceConfiguracao, idPrimeiraConfiguracaoParam, numeroTotalManobrasRestauracaoCompletaSemAlivioParam, sequenciaManobrasAlivioParam, flagManobrasAlivio, (*vetorPiParam), &flagAtualizacaoPopulacao);
		}
		idConfiguracao = idConfiguracaoAuxiliar;


		//Ajusta a taxa de probabilidade de aplicação do PAO e CAO
		if(LROAplicado == false){ //Só ajusta a taxa se o operador aplicado foi o PAO ou o CAO
			sucessoAplicacaoOperador = 0;
			if(verificaFactibilidade((*configuracoesParam)[idConfiguracao], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR))
				sucessoAplicacaoOperador = 1; //Considera-se que o operador aplicado (dentre o PAO e CAO) tenha tido sucesso se a solução gerada for factível relaxada
			taxaOperadorPAOParam = ajustarTaxaDeProbabilidadeAplicacaoOperadores(probabilidadeAplicacao, taxaOperadorPAOParam, sucessoAplicacaoOperador);
		}

      	//Verifica se algum indivíduo foi salvo em alguma tabela nesta geração
      	if(flagAtualizacaoPopulacao)
      		contadorGeracoesSemAtualizacaoPopulacao = 0;
      	else
      		contadorGeracoesSemAtualizacaoPopulacao++;


        geracao++;
        idConfiguracao++;
    	//Verificar se é necessário alocar mais posições nas variáveis "configuracoesParam" e "vetorPiParam" (isto faz-se necessário se for gerado mais de um indivíduo por geração
      	if(idConfiguracao >= numeroPosicoesAlocadasParam[0] - 100){ //Se for verdade, então a grnade maioria das posições alocadas em "configuracoesParam" e "vetorPiParam" já foram utilizadas. Logo, é necessário alocar mais posições
    		realocaVetorPi(numeroPosicoesAlocadasParam[0], (numeroPosicoesAlocadasParam[0] + (maximoGeracoes - geracao) + 1000), &(*vetorPiParam));
    		realocaIndividuo(numeroPosicoesAlocadasParam[0], (numeroPosicoesAlocadasParam[0] + (maximoGeracoes - geracao) + 1000), &(*configuracoesParam), idPrimeiraConfiguracaoParam, numeroTrafos);
    		numeroPosicoesAlocadasParam[0] = numeroPosicoesAlocadasParam[0] + (maximoGeracoes - geracao) + 1000;
    	}

    }
    numeroConfiguracoesGeradasParam[0] = idConfiguracao - 1;
    geracoesExecutadas[0] = geracao;
}

/**
 * Esse método realiza a avaliação da nova configuração para os objetivos carregamento de rede, carregamento de trafo, perdas e queda de tensão.
 * O presente método considera para a avaliação os valores dessas variáveis apenas para os alimentadores que estão participando do processo de restabelecimento.
 * Isto é, somente que alimentados que receberam setores, ou tiveram setores manobrados devido a falha no SDR.
 * @param todosAlimentadores
 * @param configuracoesParam
 * @param rnpA
 * @param rnpP
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param idAntigaConfiguracaoParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param copiarDadosEletricos
 */
/*void avaliaConfiguracaoHeuristica(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, 
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, 
        DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam, 
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos) {
    long int contador;
    int idRnp;
    double quedaMaxima, menorTensao, VF, perdasTotais;
    perdasTotais = 0;
    //verifica se deve manter os dados elétricos para cada configuração gerada individualmente. Para isso é necessário copiar os dados elétricos de uma configuração para outra
    if (idNovaConfiguracaoParam != idAntigaConfiguracaoParam) {
        //não mantém os dados elétricos individualmente, somente copia os ponteiros para os vetores com dados elétricos
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante;
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente;
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia;
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra;

    }
  //  configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    
    //configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = 0;
    if (todosAlimentadores) //calcula os valores de fitness para todos os alimentadores
    {
        //avalia todos os alimentadores da lista 
        for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; contador++) {
            idRnp = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[contador];
            VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[idRnp + 1].idTrafo].tensaoReal / sqrt(3);
            fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, idRnp, 
                    idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
                    indiceRegulador, dadosReguladorSDR, 1);
        }
    } else { //calcula o fluxo de carga somente para os alimentadores que foram alterados.
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpP, 
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam, 
                indiceRegulador, dadosReguladorSDR, 1);
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpA, 
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam, 
                indiceRegulador, dadosReguladorSDR,1);
    }
    for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; contador++) {
        idRnp = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[contador];
        quedaMaxima = 100000;
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[idRnp+1].idTrafo].tensaoReal/sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.menorTensao;
        if (quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.demandaAlimentador;
        //máximo de corrente
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.maiorCarregamentoRede)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.maiorCarregamentoRede;
    }
    for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNP; contador++) {
        idRnp = contador;
        perdasTotais+=configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.perdasResistivas;
    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
    carregamentoTrafo(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);
    calculaPonderacao(configuracoesParam, idNovaConfiguracaoParam, VF);
}*/


/**
 * Por Leandro
 * Descrição: esta função consiste na função "avaliaConfiguracaoHeuristica()" modificada
 *
 * 1) para que seja calculado o Fluxo de Carga nas Árvores Origem ('rnpP') e Destino ('rnpA')
 * somente quando rnpP e rnpA, respectivamente, forem maiores ou iguais a zero. Isto permitirá
 * que, quando forem executados os operadores LSO e LRO, o fluxo de carga seja
 * executa em apenas uma das árvores, uma vez que, no caso destes operadores, a outra
 * árvore é fictícia;
 *
 * 2) para que seja chamada a função "calculaPotenciaAtivaNaoSupridaHeuristica()"
 *
 * 3) para que seja chamada a função "carregamentoTrafoHeuristicaModificada()"
 *
 * 4) para que seja chamada a função "potenciaAtivaNaoSuprida()", que calcula, por nível de prioridade
 * o somatório de potência ativa possível ser suprida mas não atendida
 *
 * 5) para que seja passado como parâmentro a variável "grafoSDRParam", na qual estão salvas as informações do nível de prioridade de cada barra
 *
 * 6)para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRASALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 *
 * 7) para que seja passado como parametro a variável booleana "flagManobrasAlivioParam", a qual informa se a configuração sob avaliação foi ou não gerada pela
 * operação de um par de manobras. Isto é necessário para o cálculo da ENS.
 *
 *P.S.: o Tempo de operação normal e Bruto das chaves é calculado pela função que determina o número de manobras e não por esta função
 *
 * @param todosAlimentadores
 * @param configuracoesParam
 * @param rnpA
 * @param rnpP
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param idAntigaConfiguracaoParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 *@param copiarDadosEletricos
 *@param grafoSDRParam
 *@param sequenciaManobrasAlivioParam
 *@param listaChavesParam
 *@param vetorPiParam
 *@param flagManobrasAlivioParam
 *
 */
void avaliaConfiguracaoHeuristicaModificada(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam,
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador,
        DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos,
		GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam,
		LISTACHAVES *listaChavesParam, VETORPI *vetorPiParam, BOOL flagManobrasAlivioParam) {
    long int contador;
    int idRnp;
    double quedaMaxima, menorTensao, VF, perdasTotais;
    long int noMenorTensaoRNP, noMaiorCarregamentoRNP;
    perdasTotais = 0;
    //verifica se deve manter os dados elétricos para cada configuração gerada individualmente. Para isso é necessário copiar os dados elétricos de uma configuração para outra
    if (idNovaConfiguracaoParam != idAntigaConfiguracaoParam) {
        //não mantém os dados elétricos individualmente, somente copia os ponteiros para os vetores com dados elétricos
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante;
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente;
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia;
        configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra;

    }
  //  configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;

    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTotalNaoSuprida = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta = 0; // por Leandro:

    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaTotalNaoSuprida = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta = 0; // por Leandro:

    //configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = 0;
    if (todosAlimentadores) //calcula os valores de fitness para todos os alimentadores
    {
        //avalia todos os alimentadores da lista
        for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; contador++) {
            idRnp = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[contador];
            VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[idRnp + 1].idTrafo].tensaoReal / sqrt(3);
            fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, idRnp,
                    idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
                    indiceRegulador, dadosReguladorSDR, 1);
        }
    } else { //calcula o fluxo de carga somente para os alimentadores que foram alterados.
    	if(rnpP >= 0){
			VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3);
			fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpP,
					idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
					indiceRegulador, dadosReguladorSDR, 1);
    	}
    	if(rnpA >= 0){
			VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3);
			fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpA,
					idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
					indiceRegulador, dadosReguladorSDR,1);
    	}
    }
    for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; contador++) {
        idRnp = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[contador];
        quedaMaxima = 100000;
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[idRnp+1].idTrafo].tensaoReal/sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.menorTensao;
        noMenorTensaoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMenorTensao;
        noMaiorCarregamentoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMaiorCarregamentoRede;

        if (quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMenorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMenorTensao; //Por Leandro
        }
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.demandaAlimentador;
        //máximo de corrente
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.maiorCarregamentoRede){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.maiorCarregamentoRede;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMaiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMaiorCarregamentoRede;//Por Leandro:
            configuracoesParam[idNovaConfiguracaoParam].objetivo.sobrecargaRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.sobrecargaRede;//Por Leandro:
        }
    }
    for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNP; contador++) {
        idRnp = contador;
        perdasTotais+=configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.perdasResistivas;
    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
    carregamentoTrafoHeuristicaModificada(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);
    calculaPonderacao(configuracoesParam, idNovaConfiguracaoParam, VF);
    calculaPotenciaAtivaNaoSupridaPorNivelPrioridade(configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, matrizB, grafoSDRParam, flagManobrasAlivioParam);  // Por Leandro: Calcula a potência ativa não suprida na nova configuração gerada
    calculaEnergiaAtivaNaoSupridaPorNivelPrioridade(configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, flagManobrasAlivioParam);  // Por Leandro: Calcula a energia ativa não suprida na nova configuração gerada
}

/**
 * Calcula o carregamento de trafo considerando somente os alimentadores envolvidos no processo de restabelecimento.
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param dadosAlimentadorParam
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param idAntigaConfiguracaoParam
 * @param todosAlimentadores
 * @param idRNPOrigem
 * @param idRNPDestino
 */
/*void carregamentoTrafoHeuristica(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        DADOSALIMENTADOR *dadosAlimentadorParam, 
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam, 
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino)
{
  int indiceI,indiceJ;
  int idTrafo;
  int idRnp;
  double potencia;
  double carregamentoTrafo;
  carregamentoTrafo=0;
//realiza o cálculo para todos os alimentadores
    if (todosAlimentadores) {
        for(indiceI = 1; indiceI <= numeroTrafosParam; indiceI++)
        {
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = 0+ij*0;
        }
        for(indiceJ = 0; indiceJ<configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; indiceJ++)
        {
            idRnp = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[indiceJ];
            idTrafo = dadosAlimentadorParam[idRnp+1].idTrafo;
            //a sequencia de rnps obedece a mesma sequencia de alimentadores com a diferença de uma posição 
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.potenciaAlimentador;
        }
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {       
           potencia = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI]) / (dadosTrafoParam[indiceI].capacidade * 10000); //(10000=1MVA/100) em porcentagem
            if (carregamentoTrafo < potencia)
                carregamentoTrafo = potencia;
          //
        // printf("capacidade %.2lf potencia do trafo  %lf carregamento %.2lf \n", dadosTrafoParam[indiceI].capacidade, cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI])/1000000, potencia);
        } 
        
        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = carregamentoTrafo;
        
    }
    else //atualiza os valores considerando apenas os alimentadores que tiveram modificação
    {
        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = configuracoesParam[idAntigaConfiguracaoParam].objetivo.maiorCarregamentoTrafo;
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {       
           
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = configuracoesParam[idAntigaConfiguracaoParam].objetivo.potenciaTrafo[indiceI];
          //verifica se o alimentador da RNP do alimentador de origem pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1. 
            if(dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPOrigem+1].idTrafo)
           {
               configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
               configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
           }
            //verifica se o alimentador da RNP do alimentador de destino pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
           if(dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPDestino+1].idTrafo)
           {
               configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
               configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
           } 
           //calcula a potencia que está sendo utilizada em porcentagem da capacidade
           potencia = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI]) / (dadosTrafoParam[indiceI].capacidade * 10000); //10000=1MVA*100%
           //obtem o maior carregamento de trafo da rede em porcentagem 
           if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo < potencia)
                configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = potencia;
        }        
    }
}*/

/* Por Leandro
 * Descrição: esta função consiste na função "carregamentoTrafoHeuristica()" modificada para
 * 1) No cálculo do carregamento de Trafo sejam considerados somente Transformadores que atendem alimentadores pertencentes
 * à região do problema, visto que a função "carregamentoTrafo()" não considera a região a região do problema;
 * 2) Seja cálculado e salvo o valor de Potência Aparente (em VA) que excede o limite superior de potência do trafo com o maior carregamento percentual
 * 3) Que as Árvores Origem ('rnpP') e Destino ('rnpA') sejam considera somente quando
 * rnpP e rnpA, respectivamente, forem maiores ou iguais a zero.
 * Isto permitirá que, quando forem executados os operadores LSO e LRO, o carregamento de trafo seja
 * calculado desconsiderando-se a árvore é fictícia.
 * 4) salvar o identificador do transformador com maior carregamento
 *
 * OBSERVAÇÃO: a função "carregamentoTrafoHeuristica()" crianda pela Telma e presente neste código possui um erro de engenharia,
 * uma vez que no cálculo da potência demandada de cada trafo é desconsiderada a demandas dos alimentadores que estão fora da região
 * do problema
  */
void carregamentoTrafoHeuristicaModificada(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam,
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino) {
    int indiceI, indiceJ;
    int idTrafo, idAlimentador;
    double carregamento;
    double maiorCarregamentoTrafo, sobrecargaTrafo = 0, correnteMaximaSuportada, corrente;
    long int idTrafoMaiorCarregamento = 0; // por Leandro: armazenará o identificador do trafo com maior carregamento

    maiorCarregamentoTrafo = 0;
    //realiza o cálculo para todos os alimentadores
    if (todosAlimentadores) {
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = 0 + ij * 0;
        }
        for (indiceJ = 1; indiceJ <= numeroAlimentadoresParam; indiceJ++) {
            idTrafo = dadosAlimentadorParam[indiceJ].idTrafo;
            //a sequencia de rnps obedece a mesma sequencia de alimentadores com a diferença de uma posição
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo] += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceJ - 1].fitnessRNP.potenciaAlimentador;
        }

        sobrecargaTrafo = 0; //Em Amperes
        for (indiceI = 0; indiceI < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; indiceI++) {
        	idAlimentador = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[indiceI];
        	idTrafo = dadosAlimentadorParam[idAlimentador + 1].idTrafo;

        	//Por Leandro: NOTA IMPORTANTE: a potência demandada do trafo (configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo]) e a sua capacidade estão em valores TRIFÁSICOS
        	carregamento = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo]) / (dadosTrafoParam[idTrafo].capacidade * 10000); //(10000=1MVA/100) em porcentagem
			if (maiorCarregamentoTrafo < carregamento){
				maiorCarregamentoTrafo = carregamento;
				idTrafoMaiorCarregamento = dadosTrafoParam[idTrafo].idTrafo; // por Leandro: armazena o identificador do trafo com maior carregamento

				//Por Leandro: Calcula a quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo "idTrafo"
				if(carregamento > 100){
					//sobrecargaTrafo = (cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo]) - (dadosTrafoParam[idTrafo].capacidade * 1000000)) / 3; // Em VA MONOFÁSICO
					correnteMaximaSuportada = (dadosTrafoParam[idTrafoMaiorCarregamento].capacidade * 1000000) / (sqrt(3) * dadosTrafoParam[idTrafoMaiorCarregamento].tensaoReal * 1000); //Por Leandro: Corrente de Linha máxima suportada pelo trafo
					corrente = (cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafoMaiorCarregamento])) / (sqrt(3) * dadosTrafoParam[idTrafoMaiorCarregamento].tensaoReal * 1000); //Por Leandro: Corrente de Linha que Flui pelo trafo na configuração avaliada
					sobrecargaTrafo = corrente - correnteMaximaSuportada; //Por Leandro: quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo "idTrafo"
				}

			}
        }

        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = maiorCarregamentoTrafo;
        configuracoesParam[idNovaConfiguracaoParam].objetivo.idTrafoMaiorCarregamento = idTrafoMaiorCarregamento; // por Leandro: armazena o identificador do trafo com maior carregamento
        configuracoesParam[idNovaConfiguracaoParam].objetivo.sobrecargaTrafo = sobrecargaTrafo; //Por Leandro: quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo com o maior carregamento dentre os trafos da região do problema


    } else //atualiza os valores considerando apenas os alimentadores que tiveram modificação
    {
        //Copia os valores de potência de trafo do indivíduo antigo para o indivíduo novo, fazendo a correção nos trafos em que estão conectadas as RNPs origem e destino da poda/corte
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {

            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = configuracoesParam[idAntigaConfiguracaoParam].objetivo.potenciaTrafo[indiceI];
            //verifica se o alimentador da RNP do alimentador de origem pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
            if (idRNPOrigem >= 0 && dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPOrigem + 1].idTrafo) { // Por Leandro: foi modificado aqui
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
            }
            //verifica se o alimentador da RNP do alimentador de destino pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
            if (idRNPDestino >= 0 &&  dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPDestino + 1].idTrafo) { // Por Leandro: foi modificado aqui
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
            }
        }

        sobrecargaTrafo = 0; //Em Amperes
        for (indiceI = 0; indiceI < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; indiceI++) {
        	idAlimentador = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[indiceI];
        	idTrafo = dadosAlimentadorParam[idAlimentador + 1].idTrafo;
        	//calcula a potencia que está sendo utilizada em porcentagem da capacidade
        	//Por Leandro: NOTA IMPORTANTE: a potência demandada do trafo (configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo]) e a sua capacidade estão em valores TRIFÁSICOS
        	carregamento = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo]) / (dadosTrafoParam[idTrafo].capacidade * 10000); //(10000=1MVA/100) em porcentagem
        	//obtem o maior carregamento de trafo da região do problema em porcentagem
        	if (maiorCarregamentoTrafo < carregamento){
				maiorCarregamentoTrafo = carregamento;
				idTrafoMaiorCarregamento = dadosTrafoParam[idTrafo].idTrafo; // por Leandro: armazena o identificador do trafo com maior carregamento

				//Por Leandro: Calcula a quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo "idTrafo"
				if(maiorCarregamentoTrafo > 100){
					//sobrecargaTrafo = (cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo]) - (dadosTrafoParam[idTrafo].capacidade * 1000000)) / 3; // Em VA MONOFÁSICO
					correnteMaximaSuportada = (dadosTrafoParam[idTrafoMaiorCarregamento].capacidade * 1000000) / (sqrt(3) * dadosTrafoParam[idTrafoMaiorCarregamento].tensaoReal * 1000); //Por Leandro: Corrente de Linha máxima suportada
					corrente = (cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafoMaiorCarregamento])) / (sqrt(3) * dadosTrafoParam[idTrafoMaiorCarregamento].tensaoReal * 1000); //Por Leandro: Corrente de Linha que Flui pela trafo na configuração avaliada
					sobrecargaTrafo = corrente - correnteMaximaSuportada; //Por Leandro: quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo "idTrafo"
				}

			}
        }
        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = maiorCarregamentoTrafo;
        configuracoesParam[idNovaConfiguracaoParam].objetivo.idTrafoMaiorCarregamento = idTrafoMaiorCarregamento; // por Leandro: armazena o identificador do trafo com maior carregamento
        configuracoesParam[idNovaConfiguracaoParam].objetivo.sobrecargaTrafo = sobrecargaTrafo; //Por Leandro: quantidade de corrente elétrica (em A) que excede o limite superior de capacidade de condução de corrente do trafo com o maior carregamento dentre os trafos da região do problema
    }
}


/**
 * Por Leandro:
 * Consiste na função "reconfiguradorHeuristica()" com inumeras modificações para, em especial:
 * a) chamar funções necessárias para considerar e tratar casos em que não é possível obter soluções que restabeleçam todos os consumidores desligados;
 * b) classificar a população em fronteiras e determinar a fronteira de Pareto (no caso do AEMO ser necessário);
 * c) selecionar um conjunto de soluções finais em vez de apenas uma única solução;
 * d) exlcuir a impressão de alguns arquivos de saída existente na versão original desta funçaõ (versão "reconfiguradorHeuristica()");
 * e) realizar a impressão de novos arquivos de saída, etc.
 *
 * Esta função realizada a leitura da configuração pré-falta do sistema e dos setores em falta; aloca memória para as principais variáveis do código; determina o indivíduo que é, conceitualmente,
 * o inicial do problema (aquele no qual o(s) setor(es) em falta está(ão) isolado(s) e os setores saudáveis afetados permanecem desligados; aplica a Busca Heurística (ou Busca Hexaustiva- BE)
 * para tentar obter indivíduos que reconectem todos os consumidores possíveis de serem reconectados por chaves;  verifica se algum destes indivíduos é atendem as restrições gerais e pode
 * ser considerado uma solução para o problema; chama a rotina que executa o processo evolutivo do AEMO (na verdade não apenas o processo evolutivo como também a geração da população inicial
 * do AEMO) caso a BE não tenha obtido nenhuma solução factível geral; classifica a população em fronteiras segundo o nível de dominância de cada indivíduo; determina a fronteira de Pareto formada
 * pelas soluções não dominadas; seleciona alguns indivíduos factíveis geral para serem as Soluções Finais do problema; e, por fim, imprime todos os arquivos de Saída.
 *
 * Os arquivos de Saída, as informações impressas por eles e a ordem em que as mesmas são impressas são dados a seguir:
 *
 *
 *
 * @param grafoSDRParam
 * @param numeroBarras
 * @param dadosTrafoSDRParam
 * @param numeroTrafos
 * @param dadosAlimentadorSDRParam
 * @param numeroAlimentadores
 * @param dadosReguladorSDR
 * @param numeroReguladores
 * @param rnpSetoresParam
 * @param numeroSetores
 * @param grafoSetoresParam
 * @param listaChavesParam
 * @param numeroChaves
 * @param configuracaoInicial
 * @param maximoCorrente
 * @param Z
 * @param estadoInicial
 * @param seed
 */
void reconfiguradorHeuristicaModificada(GRAFO *grafoSDRParam, long int numeroBarras, DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos,
        DADOSALIMENTADOR *dadosAlimentadorSDRParam, long int numeroAlimentadores, DADOSREGULADOR *dadosReguladorSDR,
        long int numeroReguladores, RNPSETORES *rnpSetoresParam, long int numeroSetores, GRAFOSETORES *grafoSetoresParam,
        LISTACHAVES *listaChavesParam, long int numeroChaves, CONFIGURACAO *configuracaoInicial, MATRIZMAXCORRENTE *maximoCorrente,
        MATRIZCOMPLEXA *Z, ESTADOCHAVE *estadoInicial, int seed) {

    long int *setorFalta, idConfiguracao = 0, idConfiguracaoInicial, idConfiguracaoBase, numeroConfiguracoesGeradas = 0;
    int numeroSetorFalta, *indiceReguladores;
    long int contador, numeroChaveAberta = 0, numeroChaveFechada = 0, maximoGeracoes, maximoGeracoesSemAtualizacaoPopulacao;
    int numeroDesejadoSolucoesFinais, numeroSolucoesFinaisObtidas = 0;
    long int melhorConfiguracao = -1, *idIndividuosFinais, *idSolucoesFinais;
    double tempo, taxaOperadorPAO = 0.5;
    int numeroTotalManobrasRestauracaoCompletaSemAlivio, indice, indiceIndividuo;
    long int numeroRNPFicticia = 0, idPrimeiraConfiguracao = -1,  numeroPosicoesAlocadas, geracoesExecutadas;
    int numeroDeFronteiras = 0, contadorRnp, contadorNos, numeroIndividuosPopulacaoInicial;
    VETORPI *vetorPi;
    MATRIZPI *matrizPI;
    CONFIGURACAO *configuracoes;
    VETORTABELA *populacao;
    SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivio;
    FRONTEIRAS *fronteiras, fronteiraPareto;
    clock_t inicio, fim, inicioGeral;
    BOOL copiaDados = false, todosAlimentadores = true, flagAtualizacaoPopulacao;
    int SBase;
    int tamanhoTabela[26];
    int numeroTabelas = 26;

    idConfiguracao = 0;
    idConfiguracaoInicial = idConfiguracao;
    idConfiguracaoBase = idConfiguracao;
    numeroExecucoesFluxoCarga = 0; /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/

    leituraParametrosModificada(&maximoGeracoes, tamanhoTabela, &SBase, &maximoGeracoesSemAtualizacaoPopulacao, &sequenciaManobrasAlivio, &numeroDesejadoSolucoesFinais);
    leituraDadosEntrada(&numeroSetorFalta, &setorFalta);

    /* O Tempo total estimado necessário para a recupeção de todas as falhas (dado por "tempoDeRestauracaoDaFalha") é proporcional ao número de faltas
     * simultâneas (dado por "numeroSetorFalta") e ao tempo necessário estimado para a recuperação de uma única falha (dada pela constante "TEMPO_DE_RESTAURACAO_DA_FALHA")*/
    tempoDeRestauracaoDaFalha = TEMPO_DE_RESTAURACAO_DA_FALHA * numeroSetorFalta;

    srand(seed);

    indiceReguladores = Malloc(int, (numeroBarras + 5));
    numeroPosicoesAlocadas = (maximoGeracoes + 3);
    //Nota: se o operador de corte (LSO) for utilizado, então o número de RNPs Fictícia não pode ser fixo, como é neste caso.
    numeroRNPFicticia = obtemNumeroSubarvoresDesligadas(grafoSetoresParam, numeroSetorFalta, setorFalta, numeroPosicoesAlocadas,  configuracaoInicial, idConfiguracaoInicial, rnpSetoresParam, grafoSDRParam); //Por Leandro: número de RNP Fictícia da configuração inicial do problema (aquela na qual todos os setores desligados encontram-se ainda desconectados)

    /*Testa se há setores saudáveis para restauração ou possíveis de serem restaurados. Isto porque não há restauração de energia
     * (a) quando não há setores à jusante do setor em falta (o setor em falta é o último num alimentador - nó folha), ou;
     * (b) quando nenhum dos setores saudáveis desligados pode ser restaurado devido a ausência de chaves NAs que o permita. */
    if(numeroRNPFicticia > 0){

		inicializaVetorPi(numeroPosicoesAlocadas, &vetorPi);
		inicializaMatrizPI(grafoSetoresParam, &matrizPI, numeroPosicoesAlocadas, numeroSetores+numeroRNPFicticia);  //Leandro: deve-se adicionar à matriz PI os setores artificiais que consistem nos nós-raiz das RNPs Fictícias.
		configuracoes = alocaIndividuoModificada(numeroAlimentadores, numeroRNPFicticia, idConfiguracao, numeroPosicoesAlocadas, numeroTrafos, numeroSetores);//Por Leandro: mudou a função que faz a alocação
		// Copia o Individuo Inicial para a posição 0 de "configuracoes"
		for (contadorRnp = 0; contadorRnp < configuracaoInicial[0].numeroRNP; contadorRnp++) {
			alocaRNP(configuracaoInicial[0].rnp[contadorRnp].numeroNos, &configuracoes[idConfiguracao].rnp[contadorRnp]);
			for (contadorNos = 0; contadorNos < configuracaoInicial[0].rnp[contadorRnp].numeroNos; contadorNos++) {
				configuracoes[idConfiguracao].rnp[contadorRnp].nos[contadorNos] = configuracaoInicial[0].rnp[contadorRnp].nos[contadorNos];
				adicionaColuna(matrizPI, configuracoes[idConfiguracao].rnp[contadorRnp].nos[contadorNos].idNo, idConfiguracao, contadorRnp, contadorNos);
			}
		}
		inicializaRNPsFicticias(configuracoes, idConfiguracao, numeroRNPFicticia, matrizPI, numeroSetores); //Por Leandro: Inicializa as RNPs Fictícias para a configuração "idConfiguracao"

		leituraVetorTaps(indiceReguladores, dadosReguladorSDR, numeroReguladores, numeroBarras);
		inicializaDadosEletricosPorAlimentador(grafoSDRParam, configuracoes, idConfiguracao, numeroBarras, SBase, dadosTrafoSDRParam, dadosAlimentadorSDRParam, rnpSetoresParam);

		avaliaConfiguracaoModificada(todosAlimentadores, configuracoes, -1, -1, idConfiguracao,
				dadosTrafoSDRParam, numeroTrafos, numeroAlimentadores, indiceReguladores,
				dadosReguladorSDR, dadosAlimentadorSDRParam, idConfiguracao,
				rnpSetoresParam, Z, maximoCorrente, numeroBarras, copiaDados,
				grafoSDRParam, sequenciaManobrasAlivio, listaChavesParam, vetorPi, false);

		configuracoes[idConfiguracao].objetivo.tempo = (configuracoes[idConfiguracao].objetivo.manobrasManuais * tempoOperacaoChaveManual) + (configuracoes[idConfiguracao].objetivo.manobrasAutomaticas * tempoOperacaoChaveAutomatica); // Por Leandro: calcula o tempo estimado necessário para a realização de todas as manobras que levarão a obtenção da nova configuração
		configuracoes[idConfiguracao].objetivo.tempoBruto = configuracoes[idConfiguracao].objetivo.tempo;

		idConfiguracao++;

		inicioGeral = clock(); //Por Leandro

		/*Salva DIRETAMENTE em "configuracoes" e em "populacao" também a configuração que corresponde ao setor em falta isolado e os setores saudáveis desligados.
		 * Este é chamado de "primeiro indivíduo" porque é, concentualmente, a configuração inicial do problema, isto é, aquela na qual o setor em falta encontra-se isolado
		 * e os setores saudáveis à jusante encontram-se desligados.
		 * Na ausência de um operador de reprodução que permita o corte de cargas saudáveis não afetadas inicialmente pela falta, esta configuração incial será aquela que possuirá o maior valor
		 * de energia não suprida. Em virtude disso, ela foi tomada como referência para os critérios de inserção de indivíduos nas tabelas de minimizam ENS e priorizam consumidores.
		 * Em virtude disso, o identificador desta configuração será preservado ao longo da execução do programa e
		 * salvo em "idPrimeiraConfiguracao; */
		obtemConfiguracaoInicial(grafoSetoresParam, idConfiguracaoInicial, configuracoes, matrizPI, vetorPi, setorFalta[0],
				estadoInicial, &numeroChaveAberta, &numeroChaveFechada, &idConfiguracao, listaChavesParam, rnpSetoresParam, grafoSDRParam);
		avaliaConfiguracaoHeuristicaModificada(false, configuracoes, -1, matrizPI[setorFalta[0]].colunas[0].idRNP, idConfiguracao,
				dadosTrafoSDRParam, numeroTrafos, numeroAlimentadores,
				indiceReguladores, dadosReguladorSDR, dadosAlimentadorSDRParam, idConfiguracaoInicial, rnpSetoresParam,
				Z, maximoCorrente, numeroBarras, false, grafoSDRParam, sequenciaManobrasAlivio,
				listaChavesParam, vetorPi, false);
		idPrimeiraConfiguracao = idConfiguracao;
		idConfiguracao++;

		inicio = clock();
		if (numeroSetorFalta > 1) {
			isolaRestabeleceMultiplasFaltasHeuristicaModificada(grafoSetoresParam, idConfiguracaoInicial, configuracoes, matrizPI, vetorPi, numeroSetorFalta, setorFalta,
					estadoInicial, &numeroChaveAberta, &numeroChaveFechada, &idConfiguracao, &idConfiguracaoInicial, listaChavesParam, rnpSetoresParam, grafoSDRParam); //Por Leandro: modificou a função que é chamada
		} else
			isolaRestabeleceTodasOpcoesHeuristicaModificada(grafoSetoresParam, idConfiguracaoInicial, configuracoes, matrizPI, vetorPi, setorFalta[0],
				estadoInicial, &numeroChaveAberta, &numeroChaveFechada, &idConfiguracao, listaChavesParam, rnpSetoresParam, grafoSDRParam); //Por Leandro: modificou a função que é chamada

		/* "numeroTotalManobrasRestauracaoCompletaSemAlivio" é o número total de manobras necessárias para restaurar todos os agrupamentos de setores saudáveis
		 * possíveis de serem reconectados SEM a execução de manobras para a alívio dos alimentadores adjacentes à região da falta. Neste número estão inclusas as manobras para isolar à jusante
		 * o(s) setor(es) em falta, as manobras para separar os agrupamentos saudáveis fora de serviço do(s) setor(es) em falta e as manobras para reconectá-los.
		 * Um vez que a Heurística chamada vulgarmente de Busca Exausativa é execeutada para a obtenção dos primeiros indivíduos deste AEMO, os quais possuem somente as manobras descritas acimaa,
		 * então, "numeroTotalManobrasRestauracaoCompletaSemAlivio" pode ser representado pelo número total de manobras de algum dos indivíduos obtidos pela Busca Exaustiva */
		numeroTotalManobrasRestauracaoCompletaSemAlivio =  configuracoes[idPrimeiraConfiguracao + 1].objetivo.manobrasManuais + configuracoes[idPrimeiraConfiguracao + 1].objetivo.manobrasAutomaticas; //Por Leandro: modificado aqui devido a mudanças nos identificadores das configurações obtidas pelas busca exaustiva

		alocaTabelas(numeroTabelas, tamanhoTabela, &populacao);

		for (contador = idPrimeiraConfiguracao + 1; contador < idConfiguracao; contador++) { //Por Leandro: Modificado aqui para que o for comece com "idPrimeiraConfiguracao" em vez de começar com "idConfiguracaoInicial"
			/*Embora todos os setores possíveis de serem reconectados já estão reconectados nas configurações geradas pela Busca Heurística (Busca Exaustiva),
			 * optou-se por copiar para elas "RNPs Fictícias" com a finalidade de simplificar a implementação computacional. Desta forma, todos os indivíduos
			 * salvos em "configuracoes" terão rnps Fictícia com ao menos nós ficticios*/
			configuracoes[contador].numeroRNPFicticia = configuracoes[idConfiguracaoInicial].numeroRNPFicticia;
			for (indice = 0; indice < configuracoes[idConfiguracaoInicial].numeroRNPFicticia; indice++ )
				configuracoes[contador].rnpFicticia[indice] = configuracoes[idConfiguracaoInicial].rnpFicticia[indice];

			avaliaConfiguracaoHeuristicaModificada(todosAlimentadores, configuracoes, -1, -1, contador, dadosTrafoSDRParam,
					numeroTrafos, numeroAlimentadores, indiceReguladores, dadosReguladorSDR, dadosAlimentadorSDRParam,
					idConfiguracaoBase, rnpSetoresParam, Z, maximoCorrente, numeroBarras, copiaDados, grafoSDRParam, sequenciaManobrasAlivio,
					listaChavesParam, vetorPi, false);

			calculaEnergiaAtivaNaoSupridaPorNivelPrioridadeIndividuosIsolaRestabeleceTodasOpcoesHeuristica(configuracoes, vetorPi, matrizPI,
					contador, listaChavesParam, rnpSetoresParam, grafoSDRParam);

			insereTabelasModificadaV2(populacao, configuracoes, contador, idPrimeiraConfiguracao, numeroTotalManobrasRestauracaoCompletaSemAlivio, sequenciaManobrasAlivio, false, vetorPi, &flagAtualizacaoPopulacao); //Por Leandro: modificado para salvar em "populacaoTemp" em vez de "populacao"
		}

		//Por Leandro: Imprime os dados das configurações obtidas pela Busca Exaustiva
		fim = clock();
		tempo = (double) (fim - inicio) / CLOCK_TICKS_PER_SECOND;
		imprimeIndividuosBuscaExaustiva(setorFalta, numeroSetorFalta, configuracoes, idPrimeiraConfiguracao, idConfiguracao, tempo, vetorPi);

		/*Determina a melhor dentre as soluções obtidas pela Busca Heurśitica (Busca Exaustiva)*/
	//    melhorConfiguracao = melhorSolucao(configuracoes, populacao, numeroTabelas);
		melhorConfiguracao = melhorDasSolucaoQueRestabelecemTodosSetoresPelaHeuristica(configuracoes, idPrimeiraConfiguracao + 1, idConfiguracao);

		numeroIndividuosPopulacaoInicial = idConfiguracao;

		numeroConfiguracoesGeradas = idConfiguracao - 1;
		if ((melhorConfiguracao < 0) || (!verificaFactibilidade (configuracoes[melhorConfiguracao],maxCarregamentoRede,maxCarregamentoTrafo, maxQuedaTensao ))) {

			/* Por Leandro: O indivíduo que é, conceitualmente, a configuração inicial do problema, cujo id é "idPrimeiraConfiguracao", é inserido nas
			 * tabelas somente após a verificação da existência de soluções factíveis dentre aquelas obtidas pela BE.
			 * Se isto não fosse feito, o indivíduo "idPrimeiraConfiguracao" seria escolhido como melhor solução, pois é factível e possui o menor número de manobras dentre todos os indivíduos.
			 * Contudo, o indivíduo "idPrimeiraConfiguracao" não pode ser uma solução para o problema, pois o mesmo não restabelece nenhum consumidor desligado.
			 * Ademais, ele é enserido também somente se o AEMO precisar ser executado para a obtenção de soluções, pois, embora não restabeleça nenhuma consumidor, pode ser usado para a obtenção
			 * de indivíduos (soluções) mais adequadas por meio do Operador LRO. Além disso, se fosse inserido nas tabelas na situação em que a Busca Heusrística (Exaustiva) obteve soluções,
			 * tal indivíduo iria impedir a determinação de fronteira de Pareto com soluções que reconectam os consumidores desligados*/
			insereTabelasModificadaV2(populacao, configuracoes, idPrimeiraConfiguracao, idPrimeiraConfiguracao, numeroTotalManobrasRestauracaoCompletaSemAlivio, sequenciaManobrasAlivio, false, vetorPi, &flagAtualizacaoPopulacao);

			/* Por Leandro: NOTA INFORMATIVA
			 * Uma vez que a BE não retornou uma solução que atenda as restriçẽos GERAIS do problema, será então iniciado o processo Evolutivo.
			 * Neste AEMO, diferentemente de outros anteriores, são salvos nas tabelas somente indivíduos que atendam as restrições RELAXADAS do problema.*/
			inicio = clock();
			processoEvolutivoHeuristicaModificada(dadosTrafoSDRParam, dadosAlimentadorSDRParam, &configuracoes, populacao, grafoSetoresParam,
					rnpSetoresParam, listaChavesParam, matrizPI, &vetorPi, maximoCorrente, Z,
					numeroBarras, numeroTrafos, estadoInicial, maximoGeracoes, numeroTabelas,
					taxaOperadorPAO, idConfiguracao, indiceReguladores, dadosReguladorSDR,
					&numeroChaveAberta, &numeroChaveFechada, maximoGeracoesSemAtualizacaoPopulacao, numeroTotalManobrasRestauracaoCompletaSemAlivio,
					grafoSDRParam, idPrimeiraConfiguracao, sequenciaManobrasAlivio, &numeroPosicoesAlocadas, numeroSetores, numeroSetorFalta, &numeroConfiguracoesGeradas, &geracoesExecutadas);

			//DETERMINAÇÃO DA FRONTEIRA DE PARETO E SELEÇÃO DE SOLUÇÕES FINAIS (tal ação é executada por meio das quadro funções a seguir)
			/*Classifica a população em fronteiras de Pareto segundo o grau de dominância de cada indivíduo*/
			classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobrasV3(&fronteiras, &numeroDeFronteiras, configuracoes, populacao, numeroTabelas, vetorPi, idPrimeiraConfiguracao);

			numeroSolucoesFinaisObtidas = 0;
			fronteiraPareto.numeroIndividuos = 0;
			if(numeroDeFronteiras > 0){
				/* Na posição 0 (zero) de "fronteiras" já está salva a Fronteira de Pareto com as soluções não dominada. Contudo,
				 como o processo evolutivo pode gerar várias indivíduos iguais com IDs diferentes, em todas as fronteiras salvas em "fronteiras"
				 pode ocorrer de haverem várias indivíduos iguais salvos. Para, deixar somente indivíduos com características distintas
				 na primeira fronteira, a fronteira de Pareto, executa-se a função abaixo, que compara todos os indivíduos da posição 0
				 (zero) de "fronteiras" e salva os distintos em "fronteiraPareto"*/
				otimizaFronteiraParetoV2(fronteiras, 0, &fronteiraPareto, configuracoes);
				/*Ordena os indivíduos da fronteira de Pareto em Ordem Cresecento dos valores do Objetivo 1 (Energia TOTAl não suprida)*/
				ordenaCrescenteIndividuosFronteiraParetoObjetivo1(&fronteiraPareto);
				/* Após a execução das três funções acima, na variável "fronteiraPareto" estarão salvas as soluções que formam a fronteira de Pareto obtida pelo AEMO.
				 * Contudo, a fim de escolher somente algumas dentre estas soluções, executa-se a função a seguir. Ela
				 * determina as soluções finais do problema, cuja quantidade desejada é dada por "numeroDesejadoSolucoesFinais", e salva em "idIndividuosFinais" o índice (posição) de tais indivíduos
				 * em "fronteiraPareto".*/
		//        idIndividuosFinais = selecionaSolucoesFinaisOpcao1(&numeroDesejadoSolucoesFinais, fronteiraPareto);
				idIndividuosFinais = selecionaSolucoesFinaisOpcao2(&numeroDesejadoSolucoesFinais, fronteiraPareto);
				/*Estas quadro funções a seguir podem ser usadas para a escolha de uma única solução final. Cada uma utiliza um critério diferente*/
		//        melhorConfiguracao = determinaSolucaoFinalMetodo1(fronteiraPareto);
		//        melhorConfiguracao = determinaSolucaoFinalMetodo2(fronteiraPareto, configuracoes);
		//        melhorConfiguracao = determinaSolucaoFinalMetodo3(fronteiraPareto, configuracoes);
		//        melhorConfiguracao = fronteiraPareto.individuos[idIndividuosFinais[0]].idConfiguracao;

				/*Busca em "fronteiraPareto" o índice das soluções finais na variável "configurações" e salvo-os a fim de informá-los à função "imprimeArquivosdeSaidaV4()"*/
				idSolucoesFinais = malloc(numeroDesejadoSolucoesFinais * sizeof(long int));
				for(indice = 0; indice < numeroDesejadoSolucoesFinais; indice++){
					indiceIndividuo = idIndividuosFinais[indice];
					idSolucoesFinais[indice] = fronteiraPareto.individuos[indiceIndividuo].idConfiguracao;
					numeroSolucoesFinaisObtidas++;
				}
			}

			/*Encerra a contagem de tempo */
			fim = clock();
			tempo = (double)(fim-inicioGeral)/CLOCK_TICKS_PER_SECOND;

			/* IMPRESSÃO DOS ARQUIVOS DE SAÍDA
			 * 1) Imprime diversas informações dos indivíduos salvos nas tabelas, da configuração pré-falta, dos indivíduos selecionados como soluções finais, etc*/
			imprimeArquivosdeSaidaV5(seed, setorFalta, numeroSetorFalta, configuracoes, populacao, idSolucoesFinais, numeroDesejadoSolucoesFinais, idPrimeiraConfiguracao, numeroIndividuosPopulacaoInicial, tempo, numeroTabelas, rnpSetoresParam, numeroConfiguracoesGeradas, vetorPi, sequenciaManobrasAlivio, geracoesExecutadas, numeroSolucoesFinaisObtidas);
			/* 2) Imprime a fronteira de Pareto obtida pelo AEMO*/
			imprimeFronteiraPareto(seed, setorFalta, numeroSetorFalta, fronteiraPareto, configuracoes);
			/* 3) Imprime a sequência de chaveamento das soluções selecionadas como "Soluções Finais"*/
			imprimeSequenciaChaveamentoSolucoesFinais(seed, setorFalta, numeroSetorFalta, fronteiraPareto, idIndividuosFinais, numeroSolucoesFinaisObtidas, vetorPi, configuracoes, sequenciaManobrasAlivio);
	//        mostraSequenciaChaveamentoIndividuo(melhorConfiguracao, vetorPi, configuracoes, sequenciaManobrasAlivio);

			if(numeroDeFronteiras > 0){
				free(idIndividuosFinais);
				free(idSolucoesFinais);
			}
			free(configuracoes);
			desalocaMatrizPI(matrizPI, numeroSetores);
			free(matrizPI);
			desalocaTabelas(numeroTabelas, populacao);
			free(populacao);
			desalocaVetorPi((maximoGeracoes), vetorPi);
			free(vetorPi);
			free(indiceReguladores);

			// Para testar quais setores de um SD exigem a execução do AEMO para obtenção de solução. Em outras palavras, quais casos de falta não podem ser solucionados pela BE
	//        FILE *arquivoTeste;
	//        char nomeArquivoTeste[120];
	//        sprintf(nomeArquivoTeste, "SAIDA_Setores_Que_Exigiram_Processo_Evolutivo.dad");
	//        arquivoTeste = fopen(nomeArquivoTeste, "a");
	//        fprintf(arquivoTeste, "\n%ld\t%d", setorFalta[0], seed);
	//        fclose(arquivoTeste);

		} else {

			/* Quando uma das configurações obtidas pela busca heurística (também chamada vulgarmente de Busca Exaustiva) atende as restrições GERAIS do problema,
			 * a mesma pode ser considerada uma solução para o problema, dado que é factível e que reconecta todos os consumidores desligados possívies de serem reconectados.
			 * Uma vez que todas as configurações obtidas pela Busca Heurística possuem o mesmo número total de manobras, a aplicação da
			 * função "classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras" resultaria de seleção do indivíduo com o menor valor de Energia Não Suprida Total (ENS_TOT).
			 * Contudo, do modo com o código está implementado, os valores de ENS_TOT de tais indivíduos é nulo, pois a função utilizada para calcula-los analisa as RNPs Fictícis e tais
			 * indivíduos não possuem RNP Fictícia porque reconectam todos os consumidores.
			 * Logo, para proceder a escolha de uma dentre estas soluções por meio da análise do ENS faz-se necessário implementar uma rotina que faça este cálculo para tais soluções.
			 * Em virtude de tempo, deixarei isto em aberto (sem fazer no momento) e procederei informando apenas uma única solução final: aquela obtida pela função "melhorDasSolucaoQueRestabelecemTodosSetoresPelaHeuristica()".
			 * Quando tiver sido implementada uma rotina para cálculo do ENS dos indivíduos obtidos pela Busca Heurística, as duas funções a seguir poderão ser utilizadas para a definição e
			 * ordenação de uma fronteira de Pareto. Uma vez que a fronteira "fronteiraPareto" seja obtida por tais duas funções, os 6 comandos posteriores a função "otimizaFronteiraParetoV2" deverão ser excluídos.*/

			/*DETERMINAÇÃO DA FRONTEIRA DE PARETO E SELEÇÃO DE SOLUÇÕES FINAIS (tal ação é executada por meio das quadro funções a seguir)*/
			/*Classifica a população em fronteiras de Pareto segundo o grau de dominância de cada indivíduo*/
	//        classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras(&fronteiras, &numeroDeFronteiras, configuracoes, populacao, numeroTabelas);
			/* Na posição 0 (zero) de "fronteiras" já está salva a Fronteira de Pareto com as soluções não dominada. Contudo,
			 como o processo evolutivo pode gerar várias indivíduos iguais com IDs diferentes, em todas as fronteiras salvas em "fronteiras"
			 pode ocorrer de haverem várias indivíduos iguais salvos. Para, deixar somente indivíduos com características distintas
			 na primeira fronteira, a fronteira de Pareto, executa-se a função abaixo, que compara todos os indivíduos da posição 0
			 (zero) de "fronteiras" e salva os distintos em "fronteiraPareto"*/
	//        otimizaFronteiraParetoV2(fronteiras, 0, &fronteiraPareto, configuracoes);

			/* NOTA: Exclui estas 6 linhas a seguir quando o ENS_TOT dos indivíduos obtidos pela BE tiverem seus valores determinados e as funções "classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras() e
			 * "otimizaFronteiraParetoV2()" tiverem sido descomentadas*/
			fronteiraPareto.individuos = malloc(1*sizeof(INDIVIDUOSNAODOMINADOS));
			fronteiraPareto.individuos[0].idConfiguracao = melhorConfiguracao;
			fronteiraPareto.individuos[0].valorObjetivo1 = configuracoes[melhorConfiguracao].objetivo.energiaTotalNaoSuprida;
			fronteiraPareto.individuos[0].valorObjetivo2 = configuracoes[melhorConfiguracao].objetivo.manobrasAutomaticas + configuracoes[melhorConfiguracao].objetivo.manobrasManuais;
			fronteiraPareto.nivelDominancia = 0;
			fronteiraPareto.numeroIndividuos = 1;

			/*Ordena os indivíduos da fronteira de Pareto em Ordem Cresecento dos valores do Objetivo 1 (Energia TOTAl não suprida)*/
			ordenaCrescenteIndividuosFronteiraParetoObjetivo1(&fronteiraPareto);
			/* Após a execução das três funções acima, na variável "fronteiraPareto" estarão salvas as soluções que forma a fronteira de Pareto obtida pelo AEMO.
			 * Contudo, a fim de escolher somente algumas dentre estas soluções, executa-se a função a seguir. Ela
			 * determina as soluções finais do problema, cuja quantidade desejada é dada por "numeroDesejadoSolucoesFinais", e salva em "idIndividuosFinais" o índice (posição) de tais indivíduos
			 * em "fronteiraPareto".*/
			idIndividuosFinais = selecionaSolucoesFinaisOpcao1(&numeroDesejadoSolucoesFinais, fronteiraPareto);

			/*Busca em "fronteiraPareto" o índice das soluções finais na variável "configurações" e salvo-os a fim de informá-los à função "imprimeArquivosdeSaidaV4()"*/
			numeroSolucoesFinaisObtidas= 0;
			idSolucoesFinais = malloc(numeroDesejadoSolucoesFinais * sizeof(long int));
			for(indice = 0; indice < numeroDesejadoSolucoesFinais; indice++){
				indiceIndividuo = idIndividuosFinais[indice];
				idSolucoesFinais[indice] = fronteiraPareto.individuos[indiceIndividuo].idConfiguracao;
				numeroSolucoesFinaisObtidas++;
			}

			/*Encerra a contagem de tempo */
			fim = clock();
			tempo = (double)(fim-inicioGeral)/CLOCK_TICKS_PER_SECOND;

			/* IMPRESSÃO DOS ARQUIVOS DE SAÍDA
			 * 1) Imprime diversas informações dos indivíduos salvos nas tabelas, da configuração pré-falta, dos indivíduos selecionados como soluções finais, etc*/
			imprimeArquivosdeSaidaV5(seed, setorFalta, numeroSetorFalta, configuracoes, populacao, idSolucoesFinais, numeroDesejadoSolucoesFinais, idPrimeiraConfiguracao, numeroIndividuosPopulacaoInicial, tempo, numeroTabelas, rnpSetoresParam, numeroConfiguracoesGeradas, vetorPi, sequenciaManobrasAlivio, geracoesExecutadas, numeroSolucoesFinaisObtidas);
			/* 2) Imprime a fronteira de Pareto obtida pelo AEMO*/
	//        imprimeFronteiraPareto(seed, setorFalta, numeroSetorFalta, fronteiraPareto, configuracoes);
			/* 3) Imprime a sequência de chaveamento das soluções selecionadas como "Soluções Finais"*/
			imprimeSequenciaChaveamentoSolucoesFinais(seed, setorFalta, numeroSetorFalta, fronteiraPareto, idIndividuosFinais, numeroSolucoesFinaisObtidas, vetorPi, configuracoes, sequenciaManobrasAlivio);

			free(idIndividuosFinais);
			free(idSolucoesFinais);
			free(configuracoes);
			desalocaMatrizPI(matrizPI, numeroSetores);
			free(matrizPI);
			desalocaTabelas(numeroTabelas, populacao);
			free(populacao);
			desalocaVetorPi(idConfiguracao, vetorPi);
			free(vetorPi);
			free(indiceReguladores);

		}

	//    FILE *arquivoTeste;
	//    char nomeArquivoTeste[120];
	//    sprintf(nomeArquivoTeste, "SAIDA_testeConsistencia.dad");
	//    arquivoTeste = fopen(nomeArquivoTeste, "a");
	//    fprintf(arquivoTeste, "\n%ld\t%d", setorFalta[0], seed);
	//    fclose(arquivoTeste);

    }
    else{
        /*Testa se há setores saudáveis para restauração ou possíveis de serem restaurados. Isto porque não há restauração de energia
         * (a) quando não há setores à jusante do setor em falta (o setor em falta é o último num alimentador - nó folha), ou;
         * (b) quando nenhuma dos setores saudáveis desligados não pode ser restaurado devido a ausência de chaves NAs que o permita. */
    	printf("\n - Não há nenhum consumidor saudável para ser restaurado porque não há setores à jusante do setor em falta, OU");
    	printf("\n - Nenhum dos setores saudáveis desligados pode ser restaurado porque não há chaves que o permita.");
    	printf("\n PORTANTO, este caso de falta não configura-se um problema de restabelecimento ou não possui solução!"); printf("\n");
    }
}

/* Descrição:
 * Esta função adiciona um novo alimentador (árvore ou rnp) à lista que contém os alimentadores envolvidos no problema.
 * Esta lista será utilizada na avaliação da solução, de forma que os valores das restrições serão determinados somente para
 * estes alimentadores.
 *
 * Obs.: Heurística que restringe a avaliação das configurações á região do problema.
 * 
 * @param configuracaoParam
 * @param rnp
 */
void insereRNPLista(CONFIGURACAO *configuracaoParam, int rnp)
{
    int contador;
    
    //verifica se a rnp passada como parâmetro não pertence a lista
    contador = 0;
    while (contador < configuracaoParam[0].numeroRNPsFalta && configuracaoParam[0].idRnpFalta[contador] != rnp)
        contador++;
    //a rnp não pertence a lista
    if(contador == configuracaoParam[0].numeroRNPsFalta)
    {
        configuracaoParam[0].idRnpFalta[contador] = rnp;
        configuracaoParam[0].numeroRNPsFalta++;
    }
}

/**
 * 
 * @param configuracoesParam
 * @param idConfiguracaoNova
 * @param idConfiguracaoAntiga
 */
void copiaListaRnps(CONFIGURACAO *configuracoesParam, long int idConfiguracaoNova, long int idConfiguracaoAntiga)
{

    int contador;
    
    //verifica se a rnp passada como parâmetro não pertence a lista
    contador = 0;
    while (contador < configuracoesParam[idConfiguracaoAntiga].numeroRNPsFalta)
    {
        configuracoesParam[idConfiguracaoNova].idRnpFalta[contador] = configuracoesParam[idConfiguracaoAntiga].idRnpFalta[contador];
        contador++;
    }
    
    configuracoesParam[idConfiguracaoNova].numeroRNPsFalta = configuracoesParam[idConfiguracaoAntiga].numeroRNPsFalta;
}


 
/**
* Para reestabelecimento a Configuração Inicial do SDR é aquela com o setor em falta isolado e as cargas a jusante reconectadas. Isto é para Restabelecimento, a Configuração
 * Inicial é diferente da Topologia Inicial. Nesse caso, o vetor 'estadoInicial', que armazena os estados da chave na Configuração Inicial, deve ser atualizado após a definição
 * das chaves que isolão a falta e reconectão as cargas a jusante. Porém, isso não acontecia. Para corrigir essa falha, além das duas linhas de código que foram
 * inserirdas ao final da função 'numeroManobras', foram inseridos também os caminhos abaixo que visam determinar o nome da chave que conecta o setor em falta a seu
 * alimentador, chave esta que deve ser aberta para isolar a falta. Esses comandos determinam o nome dessa chave e alteram o seu estado para 'normalmenteAberta' no vetor 'estadoInical'.
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param setorFaltaParam
 * @param estadoInicial
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param listaChavesParam
 */
void isolaRestabeleceTodasOpcoesHeuristica(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam) {
    int tamanhoTemporario;
    long int p, a, indice, indice1, indiceP, indiceR, indiceL, r,
            indiceSetorFalta, indiceA;
    long int noAnteriorFalta;
    long int indiceLFalta, idConfiguracaoBase;
    int rnpP, rnpA;
    int numeroFalta, numeroAJusante, posicao;
    long int nosFalta[500]; //é o número máximo de setores em falta ou a jusantes
    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
    long int totalCombinacoes;
    long int *idChaveAberta, *idChaveFechada;
    long int consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    long int consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    int indiceChave;
    int **indiceRestabelecimento;
    int blocoRestabelecimento=0;
    int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4;
    int colunaPI;
    RNP rnpOrigem;
    RNP rnpDestino;
    NOSRESTABELECIMENTO *nos;
    int casoManobra;
    BOOL *estadoInicialCA;
    BOOL *estadoInicialCF;
    NOSPRA *nosPRA;
    //IMPRESSAO DE DADOS PARA ANALISE DOS RESULTADOS    
    char nomeArquivo[120];
    FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
    //todos os setores do sistema
    sprintf(nomeArquivo, "SAIDA_testeConfiguracaoInicial.dad");
    arquivo1 = fopen(nomeArquivo, "a");

    //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
    grafoSetoresParam[setorFaltaParam].setorFalta = true;
    //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
  //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
   // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;

    //RECUPERA A COLUNAPI DO SETOR EM FALTA
    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
    rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
    indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
    indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

    //LOCALIZA O SETOR ANTERIOR A FALTA
    indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o éndice do nó antecessor ao nó sob falta
    while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
        indice1--;
    }
    noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

    //verifica se o setor em falta não é o último da RNP
    if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
        numeroAJusante = 0;
        //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
        if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
            indice1 = indiceP + 1;
            nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
            numeroAJusante++;
            //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
            while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
                if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                    nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    numeroAJusante++;
                }
                indice1++;
            }
            indiceLFalta = indice1 - 1;
            indice = 0;
            nosFalta[0] = setorFaltaParam;
            numeroFalta = 1;
            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
            totalCombinacoes = 1;
            blocoRestabelecimento = 0;
            //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as opções de restabelecimento
            while (indice < numeroAJusante) {
                a = 0;
                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
                //recupera o indice final da subárvore dos setore a jusante
                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
                nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
                buscaTodosNosRestabelecimento(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
                //printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
                if (nos[blocoRestabelecimento].numeroNos > 0) {
                    totalCombinacoes *= nos[blocoRestabelecimento].numeroNos;
                    blocoRestabelecimento++;

                } else {
                    free(nos[blocoRestabelecimento].noA);
                    free(nos[blocoRestabelecimento].noR);
                    free(nos[blocoRestabelecimento].noP);
                    free(nos[blocoRestabelecimento].rnpA);
                    //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
                    nosFalta[numeroFalta] = nosJusanteFalta[indice];
                    numeroFalta++;
                    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                    indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
                    // printf("%d\n",indiceAux1);
                    while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
                        nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
                        numeroFalta++;
                        indiceAux1++;
                    }
                }
                indice++;
            }
            //******************************************************************************************************//
            //salva no arquivo quantas combinacoes de restabelecimento existem e quantos setores são agrupados a falta
            fprintf(arquivo1, "Combinacoes;%ld;Setores;%d;", totalCombinacoes, numeroFalta);
            if (blocoRestabelecimento == 0) { //não existe opção de restabelecimento para os setores a jusante.
                fprintf(arquivo1, "SAR\n");
                fclose(arquivo1);
                //cria um novo individuo isolando a falta e os setores sem restabelecimento
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
                //isola a falta
                idChaveAberta = Malloc(long int, 1);
                idChaveFechada = Malloc(long int, 1);
                buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                        configuracoesParam, numChaveAberta, idChaveAberta,
                        idChaveFechada, setorFaltaParam, noAnteriorFalta,
                        1, listaChavesParam);
                nosPRA = Malloc(NOSPRA, 1);
                nosPRA[0].a = -1;
                nosPRA[0].p = setorFaltaParam;
                nosPRA[0].r = -1;
                estadoInicialCA = Malloc(BOOL, 1);
                estadoInicialCF = Malloc(BOOL, 1);
                estadoInicialCA[0] = false;
                estadoInicialCF[0] = false;
                //aloca a nova rnp
                alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta, &rnpOrigem);
                //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                 colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[0], idConfiguracaoParam);
                 indiceP = matrizPiParam[nosFalta[0]].colunas[colunaPI].posicao;
                 rnpP = matrizPiParam[nosFalta[0]].colunas[colunaPI].idRNP;
                 copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
                 //marca os setores como em falta e conta os consumidores
                 for(indice1 = 0; indice1 < numeroFalta; indice1++)
                 {
                    grafoSetoresParam[nosFalta[indice1]].setorFalta = true;
                    //conta os consumidores sem fornecimento
                    consumidoresSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidores;
                    consumidoresEspeciaisSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidoresEspeciais;
                 }
                //copia os setores anteriores a falta para a nova rnp origem
                for (indice1 = 0; indice1 < indiceP; indice1++) {
                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
                }
                //remove o setor em falta da RNP e copia os demais se existir
                if ((indiceP + numeroFalta) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                    for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta; indice1++) {
                        rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].idNo;
                        rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].profundidade;
                        adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                    }

                }

                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
                for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                    if (indice1 != rnpP)
                        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

                }
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
                //insere no vetor Pi a nova solução
                atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                        idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), 1, estadoInicialCA, estadoInicialCF, nosPRA, ESO);
                idNovaConfiguracaoParam[0]++;
            } else {
                fclose(arquivo1);
                //*************************************************************************************************//
                indiceRestabelecimento = (int **) malloc(totalCombinacoes * sizeof (int*));
                //cada coluna da matriz indiceRestabelecimento contém os indices do vetor de opções de restabelecimento para 
                //cada bloco de setores a jusante que poderá ser restabelecido.
                //Cada linha contém uma combinação das opções de restabelecimento para cada bloco
                for (indice = 0; indice < totalCombinacoes; indice++) {
                    indiceRestabelecimento[indice] = (int *) malloc(blocoRestabelecimento * sizeof (int));
                }

                //preenche as combinacoes de indices das opçoes de restabelecimento para cada raiz de bloco de setores a jusante
                indiceAux3 = 0;
                for (indice = 0; indice < blocoRestabelecimento; indice++) {
                    if (nos[indice].numeroNos > 0) {
                        repeticoes = totalCombinacoes / nos[indice].numeroNos;
                        indiceAux4 = 0;
                        for (indiceAux1 = 0; indiceAux1 < repeticoes; indiceAux1++) {
                            for (indiceAux2 = 0; indiceAux2 < nos[indice].numeroNos; indiceAux2++) {
                                indiceRestabelecimento[indiceAux4][indiceAux3] = indiceAux2;
                                indiceAux4++;
                            }
                        }
                        indiceAux3++;
                    }
                }

                //aplica as combinações de restabelecimento obtidas. 
                for (indiceAux1 = 0; indiceAux1 < totalCombinacoes; indiceAux1++) {
                    idConfiguracaoBase = idConfiguracaoParam;
                    indiceChave = 1;
                    idChaveAberta = Malloc(long int, (blocoRestabelecimento + 1));
                    idChaveFechada = Malloc(long int, (blocoRestabelecimento + 1));
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
                    buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                            configuracoesParam, numChaveAberta, idChaveAberta,
                            idChaveFechada, setorFaltaParam, noAnteriorFalta,
                            (blocoRestabelecimento + 1), listaChavesParam);
                    casoManobra = 1;
                    estadoInicialCA = Malloc(BOOL, (blocoRestabelecimento + 1));
                    estadoInicialCF = Malloc(BOOL, (blocoRestabelecimento + 1));
                    
                    nosPRA = Malloc(NOSPRA, (blocoRestabelecimento + 1));
                    estadoInicialCA[0] = false;
                    estadoInicialCF[0] = false;
                    nosPRA[0].a = -1;
                    nosPRA[0].p = setorFaltaParam;
                    nosPRA[0].r = -1;
                    consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
                    consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
                    copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
                    insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
                    for (indiceAux2 = 0; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
                        //printf("%d \t", indiceRestabelecimento[indiceAux1][indiceAux2]);
                        indice = indiceAux2;
                        posicao = indiceRestabelecimento[indiceAux1][indiceAux2];
                        rnpA = nos[indice].rnpA[posicao];
                        a = nos[indice].noA[posicao];
                        r = nos[indice].noR[posicao];
                        p = nos[indice].noP[posicao];
                        if (rnpA != -1) {
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idConfiguracaoBase);
                            indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
                            rnpA = matrizPiParam[a].colunas[colunaPI].idRNP;
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idConfiguracaoBase);
                            indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, p, idConfiguracaoBase);
                            indiceP = matrizPiParam[p].colunas[colunaPI].posicao;
                            indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);
                            p = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceP].idNo;
                            r = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceR].idNo;
                            a = configuracoesParam[idConfiguracaoBase].rnp[rnpA].nos[indiceA].idNo;
                            nosPRA[indiceAux2+1].a = a;
                            nosPRA[indiceAux2+1].p = p;
                            nosPRA[indiceAux2+1].r = r;
                            //localiza a chave a ser aberta
                            indice1 = 0;
                            while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != p) {
                                indice1++;
                            }
                            idChaveAberta[indiceAux2 + 1] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
                            //localiza a chave a ser fechada
                            indice1 = 0;
                            while (grafoSetoresParam[r].setoresAdjacentes[indice1] != a) {
                                indice1++;
                            }
                            idChaveFechada[indiceAux2 + 1] = grafoSetoresParam[r].idChavesAdjacentes[indice1];
                            //ROTINA PARA CALCULAR OS PARES DE MANOBRAS
                            numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[indiceAux2 + 1], idChaveFechada[indiceAux2 + 1], listaChavesParam,
                                    idNovaConfiguracaoParam[0], estadoInicial, &estadoInicialCA[indiceAux2+1], &estadoInicialCF[indiceAux2+1], idNovaConfiguracaoParam[0], &casoManobra);

                            tamanhoTemporario = indiceL - indiceP + 1;
                            //realiza a alocação das RNPs que serão alteradas 
                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                            //obtém a nova rnp de destino
                            constroiRNPDestino(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam[0]);
                            constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam[0]);
                            //libera a memória utilizada pela rnp do setor em falta

                            //###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpA] = rnpDestino;
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
                            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpA);
                            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNP; indice1++) {
                                if (indice1 != rnpA && indice1 != rnpP)
                                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoBase].rnp[indice1];
                            }

                        }
                        idConfiguracaoBase = idNovaConfiguracaoParam[0];
                        vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
                    }

                    //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
                    tamanhoTemporario = 1;

                    //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                    for (indice = 0; indice < numeroFalta; indice++) {
                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam[0]);
                        indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
                        rnpP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].idRNP;
                        grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
                        //conta os consumidores sem fornecimento
                        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
                        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
                        if ((indiceP + 1) <= configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos) {
                            for (indice1 = indiceP; indice1 < configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1; indice1++) {
                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].idNo;
                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].profundidade;
                                adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                            }
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1;
                        }
                    }
                    //atualiza consumidores sem fornecimento
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                    //insere no vetor Pi a nova solução
                    atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                            idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), casoManobra, 
                            estadoInicialCA, estadoInicialCF, nosPRA, ESO);
                    idNovaConfiguracaoParam[0]++;

                }
                for (indice = 0; indice < blocoRestabelecimento; indice++) {
                    free(nos[indice].noA);
                    free(nos[indice].noR);
                    free(nos[indice].noP);
                    free(nos[indice].rnpA);
                }
                free(nos);
            }
        } else //não existem setores a jusante para ser restabelecidos
        {
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            idChaveAberta = Malloc(long int, 1);
            idChaveFechada = Malloc(long int, 1);
            buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                    configuracoesParam, numChaveAberta, idChaveAberta,
                    idChaveFechada, setorFaltaParam, noAnteriorFalta,
                    1, listaChavesParam);
            nosPRA = Malloc(NOSPRA, 1);
            nosPRA[0].a = -1;
            nosPRA[0].p = setorFaltaParam;
            nosPRA[0].r = -1;
            estadoInicialCA = Malloc(BOOL, 1);
            estadoInicialCF = Malloc(BOOL, 1);
            estadoInicialCA[0] = false;
            estadoInicialCF[0] = false;
            alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1, &rnpOrigem);
            //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
            indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
            rnpP =matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
            grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
            //conta os consumidores sem fornecimento
            consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
            consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
            //copia os setores anteriores a falta para a nova rnp origem
            for (indice1 = 0; indice1 < indiceP; indice1++) {
                rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
            }
            //remove o setor em falta da RNP e copia os demais se existir
            if ((indiceP + 1) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
               for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
                    adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                }

            }

            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;

            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                if (indice1 != rnpP)
                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

            }
            copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
            //insere no vetor Pi a nova solução
            atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                    idChaveAberta, idChaveFechada, 1, casoManobra, 
                    estadoInicialCA, estadoInicialCF, nosPRA, ESO);
            idNovaConfiguracaoParam[0]++;
        }
    } else //o setor em falta é o ultimo da rnp
    {
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
        idChaveAberta = Malloc(long int, 1);
        idChaveFechada = Malloc(long int, 1);
        buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                configuracoesParam, numChaveAberta, idChaveAberta,
                idChaveFechada, setorFaltaParam, noAnteriorFalta,
                1, listaChavesParam);
        nosPRA = Malloc(NOSPRA, 1);
        nosPRA[0].a = -1;
        nosPRA[0].p = setorFaltaParam;
        nosPRA[0].r = -1;
        estadoInicialCA = Malloc(BOOL, 1);
        estadoInicialCF = Malloc(BOOL, 1);
        estadoInicialCA[0] = false;
        estadoInicialCF[0] = false;
        //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
        tamanhoTemporario = 1;
        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
        //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
        indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
        rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
        grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
        //conta os consumidores sem fornecimento
        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
        //copia os setores anteriores a falta para a nova rnp origem
        for (indice1 = 0; indice1 < indiceP; indice1++) {
            rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
            rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
        }

        rnpOrigem.numeroNos = configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
        //insere a nova rnpOrigem na configuração e atualiza os ponteiros das demais
        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
        for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
            if (indice1 != rnpP)
                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
        }
        copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
        insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
        //insere no vetor Pi a nova solução
        atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                idChaveAberta, idChaveFechada, 1, casoManobra, estadoInicialCA, estadoInicialCF, nosPRA, ESO);
        idNovaConfiguracaoParam[0]++;
    }
}

/**
 * Por Leandro:
 * Descrição: trata-se da função "isolaRestabeleceTodasOpcoesHeuristica()" modificada para que:
 *
 * a) as manobras de isolação e restauração dos agrupamentos de setores desligados
 * sigam uma sequência que priorize a restauração dos consumidores com maior prioridade.Assim, obtem-se diretamentem uma sequência tal de manobras
 * que permita que seja primeiramente isolado e restaurado o agrupamento de setor com maior potência não suprida a consumidores com Prioridade Alta, em
 * seguida com prioridade Intermediária, depois, Baixa e, por fim, sem prioridade. Desta forma, dados dois agrupamentos de setores fora de serviço, será reconectados primeiro
 * aquele com maior valor de potência fora de serviço para Consumidores com Prioridade Alta. Caso haja empate, aquele com maior potẽncia fora de serviço para Consumidores co Prioridade
 * Intermediária e assim por diante.
 *
 * b) agrupamentos de setores possíveis de serem restaurados sejam deixados fora de serviço quando os mesmos são formados apenas por setores sem a presença de cargas.
 * Em outras palavras, mesmo que um agrupamento de setores possa ser restaurado por meio de chaves NA, ele será deixado fora de serviço e
 * conectado ao setor em falta se no mesmo não houverem consumidores. Isto é feito porque a reconexão de tal agrupamento de setores não traria benefícios a nenhum consumidor
 * e ainda atrasaria o processo de restabelecimento.
 *
 * c) possam ser passados novos parametros necessário a execução das duas modificações anteriores
 *
 * A descrição da função ""isolaRestabeleceTodasOpcoesHeuristica()" é:
 * Para reestabelecimento a Configuração Inicial do SDR é aquela com o setor em falta isolado e as cargas a jusante reconectadas. Isto é para Restabelecimento, a Configuração
 * Inicial é diferente da Topologia Inicial. Nesse caso, o vetor 'estadoInicial', que armazena os estados da chave na Configuração Inicial, deve ser atualizado após a definição
 * das chaves que isolão a falta e reconectão as cargas a jusante. Porém, isso não acontecia. Para corrigir essa falha, além das duas linhas de código que foram
 * inserirdas ao final da função 'numeroManobras', foram inseridos também os caminhos abaixo que visam determinar o nome da chave que conecta o setor em falta a seu
 * alimentador, chave esta que deve ser aberta para isolar a falta. Esses comandos determinam o nome dessa chave e alteram o seu estado para 'normalmenteAberta' no vetor 'estadoInical'.
 *
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param setorFaltaParam
 * @param estadoInicial
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param listaChavesParam
 * @param matrizBParam
 */
void isolaRestabeleceTodasOpcoesHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int setorFaltaParam,
		ESTADOCHAVE *estadoInicial, long int * numChaveAberta, long int *numChaveFechada, long int *idNovaConfiguracaoParam,
		LISTACHAVES *listaChavesParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam) {
    int tamanhoTemporario;
    long int p, a, indice, indice1, indiceP, indiceR, indiceL, r,
            indiceSetorFalta, indiceA;
    long int noAnteriorFalta;
    long int indiceLFalta, idConfiguracaoBase;
    int rnpP, rnpA;
    int numeroFalta, numeroAJusante, posicao;
    long int nosFalta[500]; //é o número máximo de setores em falta ou a jusantes
    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
    long int totalCombinacoes;
    long int *idChaveAberta, *idChaveFechada;
    long int consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    long int consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    int indiceChave;
    int **indiceRestabelecimento;
    int blocoRestabelecimento=0;
    int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4, linhaIndiceAux1, linhaIndiceAux2;
    int colunaPI;
    RNP rnpOrigem;
    RNP rnpDestino;
    NOSRESTABELECIMENTO *nos;
    int casoManobra;
    BOOL *estadoInicialCA;
    BOOL *estadoInicialCF;
    NOSPRA *nosPRA;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida; //Por Leandro
    double potenciaNaoSupridaTotal; //Por Leandro

    //IMPRESSAO DE DADOS PARA ANALISE DOS RESULTADOS
//    char nomeArquivo[120];
//    FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
    //todos os setores do sistema
//    sprintf(nomeArquivo, "SAIDA_testeConfiguracaoInicial.dad");
//    arquivo1 = fopen(nomeArquivo, "a");
    NIVEISDEPRIORIDADEATENDIMENTO *potenciaRestaurada, potenciaRestauradaAux; //Por Leandro
    int *sequenciaRestauracao, sequenciaRestauracaoAux; //Por Leandro

    //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
    grafoSetoresParam[setorFaltaParam].setorFalta = true;
    //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
  //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
   // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;

    //RECUPERA A COLUNAPI DO SETOR EM FALTA
    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
    rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
    indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
    indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

    //LOCALIZA O SETOR ANTERIOR A FALTA
    indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o éndice do nó antecessor ao nó sob falta
    while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
        indice1--;
    }
    noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

    //verifica se o setor em falta não é o último da RNP
    if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
        numeroAJusante = 0;
        //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
        if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
            indice1 = indiceP + 1;
            nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
            numeroAJusante++;
            //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
            while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
                if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                    nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    numeroAJusante++;
                }
                indice1++;
            }
            indiceLFalta = indice1 - 1;
            indice = 0;
            nosFalta[0] = setorFaltaParam;
            numeroFalta = 1;
            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
            totalCombinacoes = 1;
            blocoRestabelecimento = 0;
            //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as opções de restabelecimento
            while (indice < numeroAJusante) {
                a = 0;
                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
                //recupera o indice final da subárvore dos setore a jusante
                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
                nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
                buscaTodosNosRestabelecimentoCorrigida(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);

                // Por Leandro: calcula a potência ativa total a ser restaurada
                potenciaNaoSuprida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idConfiguracaoParam, indiceP, indiceL, rnpP, matrizBParam, grafoSDRParam);
                potenciaNaoSupridaTotal = potenciaNaoSuprida.consumidoresSemPrioridade + potenciaNaoSuprida.consumidoresPrioridadeBaixa + potenciaNaoSuprida.consumidoresPrioridadeIntermediaria + potenciaNaoSuprida.consumidoresPrioridadeAlta;

                /* Por Leandro:
                 * Um determinado agrupamneto de setores será restaurado somente se:
                 * 1) houver opções (chaves NAs) que permitam reconectá-lo à rede
                 * 2) E, concomitantemente, houver algum consumidor neste agrupamento de setores*/
                //printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
                if (nos[blocoRestabelecimento].numeroNos > 0 && potenciaNaoSupridaTotal > 0) {
                    totalCombinacoes *= nos[blocoRestabelecimento].numeroNos;
                    blocoRestabelecimento++;

                } else {
                    free(nos[blocoRestabelecimento].noA);
                    free(nos[blocoRestabelecimento].noR);
                    free(nos[blocoRestabelecimento].noP);
                    free(nos[blocoRestabelecimento].rnpA);
                    //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
                    nosFalta[numeroFalta] = nosJusanteFalta[indice];
                    numeroFalta++;
                    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                    indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
                    // printf("%d\n",indiceAux1);
                    while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
                        nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
                        numeroFalta++;
                        indiceAux1++;
                    }
                }
                indice++;
            }
            //******************************************************************************************************//
            //salva no arquivo quantas combinacoes de restabelecimento existem e quantos setores são agrupados a falta
//            fprintf(arquivo1, "Combinacoes;%ld;Setores;%d;", totalCombinacoes, numeroFalta);
            if (blocoRestabelecimento == 0) { //não existe opção de restabelecimento para os setores a jusante.
//                fprintf(arquivo1, "SAR\n");
//                fclose(arquivo1);
                //cria um novo individuo isolando a falta e os setores sem restabelecimento
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
                //isola a falta
                idChaveAberta = Malloc(long int, 1);
                idChaveFechada = Malloc(long int, 1);
                buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                        configuracoesParam, numChaveAberta, idChaveAberta,
                        idChaveFechada, setorFaltaParam, noAnteriorFalta,
                        1, listaChavesParam);
                nosPRA = Malloc(NOSPRA, 1);
                nosPRA[0].a = -1;
                nosPRA[0].p = setorFaltaParam;
                nosPRA[0].r = -1;
                estadoInicialCA = Malloc(BOOL, 1);
                estadoInicialCF = Malloc(BOOL, 1);
                estadoInicialCA[0] = false;
                estadoInicialCF[0] = false;
                //aloca a nova rnp
                alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta, &rnpOrigem);
                //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                 colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[0], idConfiguracaoParam);
                 indiceP = matrizPiParam[nosFalta[0]].colunas[colunaPI].posicao;
                 rnpP = matrizPiParam[nosFalta[0]].colunas[colunaPI].idRNP;
                 copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
                 //marca os setores como em falta e conta os consumidores
                 for(indice1 = 0; indice1 < numeroFalta; indice1++)
                 {
                    grafoSetoresParam[nosFalta[indice1]].setorFalta = true;
                    //conta os consumidores sem fornecimento
                    consumidoresSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidores;
                    consumidoresEspeciaisSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidoresEspeciais;
                 }
                //copia os setores anteriores a falta para a nova rnp origem
                for (indice1 = 0; indice1 < indiceP; indice1++) {
                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
                }
                //remove o setor em falta da RNP e copia os demais se existir
                if ((indiceP + numeroFalta) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                    for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta; indice1++) {
                        rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].idNo;
                        rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].profundidade;
                        adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                    }

                }

                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
                for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                    if (indice1 != rnpP)
                        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

                }
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
                //insere no vetor Pi a nova solução
                atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                        idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), 1, estadoInicialCA, estadoInicialCF, nosPRA, ESO);
                idNovaConfiguracaoParam[0]++;
            } else {
//                fclose(arquivo1);
                //*************************************************************************************************//
                indiceRestabelecimento = (int **) malloc(totalCombinacoes * sizeof (int*));
                //cada coluna da matriz indiceRestabelecimento contém os indices do vetor de opções de restabelecimento para
                //cada bloco de setores a jusante que poderá ser restabelecido.
                //Cada linha contém uma combinação das opções de restabelecimento para cada bloco
                for (indice = 0; indice < totalCombinacoes; indice++) {
                    indiceRestabelecimento[indice] = (int *) malloc(blocoRestabelecimento * sizeof (int));
                }

                //preenche as combinacoes de indices das opçoes de restabelecimento para cada raiz de bloco de setores a jusante
                indiceAux3 = 0;
                for (indice = 0; indice < blocoRestabelecimento; indice++) {
                    if (nos[indice].numeroNos > 0) {
                        repeticoes = totalCombinacoes / nos[indice].numeroNos;
                        indiceAux4 = 0;
                        for (indiceAux1 = 0; indiceAux1 < repeticoes; indiceAux1++) {
                            for (indiceAux2 = 0; indiceAux2 < nos[indice].numeroNos; indiceAux2++) {
                                indiceRestabelecimento[indiceAux4][indiceAux3] = indiceAux2;
                                indiceAux4++;
                            }
                        }
                        indiceAux3++;
                    }
                }

                //Por Leandro: Determinação da ordem em que as Subárvores (ou agrupamento de setores) à jusante da Falta serão restauradas
                //Por Leandro: Parte 1/2 - Obtenção dos valores de potência ativa a ser restaurada
                potenciaRestaurada = Malloc(NIVEISDEPRIORIDADEATENDIMENTO, blocoRestabelecimento);
                sequenciaRestauracao = Malloc(int, blocoRestabelecimento);
                for (indice = 0; indice < blocoRestabelecimento; indice++) { //Determinação do somatório de potência Ativa em cada Subárvore, por nível de prioridade
                	p = nos[indice].noP[0];
                	potenciaRestaurada[indice] = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(configuracoesParam, idConfiguracaoParam, p, matrizBParam, matrizPiParam, vetorPiParam, grafoSDRParam);
                	sequenciaRestauracao[indice] = indice;
                }
                //Por Leandro: Parte 2/2 - Ordenação das subárvores com base nos valores de "potenciaRestaurada"
                for (indiceAux1 = 0; indiceAux1 < blocoRestabelecimento; indiceAux1++){
                    for (indiceAux2 = indiceAux1 + 1; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
                    	if((floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta)  < floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta)) ||
                    	   (floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeIntermediaria) <  floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeIntermediaria)) ||
						   (floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeIntermediaria) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeBaixa) <  floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeBaixa)) ||
						   (floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeIntermediaria) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeBaixa) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeBaixa)  && floor(potenciaRestaurada[indiceAux1].consumidoresSemPrioridade) < floor(potenciaRestaurada[indiceAux2].consumidoresSemPrioridade))
                    	){
                    		sequenciaRestauracaoAux = sequenciaRestauracao[indiceAux2];
                    		potenciaRestauradaAux = potenciaRestaurada[indiceAux2];
                    		for(indice = indiceAux2; indice > indiceAux1; indice--){
                    			sequenciaRestauracao[indice] = sequenciaRestauracao[indice - 1];
                    			potenciaRestaurada[indice] = potenciaRestaurada[indice - 1];
                    		}
                    		sequenciaRestauracao[indiceAux1] = sequenciaRestauracaoAux;
                    		potenciaRestaurada[indiceAux1] = potenciaRestauradaAux;


//                    		for(indice = 0; indice < blocoRestabelecimento; indice++){
//                    			if(sequenciaRestauracao[indice] == indiceAux1)
//                    				linhaIndiceAux1 = indice;
//                    			if(sequenciaRestauracao[indice] == indiceAux2)
//                    				linhaIndiceAux2 = indice;
//                    		}
//                    		sequenciaRestauracao[linhaIndiceAux1] = indiceAux2;
//                    		sequenciaRestauracao[linhaIndiceAux2] = indiceAux1;

                    	}
                    }
                }


                //aplica as combinações de restabelecimento obtidas.
                for (indiceAux1 = 0; indiceAux1 < totalCombinacoes; indiceAux1++) {
                    idConfiguracaoBase = idConfiguracaoParam;
                    indiceChave = 1;
                    idChaveAberta = Malloc(long int, (blocoRestabelecimento + 1));
                    idChaveFechada = Malloc(long int, (blocoRestabelecimento + 1));
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
                    buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                            configuracoesParam, numChaveAberta, idChaveAberta,
                            idChaveFechada, setorFaltaParam, noAnteriorFalta,
                            (blocoRestabelecimento + 1), listaChavesParam);
                    casoManobra = 1;
                    estadoInicialCA = Malloc(BOOL, (blocoRestabelecimento + 1));
                    estadoInicialCF = Malloc(BOOL, (blocoRestabelecimento + 1));

                    nosPRA = Malloc(NOSPRA, (blocoRestabelecimento + 1));
                    estadoInicialCA[0] = false;
                    estadoInicialCF[0] = false;
                    nosPRA[0].a = -1;
                    nosPRA[0].p = setorFaltaParam;
                    nosPRA[0].r = -1;
                    consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
                    consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
                    copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
                    insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
                    for (indiceAux2 = 0; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
                        //printf("%d \t", indiceRestabelecimento[indiceAux1][indiceAux2]);
                        indice = sequenciaRestauracao[indiceAux2]; //Por Leandro: Mudou aqui que as subárvores sejam restaudas numa sequência que respeite os níves de prioridade de atendimento
                        posicao = indiceRestabelecimento[indiceAux1][indice]; //Por Leandro: Mudou aqui que as subárvores sejam restaudas numa sequência que respeite os níves de prioridade de atendimento
                        rnpA = nos[indice].rnpA[posicao];
                        a = nos[indice].noA[posicao];
                        r = nos[indice].noR[posicao];
                        p = nos[indice].noP[posicao];
                        if (rnpA != -1) {
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idConfiguracaoBase);
                            indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
                            rnpA = matrizPiParam[a].colunas[colunaPI].idRNP;

                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idConfiguracaoBase);
                            indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, p, idConfiguracaoBase);
                            indiceP = matrizPiParam[p].colunas[colunaPI].posicao;
                            indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);
                            p = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceP].idNo;
                            r = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceR].idNo;
                            a = configuracoesParam[idConfiguracaoBase].rnp[rnpA].nos[indiceA].idNo;
                            nosPRA[indiceAux2+1].a = a;
                            nosPRA[indiceAux2+1].p = p;
                            nosPRA[indiceAux2+1].r = r;
                            //localiza a chave a ser aberta
                            indice1 = 0;
                            while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != p) {
                                indice1++;
                            }
                            idChaveAberta[indiceAux2 + 1] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
                            //localiza a chave a ser fechada
                            indice1 = 0;
                            while (grafoSetoresParam[r].setoresAdjacentes[indice1] != a) {
                                indice1++;
                            }
                            idChaveFechada[indiceAux2 + 1] = grafoSetoresParam[r].idChavesAdjacentes[indice1];
                            //ROTINA PARA CALCULAR OS PARES DE MANOBRAS
                            numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[indiceAux2 + 1], idChaveFechada[indiceAux2 + 1], listaChavesParam,
                                    idNovaConfiguracaoParam[0], estadoInicial, &estadoInicialCA[indiceAux2+1], &estadoInicialCF[indiceAux2+1], idNovaConfiguracaoParam[0], &casoManobra);

                            tamanhoTemporario = indiceL - indiceP + 1;
                            if(rnpA != rnpP){
								//realiza a alocação das RNPs que serão alteradas
								alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
								alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
								//obtém a nova rnp de destino
								constroiRNPDestino(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam[0]);
								constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam[0]);
								//libera a memória utilizada pela rnp do setor em falta

								//###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############
								configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpA] = rnpDestino;
								configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
								insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpA);
								for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNP; indice1++) {
									if (indice1 != rnpA && indice1 != rnpP)
										configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoBase].rnp[indice1];
								}
                        	}
                            else{
                            	obtemConfiguracaoOrigemDestino(configuracoesParam, idConfiguracaoBase, idNovaConfiguracaoParam[0], indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPiParam);
                            }


                        }
                        idConfiguracaoBase = idNovaConfiguracaoParam[0];
                        vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
                    }

                    //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
                    tamanhoTemporario = 1;

                    //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                    for (indice = 0; indice < numeroFalta; indice++) {
                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam[0]);
                        indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
                        rnpP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].idRNP;
                        grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
                        //conta os consumidores sem fornecimento
                        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
                        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
                        if ((indiceP + 1) <= configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos) {
                            for (indice1 = indiceP; indice1 < configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1; indice1++) {
                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].idNo;
                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].profundidade;
                                adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                            }
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1;
                        }
                    }
                    //atualiza consumidores sem fornecimento
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                    //insere no vetor Pi a nova solução
                    atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                            idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), casoManobra,
                            estadoInicialCA, estadoInicialCF, nosPRA, ESO);
                    idNovaConfiguracaoParam[0]++;

                }
                for (indice = 0; indice < blocoRestabelecimento; indice++) {
                	nos[indice].noA  = NULL; free(nos[indice].noA);
                	nos[indice].noR  = NULL; free(nos[indice].noR);
                	nos[indice].noP  = NULL; free(nos[indice].noP);
                	nos[indice].rnpA = NULL; free(nos[indice].rnpA);
                }
                nos = NULL; free(nos);
                potenciaRestaurada = NULL; free(potenciaRestaurada);
                sequenciaRestauracao = NULL; free(sequenciaRestauracao);
            }
        } else //não existem setores a jusante para ser restabelecidos
        {
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            idChaveAberta = Malloc(long int, 1);
            idChaveFechada = Malloc(long int, 1);
            buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                    configuracoesParam, numChaveAberta, idChaveAberta,
                    idChaveFechada, setorFaltaParam, noAnteriorFalta,
                    1, listaChavesParam);
            nosPRA = Malloc(NOSPRA, 1);
            nosPRA[0].a = -1;
            nosPRA[0].p = setorFaltaParam;
            nosPRA[0].r = -1;
            estadoInicialCA = Malloc(BOOL, 1);
            estadoInicialCF = Malloc(BOOL, 1);
            estadoInicialCA[0] = false;
            estadoInicialCF[0] = false;
            alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1, &rnpOrigem);
            //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
            indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
            rnpP =matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
            grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
            //conta os consumidores sem fornecimento
            consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
            consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
            //copia os setores anteriores a falta para a nova rnp origem
            for (indice1 = 0; indice1 < indiceP; indice1++) {
                rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
            }
            //remove o setor em falta da RNP e copia os demais se existir
            if ((indiceP + 1) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
               for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
                    adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                }

            }

            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;

            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                if (indice1 != rnpP)
                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

            }
            copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
            //insere no vetor Pi a nova solução
            atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                    idChaveAberta, idChaveFechada, 1, casoManobra,
                    estadoInicialCA, estadoInicialCF, nosPRA, ESO);
            idNovaConfiguracaoParam[0]++;
        }
    } else //o setor em falta é o ultimo da rnp
    {
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
        idChaveAberta = Malloc(long int, 1);
        idChaveFechada = Malloc(long int, 1);
        buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                configuracoesParam, numChaveAberta, idChaveAberta,
                idChaveFechada, setorFaltaParam, noAnteriorFalta,
                1, listaChavesParam);
        nosPRA = Malloc(NOSPRA, 1);
        nosPRA[0].a = -1;
        nosPRA[0].p = setorFaltaParam;
        nosPRA[0].r = -1;
        estadoInicialCA = Malloc(BOOL, 1);
        estadoInicialCF = Malloc(BOOL, 1);
        estadoInicialCA[0] = false;
        estadoInicialCF[0] = false;
        //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
        tamanhoTemporario = 1;
        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
        //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
        indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
        rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
        grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
        //conta os consumidores sem fornecimento
        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
        //copia os setores anteriores a falta para a nova rnp origem
        for (indice1 = 0; indice1 < indiceP; indice1++) {
            rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
            rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
        }

        rnpOrigem.numeroNos = configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
        //insere a nova rnpOrigem na configuração e atualiza os ponteiros das demais
        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
        for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
            if (indice1 != rnpP)
                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
        }
        copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
        insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
        //insere no vetor Pi a nova solução
        atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                idChaveAberta, idChaveFechada, 1, casoManobra, estadoInicialCA, estadoInicialCF, nosPRA, ESO);
        idNovaConfiguracaoParam[0]++;
    }
}
//void isolaRestabeleceTodasOpcoesHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
//        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int setorFaltaParam,
//		ESTADOCHAVE *estadoInicial, long int * numChaveAberta, long int *numChaveFechada, long int *idNovaConfiguracaoParam,
//		LISTACHAVES *listaChavesParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam) {
//    int tamanhoTemporario;
//    long int p, a, indice, indice1, indiceP, indiceR, indiceL, r,
//            indiceSetorFalta, indiceA;
//    long int noAnteriorFalta;
//    long int indiceLFalta, idConfiguracaoBase;
//    int rnpP, rnpA;
//    int numeroFalta, numeroAJusante, posicao;
//    long int nosFalta[500]; //é o número máximo de setores em falta ou a jusantes
//    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
//    long int totalCombinacoes;
//    long int *idChaveAberta, *idChaveFechada;
//    long int consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
//    long int consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
//    int indiceChave;
//    int **indiceRestabelecimento;
//    int blocoRestabelecimento=0;
//    int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4;
//    int colunaPI;
//    RNP rnpOrigem;
//    RNP rnpDestino;
//    NOSRESTABELECIMENTO *nos;
//    int casoManobra;
//    BOOL *estadoInicialCA;
//    BOOL *estadoInicialCF;
//    NOSPRA *nosPRA;
//    NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida; //Por Leandro
//    double potenciaNaoSupridaTotal; //Por Leandro
//
//    //IMPRESSAO DE DADOS PARA ANALISE DOS RESULTADOS
////    char nomeArquivo[120];
////    FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
//    //todos os setores do sistema
////    sprintf(nomeArquivo, "SAIDA_testeConfiguracaoInicial.dad");
////    arquivo1 = fopen(nomeArquivo, "a");
//    NIVEISDEPRIORIDADEATENDIMENTO *potenciaRestaurada; //Por Leandro
//    int *sequenciaRestauracao, sequenciaRestauracaoAux; //Por Leandro
//
//    //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
//    grafoSetoresParam[setorFaltaParam].setorFalta = true;
//    //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
//  //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
//   // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;
//
//    //RECUPERA A COLUNAPI DO SETOR EM FALTA
//    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
//    rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
//    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
//    indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
//    indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.
//
//    //LOCALIZA O SETOR ANTERIOR A FALTA
//    indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o éndice do nó antecessor ao nó sob falta
//    while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
//        indice1--;
//    }
//    noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//
//    //verifica se o setor em falta não é o último da RNP
//    if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
//        numeroAJusante = 0;
//        //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
//        if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
//            indice1 = indiceP + 1;
//            nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
//            numeroAJusante++;
//            //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
//            while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
//                if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
//                    nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//                    numeroAJusante++;
//                }
//                indice1++;
//            }
//            indiceLFalta = indice1 - 1;
//            indice = 0;
//            nosFalta[0] = setorFaltaParam;
//            numeroFalta = 1;
//            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
//            totalCombinacoes = 1;
//            blocoRestabelecimento = 0;
//            //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as opções de restabelecimento
//            while (indice < numeroAJusante) {
//                a = 0;
//                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
//                indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
//                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
//                //recupera o indice final da subárvore dos setore a jusante
//                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
//                nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
//                nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
//                nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
//                nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
//                buscaTodosNosRestabelecimento(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
//
//                // Por Leandro: calcula a potência ativa total a ser restaurada
//                potenciaNaoSuprida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idConfiguracaoParam, indiceP, indiceL, rnpP, matrizBParam, grafoSDRParam);
//                potenciaNaoSupridaTotal = potenciaNaoSuprida.consumidoresSemPrioridade + potenciaNaoSuprida.consumidoresPrioridadeBaixa + potenciaNaoSuprida.consumidoresPrioridadeIntermediaria + potenciaNaoSuprida.consumidoresPrioridadeAlta;
//
//                /* Por Leandro:
//                 * Um determinado agrupamneto de setores será restaurado somente se:
//                 * 1) houver opções (chaves NAs) que permitam reconectá-lo à rede
//                 * 2) E, concomitantemente, houver algum consumidor neste agrupamento de setores*/
//                //printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
//                if (nos[blocoRestabelecimento].numeroNos > 0 && potenciaNaoSupridaTotal > 0) {
//                    totalCombinacoes *= nos[blocoRestabelecimento].numeroNos;
//                    blocoRestabelecimento++;
//
//                } else {
//                    free(nos[blocoRestabelecimento].noA);
//                    free(nos[blocoRestabelecimento].noR);
//                    free(nos[blocoRestabelecimento].noP);
//                    free(nos[blocoRestabelecimento].rnpA);
//                    //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
//                    nosFalta[numeroFalta] = nosJusanteFalta[indice];
//                    numeroFalta++;
//                    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
//                    indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
//                    // printf("%d\n",indiceAux1);
//                    while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
//                        nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
//                        numeroFalta++;
//                        indiceAux1++;
//                    }
//                }
//                indice++;
//            }
//            //******************************************************************************************************//
//            //salva no arquivo quantas combinacoes de restabelecimento existem e quantos setores são agrupados a falta
////            fprintf(arquivo1, "Combinacoes;%ld;Setores;%d;", totalCombinacoes, numeroFalta);
//            if (blocoRestabelecimento == 0) { //não existe opção de restabelecimento para os setores a jusante.
////                fprintf(arquivo1, "SAR\n");
////                fclose(arquivo1);
//                //cria um novo individuo isolando a falta e os setores sem restabelecimento
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
//                //isola a falta
//                idChaveAberta = Malloc(long int, 1);
//                idChaveFechada = Malloc(long int, 1);
//                buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                        configuracoesParam, numChaveAberta, idChaveAberta,
//                        idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                        1, listaChavesParam);
//                nosPRA = Malloc(NOSPRA, 1);
//                nosPRA[0].a = -1;
//                nosPRA[0].p = setorFaltaParam;
//                nosPRA[0].r = -1;
//                estadoInicialCA = Malloc(BOOL, 1);
//                estadoInicialCF = Malloc(BOOL, 1);
//                estadoInicialCA[0] = false;
//                estadoInicialCF[0] = false;
//                //aloca a nova rnp
//                alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta, &rnpOrigem);
//                //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//                 colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[0], idConfiguracaoParam);
//                 indiceP = matrizPiParam[nosFalta[0]].colunas[colunaPI].posicao;
//                 rnpP = matrizPiParam[nosFalta[0]].colunas[colunaPI].idRNP;
//                 copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//                 //marca os setores como em falta e conta os consumidores
//                 for(indice1 = 0; indice1 < numeroFalta; indice1++)
//                 {
//                    grafoSetoresParam[nosFalta[indice1]].setorFalta = true;
//                    //conta os consumidores sem fornecimento
//                    consumidoresSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidores;
//                    consumidoresEspeciaisSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidoresEspeciais;
//                 }
//                //copia os setores anteriores a falta para a nova rnp origem
//                for (indice1 = 0; indice1 < indiceP; indice1++) {
//                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
//                }
//                //remove o setor em falta da RNP e copia os demais se existir
//                if ((indiceP + numeroFalta) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
//                    for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta; indice1++) {
//                        rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].idNo;
//                        rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].profundidade;
//                        adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
//                    }
//
//                }
//
//                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//                for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
//                    if (indice1 != rnpP)
//                        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
//
//                }
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//                insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//                //insere no vetor Pi a nova solução
//                atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                        idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), 1, estadoInicialCA, estadoInicialCF, nosPRA, ESO);
//                idNovaConfiguracaoParam[0]++;
//            } else {
////                fclose(arquivo1);
//                //*************************************************************************************************//
//                indiceRestabelecimento = (int **) malloc(totalCombinacoes * sizeof (int*));
//                //cada coluna da matriz indiceRestabelecimento contém os indices do vetor de opções de restabelecimento para
//                //cada bloco de setores a jusante que poderá ser restabelecido.
//                //Cada linha contém uma combinação das opções de restabelecimento para cada bloco
//                for (indice = 0; indice < totalCombinacoes; indice++) {
//                    indiceRestabelecimento[indice] = (int *) malloc(blocoRestabelecimento * sizeof (int));
//                }
//
//                //preenche as combinacoes de indices das opçoes de restabelecimento para cada raiz de bloco de setores a jusante
//                indiceAux3 = 0;
//                for (indice = 0; indice < blocoRestabelecimento; indice++) {
//                    if (nos[indice].numeroNos > 0) {
//                        repeticoes = totalCombinacoes / nos[indice].numeroNos;
//                        indiceAux4 = 0;
//                        for (indiceAux1 = 0; indiceAux1 < repeticoes; indiceAux1++) {
//                            for (indiceAux2 = 0; indiceAux2 < nos[indice].numeroNos; indiceAux2++) {
//                                indiceRestabelecimento[indiceAux4][indiceAux3] = indiceAux2;
//                                indiceAux4++;
//                            }
//                        }
//                        indiceAux3++;
//                    }
//                }
//
//                //Por Leandro: Determinação da ordem em que as Subárvores (ou agrupamento de setores) à jusante da Falta serão restauradas
//                //Por Leandro: Parte 1/2 - Obtenção dos valores de potência ativa a ser restaurada
//                potenciaRestaurada = Malloc(NIVEISDEPRIORIDADEATENDIMENTO, blocoRestabelecimento);
//                sequenciaRestauracao = Malloc(int, blocoRestabelecimento);
//                for (indice = 0; indice < blocoRestabelecimento; indice++) { //Determinação do somatório de potência Ativa em cada Subárvore, por nível de prioridade
//                	p = nos[indice].noP[0];
//                	potenciaRestaurada[indice] = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(configuracoesParam, idConfiguracaoParam, p, matrizBParam, matrizPiParam, vetorPiParam, grafoSDRParam);
//                	sequenciaRestauracao[indice] = indice;
//                }
//                //Por Leandro: Parte 2/2 - Ordenação das subárvores com base nos valores de "potenciaRestaurada"
//                for (indiceAux1 = 0; indiceAux1 < blocoRestabelecimento; indiceAux1++){
//                    for (indiceAux2 = indiceAux1 + 1; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
//                    	if((floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta)  < floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta)) ||
//                    	   (floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeIntermediaria) <  floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeIntermediaria)) ||
//						   (floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeIntermediaria) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeBaixa) <  floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeBaixa)) ||
//						   (floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeAlta) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeAlta) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeIntermediaria) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada[indiceAux1].consumidoresPrioridadeBaixa) == floor(potenciaRestaurada[indiceAux2].consumidoresPrioridadeBaixa)  && floor(potenciaRestaurada[indiceAux1].consumidoresSemPrioridade) < floor(potenciaRestaurada[indiceAux2].consumidoresSemPrioridade))
//                    	){
//                    		sequenciaRestauracaoAux = sequenciaRestauracao[indiceAux2];
//                    		for(indice = indiceAux2; indice > indiceAux1; indice--)
//                    			sequenciaRestauracao[indice] = sequenciaRestauracao[indice - 1];
//                    		sequenciaRestauracao[indiceAux1] = sequenciaRestauracaoAux;
//                    	}
//                    }
//                }
//
//
//                //aplica as combinações de restabelecimento obtidas.
//                for (indiceAux1 = 0; indiceAux1 < totalCombinacoes; indiceAux1++) {
//                    idConfiguracaoBase = idConfiguracaoParam;
//                    indiceChave = 1;
//                    idChaveAberta = Malloc(long int, (blocoRestabelecimento + 1));
//                    idChaveFechada = Malloc(long int, (blocoRestabelecimento + 1));
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
//                    buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                            configuracoesParam, numChaveAberta, idChaveAberta,
//                            idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                            (blocoRestabelecimento + 1), listaChavesParam);
//                    casoManobra = 1;
//                    estadoInicialCA = Malloc(BOOL, (blocoRestabelecimento + 1));
//                    estadoInicialCF = Malloc(BOOL, (blocoRestabelecimento + 1));
//
//                    nosPRA = Malloc(NOSPRA, (blocoRestabelecimento + 1));
//                    estadoInicialCA[0] = false;
//                    estadoInicialCF[0] = false;
//                    nosPRA[0].a = -1;
//                    nosPRA[0].p = setorFaltaParam;
//                    nosPRA[0].r = -1;
//                    consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
//                    consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
//                    copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//                    insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//                    for (indiceAux2 = 0; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
//                        //printf("%d \t", indiceRestabelecimento[indiceAux1][indiceAux2]);
//                        indice = sequenciaRestauracao[indiceAux2]; //Por Leandro: Mudou aqui que as subárvores sejam restaudas numa sequência que respeite os níves de prioridade de atendimento
//                        posicao = indiceRestabelecimento[indiceAux1][indice]; //Por Leandro: Mudou aqui que as subárvores sejam restaudas numa sequência que respeite os níves de prioridade de atendimento
//                        rnpA = nos[indice].rnpA[posicao];
//                        a = nos[indice].noA[posicao];
//                        r = nos[indice].noR[posicao];
//                        p = nos[indice].noP[posicao];
//                        if (rnpA != -1) {
//                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idConfiguracaoBase);
//                            indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
//                            rnpA = matrizPiParam[a].colunas[colunaPI].idRNP;
//
//                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idConfiguracaoBase);
//                            indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
//                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, p, idConfiguracaoBase);
//                            indiceP = matrizPiParam[p].colunas[colunaPI].posicao;
//                            indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);
//                            p = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceP].idNo;
//                            r = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceR].idNo;
//                            a = configuracoesParam[idConfiguracaoBase].rnp[rnpA].nos[indiceA].idNo;
//                            nosPRA[indiceAux2+1].a = a;
//                            nosPRA[indiceAux2+1].p = p;
//                            nosPRA[indiceAux2+1].r = r;
//                            //localiza a chave a ser aberta
//                            indice1 = 0;
//                            while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != p) {
//                                indice1++;
//                            }
//                            idChaveAberta[indiceAux2 + 1] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
//                            //localiza a chave a ser fechada
//                            indice1 = 0;
//                            while (grafoSetoresParam[r].setoresAdjacentes[indice1] != a) {
//                                indice1++;
//                            }
//                            idChaveFechada[indiceAux2 + 1] = grafoSetoresParam[r].idChavesAdjacentes[indice1];
//                            //ROTINA PARA CALCULAR OS PARES DE MANOBRAS
//                            numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[indiceAux2 + 1], idChaveFechada[indiceAux2 + 1], listaChavesParam,
//                                    idNovaConfiguracaoParam[0], estadoInicial, &estadoInicialCA[indiceAux2+1], &estadoInicialCF[indiceAux2+1], idNovaConfiguracaoParam[0], &casoManobra);
//
//                            tamanhoTemporario = indiceL - indiceP + 1;
//                            //realiza a alocação das RNPs que serão alteradas
//                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
//                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
//                            //obtém a nova rnp de destino
//                            constroiRNPDestino(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam[0]);
//                            constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam[0]);
//                            //libera a memória utilizada pela rnp do setor em falta
//
//                            //###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############
//                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpA] = rnpDestino;
//                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//                            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpA);
//                            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNP; indice1++) {
//                                if (indice1 != rnpA && indice1 != rnpP)
//                                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoBase].rnp[indice1];
//                            }
//
//                        }
//                        idConfiguracaoBase = idNovaConfiguracaoParam[0];
//                        vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
//                    }
//
//                    //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
//                    tamanhoTemporario = 1;
//
//                    //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//                    for (indice = 0; indice < numeroFalta; indice++) {
//                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam[0]);
//                        indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
//                        rnpP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].idRNP;
//                        grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
//                        //conta os consumidores sem fornecimento
//                        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
//                        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
//                        if ((indiceP + 1) <= configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos) {
//                            for (indice1 = indiceP; indice1 < configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1; indice1++) {
//                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].idNo;
//                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].profundidade;
//                                adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
//                            }
//                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1;
//                        }
//                    }
//                    //atualiza consumidores sem fornecimento
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//                    //insere no vetor Pi a nova solução
//                    atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                            idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), casoManobra,
//                            estadoInicialCA, estadoInicialCF, nosPRA, ESO);
//                    idNovaConfiguracaoParam[0]++;
//
//                }
//                for (indice = 0; indice < blocoRestabelecimento; indice++) {
//                	nos[indice].noA  = NULL; free(nos[indice].noA);
//                	nos[indice].noR  = NULL; free(nos[indice].noR);
//                	nos[indice].noP  = NULL; free(nos[indice].noP);
//                	nos[indice].rnpA = NULL; free(nos[indice].rnpA);
//                }
//                nos = NULL; free(nos);
//                potenciaRestaurada = NULL; free(potenciaRestaurada);
//                sequenciaRestauracao = NULL; free(sequenciaRestauracao);
//            }
//        } else //não existem setores a jusante para ser restabelecidos
//        {
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//            idChaveAberta = Malloc(long int, 1);
//            idChaveFechada = Malloc(long int, 1);
//            buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                    configuracoesParam, numChaveAberta, idChaveAberta,
//                    idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                    1, listaChavesParam);
//            nosPRA = Malloc(NOSPRA, 1);
//            nosPRA[0].a = -1;
//            nosPRA[0].p = setorFaltaParam;
//            nosPRA[0].r = -1;
//            estadoInicialCA = Malloc(BOOL, 1);
//            estadoInicialCF = Malloc(BOOL, 1);
//            estadoInicialCA[0] = false;
//            estadoInicialCF[0] = false;
//            alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1, &rnpOrigem);
//            //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
//            indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
//            rnpP =matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
//            grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
//            //conta os consumidores sem fornecimento
//            consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
//            consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
//            //copia os setores anteriores a falta para a nova rnp origem
//            for (indice1 = 0; indice1 < indiceP; indice1++) {
//                rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//                rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
//            }
//            //remove o setor em falta da RNP e copia os demais se existir
//            if ((indiceP + 1) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
//               for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
//                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
//                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
//                    adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
//                }
//
//            }
//
//            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//
//            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
//                if (indice1 != rnpP)
//                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
//
//            }
//            copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//            //insere no vetor Pi a nova solução
//            atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                    idChaveAberta, idChaveFechada, 1, casoManobra,
//                    estadoInicialCA, estadoInicialCF, nosPRA, ESO);
//            idNovaConfiguracaoParam[0]++;
//        }
//    } else //o setor em falta é o ultimo da rnp
//    {
//        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//        idChaveAberta = Malloc(long int, 1);
//        idChaveFechada = Malloc(long int, 1);
//        buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                configuracoesParam, numChaveAberta, idChaveAberta,
//                idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                1, listaChavesParam);
//        nosPRA = Malloc(NOSPRA, 1);
//        nosPRA[0].a = -1;
//        nosPRA[0].p = setorFaltaParam;
//        nosPRA[0].r = -1;
//        estadoInicialCA = Malloc(BOOL, 1);
//        estadoInicialCF = Malloc(BOOL, 1);
//        estadoInicialCA[0] = false;
//        estadoInicialCF[0] = false;
//        //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
//        tamanhoTemporario = 1;
//        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
//        //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
//        indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
//        rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
//        grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
//        //conta os consumidores sem fornecimento
//        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
//        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
//        //copia os setores anteriores a falta para a nova rnp origem
//        for (indice1 = 0; indice1 < indiceP; indice1++) {
//            rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//            rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
//        }
//
//        rnpOrigem.numeroNos = configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
//        //insere a nova rnpOrigem na configuração e atualiza os ponteiros das demais
//        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//        for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
//            if (indice1 != rnpP)
//                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
//        }
//        copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//        insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//        //insere no vetor Pi a nova solução
//        atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                idChaveAberta, idChaveFechada, 1, casoManobra, estadoInicialCA, estadoInicialCF, nosPRA, ESO);
//        idNovaConfiguracaoParam[0]++;
//    }
//}

/**
 * Método responsável por gerenciar a busca pelas opções de restabelecimento na presença de múltiplas faltas.
 * Utiliza o método @see isolaRestabeleceTodasOpcoesHeuristica. Realiza a chamada desse método para cada setor em falta.
 * A partir do segundo setor em falta passa como configuração inicial para localização da falta cada uma das configurações geradas para a falta anterior.
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param numeroFaltasParam
 * @param setorFaltaParam
 * @param estadoInicial
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoIniParam
 * @param listaChavesParam
 */
void isolaRestabeleceMultiplasFaltasHeuristica(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int numeroFaltasParam, long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam)
{
    long int idConfiguracaoIni = idConfiguracaoParam;
    long int idConfiguracaoFinal = idNovaConfiguracaoParam[0];
    int contadorFalta;
    long int contadorConfiguracoes;
    for (contadorFalta = 0; contadorFalta < numeroFaltasParam; contadorFalta++) {
        //verifica se o setor em falta não foi isolado por alguma outra falta.
        if (!grafoSetoresParam[setorFaltaParam[contadorFalta]].setorFalta) {
            for (contadorConfiguracoes = idConfiguracaoIni; contadorConfiguracoes < idConfiguracaoFinal; contadorConfiguracoes++) {
                //imprimeIndividuo(configuracoesParam[contadorConfiguracoes]);
                isolaRestabeleceTodasOpcoesHeuristica(grafoSetoresParam, contadorConfiguracoes, configuracoesParam, matrizPiParam,
                        vetorPiParam, setorFaltaParam[contadorFalta], estadoInicial, numChaveAberta, numChaveFechada,
                        idNovaConfiguracaoParam, listaChavesParam);
            }
            idConfiguracaoIni = idConfiguracaoFinal;
            idConfiguracaoIniParam[0] = idConfiguracaoIni - 1;
            idConfiguracaoFinal = idNovaConfiguracaoParam[0];
        }
    }
}

/**
 * Por Leandro: Trata-se da função "isolaRestabeleceMultiplasFaltasHeuristica()" modificada para seja chamada a
 * função "isolaRestabeleceTodasOpcoesHeuristicaModificada()" em vez da função "isolaRestabeleceTodasOpcoesHeuristica()".
 *
 * A descrição da função "isolaRestabeleceMultiplasFaltasHeuristica()" é
 * Método responsável por gerenciar a busca pelas opções de restabelecimento na presença de múltiplas faltas.
 * Utiliza o método @see isolaRestabeleceTodasOpcoesHeuristica. Realiza a chamada desse método para cada setor em falta.
 * A partir do segundo setor em falta passa como configuração inicial para localização da falta cada uma das configurações geradas para a falta anterior.
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param numeroFaltasParam
 * @param setorFaltaParam
 * @param estadoInicial
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoIniParam
 * @param listaChavesParam
 */
void isolaRestabeleceMultiplasFaltasHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int numeroFaltasParam,
		long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta, long int *numChaveFechada, long int *idNovaConfiguracaoParam,
		long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam)
{
    long int idConfiguracaoIni = idConfiguracaoParam;
    long int idConfiguracaoFinal = idNovaConfiguracaoParam[0];
    int contadorFalta;
    long int contadorConfiguracoes;
    for (contadorFalta = 0; contadorFalta < numeroFaltasParam; contadorFalta++) {
        //verifica se o setor em falta não foi isolado por alguma outra falta.
        if (!grafoSetoresParam[setorFaltaParam[contadorFalta]].setorFalta) {
            for (contadorConfiguracoes = idConfiguracaoIni; contadorConfiguracoes < idConfiguracaoFinal; contadorConfiguracoes++) {
            	isolaRestabeleceTodasOpcoesHeuristicaModificada(grafoSetoresParam, contadorConfiguracoes, configuracoesParam, matrizPiParam,
                        vetorPiParam, setorFaltaParam[contadorFalta], estadoInicial, numChaveAberta, numChaveFechada,
                        idNovaConfiguracaoParam, listaChavesParam, matrizBParam, grafoSDRParam);
            }
            idConfiguracaoIni = idConfiguracaoFinal;
            idConfiguracaoIniParam[0] = idConfiguracaoIni - 1;
            idConfiguracaoFinal = idNovaConfiguracaoParam[0];
        }
    }
}




 /*
 * Por Leandro
 * Descrição: consiste na função "isolaRestabeleceTodasOpcoesHeuristica()" modificada para:
 * 1) a fim de tornar possível a execução correta da função "atualizaMatrizPI()", o setor em falta e os demais setores sem possibilidade de reconexão não
 * poderão ser removidos da variável "configurações". Portanto, a principal diferença desta função desta função  para a função "isolaRestabeleceTodasOpcoesHeuristica()" é
 * que esta aqui não faz esta remoção, que por sua vez passa a ser feita pela função "atualizaMatrizPI()";
 *
 * 2) salvar retornar, por meio do parâmetro "numeroSubarvoresDesligadasParam", o número de subárvores desligadas após a isolação da falta
 *
 * Com o abandono da utilizaço de variáveis temporárias para "configurações", "matrizPi" e vetorPi", esta função deixou de ser útilizada
 *
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param setorFaltaParam
 * @param estadoInicial
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param listaChavesParam
 */
//void isolaRestabeleceTodasOpcoesHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
//        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
//        VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
//        long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam) {
//    int tamanhoTemporario;
//    long int p, a, indice, indice1, indiceP, indiceR, indiceL, r,
//            indiceSetorFalta, indiceA;
//    long int noAnteriorFalta;
//    long int indiceLFalta, idConfiguracaoBase;
//    int rnpP, rnpA;
//    int numeroFalta, numeroAJusante, posicao;
//    long int nosFalta[500]; //é o número máximo de setores em falta ou a jusantes
//    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
//    long int totalCombinacoes;
//    long int *idChaveAberta, *idChaveFechada;
//    long int consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
//    long int consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
//    int indiceChave;
//    int **indiceRestabelecimento;
//    int blocoRestabelecimento=0;
//    int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4;
//    int colunaPI;
//    RNP rnpOrigem;
//    RNP rnpDestino;
//    NOSRESTABELECIMENTO *nos;
//    int casoManobra;
//    BOOL *estadoInicialCA;
//    BOOL *estadoInicialCF;
//    NOSPRA *nosPRA;
//    //IMPRESSAO DE DADOS PARA ANALISE DOS RESULTADOS
//    char nomeArquivo[120];
//    FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
//    //todos os setores do sistema
//    sprintf(nomeArquivo, "SAIDA_testeConfiguracaoInicial.dad");
//    arquivo1 = fopen(nomeArquivo, "a");
//
//    //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
//    grafoSetoresParam[setorFaltaParam].setorFalta = true;
//    //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
//  //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
//   // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;
//
//    //RECUPERA A COLUNAPI DO SETOR EM FALTA
//    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
//    rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
//    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
//    indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
//    indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.
//
//    //LOCALIZA O SETOR ANTERIOR A FALTA
//    indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o índice do nó antecessor ao nó sob falta
//    while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
//        indice1--;
//    }
//    noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//
//    //verifica se o setor em falta não é o último da RNP
//    if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
//        numeroAJusante = 0;
//        //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
//        if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
//            indice1 = indiceP + 1;
//            nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
//            numeroAJusante++;
//            //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
//            while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
//                if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
//                    nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//                    numeroAJusante++;
//                }
//                indice1++;
//            }
//            indiceLFalta = indice1 - 1;
//            indice = 0;
//            nosFalta[0] = setorFaltaParam;
//            numeroFalta = 1;
//            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
//            totalCombinacoes = 1;
//            blocoRestabelecimento = 0;
//            //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as opções de restabelecimento
//            while (indice < numeroAJusante) {
//                a = 0;
//                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
//                indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
//                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
//                //recupera o indice final da subárvore dos setore a jusante
//                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
//                nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
//                nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
//                nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
//                nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
//                buscaTodosNosRestabelecimento(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
//                //printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
//                if (nos[blocoRestabelecimento].numeroNos > 0) {
//                    totalCombinacoes *= nos[blocoRestabelecimento].numeroNos;
//                    blocoRestabelecimento++;
//                } else {
//                    free(nos[blocoRestabelecimento].noA);
//                    free(nos[blocoRestabelecimento].noR);
//                    free(nos[blocoRestabelecimento].noP);
//                    free(nos[blocoRestabelecimento].rnpA);
//                    //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
//                    nosFalta[numeroFalta] = nosJusanteFalta[indice];
//                    numeroFalta++;
//                    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
//                    indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
//                    // printf("%d\n",indiceAux1);
//                    while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
//                        nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
//                        numeroFalta++;
//                        indiceAux1++;
//                    }
//                }
//                indice++;
//            }
//            //******************************************************************************************************//
//            //salva no arquivo quantas combinacoes de restabelecimento existem e quantos setores são agrupados a falta
//            fprintf(arquivo1, "Combinacoes;%ld;Setores;%d;", totalCombinacoes, numeroFalta);
//            if (blocoRestabelecimento == 0) { //não existe opção de restabelecimento para os setores a jusante.
//                fprintf(arquivo1, "SAR\n");
//                fclose(arquivo1);
//                //cria um novo individuo isolando a falta e os setores sem restabelecimento
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
//                //isola a falta
//                idChaveAberta = Malloc(long int, 1);
//                idChaveFechada = Malloc(long int, 1);
//                buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                        configuracoesParam, numChaveAberta, idChaveAberta,
//                        idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                        1, listaChavesParam);
//                nosPRA = Malloc(NOSPRA, 1);
//                nosPRA[0].a = -1;
//                nosPRA[0].p = setorFaltaParam;
//                nosPRA[0].r = -1;
//                estadoInicialCA = Malloc(BOOL, 1);
//                estadoInicialCF = Malloc(BOOL, 1);
//                estadoInicialCA[0] = false;
//                estadoInicialCF[0] = false;
//                //aloca a nova rnp
//                alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta, &rnpOrigem);
//                //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//                 colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[0], idConfiguracaoParam);
//                 indiceP = matrizPiParam[nosFalta[0]].colunas[colunaPI].posicao;
//                 rnpP = matrizPiParam[nosFalta[0]].colunas[colunaPI].idRNP;
//                 copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//                 //marca os setores como em falta e conta os consumidores
//                 for(indice1 = 0; indice1 < numeroFalta; indice1++)
//                 {
//                    grafoSetoresParam[nosFalta[indice1]].setorFalta = true;
//                    //conta os consumidores sem fornecimento
//                    consumidoresSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidores;
//                    consumidoresEspeciaisSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidoresEspeciais;
//                 }
//                //copia os setores anteriores a falta para a nova rnp origem
//                for (indice1 = 0; indice1 < indiceP; indice1++) {
//                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
//                }
//                //remove o setor em falta da RNP e copia os demais se existir
///*                if ((indiceP + numeroFalta) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
//                    for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta; indice1++) {
//                        rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].idNo;
//                        rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].profundidade;
//                        adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
//                    }
//
//                }*/
//
//                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//                for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
//                    if (indice1 != rnpP)
//                        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
//
//                }
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//                insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//                //insere no vetor Pi a nova solução
//                atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                        idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), 1, estadoInicialCA, estadoInicialCF, nosPRA);
//                idNovaConfiguracaoParam[0]++;
//            } else {
//                fclose(arquivo1);
//                //*************************************************************************************************//
//                indiceRestabelecimento = (int **) malloc(totalCombinacoes * sizeof (int*));
//                //cada coluna da matriz indiceRestabelecimento contém os indices do vetor de opções de restabelecimento para
//                //cada bloco de setores a jusante que poderá ser restabelecido.
//                //Cada linha contém uma combinação das opções de restabelecimento para cada bloco
//                for (indice = 0; indice < totalCombinacoes; indice++) {
//                    indiceRestabelecimento[indice] = (int *) malloc(blocoRestabelecimento * sizeof (int));
//                }
//
//                //preenche as combinacoes de indices das opçoes de restabelecimento para cada raiz de bloco de setores a jusante
//                indiceAux3 = 0;
//                for (indice = 0; indice < blocoRestabelecimento; indice++) {
//                    if (nos[indice].numeroNos > 0) {
//                        repeticoes = totalCombinacoes / nos[indice].numeroNos;
//                        indiceAux4 = 0;
//                        for (indiceAux1 = 0; indiceAux1 < repeticoes; indiceAux1++) {
//                            for (indiceAux2 = 0; indiceAux2 < nos[indice].numeroNos; indiceAux2++) {
//                                indiceRestabelecimento[indiceAux4][indiceAux3] = indiceAux2;
//                                indiceAux4++;
//                            }
//                        }
//                        indiceAux3++;
//                    }
//                }
//
//                //aplica as combinações de restabelecimento obtidas.
//                for (indiceAux1 = 0; indiceAux1 < totalCombinacoes; indiceAux1++) {
//                    idConfiguracaoBase = idConfiguracaoParam;
//                    indiceChave = 1;
//                    idChaveAberta = Malloc(long int, (blocoRestabelecimento + 1));
//                    idChaveFechada = Malloc(long int, (blocoRestabelecimento + 1));
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
//                    buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                            configuracoesParam, numChaveAberta, idChaveAberta,
//                            idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                            (blocoRestabelecimento + 1), listaChavesParam);
//                    casoManobra = 1;
//                    estadoInicialCA = Malloc(BOOL, (blocoRestabelecimento + 1));
//                    estadoInicialCF = Malloc(BOOL, (blocoRestabelecimento + 1));
//
//                    nosPRA = Malloc(NOSPRA, (blocoRestabelecimento + 1));
//                    estadoInicialCA[0] = false;
//                    estadoInicialCF[0] = false;
//                    nosPRA[0].a = -1;
//                    nosPRA[0].p = setorFaltaParam;
//                    nosPRA[0].r = -1;
//                    consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
//                    consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
//                    copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//                    insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//                    for (indiceAux2 = 0; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
//                        //printf("%d \t", indiceRestabelecimento[indiceAux1][indiceAux2]);
//                        indice = indiceAux2;
//                        posicao = indiceRestabelecimento[indiceAux1][indiceAux2];
//                        rnpA = nos[indice].rnpA[posicao];
//                        a = nos[indice].noA[posicao];
//                        r = nos[indice].noR[posicao];
//                        p = nos[indice].noP[posicao];
//                        if (rnpA != -1) {
//                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idConfiguracaoBase);
//                            indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
//                            rnpA = matrizPiParam[a].colunas[colunaPI].idRNP;
//                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idConfiguracaoBase);
//                            indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
//                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, p, idConfiguracaoBase);
//                            indiceP = matrizPiParam[p].colunas[colunaPI].posicao;
//                            indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);
//                            p = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceP].idNo;
//                            r = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceR].idNo;
//                            a = configuracoesParam[idConfiguracaoBase].rnp[rnpA].nos[indiceA].idNo;
//                            nosPRA[indiceAux2+1].a = a;
//                            nosPRA[indiceAux2+1].p = p;
//                            nosPRA[indiceAux2+1].r = r;
//                            //localiza a chave a ser aberta
//                            indice1 = 0;
//                            while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != p) {
//                                indice1++;
//                            }
//                            idChaveAberta[indiceAux2 + 1] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
//                            //localiza a chave a ser fechada
//                            indice1 = 0;
//                            while (grafoSetoresParam[r].setoresAdjacentes[indice1] != a) {
//                                indice1++;
//                            }
//                            idChaveFechada[indiceAux2 + 1] = grafoSetoresParam[r].idChavesAdjacentes[indice1];
//                            //ROTINA PARA CALCULAR OS PARES DE MANOBRAS
//                            numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[indiceAux2 + 1], idChaveFechada[indiceAux2 + 1], listaChavesParam,
//                                    idNovaConfiguracaoParam[0], estadoInicial, &estadoInicialCA[indiceAux2+1], &estadoInicialCF[indiceAux2+1], idNovaConfiguracaoParam[0], &casoManobra);
//
//                            tamanhoTemporario = indiceL - indiceP + 1;
//                            //realiza a alocação das RNPs que serão alteradas
//                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
//                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
//                            //obtém a nova rnp de destino
//                            constroiRNPDestino(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam[0]);
//                            constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam[0]);
//                            //libera a memória utilizada pela rnp do setor em falta
//
//                            //###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############
//                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpA] = rnpDestino;
//                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//                            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpA);
//                            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNP; indice1++) {
//                                if (indice1 != rnpA && indice1 != rnpP)
//                                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoBase].rnp[indice1];
//                            }
//
//                        }
//                        idConfiguracaoBase = idNovaConfiguracaoParam[0];
//                        vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
//                    }
//
//                    //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
//                    tamanhoTemporario = 1;
//
//                    //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//                    for (indice = 0; indice < numeroFalta; indice++) {
//                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam[0]);
//                        indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
//                        rnpP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].idRNP;
//                        grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
//                        //conta os consumidores sem fornecimento
//                        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
//                        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
//                        if ((indiceP + 1) <= configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos) {
//                            for (indice1 = indiceP; indice1 < configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1; indice1++) {
//                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].idNo;
//                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].profundidade;
//                                adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
//                            }
//                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1;
//                        }
//                    }
//                    //atualiza consumidores sem fornecimento
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//                    //insere no vetor Pi a nova solução
//                    atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                            idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), casoManobra,
//                            estadoInicialCA, estadoInicialCF, nosPRA);
//                    idNovaConfiguracaoParam[0]++;
//
//                }
//                for (indice = 0; indice < blocoRestabelecimento; indice++) {
//                    free(nos[indice].noA);
//                    free(nos[indice].noR);
//                    free(nos[indice].noP);
//                    free(nos[indice].rnpA);
//                }
//                free(nos);
//            }
//        } else //não existem setores a jusante para ser restabelecidos
//        {
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//            idChaveAberta = Malloc(long int, 1);
//            idChaveFechada = Malloc(long int, 1);
//            buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                    configuracoesParam, numChaveAberta, idChaveAberta,
//                    idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                    1, listaChavesParam);
//            nosPRA = Malloc(NOSPRA, 1);
//            nosPRA[0].a = -1;
//            nosPRA[0].p = setorFaltaParam;
//            nosPRA[0].r = -1;
//            estadoInicialCA = Malloc(BOOL, 1);
//            estadoInicialCF = Malloc(BOOL, 1);
//            estadoInicialCA[0] = false;
//            estadoInicialCF[0] = false;
//            alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1, &rnpOrigem);
//            //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
//            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
//            indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
//            rnpP =matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
//            grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
//            //conta os consumidores sem fornecimento
//            consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
//            consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
//            //copia os setores anteriores a falta para a nova rnp origem
//            for (indice1 = 0; indice1 < indiceP; indice1++) {
//                rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//                rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
//            }
//            //remove o setor em falta da RNP e copia os demais se existir
///*            if ((indiceP + 1) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
//               for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
//                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
//                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
//                    adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
//                }
//
//            }*/
//
//            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//
//            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
//                if (indice1 != rnpP)
//                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
//
//            }
//            copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//            insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//            //insere no vetor Pi a nova solução
//            atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                    idChaveAberta, idChaveFechada, 1, casoManobra,
//                    estadoInicialCA, estadoInicialCF, nosPRA);
//            idNovaConfiguracaoParam[0]++;
//        }
//    } else //o setor em falta é o ultimo da rnp
//    {
//        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
//                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
//        idChaveAberta = Malloc(long int, 1);
//        idChaveFechada = Malloc(long int, 1);
//        buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
//                configuracoesParam, numChaveAberta, idChaveAberta,
//                idChaveFechada, setorFaltaParam, noAnteriorFalta,
//                1, listaChavesParam);
//        nosPRA = Malloc(NOSPRA, 1);
//        nosPRA[0].a = -1;
//        nosPRA[0].p = setorFaltaParam;
//        nosPRA[0].r = -1;
//        estadoInicialCA = Malloc(BOOL, 1);
//        estadoInicialCF = Malloc(BOOL, 1);
//        estadoInicialCA[0] = false;
//        estadoInicialCF[0] = false;
//        //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
//        tamanhoTemporario = 1;
//        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
//        //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
///*        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
//        indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
//        rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
//        grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;*/
//        //conta os consumidores sem fornecimento
//        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
//        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
//        //copia os setores anteriores a falta para a nova rnp origem
//        for (indice1 = 0; indice1 < indiceP; indice1++) {
//            rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
//            rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
//        }
//
//        rnpOrigem.numeroNos = configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
//        //insere a nova rnpOrigem na configuração e atualiza os ponteiros das demais
//        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
//        for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
//            if (indice1 != rnpP)
//                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
//        }
//        copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
//        insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
//        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
//        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
//        //insere no vetor Pi a nova solução
//        atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
//                idChaveAberta, idChaveFechada, 1, casoManobra, estadoInicialCA, estadoInicialCF, nosPRA);
//        idNovaConfiguracaoParam[0]++;
//    }
//}

/**
 * Por Leandro:
 * Consiste na função @see isolaRestabeleceMultiplasFaltasHeuristica modificada para seja chamda a função isolaRestabeleceTodasOpcoesHeuristicaModificada em vez da
 * função isolaRestabeleceTodasOpcoesHeuristica
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param numeroFaltasParam
 * @param setorFaltaParam
 * @param estadoInicial
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoIniParam
 * @param listaChavesParam
 */
/*void isolaRestabeleceMultiplasFaltasHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int numeroFaltasParam, long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam)
{
    long int idConfiguracaoIni = idConfiguracaoParam;
    long int idConfiguracaoFinal = idNovaConfiguracaoParam[0];
    int contadorFalta;
    long int contadorConfiguracoes;
    for (contadorFalta = 0; contadorFalta < numeroFaltasParam; contadorFalta++) {
        //verifica se o setor em falta não foi isolado por alguma outra falta.
        if (!grafoSetoresParam[setorFaltaParam[contadorFalta]].setorFalta) {
            for (contadorConfiguracoes = idConfiguracaoIni; contadorConfiguracoes < idConfiguracaoFinal; contadorConfiguracoes++) {
                //imprimeIndividuo(configuracoesParam[contadorConfiguracoes]);
                isolaRestabeleceTodasOpcoesHeuristicaModificada(grafoSetoresParam, contadorConfiguracoes, configuracoesParam, matrizPiParam,
                        vetorPiParam, setorFaltaParam[contadorFalta], estadoInicial, numChaveAberta, numChaveFechada,
                        idNovaConfiguracaoParam, listaChavesParam);
            }
            idConfiguracaoIni = idConfiguracaoFinal;
            idConfiguracaoIniParam[0] = idConfiguracaoIni - 1;
            idConfiguracaoFinal = idNovaConfiguracaoParam[0];
        }
       // printf("idConfiguracaoIni %ld idConfiguracaoIniParam %ld idConfiguracaoFinal %ld\n", idConfiguracaoIni,idConfiguracaoIniParam[0], idConfiguracaoFinal);
    }


}*/


/*
* Por Leandro
* Descrição: determina o número de subárvores à jusante do setor de falta CAPAZES DE SEREM
* RESTABELECIDAS por meio de chaves normalmente abertas E que possuam consumidores.
* Dada uma certa subárvore, mesmo que seja possível reconectá-la por meio de chaves NAs, a mesma não será
* contabilida caso não possua, em seus setores, consumidores. Isto visa evitar manobras de reconexão que não beneficiariam a nenhum consumidor
* Portanto, não são contabilizadas:
* a) subárvores que não podem ser reconectadas, devido a ausência de chaves NAs, e;
* b) subárvores sem consumidores;
*
* @param grafoSetoresParam
* @param idConfiguracaoParam
* @param configuracoesParam
* @param matrizPiParam
* @param vetorPiParam
* @param setorFaltaParam
* @param estadoInicial
* @param numChaveAberta
* @param numChaveFechada
* @param idNovaConfiguracaoParam
* @param listaChavesParam
*/
long int determinaNumeroSubarvoresDesligadasReconectaveis(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
       CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam, long int setorFaltaParam) {

   long int a, p, indice, indice1, indiceP, indiceL, indiceSetorFalta, indiceA;
   long int noAnteriorFalta;
   long int indiceLFalta;
   int rnpP;
   int numeroFalta, numeroAJusante, indiceAux1;
   long int *nosFalta = Malloc(long int, 1000);//nosFalta[500]; //é o número máximo de setores em falta ou a jusantes
   long int *nosJusanteFalta = Malloc(long int, 500); //nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
   int blocoRestabelecimento=0;
   int colunaPI;
   NOSRESTABELECIMENTO *nos;
   NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida;
   double potenciaNaoSupridaTotal;
   long int numeroSubarvoresDesligadas = 0;

   /*//marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
   grafoSetoresParam[setorFaltaParam].setorFalta = true;
   //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
 //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
  // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;
*/
   //RECUPERA A COLUNAPI DO SETOR EM FALTA
   colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
   rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
   //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
   indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
   indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

   //LOCALIZA O SETOR ANTERIOR A FALTA
   if(indiceSetorFalta == 0){
	   printf("\nO setor em falta informado é um setor raiz, ou seja, de uma barra em uma subestação.\n");
	   exit(0);
   }
   indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o índice do nó antecessor ao nó sob falta
   while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
       indice1--;
   }
   noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

   //verifica se o setor em falta não é o último da RNP
   if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
       numeroAJusante = 0;
       //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
       if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
           indice1 = indiceP + 1;
           nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
           numeroAJusante++;
           //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
           while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
               if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                   nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                   numeroAJusante++;
               }
               indice1++;
           }
           indiceLFalta = indice1 - 1;
           indice = 0;
           nosFalta[0] = setorFaltaParam;
           numeroFalta = 1;
           nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
           blocoRestabelecimento = 0;
           //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as opções de restabelecimento
           while (indice < numeroAJusante) {
               a = 0;
               colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
               indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
               p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
               //recupera o indice final da subárvore dos setore a jusante
               indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
               nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
               buscaTodosNosRestabelecimentoCorrigida(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
               potenciaNaoSuprida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(configuracoesParam, idConfiguracaoParam, p, rnpSetoresParam, matrizPiParam, vetorPiParam, grafoSDRParam);
               potenciaNaoSupridaTotal = potenciaNaoSuprida.consumidoresSemPrioridade + potenciaNaoSuprida.consumidoresPrioridadeBaixa + potenciaNaoSuprida.consumidoresPrioridadeIntermediaria + potenciaNaoSuprida.consumidoresPrioridadeAlta;

               //printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
               if (nos[blocoRestabelecimento].numeroNos > 0 && potenciaNaoSupridaTotal > 0) {
                   blocoRestabelecimento++;
                   numeroSubarvoresDesligadas++; //Considera apenas as subárvores capazes de serem restauradas. As demais são desconsideradas
               }
               else{
                   //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
                   nosFalta[numeroFalta] = nosJusanteFalta[indice];
                   numeroFalta++;
                   colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                   indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
                   // printf("%d\n",indiceAux1);
                   while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
                       nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
                       numeroFalta++;
                       indiceAux1++;
                   }
               }
               indice++;
           }
           if(numeroAJusante > 0){
        	   for(indice = 0; indice < blocoRestabelecimento; indice++){
				  free(nos[indice].noA);
				  free(nos[indice].noR);
				  free(nos[indice].noP);
				  free(nos[indice].rnpA);
				}
				free(nos);
           }
       }
       else //não existem setores a jusante para ser restabelecidos
    	   numeroSubarvoresDesligadas = 0;
   }
   else //o setor em falta é o ultimo da rnp. Então não há subávores saudáveis desligadas
	   numeroSubarvoresDesligadas = 0;

   free(nosFalta);
   free(nosJusanteFalta);

   return numeroSubarvoresDesligadas;
}



/*
* Por Leandro
* Descrição: determina o número de subárvores à jusante do setor de falta CAPAZES DE SEREM
* RESTABELECIDAS por meio de chaves normalmente abertas, a fim de saber o número de RNPs Fictícias
* a serem alocadas.
* Note que as subárvores que não podem ser reconectadas, devido a ausência de chaves, não são
* contabilizadas
*
* @param
*/
long int obtemNumeroSubarvoresDesligadas(GRAFOSETORES *grafoSetoresParam, int numeroSetorFaltaParam, long int *setoresFaltaParam, int numeroPosicoesAlocadasParam, CONFIGURACAO *configuracaoInicial, long int idConfiguracaoParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam) {

   CONFIGURACAO *configuracoesTemp;
   MATRIZPI *matrizPiTemp;
   VETORPI *vetorPiTemp;
   long int idConfiguracao, numeroRNPFicticia, numeroRNPFicticiaPorSetorFalta = 0, numeroConfiguracoesTemp;
   int contadorRnp, contadorNos, indice, contador;

   inicializaVetorPi(numeroPosicoesAlocadasParam, &vetorPiTemp);        //Por Leandro
   inicializaMatrizPI(grafoSetoresParam, &matrizPiTemp, numeroPosicoesAlocadasParam, numeroSetores); //Por Leandro
   idConfiguracao = 0;
   numeroConfiguracoesTemp = 1;
   configuracoesTemp = alocaIndividuo(configuracaoInicial[0].numeroRNP, idConfiguracao, numeroConfiguracoesTemp, 1); //Por Leandro: mudou a função que faz a alocação

   //Por Leandro: Aloca somente a variável temporária "configuracoesTemp". A variável "configuracoes" será alocada somente se a BE não retornar uma solução factível

   for (contadorRnp = 0; contadorRnp < configuracaoInicial[0].numeroRNP; contadorRnp++) {
       alocaRNP(configuracaoInicial[0].rnp[contadorRnp].numeroNos, &configuracoesTemp[idConfiguracao].rnp[contadorRnp]); //Por Leandro
       for (contadorNos = 0; contadorNos < configuracaoInicial[0].rnp[contadorRnp].numeroNos; contadorNos++) {
           configuracoesTemp[idConfiguracao].rnp[contadorRnp].nos[contadorNos] = configuracaoInicial[0].rnp[contadorRnp].nos[contadorNos];              //Por Leandro
           adicionaColuna(matrizPiTemp, configuracoesTemp[idConfiguracao].rnp[contadorRnp].nos[contadorNos].idNo, idConfiguracao, contadorRnp, contadorNos);//Por Leandro
       }
   }

   numeroRNPFicticia = 0;
   for(indice = 0; indice < numeroSetorFaltaParam; indice++){
	   numeroRNPFicticiaPorSetorFalta = determinaNumeroSubarvoresDesligadasReconectaveis(grafoSetoresParam, idConfiguracaoParam, configuracoesTemp, matrizPiTemp, vetorPiTemp, rnpSetoresParam, grafoSDRParam, setoresFaltaParam[indice]); //Por Leandro
       numeroRNPFicticia = numeroRNPFicticia + numeroRNPFicticiaPorSetorFalta;
   }

   //Desaloca as variáveis temporárias
   for (contador = 0; contador < numeroConfiguracoesTemp; contador++) {
	   configuracoesTemp[contador].numeroRNPFicticia = 0;
   }
   desalocacaoCompletaConfiguracao(configuracoesTemp, numeroConfiguracoesTemp);
   desalocaMatrizPI(matrizPiTemp, numeroSetores);
   free(matrizPiTemp);
   desalocaVetorPi(numeroPosicoesAlocadasParam, vetorPiTemp);
   free(vetorPiTemp);

   return numeroRNPFicticia;
}

/* Por Leandro:
 * Descrição: localiza quais chaves serão aberta e fechada pela aplicação do PAO
 */
void localizaChavesOperadorPAO(long int *idChaveAbertaParam, long int *idChaveFechadaParam, int indicePParam, long int noPParam, long int noAParam, long int rnpPParam, long int rnpAParam, CONFIGURACAO *popConfiguracaoParam, GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoSelecionadaParam){
	long int indice, noRaizP;

	indice = indicePParam-1;
	while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePParam].profundidade)
	{
		indice--;
	}
	noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].idNo;
	idChaveAbertaParam[0] = idChaveFechadaParam[0] = -999;
	for(indice = 0; indice < grafoSetoresParam[noPParam].numeroAdjacentes && (idChaveAbertaParam[0] == -999 || idChaveFechadaParam[0] == -999); indice++)
	{
		if(grafoSetoresParam[noPParam].setoresAdjacentes[indice] == noRaizP)
			idChaveAbertaParam[0] = grafoSetoresParam[noPParam].idChavesAdjacentes[indice];
		else
			if(grafoSetoresParam[noPParam].setoresAdjacentes[indice] == noAParam)
				idChaveFechadaParam[0] = grafoSetoresParam[noPParam].idChavesAdjacentes[indice];
	}
}

/* Por Leandro:
 * Descrição: localiza quais chaves serão aberta e fechada pela aplicação do CAO
 */
void localizaChavesOperadorCAO(long int *idChaveAbertaParam, long int *idChaveFechadaParam, int indicePParam, long int noPParam, long int noRParam, long int noAParam, long int rnpPParam, long int rnpAParam, CONFIGURACAO *popConfiguracaoParam, GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoSelecionadaParam){
	long int indice, noRaizP;

	indice = indicePParam-1;
	while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePParam].profundidade)
	{
		indice--;
	}
	noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].idNo;
	idChaveAbertaParam[0] = idChaveFechadaParam[0] = -999;
	for(indice = 0; indice< grafoSetoresParam[noPParam].numeroAdjacentes && idChaveAbertaParam[0] == -999; indice++)
	{
		if(grafoSetoresParam[noPParam].setoresAdjacentes[indice] == noRaizP)
			idChaveAbertaParam[0] = grafoSetoresParam[noPParam].idChavesAdjacentes[indice];
	}

	for(indice = 0; indice< grafoSetoresParam[noRParam].numeroAdjacentes && idChaveFechadaParam[0] == -999; indice++)
	{
			if(grafoSetoresParam[noRParam].setoresAdjacentes[indice] == noAParam)
				idChaveFechadaParam[0] = grafoSetoresParam[noRParam].idChavesAdjacentes[indice];
	}
}

/* Por Leandro:
 * Descrição: localiza quais chaves serão aberta e fechada pela aplicação do LRO
 */
void localizaChavesOperadorLRO(long int *idChaveAbertaParam, long int *idChaveFechadaParam, int indicePParam, long int noPParam, long int noRParam, long int noAParam, long int rnpPParam, long int rnpAParam, CONFIGURACAO *popConfiguracaoParam, GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoSelecionadaParam){
	long int indice, noRaizP;

	if(indicePParam > 1){
		indice = indicePParam-1;
		while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indicePParam].profundidade)
		{
			indice--;
		}
		noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indice].idNo;
		idChaveAbertaParam[0] = -999;
		for(indice = 0; indice< grafoSetoresParam[noPParam].numeroAdjacentes; indice++)
		{
			if(grafoSetoresParam[noPParam].setoresAdjacentes[indice] == noRaizP)
				idChaveAbertaParam[0] = grafoSetoresParam[noPParam].idChavesAdjacentes[indice];
		}
	}
	else
		idChaveAbertaParam[0] = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

	//localiza qual chave será fechada
	idChaveFechadaParam[0] = -999;
	for(indice = 0; indice< grafoSetoresParam[noRParam].numeroAdjacentes; indice++)
	{
			if(grafoSetoresParam[noRParam].setoresAdjacentes[indice] == noAParam)
				idChaveFechadaParam[0] = grafoSetoresParam[noRParam].idChavesAdjacentes[indice];
	}
}

/**
 * Por Leandro:
 * esta função consiste na função "operadorPAOHeuristica()" modificada para:
 * 1) tratar o caso em que não tenha sido possível determinar um nó adjacente válido. Isto evitará que o código entre num loop infinito no "while(noA == 0".
 * Este caso ocorre quando, para todos os setores, os setores adjacentes estão na mesma árvore, já que existe uma restrição de que as árvores origem e destino
 * devem ser distintas
 *
 * 2) determinar o somotório de potência ativa transferido de uma árvore para outra durante a obtenção de "idNovaConfiguracaoParam" a partir de "idConfiguracaoSelecionadaParam"
 *
 * 3) para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRASALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 */
int operadorPAOHeuristicaMelhorada(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP;
    int indiceL, indiceP, indiceA, indice, contador;
    long int noRaizP;
    int rnpP, rnpA, indiceRnpP;
    long int idChaveAberta, idChaveFechada;
    int profundidadeA;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaTransferida;
    double potenciaTransferidaTotal;

    consumidoresManobrados = 0;
    //Determinação dos nós p,e a, para aplicação do operador
    //Executa até obter um nó a
    contador = 0;
    while (noA == 0 && contador < 200) { //Por Leandro: visa tratar o caso em que não existe um noA válido e portanto, evitar que o programa fique neste loop infinitamente

    	//o no P sorteado nao pode ser a raiz da árvore
       // printf("idConfiguracao %ld, numeroRNPs %d \n",idConfiguracaoSelecionadaParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
        indiceRnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta - 1));
        rnpP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indiceRnpP];
        //verifica se a árvore possui mais de um nó para poder aplicar o operador
        if (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos > 1) {
            indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos - 1));
            noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
            //determinação do no A adjacente a R
            //Por Leandro: a função "determinaNoAdjacente()" foi modificada para "determinaNoAdjacenteModificada()" a fim de evitar a escolha de noA pertencente a RNP Fictícia
            colunaNoA = determinaNoAdjacenteModificada(maximoTentativas, noP, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP, grafoSetoresParam, matrizPiParam, vetorPiParam);

    		//Por Leandro: Determina a quantidade de potência ativa transferida total
    		potenciaTransferida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(popConfiguracaoParam, idConfiguracaoSelecionadaParam, indiceP, indiceL, rnpP, matrizB, grafoSDRParam);
    		potenciaTransferidaTotal = potenciaTransferida.consumidoresSemPrioridade + potenciaTransferida.consumidoresPrioridadeBaixa + potenciaTransferida.consumidoresPrioridadeIntermediaria + potenciaTransferida.consumidoresPrioridadeAlta;

    		if (colunaNoA != NULL && potenciaTransferidaTotal > 0) { //se obteve um nó a válido recupera os dados dele.
                rnpA = colunaNoA[0].idRNP;
                indiceA = colunaNoA[0].posicao;
                noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;
                profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].profundidade;


                idChaveAberta = idChaveFechada = -999;
                localizaChavesOperadorPAO(&idChaveAberta, &idChaveFechada, indiceP, noP, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
        		if(idChaveAberta == -999 || idChaveFechada == -999)
        			noA = 0;

            }
        }
        contador++;
    }

    /*Por Leandro:
     * Esta verificação foi inserida a fim de verificar se um noA válido foi anteriormente determinado.
     * Caso, mesmo após um certo número de tentativas isto não tenha ocorrido, então a função retorna -1 para avisar ao processo evolutivo que ele deve determinar
     * um outro operador.
     * Senão, a função continua normalmente (como codificado pela Telma) a sua execução e retornará 1*/
    if(noA == 0){
    	return -1;
    }
    else{

	   //calcula o tamanho da subárvore podada
		tamanhoSubarvore = indiceL-indiceP+1;
		//localiza quais chaves serão aberta e fechada
		indice = indiceP-1;
		while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
		{
			indice--;
		}
		noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
		idChaveAberta = idChaveFechada = -999;
		for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
		{
			if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
				idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
			else
				if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noA)
					idChaveFechada = grafoSetoresParam[noP].idChavesAdjacentes[indice];
		}
		if(idChaveAberta == -999 || idChaveFechada == -999){
			printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta ou da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador PAO, Indivíduo %ld)", idNovaConfiguracaoParam);
			exit(1);
		}


		//realiza a alocação das que serão alteradas RNPs
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
		//constroi a rnp de origem nova
		constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
		//constroi a rnp de destino nova
	   // constroiRNPDestinoPAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);
		constroiRNPDestino(popConfiguracaoParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceP, matrizPiParam,
				idConfiguracaoSelecionadaParam, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], idNovaConfiguracaoParam);

		//calcula o número de manobras
		numeroManobrasModificada(popConfiguracaoParam,
			idChaveAberta, idChaveFechada, listaChavesParam, idConfiguracaoSelecionadaParam,
			estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
		//salva os setores utilizados na manobra, nesse caso os setore P e R são os mesmos.
		nosPRA[0].a = noA;
		nosPRA[0].p = noP;
		nosPRA[0].r = noP;
		//insere no vetor pi o ancestral do novo individuo obtido
		atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
				&idChaveAberta, &idChaveFechada, 1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, PAO);

		//copia o ponteiro para as rnps que não foram alteradas
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
		{
			if (indice != rnpP && indice != rnpA)
			{
				popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}
		}

		/*- por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada -*/
		popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
			popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
		}
		/*-----*/

		copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
		insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);


		//Por Leandro: Determina a quantidade de potência ativa transferida, por nível de prioridade de atendimento
		popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(popConfiguracaoParam, idConfiguracaoSelecionadaParam, indiceP, indiceL, rnpP, matrizB, grafoSDRParam);

		/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, em particular, que seja calculada a potência ativa não suprida*/
		avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, rnpP, idNovaConfiguracaoParam,
				dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
				indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
				ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam, sequenciaManobrasAlivioParam,
				listaChavesParam, vetorPiParam, true);


		//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
		for(contador = 0; contador < tamanhoSubarvore; contador++)
		{
			if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA)
			{
				consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
			else
			{
				consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
		}
		popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;
		return 1;
    }
}

/**
 * Por Leandro:
 * esta função consiste na mesma função "operadorCAOHeuristica()" modificada para:
 * 1) tratar o caso em que não tenha sido possível determinar um nó adjacente válido. Isto evitará que o código entre num loop infiniti no "while(noA == 0".
 * Este caso ocorre quando, para todos os setores, os setores adjacentes estão na mesma árvore, já que existe uma restrição de que as árvores origem e destino
 * devem ser distintas
 * 2)para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRASALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 *
 *
 * Tem como parâmetros o vetor de configurações, a geração que está sendo executada, o identificador da configuração que irá
 * dar origem a nova solução, o grafo de setores, o vetor de matriz pi e a vetor pi.
 * @param popConfiguracaoParam
 * @param geracaoParam
 * @param idFlorestaSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 */
int operadorCAOHeuristicaMelhorada(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam,
        long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noRaizP;
    int indiceL, indiceP, indiceR, indiceA, indice, contador;
    long int idChaveAberta, idChaveFechada;
    int rnpP, rnpA, indiceRnpP;
    int profundidadeA;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaTransferida;
    double potenciaTransferidaTotal;

    consumidoresManobrados = 0;

    //Determinação dos nós p,r e a, para aplicação do operador
    //Executa até obter um nó a
    contador = 0;
    while (noA == 0 && contador < 200) { //Por Leandro: visa tratar o caso em que não existe um noA válido e portanto, evitar que o programa fique neste loop infinitamente
        //o no P sorteado nao pode ser a raiz da árvore
        indiceRnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta - 1));
        rnpP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indiceRnpP];
        //verifica se a árvore possui mais de um nó para poder aplicar o operador
        if (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos > 1) {
            indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos - 1));
            noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
            //determinação do no R
            if (indiceL == indiceP) //nesse caso não existe outro nó para religar a subárvore
                indiceR = indiceP;
            else
                indiceR = inteiroAleatorio(indiceP, indiceL);
           // printf("indiceR %d indiceP %d indiceL %d \n", indiceR, indiceP, indiceL);
            noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceR].idNo;

            //determinação do no A adjacente a R
            //Por Leandro: a função "determinaNoAdjacente()" foi modificada para "determinaNoAdjacenteModificada()" a fim de evitar a escolha de noA pertencente a RNP Fictícia
            colunaNoA = determinaNoAdjacenteModificada(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP, grafoSetoresParam, matrizPiParam, vetorPiParam);

    		//Por Leandro: Determina a quantidade de potência ativa transferida total
    		potenciaTransferida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(popConfiguracaoParam, idConfiguracaoSelecionadaParam, indiceP, indiceL, rnpP, matrizB, grafoSDRParam);
    		potenciaTransferidaTotal = potenciaTransferida.consumidoresSemPrioridade + potenciaTransferida.consumidoresPrioridadeBaixa + potenciaTransferida.consumidoresPrioridadeIntermediaria + potenciaTransferida.consumidoresPrioridadeAlta;

            if (colunaNoA != NULL && potenciaTransferidaTotal > 0) {
                //recupera os dados do nó a selecionado
                rnpA = colunaNoA[0].idRNP;
                indiceA = colunaNoA[0].posicao;
                noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;
                profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].profundidade;


                idChaveAberta = idChaveFechada = -999;
                localizaChavesOperadorCAO(&idChaveAberta, &idChaveFechada, indiceP, noP, noR, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
        		if(idChaveAberta == -999 || idChaveFechada == -999)
        			noA = 0;

            }
        }
        contador++;
    }

    /*Por Leandro:
     * Esta verificação foi inserida a fim de verificar se um noA válido foi anteriormente determinad.
     * Caso, mesmo após um certo número de tentativas isto não tenha ocorrido, então a função retorna -1 para avisar ao processo evolutivo que ele deve determinar
     * um outro operador.
     * Senão, a função continua normalmente (como codificado pela Telma) a sua execução e retornará 1*/
    if(noA == 0){
    	return -1;
    }
	else{

	   //localiza quais chaves serão aberta e fechada
		indice = indiceP-1;
		while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
		{
			indice--;
		}
		noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
		idChaveAberta = idChaveFechada = -999;
		for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
		{
			if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
				idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
			 }

		for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
		{
				if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
					idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
		}
		if(idChaveAberta == -999 || idChaveFechada == -999){
			printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta ou da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador CAO, Indivíduo %ld)", idNovaConfiguracaoParam);
			exit(1);
		}

		//printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
		//calcula o tamanho da subárvore
		tamanhoSubarvore = indiceL-indiceP+1;
		//faz a alocação das novas RNPs
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
		//Obtém a nova rnp de origem, sem a subárvore podada
		constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
		//Obtém a nova rnp de destino, inserindo a subárvore podada
		constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

	   //copia os ponteiros das rnps não alteradas
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
		{
			if (indice != rnpP && indice != rnpA)
			{
				popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}
		}
		/*- Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada -*/
		popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
			popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
		}
		/*-----*/

		copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
		insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);
		//imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
		numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
		  idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
		//salva os setores utilizados na manobra
		nosPRA[0].a = noA;
		nosPRA[0].p = noP;
		nosPRA[0].r = noR;
		atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
				&idChaveAberta, &idChaveFechada,1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, CAO);

		//Determina a quantidade de potência ativa transferida, por nível de prioridade de atendimento
		popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(popConfiguracaoParam, idConfiguracaoSelecionadaParam, indiceP, indiceL, rnpP, matrizB, grafoSDRParam);

		/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, em particupar, que seja calculada a potência e a energia ativa não suprida*/
		avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, rnpP, idNovaConfiguracaoParam,
				dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
				indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
				ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
				sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, true);

		//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
		for(contador = 0; contador < tamanhoSubarvore; contador++)
		{
			//printf("\nQtdAntes: %ld - idAnt: %ld - idNova: %ld - idNo: %ld - Config(0): %d - Para: %d - a: %ld - p: %ld - Valor: %ld - ", popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoSelecionadaParam, idNovaConfiguracaoParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo, matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP, rnpA, noA, noP, grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais);
			if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA)
			{
				//printf("igual - ");
				consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
				//popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
			else
			{
				//printf("diferente - ");
				consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
				//popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
		}
		popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;
		//printf("\nQtdDepois: %ld - Valor: %ld", popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos, consumidoresManobrados);

		return 1;
	}
}

/**
 * Por Leandro
 * DESCRIÇÃO: função que gera um novo indivíduo a partir de um indivíduo selecionado por meio da realização
 * de cortes de cargas. O corte é realizado no alimentador no qual uma restrição foi violado. A escolha das cargas
 * a serem cortadas depende de qual restrição fora violada.
 * De maneira sucinta, este operador consiste no operador PAO da RNP com as seguintes diferenças:
 * 1) o nó de poda é escolhido de maneira determinística
 * 2) a árvore destino será uma das duas árvores fictícias que armazenam setores saudáveis e reconectáveis cortados.
 * 3) é do tipo 'int' e retorna -1, caso não tenha sido possivel determinar uma subárbvore a ser cortada sem a presença de CE. E retorna 1 se
 * o operador tiver sido aplicado.
 * 4)para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRASALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 */
int operadorLSOHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
	int profundidadeA;
    long int noA, noPTemp, noP;
    int indiceL, indiceA, indice, contador, contador2, indicePTemp, indiceP;
    long int noRaizP;
    int rnpP, rnpA;
    long int idChaveAberta, idChaveFechada;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados = 0;

    //Por Leandro
    double maiorCarregamentoRede = 0, menorTensao;
    int indiceRNP, indiceRNP2, indice2, indiceNoInfactivel, indicePAnterior; //Leandro
    long int noInfactivel, idTrafoMaiorCarregamento, noPAnterior; //Leandro
    int numeroSetorJusanteFalta; // Leandro: Salva a quantidade de setores, da subárvore cortada, que forma inicialmente afetados pela falta
    double potenciaAtivaNaoSuprida = 0, sobrecarga, potenciaEmCorte, maiorPotenciaEmCorte;
    int prof, profAnt, flag;
    enum{
    	carregamentoRede, /**< Indica que a restrição violada a ser tratada será carregamento Rede. */
    	carregamentoTrafo, /**< Indica que a restrição violada a ser tratada será carregamento de subestação. */
    	quedaTensao  /**< Indica que a restrição violada a ser tratada será de queda de tensao. */
    } restricaoViolada;
    struct RNPFALTATRAFO {  //Estrutura que armazenará os indices das RNP conectadas ao trafo com maior carregamento que estão na região do Problema
    	int *idRnpFalta;
    	int numeroRnp;
    } rnpFaltaTrafoMaiorCarregamento;
    struct NOS{ 	//Variável auxiliar temporária que armazenará os nós-folha do alimentador que será selecionado para corte
    	int *idNo;
    	int numeroNos;
    } nos;
    int SBase;
    leituraSBase(&SBase);

    //Determina qual das restrições será tratada pelo LSOe. Dá-se preferência à eliminação de violação de carregamento de rede, posteriormente de queda de tensão e por fim de carregamento de rede
    if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.maiorCarregamentoRede > maxCarregamentoRede)
    	restricaoViolada = carregamentoRede;
    else{
    	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.quedaMaxima > maxQuedaTensao)
    		restricaoViolada = quedaTensao;
    	else{
    		if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.maiorCarregamentoTrafo > maxCarregamentoTrafo)
    			restricaoViolada = carregamentoTrafo;
    	}
    }

    //Determinação dos nós p e a em função da restrição violada
    noP = 0;
    switch(restricaoViolada)
    {
    case carregamentoRede:
        // Determina a árvore com maior carregamento de rede dentre as árvores da Região do Problema
        maiorCarregamentoRede = 0;
        for(indice = 0; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta; indice++){
        	indiceRNP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indice];
        	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede > maiorCarregamentoRede){
        		maiorCarregamentoRede = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede;
        		rnpP = indiceRNP;
        	}
        }
        //Determina o nó no qual tem ocorrido o maior carregamento na árvore 'rnpP' e o seu índice nesta árvore
        noInfactivel = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].fitnessRNP.noMaiorCarregamentoRede;
        indice = retornaColunaPi(matrizPiParam, vetorPiParam, noInfactivel, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noInfactivel' na floresta 'idConfiguracaoSelecionadaParam'
            indiceNoInfactivel = matrizPiParam[noInfactivel].colunas[indice].posicao;
            //determinação do intervalo da subárvore que tem o 'noInfactivel' como raiz
             indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceNoInfactivel);
             //Determina o nó de poda (noP), que consistirá num nó no intervalo entre 'indiceNoInfactivel' e 'indiceL' que não tenha CEs à jusante
             noP = 0;
             contador = 0;
             while(noP == 0 && contador < 100){
        		 //Determinação aleatória de um nó intervalo da subárvore que tem o nó P como raiz
        		 indiceP = inteiroAleatorio((indiceNoInfactivel+1), indiceL); // O '+1' deve-se ao fato de que, embora o sobrecarga ocorra no 'noInfactivel', a causa da mesma encontra-se à jusante deste nó
        		 noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
        		 //Verificação da existência de Consumidores Especiais na subárvore que tem o 'noP' como raiz
        		 potenciaAtivaNaoSuprida =  calculaSomatorioPotenciaAtivaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
        		 if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], grafoSetoresParam, indiceP) || potenciaAtivaNaoSuprida <= 0)
        			 noP=0;
        		 contador++;
             }
    	break;

    case quedaTensao:
        // Determina a árvore com o menor valor de tensão dentre as árvores da Região do Problema
        menorTensao = 999999999999;
        for(indice = 0; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta; indice++){
        	indiceRNP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indice];
        	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indiceRNP].fitnessRNP.menorTensao < menorTensao){
        		menorTensao = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indiceRNP].fitnessRNP.menorTensao;
        		rnpP = indiceRNP;
        	}
        }
        //Determina o nó no qual tem ocorrido o maior carregamento na árvore 'rnpP' e o seu índice nesta árvore
        noInfactivel = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].fitnessRNP.noMenorTensao;
        indice = retornaColunaPi(matrizPiParam, vetorPiParam, noInfactivel, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noInfactivel' na floresta 'idConfiguracaoSelecionadaParam'
            indiceNoInfactivel = matrizPiParam[noInfactivel].colunas[indice].posicao;
            //determinação do intervalo da subárvore que tem o 'noInfactivel' como raiz
             indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceNoInfactivel);
             //Determina o nó de poda (noP), que consistirá num nó no intervalo entre 'indiceNoInfactivel' e 'indiceL' que não tenha CEs à jusante
             noP = 0;
             contador = 0;
             while(noP == 0 && contador < 100){
        		 //Determinação aleatória de um nó intervalo da subárvore que tem o nó P como raiz
        		 indiceP = inteiroAleatorio((indiceNoInfactivel+1), indiceL); // O '+1' deve-se ao fato de que, embora o sobrecarga ocorra no 'noInfactivel', a causa da mesma encontra-se à jusante deste nó
        		 noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
        		 //Verificação da existência de Consumidores Especiais na subárvore que tem o 'noP' como raiz
        		 potenciaAtivaNaoSuprida =  calculaSomatorioPotenciaAtivaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
        		 if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], grafoSetoresParam, indiceP) || potenciaAtivaNaoSuprida <= 0)
        			 noP=0;
        		 contador++;
             }
    	break;

    case carregamentoTrafo:
    	nos.idNo = Malloc(int, 1);
    	//Determina o índice do trafo com o Maior Carregamento
    	idTrafoMaiorCarregamento = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.idTrafoMaiorCarregamento;
    	//Determina o tamanho da sobrecarga neste Trafo
    	sobrecarga = ((popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.maiorCarregamentoTrafo - 100)*dadosTrafoParam[idTrafoMaiorCarregamento].capacidade/100)*SBase; //Sobrecarga em VA

    	rnpFaltaTrafoMaiorCarregamento.idRnpFalta = Malloc(int, dadosTrafoParam[idTrafoMaiorCarregamento].numeroAlimentadores);
    	rnpFaltaTrafoMaiorCarregamento.numeroRnp = 0;

    	//Determina um conjunto formado pelos alimentadores conectados ao trafo com maior carregamento que pertencem a região do problema e salva-os em "rnpFaltaTrafoMaiorCarregamento"
    	for(indice = 0; indice < dadosTrafoParam[idTrafoMaiorCarregamento].numeroAlimentadores; indice++){   //Percorre as RNP conectadas ao trafo com sobrecarga
    		indiceRNP = dadosTrafoParam[idTrafoMaiorCarregamento].alimentador[indice];
    		for(indice2 = 0; indice2 < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta; indice2++){ //Percorrer as RNPs presentes na região do problema
    			indiceRNP2 = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indice2];
    			if(indiceRNP == indiceRNP2){																			//Verifica a existência de RNP conectados ao trafo com sobrecarga que estejam na região do problema
    				rnpFaltaTrafoMaiorCarregamento.idRnpFalta[rnpFaltaTrafoMaiorCarregamento.numeroRnp] = indiceRNP;    //Salva estas RNP numa estrutura temporária
    				rnpFaltaTrafoMaiorCarregamento.numeroRnp++;
    			}
    		}
    	}
    	if(rnpFaltaTrafoMaiorCarregamento.numeroRnp > 0){
    		//Seleciona aleatoriamente um dos Alimentadores pertencente ao conjunto determinado anteriormente

        	indice = inteiroAleatorio(0,rnpFaltaTrafoMaiorCarregamento.numeroRnp-1);
        	rnpP = rnpFaltaTrafoMaiorCarregamento.idRnpFalta[indice];
        	nos.idNo = (int *) realloc(nos.idNo, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos*sizeof(int) );
        	nos.numeroNos = 0;
        	//Determina todos os nós-folha do alimentador (RNP) selecionado
        	for(indice = 1; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos; indice++){
        		profAnt = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice-1].profundidade;
        		prof = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade;
        		if(prof <= profAnt){
        			nos.idNo[nos.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice-1].idNo;
        			nos.numeroNos++;
        		}
        	}
        	//Pelo procedimento acima, são obtidos todos os nós-folha de "rnpP" exceto aquele que ocupa a última posição no array. Mas aqui ele também é adicionado
        	indice = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-1;
        	nos.idNo[nos.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
			nos.numeroNos++;


			//Percorre todos os nós folhas para determinar o nó de poda (noP)
	    	noP = 0;
			maiorPotenciaEmCorte = 0;
			for(contador = 0; contador < nos.numeroNos; contador++){
				//Seleciona o nó-folha na posição "contador" para ser candidato a ser "noP"
				noPTemp = nos.idNo[contador];
				//Determina o indice de "noP" no alimentador a que pertence na configuração "idConfiguracaoSelecionadaParam"
				indice = retornaColunaPi(matrizPiParam, vetorPiParam, noPTemp, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noP' na floresta 'idConfiguracaoSelecionadaParam'
				indicePTemp = matrizPiParam[noPTemp].colunas[indice].posicao;
				flag = 0;
				noPAnterior = 0;
				while(flag != 1 && indicePTemp > 0){ //A partir do "noP" atual percorre a RNP no sentido contrário ao fluxo a fim de determinar um nó que tenha o nó-folha selecionado e que corte uma quantidade de carga próximo àquela desejada
					//Verifica a existência de CEs
					if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], grafoSetoresParam, indicePTemp)){
						flag = 1;
						noPTemp = 0;
					}
					else{
						//Determina a potência Aparente que ficará fora de serviço se "noP" atual vier a ser o nó de poda
						potenciaEmCorte =  calculaSomatorioPotenciaAparenteSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
						if(potenciaEmCorte >= sobrecarga){ //Veririfa se a quantidade de carga a ser cortada é menor que o valor limite para corte
							noPTemp = noPAnterior;
							indicePTemp = indicePAnterior;
							flag = 1;
						}
						else{//Se não houver CEs e "potenciaASerCortada" ainda for menor que o limite a ser cortado, então o noP é atualizado para ser um que corte uma quantidade maior de carga tendo o nó-folha previamente selecionado dentre aqueles que serão cortados
							noPAnterior = noPTemp;
							indicePAnterior = indicePTemp;
							//procurar o nó anterior ao noP atual para que este passe a ser o novo noP
							indice = indicePTemp-1;
							while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indicePTemp].profundidade)
							{
								indice--;
							}
							//atualiza o no de poda para que possa ser cortada uma quantidade maior de carga para alívio maior do tafo
							indicePTemp = indice;
							noPTemp = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indicePTemp].idNo;
						}
					}
				}
				// Verifica se foi obtido um nó de poda válido
				if(noPTemp!=0){
					potenciaEmCorte =  calculaSomatorioPotenciaAparenteSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
					if(potenciaEmCorte == 0) //Verifica se há cargas na subárvore a ser cortada
						noPTemp=0;
				}
				else
					potenciaEmCorte = 0;

				//Escolhe como noP aquele que, dentre os nós de poda temporário, cortará uma potência mais próxima daquela desejada
				if(potenciaEmCorte > maiorPotenciaEmCorte && noPTemp != 0){
					noP = noPTemp;
					indiceP = indicePTemp;
					maiorPotenciaEmCorte = potenciaEmCorte;
				}

			}

    	}
    	else{   //Se no trafo sobrecarregado não há nenhuma da RNP pertencente a região do problema, significa que esta sobrecarga é um problema de planejamento. Logo, o LSO não deve ser aplicado!
    		return(-1);
    	}
        free(rnpFaltaTrafoMaiorCarregamento.idRnpFalta);
	  	free(nos.idNo);
    	break;
    }



     if(noP == 0){
    	 return(-1);
    	 /*printf("\nFunção operadorLSOHeuristica(): tentativas de escolha do nó P excedidas. Todos os nós pesquisados possuem Consumidores Especiais!\n");
    	 exit(1);*/
     }
     else{
    	 //Escolha da RNP Fictícia Destino
		 /*Verifica a existência de Setores não Afetados pela Falta (não desligados após a sua ocorrência) dentre
		  * os setores presentes na subárvore que será cortada. Se todos os setores da subárvore cortada tiverem
		  * sido inicialmente afetados pela falta, então, a subárvore podada será transferida para a primeira
		  * RNP Fictícia.
		  * Caso, todos ou pelo menos um dos setores presentes na subárvore cortada não tenha sido inicialmente
		  * afetado pela falta, então a subárvore cortada será transferida para a segunda RNP Fictícia.
		  */
		 //Determina, dentre os setores a serem cortados, a quantidade de setores que foram inicialmente afetados pela falta
		 numeroSetorJusanteFalta = determinaQuantidadeSetoresJusanteFalta(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], grafoSetoresParam, indiceP);
		 // Determina o tamanho da subárvore podada
		 indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
		 tamanhoSubarvore = indiceL-indiceP+1;

		 if(numeroSetorJusanteFalta == tamanhoSubarvore){ //Se todos os setores a serem transferidos foram inicialmente afetados pela falta
			 //Então, a árvore destino será a RNP Fictícia 1 (que armazena exclusivamente setores afetados pela falta)
			 rnpA = 0;
			 indiceA = 0; //A nó no qual será inserida a árvore cortada é sempre o nó raiz da RNP Fictícia
			 noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[0].idNo;
			 profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[indiceA].profundidade;
		 }
		 else{  //Se há, dentre os setores a serem cortados, pelo menos um que não tenha qualquer relação com a falta, isto é, que não tenha sido inicialmente afetado
			 //Então, a árvore destino será a RNP Fictícia 2 (que armazena subárvores cortadas com setores não afetados pela falta)
			 rnpA = 1;
			 indiceA = 0; //A nó no qual será inserida a árvore cortada é sempre o nó raiz da RNP Fictícia
			 noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[indiceA].idNo;
			 profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[indiceA].profundidade;
		 }


		//localiza qual chave será aberta
		indice = indiceP-1;
		while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
		{
			indice--;
		}
		noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
		for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
		{
			if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
				idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
			/*else
				if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noA)
					idChaveFechada = grafoSetoresParam[noP].idChavesAdjacentes[indice];*/
			idChaveFechada = -1;
			//(*idChaveFechada)[0] = -1; //não precisa fechar chave
		}
		//realiza a alocação das RNPS que serão alteradas
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpA]);
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
		//constroi a rnp de origem nova
		constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
		//constroi a rnp de destino nova
	   // constroiRNPDestinoPAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);
		constroiRNPDestinoComDestinoFicticia(popConfiguracaoParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceP, matrizPiParam,
				idConfiguracaoSelecionadaParam, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpA], idNovaConfiguracaoParam);
		//calcula o número de manobras
		numeroManobrasModificada(popConfiguracaoParam,
			idChaveAberta, idChaveFechada, listaChavesParam, idConfiguracaoSelecionadaParam,
			estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
		//salva os setores utilizados na manobra, nesse caso os setore P e R são os mesmos.
		nosPRA[0].a = noA;
		nosPRA[0].p = noP;
		nosPRA[0].r = noP;
		//insere no vetor pi o ancestral do novo individuo obtido
		atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
				&idChaveAberta, &idChaveFechada,1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA);
		//copia o ponteiro para as rnps que não foram alteradas
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
		{
			if (indice != rnpP)
			{
				popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}
		}

		//copia o ponteiro para a rnp FICTÍCIA que não foi alterada
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ )
		{
			if (indice != rnpA)
			{
				popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
		}

		copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
		//Não é necessário que a RNP Fictícia entre no conjunto de RNPs da região do problema
		//insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);
		avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, -1, rnpP, idNovaConfiguracaoParam,
				dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
				indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
				ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
				sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

		/*TALVEZ NÃO SEJA NECESSÁRIO CALCULAR A QUANTIDADE DE CES TRANSFERIDOS, POIS, NO INICIO DESTA FUNÇÃO
	   ASSEGUROU-SE A INEXISTÊNCIA DE CES NOS NÓS CORTADOS*/

		//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
	    consumidoresManobrados = 0;
		for(contador = 0; contador < tamanhoSubarvore; contador++)
		{
			if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP)
			{
				consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
			else
			{
				consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
		}
		if(consumidoresManobrados != 0){
			printf("UAI!rs");
		}

		popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;

		return(1);
     }
}

/**
 * Por Leandro
 * DESCRIÇÃO: função que gera um novo indivíduo a partir de um indivíduo selecionado por meio da realização
 * de cortes de cargas. O corte é realizado no alimentador no qual uma restrição foi violado. A escolha das cargas
 * a serem cortadas depende de qual restrição fora violada.
 * De maneira sucinta, este operador consiste no operador PAO da RNP com as seguintes diferenças:
 * 1) o nó de poda é escolhido de maneira determinística
 * 2) a árvore destino será uma das duas árvores fictícias que armazenam setores saudáveis e reconectáveis cortados.
 * 3) é do tipo 'int' e retorna -1, caso não tenha sido possivel determinar uma subárbvore a ser cortada sem a presença de CE. E retorna 1 se
 * o operador tiver sido aplicado.
 * 4)para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRASALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 *
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 */
int operadorLSOHeuristicaV2(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
	int profundidadeA;
    long int noA, noPTemp, noP=0;;
    int indiceL, indiceA, indice, contador, contador2, indicePTemp, indiceP=0;
    long int noRaizP;
    int rnpP, rnpA;
    long int idChaveAberta, idChaveFechada;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados = 0;
    //Por Leandro
    int indiceRNP, indiceRNP2, indice2, indiceNoInfactivel, indicePAnterior; //Leandro
    long int noInfactivel, idTrafoMaiorCarregamento, noPAnterior; //Leandro
    int numeroSetorJusanteFalta; // Leandro: Salva a quantidade de setores, da subárvore cortada, que forma inicialmente afetados pela falta
    double potenciaAtivaNaoSuprida = 0, sobrecarga, potenciaEmCorte, maiorPotenciaEmCorte;
    double maiorDemanda;
    int prof, profAnt, flag;
    enum{
    	carregamentoRede, /**< Indica que a restrição violada a ser tratada será carregamento Rede. */
    	carregamentoTrafo, /**< Indica que a restrição violada a ser tratada será carregamento de subestação. */
    	quedaTensao  /**< Indica que a restrição violada a ser tratada será de queda de tensao. */
    } restricaoViolada;
    struct RNPFALTATRAFO {  //Estrutura que armazenará os indices das RNP conectadas ao trafo com maior carregamento que estão na região do Problema
    	int *idRnpFalta;
    	int numeroRnp;
    } rnpFaltaTrafoMaiorCarregamento;


    //Determina qual das restrições será tratada pelo LSOe. Dá-se preferência à eliminação de violação de carregamento de rede, posteriormente de queda de tensão e por fim de carregamento de rede
    if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.maiorCarregamentoRede > maxCarregamentoRede)
    	restricaoViolada = carregamentoRede;
    else{
    	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.quedaMaxima > maxQuedaTensao)
    		restricaoViolada = quedaTensao;
    	else{
    		if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.maiorCarregamentoTrafo > maxCarregamentoTrafo)
    			restricaoViolada = carregamentoTrafo;
    	}
    }

    //Determinação dos nós p e a em função da restrição violada
    noP = 0;
    switch(restricaoViolada)
    {
    case carregamentoRede:
        //Obtém o nó no qual tem ocorrido o maior carregamento, a árvore que o contém e o índice deste nó nesta árvore
        noInfactivel = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.noMaiorCarregamentoRede;
        indice = retornaColunaPi(matrizPiParam, vetorPiParam, noInfactivel, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noInfactivel' na floresta 'idConfiguracaoSelecionadaParam'
        indiceNoInfactivel = matrizPiParam[noInfactivel].colunas[indice].posicao;
        rnpP  = matrizPiParam[noInfactivel].colunas[indice].idRNP; //Árvore com maior carregamento de rede dentre as árvores da Região do Problema

		//Obtém o valor da sobrecargarga da rede (em Ámperes)
        sobrecarga = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.sobrecargaRede; // Valor em Amperes que ultrapassa o limite superior de corrente no trecho da rede com o maior carregamente percentual de rede

		//Determina o nó de Poda
		noP = determinaNoPodaCorrenteV2(sobrecarga, indiceNoInfactivel, rnpP, popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam, matrizB);
		//Determina o indice do "noP" na configuração "idConfiguracaoSelecionadaParam"
		if(noP!=0){
			indice = retornaColunaPi(matrizPiParam, vetorPiParam, noP, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noP' na floresta 'idConfiguracaoSelecionadaParam'
			indiceP = matrizPiParam[noP].colunas[indice].posicao;
		}
    	break;

    case quedaTensao:
        //Obtém o nó no qual tem ocorrido o maior carregamento, a árvore  que o contém e o indice deste nó nesta árvore
        noInfactivel = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.noMenorTensao;
        indice = retornaColunaPi(matrizPiParam, vetorPiParam, noInfactivel, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noInfactivel' na floresta 'idConfiguracaoSelecionadaParam'
		indiceNoInfactivel = matrizPiParam[noInfactivel].colunas[indice].posicao;
		rnpP = matrizPiParam[noInfactivel].colunas[indice].idRNP;

		//determinação do intervalo da subárvore que tem o 'noInfactivel' como raiz
		 indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceNoInfactivel);
		 //Determina o nó de poda (noP), que consistirá num nó no intervalo entre 'indiceNoInfactivel' e 'indiceL' que não tenha CEs à jusante
		 noP = 0;
		 contador = 0;
		 while(noP == 0 && contador < 100){
			 //Determinação aleatória de um nó intervalo da subárvore que tem o nó P como raiz
			 indiceP = inteiroAleatorio((indiceNoInfactivel+1), indiceL); // O '+1' deve-se ao fato de que, embora o sobrecarga ocorra no 'noInfactivel', a causa da mesma encontra-se à jusante deste nó
			 noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
			 //Verificação da existência de Consumidores Especiais na subárvore que tem o 'noP' como raiz
			 potenciaAtivaNaoSuprida =  calculaSomatorioPotenciaAtivaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
			 if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], grafoSetoresParam, indiceP) || potenciaAtivaNaoSuprida <= 0)
				 noP=0;
			 contador++;
             }
    	break;
    case carregamentoTrafo:
    	//Determina o índice do trafo com o Maior Carregamento
    	idTrafoMaiorCarregamento = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.idTrafoMaiorCarregamento;
    	//Determina o tamanho da sobrecarga neste Trafo (em Amperes)
    	sobrecarga = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.sobrecargaTrafo; // Valor, em Amperes, da corrente que excede o limite superior de condução de corrente do trafo da região do problema que está com o maior carregamente percentual

    	rnpFaltaTrafoMaiorCarregamento.idRnpFalta = Malloc(int, dadosTrafoParam[idTrafoMaiorCarregamento].numeroAlimentadores);
    	rnpFaltaTrafoMaiorCarregamento.numeroRnp = 0;

    	//Determina um conjunto formado pelos alimentadores conectados ao trafo com maior carregamento que pertencem a região do problema e salva-os em "rnpFaltaTrafoMaiorCarregamento"
    	for(indice = 0; indice < dadosTrafoParam[idTrafoMaiorCarregamento].numeroAlimentadores; indice++){   //Percorre as RNP conectadas ao trafo com sobrecarga
    		indiceRNP = dadosTrafoParam[idTrafoMaiorCarregamento].alimentador[indice];
    		for(indice2 = 0; indice2 < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPsFalta; indice2++){ //Percorrer as RNPs presentes na região do problema
    			indiceRNP2 = popConfiguracaoParam[idConfiguracaoSelecionadaParam].idRnpFalta[indice2];
    			if(indiceRNP == indiceRNP2){																			//Verifica a existência de RNP conectados ao trafo com sobrecarga que estejam na região do problema
    				rnpFaltaTrafoMaiorCarregamento.idRnpFalta[rnpFaltaTrafoMaiorCarregamento.numeroRnp] = indiceRNP;    //Salva estas RNP numa estrutura temporária
    				rnpFaltaTrafoMaiorCarregamento.numeroRnp++;
    			}
    		}
    	}


    	if(rnpFaltaTrafoMaiorCarregamento.numeroRnp > 0){
    		//Seleciona aleatoriamente um dos Alimentadores pertencente ao conjunto determinado anteriormente
    		contador = 0;
    		while(noP == 0 && contador < rnpFaltaTrafoMaiorCarregamento.numeroRnp){
    			//Determina, dentre os alimentadores ainda não verificados, o alimentador com a maior demanda para que seja a RNP na qual haverá corte ("rnpP")
    			maiorDemanda = 0;
    			for(indice = 0; indice < rnpFaltaTrafoMaiorCarregamento.numeroRnp; indice++){
    				if(rnpFaltaTrafoMaiorCarregamento.idRnpFalta[indice] > 0){  //Se o alimentador salvo em "indice"  ainda não foi verificado para determinação do "noP"
    		    		indiceRNP = rnpFaltaTrafoMaiorCarregamento.idRnpFalta[indice];

    					if(maiorDemanda < popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador){
    						maiorDemanda = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador;
    						rnpP = indiceRNP;
    						indice2 = indice; //indice2 salvará o índice em "rnpFaltaTrafoMaiorCarregamento.idRnpFalta[]" do alimentador com maior demanda, para que, caso ele não tenha sucesso na determinação de um nó de poda válido, o alimentador salvo na posição "indice2" de rnpFaltaTrafoMaiorCarregamento.idRnpFalta[] não seja mais verificado
    					}

    				}
    			}
    			rnpFaltaTrafoMaiorCarregamento.idRnpFalta[indice2] = -1; //Descarta o alimentador com maior demanda para que, caso não seja obtido nele um "noP" válido, ele não volte a ser verificado

				//Determina um nó de poda
				noP = determinaNoPodaCorrenteV2(sobrecarga, 0, rnpP, popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam, matrizB);
				//Determina o indice do "noP" na configuração "idConfiguracaoSelecionadaParam"
				if(noP!=0){
					indice = retornaColunaPi(matrizPiParam, vetorPiParam, noP, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noP' na floresta 'idConfiguracaoSelecionadaParam'
					indiceP = matrizPiParam[noP].colunas[indice].posicao;
				}
				contador++;
    		}
    	}
    	else{   //Se no trafo sobrecarregado não há nenhuma da RNP pertencente a região do problema, significa que esta sobrecarga é um problema de planejamento. Logo, o LSO não deve ser aplicado!
    		return(-1);
    	}
        free(rnpFaltaTrafoMaiorCarregamento.idRnpFalta);
    	break;
    }


     if(noP == 0){
    	 return(-1);
     }
     else{
    	 //Escolha da RNP Fictícia Destino
		 /*Verifica a existência de Setores não Afetados pela Falta (não desligados após a sua ocorrência) dentre
		  * os setores presentes na subárvore que será cortada. Se todos os setores da subárvore cortada tiverem
		  * sido inicialmente afetados pela falta, então, a subárvore podada será transferida para a primeira
		  * RNP Fictícia.
		  * Caso, todos ou pelo menos um dos setores presentes na subárvore cortada não tenha sido inicialmente
		  * afetado pela falta, então a subárvore cortada será transferida para a segunda RNP Fictícia.
		  */
		 //Determina, dentre os setores a serem cortados, a quantidade de setores que foram inicialmente afetados pela falta
		 numeroSetorJusanteFalta = determinaQuantidadeSetoresJusanteFalta(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], grafoSetoresParam, indiceP);
		 // Determina o tamanho da subárvore podada
		 indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
		 tamanhoSubarvore = indiceL-indiceP+1;

		 if(numeroSetorJusanteFalta == tamanhoSubarvore){ //Se todos os setores a serem transferidos foram inicialmente afetados pela falta
			 //Então, a árvore destino será a RNP Fictícia 1 (que armazena exclusivamente setores afetados pela falta)
			 rnpA = 0;
			 indiceA = 0; //A nó no qual será inserida a árvore cortada é sempre o nó raiz da RNP Fictícia
			 noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[0].idNo;
			 profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[indiceA].profundidade;
		 }
		 else{  //Se há, dentre os setores a serem cortados, pelo menos um que não tenha qualquer relação com a falta, isto é, que não tenha sido inicialmente afetado
			 //Então, a árvore destino será a RNP Fictícia 2 (que armazena subárvores cortadas com setores não afetados pela falta)
			 rnpA = 1;
			 indiceA = 0; //A nó no qual será inserida a árvore cortada é sempre o nó raiz da RNP Fictícia
			 noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[indiceA].idNo;
			 profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].nos[indiceA].profundidade;
		 }


		//localiza qual chave será aberta
		indice = indiceP-1;
		while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
		{
			indice--;
		}
		noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
		for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
		{
			if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP){
				idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				break;
			}
			/*else
				if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noA)
					idChaveFechada = grafoSetoresParam[noP].idChavesAdjacentes[indice];*/
		}
		idChaveFechada = -1; //A carga será cortada. Logo, não há chave a ser fechada
		//realiza a alocação das RNPS que serão alteradas
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpA]);
		alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
		//constroi a rnp de origem nova
		constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
		//constroi a rnp de destino nova
	   // constroiRNPDestinoPAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);
		constroiRNPDestinoComDestinoFicticia(popConfiguracaoParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceP, matrizPiParam,
				idConfiguracaoSelecionadaParam, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpA], idNovaConfiguracaoParam);
		//calcula o número de manobras
		numeroManobrasModificada(popConfiguracaoParam,
			idChaveAberta, idChaveFechada, listaChavesParam, idConfiguracaoSelecionadaParam,
			estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
		//salva os setores utilizados na manobra, nesse caso os setore P e R são os mesmos.
		nosPRA[0].a = noA;
		nosPRA[0].p = noP;
		nosPRA[0].r = noP;
		//insere no vetor pi o ancestral do novo individuo obtido
		atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
				&idChaveAberta, &idChaveFechada,1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, LSO);
		//copia o ponteiro para as rnps que não foram alteradas
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
		{
			if (indice != rnpP)
			{
				popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}
		}

		//copia o ponteiro para a rnp FICTÍCIA que não foi alterada
		for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ )
		{
			if (indice != rnpA)
			{
				popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
		}

		copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
		//Não é necessário que a RNP Fictícia entre no conjunto de RNPs da região do problema
		//insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);
		avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, -1, rnpP, idNovaConfiguracaoParam,
				dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
				indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
				ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
				sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

		/*TALVEZ NÃO SEJA NECESSÁRIO CALCULAR A QUANTIDADE DE CES TRANSFERIDOS, POIS, NO INICIO DESTA FUNÇÃO
	   ASSEGUROU-SE A INEXISTÊNCIA DE CES NOS NÓS CORTADOS*/

		//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
	    consumidoresManobrados = 0;
		for(contador = 0; contador < tamanhoSubarvore; contador++)
		{
			if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP)
			{
				consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
			else
			{
				consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
			}
		}
		if(consumidoresManobrados != 0){
			printf("UAI!rs");
		}

		popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;

		return(1);
     }
}

/*
 * Por Leandro:
 * DESCRIÇÃO: função que gera um novo indivíduo a partir de um indivíduo selecionado por meio da realização
 * de reconexão de cargas previamente cortadas.
 * De maneira sucinta, este operador consiste no operador CAO da RNP com as seguintes diferenças:
 * 1) a árvore origem é sempre uma das RNPs fictícias. É dado prioridade para reconexão de cargas presentes na RNP que armazena
 * cargas não afetadas pela falta
 * 2) o nó de poda é escolhido de maneira determinística e consiste no nó com profundidade 1 que permite a reconexão da maior quantidade de potência ativa
 *
 * Tem como parâmetros o vetor de configurações, a geração que está sendo executada, o identificador da configuração que irá
 * dar origem a nova solução, o grafo de setores, o vetor de matriz pi e a vetor pi.
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 * @param grafoSDRParam
 * @param sequenciaManobrasAlivioParam
 */
void operadorLROHeuristicaV00(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        double VFParam, DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR;
    int indiceL, indiceP, indiceA, indice, contador, indiceR;
    /*long int noRaizP;*/
    int rnpP, rnpA;
    long int idChaveAberta, idChaveFechada;
    int profundidadeA;
    int tamanhoSubarvore;
    noA = 0;
    int casoManobra;
    BOOL estadoInicialCA;
    BOOL estadoInicialCF;
    NOSPRA *nosPRA = Malloc(NOSPRA, 1);
    long int consumidoresManobrados = 0;
/*    int idNosProf;
    long int nosProf[200];*/
    double potenciaAtivaNaoSuprida = 0, maiorPotenciaAtivaNaoSuprida = 0;
    long int no;

    //potenciaAtivaNaoSuprida =  calculaSomatorioPotenciaAtivaSubarvore(popConfiguracaoParam, idNovaConfiguracaoParam, noP, matrizB, matrizPiParam, vetorPiParam);

    maiorPotenciaAtivaNaoSuprida = 0;
    //Determinação dos nós p,e a, para aplicação do operador
    //Executa até obter um nó a
    while (noA == 0) {
    	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[1].numeroNos > 1){ //Testa qual das duas RNPs Fictícias possui os setores cortados, dando preferência para a reconexão de setores na RNP Fictícia 2 (de cons. saudáveis não afetados pela falta)
    		rnpP = 1; //RNP FICTÍCIA 2
    		//Determina, dentre os nós com profundidade 1 na RNP Fictícia escolhida aquele que é raiz da subárvore com maior potência Ativa nao suprida
    		for(indice = 1; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos; indice++){
    			//O Nó de poda será, dentre os nós com profundidade 1 em rnp fictícia, que reconectar a maior quantidade de potência ativa não suprida
    			if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade == 1){
    				no = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
    				potenciaAtivaNaoSuprida =  calculaSomatorioPotenciaAtivaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, no, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
    				if(potenciaAtivaNaoSuprida > maiorPotenciaAtivaNaoSuprida){
    					maiorPotenciaAtivaNaoSuprida = potenciaAtivaNaoSuprida;
    					noP = no;
    					indiceP = indice;
    				}
    			}
    		}
			//determinação do intervalo correspondente a subárvore do nó P na RNP
			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
			//determinação do no R
			if (indiceL == indiceP) //nesse caso não existe outro nó para religar a subárvore
				indiceR = indiceP;
			else
				indiceR = inteiroAleatorio(indiceP, indiceL);
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;
    	}
    	else{
    		rnpP = 0; //RNP FICTÍCIA 1
    		//Determina, dentre os nós com profundidade 1 na RNP Fictícia escolhida aquele que é raiz da subárvore com maior potência Ativa nao suprida
    		for(indice = 1; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos; indice++){
    			//O Nó de poda será, dentre os nós com profundidade 1 em rnp fictícia, que reconectar a maior quantidade de potência ativa não suprida
    			if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade == 1){
    				no = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
    				potenciaAtivaNaoSuprida =  calculaSomatorioPotenciaAtivaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, no, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
    				if(potenciaAtivaNaoSuprida > maiorPotenciaAtivaNaoSuprida){
    					maiorPotenciaAtivaNaoSuprida = potenciaAtivaNaoSuprida;
    					noP = no;
    					indiceP = indice;
    				}
    			}
    		}
    		//determinação do intervalo correspondente a subárvore do nó P na RNP
			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
			//determinação do no R
			if (indiceL == indiceP) //nesse caso não existe outro nó para religar a subárvore
				indiceR = indiceP;
			else
				indiceR = inteiroAleatorio(indiceP, indiceL);
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;
    	}

        //determinação do intervalo correspondente a subárvore do nó P na RNP
       //indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
        //determinação do no A adjacente a R
       // printf("rnpP %d noP %ld \n",rnpP, noP);
        colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP, grafoSetoresParam, matrizPiParam, vetorPiParam);
       // if (colunaNoA != NULL) { //se obteve um nó a válido recupera os dados dele.
        if (colunaNoA != NULL && colunaNoA[0].idRNP < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP) { //se obteve um nó a válido recupera os dados dele.
            rnpA = colunaNoA[0].idRNP;
            indiceA = colunaNoA[0].posicao;
            noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;
            profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].profundidade;
        }
    }


    //localiza quais chaves serão aberta e fechada
    idChaveAberta = -1;
    for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
    {
            if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
                idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
    }

    //printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
    //calcula o tamanho da subárvore
    tamanhoSubarvore = indiceL-indiceP+1;
    //faz a alocação das novas RNPs
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP]);
    //Obtém a nova rnp de origem, sem a subárvore podada
    constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
    //Obtém a nova rnp de destino, inserindo a subárvore podada
    constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

   //copia os ponteiros das rnps não alteradas
    for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
    {
        if (indice != rnpA)
            popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
    }
    // /*- Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada -*/
    for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
        if (indice != rnpP)
    	popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
    }
    /*-----*/

    copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
    insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA);
    //imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
    numeroManobras(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
      idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
    //salva os setores utilizados na manobra
    nosPRA[0].a = noA;
    nosPRA[0].p = noP;
    nosPRA[0].r = noR;
    atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
            &idChaveAberta, &idChaveFechada,1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA);

    /*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, principalmente, que seja calculada a potência ativa não suprida*/
    avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, -1, idNovaConfiguracaoParam,
			dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
			indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
			ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
			sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

     /*TALVEZ NÃO SEJA NECESSÁRIO CALCULAR A QUANTIDADE DE CES TRANSFERIDOS, POIS, NO INICIO DESTA FUNÇÃO
    ASSEGUROU-SE A INEXISTÊNCIA DE CES NOS NÓS CORTADOS*/

    //Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
    consumidoresManobrados = 0;
    for(contador = 0; contador < tamanhoSubarvore; contador++)
    {
        //printf("\nQtdAntes: %ld - idAnt: %ld - idNova: %ld - idNo: %ld - Config(0): %d - Para: %d - a: %ld - p: %ld - Valor: %ld - ", popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoSelecionadaParam, idNovaConfiguracaoParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo, matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP, rnpA, noA, noP, grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais);
        if(matrizPiParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP + contador].idNo].colunas[0].idRNP == rnpA)
        {
            //printf("igual - ");
            consumidoresManobrados = consumidoresManobrados - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
            //popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos - grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
        }
        else
        {
            //printf("diferente - ");
            consumidoresManobrados = consumidoresManobrados + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
            //popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos + grafoSetoresParam[popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP + contador].idNo].numeroConsumidoresEspeciais;
        }
    }
	if(consumidoresManobrados != 0){
		printf("UAI!");
	}
    popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;

}



/*
 * Por Leandro:
 * DESCRIÇÃO: função que gera um novo indivíduo, a partir de um indivíduo selecionado, por meio
 * de reconexão de cargas saudáveis fora de serviço.
 *
 * De maneira sucinta, este operador consiste no operador CAO da RNP com as seguintes diferenças:
 * 1) Caso, "nosPreDefinidos" seja falso, a árvore origem é sempre uma RNP fictícia, a qual é escolhida de maneira determinística com base no ranqueamento das RNPs Fictícias.
 *    Caso, "nosPreDefinidos" seja verdadeiro, esta função permite que sejam pré-definidos e passado como parâmetro os nós de poda e transferência,
 * bem com a árvore origem e a árvore destino
 * 2) o nó de poda é escolhido a partir de três trincas PRA. A trinca escolhida será aquela que melhor priorizar o atendimento de consumidores prioritários. A escolha
 * das três trincas e randômica;
 *
 *
 * Esta função foi construída a partir da função "operadorLROHeuristica()". Algumas das diferenças entre elas estão
 * 1) na escolha da árvore origem e do nó de poda. A função "operadorLROHeuristica()" considera a existência de apenas duas RNPs
 * Fictícias - uma de setores cortados que estava envolvidos na falta e outra de setores cortados não afetados inicialmente pela falta
 * 2) esta função "operadorLROHeuristicaV2()" exige a definição de uma parâmentro adicional - "sequenciaManobrasAlivioParam", do tipo
 * "SEQUENCIAMANOBRASALIVIO", a qual informa a sequência na qual deseja-se operar as manobras que ocorrem em par para
 * trasferência de cargas entre alimentador. Esta parâmentro é necessário ao cálculo da ENS.
 * 3) diferentemente de "operadorLROHeuristica()", esta função permite que sejam pré-definidos os nós de poda e transferência,
 * bem com a árvore origem e a árvore destino
 *
 * 4) esta função retorna:
 * 		1, se alguma subárvore tiver sido restaurada, ou
 * 	   -1, se o número de RNP Fictícias for Nulo ou se nenhum dos setores presentes nas RNPs Fictícias for restaurado em virtude da ausência de chaves que permitam a restauração de carga
 *
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 * @param grafoSDRParam
 * @param nosPredefinidos
 * @param indicePParam
 * @param indiceAParam
 * @param rnpPParam
 * @param rnpAParam
 * @param sequenciaManobrasAlivioParam
 */
int operadorLROHeuristicaV01(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR;
    int indiceL, indiceP, indiceA, indice,  indiceR, numeroTentativas = 0, tamanhoSubarvore, casoManobra, rnpP, rnpA;
    long int noRaizP, idChaveAberta, idChaveFechada, numeroNos, consumidoresManobrados = 0;
    BOOL estadoInicialCA, estadoInicialCF;
    NOSPRA *nosPRA;

    //double potenciaAtivaNaoSuprida = 0, maiorPotenciaAtivaNaoSuprida = 0;
    numeroNos = determinaNumeroSetoresEmRNPFicticia(popConfiguracaoParam[idConfiguracaoSelecionadaParam]);

    if(numeroNos > popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia){ //Testa se o operador LRO pode ser aplicado na configuração selecionada

		if(nosPreDefinidos){
			indiceP = indicePParam;
			rnpP = rnpPParam;
			noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indicePParam].idNo;

			indiceR = indiceRParam;
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indiceRParam].idNo;

			indiceA = indiceAParam;
			rnpA = rnpAParam;
			noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpAParam].nos[indiceAParam].idNo;

			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);//determinação do intervalo correspondente a subárvore do nó P na RNP

		}
		else{
			//Determinação dos nós p,e a, para aplicação do operador

			//Determina a Arvore Fictícia mais próxima do topo do ranqueamento de RNPs Fictícias cujos setores possuam opção de reconexão a um alimentador energizado
			//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
			if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia > 1) //Se houver mais de uma rnpFictícia, seleciona uma com base no ranqueamento de RNPs Fictícias. Senão, a única rnp fictícia é a prórpia rnpP
				rnpP = determinaArvoreFicticiaOrigem(popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam );
			else
				rnpP = 0;

			noA = -1;
			numeroTentativas = 0;
			while (noA == -1 && rnpP >= 0 && numeroTentativas < 200 ) { 	//Executa até obter um nó A Válido ou até atingir o numero máximo de tentativas de determinação de um nó A

				//Escolha do nó de poda (nó P)
				indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos - 1));
				noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].idNo;

				//Escolha do nó do novo nó raiz (nó R)
	            indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
	            if (indiceL == indiceP) //nesse caso não existe outro nó que possa ser nó raiz para religar a subárvore
	                indiceR = indiceP;
	            else
	                indiceR = inteiroAleatorio(indiceP, indiceL);

	            noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;

	            colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP, grafoSetoresParam, matrizPiParam, vetorPiParam);
				if (colunaNoA != NULL && colunaNoA[0].idRNP < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP) { //se obteve um nó a válido recupera os dados dele.
					rnpA = colunaNoA[0].idRNP;
					indiceA = colunaNoA[0].posicao;
					noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;

	                idChaveAberta = idChaveFechada = -999;
	                localizaChavesOperadorLRO(&idChaveAberta, &idChaveFechada, indiceP, noP, noR, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
	        		if(idChaveAberta == -999 || idChaveFechada == -999)
	        			noA = -1;
				}
				numeroTentativas++;
			}
		}

		if(rnpP == -1 || noA == -1){//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
			return -1;
		}
		else{

			//localiza qual chave será aberta
			if(indiceP > 1){
				indice = indiceP-1;
				while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].profundidade)
				{
					indice--;
				}
				noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
				idChaveAberta = -999;
				for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
				{
					if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
						idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				}
				if(idChaveAberta == -999){
					printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
					exit(1);
				}

			}
			else
				idChaveAberta = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

			//localiza qual chave será fechada
			idChaveFechada = -999;
			for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
			{
					if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
						idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
			}
			if(idChaveFechada == -999){
				printf("\n\n\nNão foi possível determinar o identificador da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
				exit(1);
			}

			//printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
			//calcula o tamanho da subárvore
			tamanhoSubarvore = indiceL-indiceP+1;
			//faz a alocação das novas RNPs
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP]);
			//Obtém a nova rnp de origem, sem a subárvore podada
			constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
			//Obtém a nova rnp de destino, inserindo a subárvore podada
			constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

		   //copia os ponteiros das rnps não alteradas
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
			{
				if (indice != rnpA)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}

			// Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
				if (indice != rnpP)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
			popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;

			/*//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--; */

			copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
			insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA); //Adiciona a árvore-destino à lista que contém os alimentadores envolvidos no problema para o NOVO indivíduo gerado
			insereRNPLista(&popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpA); //Altera a lista que contém os alimentadores envolvidos no problema, para o indivíduo-PAI. Isto consiste numa modificação feita na heurística original.Ver arquivo com notas
			//imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
			numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
			  idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
			//salva os setores utilizados na manobra
			nosPRA = Malloc(NOSPRA, 1);
			nosPRA[0].a = noA;
			nosPRA[0].p = noP;
			nosPRA[0].r = noR;
			atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
					&idChaveAberta, &idChaveFechada, 1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, LRO);

			//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
/*			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--;*/

			/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, principalmente, que seja calculada a potência ativa não suprida*/
			/*avaliaConfiguracaoHeuristica(false, popConfiguracaoParam, rnpA, rnpP, idNovaConfiguracaoParam,
					dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
					indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, VFParam, idConfiguracaoSelecionadaParam, matrizB,
					ZParam, maximoCorrenteParam, numeroBarrasParam, false);*/
			avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, -1, idNovaConfiguracaoParam,
					dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
					indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
					ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
					sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

			//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
			consumidoresManobrados = 0; //Como não é feito transferência de setores entre alimentadores,  o número de consumidores transferidos por este operador é nulo

			popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;
			//Nestas variáveis abaixo são acumuladas a potência transferida de um alimentador energizado para outro. Como neste caso, há reconexão de carga e não transferência, então elas são nulas
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade = 0;

			nosPRA = NULL; free(nosPRA);
			return 1;
		}

    }
    else{
    	printf("\n A configuração selecionada não possui setores a serem reconectados! Escolha outro operador");
    	return -1;
    }

}

/*
 * Por Leandro:
 * DESCRIÇÃO: função que gera um novo indivíduo, a partir de um indivíduo selecionado, por meio
 * de reconexão de cargas saudáveis fora de serviço. Ademais, diferentemente das funções "operadorLROHeuristicaV01()"
 * e "operadorLROHeuristica00()", esta função faz com a escolha dos nos PRA ocorra a partir de três trincas PRA e que consista
 * naquela, dentre as três, que restaurar o maior valor de potência ativa considerando os graus de prioridade de atendimento
 * entre os conumidores.
 *
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 * @param grafoSDRParam
 * @param nosPredefinidos
 * @param indicePParam
 * @param indiceAParam
 * @param rnpPParam
 * @param rnpAParam
 * @param sequenciaManobrasAlivioParam
 */
int operadorLROHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noAAux, noPAux = -1, noRAux;
    int indiceL, indiceP, indiceA, indice,  indiceR, numeroTentativas = 0, numeroTrincasObtidas = 0, contadorTentativasSorteioTrinca = 0, tamanhoSubarvore, casoManobra, rnpP, rnpA, indiceAAux, indicePAux, indiceRAux, rnpAAux, rnpPAux, indiceLAux;
    long int noRaizP, idChaveAberta, idChaveFechada, consumidoresManobrados = 0;//, numeroNos;
    BOOL estadoInicialCA, estadoInicialCF;
    NOSPRA *nosPRA;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaRestaurada, potenciaRestauradaAux;
    double potenciaRestauradaTotal;

//    numeroNos = determinaNumeroSetoresEmRNPFicticia(popConfiguracaoParam[idConfiguracaoSelecionadaParam]);
//    if(numeroNos > popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia){ //Testa se o operador LRO pode ser aplicado na configuração selecionada
	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.potenciaTotalNaoSuprida > 0){ //Testa se há cargas para serem restauradas, ie., se o operador LRO pode ser aplicado na configuração selecionada

		if(nosPreDefinidos){
			indiceP = indicePParam;
			rnpP = rnpPParam;
			noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indicePParam].idNo;

			indiceR = indiceRParam;
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indiceRParam].idNo;

			indiceA = indiceAParam;
			rnpA = rnpAParam;
			noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpAParam].nos[indiceAParam].idNo;

			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);//determinação do intervalo correspondente a subárvore do nó P na RNP

		}
		else{
			//Determinação dos nós p,e a, para aplicação do operador

//			//Determina a Arvore Fictícia mais próxima do topo do ranqueamento de RNPs Fictícias cujos setores possuam opção de reconexão a um alimentador energizado
//			//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
//			if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia > 1) //Se houver mais de uma rnpFictícia, seleciona uma com base no ranqueamento de RNPs Fictícias. Senão, a única rnp fictícia é a prórpia rnpP
//				rnpP = determinaArvoreFicticiaOrigemV0(popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam );
//			else
//				rnpP = 0;

			//Escolha de uma RNP Fictícia
			rnpP = determinaArvoreFicticiaOrigem(popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam );

//			TRINCA *trinca = Malloc(TRINCA, 3);
			numeroTrincasObtidas = 0;
			contadorTentativasSorteioTrinca = 0;
			potenciaRestauradaAux.consumidoresPrioridadeAlta = -1;
			potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = -1;
			potenciaRestauradaAux.consumidoresPrioridadeBaixa = -1;
			potenciaRestauradaAux.consumidoresSemPrioridade = -1;
			noAAux = -1;
			noPAux = -1;
			while(numeroTrincasObtidas < 3 && contadorTentativasSorteioTrinca < 6 && rnpP >= 0){
				noA = -1;
				numeroTentativas = 0;
				while (noA == -1 && rnpP >= 0 && numeroTentativas < 100 ) { 	//Executa até obter um nó A Válido ou até atingir o numero máximo de tentativas de determinação de um nó A

					//Escolha do nó de poda (nó P)
					potenciaRestauradaTotal = 0;
					while(potenciaRestauradaTotal == 0){
						indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos - 1));
						noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].idNo;
						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
						potenciaRestauradaTotal = potenciaRestaurada.consumidoresPrioridadeAlta + potenciaRestaurada.consumidoresPrioridadeBaixa + potenciaRestaurada.consumidoresPrioridadeIntermediaria + potenciaRestaurada.consumidoresSemPrioridade;
					}

					//Escolha do nó do novo nó raiz (nó R)
					indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
					if (indiceL == indiceP) //nesse caso não existe outro nó que possa ser nó raiz para religar a subárvore
						indiceR = indiceP;
					else
						indiceR = inteiroAleatorio(indiceP, indiceL);

					noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;

					colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP, grafoSetoresParam, matrizPiParam, vetorPiParam);
					if (colunaNoA != NULL && colunaNoA[0].idRNP < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP) { //se obteve um nó a válido recupera os dados dele.
						rnpA = colunaNoA[0].idRNP;
						indiceA = colunaNoA[0].posicao;
						noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;

//						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);

						idChaveAberta = idChaveFechada = -999;
						localizaChavesOperadorLRO(&idChaveAberta, &idChaveFechada, indiceP, noP, noR, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
						if(idChaveAberta == -999 || idChaveFechada == -999)
							noA = -1;
					}
					numeroTentativas++;
				}

				if(noA != -1 && noP != noPAux){ //Se uma nova trinca foi determinada
					if((floor(potenciaRestaurada.consumidoresPrioridadeAlta) >  floor(potenciaRestauradaAux.consumidoresPrioridadeAlta)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) >  floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) >  floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) == floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa) && floor(potenciaRestaurada.consumidoresSemPrioridade) > floor(potenciaRestauradaAux.consumidoresSemPrioridade))
					){
						noAAux = noA;
						noPAux = noP;
						noRAux = noR;
						indiceAAux = indiceA;
						indicePAux = indiceP;
						indiceRAux = indiceR;
						rnpAAux = rnpA;
						rnpPAux = rnpP;
						indiceLAux = indiceL;
						potenciaRestauradaAux.consumidoresPrioridadeAlta = potenciaRestaurada.consumidoresPrioridadeAlta;
						potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = potenciaRestaurada.consumidoresPrioridadeIntermediaria;
						potenciaRestauradaAux.consumidoresPrioridadeBaixa = potenciaRestaurada.consumidoresPrioridadeBaixa;
						potenciaRestauradaAux.consumidoresSemPrioridade = potenciaRestaurada.consumidoresSemPrioridade;
					}
					numeroTrincasObtidas++;
				}
				contadorTentativasSorteioTrinca++;
			}
		}

		if(rnpP == -1 || numeroTrincasObtidas == 0){//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
			return -1;
		}
		else{
			noA = noAAux;
			noP = noPAux;
			noR = noRAux;
			indiceA = indiceAAux;
			indiceP = indicePAux;
			indiceR = indiceRAux;
			rnpA = rnpAAux;
			rnpP = rnpPAux;
			indiceL = indiceLAux;

			//localiza qual chave será aberta
			if(indiceP > 1){
				indice = indiceP-1;
				while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].profundidade)
				{
					indice--;
				}
				noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
				idChaveAberta = -999;
				for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
				{
					if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
						idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				}
				if(idChaveAberta == -999){
					printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
					exit(1);
				}

			}
			else
				idChaveAberta = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

			//localiza qual chave será fechada
			idChaveFechada = -999;
			for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
			{
					if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
						idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
			}
			if(idChaveFechada == -999){
				printf("\n\n\nNão foi possível determinar o identificador da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
				exit(1);
			}

			//printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
			//calcula o tamanho da subárvore
			tamanhoSubarvore = indiceL-indiceP+1;
			//faz a alocação das novas RNPs
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP]);
			//Obtém a nova rnp de origem, sem a subárvore podada
			constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
			//Obtém a nova rnp de destino, inserindo a subárvore podada
			constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

		   //copia os ponteiros das rnps não alteradas
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
			{
				if (indice != rnpA)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}

			// Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
				if (indice != rnpP)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
			popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;

			/*//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--; */

			copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
			insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA); //Adiciona a árvore-destino à lista que contém os alimentadores envolvidos no problema para o NOVO indivíduo gerado
			insereRNPLista(&popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpA); //Altera a lista que contém os alimentadores envolvidos no problema, para o indivíduo-PAI. Isto consiste numa modificação feita na heurística original.Ver arquivo com notas
			//imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
			numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
			  idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
			//salva os setores utilizados na manobra
			nosPRA = Malloc(NOSPRA, 1);
			nosPRA[0].a = noA;
			nosPRA[0].p = noP;
			nosPRA[0].r = noR;
			atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
					&idChaveAberta, &idChaveFechada, 1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, LRO);

			//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
/*			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--;*/

			//Nestas variáveis abaixo são acumuladas a potência transferida de um alimentador energizado para outro. Como neste caso, há reconexão de carga e não transferência, então elas são nulas
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade = 0;

			/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, principalmente, que seja calculada a potência ativa não suprida*/
			avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, -1, idNovaConfiguracaoParam,
					dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
					indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
					ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
					sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

			//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
			consumidoresManobrados = 0; //Como não é feito transferência de setores entre alimentadores,  o número de consumidores transferidos por este operador é nulo

			popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;


			nosPRA = NULL; free(nosPRA);
			return 1;
		}

    }
    else{
    	printf("\n A configuração selecionada não possui setores a serem reconectados! Escolha outro operador");
    	return -1;
    }

}

/*
 * Por Leandro:
 * DESCRIÇÃO: consiste na função "operadorLROHeuristica()" modificada para:
 * (a) que no processo de escolha das três subárvores candidatas possam ser selecionados qualquer uma das RNPs Fictícias. Assim,
 * poderão ocorrer, por exemplo, duas subárvores de uma RNP fictícia e uma terceira de uma outra RNP.
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 * @param grafoSDRParam
 * @param nosPredefinidos
 * @param indicePParam
 * @param indiceAParam
 * @param rnpPParam
 * @param rnpAParam
 * @param sequenciaManobrasAlivioParam
 */
int operadorLROHeuristicaV2(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noAAux, noPAux, noRAux;
    int indiceL, indiceP, indiceA, indice,  indiceR, numeroTentativas = 0, numeroTrincasObtidas = 0, contadorTentativasSorteioTrinca = 0, tamanhoSubarvore, casoManobra, rnpP, rnpA, indiceAAux, indicePAux, indiceRAux, rnpAAux, rnpPAux, indiceLAux;
    long int noRaizP, idChaveAberta, idChaveFechada, numeroNos, consumidoresManobrados = 0;
    BOOL estadoInicialCA, estadoInicialCF;
    NOSPRA *nosPRA;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaRestaurada, potenciaRestauradaAux;
    double potenciaRestauradaTotal;

	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.potenciaTotalNaoSuprida > 0){ //Testa se há cargas para serem restauradas, ie., se o operador LRO pode ser aplicado na configuração selecionada

		if(nosPreDefinidos){
			indiceP = indicePParam;
			rnpP = rnpPParam;
			noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indicePParam].idNo;

			indiceR = indiceRParam;
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indiceRParam].idNo;

			indiceA = indiceAParam;
			rnpA = rnpAParam;
			noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpAParam].nos[indiceAParam].idNo;

			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);//determinação do intervalo correspondente a subárvore do nó P na RNP

		}
		else{
			//Determinação dos nós p,e a, para aplicação do operador
			numeroTrincasObtidas = 0;
			contadorTentativasSorteioTrinca = 0;
			potenciaRestauradaAux.consumidoresPrioridadeAlta = -1;
			potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = -1;
			potenciaRestauradaAux.consumidoresPrioridadeBaixa = -1;
			potenciaRestauradaAux.consumidoresSemPrioridade = -1;
			noAAux = -1;
			noPAux = -1;
			while(numeroTrincasObtidas < 3 && contadorTentativasSorteioTrinca < 6){
				noA = -1;
				numeroTentativas = 0;
				rnpP = 0;
				while (noA == -1 && rnpP >= 0 && numeroTentativas < 100 ) { 	//Executa até obter um nó A Válido ou até atingir o numero máximo de tentativas de determinação de um nó A
					//Escolha de uma RNP Fictícia
					potenciaRestauradaTotal = 0;
					while(potenciaRestauradaTotal == 0){ //Essa RNP precisa ter cargas (potencia ativa) não suprida
						rnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia - 1));
						if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos > 1){
							potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[1].idNo, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
							potenciaRestauradaTotal = potenciaRestaurada.consumidoresPrioridadeAlta + potenciaRestaurada.consumidoresPrioridadeBaixa + potenciaRestaurada.consumidoresPrioridadeIntermediaria + potenciaRestaurada.consumidoresSemPrioridade;
						}
						else
							potenciaRestauradaTotal = 0;
					}

					//Escolha do nó de poda (nó P)
					potenciaRestauradaTotal = 0;
					while(potenciaRestauradaTotal == 0){
						indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos - 1));
						noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].idNo;
						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
						potenciaRestauradaTotal = potenciaRestaurada.consumidoresPrioridadeAlta + potenciaRestaurada.consumidoresPrioridadeBaixa + potenciaRestaurada.consumidoresPrioridadeIntermediaria + potenciaRestaurada.consumidoresSemPrioridade;
					}

					//Escolha do nó do novo nó raiz (nó R)
					indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
					if (indiceL == indiceP) //nesse caso não existe outro nó que possa ser o novo nó raiz, para religar a subárvore
						indiceR = indiceP;
					else
						indiceR = inteiroAleatorio(indiceP, indiceL);

					noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;

					colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP, grafoSetoresParam, matrizPiParam, vetorPiParam);
					if (colunaNoA != NULL && colunaNoA[0].idRNP < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP) { //se obteve um nó a válido recupera os dados dele.
						rnpA = colunaNoA[0].idRNP;
						indiceA = colunaNoA[0].posicao;
						noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;

						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);

						idChaveAberta = idChaveFechada = -999;
						localizaChavesOperadorLRO(&idChaveAberta, &idChaveFechada, indiceP, noP, noR, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
						if(idChaveAberta == -999 || idChaveFechada == -999)
							noA = -1;
					}
					numeroTentativas++;
				}

				if(noA != -1 && noP != noPAux){//Se uma nova trinca foi determinada
					if((floor(potenciaRestaurada.consumidoresPrioridadeAlta) >  floor(potenciaRestauradaAux.consumidoresPrioridadeAlta)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) >  floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) >  floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) == floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa) && floor(potenciaRestaurada.consumidoresSemPrioridade) > floor(potenciaRestauradaAux.consumidoresSemPrioridade))
					){
						noAAux = noA;
						noPAux = noP;
						noRAux = noR;
						indiceAAux = indiceA;
						indicePAux = indiceP;
						indiceRAux = indiceR;
						rnpAAux = rnpA;
						rnpPAux = rnpP;
						indiceLAux = indiceL;
						potenciaRestauradaAux.consumidoresPrioridadeAlta = potenciaRestaurada.consumidoresPrioridadeAlta;
						potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = potenciaRestaurada.consumidoresPrioridadeIntermediaria;
						potenciaRestauradaAux.consumidoresPrioridadeBaixa = potenciaRestaurada.consumidoresPrioridadeBaixa;
						potenciaRestauradaAux.consumidoresSemPrioridade = potenciaRestaurada.consumidoresSemPrioridade;
					}
					numeroTrincasObtidas++;
				}
				contadorTentativasSorteioTrinca++;
			}
		}

		if(rnpP == -1 || numeroTrincasObtidas == 0){//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
			return -1;
		}
		else{
			noA = noAAux;
			noP = noPAux;
			noR = noRAux;
			indiceA = indiceAAux;
			indiceP = indicePAux;
			indiceR = indiceRAux;
			rnpA = rnpAAux;
			rnpP = rnpPAux;
			indiceL = indiceLAux;

			//localiza qual chave será aberta
			if(indiceP > 1){
				indice = indiceP-1;
				while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].profundidade)
				{
					indice--;
				}
				noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
				idChaveAberta = -999;
				for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
				{
					if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
						idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				}
				if(idChaveAberta == -999){
					printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
					exit(1);
				}

			}
			else
				idChaveAberta = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

			//localiza qual chave será fechada
			idChaveFechada = -999;
			for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
			{
					if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
						idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
			}
			if(idChaveFechada == -999){
				printf("\n\n\nNão foi possível determinar o identificador da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
				exit(1);
			}

			//printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
			//calcula o tamanho da subárvore
			tamanhoSubarvore = indiceL-indiceP+1;
			//faz a alocação das novas RNPs
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP]);
			//Obtém a nova rnp de origem, sem a subárvore podada
			constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
			//Obtém a nova rnp de destino, inserindo a subárvore podada
			constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

		   //copia os ponteiros das rnps não alteradas
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
			{
				if (indice != rnpA)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}

			// Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
				if (indice != rnpP)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
			popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;

			/*//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--; */

			copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
			insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA); //Adiciona a árvore-destino à lista que contém os alimentadores envolvidos no problema para o NOVO indivíduo gerado
			insereRNPLista(&popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpA); //Altera a lista que contém os alimentadores envolvidos no problema, para o indivíduo-PAI. Isto consiste numa modificação feita na heurística original.Ver arquivo com notas
			//imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
			numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
			  idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
			//salva os setores utilizados na manobra
			nosPRA = Malloc(NOSPRA, 1);
			nosPRA[0].a = noA;
			nosPRA[0].p = noP;
			nosPRA[0].r = noR;
			atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
					&idChaveAberta, &idChaveFechada, 1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, LRO);

			//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
/*			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--;*/

			//Nestas variáveis abaixo são acumuladas a potência transferida de um alimentador energizado para outro. Como neste caso, há reconexão de carga e não transferência, então elas são nulas
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade = 0;

			/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, principalmente, que seja calculada a potência ativa não suprida*/
			avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, -1, idNovaConfiguracaoParam,
					dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
					indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
					ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
					sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

			//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
			consumidoresManobrados = 0; //Como não é feito transferência de setores entre alimentadores,  o número de consumidores transferidos por este operador é nulo

			popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;


			nosPRA = NULL; free(nosPRA);
			return 1;
		}

    }
    else{
    	printf("\n A configuração selecionada não possui setores a serem reconectados! Escolha outro operador");
    	return -1;
    }

}

/*
 * Por Leandro:
 * DESCRIÇÃO: consiste na função "operadorLROHeuristica()" modificada para:
 * (a) que haja apenas uma única trinca PRA selecionada, evitando assim a competição entre 3 trincas, como na função "operadorLROHeuristica()", e consequentemente,
 * corroborando para que a reconexão dos setores ocorra em numero menor.
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 * @param grafoSDRParam
 * @param nosPredefinidos
 * @param indicePParam
 * @param indiceAParam
 * @param rnpPParam
 * @param rnpAParam
 * @param sequenciaManobrasAlivioParam
 */
int operadorLROHeuristicaV3(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noAAux, noPAux = -1, noRAux;
    int indiceL, indiceP, indiceA, indice,  indiceR, numeroTentativas = 0, numeroTrincasObtidas = 0, contadorTentativasSorteioTrinca = 0, tamanhoSubarvore, casoManobra, rnpP, rnpA, indiceAAux, indicePAux, indiceRAux, rnpAAux, rnpPAux, indiceLAux;
    long int noRaizP, idChaveAberta, idChaveFechada, consumidoresManobrados = 0;//, numeroNos;
    BOOL estadoInicialCA, estadoInicialCF;
    NOSPRA *nosPRA;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaRestaurada, potenciaRestauradaAux;
    double potenciaRestauradaTotal;

//    numeroNos = determinaNumeroSetoresEmRNPFicticia(popConfiguracaoParam[idConfiguracaoSelecionadaParam]);
//    if(numeroNos > popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia){ //Testa se o operador LRO pode ser aplicado na configuração selecionada
	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.potenciaTotalNaoSuprida > 0){ //Testa se há cargas para serem restauradas, ie., se o operador LRO pode ser aplicado na configuração selecionada

		if(nosPreDefinidos){
			indiceP = indicePParam;
			rnpP = rnpPParam;
			noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indicePParam].idNo;

			indiceR = indiceRParam;
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indiceRParam].idNo;

			indiceA = indiceAParam;
			rnpA = rnpAParam;
			noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpAParam].nos[indiceAParam].idNo;

			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);//determinação do intervalo correspondente a subárvore do nó P na RNP

		}
		else{
			//Determinação dos nós p,e a, para aplicação do operador

//			//Determina a Arvore Fictícia mais próxima do topo do ranqueamento de RNPs Fictícias cujos setores possuam opção de reconexão a um alimentador energizado
//			//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
//			if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia > 1) //Se houver mais de uma rnpFictícia, seleciona uma com base no ranqueamento de RNPs Fictícias. Senão, a única rnp fictícia é a prórpia rnpP
//				rnpP = determinaArvoreFicticiaOrigemV0(popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam );
//			else
//				rnpP = 0;

			//Escolha de uma RNP Fictícia
			rnpP = determinaArvoreFicticiaOrigem(popConfiguracaoParam, idConfiguracaoSelecionadaParam, grafoSetoresParam, matrizPiParam, vetorPiParam );

//			TRINCA *trinca = Malloc(TRINCA, 3);
			numeroTrincasObtidas = 0;
			contadorTentativasSorteioTrinca = 0;
			potenciaRestauradaAux.consumidoresPrioridadeAlta = -1;
			potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = -1;
			potenciaRestauradaAux.consumidoresPrioridadeBaixa = -1;
			potenciaRestauradaAux.consumidoresSemPrioridade = -1;
			noAAux = -1;
			noPAux = -1;
//			while(numeroTrincasObtidas < 3 && contadorTentativasSorteioTrinca < 6 && rnpP >= 0){
			while(numeroTrincasObtidas < 1 && contadorTentativasSorteioTrinca < 6 && rnpP >= 0){
				noA = -1;
				numeroTentativas = 0;
				while (noA == -1 && rnpP >= 0 && numeroTentativas < 100 ) { 	//Executa até obter um nó A Válido ou até atingir o numero máximo de tentativas de determinação de um nó A

					//Escolha do nó de poda (nó P)
					potenciaRestauradaTotal = 0;
					while(potenciaRestauradaTotal == 0){
						indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos - 1));
						noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].idNo;
						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
						potenciaRestauradaTotal = potenciaRestaurada.consumidoresPrioridadeAlta + potenciaRestaurada.consumidoresPrioridadeBaixa + potenciaRestaurada.consumidoresPrioridadeIntermediaria + potenciaRestaurada.consumidoresSemPrioridade;
					}

					//Escolha do nó do novo nó raiz (nó R)
					indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
					if (indiceL == indiceP) //nesse caso não existe outro nó que possa ser nó raiz para religar a subárvore
						indiceR = indiceP;
					else
						indiceR = inteiroAleatorio(indiceP, indiceL);

					noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;

					colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP, grafoSetoresParam, matrizPiParam, vetorPiParam);
					if (colunaNoA != NULL && colunaNoA[0].idRNP < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP) { //se obteve um nó a válido recupera os dados dele.
						rnpA = colunaNoA[0].idRNP;
						indiceA = colunaNoA[0].posicao;
						noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;

//						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);

						idChaveAberta = idChaveFechada = -999;
						localizaChavesOperadorLRO(&idChaveAberta, &idChaveFechada, indiceP, noP, noR, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
						if(idChaveAberta == -999 || idChaveFechada == -999)
							noA = -1;
					}
					numeroTentativas++;
				}

				if(noA != -1 && noP != noPAux){ //Se uma nova trinca foi determinada
					if((floor(potenciaRestaurada.consumidoresPrioridadeAlta) >  floor(potenciaRestauradaAux.consumidoresPrioridadeAlta)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) >  floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) >  floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) == floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa) && floor(potenciaRestaurada.consumidoresSemPrioridade) > floor(potenciaRestauradaAux.consumidoresSemPrioridade))
					){
						noAAux = noA;
						noPAux = noP;
						noRAux = noR;
						indiceAAux = indiceA;
						indicePAux = indiceP;
						indiceRAux = indiceR;
						rnpAAux = rnpA;
						rnpPAux = rnpP;
						indiceLAux = indiceL;
						potenciaRestauradaAux.consumidoresPrioridadeAlta = potenciaRestaurada.consumidoresPrioridadeAlta;
						potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = potenciaRestaurada.consumidoresPrioridadeIntermediaria;
						potenciaRestauradaAux.consumidoresPrioridadeBaixa = potenciaRestaurada.consumidoresPrioridadeBaixa;
						potenciaRestauradaAux.consumidoresSemPrioridade = potenciaRestaurada.consumidoresSemPrioridade;
					}
					numeroTrincasObtidas++;
				}
				contadorTentativasSorteioTrinca++;
			}
		}

		if(rnpP == -1 || numeroTrincasObtidas == 0){//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
			return -1;
		}
		else{
			noA = noAAux;
			noP = noPAux;
			noR = noRAux;
			indiceA = indiceAAux;
			indiceP = indicePAux;
			indiceR = indiceRAux;
			rnpA = rnpAAux;
			rnpP = rnpPAux;
			indiceL = indiceLAux;

			//localiza qual chave será aberta
			if(indiceP > 1){
				indice = indiceP-1;
				while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].profundidade)
				{
					indice--;
				}
				noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
				idChaveAberta = -999;
				for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
				{
					if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
						idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				}
				if(idChaveAberta == -999){
					printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
					exit(1);
				}

			}
			else
				idChaveAberta = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

			//localiza qual chave será fechada
			idChaveFechada = -999;
			for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
			{
					if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
						idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
			}
			if(idChaveFechada == -999){
				printf("\n\n\nNão foi possível determinar o identificador da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
				exit(1);
			}

			//printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
			//calcula o tamanho da subárvore
			tamanhoSubarvore = indiceL-indiceP+1;
			//faz a alocação das novas RNPs
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP]);
			//Obtém a nova rnp de origem, sem a subárvore podada
			constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
			//Obtém a nova rnp de destino, inserindo a subárvore podada
			constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

		   //copia os ponteiros das rnps não alteradas
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
			{
				if (indice != rnpA)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}

			// Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
				if (indice != rnpP)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
			popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;

			/*//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--; */

			copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
			insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA); //Adiciona a árvore-destino à lista que contém os alimentadores envolvidos no problema para o NOVO indivíduo gerado
			insereRNPLista(&popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpA); //Altera a lista que contém os alimentadores envolvidos no problema, para o indivíduo-PAI. Isto consiste numa modificação feita na heurística original.Ver arquivo com notas
			//imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
			numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
			  idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
			//salva os setores utilizados na manobra
			nosPRA = Malloc(NOSPRA, 1);
			nosPRA[0].a = noA;
			nosPRA[0].p = noP;
			nosPRA[0].r = noR;
			atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
					&idChaveAberta, &idChaveFechada, 1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, LRO);

			//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
/*			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--;*/

			//Nestas variáveis abaixo são acumuladas a potência transferida de um alimentador energizado para outro. Como neste caso, há reconexão de carga e não transferência, então elas são nulas
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade = 0;

			/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, principalmente, que seja calculada a potência ativa não suprida*/
			avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, -1, idNovaConfiguracaoParam,
					dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
					indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
					ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
					sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

			//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
			consumidoresManobrados = 0; //Como não é feito transferência de setores entre alimentadores,  o número de consumidores transferidos por este operador é nulo

			popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;


			nosPRA = NULL; free(nosPRA);
			return 1;
		}

    }
    else{
    	printf("\n A configuração selecionada não possui setores a serem reconectados! Escolha outro operador");
    	return -1;
    }

}


/*
 * Por Leandro:
 * DESCRIÇÃO: consiste na função "operadorLROHeuristicaV2()" modificada para:
 * (a) que no processo de escolha das três subárvores candidatas possam ser selecionados qualquer uma das RNPs Fictícias. Assim,
 * poderão ocorrer, por exemplo, duas subárvores de uma RNP fictícia e uma terceira de uma outra RNP.
 *
 * @param popConfiguracaoParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @param dadosAlimentadorParam
 * @param VFParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param estadoInicial
 * @param listaChavesParam
 * @param numChaveAberta
 * @param numChaveFechada
 * @param grafoSDRParam
 * @param nosPredefinidos
 * @param indicePParam
 * @param indiceAParam
 * @param rnpPParam
 * @param rnpAParam
 * @param sequenciaManobrasAlivioParam
 */
int operadorLROHeuristicaV4(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noAAux, noPAux, noRAux;
    int indiceL, indiceP, indiceA, indice,  indiceR, numeroTentativas = 0, numeroTrincasObtidas = 0, contadorTentativasSorteioTrinca = 0, tamanhoSubarvore, casoManobra, rnpP, rnpA, indiceAAux, indicePAux, indiceRAux, rnpAAux, rnpPAux, indiceLAux;
    long int noRaizP, idChaveAberta, idChaveFechada, numeroNos, consumidoresManobrados = 0;
    BOOL estadoInicialCA, estadoInicialCF;
    NOSPRA *nosPRA;
    NIVEISDEPRIORIDADEATENDIMENTO potenciaRestaurada, potenciaRestauradaAux;
    double potenciaRestauradaTotal;

	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.potenciaTotalNaoSuprida > 0){ //Testa se há cargas para serem restauradas, ie., se o operador LRO pode ser aplicado na configuração selecionada

		if(nosPreDefinidos){
			indiceP = indicePParam;
			rnpP = rnpPParam;
			noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indicePParam].idNo;

			indiceR = indiceRParam;
			noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpPParam].nos[indiceRParam].idNo;

			indiceA = indiceAParam;
			rnpA = rnpAParam;
			noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpAParam].nos[indiceAParam].idNo;

			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);//determinação do intervalo correspondente a subárvore do nó P na RNP

		}
		else{
			//Determinação dos nós p,e a, para aplicação do operador
			numeroTrincasObtidas = 0;
			contadorTentativasSorteioTrinca = 0;
			potenciaRestauradaAux.consumidoresPrioridadeAlta = -1;
			potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = -1;
			potenciaRestauradaAux.consumidoresPrioridadeBaixa = -1;
			potenciaRestauradaAux.consumidoresSemPrioridade = -1;
			noAAux = -1;
			noPAux = -1;
//			while(numeroTrincasObtidas < 3 && contadorTentativasSorteioTrinca < 6){
			while(numeroTrincasObtidas < 1 && contadorTentativasSorteioTrinca < 6){
				noA = -1;
				numeroTentativas = 0;
				rnpP = 0;
				while (noA == -1 && rnpP >= 0 && numeroTentativas < 100 ) { 	//Executa até obter um nó A Válido ou até atingir o numero máximo de tentativas de determinação de um nó A
					//Escolha de uma RNP Fictícia
					potenciaRestauradaTotal = 0;
					while(potenciaRestauradaTotal == 0){ //Essa RNP precisa ter cargas (potencia ativa) não suprida
						rnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia - 1));
						if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos > 1){
							potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[1].idNo, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
							potenciaRestauradaTotal = potenciaRestaurada.consumidoresPrioridadeAlta + potenciaRestaurada.consumidoresPrioridadeBaixa + potenciaRestaurada.consumidoresPrioridadeIntermediaria + potenciaRestaurada.consumidoresSemPrioridade;
						}
						else
							potenciaRestauradaTotal = 0;
					}

					//Escolha do nó de poda (nó P)
					potenciaRestauradaTotal = 0;
					while(potenciaRestauradaTotal == 0){
						indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos - 1));
						noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].idNo;
						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);
						potenciaRestauradaTotal = potenciaRestaurada.consumidoresPrioridadeAlta + potenciaRestaurada.consumidoresPrioridadeBaixa + potenciaRestaurada.consumidoresPrioridadeIntermediaria + potenciaRestaurada.consumidoresSemPrioridade;
					}

					//Escolha do nó do novo nó raiz (nó R)
					indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceP);
					if (indiceL == indiceP) //nesse caso não existe outro nó que possa ser o novo nó raiz, para religar a subárvore
						indiceR = indiceP;
					else
						indiceR = inteiroAleatorio(indiceP, indiceL);

					noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceR].idNo;

					colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP, grafoSetoresParam, matrizPiParam, vetorPiParam);
					if (colunaNoA != NULL && colunaNoA[0].idRNP < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP) { //se obteve um nó a válido recupera os dados dele.
						rnpA = colunaNoA[0].idRNP;
						indiceA = colunaNoA[0].posicao;
						noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;

						potenciaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noP, matrizB, matrizPiParam, vetorPiParam, grafoSDRParam);

						idChaveAberta = idChaveFechada = -999;
						localizaChavesOperadorLRO(&idChaveAberta, &idChaveFechada, indiceP, noP, noR, noA, rnpP, rnpA, popConfiguracaoParam, grafoSetoresParam, idConfiguracaoSelecionadaParam);
						if(idChaveAberta == -999 || idChaveFechada == -999)
							noA = -1;
					}
					numeroTentativas++;
				}

				if(noA != -1 && noP != noPAux){//Se uma nova trinca foi determinada
					if((floor(potenciaRestaurada.consumidoresPrioridadeAlta) >  floor(potenciaRestauradaAux.consumidoresPrioridadeAlta)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) >  floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) >  floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa)) ||
					   (floor(potenciaRestaurada.consumidoresPrioridadeAlta) == floor(potenciaRestauradaAux.consumidoresPrioridadeAlta) && floor(potenciaRestaurada.consumidoresPrioridadeIntermediaria) == floor(potenciaRestauradaAux.consumidoresPrioridadeIntermediaria) && floor(potenciaRestaurada.consumidoresPrioridadeBaixa) == floor(potenciaRestauradaAux.consumidoresPrioridadeBaixa) && floor(potenciaRestaurada.consumidoresSemPrioridade) > floor(potenciaRestauradaAux.consumidoresSemPrioridade))
					){
						noAAux = noA;
						noPAux = noP;
						noRAux = noR;
						indiceAAux = indiceA;
						indicePAux = indiceP;
						indiceRAux = indiceR;
						rnpAAux = rnpA;
						rnpPAux = rnpP;
						indiceLAux = indiceL;
						potenciaRestauradaAux.consumidoresPrioridadeAlta = potenciaRestaurada.consumidoresPrioridadeAlta;
						potenciaRestauradaAux.consumidoresPrioridadeIntermediaria = potenciaRestaurada.consumidoresPrioridadeIntermediaria;
						potenciaRestauradaAux.consumidoresPrioridadeBaixa = potenciaRestaurada.consumidoresPrioridadeBaixa;
						potenciaRestauradaAux.consumidoresSemPrioridade = potenciaRestaurada.consumidoresSemPrioridade;
					}
					numeroTrincasObtidas++;
				}
				contadorTentativasSorteioTrinca++;
			}
		}

		if(rnpP == -1 || numeroTrincasObtidas == 0){//Se rnpP for igual a -1, significa que todos os setores de todas as RNPs Fictícias foram verificados e nenhuma deles possui setor energizado adjacente
			return -1;
		}
		else{
			noA = noAAux;
			noP = noPAux;
			noR = noRAux;
			indiceA = indiceAAux;
			indiceP = indicePAux;
			indiceR = indiceRAux;
			rnpA = rnpAAux;
			rnpP = rnpPAux;
			indiceL = indiceLAux;

			//localiza qual chave será aberta
			if(indiceP > 1){
				indice = indiceP-1;
				while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceP].profundidade)
				{
					indice--;
				}
				noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indice].idNo;
				idChaveAberta = -999;
				for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
				{
					if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
						idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				}
				if(idChaveAberta == -999){
					printf("\n\n\nNão foi possível determinar o identificador da chave a ser aberta. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
					exit(1);
				}

			}
			else
				idChaveAberta = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

			//localiza qual chave será fechada
			idChaveFechada = -999;
			for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
			{
					if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
						idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
			}
			if(idChaveFechada == -999){
				printf("\n\n\nNão foi possível determinar o identificador da chave a ser fechada. Verifique a trinca de nós p, r, a escolhida (Operador LRO, Indivíduo %ld)", idNovaConfiguracaoParam);
				exit(1);
			}

			//printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
			//calcula o tamanho da subárvore
			tamanhoSubarvore = indiceL-indiceP+1;
			//faz a alocação das novas RNPs
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
			alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP]);
			//Obtém a nova rnp de origem, sem a subárvore podada
			constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP + popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP);
			//Obtém a nova rnp de destino, inserindo a subárvore podada
			constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);

		   //copia os ponteiros das rnps não alteradas
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
			{
				if (indice != rnpA)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
			}

			// Por Leandro: copia o ponteiro para a rnp FICTÍCIA que não foi alterada
			for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++ ){
				if (indice != rnpP)
					popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice];
			}
			popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia = popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia;

			/*//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--; */

			copiaListaRnps(popConfiguracaoParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam);
			insereRNPLista(&popConfiguracaoParam[idNovaConfiguracaoParam], rnpA); //Adiciona a árvore-destino à lista que contém os alimentadores envolvidos no problema para o NOVO indivíduo gerado
			insereRNPLista(&popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpA); //Altera a lista que contém os alimentadores envolvidos no problema, para o indivíduo-PAI. Isto consiste numa modificação feita na heurística original.Ver arquivo com notas
			//imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
			numeroManobrasModificada(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
			  idConfiguracaoSelecionadaParam, estadoInicial, &estadoInicialCA, &estadoInicialCF, idNovaConfiguracaoParam, &casoManobra);
			//salva os setores utilizados na manobra
			nosPRA = Malloc(NOSPRA, 1);
			nosPRA[0].a = noA;
			nosPRA[0].p = noP;
			nosPRA[0].r = noR;
			atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam,
					&idChaveAberta, &idChaveFechada, 1, casoManobra, &estadoInicialCA, &estadoInicialCF, nosPRA, LRO);

			//Se durante a geração do novo indivíduo, todos os setores da fictícia foram restaurados, então, decrementa-se o contador de RNP Fictícia do novo indivíduo
/*			if(popConfiguracaoParam[idNovaConfiguracaoParam].rnpFicticia[rnpP].numeroNos <=1)
				popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNPFicticia--;*/

			//Nestas variáveis abaixo são acumuladas a potência transferida de um alimentador energizado para outro. Como neste caso, há reconexão de carga e não transferência, então elas são nulas
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
			popConfiguracaoParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade = 0;

			/*Por Leandro: a função "avaliaConfiguracaoHeuristicaModificada()", permite, principalmente, que seja calculada a potência ativa não suprida*/
			avaliaConfiguracaoHeuristicaModificada(false, popConfiguracaoParam, rnpA, -1, idNovaConfiguracaoParam,
					dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP,
					indiceRegulador, dadosReguladorSDR, dadosAlimentadorParam, idConfiguracaoSelecionadaParam, matrizB,
					ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
					sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);

			//Calcula a quantidade de Consumidores Especiais transferidos (comparando a nova configuracao com a configuracao inicial - 0)
			consumidoresManobrados = 0; //Como não é feito transferência de setores entre alimentadores,  o número de consumidores transferidos por este operador é nulo

			popConfiguracaoParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos = popConfiguracaoParam[idConfiguracaoSelecionadaParam].objetivo.consumidoresEspeciaisTransferidos + consumidoresManobrados;


			nosPRA = NULL; free(nosPRA);
			return 1;
		}

    }
    else{
    	printf("\n A configuração selecionada não possui setores a serem reconectados! Escolha outro operador");
    	return -1;
    }

}


/* Por Leandro
 * Descrição:
 *
 * @param sobrecargaParam é o valor de sobrecarga (em VA) a ser eliminado por meio de corte de carga
 * @param indiceNoRaizParam é o indice nó apartir do qual o ocorrerão cortes. Se a sobrecarga for em tranformador, então este parâmentro deve ser 0. Caso a sobrecarga seja em red, então ele deve ser igual ao índice do nó no qual ocorreu a sobrecarga de rede
 * @param rnpPParam é o indice da RNP na qual deseja-se efetuar um corte
 * @param popConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param matrizBParam
 *
 * @return noP é o nó de poda. Se o seu valor for nulo significa que não foi possível determina uma subárvore para corte com potência a ser cortada menor que a sobrecarga. O valor da potência aparente cortada nunca é maior que a sobrecarga estipulada
 */
int determinaNoPodaPotenciaAparente(double sobrecargaParam, int indiceNoRaizParam, int rnpPParam, CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam)
{
	int indice, contador, prof, profAnt, flag, noPTemp, indicePTemp, noPAnterior, indicePAnterior, noP=0;
    struct NOS{ 	//Variável auxiliar temporária que armazenará os nós-folha do alimentador que será selecionado para corte
    	int *idNo;
    	int numeroNos;
    } nos;
    double potenciaEmCorte, maiorPotenciaEmCorte;

	nos.idNo = Malloc(int, popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].numeroNos);
	nos.numeroNos = 0;
	//Determina todos os nós-folha do alimentador (RNP) selecionado
	for(indice = indiceNoRaizParam + 1; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].numeroNos; indice++){
		profAnt = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice-1].profundidade;
		prof = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade;
		if(prof <= profAnt){
			nos.idNo[nos.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice-1].idNo;
			nos.numeroNos++;
		}
	}
	//Pelo procedimento acima, são obtidos todos os nós-folha de "rnpP" exceto aquele que ocupa a última posição no array. Mas aqui ele também é adicionado
	indice = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].numeroNos-1;
	nos.idNo[nos.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].idNo;
	nos.numeroNos++;

	//Percorre todos os nós folhas para determinar o nó de poda (noP)
	noP = 0;
	maiorPotenciaEmCorte = 0;
	for(contador = 0; contador < nos.numeroNos; contador++){
		//Seleciona o nó-folha na posição "contador" para ser candidato a ser "noP"
		noPTemp = nos.idNo[contador];
		//Determina o indice de "noP" no alimentador a que pertence na configuração "idConfiguracaoSelecionadaParam"
		indice = retornaColunaPi(matrizPiParam, vetorPiParam, noPTemp, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noP' na floresta 'idConfiguracaoSelecionadaParam'
		indicePTemp = matrizPiParam[noPTemp].colunas[indice].posicao;
		flag = 0;
		noPAnterior = 0;
		while(flag != 1 && indicePTemp > 0){ //A partir do "noP" atual percorre a RNP no sentido contrário ao fluxo a fim de determinar um nó que tenha o nó-folha selecionado e que corte uma quantidade de carga próximo àquela desejada
			//Verifica a existência de CEs
			if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam], grafoSetoresParam, indicePTemp)){
				flag = 1;
				noPTemp = 0;
			}
			else{
				//Determina a potência Aparente que ficará fora de serviço se "noP" atual vier a ser o nó de poda
				potenciaEmCorte =  calculaSomatorioPotenciaAparenteSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizBParam, matrizPiParam, vetorPiParam, grafoSDRParam);
				if(potenciaEmCorte >= sobrecargaParam){ //Veririfa se a quantidade de carga a ser cortada é menor que o valor limite para corte
					noPTemp = noPAnterior;
					indicePTemp = indicePAnterior;
					flag = 1;
				}
				else{//Se não houver CEs e "potenciaASerCortada" ainda for menor que o limite a ser cortado, então o noP é atualizado para ser um que corte uma quantidade maior de carga tendo o nó-folha previamente selecionado dentre aqueles que serão cortados
					noPAnterior = noPTemp;
					indicePAnterior = indicePTemp;
					//procurar o nó anterior ao noP atual para que este passe a ser o novo noP
					indice = indicePTemp-1;
					while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePTemp].profundidade)
					{
						indice--;
					}
					//atualiza o no de poda para que possa ser cortada uma quantidade maior de carga para alívio maior do tafo
					indicePTemp = indice;
					noPTemp = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePTemp].idNo;
				}
			}
		}
		// Verifica se foi obtido um nó de poda válido
		if(noPTemp!=0){
			potenciaEmCorte =  calculaSomatorioPotenciaAparenteSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizBParam, matrizPiParam, vetorPiParam, grafoSDRParam);
			if(potenciaEmCorte == 0) //Verifica se há cargas na subárvore a ser cortada
				noPTemp=0;
		}
		else
			potenciaEmCorte = 0;

		//Escolhe como noP aquele que, dentre os nós de poda temporário, cortará uma potência mais próxima daquela desejada
		if(potenciaEmCorte > maiorPotenciaEmCorte && noPTemp != 0){
			noP = noPTemp;
			//indicePParam = indicePTemp;
			maiorPotenciaEmCorte = potenciaEmCorte;
		}
	}

	free(nos.idNo);
	return(noP);
}

/* Por Leandro
 * Descrição:
 *
 * @param sobrecargaParam é o valor de sobrecarga (em VA) a ser eliminado por meio de corte de carga
 * @param indiceNoRaizParam é o indice nó apartir do qual o ocorrerão cortes. Se a sobrecarga for em tranformador, então este parâmentro deve ser 0. Caso a sobrecarga seja em red, então ele deve ser igual ao índice do nó no qual ocorreu a sobrecarga de rede
 * @param rnpPParam é o indice da RNP na qual deseja-se efetuar um corte
 * @param popConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param matrizBParam
 *
 * @return noP é o nó de poda. Se o seu valor for nulo significa que não foi possível determina uma subárvore para corte com potência a ser cortada menor que a sobrecarga. O valor da potência aparente cortada nunca é maior que a sobrecarga estipulada
 */
int determinaNoPodaCorrenteV2(double sobrecargaParam, int indiceNoRaizParam, int rnpPParam, CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *matrizBParam)
{
	int indice, contador, prof, profAnt, flag, noPTemp, indicePTemp, noPAnterior, indicePAnterior, noP=0, indiceL;
    struct NOSFOLHA{ 	//Variável auxiliar temporária que armazenará os nós-folha do alimentador que será selecionado para corte
		int *idNo;   //Nome do Nó
		int *indice; //Índice do No na RNP que o contém
    	int numeroNos; //Contador do número de nós-folha salvos
    } nosFolha;
    struct NOSCANDIDATOS{ 	//Variável auxiliar temporária que armazenará os nós candidatos a serem Nó de Poda
		int *idNo;   //Nome do Nó
		double *correnteCargaEmCorte; //Quantidade de Corrente de Carga a ser Cortada
		int *tamanhoSubarvoreEmCorte; //Número de nós da subárvore candidata a corte
    	int numeroNos;  //Contador do número de nós candidatos salvos
    } nosCandidatos;
    double correnteCargaEmCorte, maiorCorrenteCargaEmCorte;
    long int menorNumeroNosEmCorte;

    //Determinação do limite da subárvore que tem Nó raiz em "indiceNoRaizParam"
    indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam], indiceNoRaizParam);

    //Alocação e Inicialidação de Variáveis temporárias
    nosFolha.idNo = Malloc(int, (indiceL - indiceNoRaizParam));
    nosFolha.indice = Malloc(int, (indiceL - indiceNoRaizParam));
	nosFolha.numeroNos = 0;
	nosCandidatos.idNo = Malloc(int, (indiceL - indiceNoRaizParam));
	nosCandidatos.correnteCargaEmCorte = Malloc(double, (indiceL - indiceNoRaizParam));
	nosCandidatos.tamanhoSubarvoreEmCorte = Malloc(int, (indiceL - indiceNoRaizParam));
	nosCandidatos.numeroNos = 0;

	//Determina todos os nós-folha que estão na subárvore que tem como raiz o Nó raiz com índice "indiceNoRaizParam"
	for(indice = indiceNoRaizParam + 1; indice <= indiceL ; indice++){
		profAnt = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice-1].profundidade;
		prof = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade;
		if(prof <= profAnt){
			nosFolha.idNo[nosFolha.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice-1].idNo;
			nosFolha.indice[nosFolha.numeroNos] = indice-1;
			nosFolha.numeroNos++;
		}
	}
	//Pelo procedimento acima, são obtidos todos os nós-folha de "rnpP" exceto aquele que ocupa a última posição na subárvore que o nó raiz "indiceNoRaizParam". Mas aqui ele também é adicionado
	indice = indiceL; //popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].numeroNos-1;
	nosFolha.idNo[nosFolha.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].idNo;
	nosFolha.indice[nosFolha.numeroNos] = indice;
	nosFolha.numeroNos++;

	//Percorre todos os nós folhas para determinar o nó de poda (noP)
	noP = 0;
	maiorCorrenteCargaEmCorte = 0;
	for(contador = 0; contador < nosFolha.numeroNos; contador++){
		//Seleciona o nó-folha na posição "contador" para ser candidato a ser "noP"
		noPTemp = nosFolha.idNo[contador];
		//Determina o indice de "noP" no alimentador a que pertence na configuração "idConfiguracaoSelecionadaParam"
		/*indice = retornaColunaPi(matrizPiParam, vetorPiParam, noPTemp, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noP' na floresta 'idConfiguracaoSelecionadaParam'
		indicePTemp = matrizPiParam[noPTemp].colunas[indice].posicao;*/
		indicePTemp = nosFolha.indice[contador];
		flag = 0;
		noPAnterior = 0;
		while(flag != 1 && indicePTemp > 0){ //A partir do "noP" atual percorre a RNP no sentido contrário ao fluxo a fim de determinar um nó que tenha o nó-folha selecionado e que corte uma quantidade de carga próximo àquela desejada
			//Verifica a existência de CEs
			if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam], grafoSetoresParam, indicePTemp)){
				flag = 1;
				noPTemp = 0;
			}
			else{
				//Determina a potência Aparente que ficará fora de serviço se "noP" atual vier a ser o nó de poda
				correnteCargaEmCorte =  calculaSomatorioCorrenteCargaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizBParam, matrizPiParam, vetorPiParam);
				if(correnteCargaEmCorte >= sobrecargaParam){ //Veririfa se a quantidade de carga a ser cortada é menor que o valor limite para corte
					noPTemp = noPAnterior;
					indicePTemp = indicePAnterior;
					flag = 1;
				}
				else{//Se não houver CEs e "potenciaASerCortada" ainda for menor que o limite a ser cortado, então o noP é atualizado para ser um que corte uma quantidade maior de carga tendo o nó-folha previamente selecionado dentre aqueles que serão cortados
					noPAnterior = noPTemp;
					indicePAnterior = indicePTemp;
					//procurar o nó anterior ao noP atual para que este passe a ser o novo noP
					indice = indicePTemp-1;
					while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePTemp].profundidade)
					{
						indice--;
					}
					//atualiza o no de poda para que possa ser cortada uma quantidade maior de carga para alívio maior do tafo
					indicePTemp = indice;
					noPTemp = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePTemp].idNo;
				}
			}
		}
		// Verifica se foi obtido um nó de poda válido
		if(noPTemp!=0){
			correnteCargaEmCorte =  calculaSomatorioCorrenteCargaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizBParam, matrizPiParam, vetorPiParam);
			if(correnteCargaEmCorte == 0) //Verifica se há cargas na subárvore a ser cortada
				noPTemp=0;
		}
		else
			correnteCargaEmCorte = 0;

		//Salva na variável temporária o Nó atual candidato a ser Nó de Poda, se ele for válido
		if(noPTemp != 0){
			indice = nosCandidatos.numeroNos;
			nosCandidatos.idNo[indice] = noPTemp;
			nosCandidatos.correnteCargaEmCorte[indice] = correnteCargaEmCorte;
			nosCandidatos.tamanhoSubarvoreEmCorte[indice] = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam], indicePTemp) - indicePTemp + 1;
			nosCandidatos.numeroNos++;//Determina o nó que, dentre os nós de poda temporário, cortará uma potência maior, sem exceder a potência de corte desejada

			if(correnteCargaEmCorte > maiorCorrenteCargaEmCorte){
				//idNC = indice;
				maiorCorrenteCargaEmCorte = correnteCargaEmCorte;
			}
		}
	}

	//Determina o Nó de poda como sendo aquele que, dentre os nós candidatos com corrente de corte não inferior a 90% da Corrente de Sobrecarga, possui o menor número de nós sob corte,
	if(nosCandidatos.numeroNos > 0){
		menorNumeroNosEmCorte = 999999999999999;
		for(indice = 0; indice < nosCandidatos.numeroNos; indice ++){
			if(nosCandidatos.tamanhoSubarvoreEmCorte[indice] < menorNumeroNosEmCorte && (maiorCorrenteCargaEmCorte - nosCandidatos.correnteCargaEmCorte[indice]) <= maiorCorrenteCargaEmCorte*0.07){
				menorNumeroNosEmCorte = nosCandidatos.tamanhoSubarvoreEmCorte[indice];
				noP = nosCandidatos.idNo[indice];
			}
		}
	}

	free(nosFolha.idNo);
	free(nosFolha.indice);
	free(nosCandidatos.idNo);
	free(nosCandidatos.correnteCargaEmCorte);
	return(noP);
}


int determinaNoPodaCorrenteV1(double sobrecargaParam, int indiceNoRaizParam, int rnpPParam, CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *matrizBParam)
{
	int indice, contador, prof, profAnt, flag, noPTemp, indicePTemp, noPAnterior, indicePAnterior, noP=0, indiceL;
    struct NOSFOLHA{ 	//Variável auxiliar temporária que armazenará os nós-folha do alimentador que será selecionado para corte
		int *idNo;   //Nome do Nó
		int *indice; //Índice do No na RNP que o contém
    	int numeroNos;
    } nosFolha;

    double correnteCargaEmCorte, maiorCorrenteCargaEmCorte;
    //long int menorNumeroNosEmCorte;

    //Determinação do limite da subárvore que tem Nó raiz em "indiceNoRaizParam"
    indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam], indiceNoRaizParam);

    //Alocação e Inicialidação de Variáveis temporárias
    nosFolha.idNo = Malloc(int, (indiceL - indiceNoRaizParam));
    nosFolha.indice = Malloc(int, (indiceL - indiceNoRaizParam));
	nosFolha.numeroNos = 0;

	//Determina todos os nós-folha que estão na subárvore que tem como raiz o Nó raiz com índice "indiceNoRaizParam"
	for(indice = indiceNoRaizParam + 1; indice <= indiceL ; indice++){
		profAnt = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice-1].profundidade;
		prof = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade;
		if(prof <= profAnt){
			nosFolha.idNo[nosFolha.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice-1].idNo;
			nosFolha.indice[nosFolha.numeroNos] = indice-1;
			nosFolha.numeroNos++;
		}
	}
	//Pelo procedimento acima, são obtidos todos os nós-folha de "rnpP" exceto aquele que ocupa a última posição na subárvore que o nó raiz "indiceNoRaizParam". Mas aqui ele também é adicionado
	indice = indiceL; //popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].numeroNos-1;
	nosFolha.idNo[nosFolha.numeroNos] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].idNo;
	nosFolha.indice[nosFolha.numeroNos] = indice;
	nosFolha.numeroNos++;

	//Percorre todos os nós folhas para determinar o nó de poda (noP)
	noP = 0;
	maiorCorrenteCargaEmCorte = 0;
	for(contador = 0; contador < nosFolha.numeroNos; contador++){
		//Seleciona o nó-folha na posição "contador" para ser candidato a ser "noP"
		noPTemp = nosFolha.idNo[contador];
		//Determina o indice de "noP" no alimentador a que pertence na configuração "idConfiguracaoSelecionadaParam"
		/*indice = retornaColunaPi(matrizPiParam, vetorPiParam, noPTemp, idConfiguracaoSelecionadaParam); //indice da coluna da matriz PI que contém o 'noP' na floresta 'idConfiguracaoSelecionadaParam'
		indicePTemp = matrizPiParam[noPTemp].colunas[indice].posicao;*/
		indicePTemp = nosFolha.indice[contador];
		flag = 0;
		noPAnterior = 0;
		while(flag != 1 && indicePTemp > 0){ //A partir do "noP" atual percorre a RNP no sentido contrário ao fluxo a fim de determinar um nó que tenha o nó-folha selecionado e que corte uma quantidade de carga próximo àquela desejada
			//Verifica a existência de CEs
			if(verificaExistenciaConsumidoresEspeciais(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam], grafoSetoresParam, indicePTemp)){
				flag = 1;
				noPTemp = 0;
			}
			else{
				//Determina a potência Aparente que ficará fora de serviço se "noP" atual vier a ser o nó de poda
				correnteCargaEmCorte =  calculaSomatorioCorrenteCargaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizBParam, matrizPiParam, vetorPiParam);
				if(correnteCargaEmCorte >= sobrecargaParam){ //Veririfa se a quantidade de carga a ser cortada é menor que o valor limite para corte
					noPTemp = noPAnterior;
					indicePTemp = indicePAnterior;
					flag = 1;
				}
				else{//Se não houver CEs e "potenciaASerCortada" ainda for menor que o limite a ser cortado, então o noP é atualizado para ser um que corte uma quantidade maior de carga tendo o nó-folha previamente selecionado dentre aqueles que serão cortados
					noPAnterior = noPTemp;
					indicePAnterior = indicePTemp;
					//procurar o nó anterior ao noP atual para que este passe a ser o novo noP
					indice = indicePTemp-1;
					while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePTemp].profundidade)
					{
						indice--;
					}
					//atualiza o no de poda para que possa ser cortada uma quantidade maior de carga para alívio maior do tafo
					indicePTemp = indice;
					noPTemp = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpPParam].nos[indicePTemp].idNo;
				}
			}
		}
		// Verifica se foi obtido um nó de poda válido
		if(noPTemp!=0){
			correnteCargaEmCorte =  calculaSomatorioCorrenteCargaSubarvore(popConfiguracaoParam, idConfiguracaoSelecionadaParam, noPTemp, matrizBParam, matrizPiParam, vetorPiParam);
			if(correnteCargaEmCorte == 0) //Verifica se há cargas na subárvore a ser cortada
				noPTemp=0;
		}
		else
			correnteCargaEmCorte = 0;

		//Determina o nó que, dentre os nós de poda temporário, cortará uma potência maior, sem exceder a potência de corte desejada
		if(correnteCargaEmCorte > maiorCorrenteCargaEmCorte && noPTemp != 0){
			noP = noPTemp;
			//indicePParam = indicePTemp;
			maiorCorrenteCargaEmCorte = correnteCargaEmCorte;
		}

	}

	free(nosFolha.idNo);
	free(nosFolha.indice);
	return(noP);
}
