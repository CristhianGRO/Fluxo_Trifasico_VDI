/* 
 * File:   funcoesChaveamento.h
 * Author: telma
 *
 * Created on 25 de Janeiro de 2015, 16:43
 */
/**
 * @file funcoesChaveamento.h
 * @brief Este arquivo contém os protótipos das funções implementadas no arquivo funcoesChaveamento.c.
 * 
 */
#ifndef FUNCOESCHAVEAMENTO_H
#define	FUNCOESCHAVEAMENTO_H
#include "data_structures.h"
#include "funcoesProblema.h"
#include "funcoesRNP.h"
#include "funcoesSetor.h"
#include "funcoesInicializacao.h"


#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

void tiposChave(long int codChave, LISTACHAVES *listaChavesParam, char *tipoChave);

VETORPI * removeChavesEstadoInicial(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta,  LISTACHAVES *listaChavesParam);
VETORPI *removeChavesEstadoInicialModificada(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,
		  long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam);//Por Leandro

void sequenciaChaveamento(GRAFO *grafoSDRParam, long int numeroBarras, DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam, 
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam, 
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, LISTACHAVES *listaChavesParam, int seed, long int numeroChavesParam);
long int buscaChaveAlimentador(RNP rnpParam, GRAFOSETORES *grafoSetoresParam);
//funções para manipulação de lista encadeada
void inicializacao(LISTA *lista);
void insercaoListaVazia(LISTA * lista, ELEMENTO *elem);
void insercaoElemFim (LISTA *lista, ELEMENTO *elem, ELEMENTO *elemAnt);
void destruir(LISTA *lista);
void contaManobrasChaveamento(CONFIGURACAO *configuracao, ELEMENTO *elem, LISTACHAVES *listaChavesParam);

void avaliaConfiguracaoChaveamento(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, GRAFO *grafoSDRParam, int SBase);

BOOL verificaOperadorRNP(int indiceP, int indiceR, int indiceA, int indiceL, int rnpP, int rnpR, int rnpA);
void insereManobraChaveamento(ELEMENTO *chaveamento, long int idChave, BOOL temReguladorTensao, ESTADOCHAVE estado);
BOOL verificaPresencaRT(int indiceNo,  RNP vetorRNP, RNPSETORES *matrizB, int *indiceRegulador);
long int *recuperaVetorPiMultiplasFaltas(VETORPI *vetorPiParam, int numeroSetoresFalta,
        int numeroConfiguracoes, long int *configuracoes, int *numeroConfiguracoesFalta);
long int buscaChaveSetor(long int setor1, long int setor2, GRAFOSETORES *grafoSetoresParam);
long int consumidoresDesligados(CONFIGURACAO configuracaoParam, int indiceRNP,
        GRAFOSETORES *grafoSetoresParam, long int *setoresFalta,
        int numeroSetoresFalta);
long int buscaChaveDesligaTrecho(long int indiceSetor, RNP rnpParam, GRAFOSETORES *grafoSetores, LISTACHAVES *listaChavesParam);
void gravaChavesCompleto(long int numeroChavesParam, char *nomeArquivo, LISTACHAVES *listaChavesParam);

#endif	/* FUNCOESCHAVEAMENTO_H */

