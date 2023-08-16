/**
 * @file funcoesProblema.h
 * @brief Este arquivo contém os protótipos das funções implementadas no arquivo funcoesProblema.c.
 * 
 */
#ifndef FUNCOESPROBLEMA_H
#define	FUNCOESPROBLEMA_H
#include "data_structures.h"

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif
void constroiMatrizZ(GRAFO *grafoSDRParam, MATRIZCOMPLEXA **ZParam, long int numeroBarrasParam);
__complex__ double valorZ(MATRIZCOMPLEXA *ZParam, long int idNo1, long int idNo2);
void imprimeMatrizZ(MATRIZCOMPLEXA *ZParam, long int numeroBarrasParam);
void constroiMatrizMaximoCorrente(GRAFO *grafoSDRParam, MATRIZMAXCORRENTE **maximoCorrenteParam, long int numeroBarrasParam);
double maximoCorrente(MATRIZMAXCORRENTE *maximoCorrenteParam, long int idNo1, long int idNo2);
void numeroManobras(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF, 
        int idNovaConfiguracaoParam, int *casoManobra);
void numeroManobrasModificada(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra); // Por Leandro:
void numeroManobrasRestabelecimento(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra);
void numeroManobrasRestabelecimentoModificada(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra);//Por Leeandro
void numeroManobrasSubTipo(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, long int *numChaveAberta,
        long int *numChaveFechada, int idNovaConfiguracaoParam);
void buscaNosRestabelecimento(long int *a, CONFIGURACAO *configuracoesParam, int rnpP, 
        long int indiceP, GRAFOSETORES *grafoSetoresParam, LISTACHAVES *listaChavesParam, 
        int *rnpA, long int *indiceA, int idConfiguracaoParam, long int indiceL, 
        long int *nosFalta, int *numeroFalta, long int *r, long int *indiceR, 
        ESTADOCHAVE *estadoInicial, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam);
void constroiRNPDestinoRestabelecimento(CONFIGURACAO *configuracoesParam, int rnpP, 
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam, 
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam);
void constroiRNPOrigemRestabelecimento(CONFIGURACAO *configuracoesParam, int rnpP, 
        int indiceL, int indiceP, MATRIZPI *matrizPiParam, long int idConfiguracaoParam, RNP *rnpOrigem, long int idNovaConfiguracaoParam);
void fluxoCargaTodosAlimentadores(long int numeroBarrasParam, 
        DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam);
void fluxoCargaAlimentador(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, int maximoIteracoes);
void avaliaConfiguracao(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam, 
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, 
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, 
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam, 
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos);
void avaliaConfiguracaoModificada(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam,
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam,
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos,
		GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, LISTACHAVES *listaChavesParam, VETORPI *vetorPiParam, BOOL flagManobrasEmParParam) ;//Por leandro
void imprimeIndicadoresEletricos(CONFIGURACAO configuracaoParam);
void carregamentoPerdas(CONFIGURACAO *configuracaoParam, int indiceRNP, RNPSETORES *matrizB, 
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam);
void carregamentoTrafo(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam,
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino);
void calculaPonderacao(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, __complex__ double VFParam);
void reconfigurador(GRAFO *grafoSDRParam, long int numeroBarras,  DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos, 
        DADOSALIMENTADOR *dadosAlimentadorSDRParam, long int numeroAlimentadores, DADOSREGULADOR *dadosReguladorSDR,
        long int numeroReguladores, RNPSETORES *rnpSetoresParam, long int numeroSetores, GRAFOSETORES *grafoSetoresParam, 
        LISTACHAVES *listaChavesParam, long int numeroChaves, CONFIGURACAO *configuracaoInicial, MATRIZMAXCORRENTE *maximoCorrente,
    MATRIZCOMPLEXA *Z, ESTADOCHAVE *estadoInicial, ESTADOCHAVE *estadoAutomaticas, ESTADOCHAVE *estadoRestabelecimento);
void desalocaMatrizZ(MATRIZCOMPLEXA *ZParam, long int numeroBarrasParam);
void desalocaMatrizCorrente(MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam);
void buscaTodosNosRestabelecimento(CONFIGURACAO *configuracoesParam, int rnpP,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam, long int indiceL,
        long int *nosFalta, int numeroFalta, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, 
        NOSRESTABELECIMENTO *nosParam, long int indiceFalta);
void buscaTodosNosRestabelecimentoCorrigida(CONFIGURACAO *configuracoesParam, int rnpPParam,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam, long int indiceL,
        long int *nosFalta, int numeroFalta, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam,
        NOSRESTABELECIMENTO *nosParam, long int indiceFalta);
void buscaTodosNosRestabelecimentoV2(CONFIGURACAO *configuracoesParam, int rnpPParam,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, NOSRESTABELECIMENTO *nosParam); // Por Leandro
//int determinaArvoreFicticiaOrigemV0(CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam);// Por Leandro
int determinaArvoreFicticiaOrigem(CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam);// Por Leandro
void salvaPrimeiroIndividuo(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam);// Leandro
void buscaChaveIsolaFalta(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, long int *numChaveAberta, long int *idChaveAberta,
        long int *idChaveFechada, long int setorFaltaParam, long int noAnteriorFalta,
        int numeroManobras, LISTACHAVES *listaChavesParam);
BOOL verificaFactibilidade(CONFIGURACAO configuracaoParam, int limiteCarregamentoRede, int limiteCarregamentoTrafo, int limiteQuedaTensao);

BOOL verificaFactibilidadeAlimentador(CONFIGURACAO configuracaoParam, long int indiceRNPParam, int limiteCarregamentoRede, int limiteCarregamentoTrafo, int limiteQuedaTensao, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam); //Por Leandro
//long int melhorSolucao(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam);
void isolaRestabeleceTodasOpcoes(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam);
void isolaRestabeleceMultiplasFaltas(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int numeroFaltasParam, long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam);
void salvaDadosArquivo(char *nomeArquivo, char *nomeFinal, CONFIGURACAO *configuracoesParam, VETORTABELA *tabelas, int numeroTabelas, VETORPI *vetorPiParam, LISTACHAVES *listaChavesParam); 
long int melhorSolucao(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam, int numeroTabelas);
long int melhorDasSolucaoQueRestabelecemTodosSetoresPelaHeuristica(CONFIGURACAO *configuracoesParam, long int idConfiguracao1Param, long int idConfiguracaoParam); //Por Leandro
long int melhorSolucaoNaoFactivel(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam, int numeroTabelas) ;
long int determinaSolucaoFinalMetodo1(FRONTEIRAS fronteiraParetoParam);// Por Leandro
long int determinaSolucaoFinalMetodo2(FRONTEIRAS fronteiraParetoParam, CONFIGURACAO *configuracoesParam);// Por Leandro
long int determinaSolucaoFinalMetodo3(FRONTEIRAS fronteiraParetoParam, CONFIGURACAO *configuracoesParam);// Por Leandro

BOOL verificaSeJaFoiSalvo(TABELA *individuosParam, int numeroIndividuosSalvosParam, long int idConfiguracaoParam); //Por Leandro
BOOL verificaSeHaValorIgualJaSalvoTabela(VETORTABELA populacaoParam, double objetivoParam); //Por Leandro
BOOL verificaSeHaValorIgualJaSalvoTabelasManobras(VETORTABELA populacaoParam, long int idConfiguracaoParam, CONFIGURACAO *configuracoesParam); //Por Leandro
BOOL verificaSeHaValorIgualJaSalvoTabelasEnergiaNaoSuprida(VETORTABELA populacaoParam, long int idConfiguracaoParam, CONFIGURACAO *configuracoesParam); //Por Leandro
BOOL verificaSeHaValorIgualJaSalvoTabelaPotenciaTransferida(VETORTABELA populacaoParam, long int idConfiguracaoParam, CONFIGURACAO *configuracoesParam); //Por Leandro


void classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras(FRONTEIRAS **fronteiraParam, int *numeroDeFronteirasParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, int numeroTabelasParam); //Por Leandro
void classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobrasV2(FRONTEIRAS **fronteiraParam, int *numeroDeFronteirasParam,
		CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, int numeroTabelasParam, VETORPI *vetorPiParam); //Por Leandro
void classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobrasV3(FRONTEIRAS **fronteiraParam, int *numeroDeFronteirasParam,
		CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, int numeroTabelasParam, VETORPI *vetorPiParam, long int idPrimeiraConfiguracaoParam);//Por Leandro

void otimizaFronteiraPareto(FRONTEIRAS *fronteirasParam, int idFronteiraParam, FRONTEIRAS *fronteiraParetoParam, CONFIGURACAO *configuracoesParam);//Por Leandro
void otimizaFronteiraParetoV2(FRONTEIRAS *fronteirasParam, int idFronteiraParam, FRONTEIRAS *fronteiraParetoParam, CONFIGURACAO *configuracoesParam); //Por Leandro
void ordenaCrescenteIndividuosFronteiraParetoObjetivo1(FRONTEIRAS *fronteiraParetoParam);//Por Leandro
long int *selecionaSolucoesFinaisOpcao1(int *numeroSolucoesFinaisParam, FRONTEIRAS fronteiraParam);//Por Leandro
long int *selecionaSolucoesFinaisOpcao2(int *numeroSolucoesFinaisParam, FRONTEIRAS fronteiraParam);//Por Leandro
void correnteJusante(int idRNP, int carga, int iteracao,
        CONFIGURACAO configuracaoParam, RNPSETORES *matrizB, int *indiceRegulador, DADOSREGULADOR *dadosRegulador);
void inicializaObjetivos(CONFIGURACAO *configuracao, int numeroTrafosParam);
void inicializaObjetivosModificada(CONFIGURACAO *configuracao, int numeroTrafosParam);
void inicializaObjetivosTiposChaves(CONFIGURACAO *configuracao, int numeroTrafosParam);
void gravaDadosEletricos(char *nomeArquivo, CONFIGURACAO configuracoesParam);
double cargaTrecho(CONFIGURACAO configuracaoParam, int indiceRNP, long int indiceSetorRaiz, 
        long int indiceSetorInicial, long int indiceSetorFinal, RNPSETORES *matrizB);

void fluxoCargaAnelAlimentador(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador);
void salvaMelhorConfiguracao(CONFIGURACAO configuracaoParam, int numeroSetorFalta, int seed, long int melhorConfiguracao, long int *setorFalta);
void avaliaConfiguracaoAnelCaso2(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam, 
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, 
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, 
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam, 
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos);
__complex__ double calculaImpedanciaTheveninMesmoAlimentador(CONFIGURACAO configuracaoParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, long int indiceSetor1, long int indiceSetor2, int indiceRNP1, long int barraChave1, long int barraChave2);
void fluxoPotenciaAnel(GRAFO *grafoSDRParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB, double SBase,
        MATRIZCOMPLEXA *ZParam, int *tapReguladores, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam, int numeroTrafos, int numeroAlimentadores,
        int rnpA, int rnpP, long int barraDe, long int barraPara, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam,long int setorBarraDe, long int setorBarraPara);
void gravaDadosEletricosVariosTestes(char *nomeArquivo, CONFIGURACAO *configuracaoParam, int seed, char *nomeSetorFalta, long int idMelhorConfiguracaoAposBE, 
        long int idMelhorConfiguracaoAposEvolutivo, double tempoBE, double tempoEvolutivo, long int melhorSolucao, double tempoChaveamento);
void carregamentoPerdasModificada(CONFIGURACAO *configuracaoParam, int indiceRNP, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam);//Leandro
void calculaPotenciaAtivaNaoSuprida(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, RNPSETORES *matrizB); //Leandro
void calculaPotenciaAtivaNaoSupridaPorNivelPrioridade(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		RNPSETORES *matrizB, GRAFO *grafoSDRParam, BOOL flagManobrasAlivioParam); //Leandro
void calculaEnergiaAtivaNaoSupridaPorNivelPrioridade(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, long int idConfiguracaoSelecionadaParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, LISTACHAVES *listaChavesParam, VETORPI *vetorPiParam, BOOL flagManobrasEmParParam);//Leandro
void impremeArquivosdeSaidaV2(int seedParam, long int setorFaltaParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, long int idMelhorConfiguracaoParam, long int melhorConfiguracaoAposBEParam, int numeroIndividuosParam, double tempoParam, int flagSolucaoFinalParam, int numeroTabelasParam);// Por Leandro
void imprimeArquivosdeSaidaV4(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam,
		long int *idSolucoesFinaisParam, int numeroDesejadoSolucoesFinaisParam, long int idPrimeiraConfiguracaoParam, int numeroIndividuosParam, double tempoParam,
		int numeroTabelasParam, RNPSETORES *matrizBParam, long int idConfiguracaoParam); // Por Leandro

void imprimeArquivosdeSaidaV5(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam,
		long int *idSolucoesFinaisParam, int numeroDesejadoSolucoesFinaisParam, long int idPrimeiraConfiguracaoParam, int numeroIndividuosPopulacaoInicialParam, double tempoParam,
		int numeroTabelasParam, RNPSETORES *matrizBParam, long int idConfiguracaoParam, VETORPI *vetorPiParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, long int geracoesExecutadasParam, int numeroSolucoesFinaisObtidasParam);//Por Leandro

void mostraFronteiraPareto(FRONTEIRAS fronteiraParetoParam, CONFIGURACAO *configuracoesParam);//Por Leandro
void imprimeIndividuosBuscaExaustiva(long int *setorFaltaParam, int numeroSetorFaltaParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoInicialParam, long int numeroConfiguracoesParam, double tempoParam, VETORPI *vetorPiParam); //Por Leandro
double calculaSomatorioPotenciaAtivaSubarvore(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, GRAFO *grafoSDRParam); //Por Leandro
NIVEISDEPRIORIDADEATENDIMENTO calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, GRAFO *grafoSDRParam); //Por Leandro
NIVEISDEPRIORIDADEATENDIMENTO calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		long int indiceNoRaizParam, int indiceLParam, int rnpNoRaizParam, RNPSETORES *matrizB, GRAFO *grafoSDRParam);//Por Leandro
double calculaSomatorioPotenciaAparenteSubarvore(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, GRAFO *grafoSDRParam); //Por Leandro
double calculaSomatorioCorrenteCargaSubarvore(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam); //Por Leandro
void calculaEnergiaAtivaNaoSupridaPorNivelPrioridadeIndividuosIsolaRestabeleceTodasOpcoesHeuristica(CONFIGURACAO *configuracoesParam, VETORPI *vetorPiParam, MATRIZPI *matrizPiParam,
		long int idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam); //Por Leandro
BOOL verificaExistenciaConsumidoresEspeciais(RNP rnpParam, GRAFOSETORES *grafoSetoresParam, int indiceNoRaizSubarvore);
int determinaQuantidadeSetoresJusanteFalta(RNP rnpParam, GRAFOSETORES *grafoSetoresParam, int indiceNoRaizSubarvore);
void marcaSetoresJusanteFalta(long int nosJusanteFaltaParam[100], int numeroAJusanteParam, CONFIGURACAO *configuracoesParam,
								long int idConfiguracaoParam, int rnpPParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam);
void obtemConfiguracaoInicial(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
       CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
       VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
       long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam); //Por Leandro
void copiaTodasInformacoesIndividuo(CONFIGURACAO *configuracoesCopiaParam, CONFIGURACAO *configuracoesOriginalParam,
		long int idIndividuoOriginalParam, long int idIndividuoCopiaParam, VETORPI *vetorPiCopiaParam, VETORPI *vetorPiOriginalParam,
		MATRIZPI *matrizPICopiaParam, MATRIZPI *matrizPIOriginalParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam, int numeroTrafosParam); //Por Leandro
void copiaDadosEletricosObjetivos(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2, long int idIndividuoAtual,
		long int idNovoIndividuo, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam, int numeroTrafosParam, BOOL copiarDadosEletricosParam);//Por Leandro
BOOL determinaFlagManobrasAlivio(CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int idConfiguracaoParam);//Por Leandro
void mostraSequenciaChaveamentoIndividuo(long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam); //Por Leandro
void imprimeSequenciaChaveamentoIndividuoSumarizada(int seedParam, char tipoFaltaParam[20], char setoresFaltaParam[2000], long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, char *nomeArquivoParam);//Leandro
void imprimeSequenciaChaveamentoSolucoesFinais(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, FRONTEIRAS fronteiraParetoParam, long int *idIndividuosFinaisParam, int numeroSolucoesFinaisObtidas, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam);// Por Leandro
void imprimeFronteiraPareto(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, FRONTEIRAS fronteiraParam, CONFIGURACAO *configuracoesParam);//Por Leandro
void mostraInformacoesIndividuoSequenciaCorrigida(long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,  CONFIGURACAO *configuracoesChavParam);// Por Leandro

//====================FUNÇÕES TRIFÁSICAS==============================================================================
//Cristhian: Funcoes adicionadas para tornar o algoritmo trifásico

void verificaPresencaDeFaseNo(int* presencaDeFases, TIPOFASES tipoFases);

void fluxoCargaAlimentadorTrifasico(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, int maximoIteracoes);

void correnteJusanteTrifasico(int idRNP, int carga, int iteracao, CONFIGURACAO configuracaoParam, RNPSETORES *matrizB, int *indiceRegulador, DADOSREGULADOR *dadosRegulador);
void tensaoQuedaTensaoModificada(long int* pior, int indiceRNP, CONFIGURACAO configuracaoParam, double VFParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR);

//====================================================================================================================

#endif	/* FUNCOESPROBLEMA_H */

