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
void salvaChaveamento(ELEMENTO *chaveamento, char *nomeArquivo, LISTACHAVES *listaChavesParam);

void fluxoCargaTodosAlimentadoresChaveamento(long int numeroBarrasParam,
        DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam);

void imprimeVetorPi(VETORPI *vetorPi, int idConfiguracao, LISTACHAVES *listaChavesParam);

VETORPI * removeChavesEstadoInicial(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta,  LISTACHAVES *listaChavesParam);
VETORPI *removeChavesEstadoInicialModificada(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,
		  long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam);//Por Leandro

BOOL removeChaveEstadoInicialCaso2(int idConfiguracaoParam, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav, CONFIGURACAO *configuracoesChav, int *idProxConfiguracao, int totalConfiguracoes, int *idNovoAncestral);
BOOL removeChaveEstadoInicialCaso3(VETORPI *vetorPiChav, int idConfiguracaoParam, MATRIZPI *matrizPIChav, CONFIGURACAO *configuracoesChav, int *idProxConfiguracao, int totalConfiguracoes, int *idNovoAncestral);

long int *recuperaConfiguracoesIntermediarias(VETORPI *vetorPiParam, long int idConfiguracaoParam, int *totalConfiguracoes);
BOOL simulaChaveamentoPosicaoInicial(int *indiceConfiguracoes, int contador, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav,
        int *idProxConfiguracao, CONFIGURACAO *configuracoesChav, long int noPNovo, long int noANovo, long int noRNovo,
        int posicaoChave, int posicaoConfiguracao, int idConfiguracaoAncestral, long int idChaveAbertaR, 
        long int idChaveFechadaR, int *idNovoAncestral, int idConfiguracaoIni);

BOOL simulaChaveamentoPosicaoFinal(int *indiceConfiguracoes, int contador, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav,
        int *idProxConfiguracao, CONFIGURACAO *configuracoesChav, long int noPNovo, long int noANovo, long int noRNovo,
        int posicaoChave, int posicaoConfiguracao, int idConfiguracaoAncestral, long int idChaveAbertaR, 
        long int idChaveFechadaR, int *idNovoAncestral, int idConfiguracaoIni);
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

void otimizaChaveamento(DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, CONFIGURACAO configuracaoInicial, VETORPI *vetorPiChav, long int idConfiguracaoParam,
        GRAFO *grafoSDRParam, long int numeroBarras, LISTACHAVES *listaChavesParam, int numeroManobras, int consumidores, int consumidoresEspeciais, int seed,
        long int indiceAnterior, CONFIGURACAO *configuracoesParam, long int numeroChavesParam);
void otimizaChaveamentoModificada(DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, CONFIGURACAO configuracaoInicial, VETORPI *vetorPiChav, long int idConfiguracaoParam,
        GRAFO *grafoSDRParam, long int numeroBarras, LISTACHAVES *listaChavesParam, int numeroManobras, int consumidores, int consumidoresEspeciais, int seed,
        long int indiceAnterior, CONFIGURACAO *configuracoesParam, long int numeroChavesParam);//Por Leandro
void otimizaChaveamentoCorteCarga(DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, CONFIGURACAO configuracaoInicial, VETORPI *vetorPiChav, long int idConfiguracaoParam,
        GRAFO *grafoSDRParam, long int numeroBarras, LISTACHAVES *listaChavesParam, int numeroManobras, int consumidores, int consumidoresEspeciais, int seed,
        long int indiceAnterior, CONFIGURACAO *configuracoesParam, long int numeroChavesParam);
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
void verificaSequenciaChaveamento(GRAFO *grafoSDRParam, long int numeroBarras, DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, GRAFOSETORES *grafoSetoresParam, long int numeroSetoresParam,
		int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador,
		VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, long int *idConfiguracaoParam, LISTACHAVES *listaChavesParam,
		ESTADOCHAVE *estadoInicial, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, long int *numeroPosicoesAlocadasParam, long int idPrimeiraConfiguracaoParam, BOOL *sequenciaChaveamentoFactivelParam); //Por Leandro
#endif	/* FUNCOESCHAVEAMENTO_H */

