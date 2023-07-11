/**
 * @file funcoesProblema.c
 * @brief Este arquivo contém a implementação das funções e métodos para os cálculos necessários na avaliação das soluções de acordo com as características do problema.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "funcoesProblema.h"
#include "funcoesRNP.h"
#include "funcoesSetor.h"
#include "funcoesLeituraDados.h"
#include "funcoesAuxiliares.h"
#include "funcoesInicializacao.h"
#include "funcoesChaveamento.h"
//definição da constante ij em número complexo
__complex__ double ij = 1.0fI;
BOOL flagMudancaTap = false;
FILE *arquivog;

extern long int numeroExecucoesFluxoCarga; /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/
extern int tempoDeRestauracaoDaFalha; /**Por Leandro: O Tempo total estimado necessário para a recupeção de todas as falhas. Em outras palavras, o tempo no qual a rede operará na configuração obtida através de plano de restabelecimento obtido e informado. Declarada em "funcoesHeuristica.c"*/


/**
 * Método que verifica a factibilidade da sequência de chaveamento associada um indivíduo.
 *
 * @param configuracaoParam
 * @param vetorPiParam
 * @param idConfiguracaoParam
 * @return
 */
BOOL verificaFactibilidadeSequenciaChaveamento(CONFIGURACAO *configuracoesParam, VETORPI *vetorPiParam, long int idConfiguracaoParam) {
	int indice, totalConfiguracoesIntermediarias = 0;;
	long int *idConfiguracoesOriginal, idConfiguracao;
	BOOL sequenciaChaveamentoFactivel = true;

	//Verificação da adequação da configurações "idConfiguracaoParam" em relação as restrições GERAIS
	if(!verificaFactibilidade(configuracoesParam[idConfiguracaoParam], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao))
		sequenciaChaveamentoFactivel = false;
	else{ //Se a "idConfiguracaoParam" atende as restrições GERAIS, então continua-se a verificação

		//Verificação da adequação das configurações intermediárias de "idConfiguracaoParam" em relação as restrições RELAXADAS
		idConfiguracoesOriginal = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam, &totalConfiguracoesIntermediarias); //Recupera a relação de ancestralidade

		for(indice = 0; indice < (totalConfiguracoesIntermediarias -1) && sequenciaChaveamentoFactivel == true; indice++){
			idConfiguracao = idConfiguracoesOriginal[indice];

			if(!verificaFactibilidade(configuracoesParam[idConfiguracao], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR))
				sequenciaChaveamentoFactivel = false;
		}
	}

    return sequenciaChaveamentoFactivel;
}


/**
 * Recebe como parametro o grafo de setores, o identificador do setor, a lista de chaves
 *  e retorna somente os setores ligados por chaves automáticas.
 *
 * @param grafoSetoresParam
 * @param idNoSetorParam
 * @param listaChavesParam
 * @return 
 */
GRAFOSETORES *listaSetoresAutomatica(GRAFOSETORES *grafoSetoresParam, long int idNoSetorParam, LISTACHAVES *listaChavesParam) {
    int contador;
    int indice;
    long int idChave;
    GRAFOSETORES *listaAutomaticas;
    listaAutomaticas = (GRAFOSETORES *) malloc(sizeof (GRAFOSETORES));

    listaAutomaticas[0].idSetor = grafoSetoresParam[idNoSetorParam].idSetor;
    listaAutomaticas[0].idChavesAdjacentes = Malloc(long int, grafoSetoresParam[idNoSetorParam].numeroAdjacentes);
    listaAutomaticas[0].setoresAdjacentes = Malloc(long int, grafoSetoresParam[idNoSetorParam].numeroAdjacentes);
    listaAutomaticas[0].numeroAdjacentes = 0;
    indice = 0;
    for (contador = 0; contador < grafoSetoresParam[idNoSetorParam].numeroAdjacentes; contador++) {
        idChave = grafoSetoresParam[idNoSetorParam].idChavesAdjacentes[contador];
        if (listaChavesParam[idChave].tipoChave == chaveAutomatica) {
            listaAutomaticas[0].idChavesAdjacentes[indice] = grafoSetoresParam[idNoSetorParam].idChavesAdjacentes[contador];
            listaAutomaticas[0].setoresAdjacentes[indice] = grafoSetoresParam[idNoSetorParam].setoresAdjacentes[contador];
            indice++;
            listaAutomaticas[0].numeroAdjacentes++;
        }
    }
    return listaAutomaticas;
}

/**
 * Método que percorre o grafo que representa o sis54tema de distribuição e monta a matriz de impedâncias Z utilizada no
 * cálculo do fluxo de potência. Para isso faz uso de uma estrutura de forma a armazenar a matriz como matriz esparsa.
 * Além disso faz a alocação de memória da matriz
 * Tem como parâmetros o grafo do sistema, a matriz Z e o número de barras do sistema.
 *
 * @param grafoSDRParam
 * @param ZParam
 * @param numeroBarrasParam
 */
void constroiMatrizZ(GRAFO *grafoSDRParam, MATRIZCOMPLEXA **ZParam, long int numeroBarrasParam) {
    int indiceNos;
    int indiceAdjacentes;

    //aloca a memória para a matriz Z
    if (((*ZParam) = (MATRIZCOMPLEXA *) malloc((numeroBarrasParam + 1) * sizeof (MATRIZCOMPLEXA))) == NULL) {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para a matriz Z !!!!");
        exit(1);
    }
    //percorre a lista de barras do sistema no grafo
    for (indiceNos = 1; indiceNos <= numeroBarrasParam; indiceNos++) {
        //inclui os valores na lista inicial da matriz Z
        (*ZParam)[indiceNos].idNo = grafoSDRParam[indiceNos].idNo;
        (*ZParam)[indiceNos].numeroAdjacentes = grafoSDRParam[indiceNos].numeroAdjacentes;
        //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
        (*ZParam)[indiceNos].noAdjacentes = (CELULACOMPLEXA *) malloc(grafoSDRParam[indiceNos].numeroAdjacentes * sizeof (CELULACOMPLEXA));
        //percorre a lista de adjacentes calculando a impedância
        for (indiceAdjacentes = 0; indiceAdjacentes < grafoSDRParam[indiceNos].numeroAdjacentes; indiceAdjacentes++) {
            (*ZParam)[indiceNos].noAdjacentes[indiceAdjacentes].idNo = grafoSDRParam[indiceNos].adjacentes[indiceAdjacentes].idNo;
            (*ZParam)[indiceNos].noAdjacentes[indiceAdjacentes].valor = grafoSDRParam[indiceNos].adjacentes[indiceAdjacentes].resistencia + ij * grafoSDRParam[indiceNos].adjacentes[indiceAdjacentes].reatancia;
        }
    }

}
/**
 * Método para impressão da matriz de impedâncias
 * @param ZParam
 * @param numeroBarrasParam
 */
void imprimeMatrizZ(MATRIZCOMPLEXA *ZParam, long int numeroBarrasParam) {
    long int indiceNos;
    int indiceAdjacentes;

    //percorre a lista de barras do sistema no grafo
    for (indiceNos = 1; indiceNos <= numeroBarrasParam; indiceNos++) {
        //inclui os valores na lista inicial da matriz Z
        printf("Barra %ld -- adjacentes \n ", ZParam[indiceNos].idNo);
        //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
        //percorre a lista de adjacentes calculando a impedância
        for (indiceAdjacentes = 0; indiceAdjacentes < ZParam[indiceNos].numeroAdjacentes; indiceAdjacentes++) {
            printf("barra adj %ld: resistencia %.10lf reatancia %.10lf \t", ZParam[indiceNos].noAdjacentes[indiceAdjacentes].idNo,
                    __real__ ZParam[indiceNos].noAdjacentes[indiceAdjacentes].valor, __imag__ ZParam[indiceNos].noAdjacentes[indiceAdjacentes].valor);
        }
        printf("\n\n");
    }

}

/**
 * Tendo como parâmetro a matriz Z, e as barras da linha, retorna o valor da impedância.
 * @param ZParam
 * @param idNo1
 * @param idNo2
 * @return
 */
__complex__ double valorZ(MATRIZCOMPLEXA *ZParam, long int idNo1, long int idNo2) {
    int indiceAdjacentes = 0;

    while (indiceAdjacentes < ZParam[idNo1].numeroAdjacentes) {
        if (ZParam[idNo1].noAdjacentes[indiceAdjacentes].idNo == idNo2)
            return ZParam[idNo1].noAdjacentes[indiceAdjacentes].valor;
        indiceAdjacentes++;
    }
}

/**
 * Método para contruir a matriz que contém o máximo de corrente da linha.
 * Para isso faz uso de uma estrutura de forma a armazenar a matriz como matriz esparsa.
 * Além disso faz a alocação da estrutura.
 * Tem como parâmetros o grafo do sistema, a matriz de maximo de corrente e o número de barras
 *
 * @param grafoSDRParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 */
void constroiMatrizMaximoCorrente(GRAFO *grafoSDRParam, MATRIZMAXCORRENTE **maximoCorrenteParam, long int numeroBarrasParam) {
    int indiceNos;
    int indiceAdjacentes;
    //realiza a alocação de memória. A seguinte construção foi utilizada (*maximoCorrenteParam) pois o método deve alterar o valor do
    // ponteiro da estrutura.
    if (((*maximoCorrenteParam) = (MATRIZMAXCORRENTE *) malloc((numeroBarrasParam + 1) * sizeof (MATRIZMAXCORRENTE))) == NULL) {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para a matriz de máximo de corrente !!!!");
        exit(1);
    }
    //percorre o grafo e seus adjacentes preenchendo os valores da matriz
    for (indiceNos = 1; indiceNos <= numeroBarrasParam; indiceNos++) {
        (*maximoCorrenteParam)[indiceNos].idNo = grafoSDRParam[indiceNos].idNo;
        (*maximoCorrenteParam)[indiceNos].numeroAdjacentes = grafoSDRParam[indiceNos].numeroAdjacentes;
        //aloca o espaço para os dados dos adjacentes.
        (*maximoCorrenteParam)[indiceNos].noAdjacentes = (CELULACORRENTE *) malloc(grafoSDRParam[indiceNos].numeroAdjacentes * sizeof (CELULACORRENTE));
        //percorre a lista de adjacentes preenchendo os valores de máximo de corrente
        for (indiceAdjacentes = 0; indiceAdjacentes < grafoSDRParam[indiceNos].numeroAdjacentes; indiceAdjacentes++) {
            (*maximoCorrenteParam)[indiceNos].noAdjacentes[indiceAdjacentes].idNo = grafoSDRParam[indiceNos].adjacentes[indiceAdjacentes].idNo;
            (*maximoCorrenteParam)[indiceNos].noAdjacentes[indiceAdjacentes].valor = grafoSDRParam[indiceNos].adjacentes[indiceAdjacentes].ampacidade;
        }
    }
}

/**
 * Dados como parâmetro a matriz de correntes e as barras da linha, recupera o valor do máximo de corrente
 * @param maximoCorrenteParam
 * @param idNo1
 * @param idNo2
 * @return
 */
double maximoCorrente(MATRIZMAXCORRENTE *maximoCorrenteParam, long int idNo1, long int idNo2) {
    int indiceAdjacentes = 0;

    while (indiceAdjacentes < maximoCorrenteParam[idNo1].numeroAdjacentes) {
        if (maximoCorrenteParam[idNo1].noAdjacentes[indiceAdjacentes].idNo == idNo2)
            return maximoCorrenteParam[idNo1].noAdjacentes[indiceAdjacentes].valor;
        indiceAdjacentes++;
    }

}

/**
 * Por Leandro:
 * Esta função faz o cálculo do tempo bruto, que consiste no somatório do tempo de todas as operações realizadas pelo operadores SEM descontar o tempo de operação
 * de chaves que retornaram ao seu estado inicial. Em outras palavras, diferentemente da variável "(...).objetivo.tempo", a variável ""(...).objetivo.tempoBruto"
 * considera o tempo de fazer e desfazer uma manobra numa mesma chave. Isto é feito que para que o tempo Bruto seja utilizado no cálculo da ENERGIA NÃO SUPRIDA.
 * Uma vez que (i) a energia não suprida considera este tempo bruto (ii) a energia não suprida é minimizada pelo AEMO e (iii) a energia não suprida na
 * ocorrência de manobras numa mesma chave, então ao minimizar a energia não suprida cálculada com o tempo bruto, o AEMO dará preferência para indivíduos com
 * menos manobras repetidas.
 *
 *
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoParam
 * @param listaChavesParam
 * @param idChaveAberta
 * @param idChaveFechada
 */

void determinaTempoBrutoOperacaoChaves(CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam, int idConfiguracaoParam, LISTACHAVES *listaChavesParam, long int idChaveAberta, long int idChaveFechada){

	configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto = configuracoesParam[idConfiguracaoParam].objetivo.tempoBruto;

	if (idChaveAberta >= 0) {
			if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica)
				configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto = configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto + tempoOperacaoChaveAutomatica;
			else
				configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto = configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto + tempoOperacaoChaveManual;
	}
	if (idChaveFechada >= 0) {
		if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica)
			configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto = configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto + tempoOperacaoChaveAutomatica;
		else
			configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto = configuracoesParam[idNovaConfiguracaoParam].objetivo.tempoBruto + tempoOperacaoChaveManual;
	}
}

/**
 * Por Leandro: consiste na função numeroManobras() modificada:
 * 1) pela chamada, ao seu final, da função "determinaTempoBrutoOperacaoChaves()"
 * Portanto, é um método para o cálculo do número de manobras. Compara o estado da chave com o seu estado normal na configuração inicial.
 * Com base na comparação determina se a chave foi manobrada ou não em relação a configuração inicial.
 *
 * 2) Consideração duas novas situações em que ocorrerão os casos de manobras 1 ou 3, os quais resultam da inserção do LRO. Quando o LRO é aplicado e somente uma
 * parte de uma árvore (ou RNP) Fictícia é restaurada (podada e transferida para RNP Real), então, haverão duas chaves para serem manobras.
 * Contudo, quando o LRO é aplicado e todos os setores (reais) de uma árvore fictícia são restaurados, então, haverá somente uma chave para
 * ser manobrada (uma manobra de fechamento). Isto ocorre porque não houve divisão num agrupamento de setores que já haviam sido desligado.
 * Neste último caso, haverá somente uma chave que poderá ou não retornar ao seu estado inicial.
 * Como resultado, surgiu uma nova situação de ocorrência de Caso de Manobra 1 e uma nova situação de ocorrência do caso de Manobra 3.
 * São eles:
 * Caso de manobra 1: há somente uma chave para ser alterada e ela possui estado diferente do seu estado inicial
 * Caso de manobra 3: há somente uma chave para ser operada e ela voltou ao seu estado inicial
 *
 *
 * Calcula também quantas chaves foram manobradas por tipo de chave.
 * @param configuracoesParam
 * @param idChaveAberta
 * @param idChaveFechada
 * @param listaChavesParam
 * @param idConfiguracaoParam
 * @param estadoInicialChaves
 * @param estadoInicialCA
 * @param estadoInicialCF
 * @param idNovaConfiguracaoParam
 * @param casoManobra
 */
void numeroManobrasModificada(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra) {
    int manobrasAutomatica, manobrasManual;
    int seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
    int comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
    int comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
    int curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
    int curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
    //encontra o nó raiz de P na RNP
    int contaMudancaChave = 0;

    manobrasAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    manobrasManual = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    // Ao inicializar como falso, estas duas variáveis, faz-se com que estas variáveil tenham tal valor
    // quando idChaveAberta < 0 ou idChaveFechada < 0. Consequentemente,
    // evita-se falha de processamento na rotina que corrige a sequência de chaveamento
    estadoInicialCA[0] = false;
    estadoInicialCF[0] = false;

    if (idChaveAberta >= 0) {
        if (estadoInicialChaves[idChaveAberta] == normalmenteAberta) {
            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
                manobrasAutomatica--;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica--;
                } else
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaAutomatica--;
                }
            } else {
                manobrasManual--;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoManual--;
                } else {
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                        comCargaManual--;
                    } else {
                        if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca--;

                        }
                    }
                }

            }
            estadoInicialCA[0] = true;
            contaMudancaChave--;
        } else {

            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
                manobrasAutomatica++;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica++;
                } else
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaAutomatica++;
                }
            } else {
                manobrasManual++;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoManual++;
                } else {
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                        comCargaManual++;
                    } else {
                        if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca++;

                        }
                    }
                }
            }
            estadoInicialCA[0] = false;
            contaMudancaChave++;
        }
    }
    if (idChaveFechada >= 0) {
        if (estadoInicialChaves[idChaveFechada] == normalmenteFechada) {
            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
                manobrasAutomatica--;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica--;
                } else
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaAutomatica--;
                }
            } else
            {
                manobrasManual--;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoManual--;
                } else {
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                        comCargaManual--;
                    } else {
                        if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca--;

                        }
                    }
                }
            }
            estadoInicialCF[0] = true;
            contaMudancaChave--;
        } else {

            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
                manobrasAutomatica++;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica++;
                } else
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaAutomatica++;
                }
            } else
            {
                manobrasManual++;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoManual++;
                } else {
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                        comCargaManual++;
                    } else {
                        if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveFechada].subTipoChave == chaveFusivel)) {
                            seca++;

                        }
                    }
                }
            }
            estadoInicialCF[0] = false;
            contaMudancaChave++;
        }
    }

    switch (contaMudancaChave) {
        case 2: // as duas chaves da manobra possuem estado diferente do estado da configuração inicial
            casoManobra[0] = 1;
            break;
        case -2: //as duas chaves da manobra voltaram ao estado da configuração inicial
            casoManobra[0] = 2;
            break;
        case 0://uma das chaves retornou ao estado da configuração inicial e a outra possui estado diferente da configuração inicial
            casoManobra[0] = 3;
            break;
     // Por Leandro: estes dois casos a seguir passaram a existir somente após a inclusão do LRO. Isto porque, quando o LRO é aplicado e
     // todos os nós de uma árvore fictícia são reconectados, então não haverá chave a ser aberta, pois os setores já estavam desligados.
     // Nestes casos, há somente uma chave para ser operada, logo, somente uma chave pode voltar ou não ao seu estado inicial.
     // Logo: passou-se a existir as duas situações a seguir nais quais os casos de manobra 1 e 3 poderão também ocorrer.
        case -1: //Há somente uma chave para ser operada e ela voltou ao seu estado inicial
        	casoManobra[0] = 3;
        	break;
        case 1: //Há somente uma chave para ser operada e ela possui estado diferente do seu estado inicial
            casoManobra[0] = 1;
        	break;
    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais = manobrasManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas = manobrasAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.seca = seca;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual =  comCargaManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica = comCargaAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual = curtoManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica = curtoAutomatica;

    //Calcula os Tempos de Manobra das Chaves
	configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo = (configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais * tempoOperacaoChaveManual) + (configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas * tempoOperacaoChaveAutomatica); // Por Leandro: calcula o tempo estimado necessário para a realização de todas as manobras que levarão a obtenção da nova configuração
	//determinaTempoBrutoOperacaoChaves(configuracoesParam, idNovaConfiguracaoParam, idConfiguracaoParam, listaChavesParam, idChaveAberta, idChaveFechada);
}




/**
 * Método para o cálculo do número de manobras. Compara o estado da chave com o seu estado normal na configuração inicial. 
 * Com base na comparação determina se a chave foi manobrada ou não em relação a configuração inicial.
 * Calcula também quantas chaves foram manobradas por tipo de chave.
 * @param configuracoesParam
 * @param idChaveAberta
 * @param idChaveFechada
 * @param listaChavesParam
 * @param idConfiguracaoParam
 * @param estadoInicialChaves
 * @param estadoInicialCA
 * @param estadoInicialCF
 * @param idNovaConfiguracaoParam
 * @param casoManobra
 */
void numeroManobras(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra) {
    int manobrasAutomatica, manobrasManual;
    int seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
    int comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
    int comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
    int curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
    int curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
    //encontra o nó raiz de P na RNP
    int contaMudancaChave = 0;

    manobrasAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    manobrasManual = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    if (idChaveAberta >= 0) {
        if (estadoInicialChaves[idChaveAberta] == normalmenteAberta) {
            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
                manobrasAutomatica--;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica--;
                } else
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaAutomatica--;
                }
            } else {
                manobrasManual--;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoManual--;
                } else {
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                        comCargaManual--;
                    } else {
                        if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca--;

                        }
                    }
                }

            }
            estadoInicialCA[0] = true;
            contaMudancaChave--;
        } else {

            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
                manobrasAutomatica++;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica++;
                } else
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaAutomatica++;
                }
            } else {
                manobrasManual++;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoManual++;
                } else {
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                        comCargaManual++;
                    } else {
                        if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca++;

                        }
                    }
                }
            }
            estadoInicialCA[0] = false;
            contaMudancaChave++;
        }
    }
    if (idChaveFechada >= 0) {
        if (estadoInicialChaves[idChaveFechada] == normalmenteFechada) {
            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
                manobrasAutomatica--;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica--;
                } else
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaAutomatica--;
                }
            } else
            {
                manobrasManual--;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoManual--;
                } else {
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                        comCargaManual--;
                    } else {
                        if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca--;

                        }
                    }
                }
            }
            estadoInicialCF[0] = true;
            contaMudancaChave--;
        } else {

            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
                manobrasAutomatica++;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica++;
                } else
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaAutomatica++;
                }
            } else
            {
                manobrasManual++;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoManual++;
                } else {
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                        comCargaManual++;
                    } else {
                        if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveFechada].subTipoChave == chaveFusivel)) {
                            seca++;

                        }
                    }
                }
            }
            estadoInicialCF[0] = false;
            contaMudancaChave++;
        }
    }
        
    switch (contaMudancaChave) {
        case 2: // as duas chaves da manobra possuem estado diferente do estado da configuração inicial
            casoManobra[0] = 1;
            break;
        case -2: //as duas chaves da manobra voltaram ao estado da configuração inicial
            casoManobra[0] = 2;
            break;
        case 0://um das chaves retornou ao estado da configuração inicial e a outra possui estado diferente da configuração inicial
            casoManobra[0] = 3;
            break;
    }  
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais = manobrasManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas = manobrasAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.seca = seca;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual =  comCargaManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica = comCargaAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual = curtoManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica = curtoAutomatica;
   // printf("ponto 2 manobrasAuto %d manobrasManual %d \n", manobrasAutomatica, manobrasManual);
}

/**
 * Função para o calculo do número de manobras utilizadas no restabelecimento. No restabelecimento não é
 * verificado se a chave manobra retornou ao seu estado inicial. Isso é necessário para o caso de múltiplas faltas com opções de 
 * restabelecimento em locais de falta futura. 
 * @param configuracoesParam
 * @param idChaveAberta
 * @param idChaveFechada
 * @param listaChavesParam
 * @param idConfiguracaoParam
 * @param estadoInicialChaves
 * @param estadoInicialCA
 * @param estadoInicialCF
 * @param idNovaConfiguracaoParam
 * @param casoManobra
 */
void numeroManobrasRestabelecimento(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra) {
    int manobrasAutomatica, manobrasManual;
    int seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
    int comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
    int comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
    int curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
    int curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
    //encontra o nó raiz de P na RNP
    int contaMudancaChave = 0;

    manobrasAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    manobrasManual = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    if (idChaveAberta >= 0) {
        if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
            manobrasAutomatica++;
            if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                curtoAutomatica++;
            } else
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                comCargaAutomatica++;
            }
        } else {
            manobrasManual++;
            if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                curtoManual++;
            } else {
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaManual++;
                } else {
                    if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                        seca++;

                    }
                }
            }
        }
        estadoInicialCA[0] = false;
        contaMudancaChave++;

    }
    if (idChaveFechada >= 0) {


        if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
            manobrasAutomatica++;
            if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                curtoAutomatica++;
            } else
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                comCargaAutomatica++;
            }
        } else {
            manobrasManual++;
            if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                curtoManual++;
            } else {
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaManual++;
                } else {
                    if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveFechada].subTipoChave == chaveFusivel)) {
                        seca++;

                    }
                }
            }
        }
        estadoInicialCF[0] = false;
        contaMudancaChave++;

    }
    casoManobra[0] = 1;

    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais = manobrasManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas = manobrasAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.seca = seca;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual = comCargaManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica = comCargaAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual = curtoManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica = curtoAutomatica;
    // printf("ponto 2 manobrasAuto %d manobrasManual %d \n", manobrasAutomatica, manobrasManual);
}

/**
 * Por Leandro:
 * Descrição: consiste na função "numeroManobrasRestabelecimento()" modificada para que seja efetuada também
 * a determinação do tempo e do tempo bruto requerido para operação das manobras em chaves
 *
 * Função para o calculo do número de manobras utilizadas no restabelecimento. No restabelecimento não é
 * verificado se a chave manobra retornou ao seu estado inicial. Isso é necessário para o caso de múltiplas faltas com opções de
 * restabelecimento em locais de falta futura.
 * @param configuracoesParam
 * @param idChaveAberta
 * @param idChaveFechada
 * @param listaChavesParam
 * @param idConfiguracaoParam
 * @param estadoInicialChaves
 * @param estadoInicialCA
 * @param estadoInicialCF
 * @param idNovaConfiguracaoParam
 * @param casoManobra
 */
void numeroManobrasRestabelecimentoModificada(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra) {
    int manobrasAutomatica, manobrasManual;
    int seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
    int comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
    int comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
    int curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
    int curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
    //encontra o nó raiz de P na RNP
    int contaMudancaChave = 0;

    manobrasAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    manobrasManual = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    if (idChaveAberta >= 0) {
        if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
            manobrasAutomatica++;
            if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                curtoAutomatica++;
            } else
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                comCargaAutomatica++;
            }
        } else {
            manobrasManual++;
            if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                curtoManual++;
            } else {
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaManual++;
                } else {
                    if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                        seca++;

                    }
                }
            }
        }
        estadoInicialCA[0] = false;
        contaMudancaChave++;

    }
    if (idChaveFechada >= 0) {


        if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
            manobrasAutomatica++;
            if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                curtoAutomatica++;
            } else
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                comCargaAutomatica++;
            }
        } else {
            manobrasManual++;
            if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                curtoManual++;
            } else {
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaManual++;
                } else {
                    if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveFechada].subTipoChave == chaveFusivel)) {
                        seca++;

                    }
                }
            }
        }
        estadoInicialCF[0] = false;
        contaMudancaChave++;

    }
    casoManobra[0] = 1;

    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais = manobrasManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas = manobrasAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.seca = seca;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual = comCargaManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica = comCargaAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual = curtoManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica = curtoAutomatica;

    //Calcula os Tempos de Manobra das Chaves
	configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo = (configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais * tempoOperacaoChaveManual) + (configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas * tempoOperacaoChaveAutomatica); // Por Leandro: calcula o tempo estimado necessário para a realização de todas as manobras que levarão a obtenção da nova configuração
	//determinaTempoBrutoOperacaoChaves(configuracoesParam, idNovaConfiguracaoParam, idConfiguracaoParam, listaChavesParam, idChaveAberta, idChaveFechada);

}


 /**
  * Método para o cálculo de manobras que não considera os tipos de chaves.
  * @param configuracoesParam
  * @param idChaveAberta
  * @param idChaveFechada
  * @param listaChavesParam
  * @param idConfiguracaoParam
  * @param estadoInicialChaves
  * @param estadoInicialCA
  * @param estadoInicialCF
  * @param idNovaConfiguracaoParam
  * @param casoManobra
  */
void numeroManobrasAntigo(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, BOOL *estadoInicialCA, BOOL *estadoInicialCF,
        int idNovaConfiguracaoParam, int *casoManobra) {
    int manobrasAutomatica, manobrasManual;
    int contaMudancaChave = 0;

    manobrasAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    manobrasManual = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    if (idChaveAberta >= 0) {
        if (estadoInicialChaves[idChaveAberta] == normalmenteAberta) {
            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica)
                manobrasAutomatica--;
            else
                manobrasManual--;
            contaMudancaChave--;
            estadoInicialCA[0] = true;
        } else {

            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica)
                manobrasAutomatica++;
            else
                manobrasManual++;
            contaMudancaChave++;
            estadoInicialCA[0] = false;
        }
    }
    if (idChaveFechada >= 0) {
        if (estadoInicialChaves[idChaveFechada] == normalmenteFechada) {
            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica)
                manobrasAutomatica--;
            else
                manobrasManual--;
            contaMudancaChave--;
            estadoInicialCF[0] = true;
        } else {

            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica)
                manobrasAutomatica++;
            else
                manobrasManual++;
            contaMudancaChave++;
            estadoInicialCF[0] = false;
        }
    }

    switch (contaMudancaChave) {
        case 2: // as duas chaves da manobra possuem estado diferente do estado da configuração inicial
            casoManobra[0] = 1;
            break;
        case -2: //as duas chaves da manobra voltaram ao estado da configuração inicial
            casoManobra[0] = 2;
            break;
        case 0://um das chaves retornou ao estado da configuração inicial e a outra possui estado diferente da configuração inicial
            casoManobra[0] = 3;
            break;
    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais = manobrasManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas = manobrasAutomatica;
    // printf("ponto 2 manobrasAuto %d manobrasManual %d \n", manobrasAutomatica, manobrasManual);
}
/**
 * Este método realiza a busca pelos setores adjacentes ao setor informado para fazer o restabelecimento do trecho.
 * @param a
 * @param configuracoesParam
 * @param rnpP
 * @param indiceP
 * @param grafoSetoresParam
 * @param listaChavesParam
 * @param rnpA
 * @param indiceA
 * @param idConfiguracaoParam
 * @param indiceL
 * @param nosFalta
 * @param numeroFalta
 * @param r
 * @param indiceR
 * @param estadoInicial
 * @param matrizPiParam
 * @param vetorPiParam
 */
void buscaNosRestabelecimento(long int *a, CONFIGURACAO *configuracoesParam, int rnpP,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, LISTACHAVES *listaChavesParam,
        int *rnpA, long int *indiceA, int idConfiguracaoParam, long int indiceL,
        long int *nosFalta, int *numeroFalta, long int *r, long int *indiceR,
        ESTADOCHAVE *estadoInicial, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam) {
    int contador = 0;
    int indice1;
    int indice2;
    int flag;
    int flagFalta = 0;
    int indice3;
    int indiceAdjacente;
    int colunaA;
    GRAFOSETORES *listaAutomaticas;
    flag = 0;
    //Escolha do nó para religar a subárvore
    for (indice1 = indiceP; indice1 <= indiceL; indice1++) {
        //Verifica se o nó setor de índice 'indice1' possui chave automática adjacente
        listaAutomaticas = listaSetoresAutomatica(grafoSetoresParam, configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo, listaChavesParam);
        if (listaAutomaticas[0].numeroAdjacentes >= 1) {
            for (indice2 = 0; indice2 < listaAutomaticas[0].numeroAdjacentes; indice2++) {
                //Percorre a lista de chaves automáticas para determinar entre quais nós existe uma chave auto
                flagFalta = 0;
                for (indice3 = 0; indice3 < numeroFalta[0]; indice3++) {
                    // Verifica se o nó adjacente a 'r' e candidato a ser nó 'a' é um dos nós sob falta
                    if (listaAutomaticas[0].setoresAdjacentes[indice2] == nosFalta[indice3]) {
                        // Não se deve reconectar as cargas sem fornecimento ao um nó que está em falta
                        flagFalta = 1;
                        break;
                    }
                }

                //Se o nó análisado não está sob falta, isto é, flagFalta!=1
                if (flagFalta != 1) {
                    //Verifica se a chave entre o nó 'a' e o nó 'r' está aberta. Isto é, se o nó 'a' escolhido não está entre os setores afetados pela falta, se não pertence a subárvore sem fornecimento
                    if (estadoInicial[listaAutomaticas[0].idChavesAdjacentes[indice2]] == 0) {
                        indiceR[0] = indice1;
                        r[0] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                        a[0] = listaAutomaticas[0].setoresAdjacentes[indice2];
                        flag = 1; //Significa que já foi determinado um nó ao qual as cargas sem fonecimento podem ser conectadas por meio de uma chave automática
                        break;
                    }
                }
            }
            if (flag == 1) // Se já foi encontrado um nó adjacente com chave auto
                break;
        }
    }

    if (flag != 1) { //I.e, se não foi possével reconectar as cargas sem fornecimento por meio de uma chave automática
        while (a[0] == 0 && contador < 50) {
            indiceR[0] = inteiroAleatorio(indiceP, indiceL); //Seleciona o índice de um nó ajusante ao setor em falta para nova raiz do trecho sem abastecimento
            r[0] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceR[0]].idNo; // Salva o nome desse nó em 'r'
            indiceAdjacente = inteiroAleatorio(0, (grafoSetoresParam[r[0]].numeroAdjacentes - 1)); // Sorteia o índice de nó adjacente a r
            a[0] = grafoSetoresParam[r[0]].setoresAdjacentes[indiceAdjacente]; // 'a' é esse nó adjacente a 'r', onde será inserida a subárvore a jusante da falta
            colunaA = retornaColunaPi(matrizPiParam, vetorPiParam, a[0], idConfiguracaoParam);
            rnpA[0] = matrizPiParam[a[0]].colunas[colunaA].idRNP;
            if (rnpA[0] == rnpP) {
                a[0] = 0;
            } else {
                indiceA[0] = matrizPiParam[a[0]].colunas[colunaA].posicao;
            }

            contador++;
        }
    }
    if (contador == 50 && a[0] == 0) {
        nosFalta[numeroFalta[0]] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
        numeroFalta[0]++;
        a[0] = -1;
        rnpA[0] = -1;
        indiceA[0] = -1;
        printf("Não foi possível reconectar os setores a jusante %ld \n", configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo);
        // exit(1);
    }

}

/* Por Leandro:
 * Dado um setor "idNoParam" verifica se o mesmo pertence a subárvore de setores afetados/desligados pela falta.
 *
 */
BOOL pertenceSubarvoreDesligada(long int idNoParam, CONFIGURACAO *configuracoesParam, int idConfiguracaoParam,
        long int *nosFalta, int numeroFalta, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam){

	int indiceNoFalta, colunaPI, rnpP, indiceSetorFalta, indice;
	long int setorFalta;
	BOOL pertenceSubarvoreAfetada;

	pertenceSubarvoreAfetada = false;
	for(indiceNoFalta = 0; indiceNoFalta < numeroFalta && pertenceSubarvoreAfetada == false; indiceNoFalta++){
	   setorFalta = nosFalta[indiceNoFalta];
	   //Recupera o índice do Setor em Falta
	   colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFalta, idConfiguracaoParam);
	   rnpP = matrizPiParam[setorFalta].colunas[colunaPI].idRNP;
	   indiceSetorFalta = matrizPiParam[setorFalta].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

	   //Percorre a subárvore que possui "setorFalta" como raiz
	   indice = indiceSetorFalta + 1;
	   while((indice < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) && (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade) && (pertenceSubarvoreAfetada == false)){

		   if(configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].idNo == idNoParam) //Verifica se "idNoParam" pertence a subárvore desligada
			   pertenceSubarvoreAfetada = true;
		   indice++;
	   }
	}

	return pertenceSubarvoreAfetada;
}

/**
 * Dado um ramo a jusante a falta o método retorna todas as opções de restabelecimento que existem para o bloco de setores informado.
 * @param configuracoesParam
 * @param rnpP
 * @param indiceP
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param indiceL
 * @param nosFalta
 * @param numeroFalta
 * @param matrizPiParam
 * @param vetorPiParam
 * @param nosParam
 * @param indiceFalta
 */
void buscaTodosNosRestabelecimento(CONFIGURACAO *configuracoesParam, int rnpPParam,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam, long int indiceL,
        long int *nosFalta, int numeroFalta, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam,
        NOSRESTABELECIMENTO *nosParam, long int indiceFalta) {
    int contador = 0;
    int indice1;
    int indice2, indiceNo;
    int flagFalta = 0;
    int indice3;
    int rnpA;
    long int noR;
    long int adjacente;
    int colunaA;
    //printf("Busca no restabelecimento setor %ld rnp %d\n", configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo, rnpP);
    //Escolha do nó para religar a subárvore
    for (indice1 = indiceP; indice1 <= indiceL; indice1++) {
        noR = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice1].idNo;
        for (indice2 = 0; indice2 < grafoSetoresParam[noR].numeroAdjacentes; indice2++) {
            //Percorre a lista de chaves automáticas para determinar entre quais nós existe uma chave auto
            flagFalta = 0;
            // printf("numeroFalta %d \n", numeroFalta);
            for (indice3 = 0; indice3 < numeroFalta; indice3++) {
                // Verifica se o nó adjacente a 'r' e candidato a ser nó 'a' é um dos nós sob falta
                // printf("Nos Falta %ld \n", nosFalta[indice3]);
                adjacente = grafoSetoresParam[noR].setoresAdjacentes[indice2];
                if ((adjacente == nosFalta[indice3]) || (grafoSetoresParam[adjacente].setorFalta == true)) {
                    // Não se deve reconectar as cargas sem fornecimento ao um nó que está em falta
                    flagFalta = 1;
                    break;
                }
            }

            if (flagFalta != 1) {
                //Verifica se a chave entre o nó 'a' e o nó 'r' está aberta. Isto é, se o nó 'a' escolhido não está entre os setores afetados pela falta, se não pertence a subárvore sem fornecimento
                colunaA = retornaColunaPi(matrizPiParam, vetorPiParam, grafoSetoresParam[noR].setoresAdjacentes[indice2], idConfiguracaoParam);
                rnpA = matrizPiParam[grafoSetoresParam[noR].setoresAdjacentes[indice2]].colunas[colunaA].idRNP;
                indiceNo = matrizPiParam[grafoSetoresParam[noR].setoresAdjacentes[indice2]].colunas[colunaA].posicao;
                if (rnpA != rnpPParam) {
                    nosParam[0].noR[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice1].idNo;
                    nosParam[0].noP[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceP].idNo;
                    nosParam[0].noA[contador] = grafoSetoresParam[noR].setoresAdjacentes[indice2]; // 'a' é esse nó adjacente a 'r', onde será inserida a subárvore a jusante da falta
                    nosParam[0].rnpA[contador] = rnpA;
                    //  printf("noR %ld noA %ld rnpA %d \n",nosParam[0].noR[contador], nosParam[0].noA[contador], nosParam[0].rnpA[contador]);
                    contador++;
                } else
                    // || ((indiceNo>indiceL)&& (indiceNo<indiceLFalta)) para buscar posicoes após o bloco de restabelecimento
                    if ((indiceNo < indiceP) && (indiceNo > indiceFalta)) {
                    nosParam[0].noR[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice1].idNo;
                    nosParam[0].noA[contador] = grafoSetoresParam[noR].setoresAdjacentes[indice2]; // 'a' é esse nó adjacente a 'r', onde será inserida a subárvore a jusante da falta
                    nosParam[0].noP[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceP].idNo;
                    nosParam[0].rnpA[contador] = rnpA;
                    //  printf("noR %ld noA %ld rnpA %d \n",nosParam[0].noR[contador], nosParam[0].noA[contador], nosParam[0].rnpA[contador]);
                    contador++;

                }
            }

        }
    }

    nosParam[0].numeroNos = contador;
}


/**Por Leandro:
 * consiste na função "buscaTodosNosRestabelecimento()" corrigida para
 * (a) que seja verificado corretamente se um setor está presente na subávore desligada quando "rnpA == rnpPParam"
 *
 * Dado um ramo a jusante a falta o método retorna todas as opções de restabelecimento que existem para o bloco de setores informado.
 * @param configuracoesParam
 * @param rnpP
 * @param indiceP
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param indiceL
 * @param nosFalta
 * @param numeroFalta
 * @param matrizPiParam
 * @param vetorPiParam
 * @param nosParam
 * @param indiceFalta
 */
void buscaTodosNosRestabelecimentoCorrigida(CONFIGURACAO *configuracoesParam, int rnpPParam,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam, long int indiceL,
        long int *nosFalta, int numeroFalta, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam,
        NOSRESTABELECIMENTO *nosParam, long int indiceFalta) {
    int contador = 0;
    int indice1;
    int indice2, indiceNo;
    int flagFalta = 0;
    int indice3;
    int rnpA;
    long int noR;
    long int adjacente;
    int colunaA;
    //printf("Busca no restabelecimento setor %ld rnp %d\n", configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo, rnpP);
    //Escolha do nó para religar a subárvore
    for (indice1 = indiceP; indice1 <= indiceL; indice1++) {
        noR = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice1].idNo;
        for (indice2 = 0; indice2 < grafoSetoresParam[noR].numeroAdjacentes; indice2++) {
            //Percorre a lista de chaves automáticas para determinar entre quais nós existe uma chave auto
            flagFalta = 0;
            // printf("numeroFalta %d \n", numeroFalta);
            for (indice3 = 0; indice3 < numeroFalta; indice3++) {
                // Verifica se o nó adjacente a 'r' e candidato a ser nó 'a' é um dos nós sob falta
                // printf("Nos Falta %ld \n", nosFalta[indice3]);
                adjacente = grafoSetoresParam[noR].setoresAdjacentes[indice2];
                if ((adjacente == nosFalta[indice3]) || (grafoSetoresParam[adjacente].setorFalta == true)) {
                    // Não se deve reconectar as cargas sem fornecimento ao um nó que está em falta
                    flagFalta = 1;
                    break;
                }
            }

            if (flagFalta != 1) {
                //Verifica se a chave entre o nó 'a' e o nó 'r' está aberta. Isto é, se o nó 'a' escolhido não está entre os setores afetados pela falta, se não pertence a subárvore sem fornecimento
                colunaA = retornaColunaPi(matrizPiParam, vetorPiParam, grafoSetoresParam[noR].setoresAdjacentes[indice2], idConfiguracaoParam);
                rnpA = matrizPiParam[grafoSetoresParam[noR].setoresAdjacentes[indice2]].colunas[colunaA].idRNP;
                indiceNo = matrizPiParam[grafoSetoresParam[noR].setoresAdjacentes[indice2]].colunas[colunaA].posicao;
                if (rnpA != rnpPParam) {//< configuracoesParam[idConfiguracaoParam].numeroRNP
                    nosParam[0].noR[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice1].idNo;
                    nosParam[0].noP[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceP].idNo;
                    nosParam[0].noA[contador] = grafoSetoresParam[noR].setoresAdjacentes[indice2]; // 'a' é esse nó adjacente a 'r', onde será inserida a subárvore a jusante da falta
                    nosParam[0].rnpA[contador] = rnpA;
                    //  printf("noR %ld noA %ld rnpA %d \n",nosParam[0].noR[contador], nosParam[0].noA[contador], nosParam[0].rnpA[contador]);
                    contador++;
                } else


                	//POr Leandro: corrigido no "if" a seguir
                    if (!pertenceSubarvoreDesligada(configuracoesParam[0].rnp[rnpPParam].nos[indiceNo].idNo, configuracoesParam, 0, nosFalta, numeroFalta, matrizPiParam, vetorPiParam)) {
						nosParam[0].noR[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice1].idNo;
						nosParam[0].noA[contador] = grafoSetoresParam[noR].setoresAdjacentes[indice2]; // 'a' é esse nó adjacente a 'r', onde será inserida a subárvore a jusante da falta
						nosParam[0].noP[contador] = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceP].idNo;
						nosParam[0].rnpA[contador] = rnpA;
						//  printf("noR %ld noA %ld rnpA %d \n",nosParam[0].noR[contador], nosParam[0].noA[contador], nosParam[0].rnpA[contador]);
						contador++;

                }
            }

        }
    }

    nosParam[0].numeroNos = contador;
}



/**
 * Por Leandro:
 * Descrição: baseado na função "buscaTodosNosRestabelecimento()", dado um setor pertencente a uma RNP Fictícia, esta função
 * percorre a subávore que tem este nó como raiz e determina o número de opções de restabelecimento, bem como as trincas p, r, a possíveis
 * que reconectam toda a subarvore.
 *
 *@param configuracoesParam
 *@param npPParam
 *@param indiceP
 *@param grafoSetoresParam
 *@param idConfiguracaoParam
 *@param matrizPiParam
 *@param vetorPiParam
 *@param nosParam
 */
void buscaTodosNosRestabelecimentoV2(CONFIGURACAO *configuracoesParam, int rnpPParam,
        long int indiceP, GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam,
        MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, NOSRESTABELECIMENTO *nosParam) {
    int contador = 0;
    int indice1;
    int indice2, indiceNo;
    int rnpA;
    long int no;
    long int noAdjacente;
    int colunaA;
    long int indiceL;

    indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpPParam], indiceP);

    for (indice1 = indiceP; indice1 <= indiceL; indice1++) {
        no = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indice1].idNo;

        for (indice2 = 0; indice2 < grafoSetoresParam[no].numeroAdjacentes; indice2++) {
        	noAdjacente = grafoSetoresParam[no].setoresAdjacentes[indice2];

            if (grafoSetoresParam[noAdjacente].setorFalta == false) { //Se o nó adjacente em questão não é um setor em falta ou um setor desligado sem opção de reconexão

                colunaA = retornaColunaPi(matrizPiParam, vetorPiParam, grafoSetoresParam[no].setoresAdjacentes[indice2], idConfiguracaoParam);
                rnpA = matrizPiParam[grafoSetoresParam[no].setoresAdjacentes[indice2]].colunas[colunaA].idRNP;
                indiceNo = matrizPiParam[grafoSetoresParam[no].setoresAdjacentes[indice2]].colunas[colunaA].posicao;
                if (rnpA < configuracoesParam[idConfiguracaoParam].numeroRNP) { //É necessário que o nó adjacente não esteja numa RNP Fictícia
                    nosParam[0].noR[contador] = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indice1].idNo;
                    nosParam[0].noP[contador] = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indiceP].idNo;
                    nosParam[0].noA[contador] = grafoSetoresParam[no].setoresAdjacentes[indice2]; // 'a' é esse nó adjacente a 'r', onde será inserida a subárvore a jusante da falta
                    nosParam[0].rnpA[contador] = rnpA;
                    contador++;
                }
            }

        }
    }

    nosParam[0].numeroNos = contador;
}

/* Por Leandro:
 * Descrição: realiza a determinação de uma árvore origem (rnpP) para o LRO
 * Para tal, percorre o ranqueamento da RNPs verificando, na sequência em que estas aparecem neste ranqueamento,
 * o número de opções de reconexão.
 * Assim, a árvore origem será a RNP fictícia mais próxima do topo do ranqueamento que possui ao
 * menos uma opção de reconexão dos setores presentes nesta subárvore.
 *
 *@param popConfiguracaoParam
 *@param idConfiguracaoSelecionadaParam
 *@param grafoSetoresParam
 *@param matrizPiParam
 *@param vetorPiParam
 */
//int determinaArvoreFicticiaOrigemV0(CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam){
//    int rnpP, indiceRnpP, indiceNo1, numeroOpcoesReconexao, indiceL;
//	long int no1;
//	NOSRESTABELECIMENTO *nos;
//	long int numeroNos = 0;
//
//	numeroNos = determinaNumeroSetoresEmRNPFicticia(popConfiguracaoParam[idConfiguracaoSelecionadaParam]);
//	nos = Malloc(NOSRESTABELECIMENTO, numeroNos);
//
//	numeroOpcoesReconexao = 0;
//	indiceRnpP = 0;
//	while(numeroOpcoesReconexao == 0 && indiceRnpP <  popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia){
//		rnpP = popConfiguracaoParam[idConfiguracaoSelecionadaParam].ranqueamentoRnpsFicticias[indiceRnpP];
//
//		if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].numeroNos > 1){
//
//			indiceNo1 = 1;
//			no1 = popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].nos[indiceNo1].idNo;
//			indiceL = limiteSubArvore(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP], indiceNo1);
//
//			nos[indiceNo1].noA = Malloc(long int, (indiceL - indiceNo1 + 1)*10);
//			nos[indiceNo1].noR = Malloc(long int, (indiceL - indiceNo1 + 1)*10);
//			nos[indiceNo1].noP = Malloc(long int, (indiceL - indiceNo1 + 1)*10);
//			nos[indiceNo1].rnpA = Malloc(int, (indiceL - indiceNo1 + 1)*10);
//
//			buscaTodosNosRestabelecimentoV2(popConfiguracaoParam, rnpP, indiceNo1, grafoSetoresParam, idConfiguracaoSelecionadaParam, matrizPiParam, vetorPiParam, &nos[indiceNo1]);
//			numeroOpcoesReconexao = nos[indiceNo1].numeroNos;
//
//			free(nos[indiceNo1].noA);
//			free(nos[indiceNo1].noR);
//			free(nos[indiceNo1].noP);
//			free(nos[indiceNo1].rnpA);
//
//		}
//		indiceRnpP++;
//
//	}
//	free(nos);
//
//	if(numeroOpcoesReconexao == 0)
//		return(-1);
//	else
//		return(rnpP);
//}

/* Por Leandro:
 * Descrição: realiza a determinação de uma árvore origem (rnpP) para o LRO.
 * Para tal, determina, dentre todas as RNPs fictícias, aquela com o maior valore de potência não suprida, considerando-se os níveis
 * de prioridade dos consumidores desligados. Esta RNP será a árvore origem determinada.
 *
 *@param popConfiguracaoParam
 *@param idConfiguracaoSelecionadaParam
 *@param grafoSetoresParam
 *@param matrizPiParam
 *@param vetorPiParam
 */
int determinaArvoreFicticiaOrigem(CONFIGURACAO *popConfiguracaoParam, long int idConfiguracaoSelecionadaParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam){
    int rnpP, indice;

	if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia < 1) 		//Se não houver RNP Fictícia
		rnpP = -1;
	else{
		if(popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia == 1) //Se houver apenas uma RNP Fictícia
			rnpP = 0;
		else{ 																			//Se houver mais de uma rnp Fictícia, então, seleciona-se aquela com a maior quantidade de potência não suprida considerando os níveis de prioridade dos consumidores
			rnpP = 0;
			for(indice = 1; indice < popConfiguracaoParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; indice++){
				if((floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)  > floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) ||
				  ((floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) == floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) >  floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria))   ) ||
				  ((floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) == floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria))  && (floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa) >  floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa))  ) ||
				  ((floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) == floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria))  && (floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa) == floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa))   && (floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[indice].fitnessRNP.potenciaNaoSuprida.consumidoresSemPrioridade) > floor(popConfiguracaoParam[idConfiguracaoSelecionadaParam].rnpFicticia[rnpP].fitnessRNP.potenciaNaoSuprida.consumidoresSemPrioridade)) )    ){
					rnpP = indice;
				}
			}
		}
	}

	return(rnpP);
}

/**
 * Método para obter o novo array da representação nó-profundidade do alimentador em falta no processo de restabelecimento.
 * @param configuracoesParam
 * @param rnpP
 * @param indiceL
 * @param indiceP
 * @param matrizPiParam
 * @param idConfiguracaoParam
 * @param rnpOrigem
 * @param idNovaConfiguracaoParam
 */
void constroiRNPOrigemRestabelecimento(CONFIGURACAO *configuracoesParam, int rnpP,
        int indiceL, int indiceP, MATRIZPI *matrizPiParam, long int idConfiguracaoParam, RNP *rnpOrigem, long int idNovaConfiguracaoParam) {
    int indice1, indice2;
    /*##############    MONTA A ARVORE Tfrom (CONTEM O NO p)   ##############*/
    for (indice2 = 0; indice2 < indiceP; indice2++) {
        rnpOrigem[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice2].idNo;
        rnpOrigem[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice2].profundidade;
    }
    for (indice1 = indiceL + 1; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos; indice1++) {
        rnpOrigem[0].nos[indice2].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
        rnpOrigem[0].nos[indice2].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
        adicionaColuna(matrizPiParam, rnpOrigem[0].nos[indice2].idNo, idNovaConfiguracaoParam, rnpP, indice2);
        indice2++;
    }
}
/**
 * Método para isolar e restabelecer a falta por meio da escolha de uma opção aleatória de restabelecimento.
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param matrizPiParam
 * @param vetorPiParam
 * @param setorFaltaParam
 * @param chavesManobradasParam
 * @param estadoInicial
 * @param estadoAutomaticas
 * @param estadoRestabelecimento
 * @param paresManobras
 * @param CS
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 * @param listaChavesParam
 */
void isolaRestabelece(GRAFOSETORES *grafoSetoresParam, int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int setorFaltaParam, long int **chavesManobradasParam,
        ESTADOCHAVE *estadoInicial, ESTADOCHAVE *estadoAutomaticas, ESTADOCHAVE *estadoRestabelecimento,
        long int ***paresManobras, long int **CS, long int * numChaveAberta,
        long int *numChaveFechada, int idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam) {
    int tamanhoTemporario;
    long int p, a, indice, indice1, indice2, indiceP, indiceR, indiceL, r,
            indiceSetorFalta, indiceA;
    long int noAnteriorFalta;
    long int indiceLFalta;
    int rnpP, rnpA;
    int numeroFalta, numeroAJusante, posicaoSorteio;
    long int nosFalta[100]; //é o número máximo de setores em falta ou a jusantes
    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
    int colunaPI;
    RNP rnpOrigem;
    RNP rnpDestino;
    NOSRESTABELECIMENTO *nos;
    //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
    grafoSetoresParam[setorFaltaParam].setorFalta = true;
    paresManobras[idNovaConfiguracaoParam] = (long int**) malloc(1 * sizeof (long int*));
    paresManobras[idNovaConfiguracaoParam][0] = (long int*) malloc((70) * sizeof (long int)); //70 Ã© a capacidade mÃ¡xima de armazenamento de chaves fechadas para reconectar setores a jusante da falta
    rnpP = matrizPiParam[setorFaltaParam].colunas[0].idRNP;
    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
    indiceP = matrizPiParam[setorFaltaParam].colunas[0].posicao + 1;
    indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[0].posicao; //indice do setor em falta na RNP.

    //localiza o setor anterior a falta (raiz da falta) na RNP
    indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o éndice do nó antecessor ao nó sob falta
    while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
        indice1--;
    }

    noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

    //localiza a chave que liga o setor em falta ao seu nó raiz
    indice1 = 0;
    while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != noAnteriorFalta) {
        indice1++;
    }
    //calcula a manobra para isolar a falta do alimentador, abrindo a chave com o setor anterior a falta
    estadoInicial[grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1]] = 0; //Chave entre o setor em falta e o nó a montante a ele que lhe fornece energia
    estadoRestabelecimento[grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1]] = 0;
    paresManobras[idNovaConfiguracaoParam][0][numChaveAberta[0] + 30] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1]; //Salva a chave que alimenta o setor em falta e que deve ser aberta
    numChaveAberta[0]++;
    if (listaChavesParam[grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1]].tipoChave == chaveAutomatica)
        chavesManobradasParam[0][idConfiguracaoParam]++;
    else
        chavesManobradasParam[1][idConfiguracaoParam]++;
    chavesManobradasParam[2][idConfiguracaoParam]++;


    //verifica se o indiceP, ou seja, o primeiro setor a jusante a falta determinado é uma posição válida na RNP do setor em falta
    //em caso afirmativo é necessário procurar formas de restabelecer esse(s) setores sãos a jusante.
    if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
        numeroAJusante = 0;
        //verifica se a nó da posição do indiceP corresponde a um setor a jusante a falta
        if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
            indice1 = indiceP + 1;
            nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
            numeroAJusante++;
            //recupera o setor inicial de todas os blocos de setores que contém setores a jusante à falta que precisam ser restabelecidos.
            while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
                if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                    nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    numeroAJusante++;
                }
                indice1++;
            }
            indiceLFalta = indice1 - 1;
        }
        //exitem nós a jusante que necessitam ser transferidos para outra ponto do sistema de forma a restabelecer o fornecimento
        if (numeroAJusante > 0) {

            indice = 0;
            nosFalta[0] = setorFaltaParam;
            numeroFalta = 1;
            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
            //percorre a lista de raizes de subárvore a jusante do setor em falta
            while (indice < numeroAJusante) {
                a = 0;
                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idNovaConfiguracaoParam);
                indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
                //recupera o indice final da subárvore dos setore a jusante
                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
                nos[indice].noA = Malloc(long int, (indiceL - indiceP + 1)*10);
                nos[indice].noR = Malloc(long int, (indiceL - indiceP + 1)*10);
                nos[indice].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
                buscaTodosNosRestabelecimento(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[indice], indiceSetorFalta);
                //realiza a busca pelo nó de religação e seu adjacente
                // buscaNosRestabelecimento(&a, configuracoesParam, rnpP, indiceP, grafoSetoresParam, listaChavesParam, &rnpA, &indiceA, idConfiguracaoParam,
                //        indiceL, nosFalta, &numeroFalta, &r, &indiceR, estadoInicial, matrizPiParam, vetorPiParam);
                if (nos[indice].numeroNos > 0) {
                    posicaoSorteio = inteiroAleatorio(0, (nos[indice].numeroNos - 1));
                    rnpA = nos[indice].rnpA[posicaoSorteio];
                    a = nos[indice].noA[posicaoSorteio];
                    r = nos[indice].noR[posicaoSorteio];

                    //foi possivel restabelecer o setor a jusante
                    if (rnpA != -1) {
                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idNovaConfiguracaoParam);
                        indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idNovaConfiguracaoParam);
                        indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
                        // printf("Restabelece \n");
                        /*ROTINA PARA CALCULAR OS PARES DE MANOBRAS*/
                        //numeroManobras(configuracoesParam, indiceP, rnpP, r, p, a, idNovaConfiguracaoParam, chavesManobradasParam, estadoInicial, estadoAutomaticas, numChaveAberta, numChaveFechada, idNovaConfiguracaoParam);
                        tamanhoTemporario = indiceL - indiceP + 1;
                        //realiza a alocação das RNPs que serão alteradas
                        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                        //obtém a nova rnp de destino
                        constroiRNPDestino(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoParam, &rnpDestino, idNovaConfiguracaoParam);
                        constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoParam, &rnpOrigem, idNovaConfiguracaoParam);
                        /*###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############*/
                        configuracoesParam[idNovaConfiguracaoParam].rnp[rnpA] = rnpDestino;
                        configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP] = rnpOrigem;
                        for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                            if (indice1 != rnpA && indice1 != rnpP)
                                configuracoesParam[idNovaConfiguracaoParam].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
                        }

                    }
                } else {
                    free(nos[indice].noA);
                    free(nos[indice].noR);
                    free(nos[indice].rnpA);

                    nosFalta[numeroFalta] = nosJusanteFalta[indice];
                    numeroFalta++;
                    printf("Não é possível restabelecer o bloco de setores a jusante com inicio em %ld \n", nosJusanteFalta[indice]);
                }
                indice++;
            }
        }
    }
    /*Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p*/
    tamanhoTemporario = 1;

    if ((indiceSetorFalta + 1) <= configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos) {
        for (indice1 = indiceSetorFalta; indice1 < configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {

            configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
            configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
            adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam, rnpP, indice1);
        }
        configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario;

        //remove os setores que não puderam ser restabelecidos a jusante a falta
        if (numeroFalta > 1) {
            for (indice = 1; indice < numeroFalta; indice++) {
                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam);
                indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
                //recupera o indice final da subárvore dos setore a jusante
                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
                //marca os setores que não puderam ser restabelecidos como setores em falta para bloquear manobras
                for (indice2 = indiceP; indice2 <= indiceL; indice2++)
                    grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice2].idNo].setorFalta = true;

                indice2 = indiceP;
                if (indiceL + 1 < (configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos)) {
                    for (indice1 = indiceL; indice1 < (configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos); indice1++) {

                        configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice2].idNo = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
                        configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice2].profundidade = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
                        adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].nos[indice2].idNo, idNovaConfiguracaoParam, rnpP, indice2);

                        indice2++;
                    }
                }
                tamanhoTemporario = (indiceL - indiceP + 1);
                configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario;
            }
        }
    }
    for (indice = 0; indice < numeroAJusante; indice++) {
        free(nos[indice].noA);
        free(nos[indice].noR);
        free(nos[indice].rnpA);
    }
    free(nos);
}

/*
* Leandro:
* Método que salva em "configuracoesParam" o indivíduo que é, conceitualmente, a configuração inicial do problema, isto é,
* a configuração da rede na qual os setores em falta estão isolados e todos os setores saudáveis á jusante
* estão desligados.
* Os agrupamentos de setores saudáveis COM carga possíveis de serem reconectados são salvos em RNPs Fictícias.
* Isto significa que agrupamentos de setores sem possibilidade de reconexão ou, mesmo que tenham possibilidade de reconexão, sem
* consumidores são desconsiderados e tratados, pelo programa, como setores em falta.
*
* Obs.: Serão copiados para RNPs Fictícias somentes as subárvores que possuírem NAs permitindo restaurá-las
* à rede. As que não as possuírem serão desconsideradas do problema. São descartadas também subávores que não possuam cargas,
* mesmo que tenham chaves NAs que permitam a sua reconexão.
*
* @param grafoSetoresParam
* @param idConfiguracaoParam
* @param configuracoesParam
* @param matrizPiParam
* @param vetorPiParam
* @param setorFaltaParam
* @param estadoInicial
* @param numChaveAberta
* @param numChaveFechada
* @param idNovaConfiguracaoParam
* @param listaChavesParam
*/
void obtemConfiguracaoInicial(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
       CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
       VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
       long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam) {
   int tamanhoTemporario;
   long int p, a, indice, indice1, indiceP, indiceR, indiceL, r,
           indiceSetorFalta, indiceA;
   long int noAnteriorFalta;
   long int indiceLFalta, idConfiguracaoBase;
   int rnpP, rnpA;
   int numeroFalta, numeroAJusante, posicao;
   long int nosFalta[500]; //é o número máximo de setores em falta ou a jusantes
   long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
   long int totalCombinacoes;
   long int *idChaveAberta, *idChaveFechada;
   long int consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
   long int consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
   int indiceChave;
   int **indiceRestabelecimento;
   int blocoRestabelecimento=0;
   int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4;
   int colunaPI;
   RNP rnpOrigem;
   //RNP rnpDestino;
   NOSRESTABELECIMENTO *nos;
   int casoManobra;
   BOOL *estadoInicialCA, *estadoInicialCF;
   NOSPRA *nosPRA;
   NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida;
   double potenciaNaoSupridaTotal;
   //IMPRESSAO DE DADOS PARA ANALISE DOS RESULTADOS
   //char nomeArquivo[120];
   //FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
   //todos os setores do sistema
   //sprintf(nomeArquivo, "SAIDA_testeConfiguracaoInicial.dad");
  // arquivo1 = fopen(nomeArquivo, "a");

   configuracoesParam[idNovaConfiguracaoParam[0]].numeroRNPFicticia = 0;

   //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
   grafoSetoresParam[setorFaltaParam].setorFalta = true;
   //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
 //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
  // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;

   //RECUPERA A COLUNAPI DO SETOR EM FALTA
   colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
   rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
   //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
   indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
   indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

   //LOCALIZA O SETOR ANTERIOR A FALTA
   indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o índice do nó antecessor ao nó sob falta
   while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
       indice1--;
   }
   noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

   //verifica se o setor em falta não é o último da RNP
   if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
       numeroAJusante = 0;
       //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
       if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
           indice1 = indiceP + 1;
           nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
           numeroAJusante++;
           //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
           while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
               if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                   nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                   numeroAJusante++;
               }
               indice1++;
           }
           indiceLFalta = indice1 - 1;
           indice = 0;
           nosFalta[0] = setorFaltaParam;
           numeroFalta = 1;
           nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
           totalCombinacoes = 1;
           blocoRestabelecimento = 0;
           //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as subárvores que possuem Chaves NAs adjacentes e que, por isso, podem ser reconectadas
           //Para a formação do
           while (indice < numeroAJusante) {
               a = 0;
               colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
               indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
               p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
               //recupera o indice final da subárvore dos setore a jusante
               indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
               nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
               buscaTodosNosRestabelecimentoCorrigida(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
               potenciaNaoSuprida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idConfiguracaoParam, indiceP, indiceL, rnpP, rnpSetoresParam, grafoSDRParam);
               potenciaNaoSupridaTotal = potenciaNaoSuprida.consumidoresSemPrioridade + potenciaNaoSuprida.consumidoresPrioridadeBaixa + potenciaNaoSuprida.consumidoresPrioridadeIntermediaria + potenciaNaoSuprida.consumidoresPrioridadeAlta;

               /* Um determinado agrupamneto de setores será salvo numa RNP Fictícia se:
                * 1) houver opções (chaves NAs) que permitam reconectá-lo à rede
                * 2) E, concomitantemente, se houver algum consumidor neste agrupamento de setores*/
               if (nos[blocoRestabelecimento].numeroNos > 0 && potenciaNaoSupridaTotal > 0) {
            	   /*Por Leandro: estas várias opções para reconectar tais subárvores já foram aplicadas pela BE. Como está função visa apenas conectar cada subárvore afetada a uma RNP Fictícias,
            	    então será tomada apenas esta primeira opção de reconexão*/
                   nos[blocoRestabelecimento].noA[0] = configuracoesParam[idConfiguracaoParam].rnpFicticia[blocoRestabelecimento].nos[0].idNo;
                   nos[blocoRestabelecimento].noR[0] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
                   nos[blocoRestabelecimento].noP[0] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
                   nos[blocoRestabelecimento].rnpA[0] = blocoRestabelecimento;//É sempre um índice de RNP FICTÍCIA configuracoesParam[idConfiguracaoParam].numeroRNP + indice;
                   nos[blocoRestabelecimento].numeroNos = 1;

                   totalCombinacoes *= nos[blocoRestabelecimento].numeroNos;
                   blocoRestabelecimento++;

               } else { //Caso não haja chaves NAs para restaurar uma subárvore, a mesma é """apagada""", ie, é considerada setor em falta para não seja evitar alterações na mesma. Neste caso ela nem será conectada à RNP Fictícia
            	   free(nos[blocoRestabelecimento].noA);
                   free(nos[blocoRestabelecimento].noR);
                   free(nos[blocoRestabelecimento].noP);
                   free(nos[blocoRestabelecimento].rnpA);
                   //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
                   nosFalta[numeroFalta] = nosJusanteFalta[indice];
                   numeroFalta++;
                   colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                   indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
                   // printf("%d\n",indiceAux1);
                   while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
                       nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
                       numeroFalta++;
                       indiceAux1++;
                   }
               }
               indice++;
           }
           //******************************************************************************************************//

           //configuracoesParam[idNovaConfiguracaoParam[0]].numeroRNPFicticia = 0; //Se houver opções para restabelecimento, este será incrementado
           if (blocoRestabelecimento == 0) { //não existe opção de restabelecimento para nenhuma das subárvores desligadas
               //fprintf(arquivo1, "SAR\n");
               //fclose(arquivo1);
               //cria um novo individuo isolando a falta e os setores sem restabelecimento
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
               //isola a falta
               idChaveAberta = Malloc(long int, 1);
               idChaveFechada = Malloc(long int, 1);
               buscaChaveIsolaFaltaModificada(grafoSetoresParam, idNovaConfiguracaoParam[0],
                       configuracoesParam, numChaveAberta, idChaveAberta,
                       idChaveFechada, setorFaltaParam, noAnteriorFalta,
                       1, listaChavesParam);
               nosPRA = Malloc(NOSPRA, 1);
               nosPRA[0].a = -1;
               nosPRA[0].p = setorFaltaParam;
               nosPRA[0].r = -1;
               estadoInicialCA = Malloc(BOOL, 1);
               estadoInicialCF = Malloc(BOOL, 1);
               estadoInicialCA[0] = false;
               estadoInicialCF[0] = false;
               //aloca a nova rnp
               alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta, &rnpOrigem);
               //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[0], idConfiguracaoParam);
                indiceP = matrizPiParam[nosFalta[0]].colunas[colunaPI].posicao;
                rnpP = matrizPiParam[nosFalta[0]].colunas[colunaPI].idRNP;
                copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
                //marca os setores como em falta e conta os consumidores (E calcula a Energia Não Suprida)
                for(indice1 = 0; indice1 < numeroFalta; indice1++)
                {
                   grafoSetoresParam[nosFalta[indice1]].setorFalta = true;
                   //conta os consumidores sem fornecimento
                   consumidoresSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidores;
                   consumidoresEspeciaisSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidoresEspeciais;
                }
               //copia os setores anteriores a falta para a nova rnp origem
               for (indice1 = 0; indice1 < indiceP; indice1++) {
                   rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                   rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
               }
               //remove o setor em falta da RNP e copia os demais se existir
                if ((indiceP + numeroFalta) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                   for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta; indice1++) {
                       rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].idNo;
                       rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].profundidade;
                       adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                   }

               }

               configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
               for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                   if (indice1 != rnpP)
                       configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

               }
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
               insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
               //insere no vetor Pi a nova solução
               atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                       idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), 1, estadoInicialCA, estadoInicialCF, nosPRA);
               //idNovaConfiguracaoParam[0]++;
           } else {
               //fclose(arquivo1);
               //*************************************************************************************************//
               indiceRestabelecimento = (int **) malloc(totalCombinacoes * sizeof (int*));
               //cada coluna da matriz indiceRestabelecimento contém os indices do vetor de opções de restabelecimento para
               //cada bloco de setores a jusante que poderá ser restabelecido.
               //Cada linha contém uma combinação das opções de restabelecimento para cada bloco
               for (indice = 0; indice < totalCombinacoes; indice++) {
                   indiceRestabelecimento[indice] = (int *) malloc(blocoRestabelecimento * sizeof (int));
               }

               //preenche as combinacoes de indices das opçoes de restabelecimento para cada raiz de bloco de setores a jusante
               indiceAux3 = 0;
               for (indice = 0; indice < blocoRestabelecimento; indice++) {
                   if (nos[indice].numeroNos > 0) {
                       repeticoes = totalCombinacoes / nos[indice].numeroNos;
                       indiceAux4 = 0;
                       for (indiceAux1 = 0; indiceAux1 < repeticoes; indiceAux1++) {
                           for (indiceAux2 = 0; indiceAux2 < nos[indice].numeroNos; indiceAux2++) {
                               indiceRestabelecimento[indiceAux4][indiceAux3] = indiceAux2;
                               indiceAux4++;
                           }
                       }
                       indiceAux3++;
                   }
               }

               //aplica as combinações de restabelecimento obtidas.
               for (indiceAux1 = 0; indiceAux1 < totalCombinacoes; indiceAux1++) {
                   idConfiguracaoBase = idConfiguracaoParam;
                   indiceChave = 1;
                   idChaveAberta = Malloc(long int, (blocoRestabelecimento + 1));
                   idChaveFechada = Malloc(long int, (blocoRestabelecimento + 1));
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.comCargaManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoAutomatica= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.curtoManual= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.contadorManobrasTipo.seca= configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
                   buscaChaveIsolaFaltaModificada(grafoSetoresParam, idNovaConfiguracaoParam[0],
                           configuracoesParam, numChaveAberta, idChaveAberta,
                           idChaveFechada, setorFaltaParam, noAnteriorFalta,
                           (blocoRestabelecimento + 1), listaChavesParam);
                   casoManobra = 1;
                   estadoInicialCA = Malloc(BOOL, (blocoRestabelecimento + 1));
                   estadoInicialCF = Malloc(BOOL, (blocoRestabelecimento + 1));

                   nosPRA = Malloc(NOSPRA, (blocoRestabelecimento + 1));
                   estadoInicialCA[0] = false;
                   estadoInicialCF[0] = false;
                   nosPRA[0].a = -1;
                   nosPRA[0].p = setorFaltaParam;
                   nosPRA[0].r = -1;
                   consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
                   consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
                   copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
                   insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);


                   for (indiceAux2 = 0; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
                	   if(indiceAux2 != 0) //Por Leandro: após aplicar a primeira transferência para RNPFictícia, a configuração base passa a ser a configuração gerada após o primeiro corte
                		   idConfiguracaoBase = idNovaConfiguracaoParam[0];
                       //printf("%d \t", indiceRestabelecimento[indiceAux1][indiceAux2]);
                       indice = indiceAux2;
                       posicao = indiceRestabelecimento[indiceAux1][indiceAux2];
                       rnpA = nos[indice].rnpA[posicao]; //A rnpA guardará, nesta função, sempre o índice da RNP FICTÍCIA na qual a subárvore será reconectada
                       a = nos[indice].noA[posicao]; //O Nó A é sempre um nó raiz de uma RNP FICTÌCIA.
                       r = nos[indice].noR[posicao];
                       p = nos[indice].noP[posicao];
                       if (rnpA != -1) {
                           colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idConfiguracaoBase);
                           indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
                           //rnpA = matrizPiParam[a].colunas[colunaPI].idRNP;
                           colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idConfiguracaoBase);
                           indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
                           colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, p, idConfiguracaoBase);
                           indiceP = matrizPiParam[p].colunas[colunaPI].posicao;
                           indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);
                           p = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceP].idNo;
                           r = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceR].idNo;
                           //a = configuracoesParam[idConfiguracaoBase].rnp[rnpA].nos[indiceA].idNo;
                           nosPRA[indiceAux2+1].a = a;
                           nosPRA[indiceAux2+1].p = p;
                           nosPRA[indiceAux2+1].r = r;
                           //localiza a chave a ser aberta
                           indice1 = 0;
                           while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != p) {
                               indice1++;
                           }
                           idChaveAberta[indiceAux2 + 1] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
                           //localiza a chave a ser fechada
/*                           indice1 = 0;
                           while (grafoSetoresParam[r].setoresAdjacentes[indice1] != a) {
                               indice1++;
                           }
                           idChaveFechada[indiceAux2 + 1] = grafoSetoresParam[r].idChavesAdjacentes[indice1];*/
                           idChaveFechada[indiceAux2 + 1] = -1; //A carga não será restabelecida, logo não há chave a ser fechada. A carga será conectada à RNP Fictícia
                           //ROTINA PARA CALCULAR OS PARES DE MANOBRAS
                           numeroManobrasRestabelecimentoModificada(configuracoesParam, idChaveAberta[indiceAux2 + 1], idChaveFechada[indiceAux2 + 1], listaChavesParam,
                                   idNovaConfiguracaoParam[0], estadoInicial, &estadoInicialCA[indiceAux2+1], &estadoInicialCF[indiceAux2+1], idNovaConfiguracaoParam[0], &casoManobra);

                           tamanhoTemporario = indiceL - indiceP + 1;
                           //OBTENÇÃO DA NOVA RNP DESTINO FICTÍCIA
                           //alocação da RNP
//                         alocaRNP(configuracoesParam[idConfiguracaoBase].rnpFicticia[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                           alocaRNP(1 + tamanhoTemporario, &configuracoesParam[idNovaConfiguracaoParam[0]].rnpFicticia[rnpA]);
                           configuracoesParam[idNovaConfiguracaoParam[0]].rnpFicticia[rnpA].nos[0].idNo = numeroSetores + 1 + rnpA;
                           configuracoesParam[idNovaConfiguracaoParam[0]].rnpFicticia[rnpA].nos[0].profundidade = 0;
                           adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnpFicticia[rnpA].nos[0].idNo, idNovaConfiguracaoParam[0], rnpA + configuracoesParam[idNovaConfiguracaoParam[0]].numeroRNP, 0);
                           //Constroi a nova rnp de destino
//                           constroiRNPDestinoComDestinoFicticia(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceP, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam[0]);
                           constroiRNPDestinoComDestinoFicticia(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceP, matrizPiParam, idConfiguracaoBase, &configuracoesParam[idNovaConfiguracaoParam[0]].rnpFicticia[rnpA], idNovaConfiguracaoParam[0]);
                           configuracoesParam[idNovaConfiguracaoParam[0]].numeroRNPFicticia++;


                           //OBTENÇÃO DA NOVA RNP ORIGEM
                           //aloca e constroi uma rnp Origem Temporária (auxiliar)
                           alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                           constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam[0]);
                           if(idNovaConfiguracaoParam[0] != idConfiguracaoBase)//Se a rnpP da nova configuração aina não foi alocada
                        	   alocaRNP(rnpOrigem.numeroNos, &configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP]);
                           else{//se já foi alocada, então, basta apenas realocar para o novo tamanho, que é dado pelo número de nós em "rnpOrigem"
                        	   configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = rnpOrigem.numeroNos;
                        	   realloc(configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos, rnpOrigem.numeroNos*sizeof(NORNP)); //realoca
                           }
                           //Constroi a nova RNP origem
                           for (indice1 = 0; indice1 < rnpOrigem.numeroNos; indice1++) {
                        	   configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = rnpOrigem.nos[indice1].idNo;
                        	   configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = rnpOrigem.nos[indice1].profundidade;
                           }
                           //insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpA);
                           rnpOrigem.numeroNos = 0;
                           free(rnpOrigem.nos);
                       }
                       vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
                   }

                   //COPIA OS PONTEIROS PARA AS RNPs QUE NÃO FORAM ALTERADAS
				   for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
					   if (indice1 != rnpP)
						   configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
				   }


                   //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
                   tamanhoTemporario = 1;

                   //remove da solução inicial o setor em falta e os que não puderam ser restabelecidos
                    for (indice = 0; indice < numeroFalta; indice++) {
                       colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam[0]);
                      //colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idConfiguracaoParam);
                       indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
                       rnpP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].idRNP;
                       grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
                       //conta os consumidores sem fornecimento
                       consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
                       consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
                       if ((indiceP + 1) <= configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos) {
                           for (indice1 = indiceP; indice1 < configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1; indice1++) {
                               configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].idNo;
                               configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].profundidade;
                               adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                           }
                           configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1;
                       }
                   }
                   //atualiza consumidores sem fornecimento
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                   configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                   //insere no vetor Pi a nova solução
                   atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                           idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1), casoManobra,
                           estadoInicialCA, estadoInicialCF, nosPRA);
                   //idNovaConfiguracaoParam[0]++;

               }
               for (indice = 0; indice < blocoRestabelecimento; indice++) {
                   free(nos[indice].noA);
                   free(nos[indice].noR);
                   free(nos[indice].noP);
                   free(nos[indice].rnpA);
               }
               free(nos);
           }





       }
       else{ //Os setores a jusante não podem ser restabelecidos por falta de chaves NAs
    	   //configuracoesParam[idNovaConfiguracaoParam[0]].numeroRNPFicticia = 0;

           configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
           configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
           idChaveAberta = Malloc(long int, 1);
           idChaveFechada = Malloc(long int, 1);
           buscaChaveIsolaFaltaModificada(grafoSetoresParam, idNovaConfiguracaoParam[0],
                   configuracoesParam, numChaveAberta, idChaveAberta,
                   idChaveFechada, setorFaltaParam, noAnteriorFalta,
                   1, listaChavesParam);
           nosPRA = Malloc(NOSPRA, 1);
           nosPRA[0].a = -1;
           nosPRA[0].p = setorFaltaParam;
           nosPRA[0].r = -1;
           estadoInicialCA = Malloc(BOOL, 1);
           estadoInicialCF = Malloc(BOOL, 1);
           estadoInicialCA[0] = false;
           estadoInicialCF[0] = false;
           alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1, &rnpOrigem);
           //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
           colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
           indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
           rnpP =matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
           grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
           //conta os consumidores sem fornecimento
           consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
           consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
           //copia os setores anteriores a falta para a nova rnp origem
           for (indice1 = 0; indice1 < indiceP; indice1++) {
               rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
               rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
           }
           //remove o setor em falta da RNP e copia os demais se existir
            if ((indiceP + 1) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
              for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
                   rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
                   rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
                   adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
               }

           }

           configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;

           for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
               if (indice1 != rnpP)
                   configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

           }
           copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
           insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
           configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
           configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
           //insere no vetor Pi a nova solução
           atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
                   idChaveAberta, idChaveFechada, 1, casoManobra,
                   estadoInicialCA, estadoInicialCF, nosPRA);
           //idNovaConfiguracaoParam[0]++;
       }


   }
   else{ //o setor em falta é o ultimo da rnp.
//	   configuracoesParam[idNovaConfiguracaoParam[0]].numeroRNPFicticia = 0; //Se o setor em Falta é o última da RNP, então não há subárvores à jusante

       configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
               configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
       idChaveAberta = Malloc(long int, 1);
       idChaveFechada = Malloc(long int, 1);
       buscaChaveIsolaFaltaModificada(grafoSetoresParam, idNovaConfiguracaoParam[0],
               configuracoesParam, numChaveAberta, idChaveAberta,
               idChaveFechada, setorFaltaParam, noAnteriorFalta,
               1, listaChavesParam);
       nosPRA = Malloc(NOSPRA, 1);
       nosPRA[0].a = -1;
       nosPRA[0].p = setorFaltaParam;
       nosPRA[0].r = -1;
       estadoInicialCA = Malloc(BOOL, 1);
       estadoInicialCF = Malloc(BOOL, 1);
       estadoInicialCA[0] = false;
       estadoInicialCF[0] = false;
       //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
       tamanhoTemporario = 1;
       alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
       //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
       colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
       indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
       rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
       grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
       //conta os consumidores sem fornecimento
       consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
       consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
       //copia os setores anteriores a falta para a nova rnp origem
       for (indice1 = 0; indice1 < indiceP; indice1++) {
           rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
           rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
       }

       rnpOrigem.numeroNos = configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
       //insere a nova rnpOrigem na configuração e atualiza os ponteiros das demais
       configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
       for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
           if (indice1 != rnpP)
               configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
       }
       copiaListaRnps(configuracoesParam, idNovaConfiguracaoParam[0], idConfiguracaoParam);
       insereRNPLista(&configuracoesParam[idNovaConfiguracaoParam[0]], rnpP);
       configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
       configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
       //insere no vetor Pi a nova solução
       atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam,
               idChaveAberta, idChaveFechada, 1, casoManobra, estadoInicialCA, estadoInicialCF, nosPRA);

   }

   //Por Leandro
   vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
   //vetorPiParam[idNovaConfiguracaoParam[0]].operador = IIO;
}


/**
 * Faz a busca pela chave a montante para isolar a falta.
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param numChaveAberta
 * @param idChaveAberta
 * @param idChaveFechada
 * @param setorFaltaParam
 * @param noAnteriorFalta
 * @param numeroManobras
 * @param listaChavesParam
 */
void buscaChaveIsolaFalta(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, long int *numChaveAberta, long int *idChaveAberta,
        long int *idChaveFechada, long int setorFaltaParam, long int noAnteriorFalta,
        int numeroManobras, LISTACHAVES *listaChavesParam) {
    //localiza a chave que liga o setor em falta ao seu nó raiz na lista de adjacentes do setor em falta
    long int indice1 = 0;
    while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != noAnteriorFalta) {
        indice1++;
    }
    numChaveAberta[0]++;
    //(*idChaveAberta) = Malloc(long int, numeroManobras);
    //(*idChaveFechada) = Malloc(long int, numeroManobras);
    //insere a chave a ser manobrada para isolar a falta do alimentador
    idChaveAberta[0] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
    idChaveFechada[0] = -1; //não precisa fechar chave
    if (listaChavesParam[idChaveAberta[0]].tipoChave == chaveAutomatica)
        configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas += 1;
    else
        configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais += 1;

}

/**
 * Por Leandro: consiste na função "buscaChaveIsolaFalta()" modificada para que,
 * além de contabilizar a manobra da chave à montante da falta (cuja abertura isolá-a),
 * seja contabilizado também o tempo requerido por tal manobra.
 * @param grafoSetoresParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param numChaveAberta
 * @param idChaveAberta
 * @param idChaveFechada
 * @param setorFaltaParam
 * @param noAnteriorFalta
 * @param numeroManobras
 * @param listaChavesParam
 */
void buscaChaveIsolaFaltaModificada(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, long int *numChaveAberta, long int *idChaveAberta,
        long int *idChaveFechada, long int setorFaltaParam, long int noAnteriorFalta,
        int numeroManobras, LISTACHAVES *listaChavesParam) {
    //localiza a chave que liga o setor em falta ao seu nó raiz na lista de adjacentes do setor em falta
    long int indice1 = 0;
    while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != noAnteriorFalta) {
        indice1++;
    }
    numChaveAberta[0]++;
    //(*idChaveAberta) = Malloc(long int, numeroManobras);
    //(*idChaveFechada) = Malloc(long int, numeroManobras);
    //insere a chave a ser manobrada para isolar a falta do alimentador
    idChaveAberta[0] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
    idChaveFechada[0] = -1; //não precisa fechar chave
    if (listaChavesParam[idChaveAberta[0]].tipoChave == chaveAutomatica){
        configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas += 1;
        configuracoesParam[idConfiguracaoParam].objetivo.tempo += tempoOperacaoChaveAutomatica;
    }
    else{
        configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais += 1;
        configuracoesParam[idConfiguracaoParam].objetivo.tempo += tempoOperacaoChaveManual;
    }

}

/*Para reestabelecimento a Configuração Inicial do SDR é aquela com o setor em falta isolado e as cargas a jusante reconectadas. Isto é para Restabelecimento, a Configuração
 * Inicial é diferente da Topologia Inicial. Nesse caso, o vetor 'estadoInicial', que armazena os estados da chave na Configuração Inicial, deve ser atualizado após a definição
 * das chaves que isolão a falta e reconectão as cargas a jusante. Porém, isso não acontecia. Para corrigir essa falha, além das duas linhas de código que foram
 * inserirdas ao final da função 'numeroManobras', foram inseridos também os caminhos abaixo que visam determinar o nome da chave que conecta o setor em falta a seu
 * alimentador, chave esta que deve ser aberta para isolar a falta. Esses comandos determinam o nome dessa chave e alteram o seu estado para 'normalmenteAberta' no vetor 'estadoInical'.
 */

/*void isolaRestabeleceTodasOpcoes(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam) {
    int tamanhoTemporario;
    long int p, a, indice, indice1, indiceP, indiceR, indiceL, r,
            indiceSetorFalta, indiceA;
    long int noAnteriorFalta;
    long int indiceLFalta, idConfiguracaoBase;
    int rnpP, rnpA;
    int numeroFalta, numeroAJusante, posicao;
    long int nosFalta[100]; //é o número máximo de setores em falta ou a jusantes
    long int nosJusanteFalta[100]; //contém as posições da rnp referente a raiz de subárvore de nós a jusante a falta
    long int totalCombinacoes;
    long int *idChaveAberta, *idChaveFechada;
    long int consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    long int consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    int indiceChave;
    int **indiceRestabelecimento;
    int blocoRestabelecimento=0;
    int repeticoes, indiceAux1, indiceAux2, indiceAux3, indiceAux4;
    int colunaPI;
    RNP rnpOrigem;
    RNP rnpDestino;
    NOSRESTABELECIMENTO *nos;

    //IMPRESSAO DE DADOS PARA ANALISE DOS RESULTADOS
    char nomeArquivo[120];
    FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
    //todos os setores do sistema
    sprintf(nomeArquivo, "testeConfiguracaoInicial.dad");
    arquivo1 = fopen(nomeArquivo, "a");

    //marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
    grafoSetoresParam[setorFaltaParam].setorFalta = true;
    //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
    consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
    consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;

    //RECUPERA A COLUNAPI DO SETOR EM FALTA
    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
    rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
    //considera como possível nó P o nó subsequente ao setor em falta na RNP, deve ser verificado se a posição existe e se ela corresponde a setores a jusante a falta.
    indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
    indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

    //LOCALIZA O SETOR ANTERIOR A FALTA
    indice1 = indiceSetorFalta - 1; //Ao final do laço de repetição, 'indice1' armazenará o éndice do nó antecessor ao nó sob falta
    while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
        indice1--;
    }
    noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

    //verifica se o setor em falta não é o último da RNP
    if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
        numeroAJusante = 0;
        //verifica se o setor da posição do indiceP corresponde a um setor a jusante a falta
        if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
            indice1 = indiceP + 1;
            nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
            numeroAJusante++;
            //recupera a raiz de todas as subárvores que contém setores a jusante do setor em falta
            while (indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade > (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade)) {
                if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
                    nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    numeroAJusante++;
                }
                indice1++;
            }
            indiceLFalta = indice1 - 1;
            indice = 0;
            nosFalta[0] = setorFaltaParam;
            numeroFalta = 1;
            nos = Malloc(NOSRESTABELECIMENTO, numeroAJusante);
            totalCombinacoes = 1;
            blocoRestabelecimento = 0;
            //percorre a lista de raizes de subárvore a jusante do setor em falta verificando as opções de restabelecimento
            while (indice < numeroAJusante) {
                a = 0;
                colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
                p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nó 'p' é o nó subsequente ao nó 'setor em falta'
                //recupera o indice final da subárvore dos setore a jusante
                indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
                nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
                nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
                buscaTodosNosRestabelecimento(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
                printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
                if (nos[blocoRestabelecimento].numeroNos > 0) {
                    totalCombinacoes *= nos[blocoRestabelecimento].numeroNos;
                    blocoRestabelecimento++;

                } else {
                    free(nos[blocoRestabelecimento].noA);
                    free(nos[blocoRestabelecimento].noR);
                    free(nos[blocoRestabelecimento].noP);
                    free(nos[blocoRestabelecimento].rnpA);
                    //adiciona os setores a jusante sem opçao de restabelecimento ao grupo de setores em falta
                    nosFalta[numeroFalta] = nosJusanteFalta[indice];
                    numeroFalta++;
                    colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
                    indiceAux1 = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao + 1;
                    // printf("%d\n",indiceAux1);
                    while (indiceAux1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].profundidade > configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao].profundidade) {
                        nosFalta[numeroFalta] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceAux1].idNo;
                        numeroFalta++;
                        indiceAux1++;
                    }
                }
                indice++;
            }
            // ******************************************************************************************************\\
            //salva no arquivo quantas combinacoes de restabelecimento existem e quantos setores são agrupados a falta
            fprintf(arquivo1, "Combinacoes;%ld;Setores;%d;", totalCombinacoes, numeroFalta);
            if (blocoRestabelecimento == 0) { //não existe opção de restabelecimento para os setores a jusante.
                fprintf(arquivo1, "SAR\n");
                fclose(arquivo1);
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                //cria um novo individuo isolando a falta e os setores sem restabelecimento
                //isola a falta
                buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                        configuracoesParam, numChaveAberta, &idChaveAberta,
                        &idChaveFechada, setorFaltaParam, noAnteriorFalta,
                        1, listaChavesParam);
                //aloca a nova rnp
                alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta, &rnpOrigem);
                //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                 colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[0], idConfiguracaoParam);
                 indiceP = matrizPiParam[nosFalta[0]].colunas[colunaPI].posicao;
                 rnpP = matrizPiParam[nosFalta[0]].colunas[colunaPI].idRNP;
                 //marca os setores como em falta e conta os consumidores
                 for(indice1 = 0; indice1 < numeroFalta; indice1++)
                 {
                    grafoSetoresParam[nosFalta[indice1]].setorFalta = true;
                    //conta os consumidores sem fornecimento
                    consumidoresSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidores;
                    consumidoresEspeciaisSemFornecimento += grafoSetoresParam[nosFalta[indice1]].numeroConsumidoresEspeciais;
                 }
                //copia os setores anteriores a falta para a nova rnp origem
                for (indice1 = 0; indice1 < indiceP; indice1++) {
                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
                }
                //remove o setor em falta da RNP e copia os demais se existir
                if ((indiceP + numeroFalta) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                    for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - numeroFalta; indice1++) {
                        rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].idNo;
                        rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + numeroFalta].profundidade;
                        adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                    }

                }

                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
                for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                    if (indice1 != rnpP)
                        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

                }
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                //insere no vetor Pi a nova solução
                atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam, idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1));
                idNovaConfiguracaoParam[0]++;
            } else {
                fclose(arquivo1);
                // *************************************************************************************************\\
                indiceRestabelecimento = (int **) malloc(totalCombinacoes * sizeof (int*));
                //cada coluna da matriz indiceRestabelecimento contém os indices do vetor de opções de restabelecimento para
                //cada bloco de setores a jusante que poderá ser restabelecido.
                //Cada linha contém uma combinação das opções de restabelecimento para cada bloco
                for (indice = 0; indice < totalCombinacoes; indice++) {
                    indiceRestabelecimento[indice] = (int *) malloc(blocoRestabelecimento * sizeof (int));
                }

                //preenche as combinacoes de indices das opçoes de restabelecimento para cada raiz de bloco de setores a jusante
                indiceAux3 = 0;
                for (indice = 0; indice < blocoRestabelecimento; indice++) {
                    if (nos[indice].numeroNos > 0) {
                        repeticoes = totalCombinacoes / nos[indice].numeroNos;
                        indiceAux4 = 0;
                        for (indiceAux1 = 0; indiceAux1 < repeticoes; indiceAux1++) {
                            for (indiceAux2 = 0; indiceAux2 < nos[indice].numeroNos; indiceAux2++) {
                                indiceRestabelecimento[indiceAux4][indiceAux3] = indiceAux2;
                                indiceAux4++;
                            }
                        }
                        indiceAux3++;
                    }
                }

                //aplica as combinações de restabelecimento obtidas.
                for (indiceAux1 = 0; indiceAux1 < totalCombinacoes; indiceAux1++) {
                    idConfiguracaoBase = idConfiguracaoParam;
                    indiceChave = 1;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                    buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                            configuracoesParam, numChaveAberta, &idChaveAberta,
                            &idChaveFechada, setorFaltaParam, noAnteriorFalta,
                            (blocoRestabelecimento + 1), listaChavesParam);
                    consumidoresSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresSemFornecimento;
                    consumidoresEspeciaisSemFornecimento = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
                    for (indiceAux2 = 0; indiceAux2 < blocoRestabelecimento; indiceAux2++) {
                        //printf("%d \t", indiceRestabelecimento[indiceAux1][indiceAux2]);
                        indice = indiceAux2;
                        posicao = indiceRestabelecimento[indiceAux1][indiceAux2];
                        rnpA = nos[indice].rnpA[posicao];
                        a = nos[indice].noA[posicao];
                        r = nos[indice].noR[posicao];
                        p = nos[indice].noP[posicao];
                        if (rnpA != -1) {
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, a, idConfiguracaoBase);
                            indiceA = matrizPiParam[a].colunas[colunaPI].posicao;
                            rnpA = matrizPiParam[a].colunas[colunaPI].idRNP;
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, r, idConfiguracaoBase);
                            indiceR = matrizPiParam[r].colunas[colunaPI].posicao;
                            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, p, idConfiguracaoBase);
                            indiceP = matrizPiParam[p].colunas[colunaPI].posicao;
                            indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoBase].rnp[rnpP], indiceP);
                            p = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceP].idNo;
                            r = configuracoesParam[idConfiguracaoBase].rnp[rnpP].nos[indiceR].idNo;
                            a = configuracoesParam[idConfiguracaoBase].rnp[rnpA].nos[indiceA].idNo;
                            //localiza a chave a ser aberta
                            indice1 = 0;
                            while (grafoSetoresParam[setorFaltaParam].setoresAdjacentes[indice1] != p) {
                                indice1++;
                            }
                            idChaveAberta[indiceAux2 + 1] = grafoSetoresParam[setorFaltaParam].idChavesAdjacentes[indice1];
                            //localiza a chave a ser fechada
                            indice1 = 0;
                            while (grafoSetoresParam[r].setoresAdjacentes[indice1] != a) {
                                indice1++;
                            }
                            idChaveFechada[indiceAux2 + 1] = grafoSetoresParam[r].idChavesAdjacentes[indice1];
                            //ROTINA PARA CALCULAR OS PARES DE MANOBRAS

                            numeroManobras(configuracoesParam, idChaveAberta[indiceAux2 + 1], idChaveFechada[indiceAux2 + 1], listaChavesParam,
                                    idNovaConfiguracaoParam[0], estadoInicial, numChaveAberta, numChaveFechada, idNovaConfiguracaoParam[0]);

                            tamanhoTemporario = indiceL - indiceP + 1;
                            //realiza a alocação das RNPs que serão alteradas
                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                            alocaRNP(configuracoesParam[idConfiguracaoBase].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                            //obtém a nova rnp de destino
                            constroiRNPDestinoRestabelecimento(configuracoesParam, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPiParam, idConfiguracaoBase, &rnpDestino, idNovaConfiguracaoParam[0]);
                            constroiRNPOrigemRestabelecimento(configuracoesParam, rnpP, indiceL, indiceP, matrizPiParam, idConfiguracaoBase, &rnpOrigem, idNovaConfiguracaoParam[0]);
                            //libera a memória utilizada pela rnp do setor em falta

                            //###############   ARMAZENA O PONTEIRO PARA FLORESTA    ###############
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpA] = rnpDestino;
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
                            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoBase].numeroRNP; indice1++) {
                                if (indice1 != rnpA && indice1 != rnpP)
                                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoBase].rnp[indice1];
                            }

                        }
                        idConfiguracaoBase = idNovaConfiguracaoParam[0];
                        vetorPiParam[idNovaConfiguracaoParam[0]].idAncestral = idConfiguracaoParam;
                    }

                    //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
                    tamanhoTemporario = 1;

                    //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
                    for (indice = 0; indice < numeroFalta; indice++) {
                        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosFalta[indice], idNovaConfiguracaoParam[0]);
                        indiceP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].posicao;
                        rnpP = matrizPiParam[nosFalta[indice]].colunas[colunaPI].idRNP;
                        grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
                        //conta os consumidores sem fornecimento
                        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
                        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
                        if ((indiceP + 1) <= configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos) {
                            for (indice1 = indiceP; indice1 < configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1; indice1++) {
                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].idNo;
                                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].profundidade = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1 + 1].profundidade;
                                adicionaColuna(matrizPiParam, configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                            }
                            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos = configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP].numeroNos - 1;
                        }
                    }
                    //atualiza consumidores sem fornecimento
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
                    configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
                    //insere no vetor Pi a nova solução
                    atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam, idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1));
                    idNovaConfiguracaoParam[0]++;

                }
                for (indice = 0; indice < blocoRestabelecimento; indice++) {
                    free(nos[indice].noA);
                    free(nos[indice].noR);
                    free(nos[indice].noP);
                    free(nos[indice].rnpA);
                }
                free(nos);
            }
        } else //não existem setores a jusante para ser restabelecidos
        {
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                    configuracoesParam, numChaveAberta, &idChaveAberta,
                    &idChaveFechada, setorFaltaParam, noAnteriorFalta,
                    1, listaChavesParam);
            alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1, &rnpOrigem);
            //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
            colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
            indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
            rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
            grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
            //conta os consumidores sem fornecimento
            consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
            consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
            //copia os setores anteriores a falta para a nova rnp origem
            for (indice1 = 0; indice1 < indiceP; indice1++) {
                rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
                rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
            }
            //remove o setor em falta da RNP e copia os demais se existir
            if ((indiceP + 1) <= configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
                for (indice1 = indiceP; indice1 < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1; indice1++) {
                    rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].idNo;
                    rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1 + 1].profundidade;
                    adicionaColuna(matrizPiParam, rnpOrigem.nos[indice1].idNo, idNovaConfiguracaoParam[0], rnpP, indice1);
                }

            }

            configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
            for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
                if (indice1 != rnpP)
                    configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];

            }
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
            configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
            //insere no vetor Pi a nova solução
            atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam, idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1));
            idNovaConfiguracaoParam[0]++;
        }
    } else //o setor em falta é o ultimo da rnp
    {
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasAutomaticas= configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.manobrasManuais= configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
        buscaChaveIsolaFalta(grafoSetoresParam, idNovaConfiguracaoParam[0],
                configuracoesParam, numChaveAberta, &idChaveAberta,
                &idChaveFechada, setorFaltaParam, noAnteriorFalta,
                1, listaChavesParam);
        //Elimina Setor em Falta da estrutura e reduz o tamanho da arvore p
        tamanhoTemporario = 1;
        alocaRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
        //remove o setor em falta e os que não puderam ser restabelecidos da solução inicial
        colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
        indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao;
        rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
        grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].setorFalta = true;
        //conta os consumidores sem fornecimento
        consumidoresSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidores;
        consumidoresEspeciaisSemFornecimento += grafoSetoresParam[configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo].numeroConsumidoresEspeciais;
        //copia os setores anteriores a falta para a nova rnp origem
        for (indice1 = 0; indice1 < indiceP; indice1++) {
            rnpOrigem.nos[indice1].idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;
            rnpOrigem.nos[indice1].profundidade = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade;
        }

        rnpOrigem.numeroNos = configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos - 1;
        //insere a nova rnpOrigem na configuração e atualiza os ponteiros das demais
        configuracoesParam[idNovaConfiguracaoParam[0]].rnp[rnpP] = rnpOrigem;
        for (indice1 = 0; indice1 < configuracoesParam[idConfiguracaoParam].numeroRNP; indice1++) {
            if (indice1 != rnpP)
                configuracoesParam[idNovaConfiguracaoParam[0]].rnp[indice1] = configuracoesParam[idConfiguracaoParam].rnp[indice1];
        }

        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresSemFornecimento = consumidoresSemFornecimento;
        configuracoesParam[idNovaConfiguracaoParam[0]].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciaisSemFornecimento;
        //insere no vetor Pi a nova solução
        atualizaVetorPi(vetorPiParam, idNovaConfiguracaoParam[0], idConfiguracaoParam, idChaveAberta, idChaveFechada, (blocoRestabelecimento + 1));
        idNovaConfiguracaoParam[0]++;
    }




}*/

/*void isolaRestabeleceMultiplasFaltas(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
        CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, long int numeroFaltasParam, long int *setorFaltaParam, ESTADOCHAVE *estadoInicial, long int * numChaveAberta,
        long int *numChaveFechada, long int *idNovaConfiguracaoParam, long int *idConfiguracaoIniParam, LISTACHAVES *listaChavesParam) {
     long int idConfiguracaoIni = idConfiguracaoParam;
     long int idConfiguracaoFinal = idNovaConfiguracaoParam[0];
     int qtdConfiguracoesGeradas = 1;
     int contadorFalta;
     long int contadorConfiguracoes;
     for (contadorFalta = 0; contadorFalta < numeroFaltasParam; contadorFalta++) {
         for (contadorConfiguracoes = idConfiguracaoIni; contadorConfiguracoes < idConfiguracaoFinal; contadorConfiguracoes++) {
             //imprimeIndividuo(configuracoesParam[contadorConfiguracoes]);
             isolaRestabeleceTodasOpcoes(grafoSetoresParam, contadorConfiguracoes, configuracoesParam, matrizPiParam,
                     vetorPiParam, setorFaltaParam[contadorFalta], estadoInicial, numChaveAberta,numChaveFechada,
                     idNovaConfiguracaoParam, listaChavesParam);
         }
         idConfiguracaoIni = idConfiguracaoFinal;
         idConfiguracaoIniParam[0] = idConfiguracaoIni-1;
         idConfiguracaoFinal = idNovaConfiguracaoParam[0];
         printf("idConfiguracaoIni %ld idConfiguracaoIniParam %ld idConfiguracaoFinal %ld\n", idConfiguracaoIni,idConfiguracaoIniParam[0], idConfiguracaoFinal);
     }
     

}*/
/**
 * Retorna se existe um regulador de tensão entre as duas barras informadas.
 * @param barra1
 * @param barra2
 * @param grafoSDRParam
 * @return 
 */
BOOL eRegulador(long int barra1, long int barra2, GRAFO *grafoSDRParam) {
    BOOL encontrado = false;
    BOOL resultado = false;
    int contadorAdj = 0;

    while (contadorAdj < grafoSDRParam[barra1].numeroAdjacentes && !encontrado) {
        if (grafoSDRParam[barra1].adjacentes[contadorAdj].idNo == barra2) {
            encontrado = true;
            if (grafoSDRParam[barra1].adjacentes[contadorAdj].tipoAresta == reguladorTensao)
                resultado = true;
        }
    }

    return resultado;
}
/**
 * Método utilizado para cálculo do fluxo de potência de varredura direta inversa.
 * Percorre o alimentador informado na representação nó-profundidade calculando as correntes jusantes
 * no processo de varredura inversa do alimentador.
 * @param idRNP
 * @param carga informa se é utilizado o modelo de corrente constante (0) ou potência constante (1).
 * @param iteracao
 * @param configuracaoParam
 * @param matrizB
 * @param indiceRegulador
 * @param dadosRegulador
 */
void correnteJusante(int idRNP, int carga, int iteracao,
        CONFIGURACAO configuracaoParam, RNPSETORES *matrizB, int *indiceRegulador, DADOSREGULADOR *dadosRegulador) {
    long int indice, indice1, noS, noR, noN, idSetorS, idSetorR, idBarra1, idBarra2, indice2, indice3;
    double IMod, IAng;
    //long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = malloc(400 * sizeof(long int));
    __complex__ double iAcumulada;
    RNPSETOR rnpSetorSR;
    //calculo das correntes

    //determina o modelo de calculo
    if (iteracao != 0) //se nao é a primeira vez que calcula o módulo e o ângulo
    {
        indice = 0;
        noProf[configuracaoParam.rnp[idRNP].nos[indice].profundidade] = configuracaoParam.rnp[idRNP].nos[indice].idNo;
        for (indice = 1; indice < configuracaoParam.rnp[idRNP].numeroNos; indice++) {
            noS = configuracaoParam.rnp[idRNP].nos[indice].idNo;
            noR = noProf[configuracaoParam.rnp[idRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indice1 = 0; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;

                if (carga == 0) //modelo de corrente constante
                {


                    IMod = cabs(configuracaoParam.dadosEletricos.corrente[noN]);
                    IAng = carg(configuracaoParam.dadosEletricos.vBarra[noN]) - carg(configuracaoParam.dadosEletricos.potencia[noN]);
                    configuracaoParam.dadosEletricos.corrente[noN] = IMod * cos(IAng) + ij * IMod * sin(IAng);

                } else //modelo de potencia constante
                {
                    configuracaoParam.dadosEletricos.corrente[noN] = conj(configuracaoParam.dadosEletricos.potencia[noN] / configuracaoParam.dadosEletricos.vBarra[noN]);
                }
            }
            noProf[configuracaoParam.rnp[idRNP].nos[indice].profundidade] = configuracaoParam.rnp[idRNP].nos[indice].idNo;
        }
    }


    for (indice = configuracaoParam.rnp[idRNP].numeroNos - 1; indice > 0; indice--) {
        indice2 = indice - 1;
        //busca pelo nó raiz
        while (indice2 >= 0 && configuracaoParam.rnp[idRNP].nos[indice2].profundidade != (configuracaoParam.rnp[idRNP].nos[indice].profundidade - 1))
            indice2--;
        idSetorS = configuracaoParam.rnp[idRNP].nos[indice].idNo;
        idSetorR = configuracaoParam.rnp[idRNP].nos[indice2].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, idSetorS, idSetorR);

        for (indice2 = 0; indice2 < rnpSetorSR.numeroNos; indice2++) {//zera as variáveias iJusante
            idBarra1 = rnpSetorSR.nos[indice2].idNo;
            configuracaoParam.dadosEletricos.iJusante[idBarra1] = 0 + ij * 0;
        }
    }
    for (indice = configuracaoParam.rnp[idRNP].numeroNos - 1; indice > 0; indice--) {
        indice2 = indice - 1;
        //busca pelo nó raiz
        while (indice2 >= 0 && configuracaoParam.rnp[idRNP].nos[indice2].profundidade != (configuracaoParam.rnp[idRNP].nos[indice].profundidade - 1))
            indice2--;
        idSetorS = configuracaoParam.rnp[idRNP].nos[indice].idNo;
        idSetorR = configuracaoParam.rnp[idRNP].nos[indice2].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, idSetorS, idSetorR);
        for (indice2 = rnpSetorSR.numeroNos - 1; indice2 > 0; indice2--) {
            indice3 = indice2 - 1;
            while (indice3 >= 0 && rnpSetorSR.nos[indice3].profundidade != (rnpSetorSR.nos[indice2].profundidade - 1)) {
                indice3--;
            }
            idBarra1 = rnpSetorSR.nos[indice3].idNo;
            idBarra2 = rnpSetorSR.nos[indice2].idNo;
            iAcumulada = configuracaoParam.dadosEletricos.iJusante[idBarra2] + configuracaoParam.dadosEletricos.corrente[idBarra2];
            configuracaoParam.dadosEletricos.iJusante[idBarra1] = configuracaoParam.dadosEletricos.iJusante[idBarra1] + iAcumulada;

            // if (eRegulador(idBarra1, idBarra2,)) //existe um regulador de tensão entre as barras 1 e 2.
            //printf("idBarra %ld %ld\n", idBarra1, idBarra2);
            if ((indiceRegulador[idBarra1] != 0) && (indiceRegulador[idBarra2] != 0) && (indiceRegulador[idBarra1] == indiceRegulador[idBarra2])) {

                configuracaoParam.dadosEletricos.iJusante[idBarra1] = configuracaoParam.dadosEletricos.iJusante[idBarra1]*(1 + 0.00625 * dadosRegulador[indiceRegulador[idBarra1]].numeroTaps);
            }

        }

    }
    free(noProf); noProf= NULL;
}
/**
 * Calcula o número máximo de taps que podem ser utilizados no regulador de tensão durante o cálculo do fluxo de potência.
 * @param carregamentoRT
 * @param dadosReguladorSDR
 * @param regulador
 */
void calculaNumeroMaximoTaps(double carregamentoRT, DADOSREGULADOR *dadosReguladorSDR, long int regulador) {
    // *********************************************************************************************
    // Faz as condicoes para a Limitacao do tap em funcao do carregamento do RT (Tabela fornecida pelo Marcel - Copel)
    if (carregamentoRT < 1) { // Se o carregamento for menor que 100% o limite eh 16 (Quantidade maxima de taps em um RT)
        if (dadosReguladorSDR[regulador].numeroTaps > 16)
            dadosReguladorSDR[regulador].numeroTaps = 16;
        if (dadosReguladorSDR[regulador].numeroTaps < -16)
            dadosReguladorSDR[regulador].numeroTaps = -16;
    } else {
        if (carregamentoRT < 1.1) { // Se o carregamento estiver entre 100% e 110% o limite eh 14. (O tap comeca a ser limitado nao pela quantidade do RT mas sim em funcao da corrente que passa pelo RT)
            if (dadosReguladorSDR[regulador].numeroTaps > 14)
                dadosReguladorSDR[regulador].numeroTaps = 14;
            if (dadosReguladorSDR[regulador].numeroTaps < -14)
                dadosReguladorSDR[regulador].numeroTaps = -14;
        } else {
            if (carregamentoRT < 1.2) { // Se o carregamento estiver entre 110% e 120% o limite eh 12.
                if (dadosReguladorSDR[regulador].numeroTaps > 12)
                    dadosReguladorSDR[regulador].numeroTaps = 12;
                if (dadosReguladorSDR[regulador].numeroTaps < -12)
                    dadosReguladorSDR[regulador].numeroTaps = -12;
            } else {

                if (carregamentoRT < 1.35) { // Se o carregamento estiver entre 120% e 135% o limite eh 10.
                    if (dadosReguladorSDR[regulador].numeroTaps > 10)
                        dadosReguladorSDR[regulador].numeroTaps = 10;
                    if (dadosReguladorSDR[regulador].numeroTaps < -10)
                        dadosReguladorSDR[regulador].numeroTaps = -10;
                } else {

                    if (carregamentoRT > 1.35 && carregamentoRT < 1.6) { // Se o carregamento estiver entre 135% e 160% o limite eh 8.
                        if (dadosReguladorSDR[regulador].numeroTaps > 8)
                            dadosReguladorSDR[regulador].numeroTaps = 8;
                        if (dadosReguladorSDR[regulador].numeroTaps < -8)
                            dadosReguladorSDR[regulador].numeroTaps = -8;
                    } else { //o carregamento é maior do que 1.6
                        dadosReguladorSDR[regulador].numeroTaps = 0;
                    }
                }
            }
        }
    }
}

/**
 * Cálcula a tensão e a queda de tensão do alimentador na varredura direta do alimentador informado. 
 * Todo_ o cálculo é realizado com base na representação nó-profundidade do alimentador.
 * @param indiceRNP
 * @param configuracaoParam
 * @param VFParam
 * @param matrizB
 * @param ZParam
 * @param indiceRegulador
 * @param dadosReguladorSDR
 * @return 
 */
//calcula a tensão em cada barra, a menor tensão das barras e retorna a barra com menor tensão

/*long int tensaoQuedaTensao(int indiceRNP, CONFIGURACAO configuracaoParam, __complex__ double VFParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR) {
    int indice, indice1;
    long int noS, noR, noM, noN;
    int pior;
    int tapAnterior;
    double limiteInferior = 13491; // Tensao minima (tensao de linha - V) em uma barra regulada pelo RT;
    double limiteSuperior = 13763; // Tensao maxima (tensao de linha - V) em uma barra regulada pelo RT;
    double carregamentoRT;
    double aux;

    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int barraProf[400]; //armazena o ultimo nó do setor presente em uma profundidade, é indexado pela profundidade
    __complex__ double deltaV, valorz;
    double temporario;
    int tensao;
    double menorTensao;
    RNPSETOR rnpSetorSR;
    noProf[configuracaoParam.rnp[indiceRNP].nos[0].profundidade] = configuracaoParam.rnp[indiceRNP].nos[0].idNo;
    menorTensao = VFParam;
    flagMudancaTap = false;
    //printf("menorTensao %lf \n", menorTensao);
    for (indice = 1; indice < configuracaoParam.rnp[indiceRNP].numeroNos; indice++) {
        noS = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
        noR = noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        barraProf[rnpSetorSR.nos[0].profundidade] = rnpSetorSR.nos[0].idNo;
        //printf("Setor noS %ld noR %ld \t", noS, noR);
        //  imprimeRNPSetor(rnpSetorSR);
        for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
            noM = barraProf[rnpSetorSR.nos[indice1].profundidade - 1];
            noN = rnpSetorSR.nos[indice1].idNo;
            if (indiceRegulador[noM] != 0 && indiceRegulador[noN] != 0) //ha um regulador de tensao entre as barras m e n.
            {
                configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                tensao = (int) cabs(configuracaoParam.dadosEletricos.vBarra[noN] * sqrt(3));
                tapAnterior = dadosReguladorSDR[indiceRegulador[noM]].numeroTaps; // Grava o estado do tap antes das analises


                if (tensao > limiteSuperior || tensao < limiteInferior) { // Analises para a mudanca ou nao do tap

                    if (tensao > limiteSuperior) {
                        aux = limiteSuperior / (configuracaoParam.dadosEletricos.vBarra[noM] * sqrt(3)) - 1;

                        // Calcula os novos taps:
                        dadosReguladorSDR[indiceRegulador[noM]].numeroTaps = (int) floor(aux / passoRegulador); //arredondamento para baixo

                        // Calcula o carregamento do RT:
                        carregamentoRT = cabs(configuracaoParam.dadosEletricos.iJusante[noM]) / dadosReguladorSDR[indiceRegulador[noM]].ampacidade;

                        calculaNumeroMaximoTaps(carregamentoRT, dadosReguladorSDR, indiceRegulador[noM]);
                        // *********************************************************************************************

                        //    		  printf("tap[%d] = %d\n",indreg[m],regulador[indreg[m]].tap);
                        configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                    }
                } else {

                    if (tensao < limiteInferior) {
                        aux = limiteInferior / (configuracaoParam.dadosEletricos.vBarra[noM] * sqrt(3)) - 1;

                        // Calcula os novos taps:
                        dadosReguladorSDR[indiceRegulador[noM]].numeroTaps = (int) ceil(aux / passoRegulador); //arredondamento para cima

                        // Calcula o carregamento do RT:
                        carregamentoRT = cabs(configuracaoParam.dadosEletricos.iJusante[noM]) / dadosReguladorSDR[indiceRegulador[noM]].ampacidade;

                        calculaNumeroMaximoTaps(carregamentoRT, dadosReguladorSDR, indiceRegulador[noM]);

                        configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                    }
                }

                // Condicao para verificar se o tap foi alterado ou nao
                if (tapAnterior != dadosReguladorSDR[indiceRegulador[noM]].numeroTaps) {
                    flagMudancaTap = true;
                }

            } else {
                deltaV = valorZ(ZParam, noM, noN) * (configuracaoParam.dadosEletricos.iJusante[noN] + configuracaoParam.dadosEletricos.corrente[noN]);
                configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM] - deltaV;
            }
            #########################################################
                #################    Queda de Tensão    #################
            temporario = sqrt(pow(__real__ configuracaoParam.dadosEletricos.vBarra[noN], 2) + pow(__imag__ configuracaoParam.dadosEletricos.vBarra[noN], 2));
            if (temporario < menorTensao) {
                menorTensao = temporario;
                pior = noN;
            }
            //armazena o nó barra na sua profundidade
            barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;
        }
        //armazena o nó setor na sua profundidade
        noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
    }
    configuracaoParam.rnp[indiceRNP].fitnessRNP.menorTensao = menorTensao;
    // printf("\n");
    return (pior);
}*/

/*
 * Por Leandro: consiste na função "tensaoQuedaTensao()" modificada para armazenas o nó (barra) na qual está ocorrendo a menor queda
 * de tensão na rede
 *
 * Portanto, calcula a tensão em cada barra, a menor tensão das barras e retorna a barra com menor tensão
 */
long int tensaoQuedaTensaoModificada(int indiceRNP, CONFIGURACAO configuracaoParam, double VFParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR) {
    int indice, indice1;
    long int noS, noR, noM, noN;
    int pior;
    int tapAnterior;
    double limiteInferior = 13491; // Tensao minima (tensao de linha - V) em uma barra regulada pelo RT;
    double limiteSuperior = 13763; // Tensao maxima (tensao de linha - V) em uma barra regulada pelo RT;
    double carregamentoRT;
    double aux;

    //long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = malloc(400 * sizeof(long int));
    //long int barraProf[100]; //armazena o ultimo nó do setor presente em uma profundidade, é indexado pela profundidade
    long int *barraProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    barraProf = malloc(200 * sizeof(long int));

    __complex__ double deltaV, valorz;
    double temporario;
    int tensao;
    double menorTensao;
    long int noMenorTensao; // Por Leandro: salvará o nó (setor) da RNP no qual está contida a barra com o menor valor de tensão dentro da RNP em questão

    RNPSETOR rnpSetorSR;
    noProf[configuracaoParam.rnp[indiceRNP].nos[0].profundidade] = configuracaoParam.rnp[indiceRNP].nos[0].idNo;
    menorTensao = VFParam;
    flagMudancaTap = false;
    //printf("menorTensao %lf \n", menorTensao);
    for (indice = 1; indice < configuracaoParam.rnp[indiceRNP].numeroNos; indice++) {
        noS = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
        noR = noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        barraProf[rnpSetorSR.nos[0].profundidade] = rnpSetorSR.nos[0].idNo;
        //printf("Setor noS %ld noR %ld \t", noS, noR);
        //  imprimeRNPSetor(rnpSetorSR);
        for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
            noM = barraProf[rnpSetorSR.nos[indice1].profundidade - 1];
            noN = rnpSetorSR.nos[indice1].idNo;
            if (indiceRegulador[noM] != 0 && indiceRegulador[noN] != 0) //ha um regulador de tensao entre as barras m e n.
            {
                configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                tensao = (int) cabs(configuracaoParam.dadosEletricos.vBarra[noN] * sqrt(3));
                tapAnterior = dadosReguladorSDR[indiceRegulador[noM]].numeroTaps; // Grava o estado do tap antes das analises


                if (tensao > limiteSuperior || tensao < limiteInferior) { // Analises para a mudanca ou nao do tap

                    if (tensao > limiteSuperior) {
                        aux = limiteSuperior / (configuracaoParam.dadosEletricos.vBarra[noM] * sqrt(3)) - 1;

                        // Calcula os novos taps:
                        dadosReguladorSDR[indiceRegulador[noM]].numeroTaps = (int) floor(aux / passoRegulador); //arredondamento para baixo

                        // Calcula o carregamento do RT:
                        carregamentoRT = cabs(configuracaoParam.dadosEletricos.iJusante[noM]) / dadosReguladorSDR[indiceRegulador[noM]].ampacidade;

                        calculaNumeroMaximoTaps(carregamentoRT, dadosReguladorSDR, indiceRegulador[noM]);
                        //*********************************************************************************************

                        //    		  printf("tap[%d] = %d\n",indreg[m],regulador[indreg[m]].tap);
                        configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                    }
                } else {

                    if (tensao < limiteInferior) {
                        aux = limiteInferior / (configuracaoParam.dadosEletricos.vBarra[noM] * sqrt(3)) - 1;

                        // Calcula os novos taps:
                        dadosReguladorSDR[indiceRegulador[noM]].numeroTaps = (int) ceil(aux / passoRegulador); //arredondamento para cima

                        // Calcula o carregamento do RT:
                        carregamentoRT = cabs(configuracaoParam.dadosEletricos.iJusante[noM]) / dadosReguladorSDR[indiceRegulador[noM]].ampacidade;

                        calculaNumeroMaximoTaps(carregamentoRT, dadosReguladorSDR, indiceRegulador[noM]);

                        configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                    }
                }

                // Condicao para verificar se o tap foi alterado ou nao
                if (tapAnterior != dadosReguladorSDR[indiceRegulador[noM]].numeroTaps) {
                    flagMudancaTap = true;
                }

            } else {
                deltaV = valorZ(ZParam, noM, noN) * (configuracaoParam.dadosEletricos.iJusante[noN] + configuracaoParam.dadosEletricos.corrente[noN]);
                configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM] - deltaV;
            }
            /*#########################################################
                #################    Queda de Tensão    #################*/
            temporario = sqrt(pow(__real__ configuracaoParam.dadosEletricos.vBarra[noN], 2) + pow(__imag__ configuracaoParam.dadosEletricos.vBarra[noN], 2));

            if (temporario < menorTensao) {
                menorTensao = temporario;
                pior = noN;
                noMenorTensao = noS; // Por Leandro: salva o nó (setor) da RNP no qual está contida a barra com o menor valor de tensão dentro da RNP em questão
            }
            //armazena o nó barra na sua profundidade
            barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;
        }
        //armazena o nó setor na sua profundidade
        noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
    }

    configuracaoParam.rnp[indiceRNP].fitnessRNP.menorTensao = menorTensao;
    configuracaoParam.rnp[indiceRNP].fitnessRNP.noMenorTensao = noMenorTensao;
    // printf("\n");
    free(noProf); noProf = NULL;
    free(barraProf); barraProf = NULL;
    return (pior);
}



/**
 * Cálcula as perdas do alimentador informado no processo de varredura direta.
 * Os cálculos são realizados percorrendo a representação nó-profundidade do alimentador.
 * @param configuracaoParam
 * @param indiceRNP
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 */
/*void carregamentoPerdas(CONFIGURACAO *configuracaoParam, int indiceRNP, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam) {
    int indice, indice1;
    long int noS, noR, noN, noM;
    double temporario;
    double perda = 0;
    __complex__ double ijus, ibarra;
    RNPSETOR rnpSetorSR;
    double perdas = 0;
    __complex__ double perdasReativas, valorz;
    double carregamentoRede = 0;
    __complex__ double carregamentoAlimentador = 0 + ij * 0;


    //fprintf(arquivog, "Alimentador %d \n", indiceRNP+1);
    temporario = 0;
    ijus = 0;
    ibarra = 0;

    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int barraProf[400]; //armazena o ultimo nó do setor presente em uma profundidade, é indexado pela profundidade
    indice = 0;
    noProf[configuracaoParam->rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam->rnp[indiceRNP].nos[indice].idNo;
    for (indice = 1; indice < configuracaoParam->rnp[indiceRNP].numeroNos; indice++) {
        noS = configuracaoParam->rnp[indiceRNP].nos[indice].idNo;
        noR = noProf[configuracaoParam->rnp[indiceRNP].nos[indice].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        indice1 = 0;
        barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;
        for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
            noM = barraProf[rnpSetorSR.nos[indice1].profundidade - 1];
            noN = rnpSetorSR.nos[indice1].idNo;

          //  #########################################################
           //  * ############    Carregamento máximo da rede (em corrente)   ##########
            ijus = configuracaoParam->dadosEletricos.iJusante[noN];
            ibarra = configuracaoParam->dadosEletricos.corrente[noN];
            temporario = cabs(ijus + ibarra) / (maximoCorrente(maximoCorrenteParam, noM, noN));

            //  fprintf(arquivog,"BarraN %ld corrente %lf Ramo %ld--%ld corrente jusante %lf soma %lf carregamento %lf \n", noN, cabs(ibarra), noM, noN, cabs(ijus), cabs(ijus + ibarra), temporario*100);


            if (temporario > carregamentoRede) {
                carregamentoRede = temporario;
  //              printf("carregamento noM %ld noN %ld %lf\n", noM, noN, temporario);
            }
           // #########################################################
           //  * 	#####################      Perdas     #### ##############
            valorz = valorZ(ZParam, noM, noN);
            //
            perda = (3 * (pow((cabs(ijus + ibarra)), 2))) * __real__ valorz;
     //       printf("ramo %ld--%ld corrente %lf corrente jusante %lf soma %lf valorZ %lf perda %lf \n", noM, noN, cabs(ibarra), cabs(ijus), cabs(ijus + ibarra), __real__ valorz, perda);
            perdas = perdas + perda;
    //        printf("noM %ld noN %ld\t", noM, noN);
      //     printf("perda %.2lf perdas %lf \n", perda/1000, perdas/1000);
            perdasReativas = ij * 3 * pow((cabs(ijus + ibarra)), 2) * __imag__ valorZ(ZParam, noM, noN);
          //  #########################################################
           //   ###########    Carregamento dos alimentadores (em potência)   ##########
            carregamentoAlimentador = carregamentoAlimentador + (configuracaoParam->dadosEletricos.vBarra[noN] * conj(configuracaoParam->dadosEletricos.corrente[noN]))*3;

            __real__ carregamentoAlimentador += perda;

            carregamentoAlimentador += perdasReativas;
           // ##########################################################
            barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;

        }
        noProf[configuracaoParam->rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam->rnp[indiceRNP].nos[indice].idNo;
    }
   // printf("carregamento %.5lf \n", carregamentoRede*100);
    configuracaoParam->rnp[indiceRNP].fitnessRNP.potenciaAlimentador = carregamentoAlimentador;
    configuracaoParam->rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede = carregamentoRede;
    configuracaoParam->rnp[indiceRNP].fitnessRNP.demandaAlimentador = cabs(carregamentoAlimentador);
    configuracaoParam->rnp[indiceRNP].fitnessRNP.perdasResistivas = perdas / 1000;
}*/

/**
 * Por Leandro: consiste na função "carregamentoPerdas()" modificada para que
 * 1) seja salvo, ao final da função,  * o nó da RNP na qual está ocorrendo o maior carregamento de rede, bem como o valor desta sobrecarga
 * 2) sejam realizadas outra modificações que permitam executar a modificação 1.
 *
 * Portanto, além de fazer isto, esta função calcula as perdas do alimentador informado no processo de varredura direta.
 * Os cálculos são realizados percorrendo a representação nó-profundidade do alimentador.
 *
 * @param configuracaoParam
 * @param indiceRNP
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 */
void carregamentoPerdasModificada(CONFIGURACAO *configuracaoParam, int indiceRNP, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam) {
    int indice, indice1;
    long int noS, noR, noN, noM;
    double temporario, sobrecargaRede = 0, corrente, correnteMaximaSuportada;
    double perda = 0;
    __complex__ double ijus, ibarra;
    RNPSETOR rnpSetorSR;
    double perdas = 0;
    __complex__ double perdasReativas, valorz;
    double carregamentoRede = 0;
    __complex__ double carregamentoAlimentador = 0 + ij * 0;
    long int noCarregamentoRede = 0; // Por Leandro: salva o no da RNP no qual há o maior carregamento


    //fprintf(arquivog, "Alimentador %d \n", indiceRNP+1);
    temporario = 0;
    ijus = 0;
    ibarra = 0;
    sobrecargaRede = 0; //Por Leandro: Em Amperes

    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = Malloc(long int, 400);
    long int *barraProf; //armazena o ultimo nó do setor presente em uma profundidade, é indexado pela profundidade
    barraProf = Malloc(long int, 200);
    indice = 0;
    noProf[configuracaoParam->rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam->rnp[indiceRNP].nos[indice].idNo;
    for (indice = 1; indice < configuracaoParam->rnp[indiceRNP].numeroNos; indice++) {
        noS = configuracaoParam->rnp[indiceRNP].nos[indice].idNo;
        noR = noProf[configuracaoParam->rnp[indiceRNP].nos[indice].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        indice1 = 0;
        barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;
        for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
            noM = barraProf[rnpSetorSR.nos[indice1].profundidade - 1];
            noN = rnpSetorSR.nos[indice1].idNo;

            /*#########################################################
             * ############    Carregamento máximo da rede (em corrente)   ##########*/
            ijus = configuracaoParam->dadosEletricos.iJusante[noN];
            ibarra = configuracaoParam->dadosEletricos.corrente[noN];
            correnteMaximaSuportada = maximoCorrente(maximoCorrenteParam, noM, noN); //Por Leandro: criou-se esta várivável para que a função "máximoCorrente" fosse chamada somente uma única vez
            corrente = cabs(ijus + ibarra); //Por Leandro: criou-se esta várivável para que a chamada da função "cabs"  e esta soma fossem realizados uma única vez
            temporario = corrente / correnteMaximaSuportada;

            //  fprintf(arquivog,"BarraN %ld corrente %lf Ramo %ld--%ld corrente jusante %lf soma %lf carregamento %lf \n", noN, cabs(ibarra), noM, noN, cabs(ijus), cabs(ijus + ibarra), temporario*100);

            if (temporario > carregamentoRede) {
                carregamentoRede = temporario;
                noCarregamentoRede = noS; // Por Leandro: salva o setor (nó) da RNP no qual tem ocorrido o maior carregamento desta RNP

                //Por Leandro: Cálculo da corrente que excente o limite superior de corrente
                if(temporario > 1) //Por Leandro: se houver sobrecarga, então calcula-se a quantidade de Amperes que excede o limite superior de corrente entre as barras noM e noN
                	sobrecargaRede = corrente - correnteMaximaSuportada;

                //printf("noM %ld noN %ld %lf\n", noM, noN, temporario);
            }
            /*#########################################################
             * 	#####################      Perdas     #### ##############*/
            valorz = valorZ(ZParam, noM, noN);
            //
            perda = (3 * (pow((cabs(ijus + ibarra)), 2))) * __real__ valorz;
     //       printf("ramo %ld--%ld corrente %lf corrente jusante %lf soma %lf valorZ %lf perda %lf \n", noM, noN, cabs(ibarra), cabs(ijus), cabs(ijus + ibarra), __real__ valorz, perda);
            perdas = perdas + perda;
           // printf("noM %ld noN %ld\t", noM, noN);
           // printf("perda %.2lf perdas %lf \n", perda/1000, perdas/1000);
            perdasReativas = ij * 3 * pow((cabs(ijus + ibarra)), 2) * __imag__ valorZ(ZParam, noM, noN);
            /*#########################################################
              ###########    Carregamento dos alimentadores (em potência)   ##########*/
            carregamentoAlimentador = carregamentoAlimentador + (configuracaoParam->dadosEletricos.vBarra[noN] * conj(configuracaoParam->dadosEletricos.corrente[noN]))*3;

            __real__ carregamentoAlimentador += perda;

            carregamentoAlimentador += perdasReativas;
            /*##########################################################*/
            barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;

        }
        noProf[configuracaoParam->rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam->rnp[indiceRNP].nos[indice].idNo;
    }
   // printf("carregamento %.5lf \n", carregamentoRede*100);
    configuracaoParam->rnp[indiceRNP].fitnessRNP.potenciaAlimentador = carregamentoAlimentador;
    configuracaoParam->rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede = carregamentoRede;
    configuracaoParam->rnp[indiceRNP].fitnessRNP.demandaAlimentador = cabs(carregamentoAlimentador);
    configuracaoParam->rnp[indiceRNP].fitnessRNP.perdasResistivas = perdas / 1000;
    configuracaoParam->rnp[indiceRNP].fitnessRNP.noMaiorCarregamentoRede = noCarregamentoRede; // Por Leandro: salva o no da RNP no qual há o maior carregamento
    configuracaoParam->rnp[indiceRNP].fitnessRNP.sobrecargaRede = sobrecargaRede; //Por Leandro: salva o valor EM AMPERES da sobrecarga no trecho de rede do alimentador com maior carregamento percentual

    free(noProf);
    free(barraProf);
}

/* @brief Por Leandro:
 * Descrição: esta função calcula a potência ativa não suprida para um indivíduo "idNovaConfiguracaoParam".
 * Ele determina o somatório de potência ativa de todas as cargas saudáveis e reconectáveis que foram cortadas,
 * e que, por isso, estão nas duas árvores temporárias criadas.
 *
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param matrizB
 * @return
 */
void calculaPotenciaAtivaNaoSuprida(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, RNPSETORES *matrizB){
	long int indice, noS, noR, noN, indiceBarra;
//	long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = malloc(200 * sizeof(long int));
	double potenciaAtivaNaoSuprida = 0;
	int idRnp;
	RNPSETOR rnpSetorSR;

    for (idRnp = 0; idRnp < configuracoesParam[idNovaConfiguracaoParam].numeroRNPFicticia; idRnp++) {
    	indice = 0;
        noProf[configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].profundidade] = configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].idNo;
        for (indice = 1; indice < configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].numeroNos; indice++) {
            noS = configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].idNo;

            /*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
             * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
             * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
            if(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].profundidade == 1) //Se isto é verdade, significa que o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
            	noR = matrizB[noS].rnps[0].idSetorOrigem;
            else
            	noR = noProf[configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].profundidade - 1];

            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
                noN = rnpSetorSR.nos[indiceBarra].idNo;
                potenciaAtivaNaoSuprida = potenciaAtivaNaoSuprida + creal(configuracoesParam[0].dadosEletricos.potencia[noN]);
            }
            noProf[configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].profundidade] = configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].idNo;
        }
    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTotalNaoSuprida = potenciaAtivaNaoSuprida;
    free(noProf);
}

/* @brief Por Leandro:
 * Descrição: esta função calcula a potência ativa não suprida para um indivíduo "idNovaConfiguracaoParam".
 * Ele determina o somatório de potência ativa de todas as cargas saudáveis e reconectáveis que foram cortadas,
 * e que, por isso, estão nas duas árvores temporárias criadas.
 *
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @param matrizB
 * @param grafoSDRParam
 * @param flagManobrasAlivioParam
 * @return
 */
void calculaPotenciaAtivaNaoSupridaPorNivelPrioridade(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		RNPSETORES *matrizB, GRAFO *grafoSDRParam, BOOL flagManobrasAlivioParam){
	long int indice, noS, noR, noN, indiceBarra, idRank, idRnpSalvaEmIdRank;
	double potenciaAtivaTotalNaoSuprida = 0;
	NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida;
	NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSupridaPorRNP;
	NIVEISDEPRIORIDADEATENDIMENTO contadorBarras;
	contadorBarras.consumidoresSemPrioridade = 0;
	contadorBarras.consumidoresPrioridadeBaixa = 0;
	contadorBarras.consumidoresPrioridadeIntermediaria = 0;
	contadorBarras.consumidoresPrioridadeAlta = 0;


	int idRnp;
	RNPSETOR rnpSetorSR;

	if(flagManobrasAlivioParam == true){ //Se "idNovaConfiguracaoParam" foi gerada a partir de manobras para alívio de carga, então não houve atualização dos valores de PNS
		potenciaAtivaTotalNaoSuprida =                  		 configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaTotalNaoSuprida;
		potenciaNaoSuprida.consumidoresPrioridadeAlta = 		 configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta;
		potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria;
		potenciaNaoSuprida.consumidoresPrioridadeBaixa =         configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa;
		potenciaNaoSuprida.consumidoresSemPrioridade = 			 configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade;
//		//Apenas copia o ranqueamento
//		for (idRnp = 0; idRnp < configuracoesParam[idConfiguracaoSelecionadaParam].numeroRNPFicticia; idRnp++)
//			configuracoesParam[idNovaConfiguracaoParam].ranqueamentoRnpsFicticias[idRnp] = configuracoesParam[idConfiguracaoSelecionadaParam].ranqueamentoRnpsFicticias[idRnp];
	}
	else{
		potenciaNaoSuprida.consumidoresPrioridadeAlta = 0;
		potenciaNaoSuprida.consumidoresPrioridadeBaixa = 0;
		potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = 0;
		potenciaNaoSuprida.consumidoresSemPrioridade = 0;
		potenciaAtivaTotalNaoSuprida = 0;

		for (idRnp = 0; idRnp < configuracoesParam[idNovaConfiguracaoParam].numeroRNPFicticia; idRnp++) {
			potenciaNaoSupridaPorRNP.consumidoresPrioridadeAlta = 0;
			potenciaNaoSupridaPorRNP.consumidoresPrioridadeBaixa = 0;
			potenciaNaoSupridaPorRNP.consumidoresPrioridadeIntermediaria = 0;
			potenciaNaoSupridaPorRNP.consumidoresSemPrioridade = 0;

			for (indice = 1; indice < configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].numeroNos; indice++) {
				noS = configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].nos[indice].idNo;
				noR = matrizB[noS].rnps[0].idSetorOrigem; //Pega um nós adjacente qualquer, uma vez que este será usado apenas para buscar o índice das barras presentes no nó "noS
				rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);

				for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) { //Percorre somente as barras em "rnpSetorSR" que pertencem ao nó "noS"
					noN = rnpSetorSR.nos[indiceBarra].idNo;
					potenciaAtivaTotalNaoSuprida = potenciaAtivaTotalNaoSuprida + grafoSDRParam[noN].valorPQ.p;

					switch(grafoSDRParam[noN].priorizacoes.prioridadeConsumidor){
					case semPrioridade:
						potenciaNaoSupridaPorRNP.consumidoresSemPrioridade = potenciaNaoSupridaPorRNP.consumidoresSemPrioridade + grafoSDRParam[noN].valorPQ.p;
						contadorBarras.consumidoresSemPrioridade++;
						break;

					case prioridadeBaixa:
						potenciaNaoSupridaPorRNP.consumidoresPrioridadeBaixa = potenciaNaoSupridaPorRNP.consumidoresPrioridadeBaixa + grafoSDRParam[noN].valorPQ.p;
						contadorBarras.consumidoresPrioridadeBaixa++;
						break;

					case prioridadeIntermediaria:
						potenciaNaoSupridaPorRNP.consumidoresPrioridadeIntermediaria = potenciaNaoSupridaPorRNP.consumidoresPrioridadeIntermediaria + grafoSDRParam[noN].valorPQ.p;
						contadorBarras.consumidoresPrioridadeIntermediaria++;
						break;

					case prioridadeAlta:
						potenciaNaoSupridaPorRNP.consumidoresPrioridadeAlta = potenciaNaoSupridaPorRNP.consumidoresPrioridadeAlta + grafoSDRParam[noN].valorPQ.p;
						contadorBarras.consumidoresPrioridadeAlta++;
						break;
					}
				}
			}
			//Salva a potência não suprida para o alimentador em questão
			configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresSemPrioridade           = potenciaNaoSupridaPorRNP.consumidoresSemPrioridade;
			configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa         = potenciaNaoSupridaPorRNP.consumidoresPrioridadeBaixa;
			configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = potenciaNaoSupridaPorRNP.consumidoresPrioridadeIntermediaria;
			configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta          = potenciaNaoSupridaPorRNP.consumidoresPrioridadeAlta;

			//Soma a potência não suprida no alimentador em questão ao montante acumulado de energia não suprida
			potenciaNaoSuprida.consumidoresSemPrioridade           = potenciaNaoSuprida.consumidoresSemPrioridade           + potenciaNaoSupridaPorRNP.consumidoresSemPrioridade;
			potenciaNaoSuprida.consumidoresPrioridadeBaixa         = potenciaNaoSuprida.consumidoresPrioridadeBaixa         + potenciaNaoSupridaPorRNP.consumidoresPrioridadeBaixa;
			potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = potenciaNaoSuprida.consumidoresPrioridadeIntermediaria + potenciaNaoSupridaPorRNP.consumidoresPrioridadeIntermediaria;
			potenciaNaoSuprida.consumidoresPrioridadeAlta          = potenciaNaoSuprida.consumidoresPrioridadeAlta          + potenciaNaoSupridaPorRNP.consumidoresPrioridadeAlta;

//			/* Atualiza o ranqueamento de RNPs Fictícias feito em ordem decrescente do valor de energia não suprida considerando níveis de prioridade de atendimento,
//			 * necessário à execução do Operador LRO*/
//			if(idRnp == 0)
//				configuracoesParam[idNovaConfiguracaoParam].ranqueamentoRnpsFicticias[0] = 0;
//			else{
//				idRank = idRnp - 1;
//				idRnpSalvaEmIdRank = configuracoesParam[idNovaConfiguracaoParam].ranqueamentoRnpsFicticias[idRank];
//				while((idRank >= 0) &&
//						(
//						(floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) >  floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) ||
//						((floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) >  floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
//						((floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa) >  floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa))) ||
//						((floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnp].fitnessRNP.potenciaNaoSuprida.consumidoresSemPrioridade) > floor(configuracoesParam[idNovaConfiguracaoParam].rnpFicticia[idRnpSalvaEmIdRank].fitnessRNP.potenciaNaoSuprida.consumidoresSemPrioridade)))
//						)
//						){
//					configuracoesParam[idNovaConfiguracaoParam].ranqueamentoRnpsFicticias[idRank + 1] = configuracoesParam[idNovaConfiguracaoParam].ranqueamentoRnpsFicticias[idRank];
//					idRank--;
//				}
//				configuracoesParam[idNovaConfiguracaoParam].ranqueamentoRnpsFicticias[idRank + 1] = idRnp;
//			}
		}
	}

	//Salva os valores determinados
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTotalNaoSuprida =                                potenciaAtivaTotalNaoSuprida;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade           = potenciaNaoSuprida.consumidoresSemPrioridade;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa         = potenciaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = potenciaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta          = potenciaNaoSuprida.consumidoresPrioridadeAlta;
}

/* @brief Por Leandro:
 * Descrição: esta função calcula a energia ativa não suprida para um indivíduo "idNovaConfiguracaoParam", por nível de prioridade de atendimento.
 *
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param idConfiguracaoSelecionadaParam
 * @return
 */
//TODO Testar o funcionamento desta função
void calculaEnergiaAtivaNaoSupridaPorNivelPrioridade(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam, long int idConfiguracaoSelecionadaParam,
		SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, LISTACHAVES *listaChavesParam, VETORPI *vetorPiParam, BOOL flagManobrasAlivioParam) {
	double energiaAtivaTotalNaoSuprida = 0;
	double tempoOperacaoSegundaManobra =0;
	long int idChaveFechada, idChaveAberta;
	int s=0;
	NIVEISDEPRIORIDADEATENDIMENTO energiaNaoSuprida;

	energiaNaoSuprida.consumidoresPrioridadeAlta = 0;
	energiaNaoSuprida.consumidoresPrioridadeBaixa = 0;
	energiaNaoSuprida.consumidoresPrioridadeIntermediaria = 0;
	energiaNaoSuprida.consumidoresSemPrioridade = 0;

	/*
	 //Modo anterior de cálcular. P.S.: Ele não considera a influência da sequência de chaveamento escolhida para as manobras em pares
		energiaNaoSuprida.consumidoresSemPrioridade = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
				- (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade*(tempoDeRestauracaoDaFalha - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
				+ (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
				+ (configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo));

		energiaNaoSuprida.consumidoresPrioridadeBaixa = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
					- (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa*(tempoDeRestauracaoDaFalha - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
					+ (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
					+ (configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo));

		energiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
					- (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria*(tempoDeRestauracaoDaFalha - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
					+ (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
					+ (configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo));

		energiaNaoSuprida.consumidoresPrioridadeAlta = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
				- (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta*(tempoDeRestauracaoDaFalha - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
				+ (configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.tempo))
				+ (configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo));
	*/

	//Determinação de alguns parâmentros necessários ao cálculo
	//1º. a sequência de operação das manobras em Pares
	//2º. O tempo de operação da segunda manobra realizada
	s = 0;
	tempoOperacaoSegundaManobra = 0;
	if(flagManobrasAlivioParam){
		switch(sequenciaManobrasAlivioParam){
			case abreFecha:
				s = 1;
				idChaveFechada = vetorPiParam[idNovaConfiguracaoParam].idChaveFechada[vetorPiParam[idNovaConfiguracaoParam].numeroManobras-1];
				if(listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica)
					tempoOperacaoSegundaManobra = tempoOperacaoChaveAutomatica;
				else
					tempoOperacaoSegundaManobra = tempoOperacaoChaveManual;
				break;

			case fechaAbre:
				s = 0;
				idChaveAberta = vetorPiParam[idNovaConfiguracaoParam].idChaveAberta[vetorPiParam[idNovaConfiguracaoParam].numeroManobras-1];
				if(listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica)
					tempoOperacaoSegundaManobra = tempoOperacaoChaveAutomatica;
				else
					tempoOperacaoSegundaManobra = tempoOperacaoChaveManual;
				break;
		}
	}

	energiaNaoSuprida.consumidoresSemPrioridade = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
			- ((configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade - configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade)*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo))
			+ (s*(configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade)*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - tempoOperacaoSegundaManobra));


	energiaNaoSuprida.consumidoresPrioridadeBaixa = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
			- ((configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa - configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa)*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo))
			+ (s*(configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa)*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - tempoOperacaoSegundaManobra));


	energiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
			- ((configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria - configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria)*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo))
			+ (s*(configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria)*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - tempoOperacaoSegundaManobra));

	energiaNaoSuprida.consumidoresPrioridadeAlta = configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
			- ((configuracoesParam[idConfiguracaoSelecionadaParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta - configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta)*(tempoDeRestauracaoDaFalha - configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo))
			+ (s*(configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta)*(configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo - tempoOperacaoSegundaManobra));


	energiaAtivaTotalNaoSuprida = energiaNaoSuprida.consumidoresSemPrioridade + energiaNaoSuprida.consumidoresPrioridadeBaixa + energiaNaoSuprida.consumidoresPrioridadeIntermediaria + energiaNaoSuprida.consumidoresPrioridadeAlta;

    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade           = energiaNaoSuprida.consumidoresSemPrioridade;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa         = energiaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta          = energiaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaTotalNaoSuprida                                = energiaAtivaTotalNaoSuprida;
}



/**
 * Cálcula a carga em um trecho do alimentador.
 * @param configuracaoParam
 * @param indiceRNP
 * @param indiceSetorRaiz
 * @param indiceSetorInicial
 * @param indiceSetorFinal
 * @param matrizB
 * @return 
 */
double cargaTrecho(CONFIGURACAO configuracaoParam, int indiceRNP, long int indiceSetorRaiz, 
        long int indiceSetorInicial, long int indiceSetorFinal, RNPSETORES *matrizB) {
    int indice, indice1;
    long int noS, noR, noN, noM;
    RNPSETOR rnpSetorSR;
    __complex__ double cargaComplexa = 0 + ij * 0;
    __complex__ double valorz;

    //fprintf(arquivog, "Alimentador %d \n", indiceRNP+1);
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int barraProf[100]; //armazena o ultimo nó do setor presente em uma profundidade, é indexado pela profundidade
    indice = indiceSetorRaiz;
    noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
    for (indice = indiceSetorInicial; indice <= indiceSetorFinal; indice++) {
        noS = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
        noR = noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        indice1 = 0;
        barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;
        //printf("nos %ld nor %ld\n", noS, noR);
        for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
            noM = barraProf[rnpSetorSR.nos[indice1].profundidade - 1];
            noN = rnpSetorSR.nos[indice1].idNo;
            //printf("noM %ld noN %ld\n", noM, noN);
            /*#########################################################
             * 	#####################      Soma das Cargas     #### ##############*/
            cargaComplexa = cargaComplexa + (configuracaoParam.dadosEletricos.vBarra[noN] * conj(configuracaoParam.dadosEletricos.corrente[noN]))*3;

            barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;

        }
        noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
    }
    return cabs(cargaComplexa)/1000;
}
/**
 * Calcula o fluxo de carga de todos os alimentadores.
 * @param numeroBarrasParam
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param configuracoesParam
 * @param indiceConfiguracao
 * @param matrizB
 * @param ZParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param maximoCorrenteParam
 */
void fluxoCargaTodosAlimentadores(long int numeroBarrasParam,
        DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam) {

    int indiceRNP;
    __complex__ double VF;
//    char nomeArquivo[200];
//    sprintf(nomeArquivo, "dadosCorrente%ld.dad", indiceConfiguracao);
//    arquivog = fopen(nomeArquivo, "w");
    for (indiceRNP = 0; indiceRNP < configuracoesParam[indiceConfiguracao].numeroRNP; indiceRNP++) {
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, indiceRNP, indiceConfiguracao, matrizB,
                ZParam, maximoCorrenteParam, indiceRegulador, dadosRegulador,1);
    }
//    fclose(arquivog);
}
/**
 * Calcula o fluxo de carga de cada alimentador no modelo de varredura direta inversa.
 * @param numeroBarrasParam
 * @param configuracoesParam
 * @param VFParam
 * @param indiceRNP
 * @param indiceConfiguracao
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param maximoIteracoes
 */
void fluxoCargaAlimentador(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, int maximoIteracoes) {
    long int indice, noS, noR, noN, indiceBarra;
    int iteracoes = 0, carga;
    double erroV, erroA, tol, difV, difA;
    __complex__ float *V_barra_ant;
    long int pior; //armazena a barra com pior perfil de tensão de cada alimentador
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = (long int*) malloc(400 * sizeof(long int));
    RNPSETOR rnpSetorSR;
    V_barra_ant = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    tol = 0.001;

    numeroExecucoesFluxoCarga++;  /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/

    carga = 0; //0 --> corrente constante, 1--> potencia constante
    //  printf("\nALIMENTADOR %d \n", indiceRNP+1);
    do // varredura backward/forward
    {
        //   printf("----------------iteracao %d -----------------\n", iteracoes);
        indice = 0;
        noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracao].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);

            for (indiceBarra = 0; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
                noN = rnpSetorSR.nos[indiceBarra].idNo;
                if (iteracoes == 0) {
                    configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN] = VFParam;
                    configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[noN] = conj(configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[noN] / configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN]);
                }
                V_barra_ant[noN] = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN];
            }
            noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        }
        correnteJusante(indiceRNP, carga, iteracoes,
                configuracoesParam[indiceConfiguracao], matrizB, indiceRegulador, dadosRegulador);
        // printf("indiceRNP %d\n", indiceRNP);
        pior = tensaoQuedaTensaoModificada(indiceRNP, configuracoesParam[indiceConfiguracao], VFParam, matrizB, ZParam, indiceRegulador, dadosRegulador); // etapa forward - tensões
        configuracoesParam[indiceConfiguracao].rnp[indiceRNP].fitnessRNP.piorBarra = pior;

        //  printf("quedaMaxima %.2lf pior %d menorTensao %.2lf \n", quedaMaxima, pior,menorTensao);
        erroV = 0;
        erroA = 0;
        indice = 0;
        noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;

        for (indice = 1; indice < configuracoesParam[indiceConfiguracao].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indiceBarra = 0; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
                noN = rnpSetorSR.nos[indiceBarra].idNo;
                difV = cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN]) - cabs(V_barra_ant[noN]); //calcula a diferença de módulos das tensões entre duas iterações
                if (difV < 0)
                    difV = -1 * difV; //faz mÃ³dulo
                if (difV > erroV) //encontra o maior desvio
                    erroV = difV;
                difA = carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN]) - carg(V_barra_ant[noN]); //calcula a diferença de ângulos das tensões entre duas iterações
                if (difA < 0)
                    difA = -1 * difA; //faz módulo
                if (difA > erroA) //encontra o maior desvio
                    erroA = difA;
            }
            noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        }

        iteracoes++;
    } while ((erroV > tol || erroA > tol || flagMudancaTap) && iteracoes < maximoIteracoes);

    carregamentoPerdasModificada(&configuracoesParam[indiceConfiguracao], indiceRNP, matrizB, ZParam, maximoCorrenteParam);

    free(V_barra_ant);
    free(noProf);
}

/*
void fluxoCargaAnelAlimentador(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador) {

    long int indice, noS, noR, noN, indiceBarra;
    int iteracoes = 0, carga;

    double erroV, erroA, tol, difV, difA;
    __complex__ float *V_barra_ant;
    long int pior; //armazena a barra com pior perfil de tensão de cada alimentador
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    RNPSETOR rnpSetorSR;
    V_barra_ant = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    tol = 0.001;


    carga = 0; //0 --> corrente constante, 1--> potencia constante
    //  printf("\nALIMENTADOR %d \n", indiceRNP+1);
    do // varredura backward/forward
    {
        //   printf("----------------iteracao %d -----------------\n", iteracoes);
        indice = 0;
        noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracao].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);

            for (indiceBarra = 0; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
                noN = rnpSetorSR.nos[indiceBarra].idNo;
                if (iteracoes == 0) {
                    configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN] = VFParam;
                }
                V_barra_ant[noN] = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN];
            }
            noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        }
        correnteJusante(indiceRNP, carga, iteracoes,
                configuracoesParam[indiceConfiguracao], matrizB, indiceRegulador, dadosRegulador);
        // printf("indiceRNP %d\n", indiceRNP);
        pior = tensaoQuedaTensaoModificada(indiceRNP, configuracoesParam[indiceConfiguracao], VFParam, matrizB, ZParam, indiceRegulador, dadosRegulador); // etapa forward - tensões
        configuracoesParam[indiceConfiguracao].rnp[indiceRNP].fitnessRNP.piorBarra = pior;

        //  printf("quedaMaxima %.2lf pior %d menorTensao %.2lf \n", quedaMaxima, pior,menorTensao);
        erroV = 0;
        erroA = 0;
        indice = 0;
        noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;

        for (indice = 1; indice < configuracoesParam[indiceConfiguracao].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indiceBarra = 0; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
                noN = rnpSetorSR.nos[indiceBarra].idNo;
                difV = cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN]) - cabs(V_barra_ant[noN]); //calcula a diferença de módulos das tensões entre duas iterações
                if (difV < 0)
                    difV = -1 * difV; //faz mÃ³dulo
                if (difV > erroV) //encontra o maior desvio
                    erroV = difV;
                difA = carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[noN]) - carg(V_barra_ant[noN]); //calcula a diferença de ângulos das tensões entre duas iterações
                if (difA < 0)
                    difA = -1 * difA; //faz módulo
                if (difA > erroA) //encontra o maior desvio
                    erroA = difA;
            }
            noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        }

        iteracoes++;
    } while ((erroV > tol || erroA > tol || flagMudancaTap) && iteracoes < 50);

    carregamentoPerdasModificada(&configuracoesParam[indiceConfiguracao], indiceRNP, matrizB, ZParam, maximoCorrenteParam);

    free(V_barra_ant);
}
*/

/**
 * Calcula o carregamento de trafo com base na carga de cada alimentador ligado a ele.
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param dadosAlimentadorParam
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param idAntigaConfiguracaoParam
 * @param todosAlimentadores
 * @param idRNPOrigem
 * @param idRNPDestino
 */
void carregamentoTrafo(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam,
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino) {
    int indiceI, indiceJ;
    int idTrafo;
    double potencia;
    double carregamentoTrafo;
    carregamentoTrafo = 0;
    //realiza o cálculo para todos os alimentadores
    if (todosAlimentadores) {
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = 0 + ij * 0;
        }
        //printf("numeroAlimentadores %d \n",numeroAlimentadoresParam);
        for (indiceJ = 1; indiceJ <= numeroAlimentadoresParam; indiceJ++) {
            idTrafo = dadosAlimentadorParam[indiceJ].idTrafo;
            //a sequencia de rnps obedece a mesma sequencia de alimentadores com a diferença de uma posição
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo] += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceJ - 1].fitnessRNP.potenciaAlimentador;
        }
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {
            potencia = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI]) / (dadosTrafoParam[indiceI].capacidade * 10000); //(10000=1MVA/100) em porcentagem
            if (carregamentoTrafo < potencia)
                carregamentoTrafo = potencia;
            //
            // printf("capacidade %.2lf potencia do trafo  %lf carregamento %.2lf \n", dadosTrafoParam[indiceI].capacidade, cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI])/1000000, potencia);
        }

        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = carregamentoTrafo;

    } else //atualiza os valores considerando apenas os alimentadores que tiveram modificação
    {
        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;//configuracoesParam[idAntigaConfiguracaoParam].objetivo.maiorCarregamentoTrafo;
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {

            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = configuracoesParam[idAntigaConfiguracaoParam].objetivo.potenciaTrafo[indiceI];
            //verifica se o alimentador da RNP do alimentador de origem pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
            if (dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPOrigem + 1].idTrafo) {
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
            }
            //verifica se o alimentador da RNP do alimentador de destino pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
            if (dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPDestino + 1].idTrafo) {
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
            }
            //calcula a potencia que está sendo utilizada em porcentagem da capacidade
            potencia = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI]) / (dadosTrafoParam[indiceI].capacidade * 10000); //10000=1MVA*100%
            //obtem o maior carregamento de trafo da rede em porcentagem
            if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo < potencia)
                configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = potencia;
        }
    }
}


/* Por Leandro
 * Descrição: esta função consiste na função "carregamentoTrafo()" modificada para
 * 1) que as Árvores Origem ('rnpP') e Destino ('rnpA') sejam consideradas somente quando
 * rnpP e rnpA, respectivamente, forem maiores ou iguais a zero.
 * Isto permitirá que, quando forem executados os operadores LSO e LRO, o carregamento de trafo seja
 * calculado desconsiderando-se a árvore fictícia.
 * 2) salvar o identificador do transformador com maior carregamento
 *
 * Ademis, assim como a função "carregamentoTrafo()" , calcula o carregamento de trafo com base na carga de cada alimentador ligado a ele.
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param dadosAlimentadorParam
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param idAntigaConfiguracaoParam
 * @param todosAlimentadores
 * @param idRNPOrigem
 * @param idRNPDestino
 */
void carregamentoTrafoModificado(DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, int idNovaConfiguracaoParam,
        int idAntigaConfiguracaoParam, BOOL todosAlimentadores, int idRNPOrigem, int idRNPDestino) {
    int indiceI, indiceJ;
    int idTrafo;
    double potencia;
    double carregamentoTrafo;
    long int idTrafoMaiorCarregamento = 0; // por Leandro: armazenará o identificador do trafo com maior carregamento

    carregamentoTrafo = 0;
    //realiza o cálculo para todos os alimentadores
    if (todosAlimentadores) {
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = 0 + ij * 0;
        }
        for (indiceJ = 1; indiceJ <= numeroAlimentadoresParam; indiceJ++) {
            idTrafo = dadosAlimentadorParam[indiceJ].idTrafo;
            //a sequencia de rnps obedece a mesma sequencia de alimentadores com a diferença de uma posição
            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[idTrafo] += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceJ - 1].fitnessRNP.potenciaAlimentador;
        }
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {
            potencia = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI]) / (dadosTrafoParam[indiceI].capacidade * 10000); //(10000=1MVA/100) em porcentagem
            if (carregamentoTrafo < potencia){
                carregamentoTrafo = potencia;
                idTrafoMaiorCarregamento = dadosTrafoParam[indiceI].idTrafo; // por Leandro: armazena o identificador do trafo com maior carregamento
            }
            //
            // printf("capacidade %.2lf potencia do trafo  %lf carregamento %.2lf \n", dadosTrafoParam[indiceI].capacidade, cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI])/1000000, potencia);
        }

        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = carregamentoTrafo;
        configuracoesParam[idNovaConfiguracaoParam].objetivo.idTrafoMaiorCarregamento = idTrafoMaiorCarregamento; // por Leandro: armazena o identificador do trafo com maior carregamento


    } else //atualiza os valores considerando apenas os alimentadores que tiveram modificação
    {
        configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;//configuracoesParam[idAntigaConfiguracaoParam].objetivo.maiorCarregamentoTrafo;
        for (indiceI = 1; indiceI <= numeroTrafosParam; indiceI++) {

            configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] = configuracoesParam[idAntigaConfiguracaoParam].objetivo.potenciaTrafo[indiceI];
            //verifica se o alimentador da RNP do alimentador de origem pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
            if (idRNPOrigem >= 0 && dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPOrigem + 1].idTrafo) { // Por Leandro: foi modificado aqui
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPOrigem].fitnessRNP.potenciaAlimentador;
            }
            //verifica se o alimentador da RNP do alimentador de destino pertence a esse trafo. O indice do alimentador é o indice do vetor da rnp acrescido de 1.
            if (idRNPDestino >= 0 &&  dadosTrafoParam[indiceI].idTrafo == dadosAlimentadorParam[idRNPDestino + 1].idTrafo) { // Por Leandro: foi modificado aqui
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] -= configuracoesParam[idAntigaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI] += configuracoesParam[idNovaConfiguracaoParam].rnp[idRNPDestino].fitnessRNP.potenciaAlimentador;
            }
            //calcula a potencia que está sendo utilizada em porcentagem da capacidade
            potencia = cabs(configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo[indiceI]) / (dadosTrafoParam[indiceI].capacidade * 10000); //10000=1MVA*100%
            //obtem o maior carregamento de trafo da rede em porcentagem
            if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo < potencia){
                configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = potencia;
                configuracoesParam[idNovaConfiguracaoParam].objetivo.idTrafoMaiorCarregamento = dadosTrafoParam[indiceI].idTrafo; // por Leandro: armazenará o identificador do trafo com maior carregamento
            }
        }
    }
}

/**
 * Calcula o valor da função ponderação dos objetivos carregamento de Rede, carregamento de trafo e queda de tensão
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param VFParam
 */
void calculaPonderacao(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, __complex__ double VFParam) {
    int a, b, c;
    long int configuracaoInicial = 0;
    double Fit = 0.0;
    a = b = c = 1; // Por Leandro. Estava "100" e alterei para "1", conforme observei nos Estudos realizados
    if (configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede <= maxCarregamentoRede)
        a = 0;
    if (configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima <= maxQuedaTensao) /*12420 é 10% de queda de tensao*/
        b = 0;
    if (configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo <= maxCarregamentoTrafo)
        c = 0;
    Fit = configuracoesParam[idConfiguracaoParam].objetivo.perdasResistivas/configuracoesParam[configuracaoInicial].objetivo.perdasResistivas
            + a * configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede /configuracoesParam[configuracaoInicial].objetivo.maiorCarregamentoRede
            + b * ((__real__ VFParam - configuracoesParam[idConfiguracaoParam].objetivo.menorTensao) / __real__ VFParam) / ((__real__ VFParam - configuracoesParam[configuracaoInicial].objetivo.menorTensao) / __real__ VFParam)
            + c * configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo / configuracoesParam[configuracaoInicial].objetivo.maiorCarregamentoTrafo;
    configuracoesParam[idConfiguracaoParam].objetivo.ponderacao = Fit;
}

/**
 * Realiza a avaliação das configurações geradas pelo algoritmo.
 * @param todosAlimentadores
 * @param configuracoesParam
 * @param rnpA
 * @param rnpP
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param dadosAlimentadorParam
 * @param idAntigaConfiguracaoParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param copiarDadosEletricos
 */
void avaliaConfiguracao(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam,
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam,
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos) {
    long int contador;
    double quedaMaxima, menorTensao, VF, perdasTotais;
    perdasTotais = 0;
    //verifica se deve manter os dados elétricos para cada configuração gerada individualmente. Para isso é necessário copiar os dados elétricos de uma configuração para outra
    if (idNovaConfiguracaoParam != idAntigaConfiguracaoParam) {
        if (copiarDadosEletricos) {
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            for (contador = 1; contador <= numeroBarrasParam; contador++) {
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra[contador];
            }
        } else { //não mantém os dados elétricos individualmente, somente copia os ponteiros para os vetores com dados elétricos
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra;
        }
    }



    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;
    if (todosAlimentadores) //calcula os valores de fitness para todos os alimentadores
    {
        fluxoCargaTodosAlimentadores(numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam, configuracoesParam, idNovaConfiguracaoParam, matrizB, ZParam, indiceRegulador, dadosRegulador, maximoCorrenteParam);
    } else { //calcula o fluxo de carga somente para os alimentadores que foram alterados.
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpP,
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
                indiceRegulador, dadosRegulador, 1);
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpA,
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
                indiceRegulador, dadosRegulador,1);
    }
    int indiceRNP;
    for (indiceRNP = 0; indiceRNP < configuracoesParam[idNovaConfiguracaoParam].numeroRNP; indiceRNP++) {
        //  printf("alimentador %d carregamento %lf\n",indiceRNP+1,configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede*100);
        quedaMaxima = 100000;
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNP + 1].idTrafo].tensaoReal / sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.menorTensao;
        if (quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador;
        //maior carregamento de rede
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede;
        //total perdas
        //if (configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas)
        //  configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;
        perdasTotais += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;

    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
    carregamentoTrafo(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);
    calculaPonderacao(configuracoesParam, idNovaConfiguracaoParam, VF);
}


/**
 * Por Leandro:
 * Consiste na função "avaliaConfiguracao()" modificada para:
 *
 * a) chamar as funções que determina a potência e a energia total não suprida por nível de prioridade de atendimento de consumidores
 * b) salvar o nó da RNP na qual ocorre a menor tensão
 * c) salvar o nó da RNP na qual ocorre o maior carregamento de rede
 * d) chamar a função "carregamentoTrafoModificado()" em vez da "carregamentoTrafo()", o que permitirá salvar
 * o identificador do trafo com o maior carregamento
 * e) para que seja passado como parâmentro a variável "grafoSDRParam", na qual estão salvas as informações do nível de prioridade de cada barra
 * f)para que seja passado como parametro a variável "sequenciaManobrasAlivioParam", do tipo "SEQUENCIAMANOBRASALIVIO", que informa a sequência na qual deseja-se
 * operar as manobras que ocorrem em para com a finalidade de trasferir cargas de um alimentador para outro.
 * g) para que seja passado como parametro a variável booleana "flagManobrasAlivioParam", a qual informa se a configuração sob avaliação foi ou não gerada pela
 * operação de um par de manobras. Isto é necessário para o cálculo da ENS.
 *
 *
 * Realiza a avaliação das configurações geradas pelo algoritmo.
 *
 * @param todosAlimentadores
 * @param configuracoesParam
 * @param rnpA
 * @param rnpP
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param dadosAlimentadorParam
 * @param idAntigaConfiguracaoParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param copiarDadosEletricos
 */
void avaliaConfiguracaoModificada(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, 
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, 
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam, 
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos,
		GRAFO *grafoSDRParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, LISTACHAVES *listaChavesParam, VETORPI *vetorPiParam, BOOL flagManobrasAlivioParam) {
    long int contador;
    double quedaMaxima, menorTensao, VF, perdasTotais;
    long int noMenorTensaoRNP, noMaiorCarregamentoRNP;

    perdasTotais = 0;
    //verifica se deve manter os dados elétricos para cada configuração gerada individualmente. Para isso é necessário copiar os dados elétricos de uma configuração para outra
    if (idNovaConfiguracaoParam != idAntigaConfiguracaoParam) {
        if (copiarDadosEletricos) {
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            for (contador = 1; contador <= numeroBarrasParam; contador++) {
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra[contador];
            }
        } else { //não mantém os dados elétricos individualmente, somente copia os ponteiros para os vetores com dados elétricos
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra;
        }
    }



    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;

    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTotalNaoSuprida = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta = 0; // por Leandro:

    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaTotalNaoSuprida = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = 0; // por Leandro:
    configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta = 0; // por Leandro:


    if (todosAlimentadores) //calcula os valores de fitness para todos os alimentadores
    {
        fluxoCargaTodosAlimentadores(numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam, configuracoesParam, idNovaConfiguracaoParam, matrizB, ZParam, indiceRegulador, dadosRegulador, maximoCorrenteParam);
    } else { //calcula o fluxo de carga somente para os alimentadores que foram alterados.
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpP, 
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam, 
                indiceRegulador, dadosRegulador, 1);
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpA, 
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam, 
                indiceRegulador, dadosRegulador,1);
    }
    int indiceRNP;
    for (indiceRNP = 0; indiceRNP < configuracoesParam[idNovaConfiguracaoParam].numeroRNP; indiceRNP++) {
        //  printf("alimentador %d carregamento %lf\n",indiceRNP+1,configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede*100);
        quedaMaxima = 100000;
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNP + 1].idTrafo].tensaoReal / sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.menorTensao;
        noMenorTensaoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.noMenorTensao;
        noMaiorCarregamentoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.noMaiorCarregamentoRede;

        if (quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMenorTensao = noMenorTensaoRNP;
        }
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador;
        //maior carregamento de rede
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMaiorCarregamentoRede = noMaiorCarregamentoRNP;
        }
        //total perdas
        //if (configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas)
        //  configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;
        perdasTotais += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;

    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
//    carregamentoTrafo(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);
    carregamentoTrafoModificado(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);
    calculaPonderacao(configuracoesParam, idNovaConfiguracaoParam, VF);
    calculaPotenciaAtivaNaoSupridaPorNivelPrioridade(configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, matrizB, grafoSDRParam, flagManobrasAlivioParam);  // Por Leandro: Calcula a potência ativa não suprida na nova configuração gerada
    calculaEnergiaAtivaNaoSupridaPorNivelPrioridade(configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, flagManobrasAlivioParam);  // Por Leandro: Calcula a energia ativa não suprida na nova configuração gerada

}

/**
 * Leandro:
 * consite na função "avaliaConfiguracao()" modificada para:
 * 1) salvar o nó da RNP na qual ocorre a menor tensão e
 * 2) salvar o nó da RNP na qual ocorre o maior carregamento de rede
 * 3) chamar a função "carregamentoTrafoModificado()" em vez da "carregamentoTrafo()"
 *
 * Realiza a avaliação das configurações geradas pelo algoritmo.
 * @param todosAlimentadores
 * @param configuracoesParam
 * @param rnpA
 * @param rnpP
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param dadosAlimentadorParam
 * @param idAntigaConfiguracaoParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param copiarDadosEletricos
 */
/*void avaliaConfiguracaoModificada(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam,
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam,
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos) {
    long int contador;
    double quedaMaxima, menorTensao, VF, perdasTotais;
    long int noMenorTensaoRNP, noMaiorCarregamentoRNP;
    perdasTotais = 0;
    //verifica se deve manter os dados elétricos para cada configuração gerada individualmente. Para isso é necessário copiar os dados elétricos de uma configuração para outra
    if (idNovaConfiguracaoParam != idAntigaConfiguracaoParam) {
        if (copiarDadosEletricos) {
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            for (contador = 1; contador <= numeroBarrasParam; contador++) {
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra[contador];
            }
        } else { //não mantém os dados elétricos individualmente, somente copia os ponteiros para os vetores com dados elétricos
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra;
        }
    }



    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;
    if (todosAlimentadores) //calcula os valores de fitness para todos os alimentadores
    {
        fluxoCargaTodosAlimentadores(numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam, configuracoesParam, idNovaConfiguracaoParam, matrizB, ZParam, indiceRegulador, dadosRegulador, maximoCorrenteParam);
    } else { //calcula o fluxo de carga somente para os alimentadores que foram alterados.
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpP,
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
                indiceRegulador, dadosRegulador, 1);
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpA,
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam,
                indiceRegulador, dadosRegulador,1);
    }
    int indiceRNP;
    for (indiceRNP = 0; indiceRNP < configuracoesParam[idNovaConfiguracaoParam].numeroRNP; indiceRNP++) {
        //  printf("alimentador %d carregamento %lf\n",indiceRNP+1,configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede*100);
        quedaMaxima = 100000;
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNP + 1].idTrafo].tensaoReal / sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.menorTensao;
        noMenorTensaoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.noMenorTensao;
        noMaiorCarregamentoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.noMaiorCarregamentoRede;

        if (quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMenorTensao = noMenorTensaoRNP;
        }
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador;
        //máximo de corrente
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMaiorCarregamentoRede = noMaiorCarregamentoRNP;
        }
        //total perdas
        //if (configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas)
        //  configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;
        perdasTotais += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;

    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
    carregamentoTrafoModificado(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);
    calculaPonderacao(configuracoesParam, idNovaConfiguracaoParam, VF);
}*/


/**
 * Método para cálculo do fluxo de potência em anel. Nessa função é considerado o caso do fluxo de potência em anel onde os alimentadores estão ligados em trafos diferentes da mesma subestação.
 * O fluxo de potência em anel é cálculado pelo método do equivalente de Thévenin do fluxo de varredura direta inversa.
 * @param todosAlimentadores
 * @param configuracoesParam
 * @param rnpA
 * @param rnpP
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param dadosAlimentadorParam
 * @param idAntigaConfiguracaoParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param copiarDadosEletricos
 */
void avaliaConfiguracaoAnelCaso2(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam, 
        int rnpA, int rnpP, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, 
        int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, 
        DADOSREGULADOR *dadosRegulador, DADOSALIMENTADOR *dadosAlimentadorParam, 
        int idAntigaConfiguracaoParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, BOOL copiarDadosEletricos) {
    long int contador;
    double quedaMaxima, menorTensao, VF, perdasTotais;
    perdasTotais = 0;
    //verifica se deve manter os dados elétricos para cada configuração gerada individualmente. Para isso é necessário copiar os dados elétricos de uma configuração para outra
    if (idNovaConfiguracaoParam != idAntigaConfiguracaoParam) {
        if (copiarDadosEletricos) {
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
            for (contador = 1; contador <= numeroBarrasParam; contador++) {
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia[contador];
                configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra[contador] = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra[contador];
            }
        } else { //não mantém os dados elétricos individualmente, somente copia os ponteiros para os vetores com dados elétricos
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.iJusante;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.corrente = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.corrente;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.potencia;
            configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.vBarra = configuracoesParam[idAntigaConfiguracaoParam].dadosEletricos.vBarra;
        }
    }



    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.consumidoresSemFornecimento = configuracoesParam[idAntigaConfiguracaoParam].objetivo.consumidoresSemFornecimento;
    if (todosAlimentadores) //calcula os valores de fitness para todos os alimentadores
    {
        fluxoCargaTodosAlimentadores(numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam, configuracoesParam, idNovaConfiguracaoParam, matrizB, ZParam, indiceRegulador, dadosRegulador, maximoCorrenteParam);
    } else { //calcula o fluxo de carga somente para os alimentadores que foram alterados.
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpP, 
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam, 
                indiceRegulador, dadosRegulador, 1);
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, rnpA, 
                idNovaConfiguracaoParam, matrizB, ZParam, maximoCorrenteParam, 
                indiceRegulador, dadosRegulador,1);
    }
    int indiceRNP;
    for (indiceRNP = 0; indiceRNP < configuracoesParam[idNovaConfiguracaoParam].numeroRNP; indiceRNP++) {
        //  printf("alimentador %d carregamento %lf\n",indiceRNP+1,configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede*100);
        quedaMaxima = 100000;
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNP + 1].idTrafo].tensaoReal / sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.menorTensao;
        if (quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.demandaAlimentador;
        //máximo de corrente
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede;
        //total perdas
        //if (configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas)
        //  configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;
        perdasTotais += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;

    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
}


/**
 * Constroi o array com os índices de reguladores.
 * @param indicesParam
 * @param dadosReguladorParam
 * @param numeroReguladoresParam
 * @param grafoSDRParam
 * @param numeroBarrasParam
 */

void constroiVetorIndiceReguladores(int *indicesParam, DADOSREGULADOR *dadosReguladorParam, long int numeroReguladoresParam, GRAFO *grafoSDRParam, long int numeroBarrasParam) {
    long int indiceNo1, indiceNo2, indiceRegulador;
    char idRegulador[30];
    for (indiceNo1 = 1; indiceNo1 <= numeroBarrasParam; indiceNo1++) {
        for (indiceNo2 = 0; indiceNo2 < grafoSDRParam[indiceNo1].numeroAdjacentes; indiceNo2++) {
            if (grafoSDRParam[indiceNo1].adjacentes[indiceNo2].tipoAresta == reguladorTensao) {
                //existe regulador de tensão entre as barras
                //localiza o regulador de tensão
                strcpy(idRegulador, grafoSDRParam[indiceNo1].adjacentes[indiceNo2].idAresta);
                for (indiceRegulador = 0; indiceRegulador < numeroReguladoresParam; indiceRegulador++) {
                    if (strcmp(idRegulador, dadosReguladorParam[indiceRegulador].idRegulador) == 0) {
                        //printf("regulador %s barra1 %ld barra2 %ld\n", idRegulador, grafoSDRParam[indiceNo1].idNo, grafoSDRParam[indiceNo1].adjacentes[indiceNo2].idNo);
                        indicesParam[grafoSDRParam[indiceNo1].idNo] = indiceRegulador;
                        indicesParam[grafoSDRParam[indiceNo1].adjacentes[indiceNo2].idNo] = indiceRegulador;
                    }
                }
            } else //não existe regulador de tensão entre as duas barras, insere valor 0 no taps.
            {
                indicesParam[grafoSDRParam[indiceNo1].idNo] = 0;
                indicesParam[grafoSDRParam[indiceNo1].adjacentes[indiceNo2].idNo] = 0;
            }
        }
    }
}

/**
 * Imprime os indicadores elétricos da configuração passada como parâmetro.
 * @param configuracaoParam
 */
void imprimeIndicadoresEletricos(CONFIGURACAO configuracaoParam) {
    // printf("Maior Carregamento Alimentador %.2lf \n", configuracaoParam.objetivo.maiorCarregamentoRede/1000);
    printf("Maior Carregamento Rede %.2lf%%\n", configuracaoParam.objetivo.maiorCarregamentoRede);
    printf("Maior Carregamento Trafo %.2lf%%\n", configuracaoParam.objetivo.maiorCarregamentoTrafo);
    printf("Perdas Resistivas %.2lfkW \n", configuracaoParam.objetivo.perdasResistivas);
    printf("Menor Tensao %.2lfV \n", configuracaoParam.objetivo.menorTensao * sqrt(3));
    printf("Queda tensão %.2lf%%\n", configuracaoParam.objetivo.quedaMaxima);
    printf("Consumidores %ld\n", configuracaoParam.objetivo.consumidoresSemFornecimento);
    printf("Consumidores Especiais %ld\n", configuracaoParam.objetivo.consumidoresEspeciaisSemFornecimento);
    printf("Man.Manuais %d\n", configuracaoParam.objetivo.manobrasManuais);
    printf("Man.Automaticas %d\n", configuracaoParam.objetivo.manobrasAutomaticas);
    //printf("Ponderacao %.2lf \n",configuracaoParam.objetivo.ponderacao);
}

/**
 * salva os dados elétricos de cada uma das configurações de todas as subpopulações.
 * @param nomeArquivo
 * @param nomeFinal
 * @param configuracoesParam
 * @param tabelas
 * @param numeroTabelas
 * @param vetorPiParam
 * @param listaChavesParam
 */
void salvaDadosArquivo(char *nomeArquivo, char *nomeFinal, CONFIGURACAO *configuracoesParam, VETORTABELA *tabelas, int numeroTabelas, VETORPI *vetorPiParam, LISTACHAVES *listaChavesParam) {
    FILE *arquivo;
    FILE *arquivoChaveamento;
    int contadorI, contadorT;
    char nomeChaveamento[200];
    long int identificadorConfiguracao;

    arquivo = fopen(nomeArquivo, "w");
    for (contadorT = 0; contadorT < numeroTabelas; contadorT++) {
        fprintf(arquivo, "-------------------Tabela: %d ---------------------\n", contadorT + 1);
        for (contadorI = 0; contadorI < tabelas[contadorT].numeroIndividuos; contadorI++) {
            identificadorConfiguracao = tabelas[contadorT].tabela[contadorI].idConfiguracao;
//            gravaIndividuo(nomeFinal, configuracoesParam[identificadorConfiguracao]);
            fprintf(arquivo, "Identificador Configuracao: %ld \n\n", identificadorConfiguracao);
            /*for (contadorRNP = 0; contadorRNP < configuracoesParam[identificadorConfiguracao].numeroRNP; contadorRNP++) {
                for (contadorB = 0; contadorB < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorB++) {
                    fprintf(arquivo, "%10ld ", configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorB].idNo);
                }
                fprintf(arquivo, "\n");
                for (contadorB = 0; contadorB < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorB++) {
                    fprintf(arquivo, "%10d ", configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorB].profundidade);
                }
                fprintf(arquivo, "\n\n");
            }*/
            //fprintf(arquivo, "Maior Carregamento Alimentador %.2lf \n", configuracoesParam[identificadorConfiguracao].objetivo.maiorCarregamentoRede/1000);
            fprintf(arquivo, "Maior Carregamento Rede %.2lf %%\n", configuracoesParam[identificadorConfiguracao].objetivo.maiorCarregamentoRede);
            fprintf(arquivo, "Maior Carregamento Trafo %.2lf %%\n", configuracoesParam[identificadorConfiguracao].objetivo.maiorCarregamentoTrafo);
            fprintf(arquivo, "Queda Maxima %.2lf %% \n", configuracoesParam[identificadorConfiguracao].objetivo.quedaMaxima);
            fprintf(arquivo, "Menor Tensao %.2lfV \n", configuracoesParam[identificadorConfiguracao].objetivo.menorTensao * sqrt(3));
            fprintf(arquivo, "Perdas Resistivas %.2lfkW \n", configuracoesParam[identificadorConfiguracao].objetivo.perdasResistivas);
            fprintf(arquivo, "Ponderacao %.2lf\n", configuracoesParam[identificadorConfiguracao].objetivo.ponderacao);
            fprintf(arquivo, "Manobras Manuais %d\n", configuracoesParam[identificadorConfiguracao].objetivo.manobrasManuais);
            fprintf(arquivo, "ManobrasAutomaticas %d\n", configuracoesParam[identificadorConfiguracao].objetivo.manobrasAutomaticas);
            fprintf(arquivo, "\n\n");
        }

    }
    //   fprintf(arquivo, "Melhor configuracao %ld \n", identificadorConfiguracao);
    fclose(arquivo);

}

/**
 * Salva em arquivo os dados elétricos da configuração.
 * @param nomeArquivo
 * @param configuracoesParam
 */
void gravaDadosEletricos(char *nomeArquivo, CONFIGURACAO configuracoesParam) {
    FILE *arquivo;

    arquivo = fopen(nomeArquivo, "w");
            fprintf(arquivo, "CARREGAMENTO MÁXIMO REDE (%%) %.2lf\n", configuracoesParam.objetivo.maiorCarregamentoRede);
            fprintf(arquivo, "CARREGAMENTO MÁXIMO SUBESTAÇÃO (%%) %.2lf\n", configuracoesParam.objetivo.maiorCarregamentoTrafo);
            fprintf(arquivo, "QUEDA DE TENSÃO MÁXIMA (%%) %.2lf\n", configuracoesParam.objetivo.quedaMaxima);
            fprintf(arquivo, "PERDAS (KW) %.2lf\n", configuracoesParam.objetivo.perdasResistivas);
    
    fclose(arquivo);

}

/**
 * Salva os dados elétricos de todas as barras da configuração.
 * @param configuracoesParam
 * @param identificadorConfiguracao
 * @param matrizB
 */
void salvaDadosTodosIndividuos(CONFIGURACAO *configuracoesParam, long int identificadorConfiguracao, RNPSETORES *matrizB) {
    FILE *arquivo;
    long int contadorRNP, contadorSetores, contadorBarras;
    long int noS, noR, noN;
    char nomeArquivo[100];
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    RNPSETOR rnpSetorSR;

    sprintf(nomeArquivo, "individuo_%ld_dados_eletricos.dad", identificadorConfiguracao);
    // printf("nome arquivo %s \n",nomeArquivo);
    arquivo = fopen(nomeArquivo, "w");

    //   fprintf(arquivo, "Identificador Configuracao: %ld \n\n", identificadorConfiguracao);
    for (contadorRNP = 0; contadorRNP < configuracoesParam[identificadorConfiguracao].numeroRNP; contadorRNP++) {
        for (contadorSetores = 0; contadorSetores < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorSetores++) {
            fprintf(arquivo, "%10ld ", configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo);
        }
        fprintf(arquivo, "\n");
        for (contadorSetores = 0; contadorSetores < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorSetores++) {
            fprintf(arquivo, "%10d ", configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade);
        }
        fprintf(arquivo, "\n");
        contadorSetores = 0;
        noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        for (contadorSetores = 1; contadorSetores < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorSetores++) {
            noS = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
            noR = noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (contadorBarras = 1; contadorBarras < rnpSetorSR.numeroNos; contadorBarras++) {
                noN = rnpSetorSR.nos[contadorBarras].idNo;
                fprintf(arquivo, "%10ld ", noN);
            }
            noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        }
        fprintf(arquivo, "\n \n");
        contadorSetores = 0;
        noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        for (contadorSetores = 1; contadorSetores < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorSetores++) {
            noS = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
            noR = noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (contadorBarras = 1; contadorBarras < rnpSetorSR.numeroNos; contadorBarras++) {
                noN = rnpSetorSR.nos[contadorBarras].idNo;
                fprintf(arquivo, "%.2lf ", cabs(configuracoesParam[identificadorConfiguracao].dadosEletricos.vBarra[noN]));
            }
            noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        }
        fprintf(arquivo, "\n \n");
        contadorSetores = 0;
        noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        for (contadorSetores = 1; contadorSetores < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorSetores++) {
            noS = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
            noR = noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (contadorBarras = 1; contadorBarras < rnpSetorSR.numeroNos; contadorBarras++) {
                noN = rnpSetorSR.nos[contadorBarras].idNo;
                fprintf(arquivo, "%.2lf %.2lf ", __real__ configuracoesParam[identificadorConfiguracao].dadosEletricos.corrente[noN], __imag__ configuracoesParam[identificadorConfiguracao].dadosEletricos.corrente[noN]);
            }
            noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        }

        fprintf(arquivo, "\n \n");
        contadorSetores = 0;
        noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        for (contadorSetores = 1; contadorSetores < configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].numeroNos; contadorSetores++) {
            noS = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
            noR = noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (contadorBarras = 1; contadorBarras < rnpSetorSR.numeroNos; contadorBarras++) {
                noN = rnpSetorSR.nos[contadorBarras].idNo;
                fprintf(arquivo, "%.2lf %.2lf ", __real__ configuracoesParam[identificadorConfiguracao].dadosEletricos.iJusante[noN], __imag__ configuracoesParam[identificadorConfiguracao].dadosEletricos.iJusante[noN]);
            }
            noProf[configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].profundidade] = configuracoesParam[identificadorConfiguracao].rnp[contadorRNP].nos[contadorSetores].idNo;
        }
        fprintf(arquivo, "\n\n");
    }
    //fprintf(arquivo, "Maior Carregamento Alimentador %.2lf \n", configuracoesParam[contadorI].objetivo.maiorCarregamentoRede/1000);
    fprintf(arquivo, "Maior Carregamento Rede %.2lf porcento \n", configuracoesParam[identificadorConfiguracao].objetivo.maiorCarregamentoRede);
    fprintf(arquivo, "Maior Carregamento Trafo %.2lf porcento\n", configuracoesParam[identificadorConfiguracao].objetivo.maiorCarregamentoTrafo);
    fprintf(arquivo, "Menor Tensao %.2lfV \n", configuracoesParam[identificadorConfiguracao].objetivo.menorTensao * sqrt(3));
    fprintf(arquivo, "Queda Maxima %.2lf porcento \n", configuracoesParam[identificadorConfiguracao].objetivo.quedaMaxima);
    fprintf(arquivo, "Perdas Resistivas %.2lf kW\n", configuracoesParam[identificadorConfiguracao].objetivo.perdasResistivas);
    fprintf(arquivo, "Ponderacao %.2lf \n", configuracoesParam[identificadorConfiguracao].objetivo.ponderacao);
    fprintf(arquivo, "Manobras Manuais %d \n", configuracoesParam[identificadorConfiguracao].objetivo.manobrasManuais);
    fprintf(arquivo, "ManobrasAutomaticas %d \n", configuracoesParam[identificadorConfiguracao].objetivo.manobrasAutomaticas);
    fprintf(arquivo, "\n\n");
    fclose(arquivo);

}
/*
void reconfigurador(GRAFO *grafoSDRParam, long int numeroBarras, DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos,
        DADOSALIMENTADOR *dadosAlimentadorSDRParam, long int numeroAlimentadores, DADOSREGULADOR *dadosReguladorSDR,
        long int numeroReguladores, RNPSETORES *rnpSetoresParam, long int numeroSetores, GRAFOSETORES *grafoSetoresParam,
        LISTACHAVES *listaChavesParam, long int numeroChaves, CONFIGURACAO *configuracaoInicial, MATRIZMAXCORRENTE *maximoCorrente,
        MATRIZCOMPLEXA *Z, ESTADOCHAVE *estadoInicial, ESTADOCHAVE *estadoAutomaticas, ESTADOCHAVE *estadoRestabelecimento) {
    VETORPI *vetorPi;
    MATRIZPI *matrizPI;
    CONFIGURACAO *configuracoes;
    VETORTABELA *populacao;
    long int *setorFalta;
    int numeroSetorFalta;
    int *indiceRegulador;
    BOOL copiaDados = false;
    BOOL todosAlimentadores = true;
    long int idConfiguracao = 0;
    long int idConfiguracaoInicial, idConfiguracaoBase;
    int SBase;
    int tamanhoTabela[14];
    int numeroTabelas = 14;
    long int contador;
    long int numeroChaveAberta = 0;
    long int numeroChaveFechada = 0;
    long int consumidoresSemFornecimento = 0;
    long int consumidoresEspeciaisSemFornecimento = 0;
    double taxaOperadorPAO = 0.5;
    long int maximoGeracoes;
    int numeroIndividuos;
    char nomeArquivo[120];
    char nomeArquivo2[120];
    char nomeFinal[100];
    int seed;
    long int melhorConfiguracao;
    clock_t inicio, fim;
    double tempo;
     FILE *arquivo;
     FILE *arquivo1; //Arquivo a ser utilizado para teste de falta simples em
                     //todos os setores do sistema
     sprintf(nomeArquivo2, "testeConfiguracaoInicial.dad");
     arquivo1 = fopen(nomeArquivo2, "a");

     leituraParametros(&maximoGeracoes, tamanhoTabela, &SBase, &numeroIndividuos, &seed);
     leituraDadosEntrada(&numeroSetorFalta, &setorFalta);
     //imprime no arquivo para teste o seed e o setor em falta analisado
     fprintf(arquivo1, "Seed;%d;Set.Falta;%ld;", seed, setorFalta[0]);
     fclose(arquivo1);

     sprintf(nomeFinal,"_seed_%d_geracoes_%ld_setorFalta_%ld.dad", seed, maximoGeracoes, setorFalta[0]);
     sprintf(nomeArquivo, "saidaReconfigurador%s",nomeFinal);
     srand(seed);
     indiceRegulador = Malloc(int, (numeroBarras + 1));
     inicio = clock();
     alocaTabelas(numeroTabelas, tamanhoTabela, &populacao);
     inicializaVetorPi(maximoGeracoes + numeroIndividuos + 3, &vetorPi);
     inicializaMatrizPI(grafoSetoresParam, &matrizPI, (maximoGeracoes + numeroIndividuos + 3), numeroSetores);
     configuracoes = alocaIndividuo(numeroAlimentadores, idConfiguracao, (maximoGeracoes + numeroIndividuos + 3));

      int contadorRnp, contadorNos;
     for (contadorRnp = 0; contadorRnp < configuracaoInicial[0].numeroRNP; contadorRnp++) {
         alocaRNP(configuracaoInicial[0].rnp[contadorRnp].numeroNos, &configuracoes[idConfiguracao].rnp[contadorRnp]);
         for (contadorNos = 0; contadorNos < configuracaoInicial[0].rnp[contadorRnp].numeroNos; contadorNos++) {
             configuracoes[idConfiguracao].rnp[contadorRnp].nos[contadorNos] = configuracaoInicial[0].rnp[contadorRnp].nos[contadorNos];
             adicionaColuna(matrizPI, configuracoes[idConfiguracao].rnp[contadorRnp].nos[contadorNos].idNo, idConfiguracao, contadorRnp, contadorNos);
         }

     }


     constroiVetorTaps(indiceRegulador, dadosReguladorSDR, numeroReguladores, grafoSDRParam, numeroBarras);

     // copiaIndividuoInicial(configuracoes, configuracaoInicial, idConfiguracao, matrizPI,vetorPi);
     //inicializaDadosEletricos(grafoSDRParam, configuracoes, idConfiguracao, numeroBarras, SBase, VF, modelo);
     inicializaDadosEletricosPorAlimentador(grafoSDRParam, configuracoes, idConfiguracao, numeroBarras, SBase, dadosTrafoSDRParam, dadosAlimentadorSDRParam, rnpSetoresParam);
     //imprimeDadosEletricos(configuraccoes, idConfiguracao, numeroBarras);
     avaliaConfiguracao(todosAlimentadores, configuracoes, -1, -1, idConfiguracao, dadosTrafoSDRParam, numeroTrafos, numeroAlimentadores, indiceRegulador, dadosReguladorSDR, dadosAlimentadorSDRParam, VF, idConfiguracao, rnpSetoresParam, Z, maximoCorrente, numeroBarras, copiaDados);
     //imprimeDadosEletricos(configuracoes, idConfiguracao, numeroBarras);
     imprimeIndicadoresEletricos(configuracoes[idConfiguracao]);
     gravaIndividuo(nomeFinal, configuracoes[idConfiguracao]);
     idConfiguracaoInicial = idConfiguracao;
     idConfiguracaoBase = idConfiguracao;
     idConfiguracao++;


     if(numeroSetorFalta > 1)
     {
         isolaRestabeleceMultiplasFaltas(grafoSetoresParam, idConfiguracao-1, configuracoes, matrizPI, vetorPi, numeroSetorFalta, setorFalta,
             estadoInicial, &numeroChaveAberta, &numeroChaveFechada, &idConfiguracao, &idConfiguracaoInicial,listaChavesParam);
     }
     else
         isolaRestabeleceTodasOpcoes(grafoSetoresParam, idConfiguracao-1, configuracoes, matrizPI, vetorPi, setorFalta[0],
             estadoInicial, &numeroChaveAberta, &numeroChaveFechada, &idConfiguracao, listaChavesParam);
     for(contador = idConfiguracaoInicial+1; contador<idConfiguracao; contador++)
     {
       // imprimeIndividuo(configuracoes[contador]);
         avaliaConfiguracao(todosAlimentadores, configuracoes, -1, -1, contador, dadosTrafoSDRParam,
                 numeroTrafos, numeroAlimentadores, indiceRegulador, dadosReguladorSDR, dadosAlimentadorSDRParam, VF,
                 idConfiguracaoBase, rnpSetoresParam, Z, maximoCorrente, numeroBarras, copiaDados);
         imprimeIndicadoresEletricos(configuracoes[contador]);
         gravaIndividuo(nomeFinal, configuracoes[contador]);
         insereTabelas(populacao, configuracoes, contador);
         //salvaDadosTodosIndividuos(configuracoes, contador ,rnpSetoresParam);
         char nomeArquivo[200];
         FILE *chaveamento;
         sprintf(nomeArquivo, "Chaveamento_individuo%ld%s", contador,nomeFinal);
         chaveamento = fopen(nomeArquivo, "w");
         sequenciaChaveamento(chaveamento, vetorPi, contador, listaChavesParam);
         fclose(chaveamento);
         printf("\n\n");
    }
     //Rotina para verificar qual das configurações obtidas pela busca exaustiva é
    //a melhor e gravar os dados no arquivo testeConfiguracaoInicial.dad
     melhorConfiguracao = melhorSolucao(configuracoes,populacao);
    // sprintf(nomeArquivo2, "testeConfiguracaoInicial.dad");
     arquivo1 = fopen(nomeArquivo2, "a");
     if(melhorConfiguracao < 0){
         melhorConfiguracao = melhorSolucaoNaoFactivel(configuracoes,populacao);
     }
     fprintf(arquivo1,"Cons.;%ld;ConsEsp.;%ld;", configuracoes[melhorConfiguracao].objetivo.consumidoresSemFornecimento, configuracoes[melhorConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento);
     fprintf(arquivo1,"Config.;%ld;", melhorConfiguracao);
     fprintf(arquivo1, "Car.Rede;%.2lf;", configuracoes[melhorConfiguracao].objetivo.maiorCarregamentoRede);
     fprintf(arquivo1, "Car.SE;%.2lf;", configuracoes[melhorConfiguracao].objetivo.maiorCarregamentoTrafo);
     fprintf(arquivo1, "Queda;%.2lf;", configuracoes[melhorConfiguracao].objetivo.quedaMaxima);
     fprintf(arquivo1, "Perdas;%.2lfkW;", configuracoes[melhorConfiguracao].objetivo.perdasResistivas);
     fprintf(arquivo1,"Man.Manuais;%d;",configuracoes[melhorConfiguracao].objetivo.manobrasManuais);
     fprintf(arquivo1,"Man.Automaticas;%d;",configuracoes[melhorConfiguracao].objetivo.manobrasAutomaticas);
     fprintf(arquivo1,"Porc.Manobras;%f\n",(float)(configuracoes[melhorConfiguracao].objetivo.manobrasAutomaticas*100)/configuracoes[melhorConfiguracao].objetivo.manobrasManuais);
     fclose(arquivo1);


 //    idConfiguracao++;

    processoEvolutivo(dadosTrafoSDRParam, dadosAlimentadorSDRParam, configuracoes, populacao, grafoSetoresParam,
             rnpSetoresParam, listaChavesParam, matrizPI, vetorPi, maximoCorrente, Z,
             VF, numeroBarras, numeroSetores, numeroTrafos, estadoInicial,
             maximoGeracoes, numeroTabelas, taxaOperadorPAO, idConfiguracao, indiceReguladorSDR, dadosRegulador
             &numeroChaveAberta, &numeroChaveFechada, numeroIndividuos);

    fim = clock();
    tempo = (double)(fim-inicio)/CLOCK_TICKS_PER_SECOND;
     salvaDadosArquivo(nomeArquivo, nomeFinal,configuracoes, populacao, numeroTabelas, vetorPi, listaChavesParam);
     //salvaDadosTodosIndividuos(configuracoes, (maximoGeracoes + numeroIndividuos),rnpSetoresParam);
     melhorConfiguracao = melhorSolucao(configuracoes,populacao);
     if(melhorConfiguracao < 0){
         melhorConfiguracao = melhorSolucaoNaoFactivel(configuracoes,populacao);
     }
     sprintf(nomeArquivo, "MelhorConfiguracao.dad");
     arquivo = fopen(nomeArquivo, "a");
     fprintf(arquivo,"Seed: %d\tGeracoes: %d\tSetor_falta %d\t",seed, maximoGeracoes, setorFalta[0]);
     fprintf(arquivo,"Tempo %.4lf\t", tempo);
     fprintf(arquivo,"Configuracao %ld\t", melhorConfiguracao);
     fprintf(arquivo, "Carregamento Rede %.2lf\t", configuracoes[melhorConfiguracao].objetivo.maiorCarregamentoRede);
     fprintf(arquivo, "Carregamento Trafo %.2lf\t", configuracoes[melhorConfiguracao].objetivo.maiorCarregamentoTrafo);
     fprintf(arquivo, "Queda Maxima %.2lf\t", configuracoes[melhorConfiguracao].objetivo.quedaMaxima);
     fprintf(arquivo, "Perdas Resistivas %.2lfkW\t", configuracoes[melhorConfiguracao].objetivo.perdasResistivas);
     fprintf(arquivo,"Manobras Manuais %d\t",configuracoes[melhorConfiguracao].objetivo.manobrasManuais);
     fprintf(arquivo,"Manobras Automaticas %d\t",configuracoes[melhorConfiguracao].objetivo.manobrasAutomaticas);
     fprintf(arquivo,"Porcentagem manobras %f\t",(float)(configuracoes[melhorConfiguracao].objetivo.manobrasAutomaticas*100)/configuracoes[melhorConfiguracao].objetivo.manobrasManuais);
     fprintf(arquivo,"Consumidores %ld \t Consumidores Especiais %ld\n",configuracoes[melhorConfiguracao].objetivo.consumidoresSemFornecimento,configuracoes[melhorConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento);

     fclose(arquivo);
     free(setorFalta);
     free(tapReguladores);
    //imprimeTabelas(numeroTabelas,populacao);
}
*/

/**
 * Inicializa os valores de todos os objetivos analisados para a configuração.
 * @param configuracao
 * @param numeroTrafosParam
 */
void inicializaObjetivos(CONFIGURACAO *configuracao, int numeroTrafosParam) {
    configuracao[0].objetivo.maiorDemandaAlimentador = 0.0;
    configuracao[0].objetivo.maiorCarregamentoRede = 0.0;
    configuracao[0].objetivo.maiorCarregamentoTrafo = 0;
    configuracao[0].objetivo.manobrasManuais = 0;
    configuracao[0].objetivo.manobrasAutomaticas = 0;
    configuracao[0].objetivo.manobrasAlivio = 0;
    configuracao[0].objetivo.manobrasRestabelecimento = 0;
    configuracao[0].objetivo.manobrasAposChaveamento = 0;
    configuracao[0].objetivo.contadorManobrasTipo.comCargaAutomatica = 0;
    configuracao[0].objetivo.contadorManobrasTipo.comCargaManual = 0;
    configuracao[0].objetivo.contadorManobrasTipo.curtoAutomatica = 0;
    configuracao[0].objetivo.contadorManobrasTipo.curtoManual = 0;
    configuracao[0].objetivo.contadorManobrasTipo.seca = 0;
    configuracao[0].objetivo.menorTensao = 0;
    //
    configuracao[0].objetivo.ponderacao = 0;
    configuracao[0].objetivo.quedaMaxima = 0;
    configuracao[0].objetivo.rank = 0;
    configuracao[0].objetivo.fronteira = 0;
    configuracao[0].objetivo.perdasResistivas = 0;
    configuracao[0].objetivo.consumidoresSemFornecimento= 0;
    configuracao[0].objetivo.consumidoresEspeciaisSemFornecimento = 0;
    configuracao[0].objetivo.consumidoresEspeciaisTransferidos= 0;
    configuracao[0].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;
    configuracao[0].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));

}


/**
 * Consiste na função "inicializaObjetivos()", que inicializa os valores de todos os objetivos analisados para a configuração,
 * modificada para a inicialização de novas variáveis.
 * @param configuracao
 * @param numeroTrafosParam
 */
void inicializaObjetivosModificada(CONFIGURACAO *configuracao, int numeroTrafosParam) {
    configuracao[0].objetivo.maiorDemandaAlimentador = 0.0;
    configuracao[0].objetivo.maiorCarregamentoRede = 0.0;
    configuracao[0].objetivo.maiorCarregamentoTrafo = 0;
    configuracao[0].objetivo.manobrasManuais = 0;
    configuracao[0].objetivo.manobrasAutomaticas = 0;
    configuracao[0].objetivo.manobrasRestabelecimento = 0;
	configuracao[0].objetivo.manobrasAlivio = 0;
	configuracao[0].objetivo.manobrasAposChaveamento = 0;
    configuracao[0].objetivo.contadorManobrasTipo.comCargaAutomatica = 0;
    configuracao[0].objetivo.contadorManobrasTipo.comCargaManual = 0;
    configuracao[0].objetivo.contadorManobrasTipo.curtoAutomatica = 0;
    configuracao[0].objetivo.contadorManobrasTipo.curtoManual = 0;
    configuracao[0].objetivo.contadorManobrasTipo.seca = 0;
    configuracao[0].objetivo.menorTensao = 0;

    configuracao[0].objetivo.ponderacao = 0;
    configuracao[0].objetivo.quedaMaxima = 0;
    configuracao[0].objetivo.rank = 0;
    configuracao[0].objetivo.fronteira = 0;
    configuracao[0].objetivo.perdasResistivas = 0;
    configuracao[0].objetivo.consumidoresSemFornecimento= 0;
    configuracao[0].objetivo.consumidoresEspeciaisSemFornecimento = 0;
    configuracao[0].objetivo.consumidoresEspeciaisTransferidos = 0;
	configuracao[0].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;
	configuracao[0].objetivo.potenciaTrafo = 0;
    configuracao[0].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracao[0].objetivo.noMaiorCarregamentoRede = 0;    // por Leandro:
    configuracao[0].objetivo.noMenorTensao = 0;              // por Leandro:
    configuracao[0].objetivo.idTrafoMaiorCarregamento = 0; // por Leandro:

    configuracao[0].objetivo.potenciaTotalNaoSuprida = 0; // por Leandro:
    configuracao[0].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade = 0; // por Leandro:
    configuracao[0].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa = 0; // por Leandro:
    configuracao[0].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = 0; // por Leandro:
    configuracao[0].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta = 0; // por Leandro:

    configuracao[0].objetivo.energiaTotalNaoSuprida = 0; // por Leandro:
    configuracao[0].objetivo.energiaNaoSuprida.consumidoresSemPrioridade = 0; // por Leandro:
    configuracao[0].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa = 0; // por Leandro:
    configuracao[0].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = 0; // por Leandro:
    configuracao[0].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta = 0; // por Leandro:
    configuracao[0].objetivo.tempo = 0; // por Leandro:
    configuracao[0].objetivo.tempoBruto = 0; // por Leandro:
    configuracao[0].objetivo.sobrecargaRede = 0; //Por Leandro
    configuracao[0].objetivo.sobrecargaTrafo = 0; //Por Leandro

}
/**
 * Libera a memória alocada para armazenar a matriz de impedâncias.
 * @param ZParam
 * @param numeroBarrasParam
 */
void desalocaMatrizZ(MATRIZCOMPLEXA *ZParam, long int numeroBarrasParam) {
    long int contador;
    for (contador = 1; contador <= numeroBarrasParam; contador++) {
        free(ZParam[contador].noAdjacentes);
    }
    free(ZParam);
}
/**
 * Desaloca a memória utilizada para armazenar os valores máximos de corrente.
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 */
void desalocaMatrizCorrente(MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam) {
    long int contador;
    for (contador = 1; contador <= numeroBarrasParam; contador++) {
        free(maximoCorrenteParam[contador].noAdjacentes);
    }
    free(maximoCorrenteParam);
}
/**
 * Método que verifica a factibilidade da configuração de acordo com os limites informados como parâmetro.
 * @param configuracaoParam
 * @param limiteCarregamentoRede
 * @param limiteCarregamentoTrafo
 * @param limiteQuedaTensao
 * @return 
 */
BOOL verificaFactibilidade(CONFIGURACAO configuracaoParam, int limiteCarregamentoRede, int limiteCarregamentoTrafo, int limiteQuedaTensao) {

    if (configuracaoParam.objetivo.maiorCarregamentoRede > limiteCarregamentoRede)
        return false;

    if (configuracaoParam.objetivo.maiorCarregamentoTrafo > limiteCarregamentoTrafo)
        return false;

    if (configuracaoParam.objetivo.quedaMaxima > limiteQuedaTensao)
        return false;

    return true;
}

/**
 * Por Leandro:
 * Método que verifica a factibilidade de um ALIMENTADOR da configuração de acordo com os limites informados como parâmetro.
 * Diz-se que o alimentador é factível se a máxima queda de tensão e o maior carregamento forem menores ou igual aos valores limites E
 * se carrregamento do transformador no qual está conectado for menor que o valor limite
 * @param configuracaoParam
 * @param limiteCarregamentoRede
 * @param limiteCarregamentoTrafo
 * @param limiteQuedaTensao
 * @return
 */
BOOL verificaFactibilidadeAlimentador(CONFIGURACAO configuracaoParam, long int indiceRNPParam, int limiteCarregamentoRede, int limiteCarregamentoTrafo, int limiteQuedaTensao, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam) {

	long int idTrafo = dadosAlimentadorParam[indiceRNPParam + 1].idTrafo; //Trafo no qual "indiceRNPParam" está conectada
    double carregamento = cabs(configuracaoParam.objetivo.potenciaTrafo[idTrafo]) / (dadosTrafoParam[idTrafo].capacidade * 10000); //Carregamento do trafo no qual "indiceRNPParam" está conectada
    double quedaTensao = ( ((1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNPParam + 1].idTrafo].tensaoReal / sqrt(3))  -  configuracaoParam.rnp[indiceRNPParam].fitnessRNP.menorTensao) / (1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNPParam + 1].idTrafo].tensaoReal / sqrt(3))  )*100;

    if (configuracaoParam.rnp[indiceRNPParam].fitnessRNP.maiorCarregamentoRede > limiteCarregamentoRede)
        return false;

    if (carregamento > limiteCarregamentoTrafo)
        return false;

    if (quedaTensao > limiteQuedaTensao)
        return false;

    return true;
}

/*long int melhorSolucao(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam) {
    int numManobras = 500;
    long int idConfiguracaoSelecionada = -1;
    long int idConfiguracao;
    int contador, contadorT;
    //analisa tabela 6 (somente manobras automáticas)
    for (contadorT = 5; contadorT < 11; contadorT++) {
        for (contador = 0; contador < tabelasParam[contadorT].numeroIndividuos; contador++) {
            idConfiguracao = tabelasParam[contadorT].tabela[contador].idConfiguracao;
            if (verificaFactibilidade(configuracoesParam[idConfiguracao])) {
                if (numManobras > (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                    numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                    idConfiguracaoSelecionada = idConfiguracao;
                } else {
                    if (numManobras == (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                        if (configuracoesParam[idConfiguracao].objetivo.perdasResistivas < configuracoesParam[idConfiguracaoSelecionada].objetivo.perdasResistivas) {
                            numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                            idConfiguracaoSelecionada = idConfiguracao;
                        }
                    }
                }
            }
        }
    }
    return idConfiguracaoSelecionada;
}*/

/**
 * Esta função retorna a configuração com o menor número de manobras que está dentro dos limites
 * de carregamento de rede, carregamento de trafo e queda de tensão. Os limites são alterados até que uma configuração seja encontrada.
 * Assim, o método garante que a melhor configuração dentro de um intervalo pode ser encontrado.
 * @param configuracoesParam
 * @param tabelasParam
 * @param numeroTabelas
 * @return 
 */
long int melhorSolucao(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam, int numeroTabelas) {
    int numManobras = 500;
    long int idConfiguracaoSelecionada = -1;
    long int idConfiguracao;
    int contador, contadorT;
    double maximoCarregamentoRedeAdmissivel = maxCarregamentoRede, maximoCarregamentoTrafoAdmissivel = maxCarregamentoTrafo, maximaQuedaTensaoAdmissivel = maxQuedaTensao;

    while (idConfiguracaoSelecionada < 0){ //&& maximoCarregamentoTrafoAdmissivel < 200) {
        for (contadorT = 0; contadorT < numeroTabelas; contadorT++) {
            for (contador = 0; contador < tabelasParam[contadorT].numeroIndividuos; contador++) {
                idConfiguracao = tabelasParam[contadorT].tabela[contador].idConfiguracao;
                if (verificaFactibilidade(configuracoesParam[idConfiguracao], maximoCarregamentoRedeAdmissivel, maximoCarregamentoTrafoAdmissivel, maximaQuedaTensaoAdmissivel)) {
                    if (numManobras > (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                        idConfiguracaoSelecionada = idConfiguracao;
                    } else {
                        if (numManobras == (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                            if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede < configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede) {
                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                idConfiguracaoSelecionada = idConfiguracao;
                            } else {
                                if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede) {
                                    if (configuracoesParam[idConfiguracao].objetivo.menorTensao > configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao) {
                                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                        idConfiguracaoSelecionada = idConfiguracao;
                                    } else {
                                        if (configuracoesParam[idConfiguracao].objetivo.menorTensao == configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao) {
                                            if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo < configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoTrafo) {
                                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                                idConfiguracaoSelecionada = idConfiguracao;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        maximoCarregamentoRedeAdmissivel += 1; //Leandro: Para admimitir uma solução com sobrecarga na rede, descomentar esta linha
        maximoCarregamentoTrafoAdmissivel += 5;
        maximaQuedaTensaoAdmissivel += 5;
    }
    return idConfiguracaoSelecionada;
}

///**
// * Por Leandro
// * Descrição: esta função
// * a) deve ser aplicada somente aos indivíduos obtidos pela busca heurística executada no início do programa, a qual restabelece todos os
// * setores possíveis de serem reconectados
// * b)
// *
// *  esta função seleciona o melhor dentre este indivíduos (ou soluções).
// * O melhor indivíduo (dentre aqueles obtidos pela referida heurística), ou solução, será aquele que possuir o menor nṕuu
// * Esta função deve ser aplicada para selecionar, dentre as configurações obtidas pela Heuristica que testa
// * retorna a configuração com o menor número de manobras que está dentro dos limites
// * de carregamento de rede, carregamento de trafo e queda de tensão. Os limites são alterados até que uma configuração seja encontrada.
// * Assim, o método garante que a melhor configuração dentro de um intervalo pode ser encontrado.
// * @param configuracoesParam
// * @param tabelasParam
// * @param numeroTabelas
// * @return
// */
//long int melhorDasSolucaoQueRestabelecemTodosSetoresPelaHeuristica(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam, int numeroTabelas) {
//    int numManobras = 500;
//    long int idConfiguracaoSelecionada = -1;
//    long int idConfiguracao;
//    int contador, contadorT, numeroTentativa;
//    double maximoCarregamentoRedeAdmissivel = maxCarregamentoRede, maximoCarregamentoTrafoAdmissivel = maxCarregamentoTrafo, maximaQuedaTensaoAdmissivel = maxQuedaTensao;
//
//    numeroTentativa = 0;
//    while (idConfiguracaoSelecionada < 0 && numeroTentativa < 1){ //&& maximoCarregamentoTrafoAdmissivel < 200) {
//        for (contadorT = 0; contadorT < numeroTabelas; contadorT++) {
//            for (contador = 0; contador < tabelasParam[contadorT].numeroIndividuos; contador++) {
//                idConfiguracao = tabelasParam[contadorT].tabela[contador].idConfiguracao;
//                if (verificaFactibilidade(configuracoesParam[idConfiguracao], maximoCarregamentoRedeAdmissivel, maximoCarregamentoTrafoAdmissivel, maximaQuedaTensaoAdmissivel)) {
//                    if (numManobras > (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
//                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
//                        idConfiguracaoSelecionada = idConfiguracao;
//                    } else {
//                        if (numManobras == (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
//                            if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede < configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede) {
//                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
//                                idConfiguracaoSelecionada = idConfiguracao;
//                            } else {
//                                if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede) {
//                                    if (configuracoesParam[idConfiguracao].objetivo.menorTensao > configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao) {
//                                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
//                                        idConfiguracaoSelecionada = idConfiguracao;
//                                    } else {
//                                        if (configuracoesParam[idConfiguracao].objetivo.menorTensao == configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao) {
//                                            if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo < configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoTrafo) {
//                                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
//                                                idConfiguracaoSelecionada = idConfiguracao;
//                                            }
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        numeroTentativa++;
//    }
//    return idConfiguracaoSelecionada;
//}

/**
 * Por Leandro
 * Descrição: esta função
 * a) deve ser aplicada somente aos indivíduos obtidos pela busca heurística executada no início do programa, a qual restabelece todos os
 * setores possíveis de serem reconectados
 *
 * Esta função considera que todos os indivíduos obtidos pela busca heurística (ou busca exaustiva), possuem o mesmo número total de manobras.
 * Portanto, define como sendo a melhor soluções, dentre as soluções obtidas pela busca exaustiva, aquele com o maior número de manobras
 * em chaves controladas remotamente dentre as soluções factíveis.
 *
 * Caso nenhuma das soluções seja factível, então retorna esta informação.

 * @param configuracoesParam
 * @param idConfiguracao1Param é o indice do primeiro indivíduo obtido pela busca heurística
 * @param idConfiguracaoParam define o limite superior para os valores de idConfiguracao que serão verificados
 * @return -1, se nenhuma solução for factível, ou o identificador da solução como o maior número de manobras em CCRs.
 */
long int melhorDasSolucaoQueRestabelecemTodosSetoresPelaHeuristica(CONFIGURACAO *configuracoesParam, long int idConfiguracao1Param, long int idConfiguracaoParam) {
    int numManobras = -1, indice;
    long int idConfiguracaoSelecionada = -1;

    for(indice = idConfiguracao1Param; indice < idConfiguracaoParam; indice++){
    	if (verificaFactibilidade(configuracoesParam[indice], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
    		 if (configuracoesParam[indice].objetivo.manobrasAutomaticas > numManobras){
    			 numManobras = configuracoesParam[indice].objetivo.manobrasAutomaticas;
				 idConfiguracaoSelecionada = indice;
    		 }
    	}
    }

    return idConfiguracaoSelecionada;
}

long int melhorSolucaoNaoFactivel(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam, int numeroTabelas) {
    int numManobras = 500;
    long int idConfiguracaoSelecionada = -1;
    long int idConfiguracao;
    int contador, contadorT;
    for (contadorT = 0; contadorT < numeroTabelas; contadorT++) {
        for (contador = 0; contador < tabelasParam[contadorT].numeroIndividuos; contador++) {
            idConfiguracao = tabelasParam[contadorT].tabela[contador].idConfiguracao;
            if (numManobras > (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                idConfiguracaoSelecionada = idConfiguracao;
            } else {
                if (numManobras == (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                    if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede < configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorDemandaAlimentador) {
                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                        idConfiguracaoSelecionada = idConfiguracao;
                    } else {
                        if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede) {
                            if (configuracoesParam[idConfiguracao].objetivo.menorTensao > configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao) {
                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                idConfiguracaoSelecionada = idConfiguracao;
                            } else {
                                if (configuracoesParam[idConfiguracao].objetivo.menorTensao == configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao) {
                                    if (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo < configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoTrafo) {
                                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                        idConfiguracaoSelecionada = idConfiguracao;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return idConfiguracaoSelecionada;
}

/* Por Leandro:
 * Método desenvolvido para escolha de uma solução na fronteira de Pareto.
 * Dada uma fronteira de Pareto, por meio deste método, a solução final será aquela
 * que minimiza o valor do quadrado da diferença entre os valores normalizados de
 * dois objetivos.
 *
 *@param fronteiraParetoParam - fronteira da qual deseja-se extrair um indivíduo para ser solução
 *@return indice da configuração escolhida
 */
long int determinaSolucaoFinalMetodo1(FRONTEIRAS fronteiraParetoParam){
	int indice, indice2;
	double menorValorObjetivo1, menorValorObjetivo2, maiorValorObjetivo1, maiorValorObjetivo2, menorValorMetrica, metrica;
	FRONTEIRAS fronteiraParetoNormalizada;

	//PARTE 1 - NORMALIZAÇÃO DOS VALORES DOS OBJETIVOS 1 E 2
	//Determinação dos valores máximos e mínimos para os objetivos
	indice=0;
	menorValorObjetivo1 = fronteiraParetoParam.individuos[indice].valorObjetivo1;
	menorValorObjetivo2 = fronteiraParetoParam.individuos[indice].valorObjetivo2;
	maiorValorObjetivo1 = fronteiraParetoParam.individuos[indice].valorObjetivo1;
	maiorValorObjetivo2 = fronteiraParetoParam.individuos[indice].valorObjetivo2;
	for(indice = 1; indice < fronteiraParetoParam.numeroIndividuos; indice++){
		//Determina o menor valor para Objetivo 1
		if(fronteiraParetoParam.individuos[indice].valorObjetivo1 < menorValorObjetivo1)
			menorValorObjetivo1 = fronteiraParetoParam.individuos[indice].valorObjetivo1;

		//Determina o menor valor para Objetivo 2
		if(fronteiraParetoParam.individuos[indice].valorObjetivo2 < menorValorObjetivo2)
			menorValorObjetivo2 = fronteiraParetoParam.individuos[indice].valorObjetivo2;

		//Determina o maior valor para Objetivo 1
		if(fronteiraParetoParam.individuos[indice].valorObjetivo1 > maiorValorObjetivo1)
			maiorValorObjetivo1 = fronteiraParetoParam.individuos[indice].valorObjetivo1;

		//Determina o maior valor para Objetivo 2
		if(fronteiraParetoParam.individuos[indice].valorObjetivo2 > maiorValorObjetivo2)
			maiorValorObjetivo2 = fronteiraParetoParam.individuos[indice].valorObjetivo2;
	}

	//Normalização dos valores de objetivo 1 e 2
	fronteiraParetoNormalizada.individuos = Malloc(INDIVIDUOSNAODOMINADOS, fronteiraParetoParam.numeroIndividuos);
	for(indice = 0; indice < fronteiraParetoParam.numeroIndividuos; indice++){
		fronteiraParetoNormalizada.individuos[indice].valorObjetivo1 = (fronteiraParetoParam.individuos[indice].valorObjetivo1 - menorValorObjetivo1) / (maiorValorObjetivo1 - menorValorObjetivo1);
		fronteiraParetoNormalizada.individuos[indice].valorObjetivo2 = (fronteiraParetoParam.individuos[indice].valorObjetivo2 - menorValorObjetivo2) / (maiorValorObjetivo2 - menorValorObjetivo2);
	}
	//fronteiraParetoNormalizada.numeroIndividuos = fronteiraParetoParam.numeroIndividuos;

	//PARTE 2 - DETERMINAÇÃO DA INDIVÍDUO QUE MINIMIZA O QUADRADO DA DIFERENÇA DOS VALORES NORMALIZADOS DOS OBJETIVOS 1 E 2
	indice = 0;
	indice2 = indice;
	menorValorMetrica = pow((fronteiraParetoNormalizada.individuos[indice2].valorObjetivo1 - fronteiraParetoNormalizada.individuos[indice2].valorObjetivo2), 2);
	for(indice = 1; indice < fronteiraParetoParam.numeroIndividuos; indice++){
		metrica = pow((fronteiraParetoNormalizada.individuos[indice].valorObjetivo1 - fronteiraParetoNormalizada.individuos[indice].valorObjetivo2),2);
		if(metrica < menorValorMetrica){
			indice2 = indice;
			menorValorMetrica = metrica;
		}
	}


	return fronteiraParetoParam.individuos[indice2].idConfiguracao;
}

/* Por Leandro:
 * Método desenvolvido para escolha de uma solução na fronteira de Pareto.
 * Dada uma fronteira de Pareto, por meio deste método, a solução final será aquela
 * que minimiza a energia TOTAL não suprida (sem considerar os vários níveis de prioridade).
 *
 *@param fronteiraParetoParam - fronteira da qual deseja-se extrair um indivíduo para ser solução
 *@return indice da configuração escolhida
 */
long int determinaSolucaoFinalMetodo2(FRONTEIRAS fronteiraParetoParam, CONFIGURACAO *configuracoesParam){
	int indice, indiceMenorValorEnergiaTOTALNaoSuprida;
	long int idConfiguracao;
	double menorValorEnergiaTOTALNaoSuprida;

	indice=0;
	idConfiguracao = fronteiraParetoParam.individuos[indice].idConfiguracao;
	menorValorEnergiaTOTALNaoSuprida = configuracoesParam[idConfiguracao].objetivo.energiaTotalNaoSuprida;
	indiceMenorValorEnergiaTOTALNaoSuprida = indice; //Índice em "fronteiraParetoParam"

	for(indice = 1; indice < fronteiraParetoParam.numeroIndividuos; indice++){
		idConfiguracao = fronteiraParetoParam.individuos[indice].idConfiguracao;
		if(configuracoesParam[idConfiguracao].objetivo.energiaTotalNaoSuprida < menorValorEnergiaTOTALNaoSuprida){
			menorValorEnergiaTOTALNaoSuprida = configuracoesParam[idConfiguracao].objetivo.energiaTotalNaoSuprida;
			indiceMenorValorEnergiaTOTALNaoSuprida = indice;
		}
	}

	return fronteiraParetoParam.individuos[indiceMenorValorEnergiaTOTALNaoSuprida].idConfiguracao;
}

/* Por Leandro:
 * Método desenvolvido para escolha de uma solução na fronteira de Pareto.
 * Dada uma fronteira de Pareto, por meio deste método, a solução final será aquela
 * que minimiza a energia não suprida, CONSIDERANDO-SE os vários níveis de prioridade dos consumidores.
 *
 *@param fronteiraParetoParam - fronteira da qual deseja-se extrair um indivíduo para ser solução
 *@return indice da configuração escolhida
 */
long int determinaSolucaoFinalMetodo3(FRONTEIRAS fronteiraParetoParam, CONFIGURACAO *configuracoesParam){
	int indice, indiceMenorValorEnergiaNaoSuprida;
	long int idConfiguracao;
	NIVEISDEPRIORIDADEATENDIMENTO menorEnergiaNaoSuprida;

	indice=0;
	idConfiguracao = fronteiraParetoParam.individuos[indice].idConfiguracao;
	menorEnergiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
	menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
	menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
	menorEnergiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;
	indiceMenorValorEnergiaNaoSuprida = indice; //Índice em "fronteiraParetoParam"

	for(indice = 1; indice < fronteiraParetoParam.numeroIndividuos; indice++){
		idConfiguracao = fronteiraParetoParam.individuos[indice].idConfiguracao;
		if( (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)  < floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)  < floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)  < floor(menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa)) ) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)  < floor(menorEnergiaNaoSuprida.consumidoresSemPrioridade))) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) == floor(menorEnergiaNaoSuprida.consumidoresSemPrioridade)) && (idConfiguracao < fronteiraParetoParam.individuos[indiceMenorValorEnergiaNaoSuprida].idConfiguracao) ) //A tendencia é a de que indivíduos que forma gerados primeiro, tenham menos chaves repetidas
		){
			menorEnergiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
			menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
			menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
			menorEnergiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;
			indiceMenorValorEnergiaNaoSuprida = indice; //Índice em "fronteiraParetoParam"
		}
	}

	return fronteiraParetoParam.individuos[indiceMenorValorEnergiaNaoSuprida].idConfiguracao;
}

/*
 * Por Leandro:
 */
BOOL verificaSeJaFoiSalvo(TABELA *individuosParam, int numeroIndividuosSalvosParam, long int idConfiguracaoParam){
	int indice;
	BOOL jaEstaSalvo;

	jaEstaSalvo = false;
	indice = 0;
	while(indice < numeroIndividuosSalvosParam && jaEstaSalvo == false ){
		if(individuosParam[indice].idConfiguracao == idConfiguracaoParam)
			jaEstaSalvo = true;
		indice++;
	}

	return jaEstaSalvo;
}


/*
 * Por Leandro: esta função verifica se há na tabela "populacaoParam" algum indivíduo salvo cujo valor de Objetivo sejam igual
 * ao valor de Objetivo do indivíduo que está tentando entrar na tabela.
 *
 * Esta função é usada nas tabelas de subpopulação que utilizam o método de seleção do AEMT e não do NSGA-II.
 *
 */
BOOL verificaSeHaValorIgualJaSalvoTabela(VETORTABELA populacaoParam, double objetivoParam){
	int indice;
	BOOL jaEstaSalvo;

	jaEstaSalvo = false;
	indice = 0;
	while(indice < populacaoParam.numeroIndividuos && jaEstaSalvo == false ){
		if(floor(populacaoParam.tabela[indice].valorObjetivo) == floor(objetivoParam))
			jaEstaSalvo = true;
		indice++;
	}

	return jaEstaSalvo;
}

/*
 * Por Leandro: esta função verifica se há na tabela "populacaoParam" algum indivíduo salvo cujo valor de Objetivo sejam igual
 * ao valor de Objetivo do indivíduo que está tentando entrar na tabela.
 *
 * Esta função é usada nas 5 tabelas de subpopulação que minimizam e priorizam manobras.
 *
 */
BOOL verificaSeHaValorIgualJaSalvoTabelasManobras(VETORTABELA populacaoParam, long int idConfiguracaoParam, CONFIGURACAO *configuracoesParam){
	int indice;
	long int idConfiguracaoSalva;
	BOOL jaEstaSalvo;

	jaEstaSalvo = false;
	indice = 0;
	while(indice < populacaoParam.numeroIndividuos && jaEstaSalvo == false ){
		idConfiguracaoSalva = populacaoParam.tabela[indice].idConfiguracao;

		if(configuracoesParam[idConfiguracaoSalva].objetivo.manobrasManuais ==  configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais &&
		    configuracoesParam[idConfiguracaoSalva].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas &&
			floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) &&
			floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) &&
			floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)  &&
			floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)
		){
			jaEstaSalvo = true;
		}
		indice++;
	}

	return jaEstaSalvo;
}

/*
 * Por Leandro: esta função verifica se há na tabela "populacaoParam" algum indivíduo salvo cujo valor de Objetivo sejam igual
 * ao valor de Objetivo do indivíduo que está tentando entrar na tabela.
 *
 * Esta função é usada nas 5 tabelas de subpopulação que minimizam a Energia não Suprida considerando os níveis de prioridade.
 *
 */
BOOL verificaSeHaValorIgualJaSalvoTabelasEnergiaNaoSuprida(VETORTABELA populacaoParam, long int idConfiguracaoParam, CONFIGURACAO *configuracoesParam){
	int indice;
	long int idConfiguracaoSalva;
	BOOL jaEstaSalvo;

	jaEstaSalvo = false;
	indice = 0;
	while(indice < populacaoParam.numeroIndividuos && jaEstaSalvo == false ){
		idConfiguracaoSalva = populacaoParam.tabela[indice].idConfiguracao;

		if( (floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) &&
		    (floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) &&
			(floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) &&
			(floor(configuracoesParam[idConfiguracaoSalva].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) &&
			(configuracoesParam[idConfiguracaoSalva].objetivo.manobrasManuais ==  configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais) &&
			(configuracoesParam[idConfiguracaoSalva].objetivo.manobrasAutomaticas ==  configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
		){
			jaEstaSalvo = true;
		}
		indice++;
	}

	return jaEstaSalvo;
}


/*
 * Por Leandro: esta função verifica se há na tabela "populacaoParam" algum indivíduo salvo cujo valor de Objetivo sejam igual
 * ao valor de Objetivo do indivíduo que está tentando entrar na tabela.
 *
 * Esta função é usada na tabela de subpopulação que minimizam a potência transferida considerando os níveis de prioridade.
 *
 */
BOOL verificaSeHaValorIgualJaSalvoTabelaPotenciaTransferida(VETORTABELA populacaoParam, long int idConfiguracaoParam, CONFIGURACAO *configuracoesParam){
	int indice;
	long int idConfiguracaoSalva;
	BOOL jaEstaSalvo;

	jaEstaSalvo = false;
	indice = 0;
	while(indice < populacaoParam.numeroIndividuos && jaEstaSalvo == false ){
		idConfiguracaoSalva = populacaoParam.tabela[indice].idConfiguracao;

		if((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConfiguracaoSalva].potenciaTransferida.consumidoresPrioridadeAlta)) &&
		   (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConfiguracaoSalva].potenciaTransferida.consumidoresPrioridadeIntermediaria)) &&
		   (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idConfiguracaoSalva].potenciaTransferida.consumidoresPrioridadeBaixa)) &&
		   (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade) == floor(configuracoesParam[idConfiguracaoSalva].potenciaTransferida.consumidoresSemPrioridade))
		){
			jaEstaSalvo = true;
		}
		indice++;
	}

	return jaEstaSalvo;
}


/**
 * Por Leandro:
 * Esta função ordena os indivíduos FACTÍVEIS GERAL da população de acordo com o nível de dominância de cada um deles.
 * Em outras palavras, separa os indivíduos factíveis em fronteiras de Pareto.
 * IMPORTANTE: para tal, considera-se dois objetivos: minimização do número TOTAL de manobras e da Energia TOTAL não suprida
 *
 *@param fronteira salvas as informações das fronteiras
 *@param numeroDeFronteiras salva o número de fronteiras
 *@param configuracoesParam
 *@param tabelasParam
 *@param numeroTabelas
 *@return
 */
void classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras(FRONTEIRAS **fronteiraParam, int *numeroDeFronteirasParam,
		CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, int numeroTabelasParam) {
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    int contador, contadorT, indice, contadorI, tamanhoPopulacao, numeroIndividuosFactiveis, contadorJ, contadorIndividuosVerificados, rank, idFronteira, numIndividuos;
    TABELA *individuos;

    tamanhoPopulacao = 0;
    for(indice = 0; indice < numeroTabelasParam; indice++)
    	tamanhoPopulacao = tamanhoPopulacao + populacaoParam[indice].numeroIndividuos;
    //Salva em "individuos" todos os indivíduos FACTÍVEIS
    individuos = (TABELA *) Malloc(TABELA, tamanhoPopulacao);
    contadorI = 0;
    for (contadorT = 0; contadorT < numeroTabelasParam; contadorT++) {
        for (contador = 0; contador < populacaoParam[contadorT].numeroIndividuos; contador++) {
            idConfiguracao = populacaoParam[contadorT].tabela[contador].idConfiguracao;
            if (verificaFactibilidade(configuracoesParam[idConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
            	if(!verificaSeJaFoiSalvo(individuos, contadorI, idConfiguracao)){
					individuos[contadorI].idConfiguracao = idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
					contadorI++;
            	}
            }
        }
    }
    numeroIndividuosFactiveis = contadorI;

    (*fronteiraParam) =  (FRONTEIRAS *)Malloc(FRONTEIRAS, numeroIndividuosFactiveis);// Como não se sabe, a priori, em quantas fronteiras a população está dividida, considera-se o caso extremo em que cada indivíduo está numa fronteira diferente
    for(contadorI = 0; contadorI < numeroIndividuosFactiveis; contadorI++){
		(*fronteiraParam)[contadorI].numeroIndividuos = 0;
		(*fronteiraParam)[contadorI].nivelDominancia = 0;
		(*fronteiraParam)[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, numeroIndividuosFactiveis); // Como não se sabe, a priori, quantos indivíduos estarão em cada fronteira, considera-se o caso extremo em que todos os indivíduos estejam numa mesma fronteira
	}


    //Divide a população factível em fronteira segundo o critério de não-dominancia
    idFronteira = 0;
	contadorIndividuosVerificados = 0;
	while(contadorIndividuosVerificados < numeroIndividuosFactiveis){
		//Determina a qual fronteira cada indivíduo pertence
		for (contadorI = 0; contadorI < numeroIndividuosFactiveis; contadorI++) {
			idConfiguracao1 = individuos[contadorI].idConfiguracao;
			rank = 0;

			if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
				for (contadorJ = 0; contadorJ < numeroIndividuosFactiveis; contadorJ++) {
					idConfiguracao2 = individuos[contadorJ].idConfiguracao;
					// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
					if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
						if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
							if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
									&& (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) >= (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas)) {
								rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
								if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
										&& (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas)) {
									rank--;
								}
							}
						}
					}
				}
			}


			//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
			if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertence a fronteira "idFronteira"
				numIndividuos = (*fronteiraParam)[idFronteira].numeroIndividuos;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].valorObjetivo1 = configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].valorObjetivo2 = (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas);
				(*fronteiraParam)[idFronteira].nivelDominancia = idFronteira;
				(*fronteiraParam)[idFronteira].numeroIndividuos++;
				//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				contadorIndividuosVerificados++;
			}


		}
		for(contadorJ = 0; contadorJ < (*fronteiraParam)[idFronteira].numeroIndividuos; contadorJ++){
			idConfiguracao1 = (*fronteiraParam)[idFronteira].individuos[contadorJ].idConfiguracao;
			configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
		}

		idFronteira++;
	}
	numeroDeFronteirasParam[0] = idFronteira - 1;
	individuos = NULL; free(individuos);
}

/**
 * Por Leandro:
 * Esta função consiste na função "classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras()" modificada
 * para:
 * (a) ao verificar a factibilidade de um indivíduo em relação as restrições gerais, verificar também
 * as restrições relaxadas em todas as configurações pelas quais a rede passará até a obtenção do referido indivíduo.
 * Isto é feito porque, neste AEMO, soluções infactíveis em relação as restriçẽos ralaxadas podem ser salvas nas tabelas.
 * Mas, como um indivíduo só é factível se possuir uma sequência factível, então, nesta função passou-se a verificar também
 * as restrições nas configurações temporárias, o que, consiste, na verdade, em verificar a factíbilidade da sequência associadas
 * a um indivíduo.
 *
 * Assim, esta função verifica (i) a adequção dos indivíduos salvos nas tabelas de subpopulação em relação as restrições Gerais
 * e (ii) a factibilidade da sequência de chaveamento associada aos indivíduos. Em seguida, ordena os indivíduos FACTÍVEIS
 * de acordo com o nível de dominância de cada um deles.
 *
 * IMPORTANTE: para tal, considera-se dois objetivos: minimização do número TOTAL de manobras e da Energia TOTAL não suprida
 *
 *@param fronteira salvas as informações das fronteiras
 *@param numeroDeFronteiras salva o número de fronteiras
 *@param configuracoesParam
 *@param tabelasParam
 *@param numeroTabelas
 *@return
 */
void classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobrasV2(FRONTEIRAS **fronteiraParam, int *numeroDeFronteirasParam,
		CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, int numeroTabelasParam, VETORPI *vetorPiParam) {
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    int contador, contadorT, indice, contadorI, tamanhoPopulacao, numeroIndividuosFactiveis, contadorJ, contadorIndividuosVerificados, rank, idFronteira, numIndividuos;
    TABELA *individuos;

    tamanhoPopulacao = 0;
    for(indice = 0; indice < numeroTabelasParam; indice++)
    	tamanhoPopulacao = tamanhoPopulacao + populacaoParam[indice].numeroIndividuos;
    //Salva em "individuos" todos os indivíduos FACTÍVEIS
    individuos = (TABELA *) Malloc(TABELA, tamanhoPopulacao);
    contadorI = 0;
    for (contadorT = 0; contadorT < numeroTabelasParam; contadorT++) {
        for (contador = 0; contador < populacaoParam[contadorT].numeroIndividuos; contador++) {
            idConfiguracao = populacaoParam[contadorT].tabela[contador].idConfiguracao;
            if (verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracao) &&
            		verificaFactibilidade(configuracoesParam[idConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
            	if(!verificaSeJaFoiSalvo(individuos, contadorI, idConfiguracao)){
					individuos[contadorI].idConfiguracao = idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
					contadorI++;
            	}
            }
        }
    }
    numeroIndividuosFactiveis = contadorI;

    (*fronteiraParam) =  (FRONTEIRAS *)Malloc(FRONTEIRAS, numeroIndividuosFactiveis);// Como não se sabe, a priori, em quantas fronteiras a população está dividida, considera-se o caso extremo em que cada indivíduo está numa fronteira diferente
    for(contadorI = 0; contadorI < numeroIndividuosFactiveis; contadorI++){
		(*fronteiraParam)[contadorI].numeroIndividuos = 0;
		(*fronteiraParam)[contadorI].nivelDominancia = 0;
		(*fronteiraParam)[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, numeroIndividuosFactiveis); // Como não se sabe, a priori, quantos indivíduos estarão em cada fronteira, considera-se o caso extremo em que todos os indivíduos estejam numa mesma fronteira
	}


    //Divide a população factível em fronteira segundo o critério de não-dominancia
    idFronteira = 0;
	contadorIndividuosVerificados = 0;
	while(contadorIndividuosVerificados < numeroIndividuosFactiveis){
		//Determina a qual fronteira cada indivíduo pertence
		for (contadorI = 0; contadorI < numeroIndividuosFactiveis; contadorI++) {
			idConfiguracao1 = individuos[contadorI].idConfiguracao;
			rank = 0;

			if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
				for (contadorJ = 0; contadorJ < numeroIndividuosFactiveis; contadorJ++) {
					idConfiguracao2 = individuos[contadorJ].idConfiguracao;
					// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
					if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
						if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
							if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
									&& (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) >= (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas)) {
								rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
								if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
										&& (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas)) {
									rank--;
								}
							}
						}
					}
				}
			}


			//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
			if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertence a fronteira "idFronteira"
				numIndividuos = (*fronteiraParam)[idFronteira].numeroIndividuos;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].valorObjetivo1 = configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].valorObjetivo2 = (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas);
				(*fronteiraParam)[idFronteira].nivelDominancia = idFronteira;
				(*fronteiraParam)[idFronteira].numeroIndividuos++;
				//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				contadorIndividuosVerificados++;
			}


		}
		for(contadorJ = 0; contadorJ < (*fronteiraParam)[idFronteira].numeroIndividuos; contadorJ++){
			idConfiguracao1 = (*fronteiraParam)[idFronteira].individuos[contadorJ].idConfiguracao;
			configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
		}

		idFronteira++;
	}
	numeroDeFronteirasParam[0] = idFronteira - 1;
	individuos = NULL; free(individuos);
}

BOOL individuosIguaisEnergiaNaoSupridaTotalManobraTotal(CONFIGURACAO *configuracoesParam, long int idConfiguracao1, long int idConfiguracao2){
	BOOL iguais;
	int totalManobrasConfiguracao1, totalManobrasConfiguracao2;
	double totalENSConfiguracao1, totalENSConfiguracao2;

	totalManobrasConfiguracao1 = configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas;
	totalManobrasConfiguracao2 = configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas;
	totalENSConfiguracao1 = configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta + configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria + configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa + configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;
	totalENSConfiguracao2 = configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta + configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria + configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa + configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;

	iguais = false;
	if(totalManobrasConfiguracao1 == totalManobrasConfiguracao2){// && floor(totalENSConfiguracao1) == floor(totalENSConfiguracao2)){
		iguais = true;
	}
	return iguais;
}

/**
 * Por Leandro:
 * Esta função consiste na função "classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobras()" modificada
 * para:
 * (a) evitar que a configuração conceitualmente inicial do problema (aquela na qual os setores em falta estão isolados e os setores saudávis encontram-se
 * desligados) e outras configurações iguais a esta não sejam admitidas a fronteira de Pareo, uma vez que esta não restaura nenhum indivíduo;
 *
 * (b) ao verificar a factibilidade de um indivíduo em relação as restrições gerais, verificar também
 * as restrições relaxadas em todas as configurações pelas quais a rede passará até a obtenção do referido indivíduo.
 * Isto é feito porque, neste AEMO, soluções infactíveis em relação as restriçẽos ralaxadas podem ser salvas nas tabelas.
 * Mas, como um indivíduo só é factível se possuir uma sequência factível, então, nesta função passou-se a verificar também
 * as restrições nas configurações temporárias, o que, consiste, na verdade, em verificar a factíbilidade da sequência associadas
 * a um indivíduo.
 *
 * Assim, esta função verifica (i) a adequção dos indivíduos salvos nas tabelas de subpopulação em relação as restrições Gerais
 * e (ii) a factibilidade da sequência de chaveamento associada aos indivíduos. Em seguida, ordena os indivíduos FACTÍVEIS
 * de acordo com o nível de dominância de cada um deles.
 *
 * IMPORTANTE: para tal, considera-se dois objetivos: minimização do número TOTAL de manobras e da Energia TOTAL não suprida
 *
 *@param fronteira salvas as informações das fronteiras
 *@param numeroDeFronteiras salva o número de fronteiras
 *@param configuracoesParam
 *@param tabelasParam
 *@param numeroTabelas
 *@return
 */
void classificacaoPorNaoDominanciaEnergiaTotalNaoSupridaVsTotalManobrasV3(FRONTEIRAS **fronteiraParam, int *numeroDeFronteirasParam,
		CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, int numeroTabelasParam, VETORPI *vetorPiParam, long int idPrimeiraConfiguracaoParam) {
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    int contador, contadorT, indice, contadorI, tamanhoPopulacao, numeroIndividuosFactiveis, contadorJ, contadorIndividuosVerificados, rank, idFronteira, numIndividuos;
    TABELA *individuos;

    tamanhoPopulacao = 0;
    for(indice = 0; indice < numeroTabelasParam; indice++)
    	tamanhoPopulacao = tamanhoPopulacao + populacaoParam[indice].numeroIndividuos;
    //Salva em "individuos" todos os indivíduos FACTÍVEIS
    individuos = (TABELA *) Malloc(TABELA, tamanhoPopulacao);
    contadorI = 0;
    for (contadorT = 0; contadorT < numeroTabelasParam; contadorT++) {
        for (contador = 0; contador < populacaoParam[contadorT].numeroIndividuos; contador++) {
            idConfiguracao = populacaoParam[contadorT].tabela[contador].idConfiguracao;

            if(!individuosIguaisEnergiaNaoSupridaTotalManobraTotal(configuracoesParam, idConfiguracao, idPrimeiraConfiguracaoParam)){ //Verifica se "idConfiguracao" é igual a configuração conceitualmente inicial do problema

				if (verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracao) ) {
					if(!verificaSeJaFoiSalvo(individuos, contadorI, idConfiguracao)){
						individuos[contadorI].idConfiguracao = idConfiguracao;
						individuos[contadorI].valorObjetivo = 0;
						individuos[contadorI].distanciaMultidao = 0;
						configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
						contadorI++;
					}
				}

            }

        }
    }
    numeroIndividuosFactiveis = contadorI;

    (*fronteiraParam) =  (FRONTEIRAS *)Malloc(FRONTEIRAS, numeroIndividuosFactiveis);// Como não se sabe, a priori, em quantas fronteiras a população está dividida, considera-se o caso extremo em que cada indivíduo está numa fronteira diferente
    for(contadorI = 0; contadorI < numeroIndividuosFactiveis; contadorI++){
		(*fronteiraParam)[contadorI].numeroIndividuos = 0;
		(*fronteiraParam)[contadorI].nivelDominancia = 0;
		(*fronteiraParam)[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, numeroIndividuosFactiveis); // Como não se sabe, a priori, quantos indivíduos estarão em cada fronteira, considera-se o caso extremo em que todos os indivíduos estejam numa mesma fronteira
	}


    //Divide a população factível em fronteira segundo o critério de não-dominancia
    idFronteira = 0;
	contadorIndividuosVerificados = 0;
	while(contadorIndividuosVerificados < numeroIndividuosFactiveis){
		//Determina a qual fronteira cada indivíduo pertence
		for (contadorI = 0; contadorI < numeroIndividuosFactiveis; contadorI++) {
			idConfiguracao1 = individuos[contadorI].idConfiguracao;
			rank = 0;

			if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
				for (contadorJ = 0; contadorJ < numeroIndividuosFactiveis; contadorJ++) {
					idConfiguracao2 = individuos[contadorJ].idConfiguracao;
					// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
					if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
						if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
							if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
									&& (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) >= (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas)) {
								rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
								if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
										&& (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas)) {
									rank--;
								}
							}
						}
					}
				}
			}


			//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
			if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertence a fronteira "idFronteira"
				numIndividuos = (*fronteiraParam)[idFronteira].numeroIndividuos;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].valorObjetivo1 = configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida;
				(*fronteiraParam)[idFronteira].individuos[numIndividuos].valorObjetivo2 = (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas);
				(*fronteiraParam)[idFronteira].nivelDominancia = idFronteira;
				(*fronteiraParam)[idFronteira].numeroIndividuos++;
				//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				contadorIndividuosVerificados++;
			}


		}
		for(contadorJ = 0; contadorJ < (*fronteiraParam)[idFronteira].numeroIndividuos; contadorJ++){
			idConfiguracao1 = (*fronteiraParam)[idFronteira].individuos[contadorJ].idConfiguracao;
			configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
		}

		idFronteira++;
	}
	if(idFronteira>0)
		numeroDeFronteirasParam[0] = idFronteira - 1;
	free(individuos); individuos = NULL;
}

/*
 * Por Leandro:
 * Dada uma fronteira de Pareto com um nível de dominância qualquer, esta função procura por indivíduos pertencentes a esta fronteiras que,
 * embora tenham indices diferentes, consistem, na verdade, numa mesma configuração da rede.
 * Para tal, esta função compara diversas características dos indivíduos presentes na fronteira de tabela
 * Após localizar os indivídos iguais, esta função exclui os repetidos, deixando apenas um deles.
 */
void otimizaFronteiraPareto(FRONTEIRAS *fronteirasParam, int idFronteiraParam, FRONTEIRAS *fronteiraParetoParam, CONFIGURACAO *configuracoesParam){
	int indice1, indice2, indice3;
	long int idConfiguracao1, idConfiguracao2;
	long int *idConfiguracoesIguais, *indicesConfiguracoesIguaisEmFronteiasParam ;
	int contadorIndividuosIguais;
	idConfiguracoesIguais =  Malloc(long int, fronteirasParam[idFronteiraParam].numeroIndividuos);
	indicesConfiguracoesIguaisEmFronteiasParam =  Malloc(long int, fronteirasParam[idFronteiraParam].numeroIndividuos);

	fronteiraParetoParam[0].individuos = Malloc(INDIVIDUOSNAODOMINADOS, fronteirasParam[idFronteiraParam].numeroIndividuos);
	fronteiraParetoParam[0].nivelDominancia = 0;
	fronteiraParetoParam[0].numeroIndividuos = 0;

	for(indice1 = 0; indice1< fronteirasParam[idFronteiraParam].numeroIndividuos; indice1++){
		idConfiguracao1 = fronteirasParam[idFronteiraParam].individuos[indice1].idConfiguracao;
		indice3 = indice1;
		if(idConfiguracao1 > 0 ){
			contadorIndividuosIguais = 0;
			for(indice2 = indice1+1; indice2 < fronteirasParam[idFronteiraParam].numeroIndividuos; indice2++){
				idConfiguracao2 = fronteirasParam[idFronteiraParam].individuos[indice2].idConfiguracao;
				if(idConfiguracao2 > 0 ){
					if(idConfiguracao1 != idConfiguracao2 &&
							floor(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede)    == floor(configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo)   == floor(configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.quedaMaxima) 			 == floor(configuracoesParam[idConfiguracao2].objetivo.quedaMaxima) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.potenciaTotalNaoSuprida)  == floor(configuracoesParam[idConfiguracao2].objetivo.potenciaTotalNaoSuprida) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida)   == floor(configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)          == floor(configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)      == floor(configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) &&
							//floor(configuracoesParam[idConfiguracao1].objetivo.perdasResistivas)         == floor(configuracoesParam[idConfiguracao2].objetivo.perdasResistivas) &&
							//floor(configuracoesParam[idConfiguracao1].objetivo.menorTensao)              == floor(configuracoesParam[idConfiguracao2].objetivo.menorTensao) &&
							//floor(configuracoesParam[idConfiguracao1].objetivo.noMenorTensao)            == floor(configuracoesParam[idConfiguracao2].objetivo.noMenorTensao) &&
							//floor(configuracoesParam[idConfiguracao1].objetivo.noMaiorCarregamentoRede)  == floor(configuracoesParam[idConfiguracao2].objetivo.noMaiorCarregamentoRede) &&
							//floor(configuracoesParam[idConfiguracao1].objetivo.idTrafoMaiorCarregamento) == floor(configuracoesParam[idConfiguracao2].objetivo.idTrafoMaiorCarregamento) &&
							//floor(configuracoesParam[idConfiguracao1].objetivo.ponderacao) 				 == floor(configuracoesParam[idConfiguracao2].objetivo.ponderacao) &&

							floor(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) 		   == floor(configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)  == floor(configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)          == floor(configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)            == floor(configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) &&

							floor(configuracoesParam[idConfiguracao1].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta)          == floor(configuracoesParam[idConfiguracao2].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConfiguracao2].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa)         == floor(configuracoesParam[idConfiguracao2].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa) &&
							floor(configuracoesParam[idConfiguracao1].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade)           == floor(configuracoesParam[idConfiguracao2].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade)   )
					{
						idConfiguracoesIguais[contadorIndividuosIguais] = idConfiguracao2; //Armazena o ID da configuração que é igual a "idConfiguracao1"
						indicesConfiguracoesIguaisEmFronteiasParam[contadorIndividuosIguais] = indice2; //Armazena o indice que a configuração que é igual a "idConfiguracao1" possui em "fronteirasParam"
						contadorIndividuosIguais++;
						fronteirasParam[idFronteiraParam].individuos[indice2].idConfiguracao = -1;
					}
				}

			}

			if(contadorIndividuosIguais > 0){
				for(indice2 = 0; indice2 < contadorIndividuosIguais; indice2++){ //determina o individuo com o menor ID (normalmente, individuos com ID menor possuem menos chaves repetidas
					if(idConfiguracoesIguais[indice2] < idConfiguracao1){
						idConfiguracao1 = idConfiguracoesIguais[indice2];
						indice3 = indicesConfiguracoesIguaisEmFronteiasParam[indice2];
					}
				}
			}
			fronteiraParetoParam[0].individuos[fronteiraParetoParam[0].numeroIndividuos].idConfiguracao = idConfiguracao1;
			fronteiraParetoParam[0].individuos[fronteiraParetoParam[0].numeroIndividuos].valorObjetivo1 = fronteirasParam[idFronteiraParam].individuos[indice3].valorObjetivo1;
			fronteiraParetoParam[0].individuos[fronteiraParetoParam[0].numeroIndividuos].valorObjetivo2 = fronteirasParam[idFronteiraParam].individuos[indice3].valorObjetivo2;
			fronteiraParetoParam[0].numeroIndividuos++;
	   }
	}

	free(idConfiguracoesIguais);
}
/*
 * Por Leandro:
 * Dada uma fronteira de Pareto com um nível de dominância qualquer, esta função procura por indivíduos pertencentes a esta fronteiras que,
 * embora tenham indices diferentes, consistem, na verdade, numa mesma configuração da rede.
 * Após localizar os indivídos iguais, esta função exclui os repetidos, deixando apenas um deles.
 *
 * A única diferença desta funçaõ em relação a função "otimizaFronteiraPareto()" reside no fato de que nesta nova versão esta função
 * compara os indivíduos presentes na fronteira em relação aos seus valores de Objetivo1 e de Objetivo2.
 *
 */
void otimizaFronteiraParetoV2(FRONTEIRAS *fronteirasParam, int idFronteiraParam, FRONTEIRAS *fronteiraParetoParam, CONFIGURACAO *configuracoesParam){
	int indice1, indice2;
	int contadorSolucoesDistintas;
	BOOL solucaoDistinta;
	FRONTEIRAS fronteiraSolucoesDistintas;

	//Determina na fronteira aquelas soluções que são distintas
	fronteiraSolucoesDistintas.individuos = malloc(fronteirasParam[idFronteiraParam].numeroIndividuos*sizeof(INDIVIDUOSNAODOMINADOS));
	fronteiraSolucoesDistintas.individuos[0].idConfiguracao = fronteirasParam[idFronteiraParam].individuos[0].idConfiguracao;
	fronteiraSolucoesDistintas.individuos[0].valorObjetivo1 = fronteirasParam[idFronteiraParam].individuos[0].valorObjetivo1;
	fronteiraSolucoesDistintas.individuos[0].valorObjetivo2 = fronteirasParam[idFronteiraParam].individuos[0].valorObjetivo2;
	contadorSolucoesDistintas = 1;
	for(indice1 = 1; indice1 < fronteirasParam[idFronteiraParam].numeroIndividuos; indice1++){
		solucaoDistinta = true;
		for(indice2 = 0; indice2 < contadorSolucoesDistintas && solucaoDistinta == true; indice2++){
			if(floor(fronteirasParam[idFronteiraParam].individuos[indice1].valorObjetivo1) == floor(fronteiraSolucoesDistintas.individuos[indice2].valorObjetivo1) && floor(fronteirasParam[idFronteiraParam].individuos[indice1].valorObjetivo2) == floor(fronteiraSolucoesDistintas.individuos[indice2].valorObjetivo2))
				solucaoDistinta = false;
		}

		if(solucaoDistinta == true){
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].idConfiguracao = fronteirasParam[idFronteiraParam].individuos[indice1].idConfiguracao;
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].valorObjetivo1 = fronteirasParam[idFronteiraParam].individuos[indice1].valorObjetivo1;
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].valorObjetivo2 = fronteirasParam[idFronteiraParam].individuos[indice1].valorObjetivo2;
			contadorSolucoesDistintas++;
		}
	}
	fronteiraSolucoesDistintas.numeroIndividuos = contadorSolucoesDistintas;
	fronteiraSolucoesDistintas.nivelDominancia = fronteirasParam[idFronteiraParam].nivelDominancia;

	//Copia as soluções distintas para a Fronteira de Pareto
	fronteiraParetoParam[0].individuos = malloc(fronteiraSolucoesDistintas.numeroIndividuos*sizeof(INDIVIDUOSNAODOMINADOS));
	for(indice1 = 0; indice1 < fronteiraSolucoesDistintas.numeroIndividuos; indice1++){
		fronteiraParetoParam[0].individuos[indice1].idConfiguracao = fronteiraSolucoesDistintas.individuos[indice1].idConfiguracao;
		fronteiraParetoParam[0].individuos[indice1].valorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice1].valorObjetivo1;
		fronteiraParetoParam[0].individuos[indice1].valorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice1].valorObjetivo2;
	}
	fronteiraParetoParam[0].nivelDominancia = fronteiraSolucoesDistintas.nivelDominancia;
	fronteiraParetoParam[0].numeroIndividuos = fronteiraSolucoesDistintas.numeroIndividuos;


	free(fronteiraSolucoesDistintas.individuos);
}

/* Por Leandro:
 * Descrição: dada uma fornteira de Pareto, ordena os indivíduos em ordem decrescente dos valores do "Objetivo 1"
 *
 * @param fronteiraParetoParam é a fronteira de Pareto
 */

void ordenaCrescenteIndividuosFronteiraParetoObjetivo1(FRONTEIRAS *fronteiraParetoParam){
	int indice1, indice2, indice3;
	double valorObjetivo1Aux, valorObjetivo2Aux;
	long int idConfiguracaoAux;

	for (indice1 = 0; indice1 < fronteiraParetoParam[0].numeroIndividuos; indice1++){
        for (indice2 = indice1 + 1; indice2 < fronteiraParetoParam[0].numeroIndividuos; indice2++) {
        	if(floor(fronteiraParetoParam[0].individuos[indice1].valorObjetivo1)  > floor(fronteiraParetoParam[0].individuos[indice2].valorObjetivo1) ){

        		valorObjetivo1Aux = fronteiraParetoParam[0].individuos[indice2].valorObjetivo1;
        		valorObjetivo2Aux = fronteiraParetoParam[0].individuos[indice2].valorObjetivo2;
        		idConfiguracaoAux = fronteiraParetoParam[0].individuos[indice2].idConfiguracao;
        		for(indice3 = indice2; indice3 > indice1; indice3--){
        			fronteiraParetoParam[0].individuos[indice3].valorObjetivo1 = fronteiraParetoParam[0].individuos[indice3 - 1].valorObjetivo1;
        			fronteiraParetoParam[0].individuos[indice3].valorObjetivo2 = fronteiraParetoParam[0].individuos[indice3 - 1].valorObjetivo2;
        			fronteiraParetoParam[0].individuos[indice3].idConfiguracao = fronteiraParetoParam[0].individuos[indice3 - 1].idConfiguracao;
        		}
        		fronteiraParetoParam[0].individuos[indice1].valorObjetivo1 = valorObjetivo1Aux;
        		fronteiraParetoParam[0].individuos[indice1].valorObjetivo2 = valorObjetivo2Aux;
        		fronteiraParetoParam[0].individuos[indice1].idConfiguracao = idConfiguracaoAux;
        	}
        }
    }

}

/*
 * Por Leandro
 * Descrição: esta função seleciona as soluções finais para o problema e retorna índice destes indivíduos na estrutura "fronteiraParam".
 * Na seleção das soluções finais, inicia-se pela solução mais próxima do joelho da curva e depois caminha-se nos dois sentidos da curva pegando
 * as soluções mais próximas, até que seja obtido uma quantidade de soluções finais igual àquela desejada.
 *
 * A partir de tal informação é possível obter em ""fronteiraParam" o índice destes indivíduos na estrutura "configuracoesParam".
 *
 * P.S.: Esta função exige que as soluções em fronteiraParam estejam ordenadas em ordem crescente do valor do objetivo 1
 *
 * @return os indíces em "fronteiraParam" dos indivíduos selecionados como solução final
 */
long int *selecionaSolucoesFinaisOpcao1(int *numeroSolucoesFinaisParam, FRONTEIRAS fronteiraParam){
	long int *idSolucoesFinais;
	int indice, indice1, indice2, contadorSolucoesDistintas = 0, contadorSolucoesFinais;
	BOOL solucaoDistinta;
	FRONTEIRAS fronteiraSolucoesDistintas;
	double menorValorObjetivo1, menorValorObjetivo2, maiorValorObjetivo1, maiorValorObjetivo2;

	//Determina na fronteira aquelas soluções que são distintas
	fronteiraSolucoesDistintas.individuos = malloc(fronteiraParam.numeroIndividuos*sizeof(INDIVIDUOSNAODOMINADOS));
	fronteiraSolucoesDistintas.individuos[0].idConfiguracao = 0; //Excepicionalmente nesta função, esta variável irá armazenar o índice do indivíduo em "fronteiraParam" em vez do índice do mesmo em "fronteiraParam"
	fronteiraSolucoesDistintas.individuos[0].valorObjetivo1 = fronteiraParam.individuos[0].valorObjetivo1;
	fronteiraSolucoesDistintas.individuos[0].valorObjetivo2 = fronteiraParam.individuos[0].valorObjetivo2;
	contadorSolucoesDistintas = 1;
	for(indice1 = 1; indice1 < fronteiraParam.numeroIndividuos; indice1++){
		solucaoDistinta = true;
		for(indice2 = 0; indice2 < contadorSolucoesDistintas && solucaoDistinta == true; indice2++){
			if(floor(fronteiraParam.individuos[indice1].valorObjetivo1) == floor(fronteiraSolucoesDistintas.individuos[indice2].valorObjetivo1) && floor(fronteiraParam.individuos[indice1].valorObjetivo2) == floor(fronteiraSolucoesDistintas.individuos[indice2].valorObjetivo2))
				solucaoDistinta = false;
		}

		if(solucaoDistinta == true){
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].idConfiguracao = indice1;
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].valorObjetivo1 = fronteiraParam.individuos[indice1].valorObjetivo1;
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].valorObjetivo2 = fronteiraParam.individuos[indice1].valorObjetivo2;
			contadorSolucoesDistintas++;
		}
	}
	fronteiraSolucoesDistintas.numeroIndividuos = contadorSolucoesDistintas;

	if(contadorSolucoesDistintas <= numeroSolucoesFinaisParam[0]){ //Se o número de soluções diferentes presentes na fronteira é igual ou menor que o número desejado de soluções Finais, Então as soluções Finais são as próprias soluções distintas
		numeroSolucoesFinaisParam[0] = contadorSolucoesDistintas;
		//Salva as soluções distintas
		idSolucoesFinais = malloc(numeroSolucoesFinaisParam[0]*sizeof(long int));
		for(indice1 = 0; indice1 < numeroSolucoesFinaisParam[0]; indice1++)
			idSolucoesFinais[indice1] = fronteiraSolucoesDistintas.individuos[indice1].idConfiguracao;
	}
	else{ //Se o número de soluções distintas é maior que o número desejado de soluções finais, então deve-se selecionar, dentre as soluções distintas, as soluções finais

		idSolucoesFinais = malloc(numeroSolucoesFinaisParam[0]*sizeof(long int));
		//NORMALIZAÇÃO DOS VALORES DOS OBJETIVOS 1 E 2
		//Determinação dos valores máximos e mínimos para os objetivos
		indice=0;
		menorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;
		menorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
		maiorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;
		maiorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
		for(indice = 1; indice < fronteiraSolucoesDistintas.numeroIndividuos; indice++){
			//Determina o menor valor para Objetivo 1
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 < menorValorObjetivo1)
				menorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;

			//Determina o menor valor para Objetivo 2
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 < menorValorObjetivo2)
				menorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;

			//Determina o maior valor para Objetivo 1
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 > maiorValorObjetivo1)
				maiorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;

			//Determina o maior valor para Objetivo 2
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 > maiorValorObjetivo2)
				maiorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
		}

		//Normalização dos valores de objetivo 1 e 2
		for(indice = 0; indice < fronteiraSolucoesDistintas.numeroIndividuos; indice++){
			fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 = (fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 - menorValorObjetivo1) / (maiorValorObjetivo1 - menorValorObjetivo1);
			fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 = (fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 - menorValorObjetivo2) / (maiorValorObjetivo2 - menorValorObjetivo2);
		}

		//Determina o indice em "fronteiraSolucoesDistintas" da solução (indivíduo) mais próxima do joelho da fronteira de Pareto
		indice = 0;
		while(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 < fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2)
			indice++;
		indice--;
		//As soluções finais serão aquelas no entorno do joelho da Fronteira de Pareto
		idSolucoesFinais[0] = fronteiraSolucoesDistintas.individuos[indice].idConfiguracao;
		contadorSolucoesFinais = 1;
		indice1 = indice2 = indice;
		while(contadorSolucoesFinais < numeroSolucoesFinaisParam[0]){
			indice1--;
			indice2++;
			if(indice1 >= 0 && contadorSolucoesFinais < numeroSolucoesFinaisParam[0]){
				idSolucoesFinais[contadorSolucoesFinais] = fronteiraSolucoesDistintas.individuos[indice1].idConfiguracao;
				contadorSolucoesFinais++;
			}
			if(indice2 < fronteiraSolucoesDistintas.numeroIndividuos && contadorSolucoesFinais < numeroSolucoesFinaisParam[0]){
				idSolucoesFinais[contadorSolucoesFinais] = fronteiraSolucoesDistintas.individuos[indice2].idConfiguracao;
				contadorSolucoesFinais++;
			}
		}
	}

	free(fronteiraSolucoesDistintas.individuos);
	return idSolucoesFinais;
}

/*
 * Por Leandro
 * Descrição: esta função seleciona as soluções finais para o problema e retorna índice destes indivíduos na estrutura "fronteiraParam".
 * Nesta opção são selecionadas apenas  3 soluções finais. São elas:
 *  1. A solução mais róxima do joelho da curva.
 *  2. A solução com o menor valor de Objetivo 1, e
 *  3. A solução com o menor valor de Objetivo 3.
 *
 * A partir de tal informação é possível obter em ""fronteiraParam" o índice destes indivíduos na estrutura "configuracoesParam".
 *
 * P.S.: Esta função exige que as soluções em fronteiraParam estejam ordenadas em ordem crescente do valor do objetivo 1
 *
 * @return os indíces em "fronteiraParam" dos indivíduos selecionados como solução final
 */
long int *selecionaSolucoesFinaisOpcao2(int *numeroSolucoesFinaisParam, FRONTEIRAS fronteiraParam){
	long int *idSolucoesFinais, indiceSolucaoFinalObjetivo1, indiceSolucaoFinalObjetivo2;
	int indice, indice1, indice2, contadorSolucoesDistintas = 0, contadorSolucoesFinais;
	BOOL solucaoDistinta;
	FRONTEIRAS fronteiraSolucoesDistintas;
	double menorValorObjetivo1, menorValorObjetivo2, maiorValorObjetivo1, maiorValorObjetivo2;



	numeroSolucoesFinaisParam[0] = 3;



	//Determina na fronteira aquelas soluções que são distintas
	fronteiraSolucoesDistintas.individuos = malloc(fronteiraParam.numeroIndividuos*sizeof(INDIVIDUOSNAODOMINADOS));
	fronteiraSolucoesDistintas.individuos[0].idConfiguracao = 0; //Excepicionalmente nesta função, esta variável irá armazenar o índice do indivíduo em "fronteiraParam" em vez do índice do mesmo em "fronteiraParam"
	fronteiraSolucoesDistintas.individuos[0].valorObjetivo1 = fronteiraParam.individuos[0].valorObjetivo1;
	fronteiraSolucoesDistintas.individuos[0].valorObjetivo2 = fronteiraParam.individuos[0].valorObjetivo2;
	contadorSolucoesDistintas = 1;
	for(indice1 = 1; indice1 < fronteiraParam.numeroIndividuos; indice1++){
		solucaoDistinta = true;
		for(indice2 = 0; indice2 < contadorSolucoesDistintas && solucaoDistinta == true; indice2++){
			if(floor(fronteiraParam.individuos[indice1].valorObjetivo1) == floor(fronteiraSolucoesDistintas.individuos[indice2].valorObjetivo1) && floor(fronteiraParam.individuos[indice1].valorObjetivo2) == floor(fronteiraSolucoesDistintas.individuos[indice2].valorObjetivo2))
				solucaoDistinta = false;
		}

		if(solucaoDistinta == true){
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].idConfiguracao = indice1;
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].valorObjetivo1 = fronteiraParam.individuos[indice1].valorObjetivo1;
			fronteiraSolucoesDistintas.individuos[contadorSolucoesDistintas].valorObjetivo2 = fronteiraParam.individuos[indice1].valorObjetivo2;
			contadorSolucoesDistintas++;
		}

	}
	fronteiraSolucoesDistintas.numeroIndividuos = contadorSolucoesDistintas;

	if(contadorSolucoesDistintas <= numeroSolucoesFinaisParam[0]){ //Se o número de soluções diferentes presentes na fronteira é igual ou menor que o número desejado de soluções Finais, Então as soluções Finais são as próprias soluções distintas
		numeroSolucoesFinaisParam[0] = contadorSolucoesDistintas;
		//Salva as soluções distintas
		idSolucoesFinais = malloc(contadorSolucoesDistintas*sizeof(long int));
		for(indice1 = 0; indice1 < contadorSolucoesDistintas; indice1++)
			idSolucoesFinais[indice1] = fronteiraSolucoesDistintas.individuos[indice1].idConfiguracao;
	}
	else{ //Se o número de soluções distintas é maior que o número desejado de soluções finais, então deve-se selecionar, dentre as soluções distintas, as soluções finais

		idSolucoesFinais = malloc(numeroSolucoesFinaisParam[0]*sizeof(long int));
		//NORMALIZAÇÃO DOS VALORES DOS OBJETIVOS 1 E 2
		//Determinação dos valores máximos e mínimos para os objetivos
		indice=0;
		menorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;
		menorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
		maiorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;
		maiorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
		for(indice = 1; indice < fronteiraSolucoesDistintas.numeroIndividuos; indice++){
			//Determina o menor valor para Objetivo 1
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 < menorValorObjetivo1){
				menorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;
				indiceSolucaoFinalObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].idConfiguracao;
			}

			//Determina o menor valor para Objetivo 2
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 < menorValorObjetivo2){
				menorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
				indiceSolucaoFinalObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].idConfiguracao;
			}

			//Determina o maior valor para Objetivo 1
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 > maiorValorObjetivo1)
				maiorValorObjetivo1 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1;

			//Determina o maior valor para Objetivo 2
			if(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 > maiorValorObjetivo2)
				maiorValorObjetivo2 = fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2;
		}

		//Normalização dos valores de objetivo 1 e 2
		for(indice = 0; indice < fronteiraSolucoesDistintas.numeroIndividuos; indice++){
			fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 = (fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 - menorValorObjetivo1) / (maiorValorObjetivo1 - menorValorObjetivo1);
			fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 = (fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2 - menorValorObjetivo2) / (maiorValorObjetivo2 - menorValorObjetivo2);
		}

		//DEFINE AS SOLUÇÕES FINAIS
		//Determina o indice em "fronteiraSolucoesDistintas" da solução (indivíduo) mais próxima do joelho da fronteira de Pareto
		indice = 0;
		while(fronteiraSolucoesDistintas.individuos[indice].valorObjetivo1 < fronteiraSolucoesDistintas.individuos[indice].valorObjetivo2)
			indice++;
		indice--;

		//SOLUÇÃO FINAL 1 - a solução (indivíduo) mais próxima do joelho da fronteira de Pareto
		idSolucoesFinais[0] = fronteiraSolucoesDistintas.individuos[indice].idConfiguracao;
		contadorSolucoesFinais = 1;
		//SOLUÇÃO FINAL 2 - a solução (indivíduo) com o menor valor de Objetivo 1 dentre aquelas presentes na fronteira de Pareto
		contadorSolucoesFinais++;
		idSolucoesFinais[1] = indiceSolucaoFinalObjetivo1;
		//SOLUÇÃO FINAL 3 - a solução (indivíduo) com o menor valor de Objetivo 2 dentre aquelas presentes na fronteira de Pareto
		idSolucoesFinais[2] = indiceSolucaoFinalObjetivo1;
		contadorSolucoesFinais++;

	}

	free(fronteiraSolucoesDistintas.individuos);
	return idSolucoesFinais;
}


/**
 * Cálcula o número de manobras de acordo com o subtipo das chaves.
 * @param configuracoesParam
 * @param idChaveAberta
 * @param idChaveFechada
 * @param listaChavesParam
 * @param idConfiguracaoParam
 * @param estadoInicialChaves
 * @param numChaveAberta
 * @param numChaveFechada
 * @param idNovaConfiguracaoParam
 */
void numeroManobrasSubTipo(CONFIGURACAO *configuracoesParam,
        long int idChaveAberta, long int idChaveFechada, LISTACHAVES *listaChavesParam,
        int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves, long int *numChaveAberta,
        long int *numChaveFechada, int idNovaConfiguracaoParam) {
    int manobrasAutomatica, manobrasManual;
    int seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
    int comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
    int comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
    int curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
    int curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;
    //encontra o nó raiz de P na RNP


    manobrasAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    manobrasManual = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    if (idChaveAberta >= 0) {
        if (estadoInicialChaves[idChaveAberta] == normalmenteAberta) {
            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
                manobrasAutomatica--;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica--;
                } else
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaAutomatica--;
                }
            } else {
                manobrasManual--;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoManual--;
                } else {
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                        comCargaManual--;
                    } else {
                        if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca--;

                        }
                    }
                }

            }
        } else {

            if (listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica) {
                manobrasAutomatica++;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica++;
                } else
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                    comCargaAutomatica++;
                }
            } else {
                manobrasManual++;
                if (listaChavesParam[idChaveAberta].subTipoChave == chaveCurtoCircuito) {
                    curtoManual++;
                } else {
                    if (listaChavesParam[idChaveAberta].subTipoChave == chaveCarga) {
                        comCargaManual++;
                    } else {
                        if ((listaChavesParam[idChaveAberta].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca++;

                        }
                    }
                }
            }
        }
    }
    if (idChaveFechada >= 0) {
        if (estadoInicialChaves[idChaveFechada] == normalmenteFechada) {
            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
                manobrasAutomatica--;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica--;
                } else
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaAutomatica--;
                }
            } else
            {
                manobrasManual--;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoManual--;
                } else {
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                        comCargaManual--;
                    } else {
                        if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveAberta].subTipoChave == chaveFusivel)) {
                            seca--;

                        }
                    }
                }
            }
        } else {

            if (listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica) {
                manobrasAutomatica++;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoAutomatica++;
                } else
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                    comCargaAutomatica++;
                }
            } else
            {
                manobrasManual++;
                if (listaChavesParam[idChaveFechada].subTipoChave == chaveCurtoCircuito) {
                    curtoManual++;
                } else {
                    if (listaChavesParam[idChaveFechada].subTipoChave == chaveCarga) {
                        comCargaManual++;
                    } else {
                        if ((listaChavesParam[idChaveFechada].subTipoChave == chaveSeca) || (listaChavesParam[idChaveFechada].subTipoChave == chaveFusivel)) {
                            seca++;

                        }
                    }
                }
            }
        }
    }
        numChaveAberta[0]++;
        numChaveFechada[0]++;
      
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasManuais = manobrasManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.manobrasAutomaticas = manobrasAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.seca = seca;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual =  comCargaManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica = comCargaAutomatica;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual = curtoManual;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica = curtoAutomatica;
   // printf("ponto 2 manobrasAuto %d manobrasManual %d \n", manobrasAutomatica, manobrasManual);
}

/*void inicializaObjetivosTiposChaves(CONFIGURACAO *configuracao, int numeroTrafosParam) {
    configuracao[0].objetivo.maiorDemandaAlimentador = 0.0;
    configuracao[0].objetivo.maiorCarregamentoRede = 0.0;
    configuracao[0].objetivo.maiorCarregamentoTrafo = 0;
    configuracao[0].objetivo.manobrasManuais = 0;
    configuracao[0].objetivo.manobrasAutomaticas = 0;
    configuracao[0].objetivo.contadorManobrasTipo.comCargaAutomatica = 0;
    configuracao[0].objetivo.contadorManobrasTipo.comCargaManual = 0;
    configuracao[0].objetivo.contadorManobrasTipo.curtoAutomatica = 0;
    configuracao[0].objetivo.contadorManobrasTipo.curtoManual = 0;
    configuracao[0].objetivo.contadorManobrasTipo.seca = 0;
    configuracao[0].objetivo.menorTensao = 0;
    //
    configuracao[0].objetivo.ponderacao = 0;
    configuracao[0].objetivo.quedaMaxima = 0;
    configuracao[0].objetivo.rank = 0;
    configuracao[0].objetivo.fronteira = 0;
    configuracao[0].objetivo.perdasResistivas = 0;
    configuracao[0].objetivo.consumidoresSemFornecimento= 0;
    configuracao[0].objetivo.consumidoresEspeciaisSemFornecimento = 0;
    configuracao[0].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
}
*/
/**
 * Cálcula a impedância de Thévenin do ramo informado.
 * @param configuracaoParam
 * @param matrizB
 * @param ZParam
 * @param setorBarra
 * @param barraChave
 * @param indiceRNP
 * @param indiceSetor
 * @return 
 */
__complex__ double calculaImpedanciaTheveninRamo(CONFIGURACAO configuracaoParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, long int setorBarra, long int barraChave, long int indiceRNP, long int indiceSetor) {//, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam) {
    int indice, indice1,indiceBarra;
    long int noS, noR, noM, noN;
    int pior;
    
    __complex__ double somaImpedancia;

    
    RNPSETOR rnpSetorSR;
    somaImpedancia = 0 + I * 0;
    //printf("menorTensao %lf \n", menorTensao);
    indice = indiceSetor;
    //percorre a rnp até a raiz do alimentador do setor da barra
    while(indice >= 0) {
        //localiza o setor anterior ao indiceSetor
        while(configuracaoParam.rnp[indiceRNP].nos[indice].profundidade >= configuracaoParam.rnp[indiceRNP].nos[indiceSetor].profundidade)
        {
            indice--;
        }
        noS = configuracaoParam.rnp[indiceRNP].nos[indiceSetor].idNo;
        noR = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        //printf("Setor noS %ld noR %ld \t", noS, noR);
      //  imprimeRNPSetor(rnpSetorSR);
        //localiza a barra do setor
        indiceBarra = 0;
        while(indiceBarra < rnpSetorSR.numeroNos && rnpSetorSR.nos[indiceBarra].idNo != barraChave)
            indiceBarra++;
        //percorre o setor somando as impedâncias no caminho da barra da chave e a raiz do alimentador
        indice1 = indiceBarra-1;
        while(indice1>=0) {
            while(rnpSetorSR.nos[indice1].profundidade > (rnpSetorSR.nos[indiceBarra].profundidade-1)){
                indice1--;
            }
            noM = rnpSetorSR.nos[indiceBarra].idNo;
            noN = rnpSetorSR.nos[indice1].idNo;
            //printf("Barra 1 %ld Barra2 %ld \n", noM, noN);
            somaImpedancia += valorZ(ZParam, noM, noN);
            indiceBarra = indice1;
            indice1--;
        }
        barraChave = rnpSetorSR.nos[indiceBarra].idNo;
        indiceSetor = indice;
        indice--;
    }
    return(somaImpedancia);
}

/**
 * Calcula a impedância de Thevenin para ramos no mesmo alimentador.
 * @param configuracaoParam
 * @param matrizB
 * @param ZParam
 * @param indiceSetor1
 * @param indiceSetor2
 * @param indiceRNP1
 * @param barraChave1
 * @param barraChave2
 * @return 
 */
__complex__ double calculaImpedanciaTheveninMesmoAlimentador(CONFIGURACAO configuracaoParam, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, long int indiceSetor1, long int indiceSetor2, int indiceRNP1, long int barraChave1, long int barraChave2) {
    int indiceS1,indiceS2, indice1, indiceBarra1, indiceBarra2;
    long int noS, noR, noM, noN;
    int pior;
    
    __complex__ double somaImpedancia;
    RNPSETOR rnpSetorSR1;
    RNPSETOR rnpSetorSR2;
    somaImpedancia = 0 + I * 0;
    //printf("menorTensao %lf \n", menorTensao);
    indiceS1 = indiceSetor1;
    indiceS2 = indiceSetor2;
    //percorre a rnp até a raiz do alimentador do setor da barra
    while(indiceSetor1 != indiceSetor2) {
        if (indiceS1 > indiceS2) {
            //localiza o setor anterior ao indiceSetor1
            while (configuracaoParam.rnp[indiceRNP1].nos[indiceS1].profundidade >= configuracaoParam.rnp[indiceRNP1].nos[indiceSetor1].profundidade) {
                indiceS1--;
            }
            noS = configuracaoParam.rnp[indiceRNP1].nos[indiceSetor1].idNo;
            noR = configuracaoParam.rnp[indiceRNP1].nos[indiceS1].idNo;
            rnpSetorSR1 = buscaRNPSetor(matrizB, noS, noR);
            //printf("Setor noS %ld noR %ld \t", noS, noR);
            //  imprimeRNPSetor(rnpSetorSR);
            //localiza a barra do setor
            indiceBarra1 = 0;
            while (indiceBarra1 < rnpSetorSR1.numeroNos && rnpSetorSR1.nos[indiceBarra1].idNo != barraChave1)
                indiceBarra1++;
            //percorre o setor somando as impedâncias no caminho da barra da chave e a raiz do alimentador
            indice1 = indiceBarra1 - 1;
            while (indice1 >= 0) {
                while (rnpSetorSR1.nos[indice1].profundidade > (rnpSetorSR1.nos[indiceBarra1].profundidade - 1)) {
                    indice1--;
                }
                noM = rnpSetorSR1.nos[indiceBarra1].idNo;
                noN = rnpSetorSR1.nos[indice1].idNo;
                //printf("Barra 1 %ld Barra2 %ld \n", noM, noN);
                somaImpedancia += valorZ(ZParam, noM, noN);
                indiceBarra1 = indice1;
                indice1--;
            }
            barraChave1 = rnpSetorSR1.nos[indiceBarra1].idNo;
            indiceSetor1 = indiceS1;
            indiceS1--;
        }
        else {

            //localiza o setor anterior ao indiceSetor2
            while (configuracaoParam.rnp[indiceRNP1].nos[indiceS2].profundidade >= configuracaoParam.rnp[indiceRNP1].nos[indiceSetor2].profundidade) {
                indiceS2--;
            }
            noS = configuracaoParam.rnp[indiceRNP1].nos[indiceSetor2].idNo;
            noR = configuracaoParam.rnp[indiceRNP1].nos[indiceS2].idNo;
            rnpSetorSR2 = buscaRNPSetor(matrizB, noS, noR);
            //printf("Setor noS %ld noR %ld \t", noS, noR);
            //  imprimeRNPSetor(rnpSetorSR);
            //localiza a barra do setor
            indiceBarra2 = 0;
            while (indiceBarra2 < rnpSetorSR2.numeroNos && rnpSetorSR2.nos[indiceBarra2].idNo != barraChave2)
                indiceBarra2++;
            //percorre o setor somando as impedâncias no caminho da barra da chave e a raiz do alimentador
            indice1 = indiceBarra2 - 1;
            while (indice1 >= 0) {
                while (rnpSetorSR2.nos[indice1].profundidade > (rnpSetorSR2.nos[indiceBarra2].profundidade - 1)) {
                    indice1--;
                }
                noM = rnpSetorSR2.nos[indiceBarra2].idNo;
                noN = rnpSetorSR2.nos[indice1].idNo;
                //printf("Barra 1 %ld Barra2 %ld \n", noM, noN);
                somaImpedancia += valorZ(ZParam, noM, noN);
                indiceBarra2 = indice1;
                indice1--;
            }
            barraChave2 = rnpSetorSR2.nos[indiceBarra2].idNo;

            indiceSetor2 = indiceS2;
            indiceS2--;

        }    
    }
    //localiza a barra comum dentro do alimentador em comum
    //localiza o setor anterior ao indiceSetor1
    while (configuracaoParam.rnp[indiceRNP1].nos[indiceS1].profundidade >= configuracaoParam.rnp[indiceRNP1].nos[indiceSetor1].profundidade) {
        indiceS1--;
    }
    noS = configuracaoParam.rnp[indiceRNP1].nos[indiceSetor1].idNo;
    noR = configuracaoParam.rnp[indiceRNP1].nos[indiceS1].idNo;
    rnpSetorSR2 = buscaRNPSetor(matrizB, noS, noR);
    //printf("Setor noS %ld noR %ld \t", noS, noR);
    //  imprimeRNPSetor(rnpSetorSR);
    //localiza a barra do setor 1
    indiceBarra1 = 0;
    while (indiceBarra1 < rnpSetorSR2.numeroNos && rnpSetorSR2.nos[indiceBarra1].idNo != barraChave1)
        indiceBarra1++;
    
    //localiza a barra do setor 2
    indiceBarra2 = 0;
    while (indiceBarra2 < rnpSetorSR2.numeroNos && rnpSetorSR2.nos[indiceBarra2].idNo != barraChave2)
        indiceBarra2++;
    
    while(indiceBarra1 != indiceBarra2)
    {
        if (indiceBarra1 > indiceBarra2) {
            indice1 = indiceBarra1-1;
             while (rnpSetorSR2.nos[indice1].profundidade > (rnpSetorSR2.nos[indiceBarra1].profundidade - 1)) {
                indice1--;
            }
            noM = rnpSetorSR2.nos[indiceBarra1].idNo;
            noN = rnpSetorSR2.nos[indice1].idNo;
            somaImpedancia += valorZ(ZParam, noM, noN);
            indiceBarra1 = indice1;
        } else {
            indice1 = indiceBarra2-1;
            while (rnpSetorSR2.nos[indice1].profundidade > (rnpSetorSR2.nos[indiceBarra2].profundidade - 1)) {
                indice1--;
            }
            noM = rnpSetorSR2.nos[indiceBarra2].idNo;
            noN = rnpSetorSR2.nos[indice1].idNo;
            somaImpedancia += valorZ(ZParam, noM, noN);
            indiceBarra2 = indice1;
        }
    }    
    return(somaImpedancia);
}

/**
 *Fluxo de Potência Indicativo - Caso 1: Loop entre alimentadores do mesmo trafo de subestação 
 * @param grafoSDRParam
 * @param numeroBarrasParam
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param configuracoesParam
 * @param indiceConfiguracao
 * @param matrizB
 * @param SBase
 * @param ZParam
 * @param tapReguladores
 * @param dadosRegulador
 * @param maximoCorrenteParam
 * @param numeroTrafos
 * @param numeroAlimentadores
 * @param rnpA
 * @param rnpP
 * @param barraDe
 * @param barraPara
 * @param setorBarraDe
 * @param setorBarraPara
 */
//----------------------------------------------------------------------------------
//
//
void fluxoPotenciaAnelCaso1(GRAFO *grafoSDRParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB, double SBase,
        MATRIZCOMPLEXA *ZParam, int *tapReguladores, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam, int numeroTrafos, int numeroAlimentadores,
        int rnpA, int rnpP, long int barraDe, long int barraPara, long int setorBarraDe, long int setorBarraPara)
{
    __complex__ double VThevenin;
    __complex__ double ZThevenin;
    __complex__ double ICompensacao = 0 + 0*I;
    __complex__ double IChave = 0 + 0*I;
    __complex__ double CargaDe = 0 + 0*I;
    __complex__ double CargaPara = 0 + 0*I;
    long int indiceSetorDe;
    long int indiceSetorPara;
    int itbk = 0,contador2;
    BOOL todosAlimentadores = true;
    BOOL copiaDados = false;
    
    //--------------------------------------------------------------------------
    //Encontra o índice dos setores - Pode ser alterado pela MatrizPi - retornar o parâmetro na função
    for(contador2 = 0; contador2 < configuracoesParam[indiceConfiguracao].rnp[rnpP].numeroNos; contador2++)
    {
        if(configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador2].idNo == setorBarraDe)
        {
            indiceSetorDe = contador2;
        }
    }
    for(contador2 = 0; contador2 < configuracoesParam[indiceConfiguracao].rnp[rnpA].numeroNos; contador2++)
    {
        if(configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador2].idNo == setorBarraPara)
        {
            indiceSetorPara = contador2;
        }
    }
    
    
    //--------------------------------------------------------------------------
    //PASSO 1: calcula fluxo atual sem loop
    // ****** Deve calcular o fluxo considerando a chave do loop como aberta
    // ****** Nesse ponto calcula todos.
    inicializaDadosEletricosPorAlimentador(grafoSDRParam, configuracoesParam, indiceConfiguracao, numeroBarrasParam, SBase, dadosTrafoParam, dadosAlimentadorParam, matrizB);   
    avaliaConfiguracao(todosAlimentadores, configuracoesParam, -1, -1, indiceConfiguracao, 
            dadosTrafoParam, numeroTrafos, numeroAlimentadores, tapReguladores, 
            dadosRegulador, dadosAlimentadorParam, indiceConfiguracao, 
            matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, copiaDados);
    
    imprimeIndicadoresEletricos(configuracoesParam[indiceConfiguracao]);
    
    //--------------------------------------------------------------------------
    //PASSO 2:
    // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
    VThevenin = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe] - configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara];
   //printf("\n\nVthevenin = %lf + j. %lf",__real__ VThevenin,__imag__ VThevenin );
    
    //--------------------------------------------------------------------------
    //PASSO 3: calcula impedancia thevenin
    // ******* Recebe o índice das barras "barraDe" e "barraPara" da chave que formou o loop e retorna a impedância de thévenin (Real + Imag) entre estas barras (soma de impedâncias)
    //a ligação ocorre na mesma rnp
    if (rnpA == rnpP) {
        ZThevenin = calculaImpedanciaTheveninMesmoAlimentador(configuracoesParam[indiceConfiguracao], matrizB,
                ZParam, indiceSetorDe, indiceSetorPara, rnpA, barraDe, barraPara);
    }
    else
    {
        ZThevenin = calculaImpedanciaTheveninRamo(configuracoesParam[indiceConfiguracao], matrizB,
                ZParam, setorBarraDe, barraDe,rnpP,indiceSetorDe) + calculaImpedanciaTheveninRamo(configuracoesParam[indiceConfiguracao], matrizB,
                ZParam, setorBarraPara, barraPara,rnpA,indiceSetorPara);
    }
   // printf("\nZthevenin = %lf + j. %lf\n",__real__ ZThevenin,__imag__ ZThevenin);
    
    CargaDe = configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe];
    CargaPara = configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara];
    
    //--------------------------------------------------------------------------
    //PASSO 4: verifica convergencia
    // ******* Verifica a convergência: abs(Vthénvenin - Z_chave*Ichave) < tolerancia ---- Acredito que o Z_chave seja zero no modelo da COPEL. Este é um ponto que podemos ver com mais atenção dependendo dos dados da COPEL.
    // ******* Inicialmente Icompesação = 0;
    // ******* Se convergiu retornar para o main() pois a corrente não precisa de mais compensação.
    // ******* Se não convergiu, continua com a atualização das correntes.
    while ((cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave) > 0.01) && (itbk <= 20)) {        
        itbk++;
      //  printf("\nitbk = %d",itbk);
      //  printf("\n\nV Barra de: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe])*180/3.1415);
      //  printf("\nV Barra para: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara])*180/3.1415);
      //  printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        //----------------------------------------------------------------------
        //PASSO 5: atualiza as correntes
        // ******* A corrente a ser atualizada será: 
        ICompensacao = (VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave)/(ZThevenin+valorZ(ZParam, barraDe, barraPara));
        IChave += ICompensacao;
      //  printf("\nIcomp = %lf + j. %lf\n",__real__ ICompensacao,__imag__ ICompensacao);
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
        
        // ******* A atualização deve acrescentar na "barraDe" uma corrente igual a Icompensação e na "barraPara" uma corrente igual a -Icompensação:
        // ******* Foi implementado atualizando as potências das barras (no fim do algoritmo retorna as demandas para os valores originais de carga)
        configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe] += configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]*conj(ICompensacao);
        configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara] -= configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]*conj(ICompensacao);
        
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].corrente += -Icomensação
        // ******* 
        // ******* Só um detalhe neste ponto da atualização. Dentro da sua rotina do Fluxo (avaliaConfiguracao), você recalcula esse valor configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].corrente
        // ******* com os valores de potência da barra (configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].potencia). Então a atualização deve ser na potência para você não precisar alterar a função do fluxo de potência:
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].potencia += configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].vBarra*conj(Icomensação)
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].potencia+= configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].vBarra*conj(-Icomensação)
        // ******* 
        // ******* Retorna para o PASSO 1
        //PASSO 1: calcula fluxo atual sem loop
        // ****** Deve calcular o fluxo considerando a chave do loop como aberta
        // ****** Calcular somente para alimentadores envolvidos no loop
        avaliaConfiguracao(todosAlimentadores, configuracoesParam, -1, -1, 
                indiceConfiguracao, dadosTrafoParam, numeroTrafos, numeroAlimentadores, 
                tapReguladores, dadosRegulador, dadosAlimentadorParam,
                indiceConfiguracao, matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, copiaDados);
        
        //PASSO 2:
        // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
        VThevenin = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe] - configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara];
    }
    //--------------------------------------------------------------------------
    // ******Convergência com número de iterações máximo
    if (itbk >= 20){
        printf("Fluxo de Potencia em Anel Divergiu.");
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
    }
    else{
        printf("\nitbk = %d",itbk);
        printf("\n\nV Barra de: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe])*180/3.1415);
        printf("\nV Barra para: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara])*180/3.1415);
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
    }
    //--------------------------------------------------------------------------
    //Retorna os valores das cargas das barras da chave ao valor inicial
    configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe] = CargaDe;
    configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara] = CargaPara;
    
    //--------------------------------------------------------------------------
    //Atualiza o objetivo de carregamento caso o carregamento da chave seja maior
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara)))
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara));
    
    imprimeIndicadoresEletricos(configuracoesParam[indiceConfiguracao]);
}


//----------------------------------------------------------------------------------
//
//
/**
 * Fluxo de Potência Idicativo - Caso 2: Loop entre trafos distintos da mesma subestação 
 * @param grafoSDRParam
 * @param numeroBarrasParam
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param configuracoesParam
 * @param indiceConfiguracao
 * @param matrizB
 * @param SBase
 * @param numeroSetores
 * @param ZParam
 * @param indiceReguladores
 * @param dadosRegulador
 * @param maximoCorrenteParam
 * @param numeroTrafos
 * @param numeroAlimentadores
 * @param rnpA
 * @param rnpP
 * @param barraDe
 * @param barraPara
 * @param setorBarraDe
 * @param setorBarraPara
 */
void fluxoPotenciaAnelCaso2(GRAFO *grafoSDRParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB, double SBase, long int numeroSetores,
        MATRIZCOMPLEXA *ZParam, int *indiceReguladores, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam, int numeroTrafos, int numeroAlimentadores,
        int rnpA, int rnpP, long int barraDe, long int barraPara,long int setorBarraDe, long int setorBarraPara) //Removido para teste: , MATRIZPI *matrizPiParam, VETORPI *vetorPiParam
{
    __complex__ double VThevenin;
    __complex__ double ZThevenin;
    __complex__ double ICompensacao = 0 + 0*I;
    __complex__ double IChave = 0 + 0*I;
    __complex__ double CargaDe = 0 + 0*I;
    __complex__ double CargaPara = 0 + 0*I;
    int itbk = 0;
    BOOL todosAlimentadores = true;
    BOOL copiaDados = false;
    
    long int contador, contador2;
    long int tamanhoRNP = configuracoesParam[indiceConfiguracao].rnp[rnpA].numeroNos + configuracoesParam[indiceConfiguracao].rnp[rnpP].numeroNos +2;
    CONFIGURACAO *configuracaoAuxiliar;
    long int barraAlimentadorP = dadosAlimentadorParam[rnpP+1].barraAlimentador;
    long int barraAlimentadorA = dadosAlimentadorParam[rnpA+1].barraAlimentador;
    long int barraAuxP = numeroBarrasParam+1;
    long int barraAuxA = numeroBarrasParam+2;
    long int barraSubEstacao = 0;
    long int barraAuxSub = numeroBarrasParam+3;
    long int indiceSetorDe;
    long int indiceSetorPara;
    long int setorAlimentadorA = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[0].idNo;
    long int setorAlimentadorP = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[0].idNo;
    long int setorRaiz = 0;
    long int setorAux = numeroSetores+1;
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    int posicaoRnpSetorP;
    int posicaoRnpSetorA;
    RNPSETOR rnpSetorSR;
    long int noS, noR, noN;
    double VF;
    double carregamentoTrafo1, carregamentoTrafo2;
    DADOSREGULADOR *dadosReguladorTemp = Malloc(DADOSREGULADOR, 3);
    //cria estrutura de configuração auxiliar
    configuracaoAuxiliar = alocaIndividuo(1, 0, 1, numeroTrafos); //, numeroTrafos);
    alocaRNP(tamanhoRNP, &configuracaoAuxiliar[0].rnp[0]);
    for (contador=0; contador<numeroTrafos; contador++)
    {
        configuracaoAuxiliar[0].objetivo.potenciaTrafo[contador] = configuracoesParam[indiceConfiguracao].objetivo.potenciaTrafo[contador];
    }
    
    //inicializa dados barra 0 
    grafoSDRParam[0].idSetor = 0;
    grafoSDRParam[0].idNo = 0;
    grafoSDRParam[0].valorPQ.p = 0;
    grafoSDRParam[0].valorPQ.q = 0;
    
    //inicializa rnpSetores, setor 0 com a barra equivalente;
    
    //rnp1 com origem no setor raiz da rnpP
    matrizB[setorRaiz].rnps[0].nos = Malloc(NORNP, (2));
    matrizB[setorRaiz].rnps[0].numeroNos = 2;
    matrizB[setorRaiz].rnps[0].idSetorOrigem = setorAux;
    matrizB[setorRaiz].rnps[0].nos[0].idNo = barraAuxSub;
    matrizB[setorRaiz].rnps[0].nos[0].profundidade = 0;
    matrizB[setorRaiz].rnps[0].nos[1].idNo = barraSubEstacao;
    matrizB[setorRaiz].rnps[0].nos[1].profundidade = 1;
    
    matrizB[setorAux].rnps[0].nos = Malloc(NORNP, (4));
    matrizB[setorAux].rnps[0].numeroNos = 4;
    matrizB[setorAux].rnps[0].idSetorOrigem = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[0].idNo;
    matrizB[setorAux].rnps[0].nos[0].idNo = barraAlimentadorP;
    matrizB[setorAux].rnps[0].nos[0].profundidade = 0;
    matrizB[setorAux].rnps[0].nos[1].idNo = barraAuxP;
    matrizB[setorAux].rnps[0].nos[1].profundidade = 1;
    matrizB[setorAux].rnps[0].nos[2].idNo = barraAuxSub;
    matrizB[setorAux].rnps[0].nos[2].profundidade = 2;
    matrizB[setorAux].rnps[0].nos[3].idNo = barraAuxA;
    matrizB[setorAux].rnps[0].nos[3].profundidade = 3;
    
    
    //rnp2 com origem no setor raiz da rnpA
    matrizB[setorAux].rnps[1].nos = Malloc(NORNP, (4));
    matrizB[setorAux].rnps[1].numeroNos = 4;
    matrizB[setorAux].rnps[1].idSetorOrigem = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[0].idNo;
    matrizB[setorAux].rnps[1].nos[0].idNo = barraAlimentadorA;
    matrizB[setorAux].rnps[1].nos[0].profundidade = 0;
    matrizB[setorAux].rnps[1].nos[1].idNo = barraAuxA;
    matrizB[setorAux].rnps[1].nos[1].profundidade = 1;
    matrizB[setorAux].rnps[1].nos[2].idNo = barraAuxSub;
    matrizB[setorAux].rnps[1].nos[2].profundidade = 2;
    matrizB[setorAux].rnps[1].nos[3].idNo = barraAuxP;
    matrizB[setorAux].rnps[1].nos[3].profundidade = 3;
    
    matrizB[setorAux].rnps[2].nos = Malloc(NORNP, (4));
    matrizB[setorAux].rnps[2].numeroNos = 4;
    matrizB[setorAux].rnps[2].idSetorOrigem = setorRaiz;
    matrizB[setorAux].rnps[2].nos[0].idNo = barraSubEstacao;
    matrizB[setorAux].rnps[2].nos[0].profundidade = 0;
    matrizB[setorAux].rnps[2].nos[1].idNo = barraAuxSub;
    matrizB[setorAux].rnps[2].nos[1].profundidade = 1;
    matrizB[setorAux].rnps[2].nos[2].idNo = barraAuxP;
    matrizB[setorAux].rnps[2].nos[2].profundidade = 2;
    matrizB[setorAux].rnps[2].nos[3].idNo = barraAuxA;
    matrizB[setorAux].rnps[2].nos[3].profundidade = 2;
    
    
   // matrizB[setorAlimentadorA].rnps = (RNPSETOR*)realloc(matrizB[setorAlimentadorA].rnps,(matrizB[setorAlimentadorA].numeroRNPs+1)*sizeof(RNPSETOR));
    posicaoRnpSetorA = matrizB[setorAlimentadorA].numeroRNPs;
    matrizB[setorAlimentadorA].numeroRNPs++;
    //matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].nos =  Malloc(NORNP, (2));
    matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].numeroNos = 2;
    matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].idSetorOrigem = setorAux;
    matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].nos[0].idNo = barraAuxA;
    matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].nos[0].profundidade = 0;        
    matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].nos[1].idNo = barraAlimentadorA;
    matrizB[setorAlimentadorA].rnps[posicaoRnpSetorA].nos[1].profundidade = 1;
    
   // matrizB[setorAlimentadorP].rnps = (RNPSETOR*)realloc(matrizB[setorAlimentadorP].rnps,(matrizB[setorAlimentadorP].numeroRNPs+1)*sizeof(RNPSETOR));
      posicaoRnpSetorP = matrizB[setorAlimentadorP].numeroRNPs;
    matrizB[setorAlimentadorP].numeroRNPs++;
    //matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].nos =  Malloc(NORNP, (2));
    matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].numeroNos = 2;
    matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].idSetorOrigem = setorAux;
    matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].nos[0].idNo = barraAuxP;
    matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].nos[0].profundidade = 0;        
    matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].nos[1].idNo = barraAlimentadorP;
    matrizB[setorAlimentadorP].rnps[posicaoRnpSetorP].nos[1].profundidade = 1;
    
     //copia os dados da rnp
    contador = 0;
    configuracaoAuxiliar[0].rnp[0].nos[contador].idNo = setorRaiz;
    configuracaoAuxiliar[0].rnp[0].nos[contador].profundidade = 0;
    contador++;
    configuracaoAuxiliar[0].rnp[0].nos[contador].idNo = setorAux;
    configuracaoAuxiliar[0].rnp[0].nos[contador].profundidade = 1;
    contador++;
    //copia os setores da rnpP
    for(contador2 = 0; contador2 < configuracoesParam[indiceConfiguracao].rnp[rnpP].numeroNos; contador2++)
    {
        configuracaoAuxiliar[0].rnp[0].nos[contador].idNo = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador2].idNo;
        configuracaoAuxiliar[0].rnp[0].nos[contador].profundidade = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador2].profundidade+2;
        if(configuracaoAuxiliar[0].rnp[0].nos[contador].idNo == setorBarraDe)
        {
            indiceSetorDe = contador;
        }
        contador++;
    }
    //copia os setores da rnpA
    for(contador2 = 0; contador2 < configuracoesParam[indiceConfiguracao].rnp[rnpA].numeroNos; contador2++)
    {
        configuracaoAuxiliar[0].rnp[0].nos[contador].idNo = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador2].idNo;
        configuracaoAuxiliar[0].rnp[0].nos[contador].profundidade = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador2].profundidade+2;
        if(configuracaoAuxiliar[0].rnp[0].nos[contador].idNo == setorBarraPara)
        {
            indiceSetorPara = contador;
        }
        contador++;
    }

    //inicializa dados elétricos
    configuracaoAuxiliar[0].dadosEletricos.iJusante = malloc((numeroBarrasParam + 4) * sizeof (__complex__ double));
    configuracaoAuxiliar[0].dadosEletricos.corrente = malloc((numeroBarrasParam + 4) * sizeof (__complex__ double));
    configuracaoAuxiliar[0].dadosEletricos.potencia = malloc((numeroBarrasParam + 4) * sizeof (__complex__ double));
    configuracaoAuxiliar[0].dadosEletricos.vBarra = malloc((numeroBarrasParam + 4) * sizeof (__complex__ double));
    
    VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].tensaoReal / sqrt(3.0);
    configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAlimentadorP] = VF;
    noProf[configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[0].profundidade] = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[0].idNo;
    for (contador = 1; contador < configuracoesParam[indiceConfiguracao].rnp[rnpP].numeroNos; contador++) {
        noS = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador].idNo;
        noR = noProf[configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador].profundidade - 1];
       rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        for (contador2 = 1; contador2 < rnpSetorSR.numeroNos; contador2++) {
            noN = rnpSetorSR.nos[contador2].idNo;
            __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
            __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
            //transforma em modelo monofásico...
            configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = (configuracaoAuxiliar[0].dadosEletricos.potencia[noN] * SBase) / 3.0;
            configuracaoAuxiliar[0].dadosEletricos.vBarra[noN] = VF;
            configuracaoAuxiliar[0].dadosEletricos.corrente[noN] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[noN] / configuracaoAuxiliar[0].dadosEletricos.vBarra[noN]);

        }
        //armazena o nó setor na sua profundidade
        noProf[configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador].profundidade] = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador].idNo;
    }
    //Calcula o setor do Alimentador
    if (configuracoesParam[indiceConfiguracao].rnp[rnpP].numeroNos > 1) {
        noS = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[0].idNo;
        noR = configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[1].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        for (contador2 = 1; contador2 < rnpSetorSR.numeroNos; contador2++) {
            noN = rnpSetorSR.nos[contador2].idNo;
            __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
            __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
            
            //transforma em modelo monofásico...
            configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = (configuracaoAuxiliar[0].dadosEletricos.potencia[noN] * SBase) / 3.0;
            configuracaoAuxiliar[0].dadosEletricos.vBarra[noN] = VF;
            configuracaoAuxiliar[0].dadosEletricos.corrente[noN] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[noN] / configuracaoAuxiliar[0].dadosEletricos.vBarra[noN]);

        }
    }
    VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].tensaoReal / sqrt(3.0);
    noProf[configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[0].profundidade] = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[0].idNo;
    for (contador = 1; contador < configuracoesParam[indiceConfiguracao].rnp[rnpA].numeroNos; contador++) {
        noS = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador].idNo;
        noR = noProf[configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        for (contador2 = 1; contador2 < rnpSetorSR.numeroNos; contador2++) {
            noN = rnpSetorSR.nos[contador2].idNo;
            __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
            __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
            //transforma em modelo monofásico...
            configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = (configuracaoAuxiliar[0].dadosEletricos.potencia[noN] * SBase) / 3.0;
            configuracaoAuxiliar[0].dadosEletricos.vBarra[noN] = VF;
            configuracaoAuxiliar[0].dadosEletricos.corrente[noN] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[noN] / configuracaoAuxiliar[0].dadosEletricos.vBarra[noN]);

        }
        //armazena o nó setor na sua profundidade
        noProf[configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador].profundidade] = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador].idNo;
    }
    //Calcula o setor do Alimentador
    if (configuracoesParam[indiceConfiguracao].rnp[rnpA].numeroNos > 1) {
        noS = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[0].idNo;
        noR = configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[1].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        for (contador2 = 1; contador2 < rnpSetorSR.numeroNos; contador2++) {
            noN = rnpSetorSR.nos[contador2].idNo;
            __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
            __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
            //transforma em modelo monofásico...
            configuracaoAuxiliar[0].dadosEletricos.potencia[noN] = (configuracaoAuxiliar[0].dadosEletricos.potencia[noN] * SBase) / 3.0;
            configuracaoAuxiliar[0].dadosEletricos.vBarra[noN] = VF;
            configuracaoAuxiliar[0].dadosEletricos.corrente[noN] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[noN] / configuracaoAuxiliar[0].dadosEletricos.vBarra[noN]);
        }
    }
    
    //inicializar as barras raiz com a potencia dos outros alimentadores
   for(contador = 1; contador <=numeroAlimentadores; contador++)
    {
        if ((dadosAlimentadorParam[contador].idTrafo == dadosAlimentadorParam[rnpP+1].idTrafo) && ((rnpP+1) != contador)) {
            configuracaoAuxiliar[0].dadosEletricos.potencia[barraAlimentadorP] += (configuracoesParam[indiceConfiguracao].rnp[contador-1].fitnessRNP.potenciaAlimentador/3);
        } else {
            if ((dadosAlimentadorParam[contador].idTrafo == dadosAlimentadorParam[rnpA+1].idTrafo) && ((rnpA+1) != contador)) {
                configuracaoAuxiliar[0].dadosEletricos.potencia[barraAlimentadorA] += (configuracoesParam[indiceConfiguracao].rnp[contador-1].fitnessRNP.potenciaAlimentador/3);
            }
        }        
    }
    
    
    configuracaoAuxiliar[0].dadosEletricos.corrente[barraAlimentadorP] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[barraAlimentadorP] / configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAlimentadorP]);
    configuracaoAuxiliar[0].dadosEletricos.corrente[barraAlimentadorA] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[barraAlimentadorA] / configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAlimentadorA]);

    //inicializa os dados eletricos das barras ficticias
    __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraSubEstacao] = grafoSDRParam[0].valorPQ.p;
    __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraSubEstacao] = grafoSDRParam[0].valorPQ.q;
    configuracaoAuxiliar[0].dadosEletricos.vBarra[barraSubEstacao] = VF;
    configuracaoAuxiliar[0].dadosEletricos.corrente[barraSubEstacao] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[0] / configuracaoAuxiliar[0].dadosEletricos.vBarra[0]);
    
    __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxSub] = 0;
    __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxSub] = 0;
    configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAuxSub] = VF;
    configuracaoAuxiliar[0].dadosEletricos.corrente[barraAuxSub] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxSub] / configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAuxSub]);
    
    __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxP] = 0;
    __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxP] = 0;
    configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAuxP] = VF;
    configuracaoAuxiliar[0].dadosEletricos.corrente[barraAuxP] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxP] / configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAuxP]);
    
    __real__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxA] = 0;
    __imag__ configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxA] = 0;
    configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAuxA] = VF;
    configuracaoAuxiliar[0].dadosEletricos.corrente[barraAuxA] = conj(configuracaoAuxiliar[0].dadosEletricos.potencia[barraAuxA] / configuracaoAuxiliar[0].dadosEletricos.vBarra[barraAuxA]);
    
    //atualiza matriz de corrente
    //barra 0
    //barra 0
    maximoCorrenteParam[barraSubEstacao].noAdjacentes[0].idNo = barraAuxSub;
    maximoCorrenteParam[barraSubEstacao].noAdjacentes[0].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0));
    
    //barraAuxSub
    maximoCorrenteParam[barraAuxSub].noAdjacentes[0].idNo = barraSubEstacao;
    maximoCorrenteParam[barraAuxSub].noAdjacentes[0].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0));
    maximoCorrenteParam[barraAuxSub].noAdjacentes[1].idNo = barraAuxP;
    maximoCorrenteParam[barraAuxSub].noAdjacentes[1].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0));
    maximoCorrenteParam[barraAuxSub].noAdjacentes[2].idNo = barraAuxA;
    maximoCorrenteParam[barraAuxSub].noAdjacentes[2].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sqrt(3.0));
    
    //barraAuxP
    maximoCorrenteParam[barraAuxP].noAdjacentes[0].idNo = barraAlimentadorP;
    maximoCorrenteParam[barraAuxP].noAdjacentes[0].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0));
    maximoCorrenteParam[barraAuxP].noAdjacentes[1].idNo = barraAuxSub;
    maximoCorrenteParam[barraAuxP].noAdjacentes[1].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0));
    
    //barraAuxA
    maximoCorrenteParam[barraAuxA].noAdjacentes[0].idNo = barraAlimentadorA;
    maximoCorrenteParam[barraAuxA].noAdjacentes[0].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sqrt(3.0));
    maximoCorrenteParam[barraAuxA].noAdjacentes[1].idNo = barraAuxSub;
    maximoCorrenteParam[barraAuxA].noAdjacentes[1].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sqrt(3.0));
    
    //barraAlimentadorP
    maximoCorrenteParam[barraAlimentadorP].noAdjacentes[maximoCorrenteParam[barraAlimentadorP].numeroAdjacentes].idNo = barraAuxP;
    maximoCorrenteParam[barraAlimentadorP].noAdjacentes[maximoCorrenteParam[barraAlimentadorP].numeroAdjacentes].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0));
    maximoCorrenteParam[barraAlimentadorP].numeroAdjacentes++;
    
    //barraAlimentadorA
    maximoCorrenteParam[barraAlimentadorA].noAdjacentes[maximoCorrenteParam[barraAlimentadorA].numeroAdjacentes].idNo = barraAuxA;
    maximoCorrenteParam[barraAlimentadorA].noAdjacentes[maximoCorrenteParam[barraAlimentadorA].numeroAdjacentes].valor = 999999999;//1000*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sqrt(3.0));
    maximoCorrenteParam[barraAlimentadorA].numeroAdjacentes++ ;   
    
    //atualiza matriz Z
    ZParam[barraSubEstacao].noAdjacentes[0].idNo = barraAuxSub;
    ZParam[barraSubEstacao].noAdjacentes[0].valor = 0+ 0*I;
    
    //barra 0
    ZParam[barraAuxSub].noAdjacentes[0].idNo = barraSubEstacao;
    ZParam[barraAuxSub].noAdjacentes[0].valor = 0+ 0*I;
    ZParam[barraAuxSub].noAdjacentes[1].idNo = barraAuxP;
    ZParam[barraAuxSub].noAdjacentes[1].valor = 0+ dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].impedancia*I;
    ZParam[barraAuxSub].noAdjacentes[2].idNo = barraAuxA;
    ZParam[barraAuxSub].noAdjacentes[2].valor = 0+ dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].impedancia*I;
    
    //barraAuxP
    ZParam[barraAuxP].noAdjacentes[0].idNo = barraAlimentadorP;
    ZParam[barraAuxP].noAdjacentes[0].valor = 0+0*I;
    ZParam[barraAuxP].noAdjacentes[1].idNo = barraAuxSub;
    ZParam[barraAuxP].noAdjacentes[1].valor = 0+ dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].impedancia*I;
    
    //barraAuxA
    ZParam[barraAuxA].noAdjacentes[0].idNo = barraAlimentadorA;
    ZParam[barraAuxA].noAdjacentes[0].valor = 0+0*I;
    ZParam[barraAuxA].noAdjacentes[1].idNo = barraAuxSub;
    ZParam[barraAuxA].noAdjacentes[1].valor = 0+ dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].impedancia*I;
    
    //barraAlimentadorP
    ZParam[barraAlimentadorP].noAdjacentes[ZParam[barraAlimentadorP].numeroAdjacentes].idNo = barraAuxP;
    ZParam[barraAlimentadorP].noAdjacentes[ZParam[barraAlimentadorP].numeroAdjacentes].valor = 0+0*I;
    ZParam[barraAlimentadorP].numeroAdjacentes++;
    
    //barraAlimentadorA
    ZParam[barraAlimentadorA].noAdjacentes[ZParam[barraAlimentadorA].numeroAdjacentes].idNo = barraAuxA;
    ZParam[barraAlimentadorA].noAdjacentes[ZParam[barraAlimentadorA].numeroAdjacentes].valor = 0+0*I;
    ZParam[barraAlimentadorA].numeroAdjacentes++;
    
      
    
    //cria reguladores de tensao
    dadosReguladorTemp[1].ampacidade = 999999999;//dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/pow(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal,1/3.0);
    dadosReguladorTemp[1].numeroTaps = 0;
    //dadosReguladorTemp[0].idRegulador = 0;
    dadosReguladorTemp[1].tipoRegulador = comFluxoReverso;

    dadosReguladorTemp[2].ampacidade = 999999999;//dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].capacidade/pow(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal,1/3.0);
    dadosReguladorTemp[2].numeroTaps = 0;
    //dadosReguladorTemp[1].idRegulador = 1;
    dadosReguladorTemp[2].tipoRegulador = comFluxoReverso;
    
    indiceReguladores[barraAlimentadorP] = 1;
    indiceReguladores[barraAlimentadorA] = 2;
    indiceReguladores[barraAuxP] = 1;
    indiceReguladores[barraAuxA] = 2;
    indiceReguladores[0] = 0;
    //--------------------------------------------------------------------------
    //PASSO 1: calcula fluxo atual sem loop
    // ****** Deve calcular o fluxo considerando a chave do loop como aberta
    // ****** Nesse ponto calcula todos.    
    avaliaConfiguracaoAnelCaso2(todosAlimentadores, configuracaoAuxiliar,-1, -1, 0,dadosTrafoParam, numeroTrafos, numeroAlimentadores, indiceReguladores, 
            dadosReguladorTemp, dadosAlimentadorParam, 0,matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam+3, copiaDados);
    
    //PASSO 2:
    // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
    VThevenin = configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe] - configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara];
    
    printf("\n\nVthevenin = %lf + j. %lf",__real__ VThevenin,__imag__ VThevenin );
    
    //--------------------------------------------------------------------------
    //PASSO 3: calcula impedancia thevenin
    // ******* Recebe o índice das barras "barraDe" e "barraPara" da chave que formou o loop e retorna a impedância de thévenin (Real + Imag) entre estas barras (soma de impedâncias)
    //a ligação ocorre na mesma rnp
    
    ZThevenin = calculaImpedanciaTheveninMesmoAlimentador(configuracaoAuxiliar[0], matrizB,
                    ZParam, indiceSetorDe, indiceSetorPara, 0, barraDe, barraPara);
    printf("\nZthevenin = %lf + j. %lf\n",__real__ ZThevenin,__imag__ ZThevenin);
    
    CargaDe = configuracaoAuxiliar[0].dadosEletricos.potencia[barraDe];
    CargaPara = configuracaoAuxiliar[0].dadosEletricos.potencia[barraPara];
    
    //--------------------------------------------------------------------------
    //PASSO 4: verifica convergencia
    // ******* Verifica a convergência: abs(Vthénvenin - Z_chave*Ichave) < tolerancia ---- Acredito que o Z_chave seja zero no modelo da COPEL. Este é um ponto que podemos ver com mais atenção dependendo dos dados da COPEL.
    // ******* Inicialmente Icompesação = 0;
    // ******* Se convergiu retornar para o main() pois a corrente não precisa de mais compensação.
    // ******* Se não convergiu, continua com a atualização das correntes.
    while ((cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave) > 0.01) && (itbk <= 20)) {        
        itbk++;
        printf("\nitbk = %d",itbk);
        printf("\n\nV Barra de: %lf < %lf",cabs(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe]),carg(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe])*180/3.1415);
        printf("\nV Barra para: %lf < %lf",cabs(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara]),carg(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara])*180/3.1415);
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        //--------------------------------------------------------------------------
        //PASSO 5: atualiza as correntes
        // ******* A corrente a ser atualizada será: 
        ICompensacao = (VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave)/(ZThevenin+valorZ(ZParam, barraDe, barraPara));
        IChave += ICompensacao;
        printf("\nIcomp = %lf + j. %lf\n",__real__ ICompensacao,__imag__ ICompensacao);
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
        
        // ******* A atualização deve acrescentar na "barraDe" uma corrente igual a Icompensação e na "barraPara" uma corrente igual a -Icompensação:
        // ******* 
//        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[barraDe] += ICompensacao;
//        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[barraPara] -= ICompensacao;
        
        configuracaoAuxiliar[0].dadosEletricos.potencia[barraDe] += configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe]*conj(ICompensacao);
        configuracaoAuxiliar[0].dadosEletricos.potencia[barraPara] -= configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara]*conj(ICompensacao);
        
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].corrente += -Icomensação
        // ******* 
        // ******* Só um detalhe neste ponto da atualização. Dentro da sua rotina do Fluxo (avaliaConfiguracao), você recalcula esse valor configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].corrente
        // ******* com os valores de potência da barra (configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].potencia). Então a atualização deve ser na potência para você não precisar alterar a função do fluxo de potência:
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].potencia += configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].vBarra*conj(Icomensação)
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].potencia+= configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].vBarra*conj(-Icomensação)
        // ******* 
        // ******* Retorna para o PASSO 1
        //PASSO 1: calcula fluxo atual sem loop
        // ****** Deve calcular o fluxo considerando a chave do loop como aberta
        // ****** Calcular somente para alimentadores envolvidos no loop
        avaliaConfiguracaoAnelCaso2(todosAlimentadores, configuracaoAuxiliar,-1, -1, 0,
                dadosTrafoParam, numeroTrafos, numeroAlimentadores, indiceReguladores, 
                dadosReguladorTemp, dadosAlimentadorParam, 0,matrizB, ZParam, 
                maximoCorrenteParam, numeroBarrasParam, copiaDados);
        
        //PASSO 2:
        // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
        VThevenin = configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe] - configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara];
    }
    //--------------------------------------------------------------------------
    // ******Convergência com número de iterações máximo
    if (itbk >= 20){
        printf("Fluxo de Potencia em Anel Divergiu.");
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
    }
    else{
        printf("\nitbk = %d",itbk);
        printf("\n\nV Barra de: %lf < %lf",cabs(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe]),carg(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraDe])*180/3.1415);
        printf("\nV Barra para: %lf < %lf",cabs(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara]),carg(configuracaoAuxiliar[0].dadosEletricos.vBarra[barraPara])*180/3.1415);
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
    }
    //--------------------------------------------------------------------------
    //Retorna os valores das cargas das barras da chave ao valor inicial
    configuracaoAuxiliar[0].dadosEletricos.potencia[barraDe] = CargaDe;
    configuracaoAuxiliar[0].dadosEletricos.potencia[barraPara] = CargaPara;
    
  //  imprimeDadosEletricos(configuracaoAuxiliar,0,numeroBarrasParam+3);
    //imprimeIndicadoresEletricos(configuracaoAuxiliar[0]);
            
    if(configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < configuracaoAuxiliar[0].objetivo.maiorCarregamentoRede)
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = configuracaoAuxiliar[0].objetivo.maiorCarregamentoRede;
    if(configuracoesParam[indiceConfiguracao].objetivo.menorTensao > configuracaoAuxiliar[0].objetivo.menorTensao)
        configuracoesParam[indiceConfiguracao].objetivo.menorTensao = configuracaoAuxiliar[0].objetivo.menorTensao;
    if(configuracoesParam[indiceConfiguracao].objetivo.quedaMaxima < configuracaoAuxiliar[0].objetivo.quedaMaxima)
        configuracoesParam[indiceConfiguracao].objetivo.quedaMaxima = configuracaoAuxiliar[0].objetivo.quedaMaxima;
    
    //--------------------------------------------------------------------------
    //Atualiza o objetivo de carregamento caso o carregamento da chave seja maior
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara)))
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara));
    
        
    //calculo do carregamento de trafo
    carregamentoTrafo1 = 100*cabs(configuracaoAuxiliar[0].dadosEletricos.iJusante[barraAuxP])/(1000*dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].tensaoReal*sqrt(3.0)));
    carregamentoTrafo2 = 100*cabs(configuracaoAuxiliar[0].dadosEletricos.iJusante[barraAuxA])/(1000*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].capacidade/(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sqrt(3.0)));
    if(carregamentoTrafo2 > carregamentoTrafo1)
        configuracaoAuxiliar[0].objetivo.maiorCarregamentoTrafo = carregamentoTrafo2;
    else
        configuracaoAuxiliar[0].objetivo.maiorCarregamentoTrafo = carregamentoTrafo1;
    imprimeIndicadoresEletricos(configuracaoAuxiliar[0]);
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoTrafo < configuracaoAuxiliar[0].objetivo.maiorCarregamentoTrafo)
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoTrafo = configuracaoAuxiliar[0].objetivo.maiorCarregamentoTrafo;
 
    
        
    indiceReguladores[barraAlimentadorP] = 0;
    indiceReguladores[barraAlimentadorA] = 0;

    maximoCorrenteParam[barraAlimentadorP].numeroAdjacentes--;
    maximoCorrenteParam[barraAlimentadorA].numeroAdjacentes--;
    ZParam[barraAlimentadorP].numeroAdjacentes--;
    ZParam[barraAlimentadorA].numeroAdjacentes--;
    matrizB[setorAlimentadorA].numeroRNPs--;
    matrizB[setorAlimentadorP].numeroRNPs--;
}



//----------------------------------------------------------------------------------
//
//
/**
 * Fluxo de Potência Idicativo - Caso 3: Loop entre trafos de subestações distintas 
 * @param grafoSDRParam
 * @param numeroBarrasParam
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param configuracoesParam
 * @param indiceConfiguracao
 * @param matrizB
 * @param SBase
 * @param ZParam
 * @param tapReguladores
 * @param dadosRegulador
 * @param maximoCorrenteParam
 * @param numeroTrafos
 * @param numeroAlimentadores
 * @param rnpA
 * @param rnpP
 * @param barraDe
 * @param barraPara
 * @param setorBarraDe
 * @param setorBarraPara
 */
void fluxoPotenciaAnelCaso3(GRAFO *grafoSDRParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB, double SBase,
        MATRIZCOMPLEXA *ZParam, int *tapReguladores, DADOSREGULADOR *dadosRegulador, MATRIZMAXCORRENTE * maximoCorrenteParam, int numeroTrafos, int numeroAlimentadores,
        int rnpA, int rnpP, long int barraDe, long int barraPara, long int setorBarraDe, long int setorBarraPara)
{
    __complex__ double VThevenin;
    __complex__ double ZThevenin;
    __complex__ double ICompensacao = 0 + 0*I;
    __complex__ double IChave = 0 + 0*I;
    __complex__ double CargaDe = 0 + 0*I;
    __complex__ double CargaPara = 0 + 0*I;
    int itbk = 0,SEs = 0,contador,i,j,contador2;
    double **Matriz_DIF_Angular,DIF_Angular; //DIF_Angular representa a diferença angular em rad entre as SEs envolvidas no loop
    long int indiceSetorDe;
    long int indiceSetorPara;
    BOOL todosAlimentadores = true;
    BOOL copiaDados = false;
    
    double MenorTensao, MaiorCarregamentoRede,MaiorCarregamentoCorrente,MaiorCarregamentoTrafo,PerdasResistivas,Ponderacao,QuedaMaxima;
    
    
    FILE *arquivo; /* Variável ponteiro para a manipulação do arquivo. */
    char blocoLeitura[100]; /* Variável para realizar a leitura de caracteres do arquivo. */
    char *dados; /* Variável do tipo ponteiro para char, utilizada para alterar o ponteiro da string lida do arquivo de forma a realizar o loop no sscanf. */
    
    //--------------------------------------------------------------------------
    // Abertura do arquivo dados_barras_linhas_trafos.dad para leitura e associação ao ponteiro de arquivo
    arquivo = fopen("dados_SE_diferenca_angular.dad","r");
    
    //Verifica se o arquivo foi aberto com sucesso, caso ocorra um erro retorna aviso ao usuário e encerra o programa.
    if(arquivo == NULL)
    {
        printf("Erro ao abrir arquivo dados_SE_difernca_angular.dad !!!\n");
        printf("Maxima diferenca angular adotada como 5 graus!!!\n");
        DIF_Angular = 5*3.141516/180;
    }
    else{
        fgets(blocoLeitura, 2000, arquivo);
        fgets(blocoLeitura, 2000, arquivo);
        dados = blocoLeitura;
        sscanf(dados, "%d",&SEs);
        
        Matriz_DIF_Angular = malloc ((SEs+1) * sizeof (double *));
        for (contador = 0; contador < SEs+1; contador++)
           Matriz_DIF_Angular[contador] = malloc ((SEs+1) * sizeof (double));
        
        //indices das SEs na matriz de diferenças angulares
        for (contador = 1; contador < SEs+1; contador++){
            Matriz_DIF_Angular[0][contador] = contador;
            Matriz_DIF_Angular[contador][0] = contador;
        }
        fgets(blocoLeitura, 2000, arquivo);
        //alocação da matriz de diferenças angulares
        for (contador = 0;contador<SEs+1;contador++){
            fgets(blocoLeitura, 2000, arquivo);
            
            //atribui a string lida do arquivo a variável auxiliar dados, do tipo ponteiro para permitir a alteração do ponteiro da string
            dados = blocoLeitura;
            sscanf(dados, "%d %d %lf",&i,&j,&DIF_Angular);
            Matriz_DIF_Angular[i][j] = DIF_Angular;
            Matriz_DIF_Angular[j][i] = DIF_Angular;
        }
        
        //identifica para a barra de e barra para o alimentador, respeitvo trafo e subestação que as alimentam
        long int SEbarraDe = dadosTrafoParam[dadosAlimentadorParam[rnpP+1].idTrafo].idSubEstacao;
        long int SEbarraPara = dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].idSubEstacao;
        
        //busca na matriz de diferenças a angulares as SEs envolvidas no loop e atualizam o valor de DIF_Angular
        for (contador = 0;contador<SEs+1;contador++){
            if (Matriz_DIF_Angular[contador][0] == SEbarraDe){
                for (i = 0; i < SEs+1; i++){
                    if (Matriz_DIF_Angular[0][i] == SEbarraPara)
                        DIF_Angular = Matriz_DIF_Angular[contador][i]*3.141516/180; //Diferença angular entre as SEs
                }
            }
        }
    }
    fclose(arquivo);
    
    //--------------------------------------------------------------------------
    //Encontra o índice dos setores
    for(contador2 = 0; contador2 < configuracoesParam[indiceConfiguracao].rnp[rnpP].numeroNos; contador2++)
    {
        if(configuracoesParam[indiceConfiguracao].rnp[rnpP].nos[contador2].idNo == setorBarraDe)
        {
            indiceSetorDe = contador2;
        }
    }
    for(contador2 = 0; contador2 < configuracoesParam[indiceConfiguracao].rnp[rnpA].numeroNos; contador2++)
    {
        if(configuracoesParam[indiceConfiguracao].rnp[rnpA].nos[contador2].idNo == setorBarraPara)
        {
            indiceSetorPara = contador2;
        }
    }    
    
    //--------------------------------------------------------------------------
    //
    // CALCULA O FLUXO DE POTÊNCIA PARA DIF_ANGULAR POSITIVA
    //
    //--------------------------------------------------------------------------
    //Atualiza a tensão e ângulo dos transformadores
    dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal = dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*cos(DIF_Angular) + I*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sin(DIF_Angular);
    
    inicializaDadosEletricosPorAlimentador(grafoSDRParam, configuracoesParam, indiceConfiguracao, numeroBarrasParam, SBase, dadosTrafoParam, dadosAlimentadorParam, matrizB);
    
    //PASSO 1: calcula fluxo atual sem loop
    // ****** Deve calcular o fluxo considerando a chave do loop como aberta
    // ****** Nesse ponto calcula todos.
//    avaliaConfiguracaoChaveamento(configuracoesParam, indiceConfiguracao, dadosTrafoParam, 
//            numeroTrafos, numeroAlimentadores, tapReguladores, dadosRegulador, dadosAlimentadorParam, 
//            matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, grafoSDRParam, SBase);
    
    
    avaliaConfiguracao(todosAlimentadores, configuracoesParam, -1, -1, 
            indiceConfiguracao, dadosTrafoParam, numeroTrafos, numeroAlimentadores, 
            tapReguladores, dadosRegulador, dadosAlimentadorParam, indiceConfiguracao, 
            matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, copiaDados);
    
    //--------------------------------------------------------------------------   
    //PASSO 2:
    // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
    VThevenin = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe] - configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara];
    
   // printf("\n\nVthevenin = %lf + j. %lf",__real__ VThevenin,__imag__ VThevenin );
    
    //--------------------------------------------------------------------------
    //PASSO 3: calcula impedancia thevenin
    // ******* Recebe o índice das barras "barraDe" e "barraPara" da chave que formou o loop e retorna a impedância de thévenin (Real + Imag) entre estas barras (soma de impedâncias)
    //a ligação ocorre na mesma rnp
    
    ZThevenin = calculaImpedanciaTheveninRamo(configuracoesParam[indiceConfiguracao], matrizB,
            ZParam, setorBarraDe, barraDe,rnpP,indiceSetorDe) + calculaImpedanciaTheveninRamo(configuracoesParam[indiceConfiguracao], matrizB,
            ZParam, setorBarraPara, barraPara,rnpA,indiceSetorPara);
    
    //printf("\nZthevenin = %lf + j. %lf\n",__real__ ZThevenin,__imag__ ZThevenin);
    
    CargaDe = configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe];
    CargaPara = configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara];
    
    //--------------------------------------------------------------------------
    //PASSO 4: verifica convergencia
    // ******* Verifica a convergência: abs(Vthénvenin - Z_chave*Ichave) < tolerancia ---- Acredito que o Z_chave seja zero no modelo da COPEL. Este é um ponto que podemos ver com mais atenção dependendo dos dados da COPEL.
    // ******* Inicialmente Icompesação = 0;
    // ******* Se convergiu retornar para o main() pois a corrente não precisa de mais compensação.
    // ******* Se não convergiu, continua com a atualização das correntes.
    while ((cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave) > 0.01) && (itbk <= 20)) {        
        itbk++;
        //printf("\nitbk = %d",itbk);
        //printf("\n\nV Barra de: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe])*180/3.1415);
        //printf("\nV Barra para: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara])*180/3.1415);
        //printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        //--------------------------------------------------------------------------
        //PASSO 5: atualiza as correntes
        // ******* A corrente a ser atualizada será: 
        ICompensacao = (VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave)/(ZThevenin+valorZ(ZParam, barraDe, barraPara));
        IChave += ICompensacao;
        //printf("\nIcomp = %lf + j. %lf\n",__real__ ICompensacao,__imag__ ICompensacao);
        //printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
        
        // ******* A atualização deve acrescentar na "barraDe" uma corrente igual a Icompensação e na "barraPara" uma corrente igual a -Icompensação:
        // ******* 
//        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[barraDe] += ICompensacao;
//        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[barraPara] -= ICompensacao;
        
        configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe] += configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]*conj(ICompensacao);
        configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara] -= configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]*conj(ICompensacao);
        
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].corrente += -Icomensação
        // ******* 
        // ******* Só um detalhe neste ponto da atualização. Dentro da sua rotina do Fluxo (avaliaConfiguracao), você recalcula esse valor configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].corrente
        // ******* com os valores de potência da barra (configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].potencia). Então a atualização deve ser na potência para você não precisar alterar a função do fluxo de potência:
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].potencia += configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].vBarra*conj(Icomensação)
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].potencia+= configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].vBarra*conj(-Icomensação)
        // ******* 
        // ******* Retorna para o PASSO 1
        //PASSO 1: calcula fluxo atual sem loop
        // ****** Deve calcular o fluxo considerando a chave do loop como aberta
        // ****** Calcular somente para alimentadores envolvidos no loop
        avaliaConfiguracao(todosAlimentadores, configuracoesParam, -1, -1, 
                indiceConfiguracao, dadosTrafoParam, numeroTrafos, numeroAlimentadores, 
                tapReguladores, dadosRegulador, dadosAlimentadorParam,
                indiceConfiguracao, matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, copiaDados);
        
//        avaliaConfiguracaoAnel(true, configuracoesParam, rnpP, rnpA, indiceConfiguracao, dadosTrafoParam,
//                numeroTrafos, numeroAlimentadores, tapReguladores, dadosRegulador, dadosAlimentadorParam,
//                VF, indiceConfiguracao, matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, false);
        
        //--------------------------------------------------------------------------
        //PASSO 2:
        // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
        VThevenin = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe] - configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara];
    }
    //--------------------------------------------------------------------------
    // ******Convergência com número de iterações máximo
    if (itbk >= 20){
        printf("Fluxo de Potencia em Anel Divergiu.");
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
    }
    else{
        printf("\nitbk = %d",itbk);
        printf("\n\nV Barra de: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe])*180/3.1415);
        printf("\nV Barra para: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara])*180/3.1415);
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
    }
    configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe] = CargaDe;
    configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara] = CargaPara;
    dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal = cabs(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal);
    
    //--------------------------------------------------------------------------
    //Atualiza o objetivo de carregamento caso o carregamento da chave seja maior
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara)))
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara));
    
    MenorTensao = configuracoesParam[indiceConfiguracao].objetivo.menorTensao;
    MaiorCarregamentoRede = configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede;
    MaiorCarregamentoCorrente = configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede;
    MaiorCarregamentoTrafo = configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoTrafo;
    PerdasResistivas = configuracoesParam[indiceConfiguracao].objetivo.perdasResistivas;
    Ponderacao = configuracoesParam[indiceConfiguracao].objetivo.ponderacao;
    QuedaMaxima = configuracoesParam[indiceConfiguracao].objetivo.quedaMaxima;
    
//    imprimeDadosEletricos(configuracoesParam,indiceConfiguracao,numeroBarrasParam);
    
    imprimeIndicadoresEletricos(configuracoesParam[indiceConfiguracao]);
    
    //--------------------------------------------------------------------------
    //
    // CALCULA O FLUXO DE POTÊNCIA PARA DIF_ANGULAR NEGATIVA
    //
    //--------------------------------------------------------------------------
    VThevenin = 0 + 0*I;
    ZThevenin = 0 + 0*I;
    ICompensacao = 0 + 0*I;
    IChave = 0 + 0*I;
    itbk = 0;
    
    //Atualiza a tensão e ângulo dos transformadores
    dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal = dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*cos(-DIF_Angular) + I*dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal*sin(-DIF_Angular);
    
    inicializaDadosEletricosPorAlimentador(grafoSDRParam, configuracoesParam, indiceConfiguracao, numeroBarrasParam, SBase, dadosTrafoParam, dadosAlimentadorParam, matrizB);
    
    //--------------------------------------------------------------------------
    //PASSO 1: calcula fluxo atual sem loop
    // ****** Deve calcular o fluxo considerando a chave do loop como aberta
    // ****** Nesse ponto calcula todos.
//    avaliaConfiguracaoChaveamento(configuracoesParam, indiceConfiguracao, dadosTrafoParam, 
//            numeroTrafos, numeroAlimentadores, tapReguladores, dadosRegulador, dadosAlimentadorParam, 
//            matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, grafoSDRParam, SBase);
    
    
    avaliaConfiguracao(todosAlimentadores, configuracoesParam, -1, -1, indiceConfiguracao, 
            dadosTrafoParam, numeroTrafos, numeroAlimentadores, tapReguladores, 
            dadosRegulador, dadosAlimentadorParam, indiceConfiguracao, matrizB, 
            ZParam, maximoCorrenteParam, numeroBarrasParam, copiaDados);
    
    //--------------------------------------------------------------------------
    //PASSO 2:
    // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
    VThevenin = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe] - configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara];
    
    printf("\n\nVthevenin = %lf + j. %lf",__real__ VThevenin,__imag__ VThevenin );
    
    //--------------------------------------------------------------------------
    //PASSO 3: calcula impedancia thevenin
    // ******* Recebe o índice das barras "barraDe" e "barraPara" da chave que formou o loop e retorna a impedância de thévenin (Real + Imag) entre estas barras (soma de impedâncias)
    //a ligação ocorre na mesma rnp
    
    ZThevenin = calculaImpedanciaTheveninRamo(configuracoesParam[indiceConfiguracao], matrizB,
            ZParam, setorBarraDe, barraDe,rnpP,indiceSetorDe) + calculaImpedanciaTheveninRamo(configuracoesParam[indiceConfiguracao], matrizB,
            ZParam, setorBarraPara, barraPara,rnpA,indiceSetorPara);
    
    printf("\nZthevenin = %lf + j. %lf\n",__real__ ZThevenin,__imag__ ZThevenin);
    
    CargaDe = configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe];
    CargaPara = configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara];
    
    //--------------------------------------------------------------------------
    //PASSO 4: verifica convergencia
    // ******* Verifica a convergência: abs(Vthénvenin - Z_chave*Ichave) < tolerancia ---- Acredito que o Z_chave seja zero no modelo da COPEL. Este é um ponto que podemos ver com mais atenção dependendo dos dados da COPEL.
    // ******* Inicialmente Icompesação = 0;
    // ******* Se convergiu retornar para o main() pois a corrente não precisa de mais compensação.
    // ******* Se não convergiu, continua com a atualização das correntes.
    while ((cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave) > 0.01) && (itbk <= 20)) {        
        itbk++;
        printf("\nitbk = %d",itbk);
        printf("\n\nV Barra de: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe])*180/3.1415);
        printf("\nV Barra para: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara])*180/3.1415);
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        //--------------------------------------------------------------------------
        //PASSO 5: atualiza as correntes
        // ******* A corrente a ser atualizada será: 
        ICompensacao = (VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave)/(ZThevenin+valorZ(ZParam, barraDe, barraPara));
        IChave += ICompensacao;
        printf("\nIcomp = %lf + j. %lf\n",__real__ ICompensacao,__imag__ ICompensacao);
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
        
        // ******* A atualização deve acrescentar na "barraDe" uma corrente igual a Icompensação e na "barraPara" uma corrente igual a -Icompensação:
        // ******* 
//        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[barraDe] += ICompensacao;
//        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[barraPara] -= ICompensacao;
        
        configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe] += configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]*conj(ICompensacao);
        configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara] -= configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]*conj(ICompensacao);
        
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].corrente += -Icomensação
        // ******* 
        // ******* Só um detalhe neste ponto da atualização. Dentro da sua rotina do Fluxo (avaliaConfiguracao), você recalcula esse valor configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].corrente
        // ******* com os valores de potência da barra (configuracaoParam.rnp[indiceRNP].dadosEletricos["x"].potencia). Então a atualização deve ser na potência para você não precisar alterar a função do fluxo de potência:
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].potencia += configuracaoParam.rnp[indiceRNP].dadosEletricos["barraDe"].vBarra*conj(Icomensação)
        // ******* configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].potencia+= configuracaoParam.rnp[indiceRNP].dadosEletricos["barraPara"].vBarra*conj(-Icomensação)
        // ******* 
        // ******* Retorna para o PASSO 1
        //PASSO 1: calcula fluxo atual sem loop
        // ****** Deve calcular o fluxo considerando a chave do loop como aberta
        // ****** Calcular somente para alimentadores envolvidos no loop
        avaliaConfiguracao(todosAlimentadores, configuracoesParam, -1, -1, indiceConfiguracao, dadosTrafoParam, 
                numeroTrafos, numeroAlimentadores, tapReguladores, dadosRegulador, dadosAlimentadorParam, 
                indiceConfiguracao, matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, copiaDados);
        
//        avaliaConfiguracaoAnel(true, configuracoesParam, rnpP, rnpA, indiceConfiguracao, dadosTrafoParam,
//                numeroTrafos, numeroAlimentadores, tapReguladores, dadosRegulador, dadosAlimentadorParam,
//                VF, indiceConfiguracao, matrizB, ZParam, maximoCorrenteParam, numeroBarrasParam, false);
        
        //--------------------------------------------------------------------------
        //PASSO 2:
        // ******* Calcula a tensão de thévenin com o resultado do Fluxo: Tensão Complexa Calculada da barra "barraDe" - Tensão Complexa Calculada da barra "barraPara"
        VThevenin = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe] - configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara];
    }
    //--------------------------------------------------------------------------
    //Convergência com número de iterações máximo
/*    if (itbk >= 20){
        printf("Fluxo de Potencia em Anel Divergiu.");
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
    }
    else{
        printf("\nitbk = %d",itbk);
        printf("\n\nV Barra de: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraDe])*180/3.1415);
        printf("\nV Barra para: %lf < %lf",cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara]),carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[barraPara])*180/3.1415);
        printf("\nDeltaV = %lf",cabs(VThevenin - valorZ(ZParam, barraDe, barraPara) * IChave));
        
        printf("\nIchave = %lf + j. %lf\n",__real__ IChave,__imag__ IChave);
    }*/
    configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraDe] = CargaDe;
    configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[barraPara] = CargaPara;
    dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal = cabs(dadosTrafoParam[dadosAlimentadorParam[rnpA+1].idTrafo].tensaoReal);
    
    //--------------------------------------------------------------------------
    //Atualiza o objetivo de carregamento caso o carregamento da chave seja maior
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara)))
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = 100*cabs(IChave) / (maximoCorrente(maximoCorrenteParam, barraDe, barraPara));
    
    imprimeIndicadoresEletricos(configuracoesParam[indiceConfiguracao]);
    
    //--------------------------------------------------------------------------
    //Atualiaza os objetivos da configuração com os piores resultados entre os dois casos de diferença angular
    if (configuracoesParam[indiceConfiguracao].objetivo.menorTensao > MenorTensao )
        configuracoesParam[indiceConfiguracao].objetivo.menorTensao = MenorTensao;
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < MaiorCarregamentoRede)
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = MaiorCarregamentoRede;
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede < MaiorCarregamentoCorrente)
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoRede = MaiorCarregamentoCorrente;
    if (configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoTrafo < MaiorCarregamentoTrafo)
        configuracoesParam[indiceConfiguracao].objetivo.maiorCarregamentoTrafo = MaiorCarregamentoTrafo;
    if (configuracoesParam[indiceConfiguracao].objetivo.perdasResistivas < PerdasResistivas)
        configuracoesParam[indiceConfiguracao].objetivo.perdasResistivas = PerdasResistivas;
    if (configuracoesParam[indiceConfiguracao].objetivo.ponderacao < Ponderacao)
        configuracoesParam[indiceConfiguracao].objetivo.ponderacao = Ponderacao;
    if (configuracoesParam[indiceConfiguracao].objetivo.quedaMaxima < QuedaMaxima)
        configuracoesParam[indiceConfiguracao].objetivo.quedaMaxima = QuedaMaxima;   
    
    
  //  imprimeIndicadoresEletricos(configuracoesParam[indiceConfiguracao]);
    
}
/**
 * Verifica se uma configuração em anel é factível.
 * @param configuracaoParam
 * @param idSetorP
 * @param idSetorA
 * @param rnpP
 * @param rnpA
 * @param grafoSDRParam
 * @param numeroBarrasParam
 * @param dadosAlimentadorParam
 * @param dadosTrafoParam
 * @param indiceConfiguracao
 * @param matrizB
 * @param SBase
 * @param numeroSetores
 * @param ZParam
 * @param indiceReguladores
 * @param dadosRegulador
 * @param maximoCorrenteParam
 * @param numeroTrafos
 * @param numeroAlimentadores
 * @return 
 */
BOOL verificaFactibilidadeAnel(CONFIGURACAO *configuracaoParam, long int idSetorP,
        long int idSetorA, long int rnpP, long int rnpA,
        GRAFO *grafoSDRParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        long int indiceConfiguracao, RNPSETORES *matrizB, double SBase, long int numeroSetores,
        MATRIZCOMPLEXA *ZParam, int *indiceReguladores, DADOSREGULADOR *dadosRegulador,
        MATRIZMAXCORRENTE * maximoCorrenteParam, int numeroTrafos, int numeroAlimentadores) {

    long int barraDe;
    long int barraPara;
    RNPSETOR rnpSetor = buscaRNPSetor(matrizB, idSetorP, idSetorA);
    barraPara = rnpSetor.nos[0].idNo;
    barraDe = rnpSetor.nos[1].idNo;
    //como a configuração do chaveamento possui uma RNP a mais, precisa remover essa rnp para o cálculo do fluxo em anel
    configuracaoParam[indiceConfiguracao].numeroRNP=configuracaoParam[indiceConfiguracao].numeroRNP-1;
    //caso 1 -- o loop é fechado no mesmo alimentador
    if (dadosAlimentadorParam[rnpP + 1].idTrafo == dadosAlimentadorParam[rnpA + 1].idTrafo) {
        fluxoPotenciaAnelCaso1(grafoSDRParam, numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam,
                configuracaoParam, indiceConfiguracao, matrizB, SBase,
                ZParam, indiceReguladores, dadosRegulador, maximoCorrenteParam, numeroTrafos, numeroAlimentadores,
                rnpA, rnpP, barraDe, barraPara, idSetorP, idSetorA);

    } else {
        //caso 2 --  o loop é fechado com alimentadores em trafos diferentes da mesma subestação
        if (dadosTrafoParam[dadosAlimentadorParam[rnpP + 1].idTrafo].idSubEstacao == dadosTrafoParam[dadosAlimentadorParam[rnpA + 1].idTrafo].idSubEstacao) {
            fluxoPotenciaAnelCaso2(grafoSDRParam, numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam,
                    configuracaoParam, indiceConfiguracao, matrizB, SBase, numeroSetores,
                    ZParam, indiceReguladores, dadosRegulador, maximoCorrenteParam, numeroTrafos, numeroAlimentadores,
                    rnpA, rnpP, barraDe, barraPara, idSetorP, idSetorA);
        } else //caso 3 -- o loop é fechado com alimentadores em trafos de subestações diferentes
        {
            fluxoPotenciaAnelCaso3(grafoSDRParam, numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam,
                    configuracaoParam, indiceConfiguracao, matrizB, SBase,
                    ZParam, indiceReguladores, dadosRegulador, maximoCorrenteParam, numeroTrafos, numeroAlimentadores,
                    rnpA, rnpP, barraDe, barraPara, idSetorP, idSetorA);

        }
    }
    configuracaoParam[indiceConfiguracao].numeroRNP=configuracaoParam[indiceConfiguracao].numeroRNP+1;
    return (verificaFactibilidade(configuracaoParam[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR));
}
/**
 * Método para gravar em arquivo a melhor configuração obtida.
 * @param configuracaoParam
 * @param numeroSetorFalta
 * @param seed
 * @param melhorConfiguracao
 * @param setorFalta
 */
void salvaMelhorConfiguracao(CONFIGURACAO configuracaoParam, int numeroSetorFalta, int seed, long int melhorConfiguracao, long int *setorFalta)
{
    int contador;
    char nomeArquivo[100];
    FILE *arquivo;
    
        sprintf(nomeArquivo, "MelhorConfiguracao.dad");
        arquivo = fopen(nomeArquivo, "w");
        //fprintf(arquivo, "Seed: %d\tGeracoes: %ld\tSetor_falta %ld\t", seed, maximoGeracoes, setorFalta[0]);
       // fprintf(arquivo, "Tempo %.4lf\t", tempo);
        fprintf(arquivo, "Seed: %d\n", seed);
        fprintf(arquivo, "Configuracao %ld\n", melhorConfiguracao);
        fprintf(arquivo, "Setor_falta ");
        for(contador=0;contador<numeroSetorFalta; contador++)
            fprintf(arquivo,"%ld ", setorFalta[contador]);
        fprintf(arquivo,"\n");
        fprintf(arquivo, "CarregamentoRede %.2lf\n", configuracaoParam.objetivo.maiorCarregamentoRede);
        fprintf(arquivo, "CarregamentoTrafo %.2lf\n", configuracaoParam.objetivo.maiorCarregamentoTrafo);
        fprintf(arquivo, "QuedaMaxima %.2lf\n", configuracaoParam.objetivo.quedaMaxima);
        fprintf(arquivo, "PerdasResistivas %.2lf kW\n", configuracaoParam.objetivo.perdasResistivas);
        fprintf(arquivo, "ManobrasManuais %d\n", configuracaoParam.objetivo.manobrasManuais);
        fprintf(arquivo, "ManobrasAutomaticas %d\n", configuracaoParam.objetivo.manobrasAutomaticas);
        fprintf(arquivo, "Consumidores %ld \nConsumidoresEspeciais %ld\n", configuracaoParam.objetivo.consumidoresSemFornecimento, configuracaoParam.objetivo.consumidoresEspeciaisSemFornecimento);

        fclose(arquivo);
}
/**
 * Por meio desta função é possível determinar uma solução final infactível. 
 * Está solução será que a possuir o menor número ponderado de manobras com os menores valores de carregamento de rede, 
 * de trafo e queda de tensão existentes.
 *
 * Modo como a função deve ser chamada: "melhorConfiguracao = melhorSolucaoNaoFactivelModificada(configuracoes,populacao);"
 * @param configuracoesParam
 * @param tabelasParam
 * @param numeroTabelasParam
 * @return 
 */
long int melhorSolucaoNaoFactivelModificada(CONFIGURACAO *configuracoesParam, VETORTABELA *tabelasParam, int numeroTabelasParam) {
    int numManobras = 500;
    long int idConfiguracaoSelecionada = -1;
    long int idConfiguracao;
    int contador, contadorT;
    double maximoCarregamentoRedeAdmissivel = 100, maximoCarregamentoTrafoAdmissivel = 100, maximaQuedaTensaoAdmissivel = 10;

    while (idConfiguracaoSelecionada < 0 && maximoCarregamentoTrafoAdmissivel < 200) {
        maximoCarregamentoRedeAdmissivel += 5; //Leandro: Para admimitir uma solução com sobrecarga na rede, descomentar esta linha
        maximoCarregamentoTrafoAdmissivel += 5;
        maximaQuedaTensaoAdmissivel += 1;

        for (contadorT = 0; contadorT < numeroTabelasParam; contadorT++) {
            for (contador = 0; contador < tabelasParam[contadorT].numeroIndividuos; contador++) {
                idConfiguracao = tabelasParam[contadorT].tabela[contador].idConfiguracao;
                //printf("\nTabela = %d / Ind. = %d / Man.Manuais = %d / Aut. = %d / idConfig. = %ld / Rede = %.2lf / SE = %.2lf / Q.Tensao = %.2lf", contadorT, contador, configuracoesParam[idConfiguracao].objetivo.manobrasManuais, configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoCorrente, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracao].objetivo.quedaMaxima);
                if (verificaFactibilidade(configuracoesParam[idConfiguracao], maximoCarregamentoRedeAdmissivel, maximoCarregamentoTrafoAdmissivel, maximaQuedaTensaoAdmissivel)) {
                    if (numManobras > (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                        idConfiguracaoSelecionada = idConfiguracao;
                    } else {
                        if (numManobras == (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais)) {
                            if ((int) (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede * 100) < (int) (configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede * 100)) {
                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                idConfiguracaoSelecionada = idConfiguracao;
                            } else {
                                if ((int) (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede * 100) == (int) (configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoRede * 100)) {
                                    if ((int) (configuracoesParam[idConfiguracao].objetivo.menorTensao * 100) > (int) (configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao * 100)) {
                                        numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                        idConfiguracaoSelecionada = idConfiguracao;
                                    } else {
                                        if ((int) (configuracoesParam[idConfiguracao].objetivo.menorTensao * 100) == (int) (configuracoesParam[idConfiguracaoSelecionada].objetivo.menorTensao * 100)) {
                                            if ((int) (configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo * 100) < (int) (configuracoesParam[idConfiguracaoSelecionada].objetivo.maiorCarregamentoTrafo * 100)) {
                                                numManobras = (configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracao].objetivo.manobrasManuais);
                                                idConfiguracaoSelecionada = idConfiguracao;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return idConfiguracaoSelecionada;
}
/**
 * Salva em arquivo os dados elétricos da configuração para testes com diversos casos de falta
 * @param nomeArquivo
 * @param configuracoesParam
 */
void gravaDadosEletricosVariosTestes(char *nomeArquivo, CONFIGURACAO *configuracaoParam, int seed, char *nomeSetorFalta, long int idMelhorConfiguracaoAposBE, 
        long int idMelhorConfiguracaoAposEvolutivo, double tempoBE, double tempoEvolutivo, long int melhorSolucao, double tempoChaveamento){
    
    FILE *arquivo;

    arquivo = fopen(nomeArquivo, "a");

    fprintf(arquivo, "%d;%s;%ld;", seed, nomeSetorFalta, idMelhorConfiguracaoAposBE);
	if(idMelhorConfiguracaoAposBE>0){
		if((configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.maiorCarregamentoRede <= 100) && (configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.maiorCarregamentoTrafo <= 100) &&
				(configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.quedaMaxima <= 10))
		{
			fprintf(arquivo, "BE_OK;");
		}
		else
		{
			fprintf(arquivo, "BE_N_OK;");
		}
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.maiorCarregamentoRede);
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.maiorCarregamentoTrafo);
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.quedaMaxima);
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.perdasResistivas);
		fprintf(arquivo, "%ld;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.consumidoresSemFornecimento);
		fprintf(arquivo, "%ld;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.consumidoresEspeciaisSemFornecimento);
		fprintf(arquivo, "%d;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.manobrasManuais);
		fprintf(arquivo, "%d;", configuracaoParam[idMelhorConfiguracaoAposBE].objetivo.manobrasAutomaticas);
		fprintf(arquivo, "%.6lf;", tempoBE);
		fprintf(arquivo, "%ld;", idMelhorConfiguracaoAposEvolutivo);
		if((configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.maiorCarregamentoRede <= 100) && (configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.maiorCarregamentoTrafo <= 100) &&
				(configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.quedaMaxima <= 10))
		{
			fprintf(arquivo, "Evolutivo_OK;");
		}
		else
		{
			fprintf(arquivo, "Evolutivo_N_OK;");
		}
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.maiorCarregamentoRede);
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.maiorCarregamentoTrafo);
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.quedaMaxima);
		fprintf(arquivo, "%.2lf;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.perdasResistivas);
		fprintf(arquivo, "%ld;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.consumidoresSemFornecimento);
		fprintf(arquivo, "%ld;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.consumidoresEspeciaisSemFornecimento);
		fprintf(arquivo, "%d;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.manobrasManuais);
		fprintf(arquivo, "%d;", configuracaoParam[idMelhorConfiguracaoAposEvolutivo].objetivo.manobrasAutomaticas);
		fprintf(arquivo, "%.6lf;", tempoEvolutivo);
		fprintf(arquivo, "%ld;", melhorSolucao);
		fprintf(arquivo, "%.6lf;", tempoChaveamento);
		fprintf(arquivo, "%d;", configuracaoParam[melhorSolucao].objetivo.manobrasRestabelecimento);
		fprintf(arquivo, "%d;", configuracaoParam[melhorSolucao].objetivo.manobrasAlivio);
		fprintf(arquivo, "%d;", configuracaoParam[melhorSolucao].objetivo.manobrasAposChaveamento);
		fprintf(arquivo, "%d\n", configuracaoParam[melhorSolucao].objetivo.consumidoresDesligadosEmCorteDeCarga);
	}
    fclose(arquivo);

}

/*
 * Leandro:
 * dentre outras moficações, esta nova versão da função para impressão de arquivos de saída imprime dados das barras que foram cortadas ou que ficaram fora de serviço
 */

void impremeArquivosdeSaidaV2(int seedParam, long int setorFaltaParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, long int idMelhorConfiguracaoParam, long int melhorConfiguracaoAposBEParam,  int numeroIndividuosParam, double tempoParam, int flagSolucaoFinalParam, int numeroTabelasParam){
    int contadorT,contador;
    int a, b, c;
    double maximaQuedaTensaoRelativa = 0.0;
    long int idConfiguracaoImpressao;
	char nomeArquivo1[120];
    char nomeArquivo2[120];
    char nomeArquivo3[120];
    char nomeArquivo4[120];
    char nomeArquivo5[120];
    FILE *arquivo1;
    FILE *arquivo2;
    FILE *arquivo3;
    FILE *arquivo4;
    //FILE *arquivo5;

    sprintf(nomeArquivo1, "ArquivoSaida1.txt");
    arquivo1 = fopen(nomeArquivo1, "a");
    sprintf(nomeArquivo2, "ArquivoSaida2.txt");
    arquivo2 = fopen(nomeArquivo2, "a");
    sprintf(nomeArquivo3, "ArquivoSaida3.txt");
    arquivo3 = fopen(nomeArquivo3, "a");
    sprintf(nomeArquivo4, "ArquivoSaida4.txt");
    arquivo4 = fopen(nomeArquivo4, "a");
    //sprintf(nomeArquivo5, "ArquivoSaidaUm.txt");
    //arquivo5 = fopen(nomeArquivo5, "a");

    //Arquivo 1 - Impressão dos dados de todos os indivíduos salvos em todas as tabelas
    for(contadorT=0; contadorT<numeroTabelasParam; contadorT++){
    	for (contador = 0; contador < populacaoParam[contadorT].numeroIndividuos; contador++){
    		idConfiguracaoImpressao = populacaoParam[contadorT].tabela[contador].idConfiguracao;

    		maximaQuedaTensaoRelativa = configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima*10;

    		fprintf(arquivo1, "%ld\t%d\t%d\t%d\t%ld\t%.2lf\t%d\t%.2f\t%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%ld\n", setorFaltaParam, seedParam, contadorT, contador, idConfiguracaoImpressao, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, maximaQuedaTensaoRelativa, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos );
    		//fprintf(arquivo5, "%ld\t%d\t%d\t%d\t%ld\t%.2lf\t%d\t%.2f\t%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%ld\n", setorFaltaParam, seedParam, contadorT, contador, idConfiguracaoImpressao, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoCorrente, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, maximaQuedaTensaoRelativa, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos );
    	}
    }
    fclose(arquivo1);

    //Arquivo 2 - Impressão dos dados da Melhor solução obtida pela Busca Exaustiva
	maximaQuedaTensaoRelativa = configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.quedaMaxima*10;

    fprintf(arquivo2, "%ld\t%d\t%.2lf\t%d\t%.2lf\t%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%ld\n", setorFaltaParam, seedParam, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.ponderacao, (configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.manobrasManuais + configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.manobrasAutomaticas), (configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.manobrasManuais), configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.manobrasAutomaticas, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.manobrasManuais, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.maiorCarregamentoRede, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.maiorCarregamentoTrafo, maximaQuedaTensaoRelativa, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.quedaMaxima, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.menorTensao, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.perdasResistivas, configuracoesParam[melhorConfiguracaoAposBEParam].objetivo.consumidoresEspeciaisTransferidos);
    fclose(arquivo2);

    //Arquivo 3 - Impressão dos dados da Solução definida para ser a Solução Final do problema
    if(flagSolucaoFinalParam == 0){     // SE a solução final obtida é factível
    	maximaQuedaTensaoRelativa = configuracoesParam[idMelhorConfiguracaoParam].objetivo.quedaMaxima*10;

    	fprintf(arquivo3, "%ld\t%d\t%.2lf\t%d\t%.2lf\t%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%ld\t%ld\t%ld\t%.3lf\n", setorFaltaParam, seedParam, configuracoesParam[idMelhorConfiguracaoParam].objetivo.ponderacao, (configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasAutomaticas), (configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasManuais), configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasAutomaticas, configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasManuais, configuracoesParam[idMelhorConfiguracaoParam].objetivo.maiorCarregamentoRede, configuracoesParam[idMelhorConfiguracaoParam].objetivo.maiorCarregamentoTrafo, maximaQuedaTensaoRelativa, configuracoesParam[idMelhorConfiguracaoParam].objetivo.quedaMaxima, configuracoesParam[idMelhorConfiguracaoParam].objetivo.menorTensao, configuracoesParam[idMelhorConfiguracaoParam].objetivo.perdasResistivas, configuracoesParam[idMelhorConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos, idMelhorConfiguracaoParam, (idMelhorConfiguracaoParam - numeroIndividuosParam + 1), tempoParam );
    	fclose(arquivo3);
    }
    else{
    	if(flagSolucaoFinalParam == 1){ // SE a soluao final é infactível

            maximaQuedaTensaoRelativa = configuracoesParam[idMelhorConfiguracaoParam].objetivo.quedaMaxima*10;

            fprintf(arquivo4, "%ld\t%d\t%.2lf\t%d\t%.2lf\t%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%ld\t%ld\t%ld\t%.3lf\n", setorFaltaParam, seedParam, configuracoesParam[idMelhorConfiguracaoParam].objetivo.ponderacao, (configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasAutomaticas), (configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasManuais), configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasAutomaticas, configuracoesParam[idMelhorConfiguracaoParam].objetivo.manobrasManuais, configuracoesParam[idMelhorConfiguracaoParam].objetivo.maiorCarregamentoRede, configuracoesParam[idMelhorConfiguracaoParam].objetivo.maiorCarregamentoTrafo, maximaQuedaTensaoRelativa, configuracoesParam[idMelhorConfiguracaoParam].objetivo.quedaMaxima, configuracoesParam[idMelhorConfiguracaoParam].objetivo.menorTensao, configuracoesParam[idMelhorConfiguracaoParam].objetivo.perdasResistivas, configuracoesParam[idMelhorConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos, idMelhorConfiguracaoParam, (idMelhorConfiguracaoParam - numeroIndividuosParam + 1), tempoParam );
        }
    	else			// SE não foi possível determinar uma solução final
    		fprintf(arquivo4, "%ld\t%d\t\t\t\t\t\t\t\t\t\t\t\t\t%.3lf\tNãoHáSolução\n", setorFaltaParam, seedParam, tempoParam );
    	fclose(arquivo4);
    }

}


/*Por Leandro: Determina as barras e os setores cortados em uma configuração identificada por "idConfiguracaoImpressao" e salva estas informações, respectivamente, nos VETORES "barrasCortadas" e "setoresCortados"
 */
void determinaBarrasSetoresCortados(long int idConfiguracaoImpressao, CONFIGURACAO *configuracoesParam, RNPSETORES *matrizBParam, long int *setoresCortados, long int *barrasCortadas, int *contadorSetoresCortados,int *contadorBarrasCortadas){
	long int indice, noS, noR, noN, indiceBarra;
	long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
	int idRnp;
	//char str[20];
	RNPSETOR rnpSetorSR;

	contadorSetoresCortados[0] = 0;
	contadorBarrasCortadas[0] = 0;
	for (idRnp = 0; idRnp < configuracoesParam[idConfiguracaoImpressao].numeroRNPFicticia; idRnp++) {
		indice = 0;
		noProf[configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade] = configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].idNo;
		for (indice = 1; indice < configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].numeroNos; indice++) {
			noS = configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].idNo;
			/*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
			 * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
			 * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
			if(configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade == 1) //Se isto é verdade, significa que o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
				noR = matrizBParam[noS].rnps[0].idSetorOrigem;
			else
				noR = noProf[configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade - 1];

			setoresCortados[contadorSetoresCortados[0]] = noS;
			contadorSetoresCortados[0]++;

			rnpSetorSR = buscaRNPSetor(matrizBParam, noS, noR);
			for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
				noN = rnpSetorSR.nos[indiceBarra].idNo;
				barrasCortadas[contadorBarrasCortadas[0]] = noN;
				contadorBarrasCortadas[0]++;
			}
			noProf[configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade] = configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].idNo;
		}
	}

}

/*Por Leandro: Determina as barras e os setores cortados em uma configuração identificada por "idConfiguracaoImpressao" e salva estas informações, respectivamente, nas STRINGS "barrasCortadas" e "setoresCortados"
 */
void determinaBarrasSetoresCortadosString(long int idConfiguracaoImpressao, CONFIGURACAO *configuracoesParam, RNPSETORES *matrizBParam, char *setoresCortadosString, char *barrasCortadasString){
	long int indice, noS, noR, noN, indiceBarra;
	long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
	int idRnp;
	char str[20];
	RNPSETOR rnpSetorSR;

	for (idRnp = 0; idRnp < configuracoesParam[idConfiguracaoImpressao].numeroRNPFicticia; idRnp++) {
		indice = 0;
		noProf[configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade] = configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].idNo;
		for (indice = 1; indice < configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].numeroNos; indice++) {
			noS = configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].idNo;
			/*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
			 * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
			 * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
			if(configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade == 1) //Se isto é verdade, significa que o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
				noR = matrizBParam[noS].rnps[0].idSetorOrigem;
			else
				noR = noProf[configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade - 1];

			//Zerar str
			limparString(&str);
			sprintf(str, "%ld ", noS); //Tranforma um número em string
	    	//strcat(str, " ");			   //Copia um espaço para a string
	    	strcat(setoresCortadosString, str);      //Junto a string a string mestre que contém todas as informações

			rnpSetorSR = buscaRNPSetor(matrizBParam, noS, noR);
			for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
				noN = rnpSetorSR.nos[indiceBarra].idNo;

				limparString(&str);
				sprintf(str, "%ld ", noN); //Tranforma um número em string
		    	//strcat(str, " ");			   //Copia um espaço para a string
		    	strcat(barrasCortadasString, str);      //Junto a string a string mestre que contém todas as informaçõe
			}
			noProf[configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].profundidade] = configuracoesParam[idConfiguracaoImpressao].rnpFicticia[idRnp].nos[indice].idNo;
		}
	}

}



/* Descrição: Imprime arquivos de saída após a execução do processo evolutivo. Os arquivos impressos são os seguintes:
* "ArquivoSaida0": impreme as características dos configurações obtidas pelas Heuristica que chamamos vulgarmente de Busca Exaustiva (é impresso no começo da execução do programa e não por esta função)
* "ArquivoSaida1": imprime as características da configuração pré-falta;
* "ArquivoSaida2": imprime as características da configuração conceitualmente INICIAL do problema, ie, a configuração da rede na qual os setores em falta encontram-se isolados e todos setores saudáveis afetados encontram-se desligados;
* "ArquivoSaida3": imprime os setores e as barras fora de serviço, dentre outras informações adicionais, da configuração conceitualmente inicial do problema;
* "ArquivoSaida4": imprime as caractetísticas de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo;
* "ArquivoSaida5": imprime as características exclusivamente dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo (consiste no "ArquivoSaida4"  filtrado para impressão APENAS de indivíduos factíveis);
* "ArquivoSaida6": imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo
* "ArquivoSaida7": imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, dos indivíduos FACTÍVEIS salvos em todas as tabelas ao término do processo evolutivo
* "ArquivoSaida8": imprime as características das configurações selecionadas como "Soluções Finais" se esta for INFACTÍVEL;
* "ArquivoSaida9": imprime as características das configurações selecionadas como "Soluções Finais" se esta for FACTÍVEL;
* "ArquivoSaida10":imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, configurações selecionadas como "Soluções Finais"
*
* Obs.:
* 1) São apresentados somente os setores e barras fora de serviços que eram possíveis de serem reconectados. Em outras palavras, não são apresentados os setores (e respectivas barras) em falta bem como os setores que, após a isolação da falta, não possuem chave normalmente aberta para reconectá-los a outro trecho da rede
* 2) O tempo BRUTO consiste no tempo total estimado para execução das manobras considerando TAMBÉM as manobras que feitas repeditas vezes numa mesma chave. A impressão deste tempo e a comparaço dele com o tempo real, permite estimar o tempo ganho com a exclusão de repetidas manboras numa mesma chaves, as quais se anulam
*
 *	Caractísticas impressas:
 *	"ArquivoSaida1": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º.N. Ponderado de Manobras | 30º. Menor Tensão (kV) | 31º Total de CEs transferidos | 32º Número de configurações/indivíduos gerados| 33º Tempo de Execução do PROGRAMA (segundos) | 33º
 *	"ArquivoSaida2": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º.N. Ponderado de Manobras | 30º. Menor Tensão (kV) | 31º Total de CEs transferidos | 32º Número de configurações/indivíduos gerados| 33º Tempo de Execução do PROGRAMA (segundos)
 *  "ArquivoSaida3": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *	"ArquivoSaida4": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º.N. Ponderado de Manobras | 30º. Menor Tensão (kV) | 31º Total de CEs transferidos | 32º Número de configurações/indivíduos gerados| 33º Tempo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida5": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º.N. Ponderado de Manobras | 30º. Menor Tensão (kV) | 31º Total de CEs transferidos | 32º Número de configurações/indivíduos gerados| 33º Tempo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida6": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *  "ArquivoSaida7": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *	"ArquivoSaida8": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º.N. Ponderado de Manobras | 30º. Menor Tensão (kV) | 31º Total de CEs transferidos | 32º Número de configurações/indivíduos gerados| 33º empo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida9": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º.N. Ponderado de Manobras | 30º. Menor Tensão (kV) | 31º Total de CEs transferidos | 32º Número de configurações/indivíduos gerados| 33º empo de Execução do PROGRAMA (segundos)
 * "ArquivoSaida10": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *
 *
 *  Parâmentros de entrada:
 *	@param seedParam é a semente para geração de número aleatórios que é um parametro de entrada do programa
 *	@param setorFaltaParam é o setor em falta
 *	@param numeroSetorFaltaParam é o número de faltas
 *	@param configuracoesParam é a estrutura que armazena as informações de todos os indivíduos (configurações) gerados
 *	@param populacaoParam é a estrutura de armazena as informações das tabelas de subpopulação
 *	@param idMelhorConfiguracaoParam é o identificador do indivíduo seleecionado para ser a "Solução Final"
 *	@param idPrimeiraConfiguracaoParam é o identificado da configuração conceitualmente inicial do problema
 *	@param numeroIndividuosParam é o parametro de entrada do programa que informa o número de indivíduos que devem ser gerados para formar a população inicial. Ele é utilizado nesta função para calcular a geração em que um indivíduo foi gerado
 *	@param tempoParam é o tempo de execução do programa. Não confundir com a variável tempo associada a cada indivíduo e que diz respeito ao tempo estimado necessário para implementá-lo
 *	@param flagSolucaoFinalParam é uma flag que informa que se a "Solução Final" é factível ou não
 *  @param numeroTabelasParam é o número tabelas existentes
 *  @param matrizBParam é a matriz da RNP que guarda informações sobre os nós/setores
 *  @param idConfiguracaoParam é o identificador da última configuraçõa gerada, o qual informa o número total de indivíduos/configurações gerados
 *
 *  */
void imprimeArquivosdeSaidaV4(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam,
		long int *idSolucoesFinaisParam, int numeroDesejadoSolucoesFinaisParam, long int idPrimeiraConfiguracaoParam, int numeroIndividuosParam, double tempoParam,
		int numeroTabelasParam, RNPSETORES *matrizBParam, long int idConfiguracaoParam){

    int contadorT,contador;
    int geracao;
    long int idConfiguracaoImpressao, auxiliar;
    char factibilidade[20], tipoFalta[20], setoresFalta[2000]= "", str[20]; /*,setoresCortados[5000]= "", barrasCortadas[15000]= ""*/;
    //long int setoresCortados[200], barrasCortadas[500];
    char setoresCortadosString[5000], barrasCortadasString[20000];
   //int contadorSetoresCortados, contadorBarrasCortadas;
	int sBase=0;
	leituraSBase(&sBase);

	char nomeArquivo1[120];
    char nomeArquivo2[120];
    char nomeArquivo3[120];
    char nomeArquivo4[120];
    char nomeArquivo5[120];
    char nomeArquivo6[120];
	char nomeArquivo7[120];
    char nomeArquivo8[120];
    char nomeArquivo9[120];
    char nomeArquivo10[120];
    FILE *arquivo1;
    FILE *arquivo2;
    FILE *arquivo3;
    FILE *arquivo4;
    FILE *arquivo5;
    FILE *arquivo6;
    FILE *arquivo7;
    FILE *arquivo8;
    FILE *arquivo9;
    FILE *arquivo10;
    sprintf(nomeArquivo1, "ArquivoSaida1.txt");
    arquivo1 = fopen(nomeArquivo1, "a");
    sprintf(nomeArquivo2, "ArquivoSaida2.txt");
    arquivo2 = fopen(nomeArquivo2, "a");
    sprintf(nomeArquivo3, "ArquivoSaida3.txt");
    arquivo3 = fopen(nomeArquivo3, "a");
    sprintf(nomeArquivo4, "ArquivoSaida4.txt");
    arquivo4 = fopen(nomeArquivo4, "a");
    sprintf(nomeArquivo5, "ArquivoSaida5.txt");
    arquivo5 = fopen(nomeArquivo5, "a");
    sprintf(nomeArquivo6, "ArquivoSaida6.txt");
    arquivo6 = fopen(nomeArquivo6, "a");
    sprintf(nomeArquivo7, "ArquivoSaida7.txt");
    arquivo7 = fopen(nomeArquivo7, "a");
    sprintf(nomeArquivo8, "ArquivoSaida8.txt");
    arquivo8 = fopen(nomeArquivo8, "a");
    sprintf(nomeArquivo9, "ArquivoSaida9.txt");
    arquivo9 = fopen(nomeArquivo9, "a");
    sprintf(nomeArquivo10, "ArquivoSaida10.txt");
    arquivo10 = fopen(nomeArquivo10, "a");

    //DETERMINAÇÃO DE DUAS INFORMAÇÕES A SEREM IMPRESSAS
    //Construção de uma String para armazenar o tipo da falta
    strcpy(tipoFalta, "Multiplas_Faltas");
    if(numeroSetorFaltaParam == 1)
    	strcpy(tipoFalta, "Falta_Simples");

    //Construção de uma string para armazenar todos os setores em falta
    for(contador = 0; contador < numeroSetorFaltaParam; contador++){
    	auxiliar = setorFaltaParam[contador];
    	sprintf(str, "%ld_", auxiliar);
    	strcat(setoresFalta, str);
    }

    // ARQUIVO DE SAÍDA 1 - imprime as características da configuração pré-falta;
	idConfiguracaoImpressao = 0;
	strcpy(factibilidade, "Infactivel");
	if(verificaFactibilidade(configuracoesParam[idConfiguracaoImpressao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao )) 	//Determina se a solução é Factível
		strcpy(factibilidade, "Factivel");
	fprintf(arquivo1, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t-1\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, tempoParam);
	fclose(arquivo1);

	// ARQUIVO DE SAÍDA 2 -  imprime as características da configuração conceitualmente inicial do problema, ie, a configuração da rede na qual os setores em falta encontram-se isolados e todos setores saudáveis afetados encontram-se desligados;
	if(idPrimeiraConfiguracaoParam > 0){
		idConfiguracaoImpressao = idPrimeiraConfiguracaoParam;
		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidade(configuracoesParam[idConfiguracaoImpressao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao )) 	//Determina se a solução é Factível
			strcpy(factibilidade, "Factivel");
		fprintf(arquivo2, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, tempoParam);
	}
	fclose(arquivo2);

	// ARQUIVO DE SAÍDA 3 - imprime os setores e as barras fora de serviço, dentre outras informações adicionais, configuração conceitualmente inicial do problema;
	if(idPrimeiraConfiguracaoParam > 0){
		idConfiguracaoImpressao = idPrimeiraConfiguracaoParam;
		limparString(&setoresCortadosString);
		limparString(&barrasCortadasString);
		determinaBarrasSetoresCortadosString(idConfiguracaoImpressao, configuracoesParam, matrizBParam, &setoresCortadosString, &barrasCortadasString); //Determinação das as barras e dos setores que foram cortados
		fprintf(arquivo3, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, setoresCortadosString, barrasCortadasString );
	}
	fclose(arquivo3);


    //ARQUIVOS DE SAÍDA 4, 5, 6 e 7
    for(contadorT=0; contadorT < numeroTabelasParam; contadorT++){
    	for (contador = 0; contador < populacaoParam[contadorT].numeroIndividuos; contador++){
    		idConfiguracaoImpressao = populacaoParam[contadorT].tabela[contador].idConfiguracao;

    		//Determina a Geração em que o indivíduo foi gerado
    	    if(idConfiguracaoImpressao >= numeroIndividuosParam)
    	    	geracao = idConfiguracaoImpressao - numeroIndividuosParam + 1;
    	    else
    	    	geracao = 0;

    	    //Determinação das as barras e dos setores que foram cortados
/*    	    limparString(&setoresCortadosString);
    	    limparString(&barrasCortadasString);
    	    determinaBarrasSetoresCortadosString(idConfiguracaoImpressao, configuracoesParam, matrizBParam, &setoresCortadosString, &barrasCortadasString);*/

    	    //Determina se a solução é Factível
    		strcpy(factibilidade, "Infactivel");
    		if(verificaFactibilidade(configuracoesParam[idConfiguracaoImpressao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)){
    			strcpy(factibilidade, "Factivel");
    			//ARQUIVO 5 - imprime as características exclusivamente dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo (consiste no "ArquivoSaida4"  filtrado para impressão APENAS de indivíduos factíveis);
    			fprintf(arquivo5, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao,(configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, tempoParam);
    			//ARQUIVO 7 - imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, dos indivíduos FACTÍVEIS salvos em todas as tabelas ao término do processo evolutivo
    			//fprintf(arquivo7, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, setoresCortadosString, barrasCortadasString );
    		}
    		//ARQUIVO 4 - imprime as caractetísticas de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo;
    		fprintf(arquivo4, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao,(configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, tempoParam);
    	    //ARQUIVO 6 - imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo
    		//fprintf(arquivo6, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, setoresCortadosString, barrasCortadasString );
    	}
    }
    fclose(arquivo4);
    fclose(arquivo5);
    fclose(arquivo6);
    fclose(arquivo7);

    //IMPRESSÃO DOS ARQUIVOS 8, 9 E 10
    for(contador = 0; contador < numeroDesejadoSolucoesFinaisParam; contador++){
		idConfiguracaoImpressao = idSolucoesFinaisParam[contador];

		//Determinação das as barras e dos setores que foram cortados
		limparString(&setoresCortadosString);
		limparString(&barrasCortadasString);
		determinaBarrasSetoresCortadosString(idConfiguracaoImpressao, configuracoesParam, matrizBParam, &setoresCortadosString, &barrasCortadasString);

		//Determina a Geração em que o indivíduo foi gerado
		if(idConfiguracaoImpressao >= numeroIndividuosParam)
			geracao = idConfiguracaoImpressao - numeroIndividuosParam + 1;
		else
			geracao = 0;

		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidade(configuracoesParam[idConfiguracaoImpressao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao )){
			strcpy(factibilidade, "Factivel");
			 //ARQUIVO 9 - imprime as características da configuração selecionada como "Solução Final" se esta for FACTÍVEL;
			fprintf(arquivo9, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, tempoParam);
		}
		else{
			//ARQUIVO 8 - imprime as características da configuração selecionada como "Solução Final" se esta for INFACTÍVEL;
			fprintf(arquivo8, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*3, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, tempoParam);

		}
		//ARQUIVO 10":imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, da configuração selecionada como "Solução Final"
		fprintf(arquivo10, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, setoresCortadosString, barrasCortadasString );
    }
	fclose(arquivo8);
    fclose(arquivo9);
    fclose(arquivo10);

}

/* Descrição:
 * Consiste na função "imprimeArquivosdeSaidaV4()" modficada para, ao verificar a factibilidade de um indivíduo, verificar também a factibilidade da sua sequência
 *
 * Imprime arquivos de saída após a execução do processo evolutivo. Os arquivos impressos são os seguintes:
* "ArquivoSaida0": impreme as características dos configurações obtidas pelas Heuristica que chamamos vulgarmente de Busca Exaustiva (é impresso no começo da execução do programa e não por esta função)
* "ArquivoSaida1": imprime as características da configuração pré-falta;
* "ArquivoSaida2": imprime as características da configuração conceitualmente INICIAL do problema, ie, a configuração da rede na qual os setores em falta encontram-se isolados e todos setores saudáveis afetados encontram-se desligados;
* "ArquivoSaida3": imprime os setores e as barras fora de serviço, dentre outras informações adicionais, da configuração conceitualmente inicial do problema;
* "ArquivoSaida4": imprime as caractetísticas de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo;
* "ArquivoSaida5": imprime as características exclusivamente dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo (consiste no "ArquivoSaida4"  filtrado para impressão APENAS de indivíduos factíveis);
* "ArquivoSaida6": imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo
* "ArquivoSaida7": imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, dos indivíduos FACTÍVEIS salvos em todas as tabelas ao término do processo evolutivo
* "ArquivoSaida8": imprime as características das configurações selecionadas como "Soluções Finais" se esta for INFACTÍVEL;
* "ArquivoSaida9": imprime as características das configurações selecionadas como "Soluções Finais" se esta for FACTÍVEL;
* "ArquivoSaida10":imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, configurações selecionadas como "Soluções Finais"
* "ArquivoSaida13":imprime a sequência de chaveamento dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo de maneira didática
* "ArquivoSaida14":imprime a sequência de chaveamento dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo de maneira sumarizada (para análises via software)
*
* Obs.:
* 1) São apresentados somente os setores e barras fora de serviços que eram possíveis de serem reconectados. Em outras palavras, não são apresentados os setores (e respectivas barras) em falta bem como os setores que, após a isolação da falta, não possuem chave normalmente aberta para reconectá-los a outro trecho da rede
* 2) O tempo BRUTO consiste no tempo total estimado para execução das manobras considerando TAMBÉM as manobras que feitas repeditas vezes numa mesma chave. A impressão deste tempo e a comparaço dele com o tempo real, permite estimar o tempo ganho com a exclusão de repetidas manboras numa mesma chaves, as quais se anulam
*
 *	Caractísticas impressas:
 *	"ArquivoSaida1": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º. Menor Tensão (kV) | 30º Total de CEs transferidos | 31º Número de configurações/indivíduos gerados| 32º Numero de Gerações executadas | 33º Tempo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida2": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º. Menor Tensão (kV) | 30º Total de CEs transferidos | 31º Número de configurações/indivíduos gerados| 32º Numero de Gerações executadas | 33º Tempo de Execução do PROGRAMA (segundos)
 *  "ArquivoSaida3": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *	"ArquivoSaida4": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º. Menor Tensão (kV) | 30º Total de CEs transferidos | 31º Número de configurações/indivíduos gerados| 32º Numero de Gerações executadas | 33º Tempo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida5": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º. Menor Tensão (kV) | 30º Total de CEs transferidos | 31º Número de configurações/indivíduos gerados| 32º Numero de Gerações executadas | 33º Tempo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida6": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *  "ArquivoSaida7": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *	"ArquivoSaida8": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º. Menor Tensão (kV) | 30º Total de CEs transferidos | 31º Número de configurações/indivíduos gerados| 32º Numero de Gerações executadas | 33º Tempo de Execução do PROGRAMA (segundos)
 *	"ArquivoSaida9": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras	| 10º. Manobras em Automáticas| 11º.Manobras Manuais | 12º.Energia Não Suprida TOTAL (kWh) | 13º.Energia Não Suprida Cons. P. Alta (kWh)  | 14º.Energia Não Suprida Cons. P. Intermediária (kWh) | 15º.Energia Não Suprida Cons. P. Baixa (kWh) | 16º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 17º.Potência Não Suprida TOTAL (kW) | 18º.Potência Não Suprida Cons. P. Alta (kW)  | 19º.Potência Não Suprida Cons. P. Intermediária (kW) | 20º.Potência Não Suprida Cons. P. Baixa (kW) | 21º. Potência Não Suprida Cons. Sem Prioridade (kW) | 22º.Car. de Rede (%) | 23º. Queda de Tensão (%) | 24º.Car. de Trafo(%) | 25º. Perdas Resistivas (kW) | 26º.Tempo Estimado para execução das manobras (horas) | 27º.Número de vezes que um alimentador foi avaliado prlo Fluxo Carga | 28º.Valor de Função Ponderação | 29º. Menor Tensão (kV) | 30º Total de CEs transferidos | 31º Número de configurações/indivíduos gerados| 32º Numero de Gerações executadas | 33º Tempo de Execução do PROGRAMA (segundos)
 * "ArquivoSaida10": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. "Factível" ou "Infactível" | 4º. Semente | 5º. Id. da Tabela | 6º.Posição na Tabela | 7º.Id. do indivíduo | 8º. Geração em que o indivíduo foi obtido | 9º.Total de Manobras   | 10º. Energia Não Suprida TOTAL (kWh) | 11º.Conjunto de Setores Cortados (ver obs. 1) | 12º. Separador  | 13º.Conjunto de Barras Cortadas (ver obs. 1)
 *
 * "ArquivoSaida14": 1º.Tipo da Falta ("Simples" ou "Múltiplas") | 2º.Setores em Falta | 3º. FACTBILIDADE da CONFIGURAÇÃO INTERMEDIARIA | 4º. Semente | 5º Id. do Indivíduo | 6º ABRIR (ou FECHAR) | 7º FECHAR (ou ABRIR) | 8ª.Energia Não Suprida Cons. P. Alta (kWh)  | 9º.Energia Não Suprida Cons. P. Intermediária (kWh) | 10º.Energia Não Suprida Cons. P. Baixa (kWh) | 11º. Energia Não Suprida Cons. Sem Prioridade (kWh) | 12º.Manobras Manuais  | 13º. Manobras em Automáticas | 14º.Potência Não Suprida Cons. P. Alta (kW)  | 15º.Potência Não Suprida Cons. P. Intermediária (kW) | 16º.Potência Não Suprida Cons. P. Baixa (kW) | 17º. Potência Não Suprida Cons. Sem Prioridade (kW) | 18º.Car. de Rede (%) | 19º. Queda de Tensão (%) | 20º.Car. de Trafo(%)
 * PS.: Os parâmentros 6º e 7º serão ABRIR e FECHAR se a seq. escolhida para a manobras de operações de transf. entre alimentadores for "Abre-Fecha". Caso, contrário, os parâmentros 6º e 7º serão FECHAR e ABRIR, pois a seq. escolhida para a manobras de operações de transf. entre alimentadores terá sido "Fecha-Abre
 *
 *
 *
 *  Parâmentros de entrada:
 *	@param seedParam é a semente para geração de número aleatórios que é um parametro de entrada do programa
 *	@param setorFaltaParam é o setor em falta
 *	@param numeroSetorFaltaParam é o número de faltas
 *	@param configuracoesParam é a estrutura que armazena as informações de todos os indivíduos (configurações) gerados
 *	@param populacaoParam é a estrutura de armazena as informações das tabelas de subpopulação
 *	@param idMelhorConfiguracaoParam é o identificador do indivíduo seleecionado para ser a "Solução Final"
 *	@param idPrimeiraConfiguracaoParam é o identificado da configuração conceitualmente inicial do problema
 *	@param numeroIndividuosParam é o parametro de entrada do programa que informa o número de indivíduos que devem ser gerados para formar a população inicial. Ele é utilizado nesta função para calcular a geração em que um indivíduo foi gerado
 *	@param tempoParam é o tempo de execução do programa. Não confundir com a variável tempo associada a cada indivíduo e que diz respeito ao tempo estimado necessário para implementá-lo
 *	@param flagSolucaoFinalParam é uma flag que informa que se a "Solução Final" é factível ou não
 *  @param numeroTabelasParam é o número tabelas existentes
 *  @param matrizBParam é a matriz da RNP que guarda informações sobre os nós/setores
 *  @param idConfiguracaoParam é o identificador da última configuraçõa gerada, o qual informa o número total de indivíduos/configurações gerados
 *
 *  */
void imprimeArquivosdeSaidaV5(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam,
		long int *idSolucoesFinaisParam, int numeroDesejadoSolucoesFinaisParam, long int idPrimeiraConfiguracaoParam, int numeroIndividuosPopulacaoInicialParam, double tempoParam,
		int numeroTabelasParam, RNPSETORES *matrizBParam, long int idConfiguracaoParam, VETORPI *vetorPiParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, long int geracoesExecutadasParam, int numeroSolucoesFinaisObtidasParam){

    int contadorT,contador;
    int geracao;
    long int idConfiguracaoImpressao, auxiliar;
    char factibilidade[20], tipoFalta[20], setoresFalta[2000]= "", str[20]; /*,setoresCortados[5000]= "", barrasCortadas[15000]= ""*/;
    //long int setoresCortados[200], barrasCortadas[500];
    char setoresCortadosString[5000], barrasCortadasString[20000];
   //int contadorSetoresCortados, contadorBarrasCortadas;
	int sBase=0;
	leituraSBase(&sBase);

	char nomeArquivo1[120];
    char nomeArquivo2[120];
    char nomeArquivo3[120];
    char nomeArquivo4[120];
    char nomeArquivo5[120];
    char nomeArquivo6[120];
	char nomeArquivo7[120];
    char nomeArquivo8[120];
    char nomeArquivo9[120];
    char nomeArquivo10[120];
    char nomeArquivo13[120];
    char nomeArquivo14[120];
    FILE *arquivo1;
    FILE *arquivo2;
    FILE *arquivo3;
    FILE *arquivo4;
    FILE *arquivo5;
    FILE *arquivo6;
    FILE *arquivo7;
    FILE *arquivo8;
    FILE *arquivo9;
    FILE *arquivo10;
    FILE *arquivo13;
    FILE *arquivo14;
    sprintf(nomeArquivo1, "ArquivoSaida1.txt");
    arquivo1 = fopen(nomeArquivo1, "a");
    sprintf(nomeArquivo2, "ArquivoSaida2.txt");
    arquivo2 = fopen(nomeArquivo2, "a");
    sprintf(nomeArquivo3, "ArquivoSaida3.txt");
    arquivo3 = fopen(nomeArquivo3, "a");
    sprintf(nomeArquivo4, "ArquivoSaida4.txt");
    arquivo4 = fopen(nomeArquivo4, "a");
    sprintf(nomeArquivo5, "ArquivoSaida5.txt");
    arquivo5 = fopen(nomeArquivo5, "a");
    sprintf(nomeArquivo6, "ArquivoSaida6.txt");
    arquivo6 = fopen(nomeArquivo6, "a");
    sprintf(nomeArquivo7, "ArquivoSaida7.txt");
    arquivo7 = fopen(nomeArquivo7, "a");
    sprintf(nomeArquivo8, "ArquivoSaida8.txt");
    arquivo8 = fopen(nomeArquivo8, "a");
    sprintf(nomeArquivo9, "ArquivoSaida9.txt");
    arquivo9 = fopen(nomeArquivo9, "a");
    sprintf(nomeArquivo10, "ArquivoSaida10.txt");
    arquivo10 = fopen(nomeArquivo10, "a");
    sprintf(nomeArquivo13, "ArquivoSaida13.txt");
    arquivo13 = fopen(nomeArquivo13, "a");
    sprintf(nomeArquivo14, "ArquivoSaida14.txt");
    arquivo14 = fopen(nomeArquivo14, "a");

//	if(sequenciaManobrasAlivioParam == abreFecha)
//		fprintf(arquivo14, "TIPO_FALTA\tSETORES_FALTA\tFACTIBILIDADE\tSEED\tID_IND\tABRIR\tFECHAR\tENS_PA\tENS_PI\tENS_PB\tENS_SP\tMAN_M\tMAN_R\tPNS_PA\tPNS_PI\tPNS_PB\tPNS_SP\tCAR_REDE\tQUEDA_T\tCAR_SE\n");
//	else
//		fprintf(arquivo14, "TIPO_FALTA\tSETORES_FALTA\tFACTIBILIDADE\tSETOR_FALTA\tSEED\tID_IND\tFECHAR\tABRIR\tENS_PA\tENS_PI\tENS_PB\tENS_SP\tMAN_M\tMAN_R\tPNS_PA\tPNS_PI\tPNS_PB\tPNS_SP\tCAR_REDE\tQUEDA_T\tCAR_SE\n");


    //DETERMINAÇÃO DE DUAS INFORMAÇÕES A SEREM IMPRESSAS
    //Construção de uma String para armazenar o tipo da falta
    strcpy(tipoFalta, "Multiplas_Faltas");
    if(numeroSetorFaltaParam == 1)
    	strcpy(tipoFalta, "Falta_Simples");

    //Construção de uma string para armazenar todos os setores em falta
    for(contador = 0; contador < numeroSetorFaltaParam; contador++){
    	auxiliar = setorFaltaParam[contador];
    	sprintf(str, "%ld_", auxiliar);
    	strcat(setoresFalta, str);
    }

    // ARQUIVO DE SAÍDA 1 - imprime as características da configuração pré-falta;
	idConfiguracaoImpressao = 0;
	strcpy(factibilidade, "Infactivel");
	if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoImpressao)) 	//Determina se a solução é Factível
		strcpy(factibilidade, "Factivel");
	fprintf(arquivo1, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t-1\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%ld\t%.6f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*1, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, geracoesExecutadasParam, tempoParam);
	fclose(arquivo1);

	// ARQUIVO DE SAÍDA 2 -  imprime as características da configuração conceitualmente inicial do problema, ie, a configuração da rede na qual os setores em falta encontram-se isolados e todos setores saudáveis afetados encontram-se desligados;
	if(idPrimeiraConfiguracaoParam > 0){
		idConfiguracaoImpressao = idPrimeiraConfiguracaoParam;
		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoImpressao)) 	//Determina se a solução é Factível
			strcpy(factibilidade, "Factivel");
		fprintf(arquivo2, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%ld\t%.6f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*1, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, geracoesExecutadasParam, tempoParam);
	}
	fclose(arquivo2);

	// ARQUIVO DE SAÍDA 3 - imprime os setores e as barras fora de serviço, dentre outras informações adicionais, configuração conceitualmente inicial do problema;
	if(idPrimeiraConfiguracaoParam > 0){
		idConfiguracaoImpressao = idPrimeiraConfiguracaoParam;
		limparString(&setoresCortadosString);
		limparString(&barrasCortadasString);
		determinaBarrasSetoresCortadosString(idConfiguracaoImpressao, configuracoesParam, matrizBParam, &setoresCortadosString, &barrasCortadasString); //Determinação das as barras e dos setores que foram cortados
		fprintf(arquivo3, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, setoresCortadosString, barrasCortadasString );
	}
	fclose(arquivo3);


    //ARQUIVOS DE SAÍDA 4, 5, 6 e 7
    for(contadorT=0; contadorT < numeroTabelasParam; contadorT++){
    	for (contador = 0; contador < populacaoParam[contadorT].numeroIndividuos; contador++){
    		idConfiguracaoImpressao = populacaoParam[contadorT].tabela[contador].idConfiguracao;

    		//Determina a Geração em que o indivíduo foi gerado
    	    if(idConfiguracaoImpressao >= numeroIndividuosPopulacaoInicialParam)
    	    	geracao = idConfiguracaoImpressao - numeroIndividuosPopulacaoInicialParam + 1;
    	    else
    	    	geracao = 0;
    	    geracao = -1; //Neste AEMO_DOC_VF a quantidade de indivíduos gerados em cada geração é variável e, portanto, não é possível determinar a posteriori em qual geração um determinado indivíduo foi gerado (o que é possível em outros AEMOs, como na MRAN)

    	    //Determinação das as barras e dos setores que foram cortados
    	    limparString(&setoresCortadosString);
    	    limparString(&barrasCortadasString);
    	    determinaBarrasSetoresCortadosString(idConfiguracaoImpressao, configuracoesParam, matrizBParam, &setoresCortadosString, &barrasCortadasString);

    	    //Determina se a solução é Factível
    		strcpy(factibilidade, "Infactivel");
    		if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoImpressao)){
    			strcpy(factibilidade, "Factivel");
    			//ARQUIVO 5 - imprime as características exclusivamente dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo (consiste no "ArquivoSaida4"  filtrado para impressão APENAS de indivíduos factíveis);
    			fprintf(arquivo5, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%ld\t%.6f\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao,(configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*1, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam, geracoesExecutadasParam, tempoParam);
    			//ARQUIVO 7 - imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, dos indivíduos FACTÍVEIS salvos em todas as tabelas ao término do processo evolutivo
    			fprintf(arquivo7, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, setoresCortadosString, barrasCortadasString );
    			//ARQUIVO 13 - imprime a sequência de chaveamento dos indivíduos FACTÍVEIS salvos nas tabelas ao término do processo evolutivo
    			imprimeSequenciaChaveamentoIndividuo(seedParam, setorFaltaParam, numeroSetorFaltaParam, idConfiguracaoImpressao, vetorPiParam, configuracoesParam, sequenciaManobrasAlivioParam, nomeArquivo13);

    			imprimeSequenciaChaveamentoIndividuoSumarizada(seedParam, tipoFalta, setoresFalta, idConfiguracaoImpressao, vetorPiParam, configuracoesParam, sequenciaManobrasAlivioParam, nomeArquivo14);

    		}
    		//ARQUIVO 4 - imprime as caractetísticas de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo;
    		fprintf(arquivo4, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%ld\t%.6f\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao,(configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*1, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam,  geracoesExecutadasParam, tempoParam);
    	    //ARQUIVO 6 - imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, de TODOS os indivíduos salvos nas tabelas ao término do processo evolutivo
			fprintf(arquivo6, "%s\t%s\t%s\t%d\t%d\t%d\t%ld\t%d\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, contadorT, contador, idConfiguracaoImpressao, geracao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, setoresCortadosString, barrasCortadasString );
    	}
    }
    fclose(arquivo4);
    fclose(arquivo5);
    fclose(arquivo6);
    fclose(arquivo7);
    fclose(arquivo14);

    //IMPRESSÃO DOS ARQUIVOS 8, 9 E 10
    for(contador = 0; contador < numeroSolucoesFinaisObtidasParam; contador++){
		idConfiguracaoImpressao = idSolucoesFinaisParam[contador];

		//Determinação das as barras e dos setores que foram cortados
		limparString(&setoresCortadosString);
		limparString(&barrasCortadasString);
		determinaBarrasSetoresCortadosString(idConfiguracaoImpressao, configuracoesParam, matrizBParam, &setoresCortadosString, &barrasCortadasString);

		//Determina a Geração em que o indivíduo foi gerado
		if(idConfiguracaoImpressao >= numeroIndividuosPopulacaoInicialParam)
			geracao = idConfiguracaoImpressao - numeroIndividuosPopulacaoInicialParam + 1;
		else
			geracao = 0;
		geracao = -1; //Neste AEMO_DOC_VF a quantidade de indivíduos gerados em cada geração é variável e, portanto, não é possível determinar a posteriori em qual geração um determinado indivíduo foi gerado (o que é possível em outros AEMOs, como na MRAN)

		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoImpressao)){
			strcpy(factibilidade, "Factivel");
			 //ARQUIVO 9 - imprime as características da configuração selecionada como "Solução Final" se esta for FACTÍVEL;
			fprintf(arquivo9, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%ld\t%.6f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*1, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam,  geracoesExecutadasParam, tempoParam);
		}
		else{
			//ARQUIVO 8 - imprime as características da configuração selecionada como "Solução Final" se esta for INFACTÍVEL;
			fprintf(arquivo8, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\t%.2f\t%ld\t%ld\t%ld\t%.6f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa/sBase)*1, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade/sBase)*1, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, numeroExecucoesFluxoCarga, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, idConfiguracaoParam,  geracoesExecutadasParam,tempoParam);

		}
		//ARQUIVO 10":imprime os setores e as barras deixadas fora de serviço, dentre outras informações adicionais, da configuração selecionada como "Solução Final"
		fprintf(arquivo10, "%s\t%s\t%s\t%d\t-1\t-1\t%ld\t0\t%d\t%.2lf\t|\tSetores_Cortados:\t%s\t|\tBarras_Cortadas:\t%s\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfiguracaoImpressao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*1, setoresCortadosString, barrasCortadasString );
    }
	fclose(arquivo8);
    fclose(arquivo9);
    fclose(arquivo10);

}

/* Por Leandro:
 * Método desenvolvido para impressão das soluções na fronteira de Pareto.
 *
 *@param fronteiraParetoParam
 *@return
 */
void mostraFronteiraPareto(FRONTEIRAS fronteiraParetoParam, CONFIGURACAO *configuracoesParam){
	int indice, indiceMenorValorEnergiaNaoSuprida;
	long int idConfiguracao;
	NIVEISDEPRIORIDADEATENDIMENTO menorEnergiaNaoSuprida;

	indice=0;
	idConfiguracao = fronteiraParetoParam.individuos[indice].idConfiguracao;
	menorEnergiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
	menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
	menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
	menorEnergiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;
	indiceMenorValorEnergiaNaoSuprida = indice; //Índice em "fronteiraParetoParam"

	for(indice = 1; indice < fronteiraParetoParam.numeroIndividuos; indice++){
		idConfiguracao = fronteiraParetoParam.individuos[indice].idConfiguracao;
		if( (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)  < floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)  < floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)  < floor(menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa)) ) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)  < floor(menorEnergiaNaoSuprida.consumidoresSemPrioridade))) ||
		   ((floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) == floor(menorEnergiaNaoSuprida.consumidoresSemPrioridade)) && (idConfiguracao < fronteiraParetoParam.individuos[indiceMenorValorEnergiaNaoSuprida].idConfiguracao) ) //A tendencia é a de que indivíduos que forma gerados primeiro, tenham menos chaves repetidas
		){
			menorEnergiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
			menorEnergiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
			menorEnergiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
			menorEnergiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idConfiguracao].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;
			indiceMenorValorEnergiaNaoSuprida = indice; //Índice em "fronteiraParetoParam"
		}
	}


}



/* @brief Por Leandro:
 * Descrição: Imprime os indivíduos obtidos pela Busca Exaustiva:
 *
 *	Caractísticas impressas:
 *	1º.Setores em Falta | 2º. Ponderação   | 3º. Manobras Totais  | 4º.Manobras Ponderadas | 5º.Manobras Automáticas | 6º.Manobras Manuais   | 7º.Carregamento de Rede(%)  | 8º. Carregamento de Trafo(%)| 9º.Queda de tensão relativa(%) | 10º.Queda de Tensão(%) | 11º.Menor Tensão (kV) | 12º.Perdas Resistivas (kW)       | 13º. n. Total de CEs Transferidos | 14º.Potência Ativa Não Suprida-Por fase (kW) | 15º. Tempo Estimado para execução das manobras| 16º.Geração em que o Id foi obtido |
 *
 *	@param seedParam é a semente para geração de número aleatórios que é um parametro de entrada do programa
 *	@param setorFaltaParam é o setor em falta
 *	@param configuracoesParam é a estrutura que armazena as informações de todos os indivíduos (configurações) gerados
 *	@param idConfiguracaoInicialParam é o identificador da configuração Inicial
 *	@param numeroConfiguracoesParam é o número total de configurações obtidas pela busca exaustiva
 *	@param tempoParam é o tempo de execução do programa. Não confundir com a variável tempo associada a cada indivíduo e que diz respeito ao tempo estimado necessário para implementá-lo
 *	@param VFParam é a tensão nominal
 *  */
void imprimeIndividuosBuscaExaustiva(long int *setorFaltaParam, int numeroSetorFaltaParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoInicialParam, long int numeroConfiguracoesParam, double tempoParam, VETORPI *vetorPiParam){
	char nomeArquivo[120];
    FILE *arquivo;
    long int idConfiguracaoImpressao;
    int contador, auxiliar;
    double maximaQuedaTensaoRelativa = 0.0;
    char factibilidade[20], tipoFalta[20], setorFalta[1000]= "", str[20];
	int sBase;
	leituraSBase(&sBase);

    sprintf(nomeArquivo, "ArquivoSaida0.txt");
    arquivo = fopen(nomeArquivo, "a");

    strcpy(tipoFalta, "Multiplas_Faltas");
    if(numeroSetorFaltaParam == 1)
    	strcpy(tipoFalta, "Falta_Simples");

    for(contador = 0; contador < numeroSetorFaltaParam; contador++){
    	auxiliar = setorFaltaParam[contador];
    	sprintf(str, "%d_", auxiliar);
    	strcat(setorFalta, str);
    }

    for (contador = idConfiguracaoInicialParam + 1; contador < numeroConfiguracoesParam; contador++) {
    	idConfiguracaoImpressao = configuracoesParam[contador].idConfiguracao;
    	maximaQuedaTensaoRelativa = configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima*10;
		//maximaQuedaTensaoRelativa = ((VFParam - configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao)/(VFParam*maxQuedaTensao/100))*100;

		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoImpressao))
			strcpy(factibilidade, "Factivel");

	    fprintf(arquivo, "%s\t%.2lf\t%d\t%.2lf\t%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%ld\t%.2lf\t%.2lf\t%s\t%s\n", setorFalta, configuracoesParam[idConfiguracaoImpressao].objetivo.ponderacao, (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas), (configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais), configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoImpressao].objetivo.manobrasManuais, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoImpressao].objetivo.maiorCarregamentoTrafo, maximaQuedaTensaoRelativa, configuracoesParam[idConfiguracaoImpressao].objetivo.quedaMaxima, configuracoesParam[idConfiguracaoImpressao].objetivo.menorTensao, configuracoesParam[idConfiguracaoImpressao].objetivo.perdasResistivas, configuracoesParam[idConfiguracaoImpressao].objetivo.consumidoresEspeciaisTransferidos, (configuracoesParam[idConfiguracaoImpressao].objetivo.potenciaTotalNaoSuprida/sBase)*3, configuracoesParam[idConfiguracaoImpressao].objetivo.tempo, factibilidade, tipoFalta);
    }

    fclose(arquivo);
}



/* Por Leandro:
 * Descrição: esta função calcula a somatória de potência ativa em todos os setores que formam a
 * subárvore que tem o nó "noRaizParam" como raiz na configuração "idConfiguracaoParam"
 *
 * @param configuracoesParam é a estrutura que carrega as informaçẽos de todos as configurações geradas
 * @param idConfiguracaoParam é a configuração na qual desaja-se calcular o somatório de potência ativa
 * @param noRaizParam é o nó raiz da subárvore para a qual será determinado o somatório de potência ativa
 * @param matrizB
 * @param matrizPiParam
 * @param vetorPiParam
 * @return potenciaAtivaNaoSuprida é o somatório de potência ativa calculado
 */
double calculaSomatorioPotenciaAtivaSubarvore(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, GRAFO *grafoSDRParam){
	long int noS, noR, noN, indiceBarra, idNoRaiz;
	long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
	double potenciaAtivaNaoSuprida = 0, indiceL;
	int indice, idRnp, indiceColuna, idRnpNoRaiz, indice2;
	RNPSETOR rnpSetorSR;
	RNP rnp;

	indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noRaizParam, idConfiguracaoParam);

	idNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].posicao;
	idRnpNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP;

	if(matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se o "noRaizParam" pertence a uma RNP Fictícia
		rnp = configuracoesParam[idConfiguracaoParam].rnpFicticia[idRnpNoRaiz - configuracoesParam[idConfiguracaoParam].numeroRNP];
	else
		rnp = configuracoesParam[idConfiguracaoParam].rnp[idRnpNoRaiz];


	indiceL = limiteSubArvore(rnp, idNoRaiz);
	indice = 0;
	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
    for (indice = 1; indice <= indiceL; indice++) {

    	if(indice == idNoRaiz || (indice > idNoRaiz && rnp.nos[indice].profundidade > rnp.nos[idNoRaiz].profundidade)){

    		noS = rnp.nos[indice].idNo;

    		/*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
    		 * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
    		 * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
    		indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noS, idConfiguracaoParam);
    		if(rnp.nos[indice].profundidade == 1 && matrizPiParam[noS].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se isto é verdade, significa que o noS está numa árvore fictícia e o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
    			noR = matrizB[noS].rnps[0].idSetorOrigem;
    		else
    			noR = noProf[rnp.nos[indice].profundidade - 1];

    		rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
    		for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
    			noN = rnpSetorSR.nos[indiceBarra].idNo;
				potenciaAtivaNaoSuprida = potenciaAtivaNaoSuprida + grafoSDRParam[noN].valorPQ.p;// creal(configuracoesParam[0].dadosEletricos.potencia[noN]);
    		}

    	}

    	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
	}

    return(potenciaAtivaNaoSuprida);
}

/* Por Leandro:
 * Descrição: esta função calcula a somatória de potência ativa, por nível de prioridade,
 * em todos os setores que formam a subárvore que tem o nó "noRaizParam" como raiz na
 * configuração "idConfiguracaoParam"
 *
 * @param configuracoesParam é a estrutura que carrega as informaçẽos de todos as configurações geradas
 * @param idConfiguracaoParam é a configuração que será percorrida para o calculo do somatório de potência ativa
 * @param noRaizParam é o nó raiz da subárvore para a qual será determinado o somatório de potência ativa
 * @param matrizB
 * @param matrizPiParam
 * @param vetorPiParam
 * @return potenciaAtivaNaoSuprida é o somatório de potência ativa calculado
 */
NIVEISDEPRIORIDADEATENDIMENTO calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, GRAFO *grafoSDRParam){
	long int noS, noR, noN, indiceBarra, idNoRaiz;
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = malloc(200 * sizeof(long int));
	double indiceL;
	int indice, idRnpNoRaiz, indiceColuna;
	RNPSETOR rnpSetorSR;
	RNP rnp;

	NIVEISDEPRIORIDADEATENDIMENTO potenciaAtivaPorAlimentador;

	potenciaAtivaPorAlimentador.consumidoresSemPrioridade = 0;
	potenciaAtivaPorAlimentador.consumidoresPrioridadeBaixa = 0;
	potenciaAtivaPorAlimentador.consumidoresPrioridadeIntermediaria = 0;
	potenciaAtivaPorAlimentador.consumidoresPrioridadeAlta = 0;

	indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noRaizParam, idConfiguracaoParam);

	idNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].posicao;
	idRnpNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP;

	if(matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se o "noRaizParam" pertence a uma RNP Fictícia
		rnp = configuracoesParam[idConfiguracaoParam].rnpFicticia[idRnpNoRaiz - configuracoesParam[idConfiguracaoParam].numeroRNP];
	else
		rnp = configuracoesParam[idConfiguracaoParam].rnp[idRnpNoRaiz];


	indiceL = limiteSubArvore(rnp, idNoRaiz);
	indice = 0;
	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
    for (indice = 1; indice <= indiceL; indice++) {

    	if(indice == idNoRaiz || (indice > idNoRaiz && rnp.nos[indice].profundidade > rnp.nos[idNoRaiz].profundidade)){

    		noS = rnp.nos[indice].idNo;

    		/*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
    		 * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
    		 * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
    		indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noS, idConfiguracaoParam);
    		if(rnp.nos[indice].profundidade == 1 && matrizPiParam[noS].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se isto é verdade, significa que o noS está numa árvore fictícia e o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
    			noR = matrizB[noS].rnps[0].idSetorOrigem;
    		else
    			noR = noProf[rnp.nos[indice].profundidade - 1];

    		rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
    		for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
    			noN = rnpSetorSR.nos[indiceBarra].idNo;
				//potenciaAtivaNaoSuprida = potenciaAtivaNaoSuprida + grafoSDRParam[noN].valorPQ.p;//creal(configuracoesParam[0].dadosEletricos.potencia[noN]);

                switch(grafoSDRParam[noN].priorizacoes.prioridadeConsumidor){
                case semPrioridade:
                	potenciaAtivaPorAlimentador.consumidoresSemPrioridade = potenciaAtivaPorAlimentador.consumidoresSemPrioridade + grafoSDRParam[noN].valorPQ.p;
                	break;

                case prioridadeBaixa:
                	potenciaAtivaPorAlimentador.consumidoresPrioridadeBaixa = potenciaAtivaPorAlimentador.consumidoresPrioridadeBaixa + grafoSDRParam[noN].valorPQ.p;
                	break;

                case prioridadeIntermediaria:
                	potenciaAtivaPorAlimentador.consumidoresPrioridadeIntermediaria = potenciaAtivaPorAlimentador.consumidoresPrioridadeIntermediaria + grafoSDRParam[noN].valorPQ.p;
                	break;

                case prioridadeAlta:
                	potenciaAtivaPorAlimentador.consumidoresPrioridadeAlta = potenciaAtivaPorAlimentador.consumidoresPrioridadeAlta + grafoSDRParam[noN].valorPQ.p;
                	break;
                }
    		}
    	}
    	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
	}

    free(noProf); noProf = NULL;
    return(potenciaAtivaPorAlimentador);
}

/* Por Leandro:
 * Descrição: esta função calcula a somatória de potência ativa, por nível de prioridade,
 * em todos os setores que formam a subárvore que tem o nó "noRaizParam" como raiz na
 * configuração "idConfiguracaoParam".
 * A principal diferença desta função em relação a função "calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade()" está no
 * modo como a subárvore é percorrida. Nesta função isto é feito de maneira mais simples.
 *
 * @param configuracoesParam é a estrutura que carrega as informaçẽos de todos as configurações geradas
 * @param idConfiguracaoParam é a configuração que será percorrida para o calculo do somatório de potência ativa
 * @param noRaizParam é o nó raiz da subárvore para a qual será determinado o somatório de potência ativa
 * @param matrizB
 * @param matrizPiParam
 * @param vetorPiParam
 * @return potenciaAtivaNaoSuprida é o somatório de potência ativa calculado
 */
NIVEISDEPRIORIDADEATENDIMENTO calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		long int indiceNoRaizParam, int indiceLParam, int rnpNoRaizParam, RNPSETORES *matrizB, GRAFO *grafoSDRParam){
	long int noS, noR, noN, indiceBarra, indice;
	RNPSETOR rnpSetorSR;
	RNP rnp;

	NIVEISDEPRIORIDADEATENDIMENTO potenciaAtivaPorAlimentador;
	potenciaAtivaPorAlimentador.consumidoresSemPrioridade = 0;
	potenciaAtivaPorAlimentador.consumidoresPrioridadeBaixa = 0;
	potenciaAtivaPorAlimentador.consumidoresPrioridadeIntermediaria = 0;
	potenciaAtivaPorAlimentador.consumidoresPrioridadeAlta = 0;

	if(rnpNoRaizParam >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Testa se o "noRaizParam" pertence a uma RNP Fictícia
		rnp = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpNoRaizParam - configuracoesParam[idConfiguracaoParam].numeroRNP];
	else
		rnp = configuracoesParam[idConfiguracaoParam].rnp[rnpNoRaizParam];

	for (indice = indiceNoRaizParam; indice <= indiceLParam; indice++) {
		if(indice == indiceNoRaizParam || (indiceNoRaizParam < indice && rnp.nos[indice].profundidade > rnp.nos[indiceNoRaizParam].profundidade)){

			noS = rnp.nos[indice].idNo;
			noR = matrizB[noS].rnps[0].idSetorOrigem; //Pega um nós adjacente qualquer, uma vez que este será usado apenas para buscar as barras presentes no nó "noS"
			rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);

			for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) { //Percorre somente os nós em "rnpSetorSR" que pertencem ao nó "noS"

				noN = rnpSetorSR.nos[indiceBarra].idNo;
				switch(grafoSDRParam[noN].priorizacoes.prioridadeConsumidor){
				case semPrioridade:
					potenciaAtivaPorAlimentador.consumidoresSemPrioridade = potenciaAtivaPorAlimentador.consumidoresSemPrioridade + grafoSDRParam[noN].valorPQ.p;
					break;

				case prioridadeBaixa:
					potenciaAtivaPorAlimentador.consumidoresPrioridadeBaixa = potenciaAtivaPorAlimentador.consumidoresPrioridadeBaixa + grafoSDRParam[noN].valorPQ.p;
					break;

				case prioridadeIntermediaria:
					potenciaAtivaPorAlimentador.consumidoresPrioridadeIntermediaria = potenciaAtivaPorAlimentador.consumidoresPrioridadeIntermediaria + grafoSDRParam[noN].valorPQ.p;
					break;

				case prioridadeAlta:
					potenciaAtivaPorAlimentador.consumidoresPrioridadeAlta = potenciaAtivaPorAlimentador.consumidoresPrioridadeAlta + grafoSDRParam[noN].valorPQ.p;
					break;
				}
			}
		}
	}

    return(potenciaAtivaPorAlimentador);
}

/* Por Leandro:
 * Descrição: esta função consiste na função calculaSomatorioPotenciaAtivaSubarvore() modificada para que:
 * 1) seja calculada a potência aparente e não a potência ativa
 *
 * @param configuracoesParam é a estrutura que carrega as informaçẽos de todos as configurações geradas
 * @param idConfiguracaoParam é a configuração na qual desaja-se calcular o somatório de potência ativa
 * @param noRaizParam é o nó raiz da subárvore para a qual será determinado o somatório de potência ativa
 * @param matrizB
 * @param matrizPiParam
 * @param vetorPiParam
 * @return potenciaAtivaNaoSuprida é o somatório de potência ativa calculado
 */
double calculaSomatorioPotenciaAparenteSubarvore(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, GRAFO *grafoSDRParam){
	long int noS, noR, noN, indiceBarra, idNoRaiz;
	long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
	double potenciaAparenteNaoSuprida = 0, indiceL;
	int indice, idRnp, indiceColuna, idRnpNoRaiz, indice2;
	RNPSETOR rnpSetorSR;
	RNP rnp;

	indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noRaizParam, idConfiguracaoParam);

	idNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].posicao;
	idRnpNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP;

	if(matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se o "noRaizParam" pertence a uma RNP Fictícia
		rnp = configuracoesParam[idConfiguracaoParam].rnpFicticia[idRnpNoRaiz - configuracoesParam[idConfiguracaoParam].numeroRNP];
	else
		rnp = configuracoesParam[idConfiguracaoParam].rnp[idRnpNoRaiz];


	indiceL = limiteSubArvore(rnp, idNoRaiz);
	indice = 0;
	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
    for (indice = 1; indice <= indiceL; indice++) {

    	if(indice == idNoRaiz || (indice > idNoRaiz && rnp.nos[indice].profundidade > rnp.nos[idNoRaiz].profundidade)){

    		noS = rnp.nos[indice].idNo;

    		/*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
    		 * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
    		 * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
    		indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noS, idConfiguracaoParam);
    		if(rnp.nos[indice].profundidade == 1 && matrizPiParam[noS].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se isto é verdade, significa que o noS está numa árvore fictícia e o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
    			noR = matrizB[noS].rnps[0].idSetorOrigem;
    		else
    			noR = noProf[rnp.nos[indice].profundidade - 1];

    		rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
    		for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
    			noN = rnpSetorSR.nos[indiceBarra].idNo;
    			potenciaAparenteNaoSuprida = potenciaAparenteNaoSuprida + sqrt(pow(grafoSDRParam[noN].valorPQ.p, 2) + pow(grafoSDRParam[noN].valorPQ.q, 2) );// cabs(configuracoesParam[0].dadosEletricos.potencia[noN]);
    		}

    	}

    	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
	}

    return(potenciaAparenteNaoSuprida);
}



/* Por Leandro:
 * Descrição: esta função consiste na função calculaSomatorioPotenciaAtivaSubarvore() modificada para que:
 * 1) seja calculada a potência aparente e não a potência ativa
 *
 * @param configuracoesParam é a estrutura que carrega as informaçẽos de todos as configurações geradas
 * @param idConfiguracaoParam é a configuração na qual desaja-se calcular o somatório de potência ativa
 * @param noRaizParam é o nó raiz da subárvore para a qual será determinado o somatório de potência ativa
 * @param matrizB
 * @param matrizPiParam
 * @param vetorPiParam
 * @return potenciaAtivaNaoSuprida é o somatório de potência ativa calculado
 */
double calculaSomatorioCorrenteCargaSubarvore(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int noRaizParam, RNPSETORES *matrizB, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam){
	long int noS, noR, noN, indiceBarra, idNoRaiz;
	long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
	double somatorioCorrenteCarga = 0, indiceL;
	int indice, indiceColuna, idRnpNoRaiz;
	RNPSETOR rnpSetorSR;
	RNP rnp;

	indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noRaizParam, idConfiguracaoParam);

	idNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].posicao;
	idRnpNoRaiz = matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP;

	if(matrizPiParam[noRaizParam].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se o "noRaizParam" pertence a uma RNP Fictícia
		rnp = configuracoesParam[idConfiguracaoParam].rnpFicticia[idRnpNoRaiz - configuracoesParam[idConfiguracaoParam].numeroRNP];
	else
		rnp = configuracoesParam[idConfiguracaoParam].rnp[idRnpNoRaiz];

	indiceL = limiteSubArvore(rnp, idNoRaiz);
	indice = 0;
	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
    for (indice = 1; indice <= indiceL; indice++) {

    	if(indice == idNoRaiz || (indice > idNoRaiz && rnp.nos[indice].profundidade > rnp.nos[idNoRaiz].profundidade)){

    		noS = rnp.nos[indice].idNo;

    		/*Se o "noS" estiver conectado ao Nó Raiz da RNP Fictícia, então não haverá uma RNP do noS sendo alimentado por este
    		 * nó raiz, uma vez que o nó raiz é fictício. Neste caso, para percorrer as barras presentes no noS, tomar-se-á
    		 * a matrizB relativa à alimentação de noS por um noR correspondente ao Setor Origem que primeiro aparece na matriz B se noS*/
    		indiceColuna = retornaColunaPi(matrizPiParam, vetorPiParam, noS, idConfiguracaoParam);
    		if(rnp.nos[indice].profundidade == 1 && matrizPiParam[noS].colunas[indiceColuna].idRNP >= configuracoesParam[idConfiguracaoParam].numeroRNP) //Se isto é verdade, significa que o noS está numa árvore fictícia e o nó anterior a noS é fictício e por isso não há uma matrizB de noS sendo alimentador por este nó, neste caso tomar-se-á um outro nó adjacente a este
    			noR = matrizB[noS].rnps[0].idSetorOrigem;
    		else
    			noR = noProf[rnp.nos[indice].profundidade - 1];

    		rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
    		for (indiceBarra = 1; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
    			noN = rnpSetorSR.nos[indiceBarra].idNo;
    			somatorioCorrenteCarga = somatorioCorrenteCarga + cabs(configuracoesParam[idConfiguracaoParam].dadosEletricos.corrente[noN]);
    		}
    	}

    	noProf[rnp.nos[indice].profundidade] = rnp.nos[indice].idNo;
	}

    //free(rnp.nos);
    return(somatorioCorrenteCarga);
}
/* Por Leandro:
 * Descrição: esta função calcula os valores de energia não suprida ao longo do tempo decorrido para a execução das manobras dos indivíduos que possuem manobras tanto
 * para isolar faltas quanto para restaurar consumidores (EXCLUSIVAMENTE, para outros indivíduos esta função não funcionará corretamente).
 * Ela será aplicada aqui aos indivíduos obtidos pela busca Heurística ou Exaustiva (EXCLUSIVAMENTE DELES) e naqueles indivíduos que forem obtidos para a exclusão de
 * manobras repetidas envolvendo os indivíduos obtidos pela Busca Heurística.
 * Esta função também calcula o tempo necessário para execução de tais manobras, bem como a potência ativa total que foi restaurada, isto é, toda a potência ativa que foi
 * reenergizadas por meio das manobras executadas
 *
 *@param configuracoesParam
 *@param vetorPiParam
 *@param matrizPiParam
 *@param idNovaConfiguracaoParam
 *@param listaChavesParam
 *@param rnpSetoresParam
 *@param grafoSDRParam
 */
void calculaEnergiaAtivaNaoSupridaPorNivelPrioridadeIndividuosIsolaRestabeleceTodasOpcoesHeuristica(CONFIGURACAO *configuracoesParam, VETORPI *vetorPiParam, MATRIZPI *matrizPiParam,
		long int idNovaConfiguracaoParam, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam){
	long int noP, noR, noA, idAncestral, idChaveFechada, idChaveAberta;
	int indiceP, indiceR, indiceA, indiceL, rnpP, rnpA, rnpR, contadorChav;
	NIVEISDEPRIORIDADEATENDIMENTO potenciaAtivaNaoSuprida, energiaNaoSuprida, potenciaAtivaRestaurada, potenciaAtiva;
	double tempoExecucaoManobras;

	configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade = 0;
	configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
	configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria = 0;
	configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa = 0;
	energiaNaoSuprida.consumidoresSemPrioridade = 0;
	energiaNaoSuprida.consumidoresPrioridadeBaixa = 0;
	energiaNaoSuprida.consumidoresPrioridadeIntermediaria = 0;
	energiaNaoSuprida.consumidoresPrioridadeAlta = 0;
	potenciaAtivaNaoSuprida.consumidoresSemPrioridade = 0;
	potenciaAtivaNaoSuprida.consumidoresPrioridadeBaixa = 0;
	potenciaAtivaNaoSuprida.consumidoresPrioridadeIntermediaria = 0;
	potenciaAtivaNaoSuprida.consumidoresPrioridadeAlta = 0;
	configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo = 0;

	idAncestral = vetorPiParam[idNovaConfiguracaoParam].idAncestral;

	//Determina a potência ativa TOTAL que estará fora de serviço após a ocorrência de uma ou mais falhas manobras
	//A fim de desconsiderar a potência ativa dos setores que não podem ser restaurados, bem como dos setores em falta,
	//calcula-se a potência ativa TOTAL afetada por meio da somatório das potência ativas dos setores restauráveis (setores ou agrupamentos de setores que possuem chaves para reconexão em alimentadores energizados)
	for(contadorChav = 0; contadorChav < vetorPiParam[idNovaConfiguracaoParam].numeroManobras; contadorChav++){
		idChaveFechada = vetorPiParam[idNovaConfiguracaoParam].idChaveFechada[contadorChav];
		idChaveAberta = vetorPiParam[idNovaConfiguracaoParam].idChaveAberta[contadorChav];

		if(idChaveAberta > 0 && idChaveFechada > 0){ //Se a operação for para a reconexão de um agrupamento de setores
			noP = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].p;
			noR = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].r;
			noA = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].a;
			recuperaPosicaoPRAModificada(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);
			indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnp[rnpP], indiceP);

			potenciaAtiva = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idAncestral, indiceP, indiceL, rnpP, rnpSetoresParam, grafoSDRParam);

			potenciaAtivaNaoSuprida.consumidoresSemPrioridade 			+= potenciaAtiva.consumidoresSemPrioridade;
			potenciaAtivaNaoSuprida.consumidoresPrioridadeBaixa		    += potenciaAtiva.consumidoresPrioridadeBaixa;
			potenciaAtivaNaoSuprida.consumidoresPrioridadeIntermediaria += potenciaAtiva.consumidoresPrioridadeIntermediaria;
			potenciaAtivaNaoSuprida.consumidoresPrioridadeAlta 			+= potenciaAtiva.consumidoresPrioridadeAlta;
		}
	}

	potenciaAtivaRestaurada.consumidoresSemPrioridade = 0;
	potenciaAtivaRestaurada.consumidoresPrioridadeBaixa = 0;
	potenciaAtivaRestaurada.consumidoresPrioridadeIntermediaria = 0;
	potenciaAtivaRestaurada.consumidoresPrioridadeAlta = 0;

	for(contadorChav = 0; contadorChav < vetorPiParam[idNovaConfiguracaoParam].numeroManobras; contadorChav++){
		//Calcula o tempo para execução das manobras
		idChaveFechada = vetorPiParam[idNovaConfiguracaoParam].idChaveFechada[contadorChav];
		idChaveAberta = vetorPiParam[idNovaConfiguracaoParam].idChaveAberta[contadorChav];

		tempoExecucaoManobras = 0;
		if(idChaveFechada > 0){ //Se há chave para manobrar
			if(listaChavesParam[idChaveFechada].tipoChave == chaveAutomatica)
				tempoExecucaoManobras += tempoOperacaoChaveAutomatica;
			else
				tempoExecucaoManobras += tempoOperacaoChaveManual;
		}
		if(idChaveAberta > 0){ //Se há chave para manobrar
			if(listaChavesParam[idChaveAberta].tipoChave == chaveAutomatica)
				tempoExecucaoManobras += tempoOperacaoChaveAutomatica;
			else
				tempoExecucaoManobras += tempoOperacaoChaveManual;
		}

		//Calcula a energia não Suprida até o término da execução das manobras
		energiaNaoSuprida.consumidoresSemPrioridade           = energiaNaoSuprida.consumidoresSemPrioridade 		  + (potenciaAtivaNaoSuprida.consumidoresSemPrioridade 		 	 * tempoExecucaoManobras);
		energiaNaoSuprida.consumidoresPrioridadeBaixa         = energiaNaoSuprida.consumidoresPrioridadeBaixa 		  + (potenciaAtivaNaoSuprida.consumidoresPrioridadeBaixa 		 * tempoExecucaoManobras);
		energiaNaoSuprida.consumidoresPrioridadeIntermediaria = energiaNaoSuprida.consumidoresPrioridadeIntermediaria + (potenciaAtivaNaoSuprida.consumidoresPrioridadeIntermediaria * tempoExecucaoManobras);
		energiaNaoSuprida.consumidoresPrioridadeAlta          = energiaNaoSuprida.consumidoresPrioridadeAlta          + (potenciaAtivaNaoSuprida.consumidoresPrioridadeAlta 		 * tempoExecucaoManobras);

		//Salva o tempo necessário para execução das manobras
		configuracoesParam[idNovaConfiguracaoParam].objetivo.tempo += tempoExecucaoManobras;

		//Atualiza os valores de potência ativa que estarão fora de serviço após a realização desta manobras e que permanecerão até que sejam realizadas novas manobras
		idChaveFechada = vetorPiParam[idNovaConfiguracaoParam].idChaveFechada[contadorChav];
		idChaveAberta = vetorPiParam[idNovaConfiguracaoParam].idChaveAberta[contadorChav];

		if(idChaveFechada < 0 && idChaveAberta > 0){ //Se a manobra for para abrir a chave localizada à jusante do setor em falta a fim de cortar o seu fornecimento de energia
			potenciaAtivaRestaurada.consumidoresSemPrioridade 			= 0;
			potenciaAtivaRestaurada.consumidoresPrioridadeBaixa 		= 0;
			potenciaAtivaRestaurada.consumidoresPrioridadeIntermediaria = 0;
			potenciaAtivaRestaurada.consumidoresPrioridadeAlta		    = 0;
		}
		else{ //Se as manobras forem para restaurar cargas
			noP = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].p;
			noR = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].r;
			noA = vetorPiParam[idNovaConfiguracaoParam].nos[contadorChav].a;
			recuperaPosicaoPRAModificada(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);
			indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnp[rnpP], indiceP);
			//Calcula a potência ativa as manobras irão restaurar o fornecimento de energia
			potenciaAtivaRestaurada = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idAncestral, indiceP, indiceL, rnpP, rnpSetoresParam, grafoSDRParam);
		}
		//Atualiza os valores de potência ativa que permanecerão fora de serviço após a execução das manobras nas chaves "idChaveFechada" e "idChaveAberta"
		potenciaAtivaNaoSuprida.consumidoresSemPrioridade 			-= potenciaAtivaRestaurada.consumidoresSemPrioridade;
		potenciaAtivaNaoSuprida.consumidoresPrioridadeBaixa		    -= potenciaAtivaRestaurada.consumidoresPrioridadeBaixa;
		potenciaAtivaNaoSuprida.consumidoresPrioridadeIntermediaria -= potenciaAtivaRestaurada.consumidoresPrioridadeIntermediaria;
		potenciaAtivaNaoSuprida.consumidoresPrioridadeAlta 			-= potenciaAtivaRestaurada.consumidoresPrioridadeAlta;

		//Salva a potência ativa transferida para restauração
		configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade 			+= potenciaAtivaRestaurada.consumidoresSemPrioridade;
		configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa  		+= potenciaAtivaRestaurada.consumidoresPrioridadeBaixa;
		configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria += potenciaAtivaRestaurada.consumidoresPrioridadeIntermediaria;
		configuracoesParam[idNovaConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta 			+= potenciaAtivaRestaurada.consumidoresPrioridadeAlta;
	}

	//Salva os valores de energia não suprida
	configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade           = energiaNaoSuprida.consumidoresSemPrioridade;
	configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa         = energiaNaoSuprida.consumidoresPrioridadeBaixa;
	configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
	configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta          = energiaNaoSuprida.consumidoresPrioridadeAlta;
	configuracoesParam[idNovaConfiguracaoParam].objetivo.energiaTotalNaoSuprida                                = energiaNaoSuprida.consumidoresSemPrioridade + energiaNaoSuprida.consumidoresPrioridadeBaixa + energiaNaoSuprida.consumidoresPrioridadeIntermediaria + energiaNaoSuprida.consumidoresPrioridadeAlta;
}


/**
 * Por Leandro: percorre a subárvore que possui como nó raiz o nó com índice "indiceNoRaizSubarvore"
 * e verifica se existem consumidores especiais nesta subárvore.
 *
 * @param rnpParam
 * @param grafoSetoresParam
 * @param indiceNoRaizSubarvore
 *
 * @return 'true' se houver CEs ou 'false', caso contrário
 */
BOOL verificaExistenciaConsumidoresEspeciais(RNP rnpParam, GRAFOSETORES *grafoSetoresParam, int indiceNoRaizSubarvore){
	int indice=0, indiceL=0, indiceNoP=0;
    int tamanho;

    indiceNoP = indiceNoRaizSubarvore;
    /* Percorre a subárvore que possui o nó com índice "indiceNoRaizSubarvore" e verifica se existem
     * consumidores especiais nesta subárvore*/
    if(grafoSetoresParam[rnpParam.nos[indiceNoP].idNo].numeroConsumidoresEspeciais != 0) // Verifica para o próprio nó de poda
    	return true;
    else{
        indice = indiceNoP + 1;
        tamanho = rnpParam.numeroNos;
		while (indice < tamanho && rnpParam.nos[indiceNoP].profundidade < rnpParam.nos[indice].profundidade) { // Verifica para os nós à jusante do nó de poda
			if(grafoSetoresParam[rnpParam.nos[indice].idNo].numeroConsumidoresEspeciais != 0)
				return true;
			indice++;
		}
    }

    return false;
}

/**
 * Por Leandro: percorre a subárvore que possui como nó raiz o nó com índice "indiceNoRaizSubarvore"
 * e calcula a quantidade de nós desta subárvore que estavam, na configuração pré-falta, à jusante do no faltoso.

 * @param rnpParam é a RNP origem e que possuir a subárvore a ser percorrida
 * @param grafoSetoresParam
 * @param indiceNoRaizSubarvore
 *
 * @return a quantidade de setores que estavam, na configuração pré-falta, à jusante do no faltoso.
 */
int determinaQuantidadeSetoresJusanteFalta(RNP rnpParam, GRAFOSETORES *grafoSetoresParam, int indiceNoRaizSubarvore){

	int indice=0, indiceL=0, indiceNoP=0;
    long int tamanho, contadorSetorJusanteFalta = 00;

    indiceNoP = indiceNoRaizSubarvore;
    //tamanho = rnpParam.numeroNos;
    contadorSetorJusanteFalta = 0;
    /* Percorre a subárvore que possui o nó com índice "indiceNoRaizSubarvore" e verifica se
     * se há setores que foram inicialmente afetados pela falta (desligados) */
    if(grafoSetoresParam[rnpParam.nos[indiceNoP].idNo].setorJusanteFalta) // Verifica se o nó de poda era um setor à Jusante da Falta
    	contadorSetorJusanteFalta++;

	indice = indiceNoP + 1;
	tamanho = rnpParam.numeroNos;
	while (indice < tamanho && rnpParam.nos[indiceNoP].profundidade < rnpParam.nos[indice].profundidade) { // Verifica para os nós à jusante do nó de poda
		if(grafoSetoresParam[rnpParam.nos[indice].idNo].setorJusanteFalta) // Verifica se algum dos demais nós era um setor à Jusante da Falta
			contadorSetorJusanteFalta++;
		indice++;
	}

/*    if((contadorSetorJusanteFalta == tamanhoSubarvore) && (contadorSetorJusanteFalta != 0))  //Todos os setores da subárvore percorrida estavam à jusante do setor em falta
    	return true;
    else
    	return false;*/
    return contadorSetorJusanteFalta;
}

/**
 * Por Leandro: percorre todas os setores localizados à jusante da falta a fim de marcá-los, por meio da
 * variável 'grafoSetoresParam[].setorJusanteFalta', como sendo os setores que foram inicialmente desligados
 * pela ocorrência da falta e que, caso seja necessário cortar cargas, deverão ser os primeiros a serem cortados.
 *
 * @param nosJusanteFaltaParam[100] é um vetor que armazena os nós raiz das subarvores desligadas pela falta
 * @param numeroAJusanteParam é o número total de subárvore desligadas pela falta
 * @param configuracoesParam  é a estrutura que armazena informações dos indivíduos do AEMO
 * @param idConfiguracaoParam é o índice da configuração no qual será feita a análise (configuração 0)
 * @param rnpPParam	é o alimentador ou árvore no qual ocorreu a falta
 * @param grafoSetoresParam é a estutura que armazena as informações dos setores da rede
 * @param matrizPiParam é a matriz PI da RNP
 * @param vetorPiParam é o vetor pi da RNP
 * @return
 */
void marcaSetoresJusanteFalta(long int nosJusanteFaltaParam[100], int numeroAJusanteParam, CONFIGURACAO *configuracoesParam,
								long int idConfiguracaoParam, int rnpPParam, GRAFOSETORES *grafoSetoresParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam){

	int indice, indiceNo, indiceNoRaiz, indiceNoRaizUltimaColunaMatrizPi, tamanho;
	long int noRaiz, no;

	tamanho = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].numeroNos;

	for(indice=0; indice < numeroAJusanteParam; indice++){
		noRaiz = nosJusanteFaltaParam[indice];

		//Marca o nó raiz da subárvore em 'noRaiz'
		grafoSetoresParam[noRaiz].setorJusanteFalta = true;

		//Marca os demais nós que possuem 'noRaiz' como nó raiz
		indiceNoRaizUltimaColunaMatrizPi = retornaColunaPi(matrizPiParam, vetorPiParam, noRaiz, idConfiguracaoParam); //indice da coluna da matriz PI que contém o 'noInfactivel' na floresta 'idConfiguracaoSelecionadaParam'
		indiceNoRaiz = matrizPiParam[noRaiz].colunas[indiceNoRaizUltimaColunaMatrizPi].posicao;
		indiceNo = indiceNoRaiz + 1;
		while (indiceNo < tamanho && configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceNoRaiz].profundidade < configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceNo].profundidade) { // Verifica para os nós à jusante do nó de poda
			no = configuracoesParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceNo].idNo;
			grafoSetoresParam[no].setorJusanteFalta = true;
			indiceNo++;
		}
	}
}


/**
 * Por Leandro: função responsável por realizar a cópia de todas as informações de um
 * indivíduo para outro, o que inclui a copia dos dados da configuração (valores
 * dos objetivos, RNPs, etc), do Vetor Pi e da Matriz PI.
 *
 *
 * @param configuracoesParam ponteiro do tipo CONFIGURACAO para a configuração original.
 * @param configuracoesParam2 ponteiro do tipo CONFIGURACAO para a configuração destino da cópia.
 * @param idIndividuoAtual inteiro com o identificador do indivíduo original
 * @param idNovoIndividuo inteiro com o identificador para a cópia
 * @param matrizPIParam ponteiro para a estrutura do tipo MATRIZPI
 */
void copiaTodasInformacoesIndividuo(CONFIGURACAO *configuracoesCopiaParam, CONFIGURACAO *configuracoesOriginalParam,
		long int idIndividuoOriginalParam, long int idIndividuoCopiaParam, VETORPI *vetorPiCopiaParam, VETORPI *vetorPiOriginalParam,
		MATRIZPI *matrizPICopiaParam, MATRIZPI *matrizPIOriginalParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam, int numeroTrafosParam){

	copiaIndividuoMelhorada(configuracoesOriginalParam, configuracoesCopiaParam, idIndividuoOriginalParam, idIndividuoCopiaParam, matrizPIOriginalParam, rnpSetoresParam, numeroBarrasParam, numeroTrafosParam);
	copiaDadosVetorPI(vetorPiOriginalParam, vetorPiCopiaParam, idIndividuoOriginalParam, idIndividuoCopiaParam);
	//copiaColunasMatrizPI(matrizPICopiaParam, matrizPIOriginalParam, idIndividuoCopiaParam, idIndividuoOriginalParam, configuracoesOriginalParam);
}


/**
 * Por Leandro: Esta função é responsável por realizar a cópia das informações de dados elétricos e de objetivos
 * de uma configuração em outra.
 *
 * @param configuracoesParam ponteiro do tipo CONFIGURACAO para a configuração original.
 * @param configuracoesParam2 ponteiro do tipo CONFIGURACAO para a configuração destino da cópia.
 * @param idIndividuoAtual inteiro com o identificador do indivíduo original
 * @param idNovoIndividuo inteiro com o identificador para a cópia
 */
void copiaDadosEletricosObjetivos(CONFIGURACAO *configuracoesParam, CONFIGURACAO *configuracoesParam2, long int idIndividuoAtual,
		long int idNovoIndividuo, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam, int numeroTrafosParam, BOOL copiarDadosEletricosParam){
    int contador, contadorRnp, indiceI;
    //RNPSETOR rnpSetorSR;
    //long int noS, noN;

    if (copiarDadosEletricosParam) { //Aloca memória e copia de fato todos os valores, de dados elétricos e potência trafo
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante = 0;
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente = 0;
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia = 0;
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra = 0;

    	configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    	configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));

        for (contador = 1; contador <= numeroBarrasParam; contador++) {
        	configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante[contador] = configuracoesParam[idIndividuoAtual].dadosEletricos.iJusante[contador];
        	configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente[contador] = configuracoesParam[idIndividuoAtual].dadosEletricos.corrente[contador];
        	configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia[contador] = configuracoesParam[idIndividuoAtual].dadosEletricos.potencia[contador];
        	configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra[contador]   = configuracoesParam[idIndividuoAtual].dadosEletricos.vBarra[contador];
        }

        configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo = 0;
        configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
        for (indiceI = 0; indiceI <= numeroTrafosParam; indiceI++)
        	configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo[indiceI] = configuracoesParam[idIndividuoAtual].objetivo.potenciaTrafo[indiceI];

        //Copia outras informações importantes
        configuracoesParam2[idNovoIndividuo].numeroRNPsFalta = configuracoesParam[idIndividuoAtual].numeroRNPsFalta;
        for (indiceI = 0; indiceI < configuracoesParam[idIndividuoAtual].numeroRNPsFalta; indiceI++)
        	configuracoesParam2[idNovoIndividuo].idRnpFalta[indiceI] = configuracoesParam[idIndividuoAtual].idRnpFalta[indiceI];

//        for (indiceI = 0; indiceI < configuracoesParam[idIndividuoAtual].numeroRNPFicticia; indiceI++)
//        	configuracoesParam2[idNovoIndividuo].ranqueamentoRnpsFicticias[indiceI] = configuracoesParam[idIndividuoAtual].ranqueamentoRnpsFicticias[indiceI];

    } else { //Em vez de copiar os valores, copia os ponteiros para os vetores com dados elétricos e potêcia trafo
        configuracoesParam2[idNovoIndividuo].dadosEletricos.iJusante = configuracoesParam[idIndividuoAtual].dadosEletricos.iJusante;
        configuracoesParam2[idNovoIndividuo].dadosEletricos.corrente = configuracoesParam[idIndividuoAtual].dadosEletricos.corrente;
        configuracoesParam2[idNovoIndividuo].dadosEletricos.potencia = configuracoesParam[idIndividuoAtual].dadosEletricos.potencia;
        configuracoesParam2[idNovoIndividuo].dadosEletricos.vBarra   = configuracoesParam[idIndividuoAtual].dadosEletricos.vBarra;
        configuracoesParam2[idNovoIndividuo].objetivo.potenciaTrafo  = configuracoesParam[idIndividuoAtual].objetivo.potenciaTrafo;

        //Copia outras informações importantes
        configuracoesParam2[idNovoIndividuo].idRnpFalta = configuracoesParam[idIndividuoAtual].idRnpFalta;
        configuracoesParam2[idNovoIndividuo].numeroRNPsFalta = configuracoesParam[idIndividuoAtual].numeroRNPsFalta;
//        configuracoesParam2[idNovoIndividuo].ranqueamentoRnpsFicticias = configuracoesParam[idIndividuoAtual].ranqueamentoRnpsFicticias;
    }
    //Copia outras informações importantes
	for(contadorRnp = 0; contadorRnp < configuracoesParam[idIndividuoAtual].numeroRNP; contadorRnp++)
		configuracoesParam2[idNovoIndividuo].rnp[contadorRnp].fitnessRNP = configuracoesParam[idIndividuoAtual].rnp[contadorRnp].fitnessRNP;

    //copia os valores do objetivos
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisSemFornecimento    = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresSemFornecimento 		      = configuracoesParam[idIndividuoAtual].objetivo.consumidoresSemFornecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoRede				      = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorCarregamentoTrafo                  = configuracoesParam[idIndividuoAtual].objetivo.maiorCarregamentoTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.maiorDemandaAlimentador                 = configuracoesParam[idIndividuoAtual].objetivo.maiorDemandaAlimentador;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAutomaticas                     = configuracoesParam[idIndividuoAtual].objetivo.manobrasAutomaticas;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasManuais                         = configuracoesParam[idIndividuoAtual].objetivo.manobrasManuais;
    configuracoesParam2[idNovoIndividuo].objetivo.menorTensao                             = configuracoesParam[idIndividuoAtual].objetivo.menorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.perdasResistivas                        = configuracoesParam[idIndividuoAtual].objetivo.perdasResistivas;
    configuracoesParam2[idNovoIndividuo].objetivo.quedaMaxima                             = configuracoesParam[idIndividuoAtual].objetivo.quedaMaxima;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasRestabelecimento                = configuracoesParam[idIndividuoAtual].objetivo.manobrasRestabelecimento;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAlivio                          = configuracoesParam[idIndividuoAtual].objetivo.manobrasAlivio;
    configuracoesParam2[idNovoIndividuo].objetivo.manobrasAposChaveamento                 = configuracoesParam[idIndividuoAtual].objetivo.manobrasAposChaveamento;
    configuracoesParam2[idNovoIndividuo].objetivo.ponderacao                              = configuracoesParam[idIndividuoAtual].objetivo.ponderacao;
    configuracoesParam2[idNovoIndividuo].objetivo.rank                                    = configuracoesParam[idIndividuoAtual].objetivo.rank;
    configuracoesParam2[idNovoIndividuo].objetivo.fronteira                               = configuracoesParam[idIndividuoAtual].objetivo.fronteira;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresEspeciaisTransferidos       = configuracoesParam[idIndividuoAtual].objetivo.consumidoresEspeciaisTransferidos;
    configuracoesParam2[idNovoIndividuo].objetivo.consumidoresDesligadosEmCorteDeCarga    = configuracoesParam[idIndividuoAtual].objetivo.consumidoresDesligadosEmCorteDeCarga;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaAutomatica = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.comCargaManual     = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.comCargaManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoAutomatica    = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoAutomatica;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.curtoManual        = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.curtoManual;
    configuracoesParam2[idNovoIndividuo].objetivo.contadorManobrasTipo.seca               = configuracoesParam[idIndividuoAtual].objetivo.contadorManobrasTipo.seca;
    configuracoesParam2[idNovoIndividuo].objetivo.noMenorTensao                           = configuracoesParam[idIndividuoAtual].objetivo.noMenorTensao;
    configuracoesParam2[idNovoIndividuo].objetivo.noMaiorCarregamentoRede                 = configuracoesParam[idIndividuoAtual].objetivo.noMaiorCarregamentoRede;
    configuracoesParam2[idNovoIndividuo].objetivo.idTrafoMaiorCarregamento                = configuracoesParam[idIndividuoAtual].objetivo.idTrafoMaiorCarregamento;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaRede                          = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaRede;
    configuracoesParam2[idNovoIndividuo].objetivo.sobrecargaTrafo                         = configuracoesParam[idIndividuoAtual].objetivo.sobrecargaTrafo;
    configuracoesParam2[idNovoIndividuo].objetivo.tempo                                   = configuracoesParam[idIndividuoAtual].objetivo.tempo;
    configuracoesParam2[idNovoIndividuo].objetivo.tempoBruto                              = configuracoesParam[idIndividuoAtual].objetivo.tempoBruto;

    configuracoesParam2[idNovoIndividuo].objetivo.potenciaTotalNaoSuprida                                = configuracoesParam[idIndividuoAtual].objetivo.potenciaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade;

    configuracoesParam2[idNovoIndividuo].objetivo.energiaTotalNaoSuprida                                = configuracoesParam[idIndividuoAtual].objetivo.energiaTotalNaoSuprida;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta          = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa         = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa;
    configuracoesParam2[idNovoIndividuo].objetivo.energiaNaoSuprida.consumidoresSemPrioridade           = configuracoesParam[idIndividuoAtual].objetivo.energiaNaoSuprida.consumidoresSemPrioridade;

}

/* Por Leandro:
 * Esta função realiza a impressão de sequência de chaveamento relacionada a um indivíduo e demais informações importantes.
 * No início é feito um teste a fim de determinar se àquele indivíduo está associada uma sequência de chaveamento que foi
 * corrigida para remoção de chaves que retonaram ao seu estado inicial
 *
 *@param idConfiguracaoParam é o índice do índivuo cuja sequência de chaveamento será impressa
 *@param vetorPiParam é o vetor Pi
 *@param configuracoesParam é o vetor que contém as informações de todos os indivíduos gerados pelo AEMO
 *@param configuracoesChavParam é um verto que contém as informações das configurações intermediárias nas sequências de chaveamento
 *corrigidas. Este parametro só é importante e utilizado quando a sequência de chaveamento tiver sido corrigida e só conhecido dentro da função "corrigeSequenciaChaveamento"
 */

void mostraInformacoesIndividuoSequenciaCorrigida(long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,  CONFIGURACAO *configuracoesChavParam){
	int numeroConfiguracoes, indice, indice2;
	long int *indiceConfiguracoes, idConfig;


	/*if(vetorPiParam[idConfiguracaoParam].sequenciaCorrigida == true){

		printf("\n\n\t------SEQUENCIA DE CHAVEAMENTO CORRIGIDA------");
		printf("\nAbrir\tFechar\tCar. Rede(%)\tCar. Trafo(%)\tQueda de Tensão(%)\tManuais\tRemotas\tID_Config.");
		for(indice = 0; indice < vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.tamanho; indice++){
			idConfig = vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].idConfiguracao;

			for(indice2 = 0; indice2 < vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].numeroParesManobras; indice2++){
				if(indice2 ==  vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].numeroParesManobras - 1)
					printf("\n  %ld\t%ld\t%.2f\t%.2f\t%.2f\t%d\t%d\t%ld", vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].idChaveAberta[indice2], vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].idChaveFechada[indice2], configuracoesChavParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesChavParam[idConfig].objetivo.maiorCarregamentoTrafo, configuracoesChavParam[idConfig].objetivo.quedaMaxima,  configuracoesChavParam[idConfig].objetivo.manobrasManuais,  configuracoesChavParam[idConfig].objetivo.manobrasAutomaticas, idConfig);
				else
					printf("\n  %ld\t%ld", vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].idChaveAberta[indice2],vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].idChaveFechada[indice2]);
			}
		}

		printf("\n\n\n\tCARGAS FORA DE SERVIÇO E NUMERO DE MANOBRAS");
		printf("\nID Config.\tPNS_SP\tPNS_PB\tPNS_PI\tPNS_PA\tMnaobras Manuais\tManobras Remotas\tENS_SP\tENS_PB\tENS_PI\tENS_PA");

		for(indice = 0; indice < vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.tamanho; indice++){
			idConfig = vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.configuracao[indice].idConfiguracao;
			if(indice == vetorPiParam[idConfiguracaoParam].sequenciaChaveamento.tamanho - 1)
				printf("\n%ld\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f", idConfig, configuracoesParam[idConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfiguracaoParam].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais, configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas, configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta);
			else
				printf("\n%ld\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f", idConfig, configuracoesChavParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesChavParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesChavParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesChavParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesChavParam[idConfig].objetivo.manobrasManuais, configuracoesChavParam[idConfig].objetivo.manobrasAutomaticas, configuracoesChavParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesChavParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesChavParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesChavParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta);
		}
		printf("\n");
		printf("\n");

	}*/

}
/* Por Leandro
 * Descrição: dada uma configuração, esta função determina se a mesma foi obtida por manobras de transferência de carga entre alimentadores, as
 * quais visam o alívio de alimentadores.
 * Os operadores PAO e CAO produzem novos indivíduos por meio de operações para alívio de carga.
 * O Operador LRo produz novos indivíduos por meio de operadores para reconexão de cargas
 *
 *@param configuracoesParam
 *@param matrizPiParam
 *@param vetorPiParam
 *@param idConfiguracaoParam
 *
 *@return flagManobrasAlivio
 */
BOOL determinaFlagManobrasAlivio(CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, long int idConfiguracaoParam){
	long int noP, noA, noR, idAncestral;
	int colunaPI, rnpP, rnpA;
	BOOL flagManobrasAlivio;

	idAncestral = vetorPiParam[idConfiguracaoParam].idAncestral;
	noP = vetorPiParam[idConfiguracaoParam].nos[0].p;
	noA = vetorPiParam[idConfiguracaoParam].nos[0].a;
	noR = vetorPiParam[idConfiguracaoParam].nos[0].r;

	if(noP > 0 && noR > 0 && noA > 0){
		colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, noA, idAncestral);
		rnpA = matrizPiParam[noA].colunas[colunaPI].idRNP;

		colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, noP, idAncestral);
		rnpP = matrizPiParam[noP].colunas[colunaPI].idRNP;

		if(rnpP >= configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP)  //Operações de Reconexão de cargas (LRO)
			flagManobrasAlivio = false;
		else{
			if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP) //Operações de transferência de cargas (PAO ou CAO)
				flagManobrasAlivio = true;
			else{
				if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA >= configuracoesParam[idAncestral].numeroRNP)  //Operações de corte de carga (obtenção do indivíduo sem cargas religadas e LSO (se estiver sendo aplicado))
					flagManobrasAlivio = false;
			}
		}
	}
	else //Se os nós p,r e a não assumem valores possíveis, então não houve manobras de alívio de carga por transferência de setores entre alimentadores energizados
		flagManobrasAlivio = false;

	return flagManobrasAlivio;
}


/* Por Leandro:
 * Esta função mostra na tela a sequência de chaveamento relacionada a um indivíduo e demais informações importantes.
 *
 *@param idConfiguracaoParam é o índice do índivuo cuja sequência de chaveamento será impressa
 *@param vetorPiParam é o vetor Pi
 *@param configuracoesParam é o vetor que contém as informações de todos os indivíduos gerados pelo AEMO
 */
void mostraSequenciaChaveamentoIndividuo(long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam){
	int numeroConfiguracoes, indice, indice2;
	long int *indiceConfiguracoes, idConfig;

	numeroConfiguracoes = 0;
	indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam, &numeroConfiguracoes);


	if(sequenciaManobrasAlivioParam == abreFecha)
		printf("\nCONFIG.\t\tABRIR\tFECHAR\t\tMAN_M\tMAN_R\t\tENS_PA\tENS_PI\tENS_PB\tENS_SP\t\tPNS_PA\tPNS_PI\tPNS_PB\tPNS_SP\t\tCAR_REDE\tQUEDA_T\tCAR_SE");
	else
		printf("\nCONFIG.\t\tFECHAR\tABRIR\t\tMAN_M\tMAN_R\t\tENS_PA\tENS_PI\tENS_PB\tENS_SP\t\tPNS_PA\tPNS_PI\tPNS_PB\tPNS_SP\t\tCAR_REDE\tQUEDA_T\tCAR_SE");

	for(indice2 = 0; indice2 < numeroConfiguracoes; indice2++){
		idConfig = indiceConfiguracoes[indice2];
		for(indice = 0; indice < vetorPiParam[idConfig].numeroManobras; indice++){
			if(indice ==  vetorPiParam[idConfig].numeroManobras - 1){
				if(sequenciaManobrasAlivioParam == abreFecha)
					printf("\n%ld\t\t%ld\t%ld\t\t%d\t%d\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f", idConfig, vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice], configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo);
				else
					printf("\n%ld\t\t%ld\t%ld\t\t%d\t%d\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f", idConfig, vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice], configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo);
			}
			else{
				if(sequenciaManobrasAlivioParam == abreFecha)
					printf("\n\t\t%ld\t%ld", vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice]);
				else
					printf("\n\t\t%ld\t%ld", vetorPiParam[idConfig].idChaveFechada[indice], vetorPiParam[idConfig].idChaveAberta[indice]);
			}

		}
	}

	printf("\n");
	printf("\n");

}

/* Por Leandro:
 * Esta função imprime num arquivo a sequência de chaveamento relacionada a um indivíduo e demais informações importantes.
 *
 *@param idConfiguracaoParam é o índice do índivuo cuja sequência de chaveamento será impressa
 *@param vetorPiParam é o vetor Pi
 *@param configuracoesParam é o vetor que contém as informações de todos os indivíduos gerados pelo AEMO
 */
void imprimeSequenciaChaveamentoIndividuo(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, char *nomeArquivoParam){
	int numeroConfiguracoes, indice, indice2;
	long int *indiceConfiguracoes, idConfig;
	long int auxiliar;
	char factibilidade[20];
	FILE *arquivo1;

//	sprintf(nomeArquivoParam, "ArquivoSaida19.txt");
	arquivo1 = fopen(nomeArquivoParam, "a");

    //Construção de uma string para armazenar se a solução é factível
	strcpy(factibilidade, "Infactivel");
	if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoParam))
		strcpy(factibilidade, "Factivel");

    fprintf(arquivo1, "\nID_INDIVIDUO: %ld (%s)\n", idConfiguracaoParam, factibilidade);

	numeroConfiguracoes = 0;
	indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam, &numeroConfiguracoes);

	if(sequenciaManobrasAlivioParam == abreFecha)
		fprintf(arquivo1, "ABRIR\tFECHAR\t\tENS_PA\tENS_PI\tENS_PB\tENS_SP\t\tMAN_M\tMAN_R\t\tPNS_PA\tPNS_PI\tPNS_PB\tPNS_SP\t\tCAR_REDE\tQUEDA_T\tCAR_SE\tFACTIBILIDADE\n");
	else
		fprintf(arquivo1, "FECHAR\tABRIR\t\tENS_PA\tENS_PI\tENS_PB\tENS_SP\t\tMAN_M\tMAN_R\t\tPNS_PA\tPNS_PI\tPNS_PB\tPNS_SP\t\tCAR_REDE\tQUEDA_T\tCAR_SE\tFACTIBILIDADE\n");

	for(indice2 = 0; indice2 < numeroConfiguracoes; indice2++){
		idConfig = indiceConfiguracoes[indice2];
	    //Construção de uma string para armazenar se a solução é factível
		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoParam))
			strcpy(factibilidade, "Factível");
//		else{
//			if(verificaFactibilidade(configuracoesParam[idConfiguracaoParam], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR ))
//				strcpy(factibilidade, "Factível Relaxada");
//		}

		for(indice = 0; indice < vetorPiParam[idConfig].numeroManobras; indice++){
			if(indice ==  vetorPiParam[idConfig].numeroManobras - 1){
				if(sequenciaManobrasAlivioParam == abreFecha)
					fprintf(arquivo1, "%ld\t%ld\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%d\t%d\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\t%s\n", vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice], configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo, factibilidade);
				else
					fprintf(arquivo1, "%ld\t%ld\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%d\t%d\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\t%s\n", vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice], configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo, factibilidade);
			}
			else{
				if(sequenciaManobrasAlivioParam == abreFecha)
					fprintf(arquivo1, "%ld\t%ld\n", vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice]);
				else
					fprintf(arquivo1, "%ld\t%ld\n", vetorPiParam[idConfig].idChaveFechada[indice], vetorPiParam[idConfig].idChaveAberta[indice]);
			}

		}
	}

	fclose(arquivo1);

}
/* Por Leandro:
 * Esta função imprime num arquivo a sequência de chaveamento relacionada a um indivíduo e demais informações importantes DE MANEIRA SUMARIZA A FIM DE FACILITAR ANÁLISES
 * DA SEQUÊNCIA DE CHAVEAMENTO VIA SOFTWARE.
 *
 *@param idConfiguracaoParam é o índice do índivuo cuja sequência de chaveamento será impressa
 *@param vetorPiParam é o vetor Pi
 *@param configuracoesParam é o vetor que contém as informações de todos os indivíduos gerados pelo AEMO
 */
void imprimeSequenciaChaveamentoIndividuoSumarizada(int seedParam, char tipoFaltaParam[20], char setoresFaltaParam[2000], long int idConfiguracaoParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, char *nomeArquivoParam){
	int numeroConfiguracoes, indice, indice2;
	long int *indiceConfiguracoes, idConfig;
	char factibilidade[20];
	FILE *arquivo1;

	arquivo1 = fopen(nomeArquivoParam, "a");
    //Construção de uma string para armazenar se a solução é factível
	strcpy(factibilidade, "Infactivel");
	if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoParam))
		strcpy(factibilidade, "Factivel");

	numeroConfiguracoes = 0;
	indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam, &numeroConfiguracoes);


	for(indice2 = 0; indice2 < numeroConfiguracoes; indice2++){
		idConfig = indiceConfiguracoes[indice2];
	    //Construção de uma string para armazenar se a solução é factível
		strcpy(factibilidade, "Infactivel");
		if(verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoParam))
			strcpy(factibilidade, "Factível");


		for(indice = 0; indice < vetorPiParam[idConfig].numeroManobras; indice++){
			if(indice ==  vetorPiParam[idConfig].numeroManobras - 1){
				if(sequenciaManobrasAlivioParam == abreFecha)
					fprintf(arquivo1, "%s\t%s\t%s\t%d\t%ld\t%ld\t%ld\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", tipoFaltaParam, setoresFaltaParam, factibilidade, seedParam, idConfiguracaoParam, vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice], configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo);
				else
					fprintf(arquivo1, "%s\t%s\t%s\t%d\t%ld\t%ld\t%ld\t%.2f\t%.2f\t%.2f\t%.2f\t%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", tipoFaltaParam, setoresFaltaParam, factibilidade, seedParam, idConfiguracaoParam, vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice], configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo);
			}
			else{
				if(sequenciaManobrasAlivioParam == abreFecha)
					fprintf(arquivo1, "%s\t%s\t%s\t%d\t%ld\t%ld\t%ld\n", tipoFaltaParam, setoresFaltaParam, factibilidade, seedParam, idConfiguracaoParam, vetorPiParam[idConfig].idChaveAberta[indice], vetorPiParam[idConfig].idChaveFechada[indice]);
				else
					fprintf(arquivo1, "%s\t%s\t%s\t%d\t%ld\t%ld\t%ld\n", tipoFaltaParam, setoresFaltaParam, factibilidade, seedParam, idConfiguracaoParam, vetorPiParam[idConfig].idChaveFechada[indice], vetorPiParam[idConfig].idChaveAberta[indice]);
			}

		}
	}

	fclose(arquivo1);

}

void imprimeSequenciaChaveamentoSolucoesFinais(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, FRONTEIRAS fronteiraParetoParam, long int *idIndividuosFinaisParam, int numeroSolucoesFinaisObtidas, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam){
	int indice, indiceIndividuo;
	long int idConfiguracao, auxiliar;
	char setoresFalta[2000]= "", str[20], tipoFalta[20], nomeArquivo[120];
	FILE *arquivo1;

	sprintf(nomeArquivo, "ArquivoSaida12.txt");
	arquivo1 = fopen(nomeArquivo, "a");

    //Construção de uma String para armazenar o tipo da falta
    strcpy(tipoFalta, "Multiplas_Faltas");
    if(numeroSetorFaltaParam == 1)
    	strcpy(tipoFalta, "Falta_Simples");

    //Construção de uma string para armazenar todos os setores em falta
    for(indice = 0; indice < numeroSetorFaltaParam; indice++){
    	auxiliar = setorFaltaParam[indice];
    	sprintf(str, "%ld_", auxiliar);
    	strcat(setoresFalta, str);
    }

    fprintf(arquivo1, "\n\n%s\t%s\t%d\n", tipoFalta, setoresFalta, seedParam);
    fclose(arquivo1);


	for(indice = 0; indice < numeroSolucoesFinaisObtidas; indice++){
		indiceIndividuo = idIndividuosFinaisParam[indice];
		idConfiguracao = fronteiraParetoParam.individuos[indiceIndividuo].idConfiguracao;

		imprimeSequenciaChaveamentoIndividuo(seedParam, setorFaltaParam, numeroSetorFaltaParam, idConfiguracao, vetorPiParam, configuracoesParam, sequenciaManobrasAlivioParam, nomeArquivo);
	}

}

/* Por Leandro:
 * Esta função mostra no "ArquivoSaida11.txt" a sequência de chaveamento relacionada a um indivíduo e demais informações importantes.
 *
 *@param idConfiguracaoParam é o índice do índivuo cuja sequência de chaveamento será impressa
 *@param vetorPiParam é o vetor Pi
 *@param configuracoesParam é o vetor que contém as informações de todos os indivíduos gerados pelo AEMO
 */
void imprimeFronteiraPareto(int seedParam, long int *setorFaltaParam, int numeroSetorFaltaParam, FRONTEIRAS fronteiraParam, CONFIGURACAO *configuracoesParam){
	int indice;
	long int idConfig, auxiliar;
	char setoresFalta[2000]= "", str[20], tipoFalta[20], nomeArquivo1[120], factibilidade[20];
	FILE *arquivo1;

	sprintf(nomeArquivo1, "ArquivoSaida11.txt");
	arquivo1 = fopen(nomeArquivo1, "a");

    //Construção de uma String para armazenar o tipo da falta
    strcpy(tipoFalta, "Multiplas_Faltas");
    if(numeroSetorFaltaParam == 1)
    	strcpy(tipoFalta, "Falta_Simples");

    //Construção de uma string para armazenar todos os setores em falta
    for(indice = 0; indice < numeroSetorFaltaParam; indice++){
    	auxiliar = setorFaltaParam[indice];
    	sprintf(str, "%ld_", auxiliar);
    	strcat(setoresFalta, str);
    }

    //Imprime informações dos indivíduos presentes na fronteira de Pareto
    for(indice = 0; indice < fronteiraParam.numeroIndividuos; indice++){
    	idConfig = fronteiraParam.individuos[indice].idConfiguracao;

        //Construção de uma string para armazenar se a solução é factível
    	strcpy(factibilidade, "Infactivel");
    	if(verificaFactibilidade(configuracoesParam[idConfig], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao ))
    		strcpy(factibilidade, "Factivel");

    	fprintf(arquivo1, "%s\t%s\t%s\t%d\t%ld\t\t%.2f\t%.0f\t\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t\t%d\t%d\t\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\n", tipoFalta, setoresFalta, factibilidade, seedParam, idConfig, fronteiraParam.individuos[indice].valorObjetivo1, fronteiraParam.individuos[indice].valorObjetivo2, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.energiaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.manobrasManuais,  configuracoesParam[idConfig].objetivo.manobrasAutomaticas, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeAlta, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeIntermediaria, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresPrioridadeBaixa, configuracoesParam[idConfig].objetivo.potenciaNaoSuprida.consumidoresSemPrioridade, configuracoesParam[idConfig].objetivo.maiorCarregamentoRede, configuracoesParam[idConfig].objetivo.quedaMaxima, configuracoesParam[idConfig].objetivo.maiorCarregamentoTrafo);
    }

	fclose(arquivo1);

}

