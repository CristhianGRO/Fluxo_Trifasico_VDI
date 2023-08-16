/**
 * @file funcoesTrifasicas.h
 * @brief Este arquivo contém os protótipos das funções trifásicas implementadas no arquivo funcoesTrifasicas.c
 *
 */
#ifndef FUNCOESTRIFASICAS_H
#define	FUNCOESTRIFASICAS_H

#include "data_structures.h"

void leituraBarrasSimplicadoModificadaTrifasico(GRAFO **grafoSDRParam, long int *numeroBarras);

void leituraBarrasTrifasico(GRAFO **grafoSDRParam, long int *numeroBarras);

void leituraTrafosTrifasicos(DADOSTRAFO **dadosTrafoSDRParam, long int *numeroTrafos);

void leituraMatrizImpedanciaCorrenteTrifasicos(MATRIZCOMPLEXA **ZParam,  MATRIZMAXCORRENTE **maximoCorrenteParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentador, long int numeroAlimentadores);

void inicializaDadosEletricosPorAlimentadorTrifasico(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB);

void fluxoCargaAlimentadorTrifasico(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, int maximoIteracoes);

void verificaPresencaDeFaseNo(int* presencaDeFases, TIPOFASES tipoFases);

void correnteJusanteTrifasico(int idRNP, int carga, int iteracao, CONFIGURACAO configuracaoParam, RNPSETORES *matrizB, int *indiceRegulador,  DADOSREGULADOR *dadosRegulador);

void tensaoQuedaTensaoModificadaTrifasico(long int* pior, int indiceRNP, CONFIGURACAO configuracaoParam, double VFParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR);

void carregamentoPerdasModificadaTrifasico(CONFIGURACAO *configuracaoParam, int indiceRNP, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam);

CONFIGURACAO* alocaIndividuoModificadaTrifasico(int numeroRNPParam,  int numeroRNPFicticiaParam, long int idIndividuoInicialParam,
		long int numeroConfiguracoesParam, int numeroTrafosParam, int numeroSetoresParam);
CONFIGURACAO* alocaIndividuoTrifasico(int numeroRNPParam, long int idIndividuoInicialParam, long int numeroConfiguracoesParam, int numeroTrafosParam);
void valorZTrifasico(MATRIZCOMPLEXA *ZParam, long int idNo1, long int idNo2, __complex__ double* Zno, int fase);
void desalocacaoCompletaConfiguracaoModificadaTrifasico(CONFIGURACAO **configuracoesParam, long int numeroPosicoesAlocadasParam);
void desalocaConfiguracaoModificadaTrifasico(CONFIGURACAO configuracaoParam);
void imprimeDadosEletricosTrifasicos(CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam);
void imprimeDadosEletricosTrifasicos_POLAR(CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam);
#endif /* FUNCOESTRIFASICAS_H */