/**
 * @file funcoesLeituraDados.h
 * @author Telma Woerle de Lima Soares
 * @date 12 Sep 2013
 * @brief Contém a descrição das funções utilizadas para a leitura dos dados no novo formato.
 *
 */

#ifndef FUNCOESLEITURADADOS_H
#define	FUNCOESLEITURADADOS_H
#include "data_structures.h"
BOOL leituraBarrasLinhasTrafos(GRAFO **grafoSDRParam, DADOSTRAFO **dadosTrafoSDRParam, long int *numeroBarras, long int *numeroTrafos, long int *numeroChaves);
void leituraBarras(FILE *arquivo, GRAFO **grafoSDRParam, long int *numeroBarras);
void leituraLinhas(FILE *arquivo, GRAFO *grafoSDRParam, long int *numeroChaves);

//void leituraTrafos(FILE *arquivo, DADOSTRAFO **dadosTrafoSDRParam, long int *numeroTrafos);
void leituraTrafos(DADOSTRAFO **dadosTrafoSDRParam, long int *numeroTrafos);
BOOL leituraDadosAlimentadores(DADOSALIMENTADOR **dadosAlimentadorSDRParam);
BOOL leituraDadosReguladoresTensao(DADOSREGULADOR **dadosReguladorSDRParam);
void imprimeGrafo(GRAFO *grafoSDRParam, long int numeroBarras);

void imprimeTrafos(DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos);

void leituraParametros(long int *numeroGeracoes, int *tamanhoTabelas, int *SBase, int *tamanhoPopulacaoInicial);
void leituraParametrosModificada(long int *numeroGeracoes, int *tamanhoTabelas, int *SBase, long int *maximoGeracoesSemAtualizacaoPopulacao, SEQUENCIAMANOBRASALIVIO *sequenciaManobrasTranferenciaCargaParam, int *numeroDesejadoSolucoesFinaisParam); //Por Leandro
void leituraDadosEntrada(int *numeroSetorFalta, long int **setoresFalta);
void leituraListaAdjacenciaSetores(GRAFOSETORES **grafoSetoresParam, long int *numeroSetores);
void leituraRNPSetores(RNPSETORES **rnpSetores, long int numeroSetores);
void leituraListaChaves(LISTACHAVES **listaChavesParam, long int *numeroChaves, ESTADOCHAVE **estadoInicial);
void leituraSetorBarra(GRAFO *grafoSDRParam);
void leituraBarrasSimplicado(GRAFO **grafoSDRParam, long int *numeroBarras);
void leituraBarrasSimplicadoModificada(GRAFO **grafoSDRParam, long int *numeroBarras);
void leituraBarrasFasesConectadas(GRAFO **grafoSDRParam, long int *numeroBarras);
void leituraMatrizImpedanciaCorrente(MATRIZCOMPLEXA **ZParam,  MATRIZMAXCORRENTE **maximoCorrenteParam, long int numeroBarrasParam, 
        DADOSALIMENTADOR *dadosAlimentador, long int numeroAlimentadores);
void leituraVetorTaps(int *tapsParam, DADOSREGULADOR *dadosReguladorParam, long int numeroReguladoresParam, long int numeroBarrasParam);
void leituraIndividuoInicial(long int numeroSetores, long int numeroAlimentadores, CONFIGURACAO *individuo);
void leituraSBase(int *SBase);
#endif	/* FUNCOESLEITURADADOS_H */

