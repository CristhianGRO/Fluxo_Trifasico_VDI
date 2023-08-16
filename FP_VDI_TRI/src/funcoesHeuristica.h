/**
 *@file funcoesHeuristica.h
 *@brief Protótipos das funções implementadas no arquivo funcoesHeuristicas.c
 * Author: inf
 *
 * Created on August 22, 2014, 1:40 PM
 */

#ifndef FUNCOESHEURISTICA_H
#define	FUNCOESHEURISTICA_H


void reconfiguradorHeuristicaModificada(GRAFO *grafoSDRParam, long int numeroBarras, DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos,
        DADOSALIMENTADOR *dadosAlimentadorSDRParam, long int numeroAlimentadores, DADOSREGULADOR *dadosReguladorSDR,
        long int numeroReguladores, RNPSETORES *rnpSetoresParam, long int numeroSetores, GRAFOSETORES *grafoSetoresParam,
        LISTACHAVES *listaChavesParam, long int numeroChaves, CONFIGURACAO *configuracaoInicial, MATRIZMAXCORRENTE *maximoCorrente,
        MATRIZCOMPLEXA *Z, ESTADOCHAVE *estadoInicial, int seed);

void insereRNPLista(CONFIGURACAO *configuracaoParam, int rnp);
long int obtemNumeroSubarvoresDesligadas(GRAFOSETORES *grafoSetoresParam, int numeroSetorFaltaParam, long int *setoresFaltaParam, int numeroPosicoesAlocadasParam, CONFIGURACAO *configuracaoInicial, long int idConfiguracaoParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam);
long int determinaNumeroSubarvoresDesligadasReconectaveis(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
       CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam, long int setorFaltaParam);
       
#endif	/* FUNCOESHEURISTICA_H */

