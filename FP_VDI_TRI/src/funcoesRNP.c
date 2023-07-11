/**
 * @file funcoesRNP.c
 * @brief Este arquivo contém a implementação das funções para manipulação da estrutura de dados RNP.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoesRNP.h"
#include "funcoesAuxiliares.h"
#include "funcoesProblema.h"
#include "funcoesSetor.h"
#include "funcoesHeuristica.h"

int maximoTentativas = 5; /*<Define o número máximo de tentativas de escolha um nó adjacete ao nó raiz da subárvore a ser transferida (PAO e CAO) ou reconectada (LRO)*/

/**
 * Faz a inserção no vetor pi da RNP do identificador configuração que deu origem a nova configuração e dos nós utilizados
 * @param piParam ponteiro para vetor do tipo VETORPI com o ancestral de cada configuração.
 * @param idIndividuoParam inteiro com o identificador da configuração gerada.
 * @param idAncestralParam inteiro com o identificador da configuração ancestral.
 * @param idChaveAbertaParam ponteiro para vetor do tipo long int com o identificador das chaves abertas.
 * @param idChaveFechadaParam ponteiro para vetor do tipo long int com o identificador das chaves fechada.s
 * @param numeroManobras inteiro com a quantidade de manobras.
 * @param casoManobra inteiro identificando o caso da manobra (1, 2 ou 3).
 * @param estadoInicialCA ponteiro para vetor do tipo BOOL que indica se chave a ser aberta voltou ou não ao estado inicial.
 * @param estadoInicialCF ponteiro para vetor do tipo BOOL que indica se chave a ser fechada voltou ou não ao estado inicial.
 * @param conjuntoNosParam ponteiro para vetor do tipo NOSPRA com os nós utilizados nos operadores RNP.
 */
void atualizaVetorPi(VETORPI *piParam, long int idIndividuoParam, long int idAncestralParam, 
        long int *idChaveAbertaParam, long int *idChaveFechadaParam, int numeroManobras, 
        int casoManobra, BOOL *estadoInicialCA,BOOL *estadoInicialCF, NOSPRA *conjuntoNosParam) {
    int indice;
    piParam[idIndividuoParam].idAncestral = idAncestralParam;
    piParam[idIndividuoParam].numeroManobras = numeroManobras;
    piParam[idIndividuoParam].casoManobra = casoManobra;    
    piParam[idIndividuoParam].idChaveAberta = Malloc(long int, numeroManobras + 1);
    piParam[idIndividuoParam].idChaveFechada = Malloc(long int, numeroManobras + 1);
    piParam[idIndividuoParam].estadoInicialChaveAberta = Malloc(BOOL, numeroManobras + 1);
    piParam[idIndividuoParam].estadoInicialChaveFechada = Malloc(BOOL, numeroManobras + 1);
    piParam[idIndividuoParam].nos = conjuntoNosParam;
    if (numeroManobras > 0)
        for (indice = 0; indice < numeroManobras; indice++) {
            piParam[idIndividuoParam].idChaveAberta[indice] = idChaveAbertaParam[indice];
            piParam[idIndividuoParam].estadoInicialChaveAberta[indice] = estadoInicialCA[indice];
            piParam[idIndividuoParam].idChaveFechada[indice] = idChaveFechadaParam[indice];
            piParam[idIndividuoParam].estadoInicialChaveFechada[indice] = estadoInicialCF[indice];
        }
}

/**
 * Por Leandro: consiste na função "atualizaVetorPi()" modificada para salvar o operador que gerou a nova configuração
 *
 * Faz a inserção no vetor pi da RNP do identificador configuração que deu origem a nova configuração e dos nós utilizados
 * @param piParam ponteiro para vetor do tipo VETORPI com o ancestral de cada configuração.
 * @param idIndividuoParam inteiro com o identificador da configuração gerada.
 * @param idAncestralParam inteiro com o identificador da configuração ancestral.
 * @param idChaveAbertaParam ponteiro para vetor do tipo long int com o identificador das chaves abertas.
 * @param idChaveFechadaParam ponteiro para vetor do tipo long int com o identificador das chaves fechada.s
 * @param numeroManobras inteiro com a quantidade de manobras.
 * @param casoManobra inteiro identificando o caso da manobra (1, 2 ou 3).
 * @param estadoInicialCA ponteiro para vetor do tipo BOOL que indica se chave a ser aberta voltou ou não ao estado inicial.
 * @param estadoInicialCF ponteiro para vetor do tipo BOOL que indica se chave a ser fechada voltou ou não ao estado inicial.
 * @param conjuntoNosParam ponteiro para vetor do tipo NOSPRA com os nós utilizados nos operadores RNP.
 */
void atualizaVetorPiModificada(VETORPI *piParam, long int idIndividuoParam, long int idAncestralParam,
        long int *idChaveAbertaParam, long int *idChaveFechadaParam, int numeroManobras,
        int casoManobra, BOOL *estadoInicialCA,BOOL *estadoInicialCF, NOSPRA *conjuntoNosParam, OPERADOR operadorParam) {
    int indice;
    piParam[idIndividuoParam].idAncestral = idAncestralParam;
    piParam[idIndividuoParam].numeroManobras = numeroManobras;
    piParam[idIndividuoParam].casoManobra = casoManobra;
    piParam[idIndividuoParam].idChaveAberta = Malloc(long int, numeroManobras + 1);
    piParam[idIndividuoParam].idChaveFechada = Malloc(long int, numeroManobras + 1);
    piParam[idIndividuoParam].estadoInicialChaveAberta = Malloc(BOOL, numeroManobras + 1);
    piParam[idIndividuoParam].estadoInicialChaveFechada = Malloc(BOOL, numeroManobras + 1);
    piParam[idIndividuoParam].nos =  Malloc(NOSPRA, numeroManobras + 1);
    if (numeroManobras > 0)
        for (indice = 0; indice < numeroManobras; indice++) {
            piParam[idIndividuoParam].idChaveAberta[indice] = idChaveAbertaParam[indice];
            piParam[idIndividuoParam].estadoInicialChaveAberta[indice] = estadoInicialCA[indice];
            piParam[idIndividuoParam].idChaveFechada[indice] = idChaveFechadaParam[indice];
            piParam[idIndividuoParam].estadoInicialChaveFechada[indice] = estadoInicialCF[indice];
            piParam[idIndividuoParam].nos[indice] = conjuntoNosParam[indice];
        }
}

/**
 *Metodo para recuperar as rnps e posições dos setores utilizados para obter uma configuração dado o identificador da configuração. 
 * O método localiza na matrizPI da RNP pela coluna referente a configuração com as informações do setor.
 * @param noP
 * @param noA
 * @param noR
 * @param rnpP
 * @param rnpA
 * @param rnpR
 * @param indiceP
 * @param indiceR
 * @param indiceA
 * @param idConfiguracao
 * @param matrizPiParam
 * @param vetorPiParam
 */
void recuperaPosicaoPRA(long int noP, long int noA, long int noR, int *rnpP, int *rnpA,
        int *rnpR, int *indiceP, int *indiceR, int *indiceA, long int idConfiguracao, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam) {
    long int colunaPi;
    colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, noA, idConfiguracao);
    indiceA[0] = matrizPiParam[noA].colunas[colunaPi].posicao;
    rnpA[0] = matrizPiParam[noA].colunas[colunaPi].idRNP;

    colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, noP, idConfiguracao);
    indiceP[0] = matrizPiParam[noP].colunas[colunaPi].posicao;
    rnpP[0] = matrizPiParam[noP].colunas[colunaPi].idRNP;

    colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, noR, idConfiguracao);
    indiceR[0] = matrizPiParam[noR].colunas[colunaPi].posicao;
    rnpR[0] = matrizPiParam[noR].colunas[colunaPi].idRNP;

}

/**
 * Por Leandro:
 * Consiste na função "recuperaPosicaoPRA()" modificada
 * (a) para retornar -1 quando um nó é igual a -1. Isso tornou-se necessário após o início da correção de sequências de chaveamento.
 *
 * Descrição:Metodo para recuperar as rnps e posições dos setores utilizados para obter uma configuração dado o identificador da configuração.
 * O método localiza na matrizPI da RNP pela coluna referente a configuração com as informações do setor.
 * @param noP
 * @param noA
 * @param noR
 * @param rnpP
 * @param rnpA
 * @param rnpR
 * @param indiceP
 * @param indiceR
 * @param indiceA
 * @param idConfiguracao
 * @param matrizPiParam
 * @param vetorPiParam
 */
void recuperaPosicaoPRAModificada(long int noP, long int noA, long int noR, int *rnpP, int *rnpA,
        int *rnpR, int *indiceP, int *indiceR, int *indiceA, long int idConfiguracao, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam) {
    long int colunaPi;
    if(noA > 0){
		colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, noA, idConfiguracao);
		indiceA[0] = matrizPiParam[noA].colunas[colunaPi].posicao;
		rnpA[0] = matrizPiParam[noA].colunas[colunaPi].idRNP;
    }
    else{
    	indiceA[0] = -1;
    	rnpA[0] = -1;
    }

    if(noP > 0){
		colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, noP, idConfiguracao);
		indiceP[0] = matrizPiParam[noP].colunas[colunaPi].posicao;
		rnpP[0] = matrizPiParam[noP].colunas[colunaPi].idRNP;
    }
    else{
        indiceP[0] = -1;
        rnpP[0] = -1;
    }

    if(noR > 0){
		colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, noR, idConfiguracao);
		indiceR[0] = matrizPiParam[noR].colunas[colunaPi].posicao;
		rnpR[0] = matrizPiParam[noR].colunas[colunaPi].idRNP;
    }
    else{
        indiceR[0] =-1;
        rnpR[0] = -1;
    }

}

/**
 * Dados os alimentadores envolvidos na manobra e os índices dos setores que serão utilizados pelos operadores PAO e CAO da Representação Nó Profundidade obtém uma nova configuração construindo os arrays que representam os alimentadores.
 * @param configuracoes
 * @param idConfiguracaoAncestral
 * @param idNovaConfiguracao
 * @param indiceL
 * @param indiceP
 * @param indiceA
 * @param indiceR
 * @param rnpA
 * @param rnpP
 * @param matrizPIParam
 */
void obtemConfiguracao(CONFIGURACAO *configuracoes, long int idConfiguracaoAncestral, long int idNovaConfiguracao,
        int indiceL, int indiceP, int indiceA, int indiceR, int rnpA, int rnpP, MATRIZPI *matrizPIParam) {
    int tamanhoSubarvore;
    int indice1;
    //calcula o tamanho da subárvore podada
    tamanhoSubarvore = indiceL - indiceP + 1;
    //realiza a alocação das que serão alteradas RNPs 
    alocaRNP(configuracoes[idConfiguracaoAncestral].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoes[idNovaConfiguracao].rnp[rnpA]);
    alocaRNP(configuracoes[idConfiguracaoAncestral].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoes[idNovaConfiguracao].rnp[rnpP]);
    //constroi a rnp de origem nova
    constroiRNPOrigem(configuracoes[idConfiguracaoAncestral].rnp[rnpP], &configuracoes[idNovaConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIParam, idNovaConfiguracao, rnpP);
    constroiRNPDestino(configuracoes, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIParam, idConfiguracaoAncestral, &configuracoes[idNovaConfiguracao].rnp[rnpA], idNovaConfiguracao);
    for (indice1 = 0; indice1 < configuracoes[idConfiguracaoAncestral].numeroRNP; indice1++) {
        if (indice1 != rnpP && indice1 != rnpA) {
            configuracoes[idNovaConfiguracao].rnp[indice1] = configuracoes[idConfiguracaoAncestral].rnp[indice1];
        }
    }
}

/**
 * Por Leandro: consiste na função "obtemConfiguracao()" modificada para que:
 * a) seja capaz de reproduzir também o comportamento do operador LRO. Em outras palavras, fazer poda em RNP fictícia e transferência para RNP real
 *
 * Dados os alimentadores envolvidos na manobra e os índices dos setores que serão utilizados pelos operadores PAO e CAO da Representação Nó Profundidade
 * obtém uma nova configuração construindo os arrays que representam os alimentadores.
 *
 * @param configuracoes
 * @param idConfiguracaoAncestral
 * @param idNovaConfiguracao
 * @param indiceL
 * @param indiceP
 * @param indiceA
 * @param indiceR
 * @param rnpA
 * @param rnpP
 * @param matrizPIParam
 */
void obtemConfiguracaoModificada(CONFIGURACAO *configuracoes, long int idConfiguracaoAncestral, long int idNovaConfiguracao,
        int indiceL, int indiceP, int indiceA, int indiceR, int rnpA, int rnpP, MATRIZPI *matrizPIParam) {
    int tamanhoSubarvore;
    int indice1;

	//calcula o tamanho da subárvore podada
	tamanhoSubarvore = indiceL - indiceP + 1;
    if(rnpP >= configuracoes[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoes[idConfiguracaoAncestral].numeroRNP){//Operador LRO
		rnpP = rnpP -  configuracoes[idConfiguracaoAncestral].numeroRNP;
		//realiza a alocação das que serão alteradas RNPs
		alocaRNP(configuracoes[idConfiguracaoAncestral].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoes[idNovaConfiguracao].rnp[rnpA]);
		alocaRNP(configuracoes[idConfiguracaoAncestral].rnpFicticia[rnpP].numeroNos - tamanhoSubarvore, &configuracoes[idNovaConfiguracao].rnpFicticia[rnpP]);
		//constroi as rnps origem e destino novas
		constroiRNPOrigem(configuracoes[idConfiguracaoAncestral].rnpFicticia[rnpP], &configuracoes[idNovaConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, matrizPIParam, idNovaConfiguracao, rnpP + configuracoes[idConfiguracaoAncestral].numeroRNP);
		constroiRNPDestinoCAO(configuracoes[idConfiguracaoAncestral].rnpFicticia[rnpP], configuracoes[idConfiguracaoAncestral].rnp[rnpA], &configuracoes[idNovaConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPIParam, idNovaConfiguracao, rnpA);
		//Copia as demais RNPs
		for (indice1 = 0; indice1 < configuracoes[idConfiguracaoAncestral].numeroRNP; indice1++) {
			if (indice1 != rnpA) {
				configuracoes[idNovaConfiguracao].rnp[indice1] = configuracoes[idConfiguracaoAncestral].rnp[indice1];
			}
		}
		for (indice1 = 0; indice1 < configuracoes[idConfiguracaoAncestral].numeroRNPFicticia; indice1++ ){
			if (indice1 != rnpP)
				configuracoes[idNovaConfiguracao].rnpFicticia[indice1] = configuracoes[idConfiguracaoAncestral].rnpFicticia[indice1];
		}
		configuracoes[idNovaConfiguracao].numeroRNPFicticia = configuracoes[idConfiguracaoAncestral].numeroRNPFicticia;

	}
	else{
		if(rnpP < configuracoes[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoes[idConfiguracaoAncestral].numeroRNP){//Operador PAO ou CAO
			//realiza a alocação das que serão alteradas RNPs
			alocaRNP(configuracoes[idConfiguracaoAncestral].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoes[idNovaConfiguracao].rnp[rnpA]);
			alocaRNP(configuracoes[idConfiguracaoAncestral].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoes[idNovaConfiguracao].rnp[rnpP]);
			//constroi a rnp de origem nova
			constroiRNPOrigem(configuracoes[idConfiguracaoAncestral].rnp[rnpP], &configuracoes[idNovaConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIParam, idNovaConfiguracao, rnpP);
			constroiRNPDestino(configuracoes, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIParam, idConfiguracaoAncestral, &configuracoes[idNovaConfiguracao].rnp[rnpA], idNovaConfiguracao);
			//Copia as demais RNPs
			for (indice1 = 0; indice1 < configuracoes[idConfiguracaoAncestral].numeroRNP; indice1++) {
				if (indice1 != rnpP && indice1 != rnpA) {
					configuracoes[idNovaConfiguracao].rnp[indice1] = configuracoes[idConfiguracaoAncestral].rnp[indice1];
				}
			}
			for (indice1 = 0; indice1 < configuracoes[idConfiguracaoAncestral].numeroRNPFicticia; indice1++ ){
				configuracoes[idNovaConfiguracao].rnpFicticia[indice1] = configuracoes[idConfiguracaoAncestral].rnpFicticia[indice1];
			}
		}
	}
}

/**
 * Por Leandro: baseado na função "obtemConfiguracaoModificada()", este método obtem uma nova configuração
 * quando a transferência dos setores ocorre no mesmo alimentador, isto é, quando o nó adjacente "a" está na mesma
 * árvore dos nós "p" e "r".
 *
 * @param configuracoes
 * @param idConfiguracaoAncestral
 * @param idNovaConfiguracao
 * @param indiceL
 * @param indiceP
 * @param indiceA
 * @param indiceR
 * @param rnpA
 * @param rnpP
 * @param matrizPIParam
 */
void obtemConfiguracaoOrigemDestino(CONFIGURACAO *configuracoesParam, long int idConfiguracaoAncestralParam, long int idNovaConfiguracaoParam,
        int indiceLParam, int indicePParam, int indiceAParam, int indiceRParam, int rnpAParam, int rnpPParam, MATRIZPI *matrizPIParam) {
    //int tamanhoSubarvore;
    int indice1;
    RNP rnpOrigem;

    if(rnpPParam == rnpAParam){
		//calcula o tamanho da subárvore podada
		//tamanhoSubarvore = indiceLParam - indicePParam + 1;
		if(rnpPParam >= configuracoesParam[idConfiguracaoAncestralParam].numeroRNP){//Operações em uma RNP Fictícia
			rnpPParam = rnpPParam -  configuracoesParam[idConfiguracaoAncestralParam].numeroRNP;
			rnpAParam = rnpPParam;

			alocaRNP(configuracoesParam[idConfiguracaoAncestralParam].rnpFicticia[rnpPParam].numeroNos, &rnpOrigem);

//			rnpOrigemAnterior = configuracoes[idConfiguracaoAncestral].rnpFicticia[rnpP];

			constroiRNPOrigemDestinoFicticia(configuracoesParam, rnpPParam, rnpAParam, indiceLParam, indicePParam, indiceAParam, indiceRParam, matrizPIParam, idConfiguracaoAncestralParam, &rnpOrigem, idNovaConfiguracaoParam);

			configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[rnpPParam] = rnpOrigem;

			//Copia as demais RNPs
			for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoAncestralParam].numeroRNP; indice1++)
				configuracoesParam[idNovaConfiguracaoParam].rnp[indice1] = configuracoesParam[idConfiguracaoAncestralParam].rnp[indice1];

			for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoAncestralParam].numeroRNPFicticia; indice1++ ){
				if (indice1 != rnpPParam)
					configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[indice1] = configuracoesParam[idConfiguracaoAncestralParam].rnpFicticia[indice1];
			}
			configuracoesParam[idNovaConfiguracaoParam].numeroRNPFicticia = configuracoesParam[idConfiguracaoAncestralParam].numeroRNPFicticia;

		}
		else{
			if(rnpPParam < configuracoesParam[idConfiguracaoAncestralParam].numeroRNP){//Operações em uma RNP Real
				alocaRNP(configuracoesParam[idConfiguracaoAncestralParam].rnp[rnpPParam].numeroNos, &rnpOrigem);

//				rnpOrigemAnterior = configuracoes[idConfiguracaoAncestral].rnp[rnpP];

				constroiRNPOrigemDestino(configuracoesParam, rnpPParam, rnpAParam, indiceLParam, indicePParam, indiceAParam, indiceRParam, matrizPIParam, idConfiguracaoAncestralParam, &rnpOrigem, idNovaConfiguracaoParam);

				configuracoesParam[idNovaConfiguracaoParam].rnp[rnpPParam] = rnpOrigem;

				//Copia as demais RNPs
				for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoAncestralParam].numeroRNP; indice1++) {
					if (indice1 != rnpPParam && indice1 != rnpAParam)
						configuracoesParam[idNovaConfiguracaoParam].rnp[indice1] = configuracoesParam[idConfiguracaoAncestralParam].rnp[indice1];
				}
				for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoAncestralParam].numeroRNPFicticia; indice1++ )
					configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[indice1] = configuracoesParam[idConfiguracaoAncestralParam].rnpFicticia[indice1];
				configuracoesParam[idNovaConfiguracaoParam].numeroRNPFicticia = configuracoesParam[idConfiguracaoAncestralParam].numeroRNPFicticia;
			}
		}

	}
}

/**
 * Por Leandro:
 * Descrição: esta função realiza a reconstrução e avaliação de um indivíduo obtido pela busca heurística que possui ao menos uma chave que retornou, posteriormente, ao seu estado inicial.
 * Durante a correção de sequência de chaveamento, podem ocorrer situações em que ao menos uma das chaves envolvidas na correção da sequência
 * está no primeiro indivíduo da sequência, o qual foi gerado pela Busca Heurística (Exaustiva).
 * A reconstrução de um novo primeiro indivíduo da sequência exige uma função especial porque, para a sua obtenção, é excepcionalmente necessário isolar o setor em falta,
 * detectar os setores que não podem restaurados e reconectar em alguma árvore os setores possíveis de serem restaurados.
 * Esta função também avalia este novo indivíduo gerado.
 *
 * Nesta função as ações para isolação e reconexão de consumidores desligados são guiadas pelas informações já salvas no vetor PI e não pelo conjunto de
 * setores em falta e topologia inicial da rede.
 *
 *@param
 *@param
 *@param
 *@param
 *@param
 *@param
 *@param
 *@param
 *@param
 */
void obtemAvaliaConfiguracaoIsolaRestabelece(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, MATRIZPI *matrizPiParam,
		VETORPI *vetorPiParam, GRAFOSETORES *grafoSetoresParam, LISTACHAVES *listaChavesParam, ESTADOCHAVE *estadoInicialParam, DADOSTRAFO *dadosTrafoParam,
		int numeroTrafosParam, int *indiceReguladorParam, DADOSREGULADOR *dadosReguladorParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *rnpSetoresParam,
		MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam) {
	NOSPRA *nosPRA;
	BOOL *estadoInicialCA, *estadoInicialCF, flag;
	RNP rnpOrigem, rnpDestino;
	long int *idChaveAberta, *idChaveFechada, idConfiguracaoBase;
	long int idAncestral, consumidoresSemFornecimento = 0, consumidoresEspeciaisSemFornecimento = 0, idNo;
	int noA, noR, noP, indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, rnpR, rnp, tamanhoTemporario;
    int indice1, indiceAux, casoManobra, contadorChav, *rnpsModificadas, numeroRNPsModificadas, *rnpsFalta, numeroRNPsFalta, contador, ultimaPosicaoVerificada, indice, numeroSetoresRemovidos;

	estadoInicialCA = malloc((vetorPiParam[idNovaConfiguracaoParam].numeroManobras + 1)*sizeof(BOOL));
	estadoInicialCF = malloc((vetorPiParam[idNovaConfiguracaoParam].numeroManobras + 1)*sizeof(BOOL));
	nosPRA = malloc((vetorPiParam[idNovaConfiguracaoParam].numeroManobras + 1) * sizeof(NOSPRA));
	rnpsModificadas = malloc((2*vetorPiParam[idNovaConfiguracaoParam].numeroManobras)*sizeof(int));
	numeroRNPsModificadas = 0;

	rnpsFalta = malloc((vetorPiParam[idNovaConfiguracaoParam].numeroManobras)*sizeof(int));
	numeroRNPsFalta = 0;

	idAncestral = vetorPiParam[idNovaConfiguracaoParam].idAncestral;
	idChaveAberta = vetorPiParam[idNovaConfiguracaoParam].idChaveAberta;
	idChaveFechada = vetorPiParam[idNovaConfiguracaoParam].idChaveFechada;

    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas 					 = configuracoesParam[idAncestral].objetivo.manobrasAutomaticas;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais 						 = configuracoesParam[idAncestral].objetivo.manobrasManuais;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idAncestral].objetivo.contadorManobrasTipo.comCargaAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual 	 = configuracoesParam[idAncestral].objetivo.contadorManobrasTipo.comCargaManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica	 = configuracoesParam[idAncestral].objetivo.contadorManobrasTipo.curtoAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual 		 = configuracoesParam[idAncestral].objetivo.contadorManobrasTipo.curtoManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.seca 				 = configuracoesParam[idAncestral].objetivo.contadorManobrasTipo.seca;

	flag = true;
    for(contadorChav = 0; contadorChav < vetorPiParam[idNovaConfiguracaoParam].numeroManobras; contadorChav++){
		noP = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].p;
		noA = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].a;
		noR = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].r;
		recuperaPosicaoPRAModificada(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);

	    //Salva as rnps alteradas a fim de executar o fluxo de carga somente nas mesmas
	    contador = 0;
	    while (contador < numeroRNPsModificadas && rnpsModificadas[contador] != rnpP)
	        contador++;
		if(rnpP >=0 && contador == numeroRNPsModificadas){
			rnpsModificadas[contador] = rnpP;
			numeroRNPsModificadas++;
		}
	    contador = 0;
	    while (contador < numeroRNPsModificadas && rnpsModificadas[contador] != rnpA)
	        contador++;
		if(rnpA >=0){
			rnpsModificadas[contador] = rnpA;
			numeroRNPsModificadas++;
		}

		if(noA < 0 && noR < 0 && rnpA < 0 && rnpP >=0){ //Se verdade, significa que a trinca em questão é para isolar um setor em falta
			if(contadorChav == 0)
				idConfiguracaoBase = idAncestral;
			else
				idConfiguracaoBase = idNovaConfiguracaoParam;

			casoManobra = 1;
			estadoInicialCA[contadorChav] = false;
			estadoInicialCF[contadorChav] = false;
			nosPRA[contadorChav].a = noA;
			nosPRA[contadorChav].p = noP;
			nosPRA[contadorChav].r = noR;

			//ROTINA PARA CALCULAR OS PARES DE MANOBRAS
			numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[contadorChav], idChaveFechada[contadorChav], listaChavesParam,
					idNovaConfiguracaoParam, estadoInicialParam, &estadoInicialCA[contadorChav], &estadoInicialCF[contadorChav], idNovaConfiguracaoParam, &casoManobra);

			copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam, idConfiguracaoBase);
			insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam], rnpP);

		    contador = 0;
		    while(contador < numeroRNPsFalta && rnpsFalta[contador] != rnpP)
		        contador++;
			if(rnpP >=0 && contador == numeroRNPsFalta){
				rnpsFalta[contador] = rnpP;
				numeroRNPsFalta++;
			}

		}
		else{
			if(flag == true)
				idConfiguracaoBase = idAncestral;
			else
				idConfiguracaoBase = idNovaConfiguracaoParam;

			indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);

			nosPRA[contadorChav].a = noA;
			nosPRA[contadorChav].p = noP;
			nosPRA[contadorChav].r = noR;

			//ROTINA PARA CALCULAR OS PARES DE MANOBRAS
			numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[contadorChav], idChaveFechada[contadorChav], listaChavesParam,
					idNovaConfiguracaoParam, estadoInicialParam, &estadoInicialCA[contadorChav], &estadoInicialCF[contadorChav], idNovaConfiguracaoParam, &casoManobra);

			tamanhoTemporario = indiceL - indiceP + 1;
			//realiza a alocação das RNPs que serão alteradas
			alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
			alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
			//obtém a nova rnp de destino
			constroiRNPDestino(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam);
			constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam);

			//###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############
			configuracoesParam[idNovaConfiguracaoParam].rnp[rnpA] = rnpDestino;
			configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP] = rnpOrigem;
			insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam], rnpA);
			for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNP; indice1++) {
				if (indice1 != rnpA && indice1 != rnpP)
					configuracoesParam[idNovaConfiguracaoParam].rnp[indice1] = configuracoesParam[idConfiguracaoBase].rnp[indice1];
				else
					configuracoesParam[idNovaConfiguracaoParam].rnp[indice1].fitnessRNP = configuracoesParam[idConfiguracaoBase].rnp[indice1].fitnessRNP; // Isto é apenas um artifício de programação para esta função. Estes valores serão atualizados quando a rnp "indice1" for avaliada por um fluxo de carga
			}
			for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNPFicticia; indice1++) {
				configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[indice1] = configuracoesParam[idConfiguracaoBase].rnpFicticia[indice1];
			}

			flag = false;
		}
		//idConfiguracaoBase = idNovaConfiguracaoParam;
    }

	//Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
	tamanhoTemporario = 1;

	//remove os setores em falta e os que não puderam ser restabelecidos
	for(indice = 0; indice < numeroRNPsFalta; indice++){ //percorre as RNPs que continha setores em Falta
		rnpP = rnpsFalta[indice];

		ultimaPosicaoVerificada = 0;
		numeroSetoresRemovidos = 9999;
		while(numeroSetoresRemovidos !=0){
			numeroSetoresRemovidos = 0;
			for(contador = ultimaPosicaoVerificada; contador < configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos && numeroSetoresRemovidos == 0; contador++){
				idNo = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[contador].idNo;
				indiceAux = contador;

				if(grafoSetoresParam[idNo].setorFalta == true){ //O setor "idNo" foi marcado como "setor em falta" (setor em falta ou setor que não pode ser restaurado)
					indiceL = limiteSubArvore( configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP], indiceAux); //Busca o limite da subárvore enraizada em "idNo"
					numeroSetoresRemovidos = indiceL - indiceAux + 1; //São removidos o setor "idNo" e os que estão a jusante do mesmo.

					if(indiceL < configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos - 1){ //Se após o último nó presente na subárvore enraizada em "idNo" ainda houveram setores armazenados em "rnpP", então copia-os excluindo os setore da subárvore enraizada em "idNo"
						for(indice1 = indiceL + 1; indice1 < configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos; indice1++){
							configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indiceAux].idNo = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
							configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indiceAux].profundidade = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
							adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indiceAux].idNo, idNovaConfiguracaoParam, rnpP, indiceAux);
							indiceAux++;
						}
					}
					configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos - numeroSetoresRemovidos;
				}
				ultimaPosicaoVerificada = contador;
			}
		}
	}

	//atualiza consumidores sem fornecimento
	configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
	configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
	//insere no vetor Pi a nova solução
/*	atualizaVetorPiModificada(vetorPiParam, idNovaConfiguracaoParam, idAncestral,
			idChaveAberta, idChaveFechada, vetorPiParam[idNovaConfiguracaoParam].numeroManobras, casoManobra,
			estadoInicialCA, estadoInicialCF, nosPRA, ESO);*/
	//Executa o fluxo de carga nos Alimentadores modificados
	for(contador = 0; contador < numeroRNPsModificadas; contador++){
		if(contador == 0)
			idAncestral = vetorPiParam[idNovaConfiguracaoParam].idAncestral;
		else
			idAncestral = idNovaConfiguracaoParam;

		rnp = rnpsModificadas[contador];

		avaliaConfiguracaoHeuristicaModificada(false, configuracoesParam, -1, rnp, idNovaConfiguracaoParam,
			dadosTrafoParam, numeroTrafosParam, configuracoesParam[idAncestral].numeroRNP,
			indiceReguladorParam, dadosReguladorParam, dadosAlimentadorParam, idAncestral, rnpSetoresParam,
			ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
			sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, false);
	}
	calculaEnergiaAtivaNaoSupridaPorNivelPrioridadeIndividuosIsolaRestabeleceTodasOpcoesHeuristica(configuracoesParam, vetorPiParam, matrizPiParam,
			idNovaConfiguracaoParam, listaChavesParam, rnpSetoresParam, grafoSDRParam);


	free(estadoInicialCA);
	free(estadoInicialCF);
	free(nosPRA);
	free(rnpsModificadas);
	free(rnpsFalta);
}

/**
 * Copia os dados de uma posição de um array to tipo VETORPI para uma nova posição de outro array do tipo VETORPI
 * @param vetorPiOriginal
 * @param vetorPiCopia
 * @param idOriginal
 * @param idNovo
 */
void copiaDadosVetorPI(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idOriginal, long int idNovo)
{
    int indice;

    vetorPiCopia[idNovo].idAncestral = vetorPiOriginal[idOriginal].idAncestral;
    vetorPiCopia[idNovo].numeroManobras = vetorPiOriginal[idOriginal].numeroManobras;
    vetorPiCopia[idNovo].casoManobra = vetorPiOriginal[idOriginal].casoManobra;
    vetorPiCopia[idNovo].sequenciaVerificada = vetorPiOriginal[idOriginal].sequenciaVerificada;
    vetorPiCopia[idNovo].idChaveAberta = Malloc(long int, vetorPiOriginal[idOriginal].numeroManobras + 1);
    vetorPiCopia[idNovo].idChaveFechada = Malloc(long int, vetorPiOriginal[idOriginal].numeroManobras + 1);
    vetorPiCopia[idNovo].estadoInicialChaveAberta = Malloc(BOOL, vetorPiOriginal[idOriginal].numeroManobras + 1);
    vetorPiCopia[idNovo].estadoInicialChaveFechada = Malloc(BOOL, vetorPiOriginal[idOriginal].numeroManobras + 1);
    vetorPiCopia[idNovo].nos = Malloc(NOSPRA, vetorPiOriginal[idOriginal].numeroManobras + 1) ;
    if (vetorPiOriginal[idOriginal].numeroManobras > 0)
        for (indice = 0; indice < vetorPiOriginal[idOriginal].numeroManobras; indice++) {
            vetorPiCopia[idNovo].idChaveAberta[indice] = vetorPiOriginal[idOriginal].idChaveAberta[indice];
            vetorPiCopia[idNovo].estadoInicialChaveAberta[indice] = vetorPiOriginal[idOriginal].estadoInicialChaveAberta[indice];
            vetorPiCopia[idNovo].idChaveFechada[indice] = vetorPiOriginal[idOriginal].idChaveFechada[indice];
            vetorPiCopia[idNovo].estadoInicialChaveFechada[indice] = vetorPiOriginal[idOriginal].estadoInicialChaveFechada[indice];
            vetorPiCopia[idNovo].nos[indice].a = vetorPiOriginal[idOriginal].nos[indice].a;
            vetorPiCopia[idNovo].nos[indice].p = vetorPiOriginal[idOriginal].nos[indice].p;
            vetorPiCopia[idNovo].nos[indice].r = vetorPiOriginal[idOriginal].nos[indice].r;
        }
}
/**Por Leandro
 * Consiste na função "copiaDadosVetorPI()" modificada para:
 * a) alocar somente a quantidade necessária de memória;
 * b) alocar memória somente quanto o número de manobras for maior que zero
 *
 * Copia os dados de uma posição de um array to tipo VETORPI para uma nova posição de outro array do tipo VETORPI
 * @param vetorPiOriginal
 * @param vetorPiCopia
 * @param idOriginal
 * @param idNovo
 */
void copiaDadosVetorPIModificada(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idOriginal, long int idNovo)
{
    int indice;
    
    vetorPiCopia[idNovo].idAncestral = vetorPiOriginal[idOriginal].idAncestral;
    vetorPiCopia[idNovo].numeroManobras = vetorPiOriginal[idOriginal].numeroManobras;
    vetorPiCopia[idNovo].casoManobra = vetorPiOriginal[idOriginal].casoManobra;    
    vetorPiCopia[idNovo].sequenciaVerificada = vetorPiOriginal[idOriginal].sequenciaVerificada;
    if (vetorPiOriginal[idOriginal].numeroManobras > 0){
        vetorPiCopia[idNovo].idChaveAberta = Malloc(long int, vetorPiOriginal[idOriginal].numeroManobras);
        vetorPiCopia[idNovo].idChaveFechada = Malloc(long int, vetorPiOriginal[idOriginal].numeroManobras);
        vetorPiCopia[idNovo].estadoInicialChaveAberta = Malloc(BOOL, vetorPiOriginal[idOriginal].numeroManobras);
        vetorPiCopia[idNovo].estadoInicialChaveFechada = Malloc(BOOL, vetorPiOriginal[idOriginal].numeroManobras);
        vetorPiCopia[idNovo].nos = Malloc(NOSPRA, vetorPiOriginal[idOriginal].numeroManobras);

        for (indice = 0; indice < vetorPiOriginal[idOriginal].numeroManobras; indice++) {
            vetorPiCopia[idNovo].idChaveAberta[indice] = vetorPiOriginal[idOriginal].idChaveAberta[indice];
            vetorPiCopia[idNovo].estadoInicialChaveAberta[indice] = vetorPiOriginal[idOriginal].estadoInicialChaveAberta[indice];
            vetorPiCopia[idNovo].idChaveFechada[indice] = vetorPiOriginal[idOriginal].idChaveFechada[indice];
            vetorPiCopia[idNovo].estadoInicialChaveFechada[indice] = vetorPiOriginal[idOriginal].estadoInicialChaveFechada[indice];
            vetorPiCopia[idNovo].nos[indice].a = vetorPiOriginal[idOriginal].nos[indice].a;
            vetorPiCopia[idNovo].nos[indice].p = vetorPiOriginal[idOriginal].nos[indice].p;
            vetorPiCopia[idNovo].nos[indice].r = vetorPiOriginal[idOriginal].nos[indice].r;
        }
    }
}

/**Por Leandro
 * Tomando como base a função "copiaDadosVetorPIModificada()", este método copia de "vetorPiOriginal" para
 * "vetorPiCopia" informações sobre o indivíduo "idOriginal" para "idNovo".
 * A única diferença em relação a função "copiaDadosVetorPIModificada()" consiste no fato de não é feita alocação
 * de memória para "idNovo" em "vetorPiCopia". Logo, esta função destina a atualizar os valores de uma posição do vetor Pi que já foi
 * anteriormente alocada.
 * A função ""copiaDadosVetorPIModificada()" também permitiria esta ação, mas resultaria em vazamento de memória em virtude de uma alocação dupla
 * de memória.
 *
 * Memória é alocada somente quando o número de posições/informações a serem copiadas é maior que aquela já alocada e suportada pelo vetor novo.
 *
 * Copia os dados de uma posição de um array to tipo VETORPI para uma nova posição de outro array do tipo VETORPI
 * @param vetorPiOriginal
 * @param vetorPiCopia
 * @param idOriginal
 * @param idNovo
 */
void copiaDadosVetorPIPosicaoJaAlocada(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idOriginal, long int idNovo)
{
    int indice;

    if(vetorPiCopia[idNovo].numeroManobras < vetorPiOriginal[idOriginal].numeroManobras){
		vetorPiCopia[idNovo].idChaveAberta = (long int *) realloc(vetorPiCopia[idNovo].idChaveAberta, vetorPiOriginal[idOriginal].numeroManobras*sizeof(long int));
		vetorPiCopia[idNovo].idChaveFechada = (long int *) realloc(vetorPiCopia[idNovo].idChaveFechada, vetorPiOriginal[idOriginal].numeroManobras*sizeof(long int));
		vetorPiCopia[idNovo].estadoInicialChaveAberta = (BOOL *) realloc(vetorPiCopia[idNovo].estadoInicialChaveAberta, vetorPiOriginal[idOriginal].numeroManobras*sizeof(BOOL));
		vetorPiCopia[idNovo].estadoInicialChaveFechada = (BOOL *) realloc(vetorPiCopia[idNovo].estadoInicialChaveFechada, vetorPiOriginal[idOriginal].numeroManobras*sizeof(BOOL));
		vetorPiCopia[idNovo].nos = (NOSPRA *) realloc(vetorPiCopia[idNovo].nos, vetorPiOriginal[idOriginal].numeroManobras*sizeof(NOSPRA));
    }

    vetorPiCopia[idNovo].idAncestral = vetorPiOriginal[idOriginal].idAncestral;
    vetorPiCopia[idNovo].numeroManobras = vetorPiOriginal[idOriginal].numeroManobras;
    vetorPiCopia[idNovo].casoManobra = vetorPiOriginal[idOriginal].casoManobra;
    if (vetorPiOriginal[idOriginal].numeroManobras > 0){
        for (indice = 0; indice < vetorPiOriginal[idOriginal].numeroManobras; indice++) {
            vetorPiCopia[idNovo].idChaveAberta[indice] = vetorPiOriginal[idOriginal].idChaveAberta[indice];
            vetorPiCopia[idNovo].estadoInicialChaveAberta[indice] = vetorPiOriginal[idOriginal].estadoInicialChaveAberta[indice];
            vetorPiCopia[idNovo].idChaveFechada[indice] = vetorPiOriginal[idOriginal].idChaveFechada[indice];
            vetorPiCopia[idNovo].estadoInicialChaveFechada[indice] = vetorPiOriginal[idOriginal].estadoInicialChaveFechada[indice];
            vetorPiCopia[idNovo].nos[indice].a = vetorPiOriginal[idOriginal].nos[indice].a;
            vetorPiCopia[idNovo].nos[indice].p = vetorPiOriginal[idOriginal].nos[indice].p;
            vetorPiCopia[idNovo].nos[indice].r = vetorPiOriginal[idOriginal].nos[indice].r;
        }
    }
}

/* Por Leandro:
 * Copia de "matrizPiOriginalParam" para "matrizPiCopiaParam", os dados relativos ao setores que foram transferidos para a obtenção
 * de "idConfiguracaoOriginalParam".
 *
 */
void copiaColunasMatrizPI(MATRIZPI *matrizPiCopiaParam, MATRIZPI *matrizPiOriginalParam, int idConfiguracaoCopiaParam, long int idConfiguracaoOriginalParam,
		CONFIGURACAO *configuracoesOriginalParam){
	int indice, indiceNo, idColuna;
	long int setorFalta, rnpP, idNo;

	//Percorre as RNPs que podem ter perdido ou recebido setores para a formação de "idConfiguracaoOriginalParam"
	for(indice = 0; indice < configuracoesOriginalParam[idConfiguracaoOriginalParam].numeroRNPsFalta; indice++){
		rnpP = configuracoesOriginalParam[idConfiguracaoOriginalParam].idRnpFalta[indice];
		//Percorre a RNP e verifica, para cada setor, se ele foi transferido para a obtenção de "idConfiguracaoOriginalParam"
		for(indiceNo = 0; indiceNo < configuracoesOriginalParam[idConfiguracaoOriginalParam].rnp[rnpP].numeroNos; indiceNo++){
			idNo = configuracoesOriginalParam[idConfiguracaoOriginalParam].rnp[rnpP].nos[indiceNo].idNo;

			idColuna = buscaBinariaColunaPI(matrizPiOriginalParam, idNo, idConfiguracaoOriginalParam, matrizPiOriginalParam[idNo].numeroColunas - 1); //busca a coluna para a configuração "idConfiguracaoOriginalParam"
					//retornaColunaPi(matrizPiOriginalParam, vetorPiOriginalParam, idNo, idConfiguracaoOriginalParam);

			if(idColuna != -1){//Se o setor foi transferido para obtenção de "idConfiguracaoOriginalParam"
				/*Adiciona uma coluna em "matrizPiCopiaParam" e copia para ela as as informações da coluna em "matrizPiOriginalParam" que contém as
				informações de "idNo" na configuração "idConfiguracaoOriginalParam"*/
				adicionaColuna(matrizPiCopiaParam, idNo, idConfiguracaoCopiaParam, rnpP, matrizPiOriginalParam[idNo].colunas[idColuna].posicao);
			}

		}
	}
}


/**
 * @brief Encontra o nó adjacente à p ou r passado como parâmetro.
 * A função retorna a coluna da matriz PI relativa ao nó a, por meio da coluna obtém-se a RNP e o índice dela correspondente ao nó
 * Por meio da RNP identificamos os demais dados relativos ao nó.
 * 
 * @param tentativasParam: número de tentativas que serão realizadas na busca pelo nó adjacente
 * @param idNoParam
 * @param florestaParam
 * @param idRNPParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @return 
 */
COLUNAPI * determinaNoAdjacente(int tentativasParam, long int idNoParam, CONFIGURACAO florestaParam, 
        int idRNPParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam) 
{
    long int idNoA;
    int indiceColuna;
    COLUNAPI *colunaNoAdjacente = NULL;
    //seleciona um número aleatório da lista de adjacências do nó passado como parâmetro
    int indice = inteiroAleatorio(0, (grafoSetoresParam[idNoParam].numeroAdjacentes-1));
    BOOL encontrado = false;
    int tentativas = 0;
    //executa um loop até alcançar o número de tentativas ou encontrar um nó adjacente válido
    while (tentativas < tentativasParam && !encontrado) {
        //recupera o identificador do nó contido na posição aleatória selecionada
       // printf("indice %d idNoParam %ld \n", indice, idNoParam);
        idNoA = grafoSetoresParam[idNoParam].setoresAdjacentes[indice];
        if (grafoSetoresParam[idNoA].setorFalta == false) {
            //busca na matriz PI a posição e RNP do nó selecionado
            indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, idNoA, florestaParam.idConfiguracao);
            //verifica se o nó selecionado está em árvore diferente da passada como parâmetro
            if (matrizPiParam[idNoA].colunas[indiceColuna].idRNP != idRNPParam) {
                encontrado = true;
            } else //o nó está na mesma árvore deve-se selecionar outro 
            {
                indice = inteiroAleatorio(0, (grafoSetoresParam[idNoParam].numeroAdjacentes-1));
            }
        }
        tentativas++;
    }
  //  printf("tentativas %d tentativasParam %d encontrado %d \n", tentativas, tentativasParam, encontrado);
    //retorna a coluna da matriz pi do nó selecionado
    if(encontrado)
    {
        colunaNoAdjacente =  &(matrizPiParam[idNoA].colunas[indiceColuna]);
        return colunaNoAdjacente;
    }
    else
        return NULL;
}


/**
 * Realiza a busca binária nas colunas da matriz PI de um nó para localizar a posição deste na configuração
 * Recebe como parâmetros a matrizPI do nó, o índice do nó, o valor da busca e o número de colunas preenchidas.
 * Retorna o índice da coluna.
 *  
 * @param matrizPIParam
 * @param indiceNoParam
 * @param valorBusca
 * @param numeroColunasParam
 * @return 
 */
//int buscaBinariaColunaPI(MATRIZPI *matrizPIParam, int indiceNoParam, int valorBusca, int numeroColunasParam)
//{
//    int indicePrimeiro = 0;
//    int indiceMeio = (indicePrimeiro + numeroColunasParam) / 2;
////percorre as colunas até encontrar o vértice ou pesquisar toda a matriz
//    while ((indicePrimeiro <= numeroColunasParam) && (matrizPIParam[indiceNoParam].colunas[indiceMeio]->idConfiguracao!= valorBusca)) {
//        //verifica qual o próxima faixa de índices que deve ser procurada com base no valor do meio da matriz
//        if (matrizPIParam[indiceNoParam].colunas[indiceMeio]->idConfiguracao > valorBusca) {
//            numeroColunasParam = indiceMeio - 1;
//        }//Busca na parte superior
//        else
//            if (matrizPIParam[indiceNoParam].colunas[indiceMeio]->idConfiguracao < valorBusca) {
//            indicePrimeiro = indiceMeio + 1;
//        }//Busca na parte inferior
//        indiceMeio = (indicePrimeiro + numeroColunasParam) / 2;
//
//    }
//    //retorna o índice do meio
//    if (matrizPIParam[indiceNoParam].colunas[indiceMeio]->idConfiguracao == valorBusca)
//        return indiceMeio; //encontrou, retorna a posicao
//    else
//        return -1; //nao encontrou, retorna um aviso
//}

int buscaBinariaColunaPI(MATRIZPI *matrizPIParam, int indiceNoParam, int valorBusca, int numeroColunasParam) {
    int indicePrimeiro = 0;
    long int indiceMeio;
    while (indicePrimeiro <= numeroColunasParam) {
        indiceMeio = (indicePrimeiro + numeroColunasParam) / 2;
        //Busca na parte inferior
        if (matrizPIParam[indiceNoParam].colunas[indiceMeio].idConfiguracao > valorBusca) {
            numeroColunasParam = indiceMeio - 1;
        }//Busca na parte superior
        else {
            if (matrizPIParam[indiceNoParam].colunas[indiceMeio].idConfiguracao < valorBusca) {
                indicePrimeiro = indiceMeio + 1;
            }//Busca na parte inferior
            else
                return indiceMeio; //encontrou retorna a posicao
        }
    }
    return -1; //nao encontrou


}

/**
 * Realiza a busca binária na lista de matrizes PI pela matriz de um nó especifico.
 * Retorna o índice da posição.
 * 
 * @param matrizPIParam
 * @param parametroBusca
 * @param ultimaPosicaoParam
 * @return 
 */
int buscaBinariaMatrizPI(MATRIZPI *matrizPIParam, int parametroBusca, int ultimaPosicaoParam)
{
    int indicePrimeiro = 0;
    int indiceMeio = (indicePrimeiro + ultimaPosicaoParam) / 2;

    //percorre a lista de forma binária começando pelo indice central
    while ((indicePrimeiro <= ultimaPosicaoParam) && (matrizPIParam[indiceMeio].idNo != parametroBusca)) {
        //atualiza os extremos para direcionar a busca de acordo com o valor do índice do meio
        if (matrizPIParam[indiceMeio].idNo > parametroBusca) {
            ultimaPosicaoParam = indiceMeio - 1;
        }//Busca na parte superior
        else
            if (matrizPIParam[indiceMeio].idNo < parametroBusca) {
            indicePrimeiro = indiceMeio + 1;
        }//Busca na parte inferior
        indiceMeio = (indicePrimeiro + ultimaPosicaoParam) / 2;

    }
    if (matrizPIParam[indiceMeio].idNo == parametroBusca)
        return indiceMeio; //encontrou, retorna a posicao
    else
        return -1; //nao encontrou, retorna um aviso

}

/**
 * Recebe um nó como parâmetro e retorna o indice da coluna da PI correspondente ao no em sua última alteração relativa a uma
 * configuração esperada.
 * Tem como parâmetros a matrizPI do nó, o vetorPi de configurações, o nó buscado e a configuração desejada.
 * @param matrizPIParam
 * @param vetorPiParam
 * @param idNoParam
 * @param idConfiguracaoParam
 * @return 
 */
int retornaColunaPi(MATRIZPI *matrizPIParam, VETORPI *vetorPiParam, int idNoParam, int idConfiguracaoParam)
{
    int indice;
    int indiceColuna;
    int idConfiguracao;
    idConfiguracao = idConfiguracaoParam;

    indice = idNoParam;
    //verifica se o identificador passado como parâmetro é válido
    if (indice >= 0) {
        //busca nas colunas da matriz se existe uma coluna relativa a configuração informada
        indiceColuna = buscaBinariaColunaPI(matrizPIParam, indice, idConfiguracao, matrizPIParam[indice].numeroColunas - 1); //busca a coluna para a floresta desejada
        
        //enquanto nao encontra a coluna para a configuração desejada repete a busca com o valor do ancestral
        //pois, se a coluna não existe para uma configuração isso indica que nó não mudou de posição em relação ao ancestral
        while (indiceColuna < 0) 
        {

            //recupera o ancestral
            idConfiguracao = vetorPiParam[idConfiguracao].idAncestral; // inFirstForest possui o nome da primeira floresta e pode ser diferente de zero

            if (idConfiguracao >= 0)
                //refaz a busca pelas colunas
                indiceColuna = buscaBinariaColunaPI(matrizPIParam, indice, idConfiguracao, matrizPIParam[indice].numeroColunas - 1);
            else
                exit(1);

        }

        return indiceColuna;
    } else
        return -1;
}

/**
 * Procedimento para adicionar uma coluna na matriz PI de um nó ou, se existir, a coluna para a configuração atualiza os valores
 * Recebe como parâmetros a matriz P, o identificador do nó, o indentificador da configuração, o identificador da RNP, e a posição do nó na RNP
 * @param matrizPIParam
 * @param idNoParam
 * @param idConfiguracaoParam
 * @param idRNPParam
 * @param indiceParam
 */
void adicionaColuna(MATRIZPI *matrizPIParam, int idNoParam, int idConfiguracaoParam, int idRNPParam, int indiceParam) 
{
    int indiceColuna, tamanho;

    indiceColuna = matrizPIParam[idNoParam].numeroColunas;
    if (indiceColuna > 0) {
        //verifica se já existe uma coluna na configuracao referente ao identficador de configuração passado como parâmetro
        if (matrizPIParam[idNoParam].colunas[indiceColuna - 1].idConfiguracao != idConfiguracaoParam) {

            if (matrizPIParam[idNoParam].numeroColunas + 2 == matrizPIParam[idNoParam].maximoColunas) {
                tamanho = matrizPIParam[idNoParam].maximoColunas + 1000;
                //printf("numero linhas %d tamanho %d\n",matrizPIParam[idNoParam].numeroColunas, tamanho);
                COLUNAPI *temporario;
                temporario = (COLUNAPI *) realloc(matrizPIParam[idNoParam].colunas, (tamanho) * sizeof (COLUNAPI));
                // printf("usei realloc \n");
                if (temporario == NULL) {
                    printf("Nao foi possível expandir colunas da matrizPi do setor %ld\n", matrizPIParam[idNoParam].idNo);
                    exit(1);
                } else {
                    matrizPIParam[idNoParam].colunas = temporario;
                    matrizPIParam[idNoParam].maximoColunas += 1000;
                }
            }
            matrizPIParam[idNoParam].colunas[indiceColuna].idConfiguracao = idConfiguracaoParam;
            matrizPIParam[idNoParam].colunas[indiceColuna].idRNP = idRNPParam;
            matrizPIParam[idNoParam].colunas[indiceColuna].posicao = indiceParam;
            matrizPIParam[idNoParam].numeroColunas++;
        } else { //se existe atualiza a RNP e a posição
            matrizPIParam[idNoParam].colunas[indiceColuna-1].idRNP = idRNPParam;
            matrizPIParam[idNoParam].colunas[indiceColuna-1].posicao = indiceParam;
        }
    } else//é a primeira inserção na matrizPi
    {
        matrizPIParam[idNoParam].colunas[indiceColuna].idConfiguracao = idConfiguracaoParam;
        matrizPIParam[idNoParam].colunas[indiceColuna].idRNP = idRNPParam;
        matrizPIParam[idNoParam].colunas[indiceColuna].posicao = indiceParam;
        matrizPIParam[idNoParam].numeroColunas++;
    }
}

/**
 * Por Leandro
 * Descrição: com a execução dos procedimentos para correção de sequência de chaveamento passou a ocorrer casos em que
 * ao criar uma coluna para "idNoParam" na configuração "idConfiguracaoParam", já existam indivíduos com indice
 * superior a "idConfiguracaoParam". Como outra funções consideram que as colunas da matriz Pi estão ordenadas em ordem
 * crescente do valor de "idConfiguracaoParam", então, esta função consiste na função "adicionaColuna()" modificada para:
 * a) adicionar a coluna de "idNoParam" em "idConfiguracaoParam" numa posição que matriz Pi continue com suas colunas ordenadas
 * em ordem crescente do índice da configuração
 *
 * Portanto, esta função é mais genérica que a função adicionaColuna()", pois, embora faça o mesmo que ela, é capaz de lidar
 * com casos não tratados por aquela outra função.
 *
 * A descrição da função adicionaColuna()" original é: Procedimento para adicionar uma coluna na matriz PI de um nó ou, se existir, a coluna para a configuração atualiza os valores
 * Recebe como parâmetros a matriz P, o identificador do nó, o indentificador da configuração, o identificador da RNP, e a posição do nó na RNP
 * @param matrizPIParam
 * @param idNoParam
 * @param idConfiguracaoParam
 * @param idRNPParam
 * @param indiceParam
 */
void adicionaColunaMelhorada(MATRIZPI *matrizPIParam, int idNoParam, int idConfiguracaoParam, int idRNPParam, int indiceParam){
    int indiceColuna, indice;

    if (matrizPIParam[idNoParam].numeroColunas > 0) {  //Se já existe alguma coluna inserida

        indiceColuna = buscaBinariaColunaPI(matrizPIParam, idNoParam, idConfiguracaoParam, matrizPIParam[idNoParam].numeroColunas - 1); //busca nas colunas da matriz se existe uma coluna relativa a configuração "idConfiguracaoParam"
        if(indiceColuna == -1){//Ainda não existe

            if (matrizPIParam[idNoParam].numeroColunas + 2 == matrizPIParam[idNoParam].maximoColunas) {//Testa se é necessário alocar mais memória para MatrizPI
                int tamanho = matrizPIParam[idNoParam].maximoColunas + 1000;
                //printf("numero linhas %d tamanho %d\n",matrizPIParam[idNoParam].numeroColunas, tamanho);
                COLUNAPI *temporario;
                temporario = (COLUNAPI *) realloc(matrizPIParam[idNoParam].colunas, (tamanho) * sizeof (COLUNAPI));
                // printf("usei realloc \n");
                if (temporario == NULL) {
                    printf("Nao foi possível expandir colunas da matrizPi do setor %ld\n", matrizPIParam[idNoParam].idNo);
                    exit(1);
                } else {
                    matrizPIParam[idNoParam].colunas = temporario;
                    matrizPIParam[idNoParam].maximoColunas += 1000;
                }
            }
            //Determina em que posição da Matriz Pi a nova coluna deverá ser inserida
            indiceColuna = matrizPIParam[idNoParam].numeroColunas - 1;
            while(matrizPIParam[idNoParam].colunas[indiceColuna].idConfiguracao > idConfiguracaoParam)
            	indiceColuna--;
            indiceColuna++;
            for(indice = matrizPIParam[idNoParam].numeroColunas - 1; indice >= indiceColuna; indice--){ //Desloca as colunas que possuem valores maior de "idConfiguracao" a fim de liberar a posição correta para adicionar a coluna referente a "idConfiguracaoParam"
                matrizPIParam[idNoParam].colunas[indice + 1].idConfiguracao = matrizPIParam[idNoParam].colunas[indice].idConfiguracao;
                matrizPIParam[idNoParam].colunas[indice + 1].idRNP = matrizPIParam[idNoParam].colunas[indice].idRNP;
                matrizPIParam[idNoParam].colunas[indice + 1].posicao = matrizPIParam[idNoParam].colunas[indice].posicao;
            }

            matrizPIParam[idNoParam].colunas[indiceColuna].idConfiguracao = idConfiguracaoParam;
            matrizPIParam[idNoParam].colunas[indiceColuna].idRNP = idRNPParam;
            matrizPIParam[idNoParam].colunas[indiceColuna].posicao = indiceParam;
            matrizPIParam[idNoParam].numeroColunas++;
        }
        else { //se existe atualiza a RNP e a posição
            matrizPIParam[idNoParam].colunas[indiceColuna].idRNP = idRNPParam; //Por Leandro: foi modificado
            matrizPIParam[idNoParam].colunas[indiceColuna].posicao = indiceParam; //Por Leandro: foi modificado
        }
    }
    else//é a primeira inserção na matrizPi
    {
    	indiceColuna = matrizPIParam[idNoParam].numeroColunas;
        matrizPIParam[idNoParam].colunas[indiceColuna].idConfiguracao = idConfiguracaoParam;
        matrizPIParam[idNoParam].colunas[indiceColuna].idRNP = idRNPParam;
        matrizPIParam[idNoParam].colunas[indiceColuna].posicao = indiceParam;
        matrizPIParam[idNoParam].numeroColunas++;
    }
}

/**
 * Por Leandro
 * Descrição: dado um nó "idNoParam", modificado para a obtenção da configuração "idConfiguracaoParam", esta função busca a
 * coluna de "idNoParam" (para "idConfiguracaoParam") na matriz PI e exlcui tal coluna da matriz PI.
 * Esta função é aplicada nas rotinas de definição de sequência de chaveamento quando deseja-se atualizar os nós que foram transferidos
 * para a obtenção de uma configuração. Atualização esta que é motivada pela mudança das operações que levam a obtenção de tal configuração
 *
 * @param matrizPIParam
 * @param idNoParam
 * @param idConfiguracaoParam
 */
void excluiColuna(MATRIZPI *matrizPIParam, int idNoParam, int idConfiguracaoParam){
    int indiceColuna, indice;

    if (matrizPIParam[idNoParam].numeroColunas > 0) {  //Se já existe alguma coluna inserida

        indiceColuna = buscaBinariaColunaPI(matrizPIParam, idNoParam, idConfiguracaoParam, matrizPIParam[idNoParam].numeroColunas - 1); //busca nas colunas da matriz a coluna relativa a configuração "idConfiguracaoParam"

        if(indiceColuna >= 0){
        	//Casa haja colunas com índice maior que "indiceColuna", elas são copiadas e a posição excluída será a última (a de maior indice)
        	for(indice = indiceColuna; indice < matrizPIParam[idNoParam].numeroColunas - 1; indice++){
                matrizPIParam[idNoParam].colunas[indice].idConfiguracao = matrizPIParam[idNoParam].colunas[indice + 1].idConfiguracao;
                matrizPIParam[idNoParam].colunas[indice].idRNP = matrizPIParam[idNoParam].colunas[indice + 1].idRNP;
                matrizPIParam[idNoParam].colunas[indice].posicao = matrizPIParam[idNoParam].colunas[indice + 1].posicao;
        	}
        	indice = matrizPIParam[idNoParam].numeroColunas - 1;
            matrizPIParam[idNoParam].colunas[indice].idConfiguracao = -1;
            matrizPIParam[idNoParam].colunas[indice].idRNP = -1;
            matrizPIParam[idNoParam].colunas[indice].posicao = -1;
            matrizPIParam[idNoParam].numeroColunas--;
        }
    }
}

/**
 * Realiza a busca na RNP pela posição na RNP que determina o fim da subárvore enraizada no nó informado.
 * Recebe como parâmetros o ponteiro da RNP e o índice do nó raiz da subárvore
 * 
 * @param rnpParam
 * @param indiceNoPParam
 * @return 
 */
int limiteSubArvore(RNP rnpParam, int indiceNoPParam)
{
    int indice;
    int tamanho;
    indice = indiceNoPParam + 1;

       tamanho = rnpParam.numeroNos;
       //percorre a RNP até encontrar um nó de profundidade igual ou inferior ao nó raiz da subárvore ou o fim da RNP
       // printf("profundidade p %d \n", rnpParam.nos[indiceNoPParam].profundidade);
       // printf("profundidade p+1 %d \n", rnpParam.nos[indice].profundidade);
       while (indice < tamanho && rnpParam.nos[indiceNoPParam].profundidade < rnpParam.nos[indice].profundidade) {
            
            indice++;
        }
   //retorna o índice na rnp do fim da subárvore.
    return --indice;
}

/**
 * Esta função retorna um ponteiro para a lista de configurações. Recebe como parâmetros o número de RNPs de cada configuração
 * o identificador do individuo inicial e o número de configurações que compoem a lista.
 * 
 * @param numeroRNPParam
 * @param idIndividuoInicialParam
 * @param numeroConfiguracoesParam
 * @param numeroTrafosParam
 * @return 
 */
CONFIGURACAO* alocaIndividuo(int numeroRNPParam, long int idIndividuoInicialParam, long int numeroConfiguracoesParam, int numeroTrafosParam)
{
    //aloca a lista de configurações o identificador do individuo inicial é somado ao numero de configurações para permitir o uso 
    //do identificador como índice da lista.
    CONFIGURACAO *individuo = Malloc(CONFIGURACAO, (numeroConfiguracoesParam+idIndividuoInicialParam));
    long int indice;
    //percorre a lista de configurações fazendo a alocação do vetor de RNPs da configuração
    for (indice = idIndividuoInicialParam; indice <(numeroConfiguracoesParam+idIndividuoInicialParam); indice++ )
    {
        individuo[indice].numeroRNP = numeroRNPParam;
        individuo[indice].numeroRNPsFalta = 0;
        individuo[indice].rnp = Malloc(RNP, individuo[indice].numeroRNP);
        individuo[indice].idRnpFalta = Malloc(int, individuo[indice].numeroRNP);
        individuo[indice].idConfiguracao = indice;
        inicializaObjetivos(&individuo[indice], numeroTrafosParam);

        individuo[indice].dadosEletricos.corrente = NULL;
        individuo[indice].dadosEletricos.iJusante = NULL;
        individuo[indice].dadosEletricos.potencia = NULL;
        individuo[indice].dadosEletricos.vBarra = NULL;
        //imprimeIndicadoresEletricos(individuo[indice]);
    }
    return individuo;
}

/* Leandro: esta função faz a inicialização das árvores temporárias. Note que o nome dos setores-raíz das duas
 * árvores foi definido sem qualquer relação com a numeração dos nós da rede.
 *
 * @param configuracaoInicialParam é a configuração inicial da rede, na qual serão construidas as RNPs fictícias
 * @param numeroRNPFictParam é o número de RNPs Fictícias a serem inicializadas
 * @param numeroSetores é um número de setores da rede.
 * @return
 */
/*void inicializaRNPsFicticias(long int numeroSetoresParam, long int numeroRNPFicticiaParam, CONFIGURACAO *configuracaoParam) {

	int indice;
    for (indice = 0; indice < numeroRNPFicticiaParam; indice++) {
    	alocaRNP(1, &configuracaoParam->rnpFicticia[indice]);

    	configuracaoParam[0].rnpFicticia[indice].nos[0].idNo = numeroSetoresParam + 1 + indice;
    	configuracaoParam[0].rnpFicticia[indice].nos[0].profundidade = 0;
    	configuracaoParam[0].rnpFicticia[indice].numeroNos = 1;
	}
}*/

/* Por Leandro:
 * Descrição: Realiza a alocação e inicialização de "numeroRNPFicticiaParam" RNPs Fictícias para a configuração "idConfiguracaoParam"
 */
void inicializaRNPsFicticias(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int numeroRNPFicticiaParam, MATRIZPI *matrizPIParam, long int numeroSetoresParam){
	int contadorRnp;
	configuracoesParam[idConfiguracaoParam].numeroRNPFicticia = numeroRNPFicticiaParam;
	for (contadorRnp = 0; contadorRnp < numeroRNPFicticiaParam; contadorRnp++) {
		alocaRNP(1, &configuracoesParam[idConfiguracaoParam].rnpFicticia[contadorRnp]);

		configuracoesParam[idConfiguracaoParam].rnpFicticia[contadorRnp].nos[0].idNo = numeroSetoresParam + 1 + contadorRnp;
		configuracoesParam[idConfiguracaoParam].rnpFicticia[contadorRnp].nos[0].profundidade = 0;
		configuracoesParam[idConfiguracaoParam].rnpFicticia[contadorRnp].numeroNos = 1;

		adicionaColuna(matrizPIParam, configuracoesParam[idConfiguracaoParam].rnpFicticia[contadorRnp].nos[0].idNo, idConfiguracaoParam, contadorRnp + configuracoesParam[idConfiguracaoParam].numeroRNP, 0);
	}
}

/**
 * Leandro: Esta função consiste na função "alocaIndividuo()" modificada pela inserção de uma linha de comando
 * para a aloção de espaço para RNPs Fictícias, as quais conectarão grupos de setores saudáveis desligados,
 * sendo que estes serão acessadas somentes pelo operador de corte (LSO)
 * e pelo operador de reconexão (LRO) e que armazenarão os setores saudáveis fora de serviço (os que foram inici-
 * almente desligados pela falta serão salvos RNP Fictícia 1 e os não afetados mas que foram cortados serão salvos
 * na RNP fictícia 2.
 * É importante observar que as RNP fictícias não serão contadas entre as RNPs do sistema, a fim de que estas sejam
 * acessadas somente pelo LSO e LRO e por nenhuma outra função. Em outras palavras, se estas RNPs fossem adicionadas
 * na mesma variável que salva as RNP reais (configuração[].rnp) elas poderia ser acessadas por qualquer função do
 * programa, o que poderia causar erros de processamento, uma vez que estas funções não precisam manipular as árvores
 * fictícias e não foram desenvolvidas para identíficá-las e evitar esta manipulação.
 * Desta forma, as modificações realizadas no código para a inserção de corte de carga são menores.
 *
 * @param numeroRNPParam
 * @param idIndividuoInicialParam
 * @param numeroConfiguracoesParam
 * @param numeroSetoresParam é o número total de setores da rede. Este parâmentro será passado para a função "inicializaRNPsFicticias()"
 * @return
 */
CONFIGURACAO* alocaIndividuoModificada(int numeroRNPParam,  int numeroRNPFicticiaParam, long int idIndividuoInicialParam,
		long int numeroConfiguracoesParam, int numeroTrafosParam, int numeroSetoresParam)
{
    //aloca a lista de configurações o identificador do individuo inicial é somado ao numero de configurações para permitir o uso
    //do identificador como índice da lista.
    CONFIGURACAO *individuo = Malloc(CONFIGURACAO, (numeroConfiguracoesParam+idIndividuoInicialParam));
    long int indice, contador;
    //percorre a lista de configurações fazendo a alocação do vetor de RNPs da configuração
    for (indice = idIndividuoInicialParam; indice <(numeroConfiguracoesParam+idIndividuoInicialParam); indice++ )
    {
    	individuo[indice].dadosEletricos.potencia  = 0;
    	individuo[indice].dadosEletricos.corrente = 0;
    	individuo[indice].dadosEletricos.iJusante = 0;
    	individuo[indice].dadosEletricos.vBarra = 0;
//    	individuo[indice].ranqueamentoRnpsFicticias = 0;
    	individuo[indice].objetivo.potenciaTrafo = 0;

        individuo[indice].numeroRNP = numeroRNPParam;
        individuo[indice].numeroRNPsFalta = 0;
        individuo[indice].rnp = Malloc(RNP, individuo[indice].numeroRNP);
		for (contador = 0; contador < individuo[indice].numeroRNP; contador++)
			individuo[indice].rnp[contador].nos = 0;

        individuo[indice].idRnpFalta = Malloc(int, individuo[indice].numeroRNP);
        individuo[indice].idConfiguracao = indice;

        individuo[indice].numeroRNPFicticia = numeroRNPFicticiaParam; //Por Leandro
        if(numeroRNPFicticiaParam > 0){
			individuo[indice].rnpFicticia = Malloc(RNP, individuo[indice].numeroRNPFicticia); // Leandro: realiza a alocação das árvores fictícias que salvarão os setores saudáveis fora de serviço (desligados e cortados)
			for (contador = 0; contador < individuo[indice].numeroRNPFicticia; contador++)
				individuo[indice].rnpFicticia[contador].nos = 0;
        }

//        individuo[indice].ranqueamentoRnpsFicticias = Malloc(long int, individuo[indice].numeroRNPFicticia);  // Leandro: aloca memória para a variável que armazenará o ranqueamento entre as RNPs Fictícias, as quais guiarão a aplicação do LRO

        inicializaObjetivosModificada(&individuo[indice], numeroTrafosParam);
        //inicializaRNPsFicticias(numeroSetoresParam, numeroRNPFicticiaParam, &individuo[indice]); //Leandro:realiza a inicialização das RNPs Fictícias
        //imprimeIndicadoresEletricos(individuo[indice]);

    }
    return individuo;
}

/**
 * Leandro:
 *
 * @param numeroRNPParam
 * @param idIndividuoInicialParam
 * @param numeroConfiguracoesParam
 * @param numeroSetoresParam é o número total de setores da rede. Este parâmentro será passado para a função "inicializaRNPsFicticias()"
 * @return
 */
void alocaIndividuoModificadaV2(CONFIGURACAO *individuo, int numeroRNPParam,  int numeroRNPFicticiaParam, long int idIndividuoInicialParam,
		long int numeroConfiguracoesParam, int numeroTrafosParam, int numeroSetoresParam)
{
	long int indice;
    //aloca a lista de configurações o identificador do individuo inicial é somado ao numero de configurações para permitir o uso
    //do identificador como índice da lista.
    individuo = Malloc(CONFIGURACAO, (numeroConfiguracoesParam+idIndividuoInicialParam));
    //percorre a lista de configurações fazendo a alocação do vetor de RNPs da configuração
    for (indice = idIndividuoInicialParam; indice <(numeroConfiguracoesParam+idIndividuoInicialParam); indice++ )
    {
    	individuo[indice].numeroRNP = numeroRNPParam;
        individuo[indice].numeroRNPsFalta = 0;
        individuo[indice].rnp = 0;
        individuo[indice].rnp = Malloc(RNP, individuo[indice].numeroRNP);
        individuo[indice].idRnpFalta = 0;
        individuo[indice].idRnpFalta = Malloc(int, individuo[indice].numeroRNP);
        individuo[indice].idConfiguracao = indice;

        individuo[indice].numeroRNPFicticia = numeroRNPFicticiaParam; //Por Leandro
        individuo[indice].rnpFicticia = 0;
        individuo[indice].rnpFicticia = Malloc(RNP, individuo[indice].numeroRNPFicticia); // Leandro: realiza a alocação das árvores fictícias que salvarão os setores saudáveis fora de serviço (desligados e cortados)
//        individuo[indice].ranqueamentoRnpsFicticias = Malloc(long int, individuo[indice].numeroRNPFicticia);  // Leandro: aloca memória para a variável que armazenará o ranqueamento entre as RNPs Fictícias, as quais guiarão a aplicação do LRO

        inicializaObjetivosModificada(&individuo[indice], numeroTrafosParam);
        //inicializaRNPsFicticias(numeroSetoresParam, numeroRNPFicticiaParam, &individuo[indice]); //Leandro:realiza a inicialização das RNPs Fictícias
        //imprimeIndicadoresEletricos(individuo[indice]);

    }
}

/**
 * Realiza a alocação do vetor de nós da RNP passada como parâmetro. Para isso recebe como parâmetro o número de nós que compõem a árvore
 * representada pela RNP.
 * 
 * @param numeroNosParam
 * @param rnpParam
 */
void alocaRNP(int numeroNosParam, RNP *rnpParam) 
{
    //preenche os campos da RNP
    rnpParam[0].numeroNos = numeroNosParam;
    //aloca o vetor de nós
    rnpParam[0].nos = Malloc(NORNP,numeroNosParam);
}

/**
 * Realiza a impressão na tela da configuração passada como parâmetro. Para isso faz uso da função que
 * imprime uma RNP.
 * 
 * @param individuoParam
 */
void imprimeIndividuo(CONFIGURACAO  individuoParam)
{
    int indice;
   //imprime na tela o identificador do indivíduo
    printf("\nIndividuo %ld", individuoParam.idConfiguracao);
    //percorre o vetor de RNPs utilizando a função imprimeRNP para imprimir a RNP

    for (indice = 0; indice < individuoParam.numeroRNP; indice++) 
    {
        printf("%d ", indice);
        imprimeRNP(individuoParam.rnp[indice]);
     }
}
/**
 * Realiza a impressão na tela da configuração passada como parâmetro. Para isso faz uso da função que
 * imprime uma RNP.
 * 
 * @param nomeFinal
 * @param individuoParam
 */
void gravaIndividuo(char *nomeFinal, CONFIGURACAO individuoParam) {
    int indice, indice2;
    FILE *arquivo;
    char nomeArquivo[200];
    snprintf(nomeArquivo,sizeof(nomeArquivo), "SAIDA_individuo%s", nomeFinal);
    arquivo = fopen(nomeArquivo, "w");
    //imprime na tela o identificador do indivíduo
    //fprintf(arquivo, "Individuo\t%ld\n", individuoParam.idConfiguracao);
    //percorre o vetor de RNPs utilizando a função imprimeRNP para imprimir a RNP
    for (indice = 0; indice < individuoParam.numeroRNP; indice++) {
        //imprime o campo nó do vetor de nós da RNP
        for (indice2 = 0; indice2 < individuoParam.rnp[indice].numeroNos; indice2++) {
            fprintf(arquivo, "%ld\t", individuoParam.rnp[indice].nos[indice2].idNo);
        }
        fprintf(arquivo, "\n");

        //imprime o campo profundidade do vetor de nós da RNP
        for (indice2 = 0; indice2 < individuoParam.rnp[indice].numeroNos; indice2++) {
            fprintf(arquivo,"%d\t", individuoParam.rnp[indice].nos[indice2].profundidade);
        }
        fprintf(arquivo, "\n");
        fprintf(arquivo, "\n");
        
    }
    fclose(arquivo);
}


/**
 * Imprime na tela a RNP passada como parâmetro. Imprime o vetor de nós, com o identificador do nó e a profundidade
 * @param rnpParam
 */
void imprimeRNP(RNP rnpParam) 
{
    int indice;
    //imprime o número de nós da RNP
    printf("\n");
    printf("\t-- RNP -- Qtd Nos %d -- ", rnpParam.numeroNos);
    printf("\n");

    //imprime o campo nó do vetor de nós da RNP
    printf("\t\tNos:         ");
    for (indice = 0; indice < rnpParam.numeroNos; indice++) 
    {
        printf("%6ld ", rnpParam.nos[indice].idNo);
    }
    printf("\n");
    
    //imprime o campo profundidade do vetor de nós da RNP
    printf("\t\tProfundidade:");
    for (indice = 0; indice < rnpParam.numeroNos; indice++) 
    {
        printf("%6i ", rnpParam.nos[indice].profundidade);
    }
    printf("\n");

}

/**
 * Libera a memória utilizada para armazenar uma configuração. Recebe como parâmetro a configuração para ser desalocada
 * @param configuracaoParam
 */
void desalocaConfiguracao(CONFIGURACAO configuracaoParam) {
    long int indice = 0;
    //percorre o vetor de RNPs liberando a memória utilizada
  /*  while (indice < configuracaoParam.numeroRNP) {
        if(configuracaoParam.rnp[indice].nos!= NULL)
        {
            free(configuracaoParam.rnp[indice].nos);
            configuracaoParam.rnp[indice].nos = NULL;
        }

        indice++;
    }*/
    free(configuracaoParam.objetivo.potenciaTrafo);
    //libera a memória utilizada pelo vetor de RNPs
    free(configuracaoParam.rnp);
}


/**
 * Por Leandro: consiste na função "desalocaConfiguracao()" modificada para:
 * 1) Liberar espaço alocado para outras variáveis para quais a função anterior não liberava
 *
 * @param configuracaoParam
 */
void desalocaConfiguracaoModificada(CONFIGURACAO configuracaoParam) {
    long int indice = 0;

//    while (indice < configuracaoParam.numeroRNP) {  //Percorre o vetor de RNPs
//        if(configuracaoParam.rnp[indice].nos!= NULL){
//            free(configuracaoParam.rnp[indice].nos); // Libera memória ocupada por RNPs Reais
//            configuracaoParam.rnp[indice].nos = 0;
//        }
//
//        if(indice < configuracaoParam.numeroRNPFicticia){
//			if(configuracaoParam.rnpFicticia[indice].nos!= NULL){
//				free(configuracaoParam.rnpFicticia[indice].nos); // Libera memória ocupada por RNPs Ficticias
//				configuracaoParam.rnpFicticia[indice].nos = 0;
//			}
//        }
//        indice++;
//    }
    //libera a memória utilizada pelo vetor de RNPs
    if(configuracaoParam.rnp != NULL){         free(configuracaoParam.rnp);                 configuracaoParam.rnp = 0;}
    if(configuracaoParam.numeroRNPFicticia > 0)
    	if(configuracaoParam.rnpFicticia != NULL){ free(configuracaoParam.rnpFicticia); configuracaoParam.rnpFicticia = 0;}

    //Libera memória ocupada por outras variáveis
    if(configuracaoParam.idRnpFalta != NULL){                free(configuracaoParam.idRnpFalta);                configuracaoParam.idRnpFalta                = 0;}
//    if(configuracaoParam.ranqueamentoRnpsFicticias != NULL){ free(configuracaoParam.ranqueamentoRnpsFicticias); configuracaoParam.ranqueamentoRnpsFicticias = 0;}
    if(configuracaoParam.dadosEletricos.corrente != NULL){   free(configuracaoParam.dadosEletricos.corrente);   configuracaoParam.dadosEletricos.corrente   = 0;}
    if(configuracaoParam.dadosEletricos.iJusante != NULL){   free(configuracaoParam.dadosEletricos.iJusante);   configuracaoParam.dadosEletricos.iJusante   = 0;}
    if(configuracaoParam.dadosEletricos.potencia != NULL){   free(configuracaoParam.dadosEletricos.potencia);   configuracaoParam.dadosEletricos.potencia   = 0;}
    if(configuracaoParam.dadosEletricos.vBarra != NULL){     free(configuracaoParam.dadosEletricos.vBarra);     configuracaoParam.dadosEletricos.vBarra     = 0;}
    if(configuracaoParam.objetivo.potenciaTrafo !=NULL){ 	 free(configuracaoParam.objetivo.potenciaTrafo);    configuracaoParam.objetivo.potenciaTrafo    = 0;}

}

/*
 * Por Leandro:
 * Esta função faz a desalocação completa de uma variável do tipo "CONFIGURACOES".
 *
 *@param configuracoesParam é a variável do tipo "CONFIGURACOES"
 *@param numeroPosicoesAlocadasParam é o número de posições a serem desalocadas
 */
void desalocacaoCompletaConfiguracao(CONFIGURACAO *configuracoesParam, long int numeroPosicoesAlocadasParam){
	int contador;
	for (contador = 0; contador < numeroPosicoesAlocadasParam; contador++) {
		desalocaConfiguracaoModificada(configuracoesParam[contador]);
	}
	 if(configuracoesParam != NULL){
		 free(configuracoesParam);
		 configuracoesParam = 0;
	 }
	free(configuracoesParam);
}

void desalocacaoCompletaConfiguracaoModificada(CONFIGURACAO **configuracoesParam, long int numeroPosicoesAlocadasParam){
	int contador, indice;
	for (contador = 0; contador < numeroPosicoesAlocadasParam; contador++) {

		indice = 0;
	    while (indice < (*configuracoesParam)[contador].numeroRNP) {  //Percorre o vetor de RNPs
	        if((*configuracoesParam)[contador].rnp[indice].nos!= NULL){
	        	(*configuracoesParam)[contador].rnp[indice].nos = NULL;
	            free((*configuracoesParam)[contador].rnp[indice].nos); // Libera memória ocupada por RNPs Reais
	        }

	        if(indice < (*configuracoesParam)[contador].numeroRNPFicticia){
				if((*configuracoesParam)[contador].rnpFicticia[indice].nos!= NULL){
					(*configuracoesParam)[contador].rnpFicticia[indice].nos = NULL;
					free((*configuracoesParam)[contador].rnpFicticia[indice].nos); // Libera memória ocupada por RNPs Ficticias
				}
	        }
	        indice++;
	    }
	    //libera a memória utilizada pelo vetor de RNPs
	    if((*configuracoesParam)[contador].rnp != NULL){         (*configuracoesParam)[contador].rnp = NULL;		 free((*configuracoesParam)[contador].rnp);}
	    if((*configuracoesParam)[contador].rnpFicticia != NULL){ (*configuracoesParam)[contador].rnpFicticia = NULL; free((*configuracoesParam)[contador].rnpFicticia);}

	    //Libera memória ocupada por outras variáveis
	    if((*configuracoesParam)[contador].idRnpFalta != NULL){                (*configuracoesParam)[contador].idRnpFalta                = NULL; free((*configuracoesParam)[contador].idRnpFalta);}
//	    if((*configuracoesParam)[contador].ranqueamentoRnpsFicticias != NULL){ (*configuracoesParam)[contador].ranqueamentoRnpsFicticias = NULL; free((*configuracoesParam)[contador].ranqueamentoRnpsFicticias);}
	    if((*configuracoesParam)[contador].dadosEletricos.corrente != NULL){   (*configuracoesParam)[contador].dadosEletricos.corrente   = NULL; free((*configuracoesParam)[contador].dadosEletricos.corrente);}
	    if((*configuracoesParam)[contador].dadosEletricos.iJusante != NULL){   (*configuracoesParam)[contador].dadosEletricos.iJusante   = NULL; free((*configuracoesParam)[contador].dadosEletricos.iJusante);}
	    if((*configuracoesParam)[contador].dadosEletricos.potencia != NULL){   (*configuracoesParam)[contador].dadosEletricos.potencia   = NULL; free((*configuracoesParam)[contador].dadosEletricos.potencia);}
	    if((*configuracoesParam)[contador].dadosEletricos.vBarra != NULL){     (*configuracoesParam)[contador].dadosEletricos.vBarra     = NULL; free((*configuracoesParam)[contador].dadosEletricos.vBarra);}
	    if((*configuracoesParam)[contador].objetivo.potenciaTrafo !=NULL){ 	   (*configuracoesParam)[contador].objetivo.potenciaTrafo    = NULL; free((*configuracoesParam)[contador].objetivo.potenciaTrafo);}

	}
 if((*configuracoesParam)!= NULL){ (*configuracoesParam) = NULL; free(*configuracoesParam);}
}

/**
 * Método responsável por fazer a alocação do vetor de MatrizPi para cada nó da configuração, e da matrizPi de cada nó.
 * Além disso, faz a inicialização dos valores de identificação da matriz
 * Recebe como parâmetros o grafo de setores, o vetor de Matriz Pi que será alocado, e o número de colunas da matriz de cada nó. 
 * @param grafoSetoresParam
 * @param matrizPIParam
 * @param numeroMaximoColunasParam
 * @param numeroSetores
 */
void inicializaMatrizPI(GRAFOSETORES *grafoSetoresParam, MATRIZPI ** matrizPIParam, int numeroMaximoColunasParam, long int numeroSetores) 
{
    long int indice;
    
    //aloca o vetor de de matriz Pi    
    if(((*matrizPIParam) = (MATRIZPI *) malloc((numeroSetores+1) * sizeof(MATRIZPI)))==NULL)  
    {
        printf("Erro: Nao foi possivel alocar matriz PI");
        exit(1);
    }
    
    //percorre o vetor de matriz PI fazendo a alocação das matrizes e inicializando os dados
    for(indice=1; indice<=numeroSetores; indice++) {
        //insere o identificador do nó
        (*matrizPIParam)[indice].idNo = grafoSetoresParam[indice].idSetor;
        //inicializa o número de colunas preenchidas com zero
        (*matrizPIParam)[indice].numeroColunas = 0;
        (*matrizPIParam)[indice].maximoColunas = 0.2*numeroMaximoColunasParam;
        //aloca a matriz PI
      if(  ((*matrizPIParam)[indice].colunas = (COLUNAPI *) malloc((0.2*numeroMaximoColunasParam) * sizeof(COLUNAPI))) ==  NULL) 
      {
          printf("Nao foi possivel alocar matriz PI setor %ld \n", indice);
          exit(1);
      }
    }
}

/**
 * Este método realiza a alocação e inicialização do vetor pi da RNP.
 * Tem como parâmetros o numero maximo de configurações e o vetor pi.
 * 
 * @param numeroMaximoConfiguracoesParam
 * @param vetorPiParam
 */
void inicializaVetorPi(int numeroMaximoConfiguracoesParam, VETORPI **vetorPiParam)
{
    int indice;
    //aloca o vetor pi 
    (*vetorPiParam) = (VETORPI *) malloc((numeroMaximoConfiguracoesParam) * sizeof(VETORPI));
    //inicializa os campos de cada posição do vetor pi
    for (indice = 0; indice < numeroMaximoConfiguracoesParam; indice++) 
    {
        (*vetorPiParam)[indice].idAncestral = -1;
        (*vetorPiParam)[indice].numeroManobras = -1;
        (*vetorPiParam)[indice].sequenciaVerificada = false;
    }
}

/**
 * Método responsável por construir a nova RNP da RNP de origem (contém o nó p de poda) 
 * resultante da aplicação dos operadores PAO e CAO da RNP.
 * tem como parâmetros a RNP de origem atual, o ponteiro para a RNP de origem após a remoção da subárvore podada
 * o índice do nó raiz da subárvore (p), o índice final da subárvore, o vetor de matrizes Pi para atualizar a posição dos nós
 * após a remoção da subárvore, o identificador da configuração, e o índice da RNP no vetor de rnps da configuração.
 * 
 * @param rnpOrigemAtual
 * @param rnpOrigemNova
 * @param indicePParam
 * @param indiceLParam
 * @param matrizPIParam
 * @param idFlorestaParam
 * @param idRNPParam
 */
void constroiRNPOrigem(RNP rnpOrigemAtual, RNP *rnpOrigemNova, int indicePParam, 
        int indiceLParam, MATRIZPI *matrizPIParam, int idFlorestaParam, int idRNPParam)
{
    int indice, indiceNovaRnp;
    
    //copia na nova rnp os nós da rnp atual até o índice do nó de poda p
    for(indice =0; indice < indicePParam; indice++)
    {
        rnpOrigemNova[0].nos[indice].idNo = rnpOrigemAtual.nos[indice].idNo;
        rnpOrigemNova[0].nos[indice].profundidade = rnpOrigemAtual.nos[indice].profundidade;
    }
    indiceNovaRnp = indicePParam;
    //copia na nova rnp os nós da rnp atual após o índice L que indica o fim da subárvore podada
    for(indice=indiceLParam+1; indice<rnpOrigemAtual.numeroNos; indice++)
    {
        rnpOrigemNova[0].nos[indiceNovaRnp].idNo = rnpOrigemAtual.nos[indice].idNo;
        rnpOrigemNova[0].nos[indiceNovaRnp].profundidade = rnpOrigemAtual.nos[indice].profundidade;
        //adiciona na matriz Pi de cada um desses nós uma coluna, pois sua posição na RNP foi alterada
        //os nós anteriores não possuem inserção na matriz pois seus dados são os mesmos do ancestral
        adicionaColuna(matrizPIParam, rnpOrigemNova[0].nos[indiceNovaRnp].idNo, idFlorestaParam, idRNPParam, indiceNovaRnp);
        indiceNovaRnp++;
    }
}
/**
 * Método responsável por construir a nova RNP da RNP de destino (contém o nó a de inserção da subárvore podada) 
 * resultante da aplicação dos operador PAO da RNP.
 * Tem como parâmetros a RNP de origem atual, a RNP de destino atual, o ponteiro para a RNP de destino após a inserção da subárvore podada
 * o índice do nó raiz da subárvore (p), o índice final da subárvore, o índice do nó adjacente após o qual deve ser inserida a subárvore,
 * o vetor de matrizes Pi para atualizar a posição dos nós após a inserção da subárvore, o identificador da configuracao e o índice da RNP no vetor de rnps da configuração.
 * @param rnpOrigemAtual
 * @param rnpDestinoAtual
 * @param rnpDestinoNova
 * @param indicePParam
 * @param indiceLParam
 * @param indiceAParam
 * @param matrizPIParam
 * @param idFlorestaParam
 * @param idRNPParam
 */
void constroiRNPDestinoPAO(RNP rnpOrigemAtual, RNP rnpDestinoAtual, RNP *rnpDestinoNova, 
        int indicePParam, int indiceLParam, int indiceAParam, MATRIZPI *matrizPIParam, 
        int idFlorestaParam, int idRNPParam)
{
    int indiceNovo, indiceAtual;
    int tamanhoTemporario=0;
    indiceNovo = 0;
    
    //copia os nós da rnpDestino até o nó A;
    for(indiceAtual = 0; indiceAtual <= indiceAParam; indiceAtual++ )
    {
        rnpDestinoNova[0].nos[indiceNovo].idNo = rnpDestinoAtual.nos[indiceAtual].idNo;
        rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpDestinoAtual.nos[indiceAtual].profundidade;
        indiceNovo++;
    }
    
    //copia os nós da rnpOrigem referentes a subárvore podada
    for(indiceAtual = indicePParam, indiceNovo = (indiceAParam + 1); indiceAtual <=indiceLParam; indiceAtual++)
    {
       // printf("indiceA %d indiceNovo %d total %d \n", indiceAParam, indiceNovo, rnpDestinoNova[0].numeroNos);
        rnpDestinoNova[0].nos[indiceNovo].idNo = rnpOrigemAtual.nos[indiceAtual].idNo;
        rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpOrigemAtual.nos[indiceAtual].profundidade - rnpOrigemAtual.nos[indicePParam].profundidade + rnpDestinoAtual.nos[indiceAParam].profundidade +1;
        //atualiza a matriz pi de cada um desse nós fazendo a inserção dos dados de uma coluna
        adicionaColuna(matrizPIParam, rnpDestinoNova[0].nos[indiceNovo].idNo, idFlorestaParam, idRNPParam, indiceNovo);
        indiceNovo++;
    }
    //copia os demais nós da árvore destino original após a inserção da subárvore podada da árvore origem.
    tamanhoTemporario = indiceLParam-indicePParam+1;
    for (indiceNovo = (indiceAParam + tamanhoTemporario+1); indiceNovo < rnpDestinoNova[0].numeroNos; indiceNovo++) {
        rnpDestinoNova[0].nos[indiceNovo].idNo = rnpDestinoAtual.nos[indiceNovo-tamanhoTemporario].idNo;
        rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpDestinoAtual.nos[indiceNovo-tamanhoTemporario].profundidade;
        //atualiza a matriz pi de cada um desse nós fazendo a inserção dos dados de uma coluna
        adicionaColuna(matrizPIParam, rnpDestinoNova[0].nos[indiceNovo].idNo, idFlorestaParam, idRNPParam, indiceNovo);
    }
    
}


/**
 * Método responsável por construir a nova RNP da RNP de destino (contém o nó a de inserção da subárvore podada) 
 * resultante da aplicação dos operador CAO da RNP.
 * Tem como parâmetros a RNP de origem atual, a RNP de destino atual, o ponteiro para a RNP de destino após a inserção da subárvore podada
 * o índice do nó raiz da subárvore (p), o índice final da subárvore, o índice da nova raiz, o índice do nó adjacente após o qual deve ser inserida a subárvore,
 * o vetor de matrizes Pi para atualizar a posição dos nós após a inserção da subárvore, o identificador da configuracao e o índice da RNP no vetor de rnps da configuração. 
 * 
 * @param rnpOrigemAtual
 * @param rnpDestinoAtual
 * @param rnpDestinoNova
 * @param indicePParam
 * @param indiceLParam
 * @param indiceRParam
 * @param indiceAParam
 * @param matrizPIParam
 * @param idConfiguracaoParam
 * @param idRNPParam
 */
void constroiRNPDestinoCAO(RNP rnpOrigemAtual, RNP rnpDestinoAtual, RNP *rnpDestinoNova, 
        int indicePParam, int indiceLParam,int indiceRParam, int indiceAParam, 
        MATRIZPI *matrizPIParam, int idConfiguracaoParam, int idRNPParam)
{
    int indiceNovo, indiceAtual;
    int indiceLR;
    int tamanhoTemporario=0;
    int base, topo, raiz;
    indiceNovo = 0;
    
    //copia os nós da rnpDestino até o nó A;
    for(indiceAtual = 0; indiceAtual <= indiceAParam; indiceAtual++ )
    {
        rnpDestinoNova[0].nos[indiceNovo].idNo = rnpDestinoAtual.nos[indiceAtual].idNo;
        rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpDestinoAtual.nos[indiceAtual].profundidade;
        indiceNovo++;
    }
    
    //busca pelo limite da subárvore enraizada no nó R (nova raiz da subárvore)
    indiceLR = limiteSubArvore(rnpOrigemAtual, indiceRParam);
   // printf("idRNP %d tamanho origem %d indiceR %d \n", idRNPParam, rnpOrigemAtual.numeroNos, indiceRParam);
    //copia os nós da rnpOrigem referentes a subárvore podada enraizada no nó R
    for(indiceAtual = indiceRParam, indiceNovo = (indiceAParam + tamanhoTemporario + 1); indiceAtual <=indiceLR; indiceAtual++)
    {
       // printf("indiceAtual %d idNo %ld \n", indiceAtual,rnpOrigemAtual.nos[indiceAtual].idNo);
        rnpDestinoNova[0].nos[indiceNovo].idNo = rnpOrigemAtual.nos[indiceAtual].idNo;
        rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpOrigemAtual.nos[indiceAtual].profundidade - rnpOrigemAtual.nos[indiceRParam].profundidade + rnpDestinoAtual.nos[indiceAParam].profundidade +1;
        //insere uma coluna na matriz pi dos nós com os novos dados de posição
        adicionaColuna(matrizPIParam, rnpDestinoNova[0].nos[indiceNovo].idNo, idConfiguracaoParam, idRNPParam, indiceNovo);
        indiceNovo++;
    }
    
    tamanhoTemporario = indiceNovo - (indiceAParam + 1);
    base = indiceLR;
    topo = indiceRParam;
    //percorre a RNP de origem buscando pelos vértices no caminho entre p e r remontando as subárvores
    //Explicações detalhadas posteriormente... 
    while (topo > indicePParam) {
        raiz = topo;
        while (rnpOrigemAtual.nos[raiz].profundidade >= rnpOrigemAtual.nos[topo].profundidade) {
            if (raiz > 1)
                raiz--;
        }
        //copia os nós da raiz encontrada até o Topo
        //copiar os nohs ate inTop
        //printf("raiz %d topo %d tamanhoTemporario %d\n", raiz, topo, tamanhoTemporario);
        for (indiceAtual = raiz, indiceNovo = (indiceAParam + tamanhoTemporario+1); indiceAtual < topo; indiceAtual++) {
            //printf("raiz %d topo %d indiceAtual %d indiceNovo %d \n", raiz, topo, indiceAtual, indiceNovo);
            rnpDestinoNova[0].nos[indiceNovo].idNo = rnpOrigemAtual.nos[indiceAtual].idNo;
            rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpOrigemAtual.nos[indiceAtual].profundidade - rnpOrigemAtual.nos[raiz].profundidade + rnpDestinoAtual.nos[indiceAParam].profundidade + 1 + rnpOrigemAtual.nos[indiceRParam].profundidade - rnpOrigemAtual.nos[raiz].profundidade;
            adicionaColuna(matrizPIParam, rnpDestinoNova[0].nos[indiceNovo].idNo, idConfiguracaoParam, idRNPParam, indiceNovo);
            indiceNovo++;
        }
        tamanhoTemporario = indiceNovo - (indiceAParam+1);
        //verifica e copia se existem nós após a ultima subárvore copiada que pertence a subárvore atual
        indiceAtual = base + 1;
        if (indiceAtual < rnpOrigemAtual.numeroNos) {
            if (rnpOrigemAtual.nos[indiceAtual].profundidade >= rnpOrigemAtual.nos[topo].profundidade) {
                indiceNovo = (indiceAParam + tamanhoTemporario+1);
                while ((indiceAtual < rnpOrigemAtual.numeroNos) && (rnpOrigemAtual.nos[indiceAtual].profundidade >= rnpOrigemAtual.nos[topo].profundidade)) {
                    rnpDestinoNova[0].nos[indiceNovo].idNo = rnpOrigemAtual.nos[indiceAtual].idNo;
                    rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpOrigemAtual.nos[indiceAtual].profundidade - rnpOrigemAtual.nos[raiz].profundidade + rnpDestinoAtual.nos[indiceAParam].profundidade + 1 + rnpOrigemAtual.nos[indiceRParam].profundidade - rnpOrigemAtual.nos[raiz].profundidade;
                    adicionaColuna(matrizPIParam, rnpDestinoNova[0].nos[indiceNovo].idNo, idConfiguracaoParam, idRNPParam, indiceNovo);
                    indiceAtual++;
                    indiceNovo++;
                }
                //atualiza o tamanho da subárvore copiada
                tamanhoTemporario = indiceNovo - (indiceAParam + 1);
            }
        }
        //atualiza o topo e a base
        topo = raiz;
        base = --indiceAtual;
        //printf("topo %d raiz %d base %d tamanhoTemporario %d\n", topo, raiz, base, tamanhoTemporario);
    }

    //copia os demais nós da árvore destino original.
    tamanhoTemporario = indiceLParam-indicePParam+1;
    for (indiceNovo = (indiceAParam + tamanhoTemporario+1); indiceNovo < rnpDestinoNova[0].numeroNos; indiceNovo++) {
        rnpDestinoNova[0].nos[indiceNovo].idNo = rnpDestinoAtual.nos[indiceNovo-tamanhoTemporario].idNo;
        rnpDestinoNova[0].nos[indiceNovo].profundidade = rnpDestinoAtual.nos[indiceNovo-tamanhoTemporario].profundidade;
       //adiciona para esses nós uma nova coluna na matriz PI
        adicionaColuna(matrizPIParam, rnpDestinoNova[0].nos[indiceNovo].idNo, idConfiguracaoParam, idRNPParam, indiceNovo);
    }
    
}

/*void operadorCAO(CONFIGURACAO *popConfiguracaoParam, long int idNovaConfiguracaoParam, 
        long int idConfiguracaoSelecionadaParam,GRAFOSETORES *grafoSetoresParam, 
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam, 
        double VFParam, DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam, RNPSETORES *matrizB, 
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam,
        ESTADOCHAVE *estadoInicial, LISTACHAVES *listaChavesParam,
        long int *numChaveAberta, long int *numChaveFechada)
{
    COLUNAPI *colunaNoA;
    long int noA, noP, noR, noRaizP;
    int indiceL, indiceP, indiceR, indiceA, indice;
    long int idChaveAberta, idChaveFechada;
    int rnpP, rnpA;
    int profundidadeA;
    int tamanhoSubarvore;
    noA = 0;
    //Determinação dos nós p,r e a, para aplicação do operador
    //Executa até obter um nó a
    while (noA == 0) {
        //o no P sorteado nao pode ser a raiz da árvore
        rnpP = inteiroAleatorio(0, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP - 1));
        //verifica se a árvore possui mais de um nó para poder aplicar o operador
        if (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos > 1) {
            indiceP = inteiroAleatorio(1, (popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos - 1));
            noP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].idNo;
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], indiceP);
            //determinação do no R
            if (indiceL == indiceP) //nesse caso não existe outro nó para religar a subárvore
                indiceR = indiceP;
            else
                indiceR = inteiroAleatorio(indiceP, indiceL);
           // printf("indiceR %d indiceP %d indiceL %d \n", indiceR, indiceP, indiceL);
            noR = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceR].idNo;
            //determinação do no A adjacente a R
            colunaNoA = determinaNoAdjacente(maximoTentativas, noR, popConfiguracaoParam[idConfiguracaoSelecionadaParam], rnpP, grafoSetoresParam, matrizPiParam, vetorPiParam);
            if (colunaNoA != NULL) {
                //recupera os dados do nó a selecionado
                rnpA = colunaNoA[0].idRNP;
                indiceA = colunaNoA[0].posicao;
                noA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].idNo;
                profundidadeA = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].nos[indiceA].profundidade;
            }
        }
    }
   //localiza quais chaves serão aberta e fechada
    indice = indiceP-1;
    while(indice>0 && popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].profundidade>=popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indiceP].profundidade)
    {
        indice--;
    }
    noRaizP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].nos[indice].idNo;
    for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes; indice++)
    {
        if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
            idChaveAberta = grafoSetoresParam[noP].idChavesAdjacentes[indice];
         }
    
    for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes; indice++)
    {
            if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
                idChaveFechada = grafoSetoresParam[noR].idChavesAdjacentes[indice];
    }
    
    //printf("rnpP %d rnpA %d noP %ld noR %ld noA %ld\n",rnpP, rnpA, noP, noR, noA);
    //calcula o tamanho da subárvore
    tamanhoSubarvore = indiceL-indiceP+1;
    //faz a alocação das novas RNPs
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA].numeroNos+tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA]);
    alocaRNP(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP].numeroNos-tamanhoSubarvore, &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP]);
    //Obtém a nova rnp de origem, sem a subárvore podada
    constroiRNPOrigem(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpP], indiceP, indiceL,matrizPiParam, idNovaConfiguracaoParam, rnpP);
    //Obtém a nova rnp de destino, inserindo a subárvore podada
    constroiRNPDestinoCAO(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpP], popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[rnpA], &popConfiguracaoParam[idNovaConfiguracaoParam].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPiParam, idNovaConfiguracaoParam, rnpA);
    
    atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam, idConfiguracaoSelecionadaParam, &idChaveAberta, &idChaveFechada,1);
   //copia os ponteiros das rnps não alteradas
    for (indice = 0; indice <popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNP; indice++ )
    {
        if (indice != rnpP && indice != rnpA)
        {
            popConfiguracaoParam[idNovaConfiguracaoParam].rnp[indice] = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnp[indice];
        }
    }
    //imprimeIndividuo(popConfiguracaoParam[idNovaConfiguracaoParam]);
    numeroManobras(popConfiguracaoParam, idChaveAberta, idChaveFechada, listaChavesParam,
      idConfiguracaoSelecionadaParam, estadoInicial, numChaveAberta, numChaveFechada, idNovaConfiguracaoParam);
    avaliaConfiguracao(false, popConfiguracaoParam, rnpA, rnpP, idNovaConfiguracaoParam, 
            dadosTrafoParam, numeroTrafosParam, popConfiguracaoParam[idNovaConfiguracaoParam].numeroRNP, 
            indiceRegulador, dadosRegulador, dadosAlimentadorParam, VFParam, idConfiguracaoSelecionadaParam, matrizB,
            ZParam, maximoCorrenteParam, numeroBarrasParam, false);
}*/

/**
 * Função responsável por realizar a cópia de uma configuração em outra. 
 * Percorrer as RNPs de cada alimentador copiando os elementos na nova configuração.
 * Também realiza a cópia dos valores dos objetivos.
 * 
 * @param configuracoesParam ponteiro do tipo CONFIGURACAO para a configuração original.
 * @param configuracoesParam2 ponteiro do tipo CONFIGURACAO para a configuração destino da cópia.
 * @param idIndividuoAtual inteiro com o identificador do indivíduo original
 * @param idNovoIndividuo inteiro com o identificador para a cópia
 * @param matrizPIParam ponteiro para a estrutura do tipo MATRIZPI
 */
void copiaIndividuo(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2, 
        long int idIndividuoAtual, long int idNovoIndividuo, MATRIZPI *matrizPIParam)
{
    int contadorRnp, contadorNos;
    for(contadorRnp =0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNP; contadorRnp++)
    {
        alocaRNP(configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos, &configuracoesParam2[idNovoIndividuo].rnp[contadorRnp]);
        for (contadorNos = 0; contadorNos < configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos; contadorNos++) {
            configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos];
            if (configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo > 0)
                adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp, contadorNos);
        }
        configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].fitnessRNP;
       // atualizaVetorPi(vetorPiParam, idNovoIndividuo,-1, NULL, NULL, 0,-1, NULL,NULL);
    }
    //copia os valores do objetivos
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoTrafo = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorDemandaAlimentador = configuracoesParam[idIndividuoAtual].objetivo.maiorDemandaAlimentador;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAutomaticas = configuracoesParam[idIndividuoAtual].objetivo.manobrasAutomaticas;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasManuais = configuracoesParam[idIndividuoAtual].objetivo.manobrasManuais;
    configuracoesParam2[idNovoIndividuo].objetivo.menorTensao = configuracoesParam[idIndividuoAtual].objetivo.menorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.perdasResistivas = configuracoesParam[idIndividuoAtual].objetivo.perdasResistivas;
    configuracoesParam2[idNovoIndividuo].objetivo.quedaMaxima = configuracoesParam[idIndividuoAtual].objetivo.quedaMaxima;
}

/**
 * POr Leandro:
 * Consiste na função "copiaIndividuo()" modificada para:
 * (a) copiar também as RNPs Fictícias;
 * (b) copia informações de todos as variáveis existente no struct "objetivo"
 *
 * Em relação a função "copiaIndividuoMelhorada()" esta função, assim como a função "copiaIndividuo()", não copia
 * informações relacionadas aos dados elétricos
 *
 * Função responsável por realizar a cópia de uma configuração em outra.
 * Percorrer as RNPs de cada alimentador copiando os elementos na nova configuração.
 * Também realiza a cópia dos valores dos objetivos.
 *
 * @param configuracoesParam ponteiro do tipo CONFIGURACAO para a configuração original.
 * @param configuracoesParam2 ponteiro do tipo CONFIGURACAO para a configuração destino da cópia.
 * @param idIndividuoAtual inteiro com o identificador do indivíduo original
 * @param idNovoIndividuo inteiro com o identificador para a cópia
 * @param matrizPIParam ponteiro para a estrutura do tipo MATRIZPI
 */
void copiaIndividuoModificada(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2,
        long int idIndividuoAtual, long int idNovoIndividuo, MATRIZPI *matrizPIParam)
{
    int contadorRnp, contadorNos;
    //Copia as RNPs Reais
    for(contadorRnp =0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNP; contadorRnp++)
    {
        alocaRNP(configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos, &configuracoesParam2[idNovoIndividuo].rnp[contadorRnp]);
        for (contadorNos = 0; contadorNos < configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos; contadorNos++) {
            configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos];
            if (configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo > 0)
                adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp, contadorNos);
        }
        configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].fitnessRNP;
       // atualizaVetorPi(vetorPiParam, idNovoIndividuo,-1, NULL, NULL, 0,-1, NULL,NULL);
    }

    //Copia as RNPs Fictícias
    for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNPFicticia; contadorRnp++)
    {
        alocaRNP(configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].numeroNos, &configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp]);

        for (contadorNos = 0; contadorNos < configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].numeroNos; contadorNos++){
            configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos] = configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].nos[contadorNos];
            if (configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos].idNo > 0)
            	adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp + configuracoesParam2[idNovoIndividuo].numeroRNP, contadorNos);
        }

        configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].fitnessRNP;
    }
    configuracoesParam2[idNovoIndividuo].numeroRNPFicticia = configuracoesParam[idIndividuoAtual].numeroRNPFicticia;

    //copia os valores do objetivos
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoTrafo = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorDemandaAlimentador = configuracoesParam[idIndividuoAtual].objetivo.maiorDemandaAlimentador;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAutomaticas = configuracoesParam[idIndividuoAtual].objetivo.manobrasAutomaticas;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasManuais = configuracoesParam[idIndividuoAtual].objetivo.manobrasManuais;
    configuracoesParam2[idNovoIndividuo].objetivo.menorTensao = configuracoesParam[idIndividuoAtual].objetivo.menorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.perdasResistivas = configuracoesParam[idIndividuoAtual].objetivo.perdasResistivas;
    configuracoesParam2[idNovoIndividuo].objetivo.quedaMaxima = configuracoesParam[idIndividuoAtual].objetivo.quedaMaxima;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasRestabelecimento = configuracoesParam[idIndividuoAtual].objetivo.manobrasRestabelecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAlivio = configuracoesParam[idIndividuoAtual].objetivo.manobrasAlivio;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAposChaveamento = configuracoesParam[idIndividuoAtual].objetivo.manobrasAposChaveamento;
    configuracoesParam2[idNovoIndividuo].objetivo.ponderacao = configuracoesParam[idIndividuoAtual].objetivo.ponderacao;
    configuracoesParam2[idNovoIndividuo].objetivo.rank = configuracoesParam[idIndividuoAtual].objetivo.rank;
    configuracoesParam2[idNovoIndividuo].objetivo.fronteira = configuracoesParam[idIndividuoAtual].objetivo.fronteira;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisTransferidos = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisTransferidos;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresDesligadosEmCorteDeCarga = configuracoesParam[idIndividuoAtual].objetivo.consumidoresDesligadosEmCorteDeCarga;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.seca = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.seca;
    configuracoesParam2[idNovoIndividuo].objetivo.noMenorTensao = configuracoesParam[idIndividuoAtual].objetivo.noMenorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.noMaiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.noMaiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.idTrafoMaiorCarregamento = configuracoesParam[idIndividuoAtual].objetivo.idTrafoMaiorCarregamento;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaRede = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaRede;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaTrafo = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.tempo = configuracoesParam[idIndividuoAtual].objetivo.tempo;

    configuracoesParam2[idNovoIndividuo].objetivo.potenciaTotalNaoSuprida = configuracoesParam[idIndividuoAtual].objetivo.potenciaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade;

    configuracoesParam2[idNovoIndividuo].objetivo.energiaTotalNaoSuprida = configuracoesParam[idIndividuoAtual].objetivo.energiaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;

}

/**
 * Por Leandro:
 * Consiste na função "copiaIndividuo()" modificada para:
 * (a) copiar também as RNPs Fictícias;
 * (b) copiar informações de todos as variáveis existente no struct "objetivo"
 *
 * Em relação a função "copiaIndividuoMelhorada()" esta função, assim como a função "copiaIndividuo()", não copia
 * informações relacionadas aos dados elétricos
 *
 * Função responsável por realizar a cópia de uma configuração em outra.
 * Percorrer as RNPs de cada alimentador copiando os elementos na nova configuração.
 * Também realiza a cópia dos valores dos objetivos.
 *
 * @param configuracoesParam ponteiro do tipo CONFIGURACAO para a configuração original.
 * @param configuracoesParam2 ponteiro do tipo CONFIGURACAO para a configuração destino da cópia.
 * @param idIndividuoAtual inteiro com o identificador do indivíduo original
 * @param idNovoIndividuo inteiro com o identificador para a cópia
 * @param matrizPIParam ponteiro para a estrutura do tipo MATRIZPI
 */
void copiaPonteirosIndividuo(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2,
        long int idIndividuoAtual, long int idNovoIndividuo, MATRIZPI *matrizPIParam)
{
    int contadorRnp, contadorNos;
    //Copia as RNPs Reais
    for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNP; contadorRnp++) {

    	configuracoesParam2[idNovoIndividuo].rnp[contadorRnp] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp];

        for (contadorNos = 0; contadorNos < configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos; contadorNos++) {
            if (configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo > 0)
                adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp, contadorNos);
        }
        configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].fitnessRNP;
    }

    //Copia as RNPs Fictícias
    for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNPFicticia; contadorRnp++) {

    	configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp] = configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp];

        for (contadorNos = 0; contadorNos < configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].numeroNos; contadorNos++){
            if (configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos].idNo > 0)
            	adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp + configuracoesParam2[idNovoIndividuo].numeroRNP, contadorNos);
        }
        configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].fitnessRNP;
    }
    configuracoesParam2[idNovoIndividuo].numeroRNPFicticia = configuracoesParam[idIndividuoAtual].numeroRNPFicticia;

    //copia os valores do objetivos
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoTrafo = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorDemandaAlimentador = configuracoesParam[idIndividuoAtual].objetivo.maiorDemandaAlimentador;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAutomaticas = configuracoesParam[idIndividuoAtual].objetivo.manobrasAutomaticas;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasManuais = configuracoesParam[idIndividuoAtual].objetivo.manobrasManuais;
    configuracoesParam2[idNovoIndividuo].objetivo.menorTensao = configuracoesParam[idIndividuoAtual].objetivo.menorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.perdasResistivas = configuracoesParam[idIndividuoAtual].objetivo.perdasResistivas;
    configuracoesParam2[idNovoIndividuo].objetivo.quedaMaxima = configuracoesParam[idIndividuoAtual].objetivo.quedaMaxima;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasRestabelecimento = configuracoesParam[idIndividuoAtual].objetivo.manobrasRestabelecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAlivio = configuracoesParam[idIndividuoAtual].objetivo.manobrasAlivio;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAposChaveamento = configuracoesParam[idIndividuoAtual].objetivo.manobrasAposChaveamento;
    configuracoesParam2[idNovoIndividuo].objetivo.ponderacao = configuracoesParam[idIndividuoAtual].objetivo.ponderacao;
    configuracoesParam2[idNovoIndividuo].objetivo.rank = configuracoesParam[idIndividuoAtual].objetivo.rank;
    configuracoesParam2[idNovoIndividuo].objetivo.fronteira = configuracoesParam[idIndividuoAtual].objetivo.fronteira;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisTransferidos = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisTransferidos;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresDesligadosEmCorteDeCarga = configuracoesParam[idIndividuoAtual].objetivo.consumidoresDesligadosEmCorteDeCarga;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.seca = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.seca;
    configuracoesParam2[idNovoIndividuo].objetivo.noMenorTensao = configuracoesParam[idIndividuoAtual].objetivo.noMenorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.noMaiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.noMaiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.idTrafoMaiorCarregamento = configuracoesParam[idIndividuoAtual].objetivo.idTrafoMaiorCarregamento;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaRede = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaRede;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaTrafo = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.tempo = configuracoesParam[idIndividuoAtual].objetivo.tempo;

    configuracoesParam2[idNovoIndividuo].objetivo.potenciaTotalNaoSuprida = configuracoesParam[idIndividuoAtual].objetivo.potenciaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade;

    configuracoesParam2[idNovoIndividuo].objetivo.energiaTotalNaoSuprida = configuracoesParam[idIndividuoAtual].objetivo.energiaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;

}

/**
 * Por Leandro: consiste na função "copiaIndividuo" modificada para:
 * 1) corrigir a atualização da matriz PI (isto foi desfeito.)
 * 2) copiar também os dados elétricos
 * 3) copiar todos os campos existentes na variável "OBJETIVOS"
 * 4) copiar informações RNPs Fictícias
 *
 * Portanto, esta função é responsável por realizar a cópia de uma configuração em outra.
 * Percorrer as RNPs de cada alimentador copiando os elementos na nova configuração.
 * Também realiza a cópia dos valores dos objetivos.
 *
 * @param configuracoesParam ponteiro do tipo CONFIGURACAO para a configuração original.
 * @param configuracoesParam2 ponteiro do tipo CONFIGURACAO para a configuração destino da cópia.
 * @param idIndividuoAtual inteiro com o identificador do indivíduo original
 * @param idNovoIndividuo inteiro com o identificador para a cópia
 */
void copiaIndividuoMelhorada(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2, long int idIndividuoAtual,
		long int idNovoIndividuo, MATRIZPI *matrizPIParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam, int numeroTrafosParam){
    int contadorRnp, contadorNos, contadorBarras, indiceI;
    RNPSETOR rnpSetorSR;
    long int noR, noS, noN;
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = Malloc(long int, 200);

    configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante = 0;
    configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente = 0;
    configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia = 0;
    configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra = 0;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo = 0;

    configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));

    //Copia as RNPs
    for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNP; contadorRnp++)
    {
        alocaRNP(configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos, &configuracoesParam2[idNovoIndividuo].rnp[contadorRnp]);
        contadorNos = 0;
        configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos];
        adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp, contadorNos);
        noProf[configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos].profundidade] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos].idNo;
        for (contadorNos = 1; contadorNos < configuracoesParam[idIndividuoAtual].rnp[contadorRnp].numeroNos; contadorNos++) {
            configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos];
            if (configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo > 0)
            	adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp, contadorNos);

            //Copia os valores dos "dados elétricos" para as barras presentes no Setor em questão
            noS = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos].idNo;
            noR = noProf[configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(rnpSetoresParam, noS, noR);
            for (contadorBarras = 0; contadorBarras < rnpSetorSR.numeroNos; contadorBarras++) {
            	noN = rnpSetorSR.nos[contadorBarras].idNo;
            	configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra[noN] = configuracoesParam[idIndividuoAtual].dadosEletricos.vBarra[noN];
            	configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia[noN] = configuracoesParam[idIndividuoAtual].dadosEletricos.potencia[noN];
            	configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante[noN] = configuracoesParam[idIndividuoAtual].dadosEletricos.iJusante[noN];
            	configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente[noN] = configuracoesParam[idIndividuoAtual].dadosEletricos.corrente[noN];
            }
            noProf[configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos].profundidade] = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].nos[contadorNos].idNo;

        }
        configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].fitnessRNP;
       // atualizaVetorPi(vetorPiParam, idNovoIndividuo,-1, NULL, NULL, 0,-1, NULL,NULL);
    }
    configuracoesParam2[idNovoIndividuo].numeroRNP = configuracoesParam[idIndividuoAtual].numeroRNP;
    configuracoesParam2[idNovoIndividuo].numeroRNPsFalta = configuracoesParam[idIndividuoAtual].numeroRNPsFalta;
    configuracoesParam2[idNovoIndividuo].idRnpFalta = Malloc(int, configuracoesParam2[idNovoIndividuo].numeroRNPsFalta);
    for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNPsFalta; contadorRnp++)
    	configuracoesParam2[idNovoIndividuo].idRnpFalta[contadorRnp] = configuracoesParam[idIndividuoAtual].idRnpFalta[contadorRnp];


    //Copia as RNPs Fictícias
    for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNPFicticia; contadorRnp++)
    {
        alocaRNP(configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].numeroNos, &configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp]);

        for (contadorNos = 0; contadorNos < configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].numeroNos; contadorNos++){
            configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos] = configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].nos[contadorNos];
            if (configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos].idNo > 0)
            	adicionaColuna(matrizPIParam, configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].nos[contadorNos].idNo, configuracoesParam2[idNovoIndividuo].idConfiguracao, contadorRnp + configuracoesParam2[idNovoIndividuo].numeroRNP, contadorNos);
        }

        configuracoesParam2[idNovoIndividuo].rnpFicticia[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnpFicticia[contadorRnp].fitnessRNP;
    }
    configuracoesParam2[idNovoIndividuo].numeroRNPFicticia = configuracoesParam[idIndividuoAtual].numeroRNPFicticia;

    //copia os valores do objetivos
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresSemFornecimento = configuracoesParam[idIndividuoAtual].objetivo.consumidoresSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoTrafo = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorDemandaAlimentador = configuracoesParam[idIndividuoAtual].objetivo.maiorDemandaAlimentador;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAutomaticas = configuracoesParam[idIndividuoAtual].objetivo.manobrasAutomaticas;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasManuais = configuracoesParam[idIndividuoAtual].objetivo.manobrasManuais;
    configuracoesParam2[idNovoIndividuo].objetivo.menorTensao = configuracoesParam[idIndividuoAtual].objetivo.menorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.perdasResistivas = configuracoesParam[idIndividuoAtual].objetivo.perdasResistivas;
    configuracoesParam2[idNovoIndividuo].objetivo.quedaMaxima = configuracoesParam[idIndividuoAtual].objetivo.quedaMaxima;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasRestabelecimento = configuracoesParam[idIndividuoAtual].objetivo.manobrasRestabelecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAlivio = configuracoesParam[idIndividuoAtual].objetivo.manobrasAlivio;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAposChaveamento = configuracoesParam[idIndividuoAtual].objetivo.manobrasAposChaveamento;
    configuracoesParam2[idNovoIndividuo].objetivo.ponderacao = configuracoesParam[idIndividuoAtual].objetivo.ponderacao;
    configuracoesParam2[idNovoIndividuo].objetivo.rank = configuracoesParam[idIndividuoAtual].objetivo.rank;
    configuracoesParam2[idNovoIndividuo].objetivo.fronteira = configuracoesParam[idIndividuoAtual].objetivo.fronteira;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisTransferidos = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisTransferidos;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresDesligadosEmCorteDeCarga = configuracoesParam[idIndividuoAtual].objetivo.consumidoresDesligadosEmCorteDeCarga;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.seca = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.seca;
    configuracoesParam2[idNovoIndividuo].objetivo.noMenorTensao = configuracoesParam[idIndividuoAtual].objetivo.noMenorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.noMaiorCarregamentoRede = configuracoesParam[idIndividuoAtual].objetivo.noMaiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.idTrafoMaiorCarregamento = configuracoesParam[idIndividuoAtual].objetivo.idTrafoMaiorCarregamento;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaRede = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaRede;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaTrafo = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.tempo = configuracoesParam[idIndividuoAtual].objetivo.tempo;

    configuracoesParam2[idNovoIndividuo].objetivo.potenciaTotalNaoSuprida = configuracoesParam[idIndividuoAtual].objetivo.potenciaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade;

    configuracoesParam2[idNovoIndividuo].objetivo.energiaTotalNaoSuprida = configuracoesParam[idIndividuoAtual].objetivo.energiaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;

    for (indiceI = 0; indiceI <= numeroTrafosParam; indiceI++)
    	configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo[indiceI] = configuracoesParam[idNovoIndividuo].objetivo.potenciaTrafo[indiceI];

    free(noProf);
}

/**
 * Gera a nova representação nó-profundidade do alimentador que recebe o bloco de setores transferidos pelos operadores PAO E CAO. 
 * @param configuracoesParam
 * @param rnpP
 * @param rnpA
 * @param indiceL
 * @param indiceP
 * @param indiceA
 * @param indiceR
 * @param matrizPiParam
 * @param idConfiguracaoParam
 * @param rnpDestino
 * @param idNovaConfiguracaoParam
 */
void constroiRNPDestino(CONFIGURACAO *configuracoesParam, int rnpP, 
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam, 
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam)
{
    int indiceI, indiceF;
    int contador, indice1, indice2;
    int tamanhoTemporario;
    /*##############     MONTA A ARVORE Tto (Arvore do no a)  ###############*/
    indiceI = indiceL;
    indiceF = 0;
    contador = 0;
    tamanhoTemporario = 0;
    
    while (indiceR >= indiceP) {
        for (indice1 = indiceR, indice2 = indiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
            if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                if ((configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade || indice1 == indiceR) 
                        && indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                   rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indiceA].profundidade + 1 + contador;
                    adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA, indice2);
                    
                    indice2++;

                } 
                else 
                    indice1 = indiceL + 1;
            }
        }
        tamanhoTemporario = indice2 - (indiceA+ 1);
        indiceI = indiceR;
        indiceF = indiceR + tamanhoTemporario - 1;
        indice1 = indiceR;
        while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade - 1)
            indice1--;
        indiceR = indice1;
        contador++;
    }

    for (indice1 = 0; indice1 <= indiceA; indice1++) {
         rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].idNo;
         rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].profundidade;
    }

    for (indice1 = indiceA +tamanhoTemporario + 1; indice1 < rnpDestino[0].numeroNos; indice1++) 
    {
       rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1 - tamanhoTemporario].idNo;
       rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1 - tamanhoTemporario].profundidade;
       adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice1].idNo, idNovaConfiguracaoParam, rnpA, indice1);


    }
}

/**
 * Gera a nova representação nó-profundidade do alimentador quando a transferência dos setores ocorre no mesmo alimentador.
 * @param configuracoesParam
 * @param rnpP
 * @param rnpA
 * @param indiceL
 * @param indiceP
 * @param indiceA
 * @param indiceR
 * @param matrizPiParam
 * @param idConfiguracaoParam
 * @param rnpDestino
 * @param idNovaConfiguracaoParam
 */
void constroiRNPOrigemDestino(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam) {
    int indiceI, indiceF;
    int novoIndiceA;
    int contador, indice1, indice2;
    int tamanhoTemporario;
    /*##############     MONTA A ARVORE Tto (Arvore do no a)  ###############*/
    indiceI = indiceL;
    indiceF = 0;
    contador = 0;
    tamanhoTemporario = 0;
    indice2 = 0;
    //copia os setores na RNP que estão entre a raiz do alimentador e o ponto de religação do trecho
    for (indice1 = 0; indice1 <= indiceA; indice1++) {
        if (indice1 < indiceP || indice1 > indiceL) {
            rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].idNo;
            rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].profundidade;
            adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA, indice2);
            indice2++;
        }
    }
    if (indiceA > indiceP) {//o ponto de insercao do trecho transferido está depois do trecho na rnp original
        novoIndiceA = indice2 - 1;
        while (indiceR >= indiceP) {
            for (indice1 = indiceR, indice2 = novoIndiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
                if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                    if ((configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade || indice1 == indiceR)
                            && indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                        rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                        rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indiceA].profundidade + 1 + contador;
                        adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA, indice2);
                        indice2++;
                    } else
                        indice1 = indiceL + 1;
                }
            }
            tamanhoTemporario = indice2 - (novoIndiceA + 1);
            indiceI = indiceR;
            indiceF = indiceR + tamanhoTemporario - 1;
            indice1 = indiceR;
            while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade - 1)
                indice1--;
            indiceR = indice1;
            contador++;
        }
        for (indice1 = indiceA + 1; indice1 < rnpDestino[0].numeroNos; indice1++) {
                rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].idNo;
                rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].profundidade;
                adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice1].idNo, idNovaConfiguracaoParam, rnpA, indice1);
        }

    } else { //o ponto de insercao do trecho transferido está antes do trecho
        while (indiceR >= indiceP) {
            for (indice1 = indiceR, indice2 = indiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
                if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                    if ((configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade || indice1 == indiceR)
                            && indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                        rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                        rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indiceA].profundidade + 1 + contador;
                        adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA, indice2);

                        indice2++;
                    } else
                        indice1 = indiceL + 1;
                }
            }
            tamanhoTemporario = indice2 - (indiceA + 1);
            indiceI = indiceR;
            indiceF = indiceR + tamanhoTemporario - 1;
            indice1 = indiceR;
            while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade - 1)
                indice1--;
            indiceR = indice1;
            contador++;
        }
        indice2 = indiceA + tamanhoTemporario + 1;
        for (indice1 = indiceA + 1; indice1 < rnpDestino[0].numeroNos; indice1++) {
            if (indice1 < indiceP || indice1 > indiceL) {
                rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].idNo;
                rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].profundidade;
                adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA, indice2);
                indice2++;
            }
        }
    }


}

/**
 * Gera a nova representação nó-profundidade do alimentador quando a transferência dos setores ocorre no mesmo alimentador.
 * @param configuracoesParam
 * @param rnpP
 * @param rnpA
 * @param indiceL
 * @param indiceP
 * @param indiceA
 * @param indiceR
 * @param matrizPiParam
 * @param idConfiguracaoParam
 * @param rnpDestino
 * @param idNovaConfiguracaoParam
 */
void constroiRNPOrigemDestinoFicticia(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam) {
    int indiceI, indiceF;
    int novoIndiceA;
    int contador, indice1, indice2;
    int tamanhoTemporario;
    /*##############     MONTA A ARVORE Tto (Arvore do no a)  ###############*/
    indiceI = indiceL;
    indiceF = 0;
    contador = 0;
    tamanhoTemporario = 0;
    indice2 = 0;
    //copia os setores na RNP que estão entre a raiz do alimentador e o ponto de religação do trecho
    for (indice1 = 0; indice1 <= indiceA; indice1++) {
        if (indice1 < indiceP || indice1 > indiceL) {
            rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].idNo;
            rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].profundidade;
            adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP, indice2);
            indice2++;
        }
    }
    if (indiceA > indiceP) {//o ponto de insercao do trecho transferido está depois do trecho na rnp original
        novoIndiceA = indice2 - 1;
        while (indiceR >= indiceP) {
            for (indice1 = indiceR, indice2 = novoIndiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
                if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                    if ((configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade || indice1 == indiceR)
                            && indice1 < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].numeroNos) {
                        rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].idNo;
                        rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indiceA].profundidade + 1 + contador;
                        adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP, indice2);
                        indice2++;
                    } else
                        indice1 = indiceL + 1;
                }
            }
            tamanhoTemporario = indice2 - (novoIndiceA + 1);
            indiceI = indiceR;
            indiceF = indiceR + tamanhoTemporario - 1;
            indice1 = indiceR;
            while (configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade - 1)
                indice1--;
            indiceR = indice1;
            contador++;
        }
        for (indice1 = indiceA + 1; indice1 < rnpDestino[0].numeroNos; indice1++) {
                rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].idNo;
                rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].profundidade;
                adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice1].idNo, idNovaConfiguracaoParam, rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP, indice1);
        }

    } else { //o ponto de insercao do trecho transferido está antes do trecho
        while (indiceR >= indiceP) {
            for (indice1 = indiceR, indice2 = indiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
                if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                    if ((configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade || indice1 == indiceR)
                            && indice1 < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].numeroNos) {
                        rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].idNo;
                        rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indiceA].profundidade + 1 + contador;
                        adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP, indice2);

                        indice2++;
                    } else
                        indice1 = indiceL + 1;
                }
            }
            tamanhoTemporario = indice2 - (indiceA + 1);
            indiceI = indiceR;
            indiceF = indiceR + tamanhoTemporario - 1;
            indice1 = indiceR;
            while (configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade - 1)
                indice1--;
            indiceR = indice1;
            contador++;
        }
        indice2 = indiceA + tamanhoTemporario + 1;
        for (indice1 = indiceA + 1; indice1 < rnpDestino[0].numeroNos; indice1++) {
            if (indice1 < indiceP || indice1 > indiceL) {
                rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].idNo;
                rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].profundidade;
                adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA  + configuracoesParam[idConfiguracaoParam].numeroRNP, indice2);
                indice2++;
            }
        }
    }
}

/**
 * Libera a memória utilizada para armazenar os dados na estrutura do tipo MATRIZPI
 * @param matrizPIParam
 * @param numeroSetores
 */
void desalocaMatrizPI(MATRIZPI *matrizPIParam, long int numeroSetores) 
{
    //long int indiceColuna;
    long int indice;
   
    //percorre o vetor de matriz PI fazendo a desalocação das matrizes
/*    for(indice=1; indice<=numeroSetores; indice++) {
        free (matrizPIParam[indice].colunas);
    }*/
    for(indice=1; indice<=numeroSetores; indice++) {
        free (matrizPIParam[indice].colunas);
        matrizPIParam[indice].colunas = NULL;

    }
}
/**
 * Libera a memória utilizada para armazenar os dados na estrutura do tipo VETORPI
 * @param numeroMaximoConfiguracoesParam
 * @param vetorPiParam
 */
void desalocaVetorPi(int numeroMaximoConfiguracoesParam, VETORPI *vetorPiParam)
{
    int indice;
    //aloca o vetor pi 
    for (indice = 1; indice < numeroMaximoConfiguracoesParam; indice++)
    {
        free(vetorPiParam[indice].idChaveAberta);
        vetorPiParam[indice].idChaveAberta = NULL;
        free(vetorPiParam[indice].idChaveFechada);
        vetorPiParam[indice].idChaveFechada = NULL;
        free(vetorPiParam[indice].estadoInicialChaveAberta);
        vetorPiParam[indice].estadoInicialChaveAberta = NULL;
        free(vetorPiParam[indice].estadoInicialChaveFechada);
        vetorPiParam[indice].estadoInicialChaveFechada = NULL;
        free(vetorPiParam[indice].nos);
        vetorPiParam[indice].nos = NULL;
    }
}

/* Por Leandro:
 * Atualiza a MatrizPI com os dados salvos em MatrizPITemp relativos aos setores que mudaram de árvore para a obtenção de idConfiguracaoTempParam
 *
 */
void atualizaMatrizPIV2(MATRIZPI *matrizPiParam, long int idConfiguracaoTempParam, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam,
	        CONFIGURACAO *configuracoesTempParam, MATRIZPI *matrizPiTempParam,
	        VETORPI *vetorPiTempParam, long int setorFaltaParam){
    long int  indice, indice1, indiceP, indiceL, indiceSetorFalta, a, p;
    int rnpP;
    int numeroFalta, numeroAJusante;;
    long int nosFalta[100]; //é o número máximo de setores em falta ou a jusantes
    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
    long int totalCombinacoes;
    long int consumidoresSemFornecimento = 0;
    long int consumidoresEspeciaisSemFornecimento = 0;
    int **indiceRestabelecimento;
    int blocoRestabelecimento;
    int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4;
    int indiceNo, indiceColunaPITemp;
    long int noJusante;
    COLUNAPI *colunaPI;
    int idColunaPI;
    NOSRESTABELECIMENTO *nos;

    rnpP = matrizPiTempParam[setorFaltaParam].colunas[0].idRNP;
    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
    indiceP = matrizPiTempParam[setorFaltaParam].colunas[0].posicao + 1;
    indiceSetorFalta = matrizPiTempParam[setorFaltaParam].colunas[0].posicao; //indice do setor em falta na RNP.

    //verifica se o indiceP determinado é uma posição válida na RNP do setor em falta
    if (indiceP < configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
        numeroAJusante = 0;
        //verifica se a nó da posição do indiceP corresponde a um setor a jusante a falta
        if (configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
            indice1 = indiceP + 1;
            nosJusanteFalta[numeroAJusante] = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
            numeroAJusante++;
            //recupera a raiz de todas as subárvores que contém nós a jusante do setor em falta
            while (indice1 < configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
                if (configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                    nosJusanteFalta[numeroAJusante] = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    numeroAJusante++;
                }
                indice1++;
            }
        }
        //exitem nós a jusante que necessitam ser transferidos para outra ponto do sistema de forma a restabelecer o fornecimento
        if (numeroAJusante > 0) {
            indice = 0;
            nosFalta[0] = setorFaltaParam;
            numeroFalta = 1;
            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
            totalCombinacoes = 1;
            blocoRestabelecimento = 0;
            //percorre a lista de raizes de subárvore a jusante do setor em falta
            while (indice < numeroAJusante) {
                a = 0;
                idColunaPI = retornaColunaPi(matrizPiTempParam, vetorPiTempParam, nosJusanteFalta[indice], idConfiguracaoParam);
                indiceP = matrizPiTempParam[nosJusanteFalta[indice]].colunas[idColunaPI].posicao;
                p = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
                //recupera o indice final da subárvore dos setores a jusante
                indiceL = limiteSubArvore(configuracoesTempParam[idConfiguracaoParam].rnp[rnpP], indiceP);

                nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
                buscaTodosNosRestabelecimento(configuracoesTempParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoTempParam, indiceL, nosFalta, numeroFalta, matrizPiTempParam, vetorPiTempParam, &nos[blocoRestabelecimento], indiceSetorFalta);
                if (nos[blocoRestabelecimento].numeroNos > 0) {

					for(indiceNo = indiceP; indiceNo <= indiceL; indiceNo++){
						indiceColunaPITemp = 0;
						noJusante = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceNo].idNo;
						while(idConfiguracaoTempParam != matrizPiTempParam[noJusante].colunas[indiceColunaPITemp].idConfiguracao)
							indiceColunaPITemp++;
						 adicionaColuna(matrizPiParam, noJusante, idConfiguracaoParam+1, matrizPiTempParam[noJusante].colunas[indiceColunaPITemp].idRNP, matrizPiTempParam[noJusante].colunas[indiceColunaPITemp].posicao );
					}
					/*indice++;*/

                }
                indice++;

            }

            //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
            for (indice = 0; indice < numeroFalta; indice++) {
            	idColunaPI = retornaColunaPi(matrizPiParam, vetorPiTempParam, nosFalta[indice], idConfiguracaoParam);
            	indiceP = matrizPiParam[nosFalta[indice]].colunas[idColunaPI].posicao;
                grafoSetoresParam[configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
                //conta os consumidores sem fornecimento
                consumidoresSemFornecimento +=  grafoSetoresParam[configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
                consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
                if ((indiceP + 1) <= configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                    for (indice1 = indiceP; indice1 < configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
                        configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
                        configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
                        adicionaColuna(matrizPiParam, configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo, idConfiguracaoParam, rnpP, indice1);
                    }
                    configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].numeroNos = configuracoesTempParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
                }
            }
        }
    }
}


/**
 * @brief Por Leandro: esta função consiste na função "determinaNoAdjacente()" modificada para que:
 * 1) na escolha do nó adjacente ("noA") ao nó de poda ("noP") não seja escolhido um nó que, na
 * configuração selecionada ("idConfiguracaoSelecionadaParam"), está em uma árvore fictícia.
 *
 * @param tentativasParam: número de tentativas que serão realizadas na busca pelo nó a
 * @param idNoParam
 * @param florestaParam
 * @param idRNPParam
 * @param grafoSetoresParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @return
 */
COLUNAPI * determinaNoAdjacenteModificada(int tentativasParam, long int idNoParam, CONFIGURACAO florestaParam,
        int idRNPParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam)
{
    long int idNoA;
    int indiceColuna;
    COLUNAPI *colunaNoAdjacente = NULL;
    //seleciona um número aleatório da lista de adjacências do nó passado como parâmetro
    int indice = inteiroAleatorio(0, (grafoSetoresParam[idNoParam].numeroAdjacentes-1));
    BOOL encontrado = false;
    int tentativas = 0;
    //executa um loop até alcançar o número de tentativas ou encontrar um nó adjacente válido
    while (tentativas < tentativasParam && !encontrado) {
        //recupera o identificador do nó contido na posição aleatória selecionada
       // printf("indice %d idNoParam %ld \n", indice, idNoParam);
        idNoA = grafoSetoresParam[idNoParam].setoresAdjacentes[indice];
        if (grafoSetoresParam[idNoA].setorFalta == false) {
            //busca na matriz PI a posição e RNP do nó selecionado
            indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, idNoA, florestaParam.idConfiguracao);
            //verifica se o nó selecionado está em árvore diferente da passada como parâmetro
            if (matrizPiParam[idNoA].colunas[indiceColuna].idRNP != idRNPParam && matrizPiParam[idNoA].colunas[indiceColuna].idRNP < florestaParam.numeroRNP) {
            //if (matrizPiParam[idNoA].colunas[indiceColuna].idRNP < florestaParam.numeroRNP) {
                encontrado = true;
            } else //o nó está na mesma árvore deve-se selecionar outro
            {
                indice = inteiroAleatorio(0, (grafoSetoresParam[idNoParam].numeroAdjacentes-1));
            }
        }
        tentativas++;
    }
  //  printf("tentativas %d tentativasParam %d encontrado %d \n", tentativas, tentativasParam, encontrado);
    //retorna a coluna da matriz pi do nó selecionado
    if(encontrado && matrizPiParam[idNoA].colunas[indiceColuna].idRNP < florestaParam.numeroRNP) //Por Leandro
    {
        colunaNoAdjacente =  &(matrizPiParam[idNoA].colunas[indiceColuna]);
        return colunaNoAdjacente;
    }
    else
        return NULL;
}


/* Por Leandro: Esta função consiste na função "constroiRNPDestino()" modificada para que:
 *
 * 1)A RNP DESTINO acessada em "configuracoesParam" para a configuração "idConfiguracaoParam"
 * seja uma das RNPs Fictícias e não uma RNP real, como ocorre na função "constroiRNPDestino()";
 * 2)
 */
void constroiRNPDestinoComDestinoFicticia(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam)
{
    int indiceI, indiceF;
    int contador, indice1, indice2;
    int tamanhoTemporario;
    /*##############     MONTA A ARVORE Tto (Arvore do no a)  ###############*/
    indiceI = indiceL;
    indiceF = 0;
    contador = 0;
    tamanhoTemporario = 0;

    while (indiceR >= indiceP) {
        for (indice1 = indiceR, indice2 = indiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
            if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                if ((configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade || indice1 == indiceR)
                        && indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                   rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indiceA].profundidade + 1 + contador;
                    adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA + configuracoesParam[idNovaConfiguracaoParam].numeroRNP, indice2);

                    indice2++;
                }
                else
                    indice1 = indiceL + 1;
            }
        }
        tamanhoTemporario = indice2 - (indiceA+ 1);
        indiceI = indiceR;
        indiceF = indiceR + tamanhoTemporario - 1;
        indice1 = indiceR;
        while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR].profundidade - 1)
            indice1--;
        indiceR = indice1;
        contador++;
    }

    for (indice1 = 0; indice1 <= indiceA; indice1++) {
         rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].idNo;
         rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1].profundidade;
    }

    for (indice1 = indiceA +tamanhoTemporario + 1; indice1 < rnpDestino[0].numeroNos; indice1++)
    {
       rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1 - tamanhoTemporario].idNo;
       rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice1 - tamanhoTemporario].profundidade;
       adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice1].idNo, idNovaConfiguracaoParam, rnpA + configuracoesParam[idNovaConfiguracaoParam].numeroRNP, indice1);
    }
}

/* Por Leandro: Esta função consiste na função "constroiRNPDestino()" modificada para que:
 *
 * 1)A RNP ORGIGEM acessada em "configuracoesParam" para a configuração "idConfiguracaoParam"
 * seja uma das RNPs Fictícias e não uma RNP real, como ocorre na função "constroiRNPDestino()";
 * 2)
 */
void constroiRNPDestinoComOrigemFicticia(CONFIGURACAO *configuracoesParam, int rnpP,
        int rnpA, int indiceL, int indiceP, int indiceA, int indiceR, MATRIZPI *matrizPiParam,
        long int idConfiguracaoParam, RNP *rnpDestino, long int idNovaConfiguracaoParam)
{
    int indiceI, indiceF;
    int contador, indice1, indice2;
    int tamanhoTemporario;
    /*##############     MONTA A ARVORE Tto (Arvore do no a)  ###############*/
    indiceI = indiceL;
    indiceF = 0;
    contador = 0;
    tamanhoTemporario = 0;

    while (indiceR >= indiceP) {
        for (indice1 = indiceR, indice2 = indiceA + 1 + tamanhoTemporario; indice1 <= indiceL; indice1++) {
            if (indice1 < indiceI || indice1 > indiceF) { //Se o nó em questão estiver dentro da subárvore podada
                if ((configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade || indice1 == indiceR)
                        && indice1 < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].numeroNos) {
                   rnpDestino[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].idNo;
                    rnpDestino[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade - configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade + configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indiceA].profundidade + 1 + contador;
                    adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpA, indice2);

                    indice2++;
                }
                else
                    indice1 = indiceL + 1;
            }
        }
        tamanhoTemporario = indice2 - (indiceA+ 1);
        indiceI = indiceR;
        indiceF = indiceR + tamanhoTemporario - 1;
        indice1 = indiceR;
        while (configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice1].profundidade > configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indiceR].profundidade - 1)
            indice1--;
        indiceR = indice1;
        contador++;
    }

    for (indice1 = 0; indice1 <= indiceA; indice1++) {
         rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].idNo;
         rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1].profundidade;
    }

    for (indice1 = indiceA +tamanhoTemporario + 1; indice1 < rnpDestino[0].numeroNos; indice1++)
    {
       rnpDestino[0].nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1 - tamanhoTemporario].idNo;
       rnpDestino[0].nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice1 - tamanhoTemporario].profundidade;
       adicionaColuna(matrizPiParam, rnpDestino[0].nos[indice1].idNo, idNovaConfiguracaoParam, rnpA, indice1);
    }
}

/**
 * Por Leandro:
 * Este método realiza a realocação e inicialização das posições adicionais do vetor pi da RNP.
 *
 * @param numeroPosicoesAnteriorParam é o número de posições anteriormente alocadas
 * @param numeroPosicoesNovoParam é o numero de posições que se deseja que vetorPiParam possua
 * @param vetorPiParam é a variável que salva o vetor pi
 */
//TODO testar o funcionamento desta função
void realocaVetorPi(int numeroPosicoesAnteriorParam, int numeroPosicoesNovoParam, VETORPI **vetorPiParam)
{
    int indice;
    //realoca o vetor pi
    (*vetorPiParam) = (VETORPI *) realloc((*vetorPiParam), (numeroPosicoesNovoParam) * sizeof(VETORPI));

    if(!(*vetorPiParam)){
		printf("\nSem espaço suficiente para Realocar o Vetor Pi\n");
	}
	else{
		//inicializa os campos das novas posições do vetor pi
		for (indice = numeroPosicoesAnteriorParam; indice < numeroPosicoesNovoParam; indice++)
		{
			(*vetorPiParam)[indice].idAncestral = -1;
			(*vetorPiParam)[indice].numeroManobras = -1;
			(*vetorPiParam)[indice].casoManobra = 0;
			(*vetorPiParam)[indice].sequenciaVerificada = false;
		}
    }
}


/**
 * Por Leandro:
 * Este método realiza a realocação e inicialização das posições adicionais da variável configuracao, que salva as informações de todos os invidíduos gerados.
 *
 *@param numeroRNPParam
 *@param numeroRNPFicticiaParam
 *@param numeroPosicoesAnteriorParam é o número de posições anteriormente alocadas
 *@param numeroPosicoesNovoParam é o numero de posições que se deseja que vetorPiParam possua
 *@param numeroTrafosParam
 *@param numeroSetoresParam
 *@param *individuosParam
 */
//TODO testar o funcionamento desta função
void realocaIndividuo(int numeroPosicoesAnteriorParam, int numeroPosicoesNovoParam, CONFIGURACAO **individuosParam, long int idPrimeiraConfiguracaoParam, int numeroTrafosParam)
{
	long int indice;
	int numeroRNP,  numeroRNPFicticia;
    //Realoca a lista de configurações.
    (*individuosParam) = (CONFIGURACAO *) realloc((*individuosParam), (numeroPosicoesNovoParam) * sizeof(CONFIGURACAO));

    if((*individuosParam) == NULL)
      printf("Sem espaço suficiente para Realocar a variável 'configuracoes'\n");
    else{
		numeroRNP = (*individuosParam)[idPrimeiraConfiguracaoParam].numeroRNP;
		numeroRNPFicticia = (*individuosParam)[idPrimeiraConfiguracaoParam].numeroRNPFicticia;
		if(!(*individuosParam)){
			printf("\nSem espaço suficiente para Realocar a variável Configuracao\n");
		}
		else{
			//Percorre as posições adicionais alocadas fazendo a inicilização e alocação das posições da variável realocadas
			for (indice = numeroPosicoesAnteriorParam; indice < numeroPosicoesNovoParam; indice++ )
			{
				(*individuosParam)[indice].numeroRNP = numeroRNP;
				(*individuosParam)[indice].numeroRNPFicticia = numeroRNPFicticia;
				(*individuosParam)[indice].numeroRNPsFalta = 0;
				(*individuosParam)[indice].rnp = Malloc(RNP, (*individuosParam)[indice].numeroRNP);
				(*individuosParam)[indice].rnpFicticia = Malloc(RNP, (*individuosParam)[indice].numeroRNPFicticia); // Leandro: realiza a alocação das árvores fictícias que salvarão os setores saudáveis fora de serviço (desligados e cortados)
				(*individuosParam)[indice].idRnpFalta = Malloc(int, (*individuosParam)[indice].numeroRNP);
				(*individuosParam)[indice].idConfiguracao = indice;
//				(*individuosParam)[indice].ranqueamentoRnpsFicticias = Malloc(long int, (*individuosParam)[indice].numeroRNPFicticia);  // Leandro: aloca memória para a variável que armazenará o ranqueamento entre as RNPs Fictícias, as quais guiarão a aplicação do LRO
				inicializaObjetivosModificada(&(*individuosParam)[indice], numeroTrafosParam);
			}
		}
    }
}

/*
 * Por Leandro:
 * Determina o somatório de setores salvos em RNPs Fictícias, incluindo os nós-raiz fictícios
 */
long int determinaNumeroSetoresEmRNPFicticia(CONFIGURACAO popConfiguracaoParam){
	int indice;
	long int numeroNos;
	numeroNos = 0;
	for(indice = 0; indice < popConfiguracaoParam.numeroRNPFicticia; indice++)
		numeroNos = numeroNos +  popConfiguracaoParam.rnpFicticia[indice].numeroNos;

	return numeroNos;
}


