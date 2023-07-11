/**
 * @file   funcoesRNP.h
 * @brief Contém os protótipos da funções implementadas no arquivo funcoesRNP.c.
 * Created on 10 de Novembro de 2013, 12:10
 */

#ifndef FUNCOESRNP_H
#define	FUNCOESRNP_H
#include "data_structures.h"

#ifdef	__cplusplus
extern "C" {
#endif



#ifdef	__cplusplus
}
#endif

void insereNos(NOSPRA *conjuntoNosParam, long int p, long int r, long int a, int iadj, int pos);
void atualizaVetorPi(VETORPI *piParam, long int idIndividuoParam, long int idAncestralParam, long int *idChaveAbertaParam, 
        long int *idChaveFechadaParam, int numeroManobras, int casoManobra, BOOL *estadoInicialCA, BOOL *estadoInicialCF, NOSPRA *conjuntoNosParam);
void atualizaVetorPiModificada(VETORPI *piParam, long int idIndividuoParam, long int idAncestralParam,
        long int *idChaveAbertaParam, long int *idChaveFechadaParam, int numeroManobras,
        int casoManobra, BOOL *estadoInicialCA,BOOL *estadoInicialCF, NOSPRA *conjuntoNosParam, OPERADOR operadorParam); //Por Leandro
COLUNAPI * determinaNoAdjacente(int tentativasParam, long int idNoParam, CONFIGURACAO florestaParam, int idRNPParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam); 
int buscaBinariaColunaPI(MATRIZPI *matrizPIParam, int indiceNoParam, int valorBusca, int numeroColunasParam);
int buscaBinariaMatrizPI(MATRIZPI *matrizPIParam, int parametroBusca, int ultimaPosicaoParam);
int retornaColunaPi(MATRIZPI *matrizPIParam, VETORPI *vetorPiParam, int idNoParam, int idIndividuoParam);
void adicionaColuna(MATRIZPI *matrizPIParam, int idNoParam, int idIndividuoParam, int idRNPParam, int indiceParam);
void adicionaColunaMelhorada(MATRIZPI *matrizPIParam, int idNoParam, int idConfiguracaoParam, int idRNPParam, int indiceParam); //Por Leandro
void excluiColuna(MATRIZPI *matrizPIParam, int idNoParam, int idConfiguracaoParam);//Por Leandro
int limiteSubArvore(RNP rnpParam, int indiceNoPParam);
CONFIGURACAO* alocaIndividuo(int numeroRNPParam, long int idIndividuoInicialParam, long int numeroConfiguracoesParam, int numeroTrafosParam);
void inicializaRNPsFicticias(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int numeroRNPFicticiaParam, MATRIZPI *matrizPIParam, long int numeroSetoresParam);//Por Leandro
CONFIGURACAO* alocaIndividuoModificada(int numeroRNPParam,  int numeroRNPFicticiaParam, long int idIndividuoInicialParam, long int numeroConfiguracoesParam, int numeroTrafosParam, int numeroSetoresParam);
void alocaIndividuoModificadaV2(CONFIGURACAO *individuo, int numeroRNPParam,  int numeroRNPFicticiaParam, long int idIndividuoInicialParam,
		long int numeroConfiguracoesParam, int numeroTrafosParam, int numeroSetoresParam);//Por Leandro
void alocaRNP(int numeroNosParam, RNP *rnpParam);
void imprimeIndividuo(CONFIGURACAO individuoParam);
void gravaIndividuo(char *nomeFinal, CONFIGURACAO individuoParam);
void imprimeRNP(RNP rnpParam);
void desalocaConfiguracao(CONFIGURACAO individuoParam);
void inicializaMatrizPI(GRAFOSETORES *grafoSetoresParam, MATRIZPI ** matrizPIParam, int numeroMaximoColunasParam, long int numeroSetores);
void inicializaVetorPi(int numeroMaximoIndividuosParam, VETORPI **vetorPiParam);
void constroiRNPOrigem(RNP rnpOrigemAtual, RNP *rnpOrigemNova, int indicePParam, int indiceLParam, MATRIZPI *matrizPIParam, int idFlorestaParam, int idRNPParam);
void constroiRNPDestinoPAO(RNP rnpOrigemAtual, RNP rnpDestinoAtual, RNP *rnpDestinoNova, int indicePParam, int indiceLParam, int indiceAParam, MATRIZPI *matrizPIParam, int idFlorestaParam, int idRNPParam);
void constroiRNPDestinoCAO(RNP rnpOrigemAtual, RNP rnpDestinoAtual, 
        RNP *rnpDestinoNova, int indicePParam, int indiceLParam,
        int indiceRParam, int indiceAParam, MATRIZPI *matrizPIParam, 
        int idConfiguracaoParam, int idRNPParam);
void desalocaConfiguracaoModificada(CONFIGURACAO configuracaoParam);// Por Leandro
void desalocacaoCompletaConfiguracao(CONFIGURACAO *configuracoes, long int numeroPosicoesAlocadas); //Por Leandro
void desalocacaoCompletaConfiguracaoModificada(CONFIGURACAO **configuracoesParam, long int numeroPosicoesAlocadasParam);// Por Leandro
void copiaIndividuo(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2, long int idIndividuoAtual, long int idNovoIndividuo, MATRIZPI *matrizPIParam);
void copiaIndividuoModificada(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2,
        long int idIndividuoAtual, long int idNovoIndividuo, MATRIZPI *matrizPIParam);//Por Leandro
void copiaPonteirosIndividuo(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2,
        long int idIndividuoAtual, long int idNovoIndividuo, MATRIZPI *matrizPIParam);//Por Leandro
void copiaIndividuoMelhorada(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2, long int idIndividuoAtual,
		long int idNovoIndividuo, MATRIZPI *matrizPIParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam, int numeroTrafosParam); //Por Leandro
void copiaIndividuoInicial(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracaoInicial, long int idNovoIndividuo, MATRIZPI *matrizPIParam, VETORPI *vetorPiParam);
void copiaDadosVetorPIModificada(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idOriginal, long int idNovo); //Por Leandro
void copiaDadosVetorPIPosicaoJaAlocada(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idOriginal, long int idNovo); //Por Leandro
void copiaDadosVetorPI(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idOriginal, long int idNovo);
void copiaColunasMatrizPI(MATRIZPI *matrizPiCopiaParam, MATRIZPI *matrizPiOriginalParam, int idConfiguracaoCopiaParam, long int idConfiguracaoOriginalParam,
		CONFIGURACAO *configuracoesOriginalParam);//Por Leandro
void constroiRNPDestino(CONFIGURACAO *configuracoesParam, int rnpP, 
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam, 
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam);
void constroiRNPOrigemDestino(CONFIGURACAO *configuracoesParam, int rnpP, 
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam, 
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam);
void constroiRNPOrigemDestinoFicticia(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam);//Por Leandro
void obtemConfiguracao(CONFIGURACAO *configuracoes, long int idConfiguracaoAncestral, long int idNovaConfiguracao,
        int indiceL, int indiceP, int indiceA, int indiceR, int rnpA, int rnpP, MATRIZPI *matrizPIParam);
void obtemConfiguracaoModificada(CONFIGURACAO *configuracoes, long int idConfiguracaoAncestral, long int idNovaConfiguracao,
        int indiceL, int indiceP, int indiceA, int indiceR, int rnpA, int rnpP, MATRIZPI *matrizPIParam);//Por Leandro
void obtemConfiguracaoOrigemDestino(CONFIGURACAO *configuracoesParam, long int idConfiguracaoAncestralParam, long int idNovaConfiguracaoParam,
        int indiceLParam, int indicePParam, int indiceAParam, int indiceRParam, int rnpAParam, int rnpPParam, MATRIZPI *matrizPIParam);//Por Leandro
void obtemAvaliaConfiguracaoIsolaRestabelece(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, MATRIZPI *matrizPiParam,
		VETORPI *vetorPiParam, GRAFOSETORES *grafoSetoresParam, LISTACHAVES *listaChavesParam, ESTADOCHAVE *estadoInicialParam, DADOSTRAFO *dadosTrafoParam,
		int numeroTrafosParam, int *indiceReguladorParam, DADOSREGULADOR *dadosReguladorParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *rnpSetoresParam,
		MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam);//Por Leandro
void recuperaPosicaoPRA(long int noP, long int noA, long int noR, int *rnpP, int *rnpA,
        int *rnpR, int *indiceP, int *indiceR, int *indiceA, long int idConfiguracao, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam);
void recuperaPosicaoPRAModificada(long int noP, long int noA, long int noR, int *rnpP, int *rnpA,
        int *rnpR, int *indiceP, int *indiceR, int *indiceA, long int idConfiguracao, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam);//Por Leandro
void desalocaMatrizPI(MATRIZPI *matrizPIParam, long int numeroSetores);
void desalocaVetorPi(int numeroMaximoConfiguracoesParam, VETORPI *vetorPiParam);
void atualizaMatrizPIV2(MATRIZPI *matrizPiParam, long int idConfiguracaoTempParam, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam,
	        CONFIGURACAO *configuracoesTempParam, MATRIZPI *matrizPiTempParam,
	        VETORPI *vetorPiTempParam, long int setorFaltaParam);
COLUNAPI * determinaNoAdjacenteModificada(int tentativasParam, long int idNoParam, CONFIGURACAO florestaParam,
        int idRNPParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam); // Por Leandro
void constroiRNPDestinoComDestinoFicticia(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam); // Por Leandro
void constroiRNPDestinoComOrigemFicticia(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam); // Por Leandro
void realocaVetorPi(int numeroPosicoesAnteriorParam, int numeroPosicoesNovoParam, VETORPI **vetorPiParam); // Por Leandro
void realocaIndividuo(int numeroPosicoesAnteriorParam, int numeroPosicoesNovoParam, CONFIGURACAO **individuosParam, long int idPrimeiraConfiguracaoParam, int numeroTrafosParam); // Por Leandro
long int determinaNumeroSetoresEmRNPFicticia(CONFIGURACAO popConfiguracaoParam); // Por Leandro
#endif	/* FUNCOESRNP_H */

