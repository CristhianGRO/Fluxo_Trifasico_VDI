/**
 * @file aemt.h
 * @brief Este arquivo contém os protótipos das funções implementadas no arquivo aemt.c.
 * 
 */
#ifndef AEMT_H
#define	AEMT_H

#include "data_structures.h"
#include "funcoesRNP.h"

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif


void alocaTabelas(int numeroTabelasParam, int *numeroConfiguracoesParam, VETORTABELA **populacaoParam);

void imprimeTabelas(int numeroTabelasParam, VETORTABELA *populacaoParam);

void imprimeTabela(int idTabelaParam, VETORTABELA *populacaoParam);
void imprimeConfiguracoesTabela(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam);
int retornaPiorConfiguracaoTabela(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, BOOL maximizar);
void insereConfiguracaoTabelaModificada(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, double objetivoParam,
        CONFIGURACAO *configuracoesParam, BOOL maximizar, BOOL *torneioEntrou);
long int retornaMelhorConfiguracaoTabela(int idTabelaParam, VETORTABELA *populacaoParam, BOOL maximizar);

long int selecionaConfiguracao(VETORTABELA *populacaoParam, int numeroTabelasParam);

void insereTabelasModificada(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao, long int idPrimeiraConfiguracaoParam,
		int numeroTotalManobrasRestauracaoCompletaSemAlivioParam, int *sucessoAplicacaoOperadorParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, BOOL flagManobrasEmParParam); //Por Leandro
void insereTabelasModificadaV2(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao, long int idPrimeiraConfiguracaoParam, int numeroTotalManobrasRestauracaoCompletaSemAlivioParam,
		SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, BOOL flagManobrasAlivioParam, VETORPI *vetorPiParam, BOOL *flagAtualizacaoPopulacaoParam);//Por Leandro

void insereConfiguracaoTabelaManobras(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, 
        long int numeroMinimoManobrasManuais, int *torneioQtdInclusoesManobras);
void insereConfiguracaoTabelaManobrasModificada(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
        long int numeroMinimoManobrasManuais, int *torneioQtdInclusoesManobras); // Leandro
void insereConfiguracaoTabelaManobrasModificada(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
        long int numeroMinimoManobrasManuais, int *torneioQtdInclusoesManobras);
void insereConfiguracaoTabelaSolucoesFactiveis(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, int *torneioQtdInclusoesManobras);
/*void processoEvolutivo(DADOSTRAFO *dadosTrafoParam,
        DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, GRAFOSETORES *grafoSetoresParam,
        RNPSETORES *matrizSetoresParam, LISTACHAVES *listaChavesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, MATRIZMAXCORRENTE *maximoCorrenteParam, MATRIZCOMPLEXA *ZParam,
        double VFParam, long int numeroBarras, long int numeroSetores, int numeroTrafos, ESTADOCHAVE *estadoInicial,
        long int maximoGeracoes, int numeroTabelas,
        double taxaOperadorPAO, long int idConfiguracao, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, long int *numChaveAberta,
        long int *numChaveFechada, int numeroIndividuos);*/
void insereConfiguracaoTabelaManobrasFactiveis(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam);
void insereTabelasFactiveis(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao);
long int selecionaConfiguracaoTorneioTabelas(VETORTABELA *populacaoParam, int numeroTabelasParam, int *torneioTabelaSelecionada);
double funcaoDistanciaFactivel(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam);
void insereConfiguracaoTabelaConsumidoresEspeciais(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, 
        long int idConfiguracaoParam, int *torneioQtdInclusoesManobras);
double ajustarTaxaDeProbabilidadeAplicacaoOperadores(double probabilidadeAplicacaoParam, double taxaOperadorPAOParam, int sucessoAplicacaoOperadorParam);
#endif	/* AEMT_H */

