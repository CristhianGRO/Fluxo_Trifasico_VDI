/**
 *@file funcoesHeuristica.h
 *@brief Protótipos das funções implementadas no arquivo funcoesHeuristicas.c
 * Author: inf
 *
 * Created on August 22, 2014, 1:40 PM
 */

#ifndef FUNCOESHEURISTICA_H
#define	FUNCOESHEURISTICA_H


void operadorPAOHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam, 
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam, 
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam, 
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB, 
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, 
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, 
        long int *numChaveFechada);

void operadorCAOHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam, 
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam, 
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam, 
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB, 
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam,
        long int *numChaveAberta, long int *numChaveFechada);

void avaliaConfiguracaoHeuristica(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam, 
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, 
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, 
        DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam, 
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos) ;

void reconfiguradorHeuristicaModificada(GRAFO *grafoSDRParam, long int numeroBarras, DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos,
        DADOSALIMENTADOR *dadosAlimentadorSDRParam, long int numeroAlimentadores, DADOSREGULADOR *dadosReguladorSDR,
        long int numeroReguladores, RNPSETORES *rnpSetoresParam, long int numeroSetores, GRAFOSETORES *grafoSetoresParam,
        LISTACHAVES *listaChavesParam, long int numeroChaves, CONFIGURACAO *configuracaoInicial, MATRIZMAXCORRENTE *maximoCorrente,
        MATRIZCOMPLEXA *Z, ESTADOCHAVE *estadoInicial, int seed);

void insereRNPLista(CONFIGURACAO *configuracaoParam, int rnp);

void copiaListaRnps(CONFIGURACAO *configuracoesParam, long int idConfiguracaoNova, long int idConfiguracaoAntiga);

void processoEvolutivoHeuristicaModificada(DADOSTRAFO *dadosTrafoParam,
        DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO **configuracoesParam, VETORTABELA *populacaoParam, GRAFOSETORES *grafoSetoresParam,
        RNPSETORES *matrizSetoresParam, LISTACHAVES *listaChavesParam, MATRIZPI *matrizPiParam,
        VETORPI **vetorPiParam, MATRIZMAXCORRENTE *maximoCorrenteParam, MATRIZCOMPLEXA *ZParam,
        long int numeroBarras, int numeroTrafos, ESTADOCHAVE *estadoInicial,
        long int maximoGeracoes, int numeroTabelas,double taxaOperadorPAOParam,
        long int idConfiguracao, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, 
        long int *numChaveAberta, long int *numChaveFechada, long int maximoGeracoesSemAtualizacaoPopulacaoParam, int numeroTotalManobrasRestauracaoCompletaSemAlivioParam,
		GRAFO *grafoSDRParam, long int idPrimeiraConfiguracaoParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam,
		long int *numeroPosicoesAlocadasParam, long int numeroSetoresParam, int numeroSetorFaltaParam, long int *numeroConfiguracoesGeradasParam, long int *geracoesExecutadas); //Por Leandro

void avaliaConfiguracaoHeuristicaModificada(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam,
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador,
        DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos,
		GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam,
		LISTACHAVES *listaChavesParam, VETORPI *vetorPiParam, BOOL flagManobrasEmParParam); //Por Leandro

void carregamentoTrafoHeuristica(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, 
        DADOSALIMENTADOR *dadosAlimentadorParam, 
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam, 
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino);

void carregamentoTrafoHeuristicaModificada(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam,
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino);

void isolaRestabeleceTodasOpcoesHeuristica(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam);

void isolaRestabeleceMultiplasFaltasHeuristica(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int numeroFaltasParam, long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam);
void isolaRestabeleceTodasOpcoesHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int setorFaltaParam,
		ESTADOCHAVE *estadoInicial, long int * numChaveAberta, long int *numChaveFechada, long int *idNovaConfiguracaoParam,
		LISTACHAVES *listaChavesParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam); //Por Leandro
void isolaRestabeleceMultiplasFaltasHeuristicaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int numeroFaltasParam,
		long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta, long int *numChaveFechada, long int *idNovaConfiguracaoParam,
		long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam); //por Leandro

void classificaSetoresRNP2(CONFIGURACAO *configuracao, long int qtdSetores, GRAFOSETORES *grafoSetores,
        LISTACHAVES *listaChaves, int setorInicioSetoresFicticios);
int operadorPAOHeuristicaMelhorada(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam);
int operadorCAOHeuristicaMelhorada(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam,
        long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam);
int operadorLSOHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam);
int operadorLSOHeuristicaV2(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta,
        long int *numChaveFechada, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam);
int operadorLROHeuristica(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam); //Por Leandro
int operadorLROHeuristicaV2(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam);//Por Leandro
int operadorLROHeuristicaV3(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam);//Por Leandro
int operadorLROHeuristicaV4(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam,
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR, DADOSALIMENTADOR *dadosAlimentadorParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam, long int *numChaveAberta, long int *numChaveFechada, GRAFO *grafoSDRParam,
		BOOL nosPreDefinidos, int indicePParam, int indiceRParam, int indiceAParam, int rnpPParam, int rnpAParam,  SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam);//Por Leandro
int determinaNoPodaPotenciaAparente(double sobrecargaParam, int indiceNoRaizParam, int rnpPParam, CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *matrizBParam, GRAFO *grafoSDRParam);
int determinaNoPodaCorrenteV2(double sobrecargaParam, int indiceNoRaizParam, int rnpPParam, CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *matrizBParam);
int determinaNoPodaCorrenteV1(double sobrecargaParam, int indiceNoRaizParam, int rnpPParam, CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *matrizBParam);
long int determinaNumeroSubarvoresDesligadasReconectaveis(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
       CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam, long int setorFaltaParam); //Por Leandro
long int obtemNumeroSubarvoresDesligadas(GRAFOSETORES *grafoSetoresParam, int numeroSetorFaltaParam, long int *setoresFaltaParam, int numeroPosicoesAlocadasParam, CONFIGURACAO *configuracaoInicial, long int idConfiguracaoParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam) ; // Por Leandro
#endif	/* FUNCOESHEURISTICA_H */

