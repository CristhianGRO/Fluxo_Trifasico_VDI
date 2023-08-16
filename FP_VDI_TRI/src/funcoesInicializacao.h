/** 
 * @file funcoesInicializacao.h
 * @brief Este arquivo contém os protótipos das funções implementadas no arquivo funcoesInicializacao.c.
 * Author: sony
 *
 * Created on 10 de Novembro de 2013, 11:30
 */

#ifndef FUNCOESINICIALIZACAO_H
#define	FUNCOESINICIALIZACAO_H
#include "funcoesRNP.h"

extern long int numeroSetores; /**< Variável global para armazenar o número de setores do SDR. */
extern long int numeroAlimentadores; /**< Variável global para armazenar o número de alimentadores do SDR. */


void constroiListaChaves(GRAFO *grafoSDRParam, LISTACHAVES **listaChavesParam, ESTADOCHAVE *estadoInicial, ESTADOCHAVE *estadoAutomaticas, ESTADOCHAVE *estadoRestabelecimento, long int numeroChaves, long int numeroBarras);


int buscaChave(LISTACHAVES *listaChavesParam, int idNo1, int idNo2, long int numeroChaves);

void buscaProfundidadeSetores(int idSetorParam, BOOL *visitadoParam,RNP *rnpParam, int profundidade, int *indice, GRAFOSETORES *grafoSetoresParam, LISTACHAVES *listaChavesParam);

//void constroiIndividuoInicial(GRAFOSETORES *grafoSetoresParam, GRAFO *grafoSDRParam, LISTACHAVES *listaChavesParam, DADOSALIMENTADOR *alimentadoresParam, CONFIGURACAO *populacaoParam);
void imprimeDadosEletricos(CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam) ;
#ifdef	__cplusplus
extern "C" {
#endif

void imprimeListaChaves(LISTACHAVES *listaChavesParam, long int numeroChaves);
//void inicializaDadosEletricosPorAlimentador(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB);

//void inicializaDadosEletricosChaveamento(RNPSETORES *matrizB, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam);

void alimentadoresPorTrafo(DADOSTRAFO *dadosTrafoParam, long int numeroTrafosParam, DADOSALIMENTADOR *dadosAlimentadorParam, long int numeroAlimentadoresParam); //Por Leandro

//CRISTHIAN: FUNÇÕES TRIFASICAS ============================================================================================

void inicializaDadosEletricosPorAlimentadorTrifasico(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB);

#ifdef	__cplusplus
}
#endif

#endif	/* FUNCOESINICIALIZACAO_H */

