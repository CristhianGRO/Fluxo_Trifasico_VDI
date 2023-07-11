/**
 * @file funcoesChaveamento.c
 * @brief Este arquivo contém a implementação das funções utilizadas para obter a sequência de manobras.
 */
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "funcoesChaveamento.h"
#include "data_structures.h"
#include "funcoesProblema.h"
#include "funcoesHeuristica.h"

/**
 * Função que gerência o calculo do fluxo de carga para todos os alimentadores. 
 * A diferença para função fluxoCargaTodosAlimentadores é que a presente função ignora no cálculo o alimentador fictício criado no processo de chaveamento.
 * @param numeroBarrasParam inteiro com o número total de barras do SDR
 * @param dadosAlimentadorParam ponteiro para estrutura DADOSALIMENTADOR com os dados dos alimentadores do SDR
 * @param dadosTrafoParam ponteiro para estrutura DADOSTRAFO com os dados dos trafos do SDR
 * @param configuracoesParam ponteiro para a estrutura CONFIGURACAO 
 * @param indiceConfiguracao inteiro com o índice da configuração no vetor de configurações
 * @param matrizB ponteiro para estrutura RNPSETORES com as representações nó-profundidade das barras de cada setor
 * @param ZParam ponteiro para estrutura MATRIZCOMPLEXA com as impedâncias para cada linha do SDR
 * @param indiceRegulador ponteiro para inteiros com o índice do regulador presente entre duas barras
 * @param dadosRegulador ponteiro para estrutura DADOSREGULADOR que contém os dados de cada um dos reguladores do SDR
 * @param maximoCorrenteParam ponteiro para estrutura MATRIZMAXCORRENTE com os máximos de corrente para cada linha do SDR
 * @return 
 */
void fluxoCargaTodosAlimentadoresChaveamento(long int numeroBarrasParam,
        DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
        CONFIGURACAO *configuracoesParam, long int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, 
        MATRIZMAXCORRENTE * maximoCorrenteParam) {

    int indiceRNP;
    __complex__ double VF;
    for (indiceRNP = 0; indiceRNP < configuracoesParam[indiceConfiguracao].numeroRNP - 1; indiceRNP++) {
        VF = 1000 * dadosTrafoParam[dadosAlimentadorParam[indiceRNP + 1].idTrafo].tensaoReal / sqrt(3);
        fluxoCargaAlimentador(numeroBarrasParam, configuracoesParam, VF, indiceRNP, indiceConfiguracao, matrizB,
                ZParam, maximoCorrenteParam, indiceRegulador, dadosRegulador, 1);
    }
}

/**
 * Método para imprimir em tela os dados da estrutura VETORPI passada como parâmetro.
 * @param vetorPi
 * @param idConfiguracao
 * @param listaChavesParam
 */
void imprimeVetorPi(VETORPI *vetorPi, int idConfiguracao, LISTACHAVES *listaChavesParam) {
    int contador1, contador2;
    for (contador1 = 1; contador1 <= idConfiguracao; contador1++) {
        printf("individuo %d idAncestral %ld \n", contador1, vetorPi[contador1].idAncestral);
        for (contador2 = 0; contador2 < vetorPi[contador1].numeroManobras; contador2++) {
            printf("Chave\tAberta\t%ld\t", listaChavesParam[vetorPi[contador1].idChaveAberta[contador2]].identificador);
            //   printf("estado CA %d ", vetorPi[contador1].estadoInicialChaveAberta[contador2]);
            if (vetorPi[contador1].idChaveFechada[contador2] > 0)
                printf("Chave\tFechada\t%ld\t", listaChavesParam[vetorPi[contador1].idChaveFechada[contador2]].identificador);
            // printf("estado CF %d ", vetorPi[contador1].estadoInicialChaveFechada[contador2]);
            printf("caso manobra %d \n", vetorPi[contador1].casoManobra);
            printf("noP %ld noR %ld noA %ld \n", vetorPi[contador1].nos[contador2].p, vetorPi[contador1].nos[contador2].r, vetorPi[contador1].nos[contador2].a);
        }
    }
}

/**
 * Copia os dados da estrutura VETORPI para agrupar os dados de múltiplas faltas em uma única entrada. 
 * O processo de definição da sequência de manobras utiliza essa estrutura resultante na recuperação das manobras.
 * @param vetorPiOriginal
 * @param vetorPiCopia
 * @param idNovo
 * @param numeroConfiguracoesFalta
 * @param idConfiguracoesFalta
 */
void copiaDadosVetorPIMultiplasFaltas(VETORPI *vetorPiOriginal, VETORPI *vetorPiCopia, long int idNovo, int numeroConfiguracoesFalta, long int *idConfiguracoesFalta) {
    int indice, indiceManobras;
    int contador;
    int totalManobras = 0;
    long int idConfiguracao;
    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
        idConfiguracao = idConfiguracoesFalta[contador];
        totalManobras += vetorPiOriginal[idConfiguracao].numeroManobras;
    }

    vetorPiCopia[idNovo].numeroManobras = totalManobras;
    vetorPiCopia[idNovo].casoManobra = 1;
    vetorPiCopia[idNovo].idChaveAberta = Malloc(long int, totalManobras + 1);
    vetorPiCopia[idNovo].idChaveFechada = Malloc(long int, totalManobras + 1);
    vetorPiCopia[idNovo].estadoInicialChaveAberta = Malloc(BOOL, totalManobras + 1);
    vetorPiCopia[idNovo].estadoInicialChaveFechada = Malloc(BOOL, totalManobras + 1);
    vetorPiCopia[idNovo].nos = Malloc(NOSPRA, totalManobras + 1);
    indiceManobras = 0;
    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
        idConfiguracao = idConfiguracoesFalta[contador];
        if (vetorPiOriginal[idConfiguracao].numeroManobras > 0)
            for (indice = 0; indice < vetorPiOriginal[idConfiguracao].numeroManobras; indice++) {
                vetorPiCopia[idNovo].idChaveAberta[indiceManobras] = vetorPiOriginal[idConfiguracao].idChaveAberta[indice];
                vetorPiCopia[idNovo].estadoInicialChaveAberta[indiceManobras] = vetorPiOriginal[idConfiguracao].estadoInicialChaveAberta[indice];
                vetorPiCopia[idNovo].idChaveFechada[indiceManobras] = vetorPiOriginal[idConfiguracao].idChaveFechada[indice];
                vetorPiCopia[idNovo].estadoInicialChaveFechada[indiceManobras] = vetorPiOriginal[idConfiguracao].estadoInicialChaveFechada[indice];
                vetorPiCopia[idNovo].nos[indiceManobras].a = vetorPiOriginal[idConfiguracao].nos[indice].a;
                vetorPiCopia[idNovo].nos[indiceManobras].p = vetorPiOriginal[idConfiguracao].nos[indice].p;
                vetorPiCopia[idNovo].nos[indiceManobras].r = vetorPiOriginal[idConfiguracao].nos[indice].r;
                indiceManobras++;
            }
    }
}

/**
 * Percorre a estrutura do tipo VETORPI, que contém as configurações intermediárias geradas até obter a configuração original,
 * removendo as chaves que retornaram ao estado inicial. Existem dois casos de alteração nas manobras:
 * Caso 2: quando as duas chaves da manobra retornaram ao estado inicial. Nesse caso utiliza o método removeChaveEstadoInicialCaso2 @see removeChaveEstadoInicialCaso2.
 * Caso 3: quando somente uma das chaves da manobra retornaram ao estado inicial. Nesse caso utiliza o método removeChaveEstadoInicialCaso3 @see removeChaveEstadoInicialCaso3.
 * @param grafoSetoresParam
 * @param numeroSetores
 * @param vetorPiParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroTrafos
 * @param numeroSetoresFalta
 * @param listaChavesParam
 * @return 
 */
/*VETORPI * removeChavesEstadoInicial(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam) {
    int totalConfiguracoes, contador, somaChaveamentos = 0;
    long int indiceInd;
    long int *indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
    VETORPI *vetorPiChav;
    CONFIGURACAO *configuracoesChav;
    MATRIZPI *matrizPIChav;
    int tamanhoAlocacao;
    int idConfiguracao = 0;
    long int noP, noR, noA, colunaPI;
    int indice1, indiceP, indiceA, indiceR, rnpP, rnpA, indiceL;
    int tamanhoSubarvore;
    int idNovoAncestral;
    long int *indicesFalta;
    int numeroConfiguracoesFalta;
    BOOL chaveamentoCorreto = false;
    //soma o total de pares de chaveamento para obter a configuração no AEMT
    // printf("totalConfiguracoes %d \n", totalConfiguracoes);
    for (contador = 0; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        somaChaveamentos += vetorPiParam[indiceInd].numeroManobras;
    }
    tamanhoAlocacao = somaChaveamentos * (somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
    //aloca o vetorPi, MatrizPI e Vetor de configurações para realizar o processo de remoção de chaves repetidas
    inicializaVetorPi(tamanhoAlocacao, &vetorPiChav);
    inicializaMatrizPI(grafoSetoresParam, &matrizPIChav, (tamanhoAlocacao * 100), numeroSetores);
    configuracoesChav = alocaIndividuo(configuracoesParam[idConfiguracaoParam[0]].numeroRNP, idConfiguracao, (tamanhoAlocacao), numeroTrafos);

    //copia a configuração inicial pré falta para iniciar a montagem auxiliar das soluções do AEMT para realizar a redução de chaves
    copiaIndividuo(configuracoesParam, configuracoesChav, 0, idConfiguracao, matrizPIChav);
    idConfiguracao++;
    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiParam, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);

    copiaDadosVetorPIMultiplasFaltas(vetorPiParam, vetorPiChav, idConfiguracao, numeroConfiguracoesFalta, indicesFalta);
    //replica a primeira configuração obtida na etapa de restabelecimento
    indiceInd = indicesFalta[numeroConfiguracoesFalta - 1];
    // copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
    vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
    copiaIndividuo(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav);

    //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
    for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        idConfiguracao++;
        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
        noP = vetorPiParam[indiceInd].nos[0].p;
        noA = vetorPiParam[indiceInd].nos[0].a;
        noR = vetorPiParam[indiceInd].nos[0].r;
        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, idConfiguracao - 1);
        indiceA = matrizPIChav[noA].colunas[colunaPI].posicao;
        rnpA = matrizPIChav[noA].colunas[colunaPI].idRNP;
        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noP, idConfiguracao - 1);
        indiceP = matrizPIChav[noP].colunas[colunaPI].posicao;
        rnpP = matrizPIChav[noP].colunas[colunaPI].idRNP;
        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noR, idConfiguracao - 1);
        indiceR = matrizPIChav[noR].colunas[colunaPI].posicao;
        //determinação do intervalo correspondente a subárvore do nó P na RNP
        indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnp[rnpP], indiceP);
        //calcula o tamanho da subárvore podada
        tamanhoSubarvore = indiceL - indiceP + 1;
        //realiza a alocação das que serão alteradas RNPs
        alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
        alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpP]);

        //constroi a rnp de origem nova
        constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnp[rnpP], &configuracoesChav[idConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP);
        constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracao - 1, &configuracoesChav[idConfiguracao].rnp[rnpA], idConfiguracao);
        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++) {
            if (indice1 != rnpP && indice1 != rnpA) {
                configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
            }
        }
    }
    //localiza no vetorPi o ancestral que possui chave que retornou ao estado inicial
    idConfiguracaoParam[0] = idConfiguracao;
    idConfiguracao++;
    //imprimeVetorPi(vetorPiChav, totalConfiguracoes + 1, listaChavesParam);
    totalConfiguracoes -= (numeroConfiguracoesFalta - 1);
    for (contador = 1; contador <= totalConfiguracoes; contador++) {
        //printf("contador %d idConfiguracaoancestral %ld casoManobra %d\n", contador, vetorPiChav[contador].idAncestral, vetorPiChav[contador].casoManobra);
        chaveamentoCorreto = false;
        if (vetorPiChav[contador].casoManobra == 2) {//as duas chaves retornaram ao estado inicial
            chaveamentoCorreto = removeChaveEstadoInicialCaso2(contador, vetorPiChav, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral);
        } else {
            if (vetorPiChav[contador].casoManobra == 3) { //somente uma das chaves retornou ao estadoInicial
                chaveamentoCorreto = removeChaveEstadoInicialCaso3(vetorPiChav, contador, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral);
            }
        }
        if (chaveamentoCorreto) {
            //localiza no vetorPI a configuracao que tem como ancestral a configuracao que apresentou chave repetida e atualiza para ter como ancestral a configuracao resultante do novo chaveamento
            indice1 = contador + 1;
            while (indice1 <= totalConfiguracoes && vetorPiChav[indice1].idAncestral != contador) {
                indice1++;
            }
            if (contador == totalConfiguracoes) {
                idConfiguracaoParam[0] = idNovoAncestral;
            }
            if (indice1 <= totalConfiguracoes) {
                vetorPiChav[indice1].idAncestral = idNovoAncestral;
            }
        }
    }
    desalocaMatrizPI(matrizPIChav, numeroSetores);
    free(matrizPIChav);
    free(configuracoesChav);

    return vetorPiChav;
}

//recupera utilizando o vetorPi da RNP todas as configuracoes geradas pelo AEMT até obter a configuracao final
/**
 * Recupera utilizando o VETORPI da RNP todas as configuracões geradas pelo algoritmo evolutivo em tabelas até obter a configuracao final.
 * @param vetorPiParam
 * @param idConfiguracaoParam
 * @param totalConfiguracoes
 * @return 
 */
VETORPI * removeChavesEstadoInicial(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam) {
    int totalConfiguracoes, contador, somaChaveamentos = 0;
    long int indiceInd;
    long int *indiceConfiguracoes;// = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
    VETORPI *vetorPiChav;
    CONFIGURACAO *configuracoesChav;
    MATRIZPI *matrizPIChav;
    int tamanhoAlocacao;
    int idConfiguracao = 0;
    long int noP, noR, noA, colunaPI;
    int indice1, indiceP, indiceA, indiceR, rnpP, rnpA, indiceL;
    int tamanhoSubarvore;
    int idNovoAncestral;
    long int *indicesFalta;
    int numeroConfiguracoesFalta;
    BOOL chaveamentoCorreto = false;
    //soma o total de pares de chaveamento para obter a configuração no AEMT
    // printf("totalConfiguracoes %d \n", totalConfiguracoes);
    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
    for (contador = 0; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        somaChaveamentos += vetorPiParam[indiceInd].numeroManobras;
    }
    tamanhoAlocacao = somaChaveamentos * (somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
    //aloca o vetorPi, MatrizPI e Vetor de configurações para realizar o processo de remoção de chaves repetidas
    inicializaVetorPi(tamanhoAlocacao, &vetorPiChav);
    inicializaMatrizPI(grafoSetoresParam, &matrizPIChav, (tamanhoAlocacao * 100), numeroSetores);
    configuracoesChav = alocaIndividuo(configuracoesParam[idConfiguracaoParam[0]].numeroRNP, idConfiguracao, (tamanhoAlocacao), numeroTrafos);

    //copia a configuração inicial pré falta para iniciar a montagem auxiliar das soluções do AEMT para realizar a redução de chaves
    copiaIndividuo(configuracoesParam, configuracoesChav, 0, idConfiguracao, matrizPIChav);

    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiParam, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
    //copia as configuracoes de falta
    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
        indiceInd = indicesFalta[contador];
        idConfiguracao++;
        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
        copiaIndividuo(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav);
        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
    }

    //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
    for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        idConfiguracao++;
        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
        noP = vetorPiParam[indiceInd].nos[0].p;
        noA = vetorPiParam[indiceInd].nos[0].a;
        noR = vetorPiParam[indiceInd].nos[0].r;
        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, idConfiguracao - 1);
        indiceA = matrizPIChav[noA].colunas[colunaPI].posicao;
        rnpA = matrizPIChav[noA].colunas[colunaPI].idRNP;
        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noP, idConfiguracao - 1);
        indiceP = matrizPIChav[noP].colunas[colunaPI].posicao;
        rnpP = matrizPIChav[noP].colunas[colunaPI].idRNP;
        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noR, idConfiguracao - 1);
        indiceR = matrizPIChav[noR].colunas[colunaPI].posicao;
        //determinação do intervalo correspondente a subárvore do nó P na RNP
        indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnp[rnpP], indiceP);
        //calcula o tamanho da subárvore podada
        tamanhoSubarvore = indiceL - indiceP + 1;
        //realiza a alocação das que serão alteradas RNPs
        alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
        alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpP]);

        //constroi a rnp de origem nova
        constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnp[rnpP], &configuracoesChav[idConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP);
        constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracao - 1, &configuracoesChav[idConfiguracao].rnp[rnpA], idConfiguracao);
        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++) {
            if (indice1 != rnpP && indice1 != rnpA) {
                configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
            }
        }
    }

    //localiza no vetorPi o ancestral que possui chave que retornou ao estado inicial
    idConfiguracaoParam[0] = idConfiguracao;
    idConfiguracao++;
    //imprimeVetorPi(vetorPiChav, totalConfiguracoes + 1, listaChavesParam);
    for (contador = 1; contador <= totalConfiguracoes; contador++) {
        //printf("contador %d idConfiguracaoancestral %ld casoManobra %d\n", contador, vetorPiChav[contador].idAncestral, vetorPiChav[contador].casoManobra);
        chaveamentoCorreto = false;
        if (vetorPiChav[contador].casoManobra == 2) {//as duas chaves retornaram ao estado inicial
            chaveamentoCorreto = removeChaveEstadoInicialCaso2(contador, vetorPiChav, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral);
        } else {
            if (vetorPiChav[contador].casoManobra == 3) { //somente uma das chaves retornou ao estadoInicial
                chaveamentoCorreto = removeChaveEstadoInicialCaso3(vetorPiChav, contador, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral);
            }
        }
        if (chaveamentoCorreto) {
            //localiza no vetorPI a configuracao que tem como ancestral a configuracao que apresentou chave repetida e atualiza para ter como ancestral a configuracao resultante do novo chaveamento
            indice1 = contador + 1;
            while (indice1 <= totalConfiguracoes && vetorPiChav[indice1].idAncestral != contador) {
                indice1++;
            }
            if (contador == totalConfiguracoes) {
                idConfiguracaoParam[0] = idNovoAncestral;
            }
            if (indice1 <= totalConfiguracoes) {
                vetorPiChav[indice1].idAncestral = idNovoAncestral;
            }
        }
    }

    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam[0], &totalConfiguracoes);
    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiChav, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
    if (numeroConfiguracoesFalta > 1) {
        idConfiguracao++;
        copiaDadosVetorPIMultiplasFaltas(vetorPiChav, vetorPiChav, idConfiguracao, numeroConfiguracoesFalta, indicesFalta);
        //replica a primeira configuração obtida na etapa de restabelecimento
        indiceInd = indicesFalta[numeroConfiguracoesFalta - 1];
        // copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
        vetorPiChav[idConfiguracao].idAncestral = 0;
        copiaIndividuo(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav);
        //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
        for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
            indiceInd = indiceConfiguracoes[contador];
            idConfiguracao++;
            copiaDadosVetorPI(vetorPiChav, vetorPiChav, indiceInd, idConfiguracao);
            noP = vetorPiChav[indiceInd].nos[0].p;
            noA = vetorPiChav[indiceInd].nos[0].a;
            noR = vetorPiChav[indiceInd].nos[0].r;
            vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
            colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, idConfiguracao - 1);
            indiceA = matrizPIChav[noA].colunas[colunaPI].posicao;
            rnpA = matrizPIChav[noA].colunas[colunaPI].idRNP;
            colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noP, idConfiguracao - 1);
            indiceP = matrizPIChav[noP].colunas[colunaPI].posicao;
            rnpP = matrizPIChav[noP].colunas[colunaPI].idRNP;
            colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noR, idConfiguracao - 1);
            indiceR = matrizPIChav[noR].colunas[colunaPI].posicao;
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnp[rnpP], indiceP);
            //calcula o tamanho da subárvore podada
            tamanhoSubarvore = indiceL - indiceP + 1;
            //realiza a alocação das que serão alteradas RNPs
            alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
            alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpP]);

            //constroi a rnp de origem nova
            constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnp[rnpP], &configuracoesChav[idConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP);
            constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracao - 1, &configuracoesChav[idConfiguracao].rnp[rnpA], idConfiguracao);
            for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++) {
                if (indice1 != rnpP && indice1 != rnpA) {
                    configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
                }
            }
        }

        idConfiguracaoParam[0] = idConfiguracao;
    }
    desalocaMatrizPI(matrizPIChav, numeroSetores);
    free(matrizPIChav);
    free(configuracoesChav);

    return vetorPiChav;
}

long int *recuperaConfiguracoesIntermediarias(VETORPI *vetorPiParam, long int idConfiguracaoParam, int *totalConfiguracoes) {

    int contador, contador2 = 0;
    long int *idConfiguracoesIntermediarias;
    long int idConfiguracao = idConfiguracaoParam;
    long int vetIdAux[1000];
    totalConfiguracoes[0] = 0;
    while (idConfiguracao > 0) {
        if (vetorPiParam[idConfiguracao].numeroManobras > 0) {

            vetIdAux[contador2] = idConfiguracao;
            totalConfiguracoes[0]++;
            contador2++;
        }
        idConfiguracao = vetorPiParam[idConfiguracao].idAncestral;
    }
    idConfiguracoesIntermediarias = malloc(totalConfiguracoes[0] * sizeof(long int));
    contador = totalConfiguracoes[0];
    contador2 = 0;
    while (contador > 0) {
        idConfiguracoesIntermediarias[contador2] = vetIdAux[contador - 1];
        contador2++;
        contador--;
    }
    return idConfiguracoesIntermediarias;
}
/**
 * Método principal que gerencia o processo de definição da sequência de manobras.
 * Primeiramente, remove da ordem de configurações geradas pelo algoritmo evolutivo as manobras envolvendo chaves que retornaram ao estado inicial. @see removeChavesEstadoInicial
 * Depois verifica se a configuração passada como parâmetro é factível ou não.
 * Caso seja factível executa o método otimizaChaveamento @see otimizaChaveamento.
 * Caso seja infactível é necessário executar o corte de carga. Para isso é utilizado o método otimizaChaveamentoCorteCarga @see otimizaChaveamentoCorteCarga.
 * @param grafoSDRParam
 * @param numeroBarras
 * @param dadosAlimentadorSDRParam
 * @param dadosTrafoSDRParam
 * @param maximoCorrente
 * @param Z
 * @param rnpSetoresParam
 * @param SBase
 * @param grafoSetoresParam
 * @param numeroSetores
 * @param setoresFalta
 * @param numeroSetoresFalta
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param vetorPiParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param listaChavesParam
 * @param seed
 */
void sequenciaChaveamento(GRAFO *grafoSDRParam, long int numeroBarras, DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
		LISTACHAVES *listaChavesParam, int seed, long int numeroChavesParam) {
    long int indiceInd, indiceManobras;
    VETORPI *vetorPiOtimizado;
    indiceInd = idConfiguracaoParam;
    //existem manobras para serem impressas
    //imprimeVetorPi(vetorPiParam, idConfiguracaoParam, listaChavesParam);
    vetorPiOtimizado = removeChavesEstadoInicial(grafoSetoresParam, numeroSetores, vetorPiParam, configuracoesParam, &idConfiguracaoParam, numeroTrafosParam, numeroSetoresFalta, listaChavesParam);
    //imprimeVetorPi(vetorPiOtimizado, idConfiguracaoParam, listaChavesParam);
    //    inicializaDadosEletricosChaveamento(, );
    if (verificaFactibilidade(configuracoesParam[indiceInd], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
        otimizaChaveamento(dadosAlimentadorSDRParam, dadosTrafoSDRParam,
                maximoCorrente, Z, rnpSetoresParam, SBase, grafoSetoresParam,
                numeroSetores, setoresFalta, numeroSetoresFalta, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, configuracoesParam[0], vetorPiOtimizado, idConfiguracaoParam,
                grafoSDRParam, numeroBarras, listaChavesParam, configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais,
                configuracoesParam[indiceInd].objetivo.consumidoresSemFornecimento,
                configuracoesParam[indiceInd].objetivo.consumidoresEspeciaisSemFornecimento, seed, indiceInd, configuracoesParam, numeroChavesParam);
    } else {
        otimizaChaveamentoCorteCarga(dadosAlimentadorSDRParam, dadosTrafoSDRParam,
                maximoCorrente, Z, rnpSetoresParam, SBase, grafoSetoresParam,
                numeroSetores, setoresFalta, numeroSetoresFalta, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, configuracoesParam[0], vetorPiOtimizado, idConfiguracaoParam,
                grafoSDRParam, numeroBarras, listaChavesParam, configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais,
                configuracoesParam[indiceInd].objetivo.consumidoresSemFornecimento,
                configuracoesParam[indiceInd].objetivo.consumidoresEspeciaisSemFornecimento, seed, indiceInd, configuracoesParam, numeroChavesParam);
    }
}

/**
 * Recupera do array do tipo VETORPI as posições referentes aos ancestrais da configuração final informada.
 * Copia essas posições para um novo vetorPI.
 * @param vetorPiParam
 * @param numeroSetoresFalta
 * @param configuracoes
 * @param numeroConfiguracoesFalta
 * @return 
 */
long int *recuperaVetorPiMultiplasFaltas(VETORPI *vetorPiParam, int numeroSetoresFalta,
        int numeroConfiguracoes, long int *configuracoes, int *numeroConfiguracoesFalta) {
    int contador;
    long int *indicesFalta = Malloc(long int, numeroSetoresFalta);
    int contadorFaltas = 0;
    long int idConfiguracao;
    //localizou a partir do vetor pi as configurações iniciais que dizem respeito ao restabelecimento.
    for (contador = 0; ((contador < numeroConfiguracoes) && (contador < numeroSetoresFalta)); contador++) {
        idConfiguracao = configuracoes[contador];
        if (vetorPiParam[idConfiguracao].idChaveFechada[0] == -1) {
            indicesFalta[contadorFaltas] = idConfiguracao;
            contadorFaltas++;
        }
    }
    numeroConfiguracoesFalta[0] = contadorFaltas;
    return indicesFalta;
}
/**
 * Retorna no parâmetro tipoChave a descrição do tipo da chave em formato de texto.
 * @param codChave
 * @param listaChavesParam
 * @param tipoChave
 */
void tiposChave(long int codChave, LISTACHAVES *listaChavesParam, char *tipoChave) {
    if (listaChavesParam[codChave].tipoChave == chaveManual) {
        switch (listaChavesParam[codChave].subTipoChave) {
            case 0:
                strcpy(tipoChave, "Chave Fusivel");
                break;
            case 1: //chave seca
                strcpy(tipoChave, "Chave Seca");
                break;
            case 2: // chave carga
                strcpy(tipoChave, "Manual com Carga");
                break;
            case 3: //chave curto circuito
                strcpy(tipoChave, "Religador sem Automação");
                break;
        }

    } else {
        switch (listaChavesParam[codChave].subTipoChave) {
            case 2: // chave carga
                strcpy(tipoChave, "Chaves a Gás Automática");
                break;
            case 3: //chave curto circuito
                strcpy(tipoChave, "Religador com Automação");
                break;
        }
    }
}
/**
 * Grava em arquivo a sequência de manobras definida. Imprime a manobra que deve ser feita (abrir ou fechara), o código da chave,
 * o tipo da chave e o código 0 se não existe regulador de tensão ou 1 se existe regulador de tensão. 
 * @param chaveamento
 * @param nomeArquivo
 * @param listaChavesParam
 */
void salvaChaveamento(ELEMENTO *chaveamento, char *nomeArquivo, LISTACHAVES *listaChavesParam) {
    int contadorAux;
    FILE *arquivo;

    char *tipoChave = Malloc(char, 100);
    //   
    arquivo = fopen(nomeArquivo, "w");
    for (contadorAux = 0; contadorAux < chaveamento->numeroChaveamentos; contadorAux++) {
        tiposChave(chaveamento->idChaves[contadorAux], listaChavesParam, tipoChave);
        if (chaveamento->estadoFinal[contadorAux] == 0) {
            fprintf(arquivo, "ABRIR\t%s\t%s\t%d\n", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, tipoChave, chaveamento->reguladorTensao[contadorAux]);

        } else {
            fprintf(arquivo, "FECHAR\t%s\t%s\t%d\n", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, tipoChave, chaveamento->reguladorTensao[contadorAux]);
        }
        //   printf("\t %ld", chaveamento->idChaves[contadorAux]);
    }
    fclose(arquivo);
}

/**
 * Esse método realiza a remoção da chave que retornou ao estado inicial para o caso 3. Nesse caso somente uma das chaves da manobra retornou ao estado inicial.
 * Primeiramente verifica se a chave que retornou ao estado inicial foi com manobra para abrir ou fechar. 
 * Se foi com abrir faz a busca nos ancestrais pela posição onde ela foi fechada, inicialmente, e vice e versa.
 * Encontrado esse ancestral obtém um novo par de manobra combinando as chaves diferentes das duas configurações.
 * Obtido o novo par verifica em que posição o novo par será inserido, existem duas posições possíveis. No ponto onde a chave foi alterada ou no lugar que ela voltou ao estado inicial.
 * Para definir essa posição simula toda a sequência de manobras à partir do ancestral para cada uma das inserções.
 * Se todos as configurações intermediárias mantiverem a característica radial, a nova sequência é informada.
 * @param vetorPiChav
 * @param idConfiguracaoParam
 * @param matrizPIChav
 * @param configuracoesChav
 * @param idProxConfiguracao
 * @param totalConfiguracoes
 * @param idNovoAncestral
 * @return 
 */
BOOL removeChaveEstadoInicialCaso3(VETORPI *vetorPiChav, int idConfiguracaoParam, MATRIZPI *matrizPIChav, CONFIGURACAO *configuracoesChav, int *idProxConfiguracao, int totalConfiguracoes, int *idNovoAncestral) {
    /*As variáveis xxIni serão utilizadas para armazenar os dados da configuração onde ocorreu a primeira mudança na chave
     As variáveis xxFinal contém os dados na configuração onde a chave retornou ao seu estado inicial.*/
    int contador, contadorChav, novaPosicao, contadorConfiguracoes, indice1;
    BOOL encontrado = false;
    int idConfiguracaoIni, posIni, idConfiguracaoAncestral;
    long int noPIni, noRIni, noAIni, noPFinal, noRFinal, noAFinal, noPNovo, noRNovo, noANovo;
    long int idChaveAbertaR, idChaveFechadaR; //armazena o id das chaves que serao rearranjadas
    BOOL chaveamentoCorreto = true;
    int indiceConfiguracoes[totalConfiguracoes];
    int configuracao, noA, noP, noR, rnpA, rnpP, rnpR, indiceL, indiceA, indiceP, indiceR, colunaPi;
    //localiza no vetorPi a alteração anterior da chave
    contador = vetorPiChav[idConfiguracaoParam].idAncestral;
    contadorConfiguracoes = 0;
    indiceConfiguracoes[contadorConfiguracoes] = idConfiguracaoParam;
    int idNovaConfiguracao = idProxConfiguracao[0];
    while (contador > 0 && !encontrado) {

        if (vetorPiChav[idConfiguracaoParam].estadoInicialChaveAberta[0]) //nesse caso é a chave que deve ser aberta que retornou ao estado inicial. A busca pelo configuração onde ocorreu a alteração de estado da chave deve ser no vetor de chave fechada
        {
            contadorChav = 0;
            while (contadorChav < vetorPiChav[contador].numeroManobras && !encontrado) {
                if (vetorPiChav[contador].idChaveFechada[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveAberta[0]) {
                    encontrado = true;
                    idConfiguracaoIni = contador;
                    posIni = contadorChav;
                }
                contadorChav++;
            }
        } else//nesse caso é a chave que deve ser fechada que retornou ao estado inicial. A busca pelo configuração onde ocorreu a alteração de estado da chave deve ser no vetor de chave aberta
        {
            contadorChav = 0;
            while (contadorChav < vetorPiChav[contador].numeroManobras && !encontrado) {
                if (vetorPiChav[contador].idChaveAberta[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveFechada[0]) {
                    encontrado = true;
                    idConfiguracaoIni = contador;
                    posIni = contadorChav;
                }
                contadorChav++;
            }

        }
        //salva os identificadores das configuracoes entre a repetição de chaves
        contadorConfiguracoes++;
        indiceConfiguracoes[contadorConfiguracoes] = contador;
        contador = vetorPiChav[contador].idAncestral;
    }
    //recupera os nós P, R, A da primeira mudança na chave
    noPIni = vetorPiChav[idConfiguracaoIni].nos[posIni].p;
    noRIni = vetorPiChav[idConfiguracaoIni].nos[posIni].r;
    noAIni = vetorPiChav[idConfiguracaoIni].nos[posIni].a;
    //printf("1 alteracao noP %ld noR %ld noA %ld \n", noPIni, noRIni, noAIni);
    //imprimeIndividuo(configuracoesChav[idConfiguracaoIni]);
    //recupera os nós P, R, A da segunda mudança na chave
    noPFinal = vetorPiChav[idConfiguracaoParam].nos[0].p;
    noRFinal = vetorPiChav[idConfiguracaoParam].nos[0].r;
    noAFinal = vetorPiChav[idConfiguracaoParam].nos[0].a;
    // printf("2 alteracao noP %ld noR %ld noA %ld \n", noPFinal, noRFinal, noAFinal);
    //     imprimeIndividuo(configuracoesChav[idConfiguracaoParam]);
    if (vetorPiChav[idConfiguracaoParam].estadoInicialChaveAberta[0]) //nesse caso é a chave que deve ser aberta que retornou ao estado inicial. A busca pelo configuração onde ocorreu a alteração de estado da chave deve ser no vetor de chave fechada
    {
        idChaveAbertaR = vetorPiChav[idConfiguracaoIni].idChaveAberta[posIni]; //a chave a ser aberta no novo chaveamento e a chave aberta do par inicial
        idChaveFechadaR = vetorPiChav[idConfiguracaoParam].idChaveFechada[0]; //a chave a ser fechada no novo chaveamento e a chave fechada do par onde houve chave voltando ao estado original
        noPNovo = noPIni;
        noRNovo = noRFinal;
        noANovo = noAFinal;
        //primeiro tenta inserir o par de chaveamento rearranjado na posição onde a chave que retornou ao estado inicial foi fechada
        novaPosicao = idConfiguracaoIni;
        contador = contadorConfiguracoes;
        idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
        if (vetorPiChav[idConfiguracaoIni].numeroManobras <= 1) {
            chaveamentoCorreto = simulaChaveamentoPosicaoInicial(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                    idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                    idChaveFechadaR, idNovoAncestral, idConfiguracaoIni);

            //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
            if (!chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
            {
                //não foi possível realizar a sequencia de chaveamento proposta e deve-se testar o par rearranjado em outra posição
                novaPosicao = idConfiguracaoParam;
                contador = contadorConfiguracoes - 1;
                idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
                chaveamentoCorreto = true;
                chaveamentoCorreto = simulaChaveamentoPosicaoFinal(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                        idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                        idChaveFechadaR, idNovoAncestral, idConfiguracaoIni);
            }
        } else {
            //Esse caso indica que a chave que retornou ao estado original como chave aberta foi primeiramente alterada no restabelecimento.
            //Nessa situação o par rearranjado deve obrigatoriamente ser testado na posição do restabelecimento.
            //Para fazer isso será utilizada a configuração resultante do restabelecimento e simulada a aplicação dos nós p, r, a da configuracao onde
            //houve a repetição da chave.
            while (contador > 0 && chaveamentoCorreto) {
                configuracao = indiceConfiguracoes[contador];
                contadorChav = 0;
                if (configuracao == idConfiguracaoIni) {
                    //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                    //Nesse caso será utilizado para simular o par de chaveamento do individuo que fez a chave voltar ao estado inicial.
                    //para não ter que realizar todoo o processo do restabelecimento de novo.
                    noP = noPFinal;
                    noR = noRFinal;
                    noA = noAFinal;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    //insere dados no vetorPi
                    while (contadorChav < vetorPiChav[idConfiguracaoIni].numeroManobras) {
                        if (contadorChav == posIni) {
                            vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                            vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                            vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                            vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                            vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                            //  vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                            vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                        }
                        contadorChav++;
                    }
                    //altera o individuo para conter o chaveamento
                    copiaIndividuo(configuracoesChav, configuracoesChav, idConfiguracaoIni, idNovaConfiguracao, matrizPIChav);
                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noPNovo, noANovo, noRNovo, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                    if (rnpP != rnpR) {
                        chaveamentoCorreto = false;
                    } else {
                        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idNovaConfiguracao, matrizPIChav, vetorPiChav);
                        //determinação do intervalo correspondente a subárvore do nó P na RNP
                        indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP);
                        //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                        if (indiceR < indiceP || indiceR > indiceL) {
                            chaveamentoCorreto = false;
                        } else {
                            if (rnpA == rnpP) {
                                //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                if (indiceA >= indiceP && indiceA <= indiceL) {
                                    chaveamentoCorreto = false;
                                } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                {
                                    //realiza a alocação da RNP que será alterada
                                    alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                    constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idNovaConfiguracao, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                }
                            } else //aplica o chaveamento e gera uma nova configuracao simulada
                            {

                                int tamanhoSubarvore;
                                tamanhoSubarvore = indiceL - indiceP + 1;
                                //realiza a alocação das que serão alteradas RNPs
                                alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpA]);
                                alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                //constroi a rnp de origem nova
                                constroiRNPOrigem(configuracoesChav[idConfiguracaoIni].rnp[rnpP], &configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idNovaConfiguracao, rnpP);
                                constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoIni, &configuracoesChav[idNovaConfiguracao].rnp[rnpA], idNovaConfiguracao);

                            }
                            vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                            vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                            idConfiguracaoAncestral = idNovaConfiguracao;
                            idNovaConfiguracao++;
                        }
                    }

                } else //aplica a tripla de nos anterior
                {
                    noP = vetorPiChav[configuracao].nos[contadorChav].p;
                    noR = vetorPiChav[configuracao].nos[contadorChav].r;
                    noA = vetorPiChav[configuracao].nos[contadorChav].a;

                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                    if (rnpP != rnpR) {
                        chaveamentoCorreto = false;
                    } else {
                        //determinação do intervalo correspondente a subárvore do nó P na RNP
                        indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                        //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                        if (indiceR < indiceP && indiceR > indiceL) {
                            chaveamentoCorreto = false;
                        } else {
                            if (rnpA == rnpP) {
                                //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                if (indiceA >= indiceP && indiceA <= indiceL) {
                                    chaveamentoCorreto = false;
                                } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                {
                                    //realiza a alocação da RNP que será alterada
                                    alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                    constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                    for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                        if (indice1 != rnpP) {
                                            configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                        }
                                    }
                                }
                            } else //aplica o chaveamento e gera uma nova configuracao simulada
                            {
                                obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                        indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                            }
                            copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                            vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                            vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                            idConfiguracaoAncestral = idNovaConfiguracao;
                            idNovaConfiguracao++;
                        }
                    }
                }
                contador--;
            }
            if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
            {
                if (idProxConfiguracao[0] == idNovaConfiguracao)
                    idNovoAncestral[0] = vetorPiChav[idConfiguracaoIni].idAncestral;
                else
                    idNovoAncestral[0] = idNovaConfiguracao - 1;
            }
            idProxConfiguracao[0] = idNovaConfiguracao;
        }
    } else { //nesse caso a chave fechou e a que esta repetida.
        idChaveAbertaR = vetorPiChav[idConfiguracaoParam].idChaveAberta[0]; //a chave a ser aberta no novo chaveamento e a chave aberta do par onde houve chave voltando ao estado original
        idChaveFechadaR = vetorPiChav[idConfiguracaoIni].idChaveFechada[posIni]; //a chave a ser fechada no novo chaveamento e a chave fechada do par inicial
        noPNovo = noPFinal;
        noRNovo = noRIni;
        noANovo = noAIni;
        //primeiro tenta inserir o par de chaveamento rearranjado na posição onde a chave que retornou ao estado inicial foi fechada

        //não foi possível realizar a sequencia de chaveamento proposta e deve-se testar o par rearranjado em outra posição
        novaPosicao = idConfiguracaoParam;
        contador = contadorConfiguracoes - 1;
        idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
        chaveamentoCorreto = simulaChaveamentoPosicaoFinal(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                idChaveFechadaR, idNovoAncestral, idConfiguracaoIni);
        if (!chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
        {
            //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
            novaPosicao = idConfiguracaoIni;
            contador = contadorConfiguracoes;
            idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
            chaveamentoCorreto = simulaChaveamentoPosicaoInicial(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                    idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                    idChaveFechadaR, idNovoAncestral, idConfiguracaoIni);
        }

    }
    return chaveamentoCorreto;
}
/**
  * Esse método realiza a remoção da chave que retornou ao estado inicial para o caso 2. Nesse caso as duas chaves da manobra retornaram ao estado inicial.
 * A seguir busca-se a posição em que cada uma das chaves retornou ao seu estado inicial.
 * Se as duas chaves foram anteriormente manobradas no mesmo par, simplemente remove-se as manobras.
 * Caso elas tenham sido manobradas em ancestrais diferentes é obtido um novo par de manobra combinando as chaves diferentes das duas configurações.
 * Obtido o novo par verifica em que posição o novo par será inserido, existem duas posições possíveis. No ponto onde a chave aberta foi alterada ou no lugar em que a chave fechada foi alterada.
 * Para definir essa posição simula toda a sequência de manobras à partir do ancestral para cada uma das inserções.
 * Se todos as configurações intermediárias mantiverem a característica radial, a nova sequência é informada.
 * @param idConfiguracaoParam
 * @param vetorPiChav
 * @param matrizPIChav
 * @param configuracoesChav
 * @param idProxConfiguracao
 * @param totalConfiguracoes
 * @param idNovoAncestral
 * @return
 */
BOOL removeChaveEstadoInicialCaso2(int idConfiguracaoParam, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav, CONFIGURACAO *configuracoesChav, int *idProxConfiguracao, int totalConfiguracoes, int *idNovoAncestral) {
    /*As variáveis xxIni serão utilizadas para armazenar os dados da configuração onde ocorreu a primeira mudança na chave
     As variáveis xxFinal contém os dados na configuração onde a chave retornou ao seu estado inicial.*/
    int contador, contadorChav, novaPosicao, contadorConfiguracoes, indice1;
    BOOL encontradoCA = false;
    BOOL encontradoCF = false;
    int idConfiguracaoIniCA, posIniCA, idConfiguracaoIniCF, posIniCF, idConfiguracaoAncestral;
    long int noA, noP, noR, noPIniA, noRIniA, noAIniA, noPIniF, noRIniF, noAIniF, noPNovo, noRNovo, noANovo;
    long int idChaveAbertaR, idChaveFechadaR; //armazena o id das chaves que serao rearranjadas
    BOOL chaveamentoCorreto = true;
    int indiceConfiguracoes[totalConfiguracoes];
    int configuracao, rnpA, rnpP, rnpR, indiceL, indiceA, indiceP, indiceR;


    //localiza no vetorPi a alteração anterior da chave
    contador = vetorPiChav[idConfiguracaoParam].idAncestral;
    contadorConfiguracoes = 0;
    indiceConfiguracoes[contadorConfiguracoes] = idConfiguracaoParam;
    int idNovaConfiguracao = idProxConfiguracao[0];
    while (contador > 0 && (!encontradoCA || !encontradoCF)) {
        contadorChav = 0;
        while (contadorChav < vetorPiChav[contador].numeroManobras && (!encontradoCA || !encontradoCF)) {
            if (vetorPiChav[contador].idChaveFechada[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveAberta[0]) {
                encontradoCA = true;
                idConfiguracaoIniCA = contador;
                posIniCA = contadorChav;
            }
            if (vetorPiChav[contador].idChaveAberta[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveFechada[0]) {
                encontradoCF = true;
                idConfiguracaoIniCF = contador;
                posIniCF = contadorChav;
            }
            contadorChav++;
        }
        //salva os identificadores das configuracoes entre a repetição de chaves
        contadorConfiguracoes++;
        indiceConfiguracoes[contadorConfiguracoes] = contador;
        contador = vetorPiChav[contador].idAncestral;
    }
    //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
    idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
    //nesse caso as chaves se repetem na mesma configuração e deve ser simulado somente as configurações intermediárias entre a repetição
    if ((idConfiguracaoIniCA == idConfiguracaoIniCF) && (posIniCA == posIniCF)) {
        contador = contadorConfiguracoes - 1;
        while (contador > 0 && chaveamentoCorreto) {
            configuracao = indiceConfiguracoes[contador];
            contadorChav = 0;
            copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
            noP = vetorPiChav[configuracao].nos[contadorChav].p;
            noR = vetorPiChav[configuracao].nos[contadorChav].r;
            noA = vetorPiChav[configuracao].nos[contadorChav].a;
            //verifica se o par de chaveamento continua valido
            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
            if (rnpP != rnpR) {
                chaveamentoCorreto = false;
            } else {
                //determinação do intervalo correspondente a subárvore do nó P na RNP
                indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                if (rnpA == rnpP) {
                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                    if (indiceA >= indiceP && indiceA <= indiceL) {
                        chaveamentoCorreto = false;
                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                    {
                        //realiza a alocação da RNP que será alterada
                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                            if (indice1 != rnpP) {
                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                            }
                        }
                    }
                } else //aplica o chaveamento e gera uma nova configuracao simulada
                {
                    obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                }
                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                idConfiguracaoAncestral = idNovaConfiguracao;
                idNovaConfiguracao++;
            }
            contador--;
        }
        if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
        {
            if (idProxConfiguracao[0] == idNovaConfiguracao)
                idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
            else
                idNovoAncestral[0] = idNovaConfiguracao - 1;
        }
        idProxConfiguracao[0] = idNovaConfiguracao;

    } else {
        //nesse caso as duas chaves que retornaram ao estado inicial estão em configurações anteriores diferentes.
        //O novo chavemento será composto pela chave que abre na configuração que repete a chave que abriu e pela que chave que fecha na configuração que repete a chave que fechou.
        //recupera os nós P, R, A da primeira mudança na chave aberta, nesse ponto a chave foi fechada. A chave que abriu nesse par será utilizada no par rearranjado
        noPIniA = vetorPiChav[idConfiguracaoIniCA].nos[posIniCA].p;
        noRIniA = vetorPiChav[idConfiguracaoIniCA].nos[posIniCA].r;
        noAIniA = vetorPiChav[idConfiguracaoIniCA].nos[posIniCA].a;
        //printf("1 alteracao noP %ld noR %ld noA %ld \n", noPIni, noRIni, noAIni);
        //imprimeIndividuo(configuracoesChav[idConfiguracaoIni]);
        //recupera os nós P, R, A da primeira mudança na chave fechada, nesse ponto a chave foi aberta. A chave que fechou nesse par será utilizada no par rearranjado
        noPIniF = vetorPiChav[idConfiguracaoIniCF].nos[posIniCF].p;
        noRIniF = vetorPiChav[idConfiguracaoIniCF].nos[posIniCF].r;
        noAIniF = vetorPiChav[idConfiguracaoIniCF].nos[posIniCF].a;

        //definição do novo par de chaveamento. Esse par será composto pelas chaves diferentes presentes nos chaveamentos que contém as chaves que retornaram ao estado inicial.
        //o nó noP tem origem na chave que será aberta no par de chaveamento.
        //os nós nR e noA tem origem na chave que serpa fechada no par de chaveamento
        noPNovo = noPIniA;
        noRNovo = noRIniF;
        noANovo = noAIniF;
        idChaveAbertaR = vetorPiChav[idConfiguracaoIniCA].idChaveAberta[posIniCA];
        idChaveFechadaR = vetorPiChav[idConfiguracaoIniCF].idChaveFechada[posIniCF];
        //verifica se o par rearranjado é composto por chaves diferentes

        if (idChaveAbertaR != idChaveFechadaR) {
            //O par de chaveamento rearranjado pode ser colocado em 3 posições.
            if (vetorPiChav[idConfiguracaoIniCA].numeroManobras<=1) {
                //1)No local que a chave que fechou teve sua primeira alteração de estado.
                novaPosicao = idConfiguracaoIniCF;
                contador = contadorConfiguracoes;
                //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                while (contador > 0 && chaveamentoCorreto) {
                    configuracao = indiceConfiguracoes[contador];
                    contadorChav = 0;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    if (configuracao != idConfiguracaoIniCA) { //é necessário pular a configuração que teve a primeira alteração da chave para chave aberta
                        if (configuracao == novaPosicao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                        {
                            noP = noPNovo;
                            noR = noRNovo;
                            noA = noANovo;
                            //insere dados no vetorPi
                            if (contadorChav == posIniCF) {
                                vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                            }

                        } else //aplica a tripla de nos anterior
                        {
                            noP = vetorPiChav[configuracao].nos[contadorChav].p;
                            noR = vetorPiChav[configuracao].nos[contadorChav].r;
                            noA = vetorPiChav[configuracao].nos[contadorChav].a;

                        }
                        //  printf("nop %ld, noA %ld noR %ld \n",noP, noA, noR);
                        //verifica se o par de chaveamento continua valido
                        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                        if (rnpP != rnpR) {
                            chaveamentoCorreto = false;
                        } else {
                            //determinação do intervalo correspondente a subárvore do nó P na RNP
                            indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                            if (indiceR < indiceP && indiceR > indiceL) {
                                chaveamentoCorreto = false;
                            } else {
                                if (rnpA == rnpP) {
                                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                    if (indiceA >= indiceP && indiceA <= indiceL) {
                                        chaveamentoCorreto = false;
                                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                    {
                                        //realiza a alocação da RNP que será alterada
                                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                            if (indice1 != rnpP) {
                                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                            }
                                        }
                                    }
                                } else //aplica o chaveamento e gera uma nova configuracao simulada
                                {
                                    obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                                }
                                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                idConfiguracaoAncestral = idNovaConfiguracao;
                                idNovaConfiguracao++;
                            }
                        }
                    }
                    contador--;
                }
                //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                {
                    if (idProxConfiguracao[0] == idNovaConfiguracao) {
                        idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                    } else {
                        idNovoAncestral[0] = idNovaConfiguracao - 1;
                    }
                    idProxConfiguracao[0] = idNovaConfiguracao;
                } else {

                    //2)No local que a chave que abriu teve sua primeira alteração de estado
                    novaPosicao = idConfiguracaoIniCA;
                    contador = contadorConfiguracoes;
                    //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                    idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                    while (contador > 0 && chaveamentoCorreto) {
                        configuracao = indiceConfiguracoes[contador];
                        contadorChav = 0;
                        if (configuracao != idConfiguracaoIniCF) {
                            copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                            if (configuracao == novaPosicao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                            {
                                noP = noPNovo;
                                noR = noRNovo;
                                noA = noANovo;
                                //insere dados no vetorPi
                                if (contadorChav == posIniCA) {
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                                }

                            } else //aplica a tripla de nos anterior
                            {
                                noP = vetorPiChav[configuracao].nos[contadorChav].p;
                                noR = vetorPiChav[configuracao].nos[contadorChav].r;
                                noA = vetorPiChav[configuracao].nos[contadorChav].a;
                            }
                            //verifica se o par de chaveamento continua valido
                            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                            if (rnpP != rnpR) {
                                chaveamentoCorreto = false;
                            } else {
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
                                indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                                if (indiceR < indiceP && indiceR > indiceL) {
                                    chaveamentoCorreto = false;
                                } else {
                                    if (indiceR < indiceP && indiceR > indiceL) {
                                        chaveamentoCorreto = false;
                                    } else {
                                        if (rnpA == rnpP) {
                                            //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                            if (indiceA >= indiceP && indiceA <= indiceL) {
                                                chaveamentoCorreto = false;
                                            } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                            {
                                                //realiza a alocação da RNP que será alterada
                                                alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                                constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                                    if (indice1 != rnpP) {
                                                        configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                                    }
                                                }
                                            }
                                        } else //aplica o chaveamento e gera uma nova configuracao simulada
                                        {
                                            obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                                    indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                                        }
                                        vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                        idConfiguracaoAncestral = idNovaConfiguracao;
                                        idNovaConfiguracao++;
                                    }
                                }
                            }
                        }
                        contador--;
                    }
                    //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                    if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                    {
                        if (idProxConfiguracao[0] == idNovaConfiguracao) {
                            idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                        } else {
                            idNovoAncestral[0] = idNovaConfiguracao - 1;
                        }
                        idProxConfiguracao[0] = idNovaConfiguracao;
                    } else {

                        //3)No local onde houve a repetição das duas chaves
                        novaPosicao = idConfiguracaoParam;
                        contador = contadorConfiguracoes;
                        //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                        idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                        while (contador >= 0 && chaveamentoCorreto) {
                            configuracao = indiceConfiguracoes[contador];
                            contadorChav = 0;
                            if (configuracao != idConfiguracaoIniCF && configuracao != idConfiguracaoIniCA) {
                                copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);

                                if (configuracao == novaPosicao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                                {
                                    noP = noPNovo;
                                    noR = noRNovo;
                                    noA = noANovo;
                                    //insere dados no vetorPi
                                    if (contadorChav == posIniCA) {
                                        vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                        vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                        vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                        vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                        vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                        vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                        vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                                    }

                                } else //aplica a tripla de nos anterior
                                {

                                    noP = vetorPiChav[configuracao].nos[contadorChav].p;
                                    noR = vetorPiChav[configuracao].nos[contadorChav].r;
                                    noA = vetorPiChav[configuracao].nos[contadorChav].a;

                                }
                                //verifica se o par de chaveamento continua valido
                                recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                                if (rnpP != rnpR) {
                                    chaveamentoCorreto = false;
                                } else {
                                    //determinação do intervalo correspondente a subárvore do nó P na RNP
                                    indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                                    if (indiceR < indiceP && indiceR > indiceL) {
                                        chaveamentoCorreto = false;
                                    } else {
                                        if (rnpA == rnpP) {
                                            //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                            if (indiceA >= indiceP && indiceA <= indiceL) {
                                                chaveamentoCorreto = false;
                                            } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                            {
                                                //realiza a alocação da RNP que será alterada
                                                alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                                constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                                    if (indice1 != rnpP) {
                                                        configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                                    }
                                                }
                                            }
                                        } else //aplica o chaveamento e gera uma nova configuracao simulada
                                        {
                                            obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                                    indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                                        }
                                        vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                        idConfiguracaoAncestral = idNovaConfiguracao;
                                        idNovaConfiguracao++;
                                    }
                                }
                            }
                            contador--;
                        }
                        //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                        if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                        {
                            if (idProxConfiguracao[0] == idNovaConfiguracao) {
                                idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                            } else {
                                idNovoAncestral[0] = idNovaConfiguracao - 1;
                            }
                        }
                        idProxConfiguracao[0] = idNovaConfiguracao;
                    }

                } //else da falha na primeira tentativa de chaveamento
            } else {
                //o novo par tem que ser inserido no processo de restabelecimento,
                //pois consiste na troca da chave de restabelecimento
                novaPosicao = idConfiguracaoIniCA;
                contador = contadorConfiguracoes;
                //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                while (contador > 0 && chaveamentoCorreto) {
                    configuracao = indiceConfiguracoes[contador];
                    contadorChav = 0;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    if (configuracao != idConfiguracaoIniCF) { //é necessário pular a configuração que teve a primeira alteração da chave para chave aberta
                        if (configuracao == novaPosicao) {//deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                            noP = noPNovo;
                            noR = noRNovo;
                            noA = noANovo;
                            //insere dados no vetorPi
                            while (contadorChav < vetorPiChav[idConfiguracaoIniCA].numeroManobras) {
                                if (contadorChav == posIniCA) {
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                                }
                                contadorChav++;
                            }
                            //altera o individuo para conter o chaveamento
                            copiaIndividuo(configuracoesChav, configuracoesChav, idConfiguracaoIniCA, idNovaConfiguracao, matrizPIChav);
                            //verifica se o par de chaveamento continua valido
                            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idNovaConfiguracao, matrizPIChav, vetorPiChav);
                            if (rnpP != rnpR) {
                                chaveamentoCorreto = false;
                            } else {
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
                                indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP);
                                //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                                if (indiceR < indiceP || indiceR > indiceL) {
                                    chaveamentoCorreto = false;
                                } else {
                                    if (rnpA == rnpP) {
                                        //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                        if (indiceA >= indiceP && indiceA <= indiceL) {
                                            chaveamentoCorreto = false;
                                        } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                        {
                                            //realiza a alocação da RNP que será alterada
                                            alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                            constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idNovaConfiguracao, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                        }
                                    } else //aplica o chaveamento e gera uma nova configuracao simulada
                                    {

                                        int tamanhoSubarvore;
                                        tamanhoSubarvore = indiceL - indiceP + 1;
                                        //realiza a alocação das que serão alteradas RNPs
                                        alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpA]);
                                        alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                        //constroi a rnp de origem nova
                                        constroiRNPOrigem(configuracoesChav[idConfiguracaoIniCA].rnp[rnpP], &configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idNovaConfiguracao, rnpP);
                                        constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoIniCA, &configuracoesChav[idNovaConfiguracao].rnp[rnpA], idNovaConfiguracao);

                                    }
                                    vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                    vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                                    idConfiguracaoAncestral = idNovaConfiguracao;
                                    idNovaConfiguracao++;
                                }
                            }

                        } else {
                        noP = vetorPiChav[configuracao].nos[contadorChav].p;
                        noR = vetorPiChav[configuracao].nos[contadorChav].r;
                        noA = vetorPiChav[configuracao].nos[contadorChav].a;


                        //  printf("nop %ld, noA %ld noR %ld \n",noP, noA, noR);
                        //verifica se o par de chaveamento continua valido
                        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                        if (rnpP != rnpR) {
                            chaveamentoCorreto = false;
                        } else {
                            //determinação do intervalo correspondente a subárvore do nó P na RNP
                            indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                            if (indiceR < indiceP && indiceR > indiceL) {
                                chaveamentoCorreto = false;
                            } else {
                                if (rnpA == rnpP) {
                                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                    if (indiceA >= indiceP && indiceA <= indiceL) {
                                        chaveamentoCorreto = false;
                                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                    {
                                        //realiza a alocação da RNP que será alterada
                                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                            if (indice1 != rnpP) {
                                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                            }
                                        }
                                    }
                                } else //aplica o chaveamento e gera uma nova configuracao simulada
                                {
                                    obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                                }
                            }
                            vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                            idConfiguracaoAncestral = idNovaConfiguracao;
                            idNovaConfiguracao++;
                        }
                    }
                    }

                    contador--;
                }
                //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                {
                    if (idProxConfiguracao[0] == idNovaConfiguracao) {
                        idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                    } else {
                        idNovoAncestral[0] = idNovaConfiguracao - 1;
                    }
                    idProxConfiguracao[0] = idNovaConfiguracao;
                }
            }
        } else { //o novo par de chaveamento é composto pela abertura e fechamento da mesma chave
            contador = contadorConfiguracoes - 1;
            while (contador > 0 && chaveamentoCorreto) {
                configuracao = indiceConfiguracoes[contador];
                if (configuracao != idConfiguracaoIniCF || configuracao != idConfiguracaoIniCF) {
                    contadorChav = 0;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    noP = vetorPiChav[configuracao].nos[contadorChav].p;
                    noR = vetorPiChav[configuracao].nos[contadorChav].r;
                    noA = vetorPiChav[configuracao].nos[contadorChav].a;
                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                    if (rnpP != rnpR) {
                        chaveamentoCorreto = false;
                    } else {
                        //determinação do intervalo correspondente a subárvore do nó P na RNP
                        indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                        if (rnpA == rnpP) {
                            //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                            if (indiceA >= indiceP && indiceA <= indiceL) {
                                chaveamentoCorreto = false;
                            } else //aplica o chaveamento considerando a alteração de somente uma rnp
                            {
                                //realiza a alocação da RNP que será alterada
                                alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                    if (indice1 != rnpP) {
                                        configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                    }
                                }
                            }
                        } else //aplica o chaveamento e gera uma nova configuracao simulada
                        {
                            obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                    indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                        }
                        vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                        idConfiguracaoAncestral = idNovaConfiguracao;
                        idNovaConfiguracao++;
                    }
                }
                contador--;
            }
            if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
            {
                if (idProxConfiguracao[0] == idNovaConfiguracao)
                    idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                else
                    idNovoAncestral[0] = idNovaConfiguracao - 1;
            }
        }
    }//a repetição das chaves acontece em indivíduos diferentes
    return chaveamentoCorreto;
}

/**
 * Simula a inserção do novo par de manobra na primeira posição executando os pares de manobras posteriores.
 * @param indiceConfiguracoes
 * @param contador
 * @param vetorPiChav
 * @param matrizPIChav
 * @param idProxConfiguracao
 * @param configuracoesChav
 * @param noPNovo
 * @param noANovo
 * @param noRNovo
 * @param posicaoChave
 * @param posicaoConfiguracao
 * @param idConfiguracaoAncestral
 * @param idChaveAbertaR
 * @param idChaveFechadaR
 * @param idNovoAncestral
 * @param idConfiguracaoIni
 * @return
 */
BOOL simulaChaveamentoPosicaoInicial(int *indiceConfiguracoes, int contador, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav,
        int *idProxConfiguracao, CONFIGURACAO *configuracoesChav, long int noPNovo, long int noANovo, long int noRNovo,
        int posicaoChave, int posicaoConfiguracao, int idConfiguracaoAncestral, long int idChaveAbertaR,
        long int idChaveFechadaR, int *idNovoAncestral, int idConfiguracaoIni) {
    BOOL chaveamentoCorreto = true;
    int configuracao;
    int contadorChav;
    long int noP, noR, noA;
    int indiceA, indiceR, indiceP, indiceL, indice1;
    int rnpP, rnpR, rnpA;
    int idNovaConfiguracao = idProxConfiguracao[0];

    while (contador > 0 && chaveamentoCorreto) {
        configuracao = indiceConfiguracoes[contador];
        contadorChav = 0;

        if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
        {
            noP = noPNovo;
            noR = noRNovo;
            noA = noANovo;
        } else {
            //aplica a tripla de nos anterior
            noP = vetorPiChav[configuracao].nos[contadorChav].p;
            noR = vetorPiChav[configuracao].nos[contadorChav].r;
            noA = vetorPiChav[configuracao].nos[contadorChav].a;
        }
        //verifica se o par de chaveamento continua valido
        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
        if (rnpP != rnpR) {
            chaveamentoCorreto = false;
        } else {
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
            //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
            if (indiceR < indiceP && indiceR > indiceL) {
                chaveamentoCorreto = false;
            } else {
                if (rnpA == rnpP) {
                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                    if (indiceA >= indiceP && indiceA <= indiceL) {
                        chaveamentoCorreto = false;
                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                    {
                        //realiza a alocação da RNP que será alterada
                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                            if (indice1 != rnpP) {
                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                            }
                        }
                    }
                } else //aplica o chaveamento e gera uma nova configuracao simulada
                {
                    obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                }
                copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                {
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                    vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                }
                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                idConfiguracaoAncestral = idNovaConfiguracao;
                idNovaConfiguracao++;
            }
        }
        contador--;
    }
    if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
    {
        if (idProxConfiguracao[0] == idNovaConfiguracao)
            idNovoAncestral[0] = vetorPiChav[idConfiguracaoIni].idAncestral;
        else
            idNovoAncestral[0] = idNovaConfiguracao - 1;
    }
    idProxConfiguracao[0] = idNovaConfiguracao;
    return chaveamentoCorreto;
}

/**
 * Simula a insercão do novo par de manobra na segunda posição possível, ou seja no final da cadeia. Para tal aplica os pares de manobras intermediários a partir do ancestral da posição 1.
 * @param indiceConfiguracoes
 * @param contador
 * @param vetorPiChav
 * @param matrizPIChav
 * @param idProxConfiguracao
 * @param configuracoesChav
 * @param noPNovo
 * @param noANovo
 * @param noRNovo
 * @param posicaoChave
 * @param posicaoConfiguracao
 * @param idConfiguracaoAncestral
 * @param idChaveAbertaR
 * @param idChaveFechadaR
 * @param idNovoAncestral
 * @param idConfiguracaoIni
 * @return
 */
BOOL simulaChaveamentoPosicaoFinal(int *indiceConfiguracoes, int contador, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav,
        int *idProxConfiguracao, CONFIGURACAO *configuracoesChav, long int noPNovo, long int noANovo, long int noRNovo,
        int posicaoChave, int posicaoConfiguracao, int idConfiguracaoAncestral, long int idChaveAbertaR,
        long int idChaveFechadaR, int *idNovoAncestral, int idConfiguracaoIni) {
    BOOL chaveamentoCorreto = true;
    int configuracao;
    int contadorChav;
    long int noP, noR, noA;
    int indiceA, indiceR, indiceP, indiceL, indice1;
    int rnpP, rnpR, rnpA;
    int idNovaConfiguracao = idProxConfiguracao[0];

    while (contador >= 0 && chaveamentoCorreto) {
        configuracao = indiceConfiguracoes[contador];
        contadorChav = 0;
        if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
        {
            noP = noPNovo;
            noR = noRNovo;
            noA = noANovo;
        } else //aplica a tripla de nos anterior
        {
            noP = vetorPiChav[configuracao].nos[contadorChav].p;
            noR = vetorPiChav[configuracao].nos[contadorChav].r;
            noA = vetorPiChav[configuracao].nos[contadorChav].a;
        }
        //verifica se o par de chaveamento continua valido
        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
        if (rnpP != rnpR) {
            chaveamentoCorreto = false;
        } else {
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
            if (indiceR < indiceP && indiceR > indiceL) {
                chaveamentoCorreto = false;
            } else {
                if (rnpA == rnpP) {
                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                    if (indiceA >= indiceP && indiceA <= indiceL) {
                        chaveamentoCorreto = false;
                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                    {
                        //realiza a alocação da RNP que será alterada
                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                            if (indice1 != rnpP) {
                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                            }
                        }
                    }
                } else //aplica o chaveamento e gera uma nova configuracao simulada
                {
                    //calcula o tamanho da subárvore podada
                    obtemConfiguracao(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                }
                copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                {
                    //insere dados no vetorPi
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                    vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                }
                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                idConfiguracaoAncestral = idNovaConfiguracao;
                idNovaConfiguracao++;
            }
        }
        contador--;
    }
    if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
    {
        if (idProxConfiguracao[0] == idNovaConfiguracao) {
            idNovoAncestral[0] = vetorPiChav[idConfiguracaoIni].idAncestral;
            // printf("idNovoAncestral %d \n", idNovoAncestral[0]);
        } else
            idNovoAncestral[0] = idNovaConfiguracao - 1;
    }
    idProxConfiguracao[0] = idNovaConfiguracao;
    return chaveamentoCorreto;
}


/**
 * Função para avaliar a configuração durante o chaveamento, desconsiderando a rnp que contém os setores desligados devido a falta.
 * @param configuracoesParam
 * @param idNovaConfiguracaoParam
 * @param dadosTrafoParam
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param dadosAlimentadorParam
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 * @param numeroBarrasParam
 * @param grafoSDRParam
 * @param SBase
 */
void avaliaConfiguracaoChaveamento(CONFIGURACAO *configuracoesParam, long int idNovaConfiguracaoParam,
        DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
        int numeroAlimentadoresParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador,
        DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam,
        MATRIZMAXCORRENTE *maximoCorrenteParam, long int numeroBarrasParam, GRAFO *grafoSDRParam, int SBase) {
    double quedaMaxima, menorTensao, VF, perdasTotais;
    perdasTotais = 0;
    inicializaDadosEletricosChaveamento(matrizB, dadosTrafoParam, dadosAlimentadorParam, grafoSDRParam, configuracoesParam, idNovaConfiguracaoParam, numeroBarrasParam, SBase);
    configuracoesParam[idNovaConfiguracaoParam].objetivo.potenciaTrafo = malloc((numeroTrafosParam + 1) * sizeof (__complex__ double));
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    fluxoCargaTodosAlimentadoresChaveamento(numeroBarrasParam, dadosAlimentadorParam, dadosTrafoParam, configuracoesParam, idNovaConfiguracaoParam, matrizB, ZParam, indiceRegulador, dadosRegulador, maximoCorrenteParam);
    int indiceRNP;
    for (indiceRNP = 0; indiceRNP < configuracoesParam[idNovaConfiguracaoParam].numeroRNP - 1; indiceRNP++) {
        //  printf("alimentador %d carregamento %lf\n",indiceRNP+1,configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoCorrente*100);
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
        //carregamento de rede
        if (configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.maiorCarregamentoRede;
        perdasTotais += configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.perdasResistivas;
        // printf("rnp %d, potencia %lf\n",indiceRNP, __real__ configuracoesParam[idNovaConfiguracaoParam].rnp[indiceRNP].fitnessRNP.potenciaAlimentador);
    }
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.perdasResistivas = perdasTotais;
    carregamentoTrafo(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idNovaConfiguracaoParam, true, -1, -1);

    //libera as estruturas utilizadas no cálculo dos valores elétricos
    free(configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante);
    configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.iJusante = 0;
    free(configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia);
    configuracoesParam[idNovaConfiguracaoParam].dadosEletricos.potencia = 0;
}

/**
 * O presente método define em que ordem as manobras dos ancestrais da configuração final serão executadas.
 * Para cada manobra realizada verifica as características elétricas de forma a obter configurações intermediárias factíveis.
 * O processo inicia construíndo uma lista com todas as manobras necessárias para restabelecer os setores sãos e outra lista para as manobras de alívio.
 * As duas listas acima são criadas com ordem de prioridade definida por uma métrica relacionada ao número de consumidores manobrados.
 * A sequência de manobras é iniciada com as manobras para isolar a falta. A seguir cada uma das manobras de fechar para restabelecimento é analisada e são executadas/inseridas as que não violam as características elétricas relaxadas.
 * As manobras de fechar do restabelecimento que não puderem ser aplicadas são mantidas na lista de chaves a serem manobradas.
 * A seguir inicia-se um processo iterativo de análise de cada uma das sub-sequências possíveis de manobras de alívio até que seja possível restabelecer todos os setores sãos da configuração final.
 * Para cada chave inserida na sequência de manobras é verificada as restrições operacionais de chave seca. Casos essas sejam violadas insere-se a manobra de abertura e fechamento da primeira chave automática a montante.
 * Além disso, para as manobras de alívio é verifica a operação em anel do sistema.
 * @param dadosAlimentadorSDRParam
 * @param dadosTrafoSDRParam
 * @param maximoCorrente
 * @param Z
 * @param rnpSetoresParam
 * @param SBase
 * @param grafoSetoresParam
 * @param numeroSetores
 * @param setoresFalta
 * @param numeroSetoresFalta
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param configuracaoInicial
 * @param vetorPiChav
 * @param idConfiguracaoParam
 * @param grafoSDRParam
 * @param numeroBarras
 * @param listaChavesParam
 * @param numeroManobras
 * @param consumidores
 * @param consumidoresEspeciais
 * @param seed
 */
void otimizaChaveamento(DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, CONFIGURACAO configuracaoInicial, VETORPI *vetorPiChav, long int idConfiguracaoParam,
        GRAFO *grafoSDRParam, long int numeroBarras, LISTACHAVES *listaChavesParam, int numeroManobras, int consumidores, int consumidoresEspeciais, int seed,
        long int indiceAnterior, CONFIGURACAO *configuracoesParam, long int numeroChavesParam) {

    LISTA *listaChaveamentos = Malloc(LISTA, 1); //lista dos chaveamentos realizados
    LISTA *listaAncestrais = Malloc(LISTA, 1); //lista de chaveamentos anteriores
    LISTA *listaFactiveis = Malloc(LISTA, 1); //lista de chaveamentos que geram configuração factível
    BOOL encerra;
    ELEMENTO *chaveamento;
    ELEMENTO *chaveamentoAnt;
    int totalConfiguracoes;
    long int *posChavesFecharRest;
    long int *valorPrioridade;
    long int *paresManobrasAbrir;
    long int *paresManobrasFechar;
    NOSPRA *nosPRA;
    long int *noPRestabelecimento;
    int contador, contadorAux, rnpA, rnpP, rnpR, contadorManobras, contadorFechamento, contFechar;
    CONFIGURACAO *configuracao;
    CONFIGURACAO *configuracoes;
    long int idConfiguracao = 0;
    long int indiceInd, noP, noR, noA;
    int indiceP, indiceR, indiceA, indiceL;
    long int *indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam, &totalConfiguracoes);
    long int indiceConfiguracao = 0;
    VETORPI *vetorPi;
    MATRIZPI *matrizPI;
    RNP rnpOrigem, rnpOrigemAnterior;
    RNP rnpDestino, rnpDestinoAnterior;
    int contadorAncestrais, indiceChav;
    int numeroRnps = configuracaoInicial.numeroRNP + 1;
    int tamanhoTemporario, tamanhoAlocacao;
    long int colunaPI;
    BOOL factibilidadeManobra = false;
    BOOL factibilidadeRestab = false;
    BOOL factibilidadeAnel = false;
    char nomeFinal[100];
    double carga;
    int indiceSetorRaiz;
    long int idChaveAlimentador;
    char setorF[10];
    char nomeSetorFalta[100];
    //cria o nome do arquivo de saída
    sprintf(nomeSetorFalta, "_");
    for (contador = 0; contador < numeroSetoresFalta; contador++) {
        sprintf(setorF, "%ld_", setoresFalta[contador]);
        strcat(nomeSetorFalta, setorF);
    }
    //para colocar o seed
    sprintf(setorF, "%d", seed);
    strcat(nomeSetorFalta, setorF);

    //inicializa as estruturas de dados da rnp
    inicializaVetorPi(vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras + totalConfiguracoes * 10, &vetorPi);
    inicializaMatrizPI(grafoSetoresParam, &matrizPI, (vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras + totalConfiguracoes * 10), numeroSetores);
    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
    configuracao = alocaIndividuo(configuracaoInicial.numeroRNP + 1, idConfiguracao, 1, numeroTrafosParam);
    //copia a configuração inicial na configuração que será manipulada
    copiaIndividuo(&configuracaoInicial, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);

    configuracao[indiceConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciais;
    configuracao[indiceConfiguracao].objetivo.consumidoresSemFornecimento = consumidores;

    //inicia a lista de configurações de chaveamento
    inicializacao(listaChaveamentos);
    inicializacao(listaFactiveis);
    //aloca as estruturas de dados auxiliares para realizar o chaveamento
    posChavesFecharRest = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    valorPrioridade = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    noPRestabelecimento = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    paresManobrasAbrir = Malloc(long int, totalConfiguracoes - 1);
    paresManobrasFechar = Malloc(long int, totalConfiguracoes - 1);
    nosPRA = Malloc(NOSPRA, totalConfiguracoes - 1);

    //aloca o chaveamento pré-falta
    chaveamento = Malloc(ELEMENTO, 1);
    chaveamento->anterior = NULL;
    chaveamento->chavesRestabFechar = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras - 1);
    chaveamento->estadoFinal = Malloc(ESTADOCHAVE, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->idChaves = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->reguladorTensao = Malloc(BOOL, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->numeroChaveamentos = 0;
    chaveamento->numChavesRestab = 0;
    chaveamento->numPares = totalConfiguracoes - 1;
    chaveamento->paresManobras = Malloc(int, totalConfiguracoes - 1);
    insercaoListaVazia(listaChaveamentos, chaveamento);
    //aloca a rnp responsavel por armazenar os setores em falta e sem fornecimento
    int tamanho = 1;
    alocaRNP(tamanho, &configuracao[indiceConfiguracao].rnp[configuracao[indiceConfiguracao].numeroRNP - 1]);
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    //cria a raiz do alimentador ficticio
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].idNo = 0;
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].profundidade = 0;
    avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
            indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);

    contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
    chaveamento->consumidoresCorteCarga = 0;
    chaveamento->configuracao = configuracao;
    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
    configuracao = alocaIndividuo(configuracaoInicial.numeroRNP + 1, idConfiguracao, 1, numeroTrafosParam);
    //copia a configuração inicial na configuração que será manipulada
    copiaIndividuo(&configuracaoInicial, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);
    chaveamentoAnt = chaveamento;
    configuracao[indiceConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciais;
    configuracao[indiceConfiguracao].objetivo.consumidoresSemFornecimento = consumidores;
    //aloca o primeiro chaveamento
    chaveamento = Malloc(ELEMENTO, 1);
    chaveamento->anterior = NULL;
    tamanhoAlocacao = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2;
    chaveamento->chavesRestabFechar = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras - 1);
    chaveamento->estadoFinal = Malloc(ESTADOCHAVE, tamanhoAlocacao);
    chaveamento->idChaves = Malloc(long int, tamanhoAlocacao);
    chaveamento->reguladorTensao = Malloc(BOOL, tamanhoAlocacao);
    chaveamento->numeroChaveamentos = 0;
    chaveamento->numChavesRestab = 0;
    chaveamento->numPares = totalConfiguracoes - 1;
    chaveamento->paresManobras = Malloc(int, totalConfiguracoes - 1);

    contadorManobras = 0;
    printf("chaves restabelecimento: \n");
    configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento = 0;
    
    contadorFechamento = 0;
    contFechar = 0;
    //copia as chaves utilizadas no restabelecimento para isolar a falta
    for (contador = 0; contador < vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras; contador++) {
        //insere na sequencia de chaveamento as chaves para isolar a falta e verifica a presença de regulador de tensão
        noP = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].nos[contador].p;
        colunaPI = retornaColunaPi(matrizPI, vetorPiChav, noP, idConfiguracao);
        indiceP = matrizPI[noP].colunas[colunaPI].posicao;
        rnpP = matrizPI[noP].colunas[colunaPI].idRNP;
        indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveAberta[contador], verificaPresencaRT(indiceL, configuracaoInicial.rnp[rnpP], rnpSetoresParam, indiceRegulador), normalmenteAberta);
        printf("\tabrir %s ", listaChavesParam[vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveAberta[contador]].codOperacional);
        configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento++;

        //monta a lista com as chaves que precisam ser fechadas no restabelecimento
        if (vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contador] > 0) {
            posChavesFecharRest[contFechar] = contador;
            noPRestabelecimento[contFechar] = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].nos[contador].p;
            printf("\tfechar %s", listaChavesParam[vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveFechada[contador]].codOperacional);
            configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento++;
            contFechar++;
        }

        printf("\n");
        contadorManobras++;
    }

    long int colunaPi;
    long int prioridade;
    long int auxiliar;
    /*if (totalConfiguracoes > 1) {
        //calcula o valor de prioridade para cada chave

        for (contador = 0; contador < contFechar; contador++) {
            noP = noPRestabelecimento[contador];
            colunaPi = retornaColunaPi(matrizPI, vetorPi, noPRestabelecimento[contador], indiceConfiguracao);
            indiceP = matrizPI[noPRestabelecimento[contador]].colunas[colunaPi].posicao;
            rnpP = matrizPI[noPRestabelecimento[contador]].colunas[colunaPi].idRNP;
            contadorAux = indiceP + 1;
            prioridade = 100 * grafoSetoresParam[noP].numeroConsumidoresEspeciais + (grafoSetoresParam[noP].numeroConsumidores - grafoSetoresParam[noP].numeroConsumidoresEspeciais);
            while (contadorAux < configuracao[indiceConfiguracao].rnp[rnpP].numeroNos && configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].profundidade > configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade) {
                prioridade += 100 * grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais + (grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidores - grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais);
                contadorAux++;
            }
            valorPrioridade[contador] = prioridade;
        }
        //ordena as chaves a serem fechadas de acordo com o valor de prioridade definido
        
        for (contador = 0; contador < (contFechar - 1); contador++) {
            for (contadorAux = contador + 1; contadorAux < (contFechar); contadorAux++) {
                if (valorPrioridade[contador] < valorPrioridade[contadorAux]) {
                    auxiliar = valorPrioridade[contador];
                    valorPrioridade[contador] = valorPrioridade[contadorAux];
                    valorPrioridade[contadorAux] = auxiliar;
                    auxiliar = posChavesFecharRest[contador];
                    posChavesFecharRest[contador] = posChavesFecharRest[contadorAux];
                    posChavesFecharRest[contadorAux] = auxiliar;
                }
            }

        }
    }*/

    printf("manobras alivio: \n");
    configuracoesParam[indiceAnterior].objetivo.manobrasAlivio = 0;
    //copia o indice das chaves utilizadas para obter a configuração final
    for (contador = 1; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        paresManobrasAbrir[contador - 1] = vetorPiChav[indiceInd].idChaveAberta[0];
        paresManobrasFechar[contador - 1] = vetorPiChav[indiceInd].idChaveFechada[0];
        nosPRA[contador - 1].p = vetorPiChav[indiceInd].nos[0].p;
        nosPRA[contador - 1].r = vetorPiChav[indiceInd].nos[0].r;
        nosPRA[contador - 1].a = vetorPiChav[indiceInd].nos[0].a;
        chaveamento->paresManobras[contador - 1] = contador - 1;
        printf("\t abrir %s ", listaChavesParam[vetorPiChav[indiceInd].idChaveAberta[0]].codOperacional);
        printf("\t fechar %s\n", listaChavesParam[vetorPiChav[indiceInd].idChaveFechada[0]].codOperacional);
        configuracoesParam[indiceAnterior].objetivo.manobrasAlivio = configuracoesParam[indiceAnterior].objetivo.manobrasAlivio + 2;
    }
    //calcula a prioridade dos pares de manobras de alivio
    for (contador = 0; contador < totalConfiguracoes - 1; contador++) {
        noP = nosPRA[contador].p;
        colunaPi = retornaColunaPi(matrizPI, vetorPi, noP, indiceConfiguracao);
        indiceP = matrizPI[noP].colunas[colunaPi].posicao;
        rnpP = matrizPI[noP].colunas[colunaPi].idRNP;
        contadorAux = indiceP + 1;
        prioridade = 100 * grafoSetoresParam[noP].numeroConsumidoresEspeciais + (grafoSetoresParam[noP].numeroConsumidores - grafoSetoresParam[noP].numeroConsumidoresEspeciais);
        while (contadorAux < configuracao[indiceConfiguracao].rnp[rnpP].numeroNos && configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].profundidade > configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade) {
            prioridade += 100 * grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais + (grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidores - grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais);
            contadorAux++;
        }
        valorPrioridade[contador] = prioridade;
    }

    //ordena os pares de manobras de alivio
    for (contador = 0; contador < (totalConfiguracoes - 2); contador++) {
        for (contadorAux = contador + 1; contadorAux < (totalConfiguracoes - 1); contadorAux++) {
            if (valorPrioridade[contador] < valorPrioridade[contadorAux]) {
                auxiliar = valorPrioridade[contador];
                valorPrioridade[contador] = valorPrioridade[contadorAux];
                valorPrioridade[contadorAux] = auxiliar;
                auxiliar = chaveamento->paresManobras[contador];
                chaveamento->paresManobras[contador] = chaveamento->paresManobras[contadorAux];
                chaveamento->paresManobras[contadorAux] = auxiliar;
            }
        }

    }


    //simula o fechamento das chaves que são possíveis e transfere os setores em falta e demais setores a jusante para a rnp de desligados
    //aloca a rnp responsavel por armazenar os setores em falta e sem fornecimento
    alocaRNP(tamanho, &configuracao[indiceConfiguracao].rnp[configuracao[indiceConfiguracao].numeroRNP - 1]);
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    //cria a raiz do alimentador ficticio
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].idNo = 0;
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].profundidade = 0;
    avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
            indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);

    //imprimeIndividuo(configuracao[indiceConfiguracao]);
    //Inicia a tentativa de fechamento das chaves para restabelecer o sistema
    contador = 0;
    while (contador < contFechar) {
        contadorFechamento = posChavesFecharRest[contador];
        if (vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento] > 0) {
            noP = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].p;
            noR = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].r;
            noA = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].a;
            //verifica se o par de chaveamento continua valido
            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
            if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA)) {
                //aplica a operação de fechamento da chave para restabelecer o setor a jusante a falta
                tamanhoTemporario = indiceL - indiceP + 1;
                if (listaChavesParam[vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento]].subTipoChave == chaveSeca) {
                    //verifica a carga do trecho
                    //recupera o setor raiz do setor P
                    indiceSetorRaiz = indiceP - 1;
                    while ((indiceSetorRaiz >= 0) && (configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                        indiceSetorRaiz--;
                    }
                    carga = cargaTrecho(configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                }
                if (rnpA == rnpP) {
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                } else {
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                    //obtém as novas rnps origem e destino
                    constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                    constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                    configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                }
                avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                        indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                if (!verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR)) {
                    //a configuração obtida não é factível em relação as restrições relaxadas
                    //desfaz a operação aplicada
                    if (rnpA == rnpP) {
                        free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                        configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                    } else {
                        free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                        configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                        free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                        configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                        configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                    }
                    //transfere para a rnp de setores em fornecimento, aplicando o operador correspondente
                    noA = 0;
                    indiceA = 0;
                    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                    //obtém as novas rnps origem e destino
                    constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                    constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                    configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;

                    //faz o no p ter o mesmo valor do nó r
                    vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].p = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].r;
                    //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                    chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = contadorFechamento;
                    chaveamento->numChavesRestab++;
                } else {
                    if ((listaChavesParam[vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento]].subTipoChave == chaveSeca) && (carga > 720)) {
                        tamanhoAlocacao += 2;
                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                        //abertura do alimentador
                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                        //fechamento da chave
                        //adiciona na lista de chaves manobradas
                        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                        //fechamento do alimentador
                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                    } else {
                        //adiciona na lista de chaves manobradas
                        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                    }
                }

            } else {
                //salva no vetor de elementos do chaveamento o idP da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido a não garantir a radialidade;
                chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = contadorFechamento;
                chaveamento->numChavesRestab++;
            }
        }
        contador++;
    }
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    for (contador = 0; contador < numeroSetoresFalta; contador++) {
        colunaPI = retornaColunaPi(matrizPI, vetorPi, setoresFalta[contador], idConfiguracao);
        indiceP = matrizPI[setoresFalta[contador]].colunas[colunaPI].posicao;
        rnpP = matrizPI[setoresFalta[contador]].colunas[colunaPI].idRNP;
        if (rnpP != rnpA) {
            indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
            indiceR = indiceP;
            tamanhoTemporario = indiceL - indiceP + 1;
            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
            //obtém as novas rnps origem e destino
            constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
            constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);

            //###############   ARMAZENA O PONTEIRO PARA FLORESTA E LIBERA A MEMORIA DAS RNPS SUBSTITUÍDAS   ###############
            // free(configuracao[0].rnp[rnpA].nos);
            //free(configuracao[0].rnp[rnpP].nos);
            rnpOrigem.fitnessRNP = configuracao[indiceConfiguracao].rnp[rnpP].fitnessRNP;
            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
            //até esse ponto o setor em falta foi isolado do alimentador. 
            //Já foi inserido na sequencia de chaveamento a abertura de todas as chaves para isolar a falta
        }
    }

    //   imprimeIndividuo(configuracao[indiceConfiguracao]);
    contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
    chaveamento->consumidoresCorteCarga = consumidoresDesligados(configuracao[indiceConfiguracao], configuracao[indiceConfiguracao].numeroRNP - 1,
            grafoSetoresParam, setoresFalta, numeroSetoresFalta);
    chaveamento->configuracao = configuracao;
    insercaoElemFim(listaChaveamentos, chaveamento, chaveamentoAnt);
    //salva o ponteiro do chaveamento anterior
    listaAncestrais->inicio = chaveamento;
    listaAncestrais->fim = chaveamento;
    listaAncestrais->tamanho = 1;
    chaveamentoAnt = listaAncestrais->inicio;
    contadorAncestrais = listaAncestrais->tamanho;
    idConfiguracao++;
    encerra = false;
    if ((chaveamentoAnt->numPares > 0) || (chaveamentoAnt->numChavesRestab > 0)) {
        while (!encerra && (chaveamentoAnt->numPares > 0)) { //verifica se já encontrou um chaveamento factivel e se ainda existem pares para ser manobrados    
            //cria o primeiro novo chaveamento
            chaveamento = Malloc(ELEMENTO, 1);
            chaveamento->anterior = chaveamentoAnt;
            tamanhoAlocacao = chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab;
            chaveamento->chavesRestabFechar = Malloc(long int, chaveamentoAnt->numChavesRestab);
            chaveamento->estadoFinal = Malloc(ESTADOCHAVE, (tamanhoAlocacao)); //aplica um par e tenta fechar todas as chaves do restabelecimento
            chaveamento->idChaves = Malloc(long int, (tamanhoAlocacao));
            chaveamento->reguladorTensao = Malloc(BOOL, (tamanhoAlocacao));
            chaveamento->numChavesRestab = 0;
            chaveamento->numPares = 0;
            chaveamento->paresManobras = Malloc(int, chaveamentoAnt->numPares);
            listaAncestrais->inicio = chaveamento;
            listaAncestrais->tamanho = 0;
            while (contadorAncestrais > 0 && !encerra) {
                //para cada possível ancestral percorre a lista de pares de manobras disponíveis
                contadorManobras = 0;
                //printf("numero pares manobras restante %d \n",chaveamentoAnt->numPares);
                while (contadorManobras < chaveamentoAnt->numPares && !encerra) {
                    chaveamento->numeroChaveamentos = chaveamentoAnt->numeroChaveamentos;
                    //                    printf("chaveamentoAnt %d \n", chaveamentoAnt->numeroChaveamentos);
                    for (contadorAux = 0; contadorAux < chaveamentoAnt->numeroChaveamentos; contadorAux++) {
                        chaveamento->estadoFinal[contadorAux] = chaveamentoAnt->estadoFinal[contadorAux];
                        chaveamento->idChaves[contadorAux] = chaveamentoAnt->idChaves[contadorAux];
                        chaveamento->reguladorTensao[contadorAux] = chaveamentoAnt->reguladorTensao[contadorAux];
                    }
                    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
                    configuracao = alocaIndividuo(numeroRnps, indiceConfiguracao, 1, numeroTrafosParam);
                    configuracao[indiceConfiguracao].idConfiguracao = idConfiguracao;
                    //copia a configuração inicial na configuração que será manipulada
                    copiaIndividuo(chaveamentoAnt->configuracao, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);
                    //aplica um par de manobra
                    indiceChav = chaveamentoAnt->paresManobras[contadorManobras];
                    noP = nosPRA[indiceChav].p;
                    noR = nosPRA[indiceChav].r;
                    noA = nosPRA[indiceChav].a;
                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
                    //determinação do intervalo correspondente a subárvore do nó P na RNP
                    //    printf("rnpP %d indiceP %d numeroNosRnpP %d \n", rnpP, indiceP, configuracao[indiceConfiguracao].rnp[rnpP].numeroNos);
                    indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
                    factibilidadeAnel = false;
                    //precisa testar também se a rnpA não é a rnp de setores sem fornecimento
                    if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA) && (rnpA != (numeroRnps - 1))) {
                        //verifica a factilidade do anel antes de aplicar a manobra
                        factibilidadeAnel = verificaFactibilidadeAnel(configuracao, noR, noA, rnpP, rnpA,
                                grafoSDRParam, numeroBarras, dadosAlimentadorSDRParam,
                                dadosTrafoSDRParam, indiceConfiguracao, rnpSetoresParam,
                                SBase, numeroSetores, Z, indiceRegulador,
                                dadosRegulador, maximoCorrente, numeroTrafosParam, numeroAlimentadores);
                        //aplica a manobra
                        tamanhoTemporario = indiceL - indiceP + 1;
                        if ((listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) || (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca)) {
                            //verifica a carga do trecho
                            //recupera o setor raiz do setor P
                            indiceSetorRaiz = indiceP - 1;
                            while ((indiceSetorRaiz >= 0) && (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                                indiceSetorRaiz--;
                            }
                            carga = cargaTrecho(chaveamentoAnt->configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                        }
                        if (rnpA == rnpP) {
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                            rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                            constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                            avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                    indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                            factibilidadeManobra = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);

                        } else {
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                            rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                            rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                            //obtém as novas rnps origem e destino
                            constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                            constroiRNPOrigem(configuracao[indiceConfiguracao].rnp[rnpP], &rnpOrigem, indiceP, indiceL, matrizPI, idConfiguracao, rnpP);
                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                            avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                    indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                            factibilidadeManobra = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);

                        }
                        if (!factibilidadeManobra) {
                            if (rnpA == rnpP) {
                                //a configuração obtida não é factível em relação as restrições relaxadas
                                //desfaz a operação aplicada
                                free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                //salva no vetor de pares de manobras. Nesse caso devido as restrições elétricas;
                                chaveamento->paresManobras[chaveamento->numPares] = indiceChav;
                                chaveamento->numPares++;
                            } else {
                                //a configuração obtida não é factível em relação as restrições relaxadas
                                //desfaz a operação aplicada
                                free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                                configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                                configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                                //salva no vetor de pares de manobras. Nesse caso devido as restrições elétricas;
                                chaveamento->paresManobras[chaveamento->numPares] = indiceChav;
                                chaveamento->numPares++;
                            }
                        } else {
                            if (factibilidadeAnel) {
                                //adiciona na lista de chaves manobradas

                                if (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho para fechamento de 720kVA
                                    if (carga > 720) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //fechamento da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);

                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                }
                                if (listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho de 120kVA para abertura
                                    if (carga > 120) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceP, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //abertura da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                }
                            } else {
                                //nao foi possivel operar em anel insere as manobras no modelo abre-fecha
                                //adiciona na lista de chaves manobradas
                                if (listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho de 120kVA para abertura
                                    if (carga > 120) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceP, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //abertura da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                }

                                if (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho para fechamento de 720kVA
                                    if (carga > 720) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //fechamento da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);

                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                }

                            }
                            //copia as manobras anteriores que não foram aplicadas ainda
                            for (contadorAux = 0; contadorAux < contadorManobras; contadorAux++) {
                                chaveamento->paresManobras[chaveamento->numPares] = chaveamentoAnt->paresManobras[contadorAux];
                                chaveamento->numPares++;
                            }
                            //copia os pares de manobras restante
                            for (contadorAux = contadorManobras + 1; contadorAux < chaveamentoAnt->numPares; contadorAux++) {

                                chaveamento->paresManobras[chaveamento->numPares] = chaveamentoAnt->paresManobras[contadorAux];
                                chaveamento->numPares++;
                            }
                            //tenta restabelecer os setores sem fornecimento
                            contadorFechamento = 0;
                            while (contadorFechamento < chaveamentoAnt->numChavesRestab) {
                                noP = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].p;
                                noR = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].r;
                                noA = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].a;
                                //verifica se o par de chaveamento continua valido
                                recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
                                indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);

                                if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA)) {
                                    //aplica a operação de fechamento da chave para restabelecer o setor a jusante a falta
                                    tamanhoTemporario = indiceL - indiceP + 1;
                                    if (listaChavesParam[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].subTipoChave == chaveSeca) {
                                        //verifica a carga do trecho
                                        //recupera o setor raiz do setor P
                                        indiceSetorRaiz = indiceP - 1;
                                        while ((indiceSetorRaiz >= 0) && (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                                            indiceSetorRaiz--;
                                        }
                                        carga = cargaTrecho(chaveamentoAnt->configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                                    }
                                    if (rnpA == rnpP) {
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                                        rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                                        constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                                        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                                        factibilidadeRestab = verificaFactibilidade(configuracao[0], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);
                                        if (!factibilidadeRestab) {
                                            //a configuração obtida não é factível em relação as restrições relaxadas
                                            //desfaz a operação aplicada
                                            free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                            //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                                            chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                            chaveamento->numChavesRestab++;
                                        } else {
                                            //adiciona na lista de chaves manobradas
                                            insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        }
                                    } else {
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                                        rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                                        rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                                        //obtém as novas rnps origem e destino
                                        constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                                        constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                                        configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                                        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                                        factibilidadeRestab = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);
                                        if (!factibilidadeRestab) {
                                            //a configuração obtida não é factível em relação as restrições relaxadas
                                            //desfaz a operação aplicada
                                            free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                            free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                                            //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                                            chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                            chaveamento->numChavesRestab++;
                                        } else {

                                            if ((listaChavesParam[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].subTipoChave == chaveSeca) && (carga > 720)) {
                                                //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                                //será incluída a manobra de abertura e fechamento do alimentador
                                                tamanhoAlocacao += 2;
                                                chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                                chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                                chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                                idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                                //abertura do alimentador
                                                insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                                //fechamento da chave
                                                insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                                //fechamento do alimentador
                                                insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                            } else {

                                                //adiciona na lista de chaves manobradas
                                                insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                            }
                                        }
                                    }
                                } else {
                                    //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido a não garantir a radialidade;
                                    chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                    chaveamento->numChavesRestab++;
                                }

                                contadorFechamento++;
                            }
                        }
                        if (factibilidadeManobra || factibilidadeRestab) {
                            //insere a configuracao no chaveamento
                            contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
                            chaveamento->consumidoresCorteCarga = consumidoresDesligados(configuracao[indiceConfiguracao], configuracao[indiceConfiguracao].numeroRNP - 1,
                                    grafoSetoresParam, setoresFalta, numeroSetoresFalta);
                            chaveamento->configuracao = configuracao;
                            //insere o chaveamento na lista de chaveamentos
                            insercaoElemFim(listaChaveamentos, chaveamento, chaveamentoAnt);
                            //atualiza o ponteiro de fim e o contador da lista de ancestrais
                            listaAncestrais->fim = chaveamento;
                            listaAncestrais->tamanho++;
                            if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao) && chaveamento->numChavesRestab == 0) {
                                if (listaFactiveis->tamanho == 0) {
                                    insercaoListaVazia(listaFactiveis, chaveamento);
                                } else {
                                    insercaoElemFim(listaFactiveis, chaveamento, chaveamentoAnt);
                                }
                                encerra = true;
                            } else {
                                //        printf("idconfi %ld numeroChaveamentos %d\n", configuracao[indiceConfiguracao].idConfiguracao, chaveamento->numeroChaveamentos);
                                //cria um novo chaveamento
                                chaveamento = Malloc(ELEMENTO, 1);
                                chaveamento->anterior = NULL;
                                chaveamento->chavesRestabFechar = Malloc(long int, chaveamentoAnt->numChavesRestab);
                                chaveamento->estadoFinal = Malloc(ESTADOCHAVE, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                chaveamento->idChaves = Malloc(long int, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab));
                                chaveamento->reguladorTensao = Malloc(BOOL, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab));
                                chaveamento->numChavesRestab = 0;
                                chaveamento->numPares = 0;
                                chaveamento->paresManobras = Malloc(int, chaveamentoAnt->numPares);
                                idConfiguracao++;
                            }
                        }
                    }
                    contadorManobras++;
                }

                //recupera o próximo ancestral
                chaveamentoAnt = chaveamentoAnt->proximo;
                contadorAncestrais--;
            }
            chaveamentoAnt = listaAncestrais->inicio;
            contadorAncestrais = listaAncestrais->tamanho;
        }
    } else {
        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
        if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao) && chaveamento->numChavesRestab == 0) {
            if (listaFactiveis->tamanho == 0) {
                insercaoListaVazia(listaFactiveis, chaveamento);
            } else {
                insercaoElemFim(listaFactiveis, chaveamento, chaveamentoAnt);
            }
        }
    }
    indiceInd = 0;
    if (listaFactiveis->tamanho > 0) {
        chaveamento = listaFactiveis->inicio;
        configuracao = chaveamento->configuracao;

        while (chaveamento != NULL) {
            if (chaveamento->numeroChaveamentos != numeroManobras)
                salvaMelhorConfiguracao(chaveamento->configuracao[indiceConfiguracao], numeroSetoresFalta, 0, chaveamento->configuracao[indiceConfiguracao].idConfiguracao, setoresFalta);
            printf("chaveamento factivel %ld: manobras %d\n", chaveamento->configuracao[indiceConfiguracao].idConfiguracao, chaveamento->numeroChaveamentos);
            configuracoesParam[indiceAnterior].objetivo.manobrasAposChaveamento = chaveamento->numeroChaveamentos;
            for (contadorAux = 0; contadorAux < chaveamento->numeroChaveamentos; contadorAux++) {
                printf("\t %s %d", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, chaveamento->estadoFinal[contadorAux]);
                //   printf("\t %ld", chaveamento->idChaves[contadorAux]);
            }
            printf("\n\n");
            sprintf(nomeFinal, "SAIDA_sequenciaChaveamentoFactivel%s.dad", nomeSetorFalta);
            salvaChaveamento(chaveamento, nomeFinal, listaChavesParam);
            //Por Leandro: Comente aqui para não fazer a impressão do arquivo de chaves a cada execução
            //sprintf(nomeFinal, "chavesCompleto%s.dad", nomeSetorFalta);
            //gravaChavesCompleto(numeroChavesParam, nomeFinal, listaChavesParam);
            chaveamento = chaveamento->proximo;
        }
        //salva os individuos
        chaveamento = listaFactiveis->inicio;
        sprintf(nomeFinal, "Factivel_RNP_setorFalta%s.dad", nomeSetorFalta);
        gravaIndividuo(nomeFinal, chaveamento->configuracao[0]);
        sprintf(nomeFinal, "SAIDA_individuoFactivel_CaracteristicasEletricas_setorFalta%s.dad", nomeSetorFalta);
        gravaDadosEletricos(nomeFinal, chaveamento->configuracao[0]);
        FILE *arquivo;
        arquivo = fopen(nomeFinal, "a");
        fprintf(arquivo, "CONSUMIDORES CORTE CARGA %ld\n", chaveamento->consumidoresCorteCarga);
        //configuracoesParam[indiceAnterior].objetivo.consumidoresDesligadosEmCorteDeCarga = chaveamento->consumidoresCorteCarga;
        configuracoesParam[indiceAnterior].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;
        fclose(arquivo);
        //recupera os individuos intermediarios
        /*       
                      configuracoes = Malloc(CONFIGURACAO, 2 * totalConfiguracoes);

                      while (chaveamento != NULL) {
                          //      printf("indiceInd %ld totalConfiguracoes %d \n", indiceInd, totalConfiguracoes);
                          configuracoes[indiceInd] = chaveamento->configuracao[0];
                          indiceInd++;
                          chaveamento = chaveamento->anterior;
                      }
                      contador = 0;
                      while (indiceInd > 0) {
                          sprintf(nomeFinal, "Factivel_%d_RNP_setorFalta%s.dad", contador, nomeSetorFalta);
                          gravaIndividuo(nomeFinal, configuracoes[indiceInd - 1]);
                          sprintf(nomeFinal, "individuoFactivel_%d_CaracteristicasEletricas_setorFalta%s.dad", contador, nomeSetorFalta);
                          gravaDadosEletricos(nomeFinal, configuracoes[indiceInd - 1]);
                          indiceInd--;
                          contador++;
                      }*/

    } else {
        /* chaveamento = listaChaveamentos->inicio;
         chaveamento = chaveamento->proximo;
         contador = 1;
         while (chaveamento != NULL) {
             if (chaveamento->numeroChaveamentos != numeroManobras)
                 salvaMelhorConfiguracao(chaveamento->configuracao[indiceConfiguracao], numeroSetoresFalta, 0, chaveamento->configuracao[indiceConfiguracao].idConfiguracao, setoresFalta);
             printf("chaveamento Infactivel %ld: \n", chaveamento->configuracao[indiceConfiguracao].idConfiguracao);
             for (contadorAux = 0; contadorAux < chaveamento->numeroChaveamentos; contadorAux++) {
                 printf("\t %s %d", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, chaveamento->estadoFinal[contadorAux]);
                 //   printf("\t %ld", chaveamento->idChaves[contadorAux]);
             }
             printf("\n\n");

             sprintf(nomeFinal, "sequenciaChaveamento%s.dad", nomeSetorFalta);
             salvaChaveamento(chaveamento, nomeFinal, listaChavesParam);

             sprintf(nomeFinal, "Infactivel_%d_RNP_setorFalta%s.dad", contador, nomeSetorFalta);
             gravaIndividuo(nomeFinal, chaveamento->configuracao[0]);
             sprintf(nomeFinal, "individuoInfactivel_%d_CaracteristicasEletricas_setorFalta%s.dad", contador, nomeSetorFalta);
             gravaDadosEletricos(nomeFinal, chaveamento->configuracao[0]);
             contador++;
             chaveamento = chaveamento->proximo;
         }*/
        otimizaChaveamentoCorteCarga(dadosAlimentadorSDRParam, dadosTrafoSDRParam,
                maximoCorrente, Z, rnpSetoresParam, SBase, grafoSetoresParam,
                numeroSetores, setoresFalta, numeroSetoresFalta, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, configuracaoInicial, vetorPiChav, idConfiguracaoParam,
                grafoSDRParam, numeroBarras, listaChavesParam, numeroManobras,
                consumidores, consumidoresEspeciais, seed, indiceAnterior, configuracoesParam, numeroChavesParam);
    }
    desalocaMatrizPI(matrizPI, numeroSetores);
    free(matrizPI);
    free(posChavesFecharRest);
    free(valorPrioridade);
    free(paresManobrasAbrir);
    free(paresManobrasFechar);
}


/**Por Leandro:
 * Consiste na função "otimizaChaveamento()" modificada para
 * (a) considerar as operações que envolvem RNPs Fictícias
 * (b) considerar o indivíduo no qual nenhum setor desligado está restaurado (
 *
 * Assim, como a função "otimizaChaveamento()",
 * o presente método define em que ordem as manobras dos ancestrais da configuração final serão executadas.
 * Para cada manobra realizada verifica as características elétricas de forma a obter configurações intermediárias factíveis.
 * O processo inicia construíndo uma lista com todas as manobras necessárias para restabelecer os setores sãos e outra lista para as manobras de alívio.
 * As duas listas acima são criadas com ordem de prioridade definida por uma métrica relacionada ao número de consumidores manobrados.
 * A sequência de manobras é iniciada com as manobras para isolar a falta. A seguir cada uma das manobras de fechar para restabelecimento é analisada e são executadas/inseridas as que não violam as características elétricas relaxadas.
 * As manobras de fechar do restabelecimento que não puderem ser aplicadas são mantidas na lista de chaves a serem manobradas.
 * A seguir inicia-se um processo iterativo de análise de cada uma das sub-sequências possíveis de manobras de alívio até que seja possível restabelecer todos os setores sãos da configuração final.
 * Para cada chave inserida na sequência de manobras é verificada as restrições operacionais de chave seca. Casos essas sejam violadas insere-se a manobra de abertura e fechamento da primeira chave automática a montante.
 * Além disso, para as manobras de alívio é verifica a operação em anel do sistema.
 *
 * @param dadosAlimentadorSDRParam
 * @param dadosTrafoSDRParam
 * @param maximoCorrente
 * @param Z
 * @param rnpSetoresParam
 * @param SBase
 * @param grafoSetoresParam
 * @param numeroSetores
 * @param setoresFalta
 * @param numeroSetoresFalta
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param configuracaoInicial
 * @param vetorPiChav
 * @param idConfiguracaoParam
 * @param grafoSDRParam
 * @param numeroBarras
 * @param listaChavesParam
 * @param numeroManobras
 * @param consumidores
 * @param consumidoresEspeciais
 * @param seed
 */
void otimizaChaveamentoModificada(DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, CONFIGURACAO configuracaoInicial, VETORPI *vetorPiChav, long int idConfiguracaoParam,
        GRAFO *grafoSDRParam, long int numeroBarras, LISTACHAVES *listaChavesParam, int numeroManobras, int consumidores, int consumidoresEspeciais, int seed,
        long int indiceAnterior, CONFIGURACAO *configuracoesParam, long int numeroChavesParam) {

    LISTA *listaChaveamentos = Malloc(LISTA, 1); //lista dos chaveamentos realizados
    LISTA *listaAncestrais = Malloc(LISTA, 1); //lista de chaveamentos anteriores
    LISTA *listaFactiveis = Malloc(LISTA, 1); //lista de chaveamentos que geram configuração factível
    BOOL encerra;
    ELEMENTO *chaveamento;
    ELEMENTO *chaveamentoAnt;
    int totalConfiguracoes;
    long int *posChavesFecharRest;
    long int *valorPrioridade;
    long int *paresManobrasAbrir;
    long int *paresManobrasFechar;
    NOSPRA *nosPRA;
    long int *noPRestabelecimento;
    int contador, contadorAux, rnpA, rnpP, rnpR, contadorManobras, contadorFechamento, contFechar;
    CONFIGURACAO *configuracao;
    CONFIGURACAO *configuracoes;
    long int idConfiguracao = 0;
    long int indiceInd, noP, noR, noA;
    int indiceP, indiceR, indiceA, indiceL;
    long int *indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam, &totalConfiguracoes);
    long int indiceConfiguracao = 0;
    VETORPI *vetorPi;
    MATRIZPI *matrizPI;
    RNP rnpOrigem, rnpOrigemAnterior;
    RNP rnpDestino, rnpDestinoAnterior;
    int contadorAncestrais, indiceChav;
    int numeroRnps = configuracaoInicial.numeroRNP + 1;
    int tamanhoTemporario, tamanhoAlocacao;
    long int colunaPI;
    BOOL factibilidadeManobra = false;
    BOOL factibilidadeRestab = false;
    BOOL factibilidadeAnel = false;
    char nomeFinal[100];
    double carga;
    int indiceSetorRaiz;
    long int idChaveAlimentador;
    char setorF[10];
    char nomeSetorFalta[100];
    //cria o nome do arquivo de saída
    sprintf(nomeSetorFalta, "_");
    for (contador = 0; contador < numeroSetoresFalta; contador++) {
        sprintf(setorF, "%ld_", setoresFalta[contador]);
        strcat(nomeSetorFalta, setorF);
    }
    //para colocar o seed
    sprintf(setorF, "%d", seed);
    strcat(nomeSetorFalta, setorF);

    //inicializa as estruturas de dados da rnp
    inicializaVetorPi(vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras + totalConfiguracoes * 10, &vetorPi);
    inicializaMatrizPI(grafoSetoresParam, &matrizPI, (vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras + totalConfiguracoes * 10), numeroSetores + configuracaoInicial.numeroRNPFicticia);
    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
    configuracao = alocaIndividuoModificada(configuracaoInicial.numeroRNP + 1, configuracaoInicial.numeroRNPFicticia, idConfiguracao, 1, numeroTrafosParam, numeroSetores); //Por Leandro

    //copia a configuração inicial na configuração que será manipulada
    copiaIndividuoModificada(&configuracaoInicial, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);

    configuracao[indiceConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciais;
    configuracao[indiceConfiguracao].objetivo.consumidoresSemFornecimento = consumidores;

    //inicia a lista de configurações de chaveamento
    inicializacao(listaChaveamentos);
    inicializacao(listaFactiveis);
    //aloca as estruturas de dados auxiliares para realizar o chaveamento
    posChavesFecharRest = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    valorPrioridade = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    noPRestabelecimento = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    paresManobrasAbrir = Malloc(long int, totalConfiguracoes - 1);
    paresManobrasFechar = Malloc(long int, totalConfiguracoes - 1);
    nosPRA = Malloc(NOSPRA, totalConfiguracoes - 1);

    //aloca o chaveamento pré-falta
    chaveamento = Malloc(ELEMENTO, 1);
    chaveamento->anterior = NULL;
    chaveamento->chavesRestabFechar = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras - 1);
    chaveamento->estadoFinal = Malloc(ESTADOCHAVE, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->idChaves = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->reguladorTensao = Malloc(BOOL, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->numeroChaveamentos = 0;
    chaveamento->numChavesRestab = 0;
    chaveamento->numPares = totalConfiguracoes - 1;
    chaveamento->paresManobras = Malloc(int, totalConfiguracoes - 1);
    insercaoListaVazia(listaChaveamentos, chaveamento);
    //aloca a rnp responsavel por armazenar os setores em falta e sem fornecimento
    int tamanho = 1;
    alocaRNP(tamanho, &configuracao[indiceConfiguracao].rnp[configuracao[indiceConfiguracao].numeroRNP - 1]);
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    //cria a raiz do alimentador ficticio
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].idNo = 0;
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].profundidade = 0;
/**/    avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
            indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);

    contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
    chaveamento->consumidoresCorteCarga = 0;
    chaveamento->configuracao = configuracao;
    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
/**/    configuracao = alocaIndividuo(configuracaoInicial.numeroRNP + 1, idConfiguracao, 1, numeroTrafosParam);
    //copia a configuração inicial na configuração que será manipulada
/**/    copiaIndividuo(&configuracaoInicial, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);
    chaveamentoAnt = chaveamento;
    configuracao[indiceConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciais;
    configuracao[indiceConfiguracao].objetivo.consumidoresSemFornecimento = consumidores;
    //aloca o primeiro chaveamento
    chaveamento = Malloc(ELEMENTO, 1);
    chaveamento->anterior = NULL;
    tamanhoAlocacao = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2;
    chaveamento->chavesRestabFechar = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras - 1);
    chaveamento->estadoFinal = Malloc(ESTADOCHAVE, tamanhoAlocacao);
    chaveamento->idChaves = Malloc(long int, tamanhoAlocacao);
    chaveamento->reguladorTensao = Malloc(BOOL, tamanhoAlocacao);
    chaveamento->numeroChaveamentos = 0;
    chaveamento->numChavesRestab = 0;
    chaveamento->numPares = totalConfiguracoes - 1;
    chaveamento->paresManobras = Malloc(int, totalConfiguracoes - 1);

    contadorManobras = 0;
    printf("chaves restabelecimento: \n");
    configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento = 0;

    contadorFechamento = 0;
    contFechar = 0;
    //copia as chaves utilizadas no restabelecimento para isolar a falta
    for (contador = 0; contador < vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras; contador++) {
        //insere na sequencia de chaveamento as chaves para isolar a falta e verifica a presença de regulador de tensão
        noP = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].nos[contador].p;
        colunaPI = retornaColunaPi(matrizPI, vetorPiChav, noP, idConfiguracao);
        indiceP = matrizPI[noP].colunas[colunaPI].posicao;
        rnpP = matrizPI[noP].colunas[colunaPI].idRNP;
        indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveAberta[contador], verificaPresencaRT(indiceL, configuracaoInicial.rnp[rnpP], rnpSetoresParam, indiceRegulador), normalmenteAberta);
        printf("\tabrir %s ", listaChavesParam[vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveAberta[contador]].codOperacional);
        configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento++;

        //monta a lista com as chaves que precisam ser fechadas no restabelecimento
        if (vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contador] > 0) {
            posChavesFecharRest[contFechar] = contador;
            noPRestabelecimento[contFechar] = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].nos[contador].p;
            printf("\tfechar %s", listaChavesParam[vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveFechada[contador]].codOperacional);
            configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento++;
            contFechar++;
        }

        printf("\n");
        contadorManobras++;
    }

    long int colunaPi;
    long int prioridade;
    long int auxiliar;
    /*if (totalConfiguracoes > 1) {
        //calcula o valor de prioridade para cada chave

        for (contador = 0; contador < contFechar; contador++) {
            noP = noPRestabelecimento[contador];
            colunaPi = retornaColunaPi(matrizPI, vetorPi, noPRestabelecimento[contador], indiceConfiguracao);
            indiceP = matrizPI[noPRestabelecimento[contador]].colunas[colunaPi].posicao;
            rnpP = matrizPI[noPRestabelecimento[contador]].colunas[colunaPi].idRNP;
            contadorAux = indiceP + 1;
            prioridade = 100 * grafoSetoresParam[noP].numeroConsumidoresEspeciais + (grafoSetoresParam[noP].numeroConsumidores - grafoSetoresParam[noP].numeroConsumidoresEspeciais);
            while (contadorAux < configuracao[indiceConfiguracao].rnp[rnpP].numeroNos && configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].profundidade > configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade) {
                prioridade += 100 * grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais + (grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidores - grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais);
                contadorAux++;
            }
            valorPrioridade[contador] = prioridade;
        }
        //ordena as chaves a serem fechadas de acordo com o valor de prioridade definido

        for (contador = 0; contador < (contFechar - 1); contador++) {
            for (contadorAux = contador + 1; contadorAux < (contFechar); contadorAux++) {
                if (valorPrioridade[contador] < valorPrioridade[contadorAux]) {
                    auxiliar = valorPrioridade[contador];
                    valorPrioridade[contador] = valorPrioridade[contadorAux];
                    valorPrioridade[contadorAux] = auxiliar;
                    auxiliar = posChavesFecharRest[contador];
                    posChavesFecharRest[contador] = posChavesFecharRest[contadorAux];
                    posChavesFecharRest[contadorAux] = auxiliar;
                }
            }

        }
    }*/

    printf("manobras alivio: \n");
    configuracoesParam[indiceAnterior].objetivo.manobrasAlivio = 0;
    //copia o indice das chaves utilizadas para obter a configuração final
    for (contador = 1; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        paresManobrasAbrir[contador - 1] = vetorPiChav[indiceInd].idChaveAberta[0];
        paresManobrasFechar[contador - 1] = vetorPiChav[indiceInd].idChaveFechada[0];
        nosPRA[contador - 1].p = vetorPiChav[indiceInd].nos[0].p;
        nosPRA[contador - 1].r = vetorPiChav[indiceInd].nos[0].r;
        nosPRA[contador - 1].a = vetorPiChav[indiceInd].nos[0].a;
        chaveamento->paresManobras[contador - 1] = contador - 1;
        printf("\t abrir %s ", listaChavesParam[vetorPiChav[indiceInd].idChaveAberta[0]].codOperacional);
        printf("\t fechar %s\n", listaChavesParam[vetorPiChav[indiceInd].idChaveFechada[0]].codOperacional);
        configuracoesParam[indiceAnterior].objetivo.manobrasAlivio = configuracoesParam[indiceAnterior].objetivo.manobrasAlivio + 2;
    }
    //calcula a prioridade dos pares de manobras de alivio
    for (contador = 0; contador < totalConfiguracoes - 1; contador++) {
        noP = nosPRA[contador].p;
        colunaPi = retornaColunaPi(matrizPI, vetorPi, noP, indiceConfiguracao);
        indiceP = matrizPI[noP].colunas[colunaPi].posicao;
        rnpP = matrizPI[noP].colunas[colunaPi].idRNP;
        contadorAux = indiceP + 1;
/**/        prioridade = 100 * grafoSetoresParam[noP].numeroConsumidoresEspeciais + (grafoSetoresParam[noP].numeroConsumidores - grafoSetoresParam[noP].numeroConsumidoresEspeciais);
        while (contadorAux < configuracao[indiceConfiguracao].rnp[rnpP].numeroNos && configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].profundidade > configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade) {
            prioridade += 100 * grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais + (grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidores - grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais);
            contadorAux++;
        }
        valorPrioridade[contador] = prioridade;
    }

    //ordena os pares de manobras de alivio
    for (contador = 0; contador < (totalConfiguracoes - 2); contador++) {
        for (contadorAux = contador + 1; contadorAux < (totalConfiguracoes - 1); contadorAux++) {
            if (valorPrioridade[contador] < valorPrioridade[contadorAux]) {
                auxiliar = valorPrioridade[contador];
                valorPrioridade[contador] = valorPrioridade[contadorAux];
                valorPrioridade[contadorAux] = auxiliar;
                auxiliar = chaveamento->paresManobras[contador];
                chaveamento->paresManobras[contador] = chaveamento->paresManobras[contadorAux];
                chaveamento->paresManobras[contadorAux] = auxiliar;
            }
        }

    }


    //simula o fechamento das chaves que são possíveis e transfere os setores em falta e demais setores a jusante para a rnp de desligados
    //aloca a rnp responsavel por armazenar os setores em falta e sem fornecimento
    alocaRNP(tamanho, &configuracao[indiceConfiguracao].rnp[configuracao[indiceConfiguracao].numeroRNP - 1]);
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    //cria a raiz do alimentador ficticio
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].idNo = 0;
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].profundidade = 0;
    avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
            indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);

    //imprimeIndividuo(configuracao[indiceConfiguracao]);
    //Inicia a tentativa de fechamento das chaves para restabelecer o sistema
    contador = 0;
    while (contador < contFechar) {
        contadorFechamento = posChavesFecharRest[contador];
        if (vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento] > 0) {
            noP = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].p;
            noR = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].r;
            noA = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].a;
            //verifica se o par de chaveamento continua valido
            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
            if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA)) {
                //aplica a operação de fechamento da chave para restabelecer o setor a jusante a falta
                tamanhoTemporario = indiceL - indiceP + 1;
                if (listaChavesParam[vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento]].subTipoChave == chaveSeca) {
                    //verifica a carga do trecho
                    //recupera o setor raiz do setor P
                    indiceSetorRaiz = indiceP - 1;
                    while ((indiceSetorRaiz >= 0) && (configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                        indiceSetorRaiz--;
                    }
                    carga = cargaTrecho(configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                }
                if (rnpA == rnpP) {
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                } else {
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                    //obtém as novas rnps origem e destino
                    constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                    constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                    configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                }
                avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                        indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                if (!verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR)) {
                    //a configuração obtida não é factível em relação as restrições relaxadas
                    //desfaz a operação aplicada
                    if (rnpA == rnpP) {
                        free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                        configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                    } else {
                        free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                        configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                        free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                        configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                        configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                    }
                    //transfere para a rnp de setores em fornecimento, aplicando o operador correspondente
                    noA = 0;
                    indiceA = 0;
                    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                    //obtém as novas rnps origem e destino
                    constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                    constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                    configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;

                    //faz o no p ter o mesmo valor do nó r
                    vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].p = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].r;
                    //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                    chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = contadorFechamento;
                    chaveamento->numChavesRestab++;
                } else {
                    if ((listaChavesParam[vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento]].subTipoChave == chaveSeca) && (carga > 720)) {
                        tamanhoAlocacao += 2;
                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                        //abertura do alimentador
                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                        //fechamento da chave
                        //adiciona na lista de chaves manobradas
                        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                        //fechamento do alimentador
                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                    } else {
                        //adiciona na lista de chaves manobradas
                        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                    }
                }

            } else {
                //salva no vetor de elementos do chaveamento o idP da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido a não garantir a radialidade;
                chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = contadorFechamento;
                chaveamento->numChavesRestab++;
            }
        }
        contador++;
    }
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    for (contador = 0; contador < numeroSetoresFalta; contador++) {
        colunaPI = retornaColunaPi(matrizPI, vetorPi, setoresFalta[contador], idConfiguracao);
        indiceP = matrizPI[setoresFalta[contador]].colunas[colunaPI].posicao;
        rnpP = matrizPI[setoresFalta[contador]].colunas[colunaPI].idRNP;
        if (rnpP != rnpA) {
            indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
            indiceR = indiceP;
            tamanhoTemporario = indiceL - indiceP + 1;
            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
            //obtém as novas rnps origem e destino
/**/            constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
/**/            constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);

            //###############   ARMAZENA O PONTEIRO PARA FLORESTA E LIBERA A MEMORIA DAS RNPS SUBSTITUÍDAS   ###############
            // free(configuracao[0].rnp[rnpA].nos);
            //free(configuracao[0].rnp[rnpP].nos);
            rnpOrigem.fitnessRNP = configuracao[indiceConfiguracao].rnp[rnpP].fitnessRNP;
            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
            //até esse ponto o setor em falta foi isolado do alimentador.
            //Já foi inserido na sequencia de chaveamento a abertura de todas as chaves para isolar a falta
        }
    }

    //   imprimeIndividuo(configuracao[indiceConfiguracao]);
    contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
    chaveamento->consumidoresCorteCarga = consumidoresDesligados(configuracao[indiceConfiguracao], configuracao[indiceConfiguracao].numeroRNP - 1,
            grafoSetoresParam, setoresFalta, numeroSetoresFalta);
    chaveamento->configuracao = configuracao;
    insercaoElemFim(listaChaveamentos, chaveamento, chaveamentoAnt);
    //salva o ponteiro do chaveamento anterior
    listaAncestrais->inicio = chaveamento;
    listaAncestrais->fim = chaveamento;
    listaAncestrais->tamanho = 1;
    chaveamentoAnt = listaAncestrais->inicio;
    contadorAncestrais = listaAncestrais->tamanho;
    idConfiguracao++;
    encerra = false;
    if ((chaveamentoAnt->numPares > 0) || (chaveamentoAnt->numChavesRestab > 0)) {
        while (!encerra && (chaveamentoAnt->numPares > 0)) { //verifica se já encontrou um chaveamento factivel e se ainda existem pares para ser manobrados
            //cria o primeiro novo chaveamento
            chaveamento = Malloc(ELEMENTO, 1);
            chaveamento->anterior = chaveamentoAnt;
            tamanhoAlocacao = chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab;
            chaveamento->chavesRestabFechar = Malloc(long int, chaveamentoAnt->numChavesRestab);
            chaveamento->estadoFinal = Malloc(ESTADOCHAVE, (tamanhoAlocacao)); //aplica um par e tenta fechar todas as chaves do restabelecimento
            chaveamento->idChaves = Malloc(long int, (tamanhoAlocacao));
            chaveamento->reguladorTensao = Malloc(BOOL, (tamanhoAlocacao));
            chaveamento->numChavesRestab = 0;
            chaveamento->numPares = 0;
            chaveamento->paresManobras = Malloc(int, chaveamentoAnt->numPares);
            listaAncestrais->inicio = chaveamento;
            listaAncestrais->tamanho = 0;
            while (contadorAncestrais > 0 && !encerra) {
                //para cada possível ancestral percorre a lista de pares de manobras disponíveis
                contadorManobras = 0;
                //printf("numero pares manobras restante %d \n",chaveamentoAnt->numPares);
                while (contadorManobras < chaveamentoAnt->numPares && !encerra) {
                    chaveamento->numeroChaveamentos = chaveamentoAnt->numeroChaveamentos;
                    //                    printf("chaveamentoAnt %d \n", chaveamentoAnt->numeroChaveamentos);
                    for (contadorAux = 0; contadorAux < chaveamentoAnt->numeroChaveamentos; contadorAux++) {
                        chaveamento->estadoFinal[contadorAux] = chaveamentoAnt->estadoFinal[contadorAux];
                        chaveamento->idChaves[contadorAux] = chaveamentoAnt->idChaves[contadorAux];
                        chaveamento->reguladorTensao[contadorAux] = chaveamentoAnt->reguladorTensao[contadorAux];
                    }
                    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
                    configuracao = alocaIndividuo(numeroRnps, indiceConfiguracao, 1, numeroTrafosParam);
                    configuracao[indiceConfiguracao].idConfiguracao = idConfiguracao;
                    //copia a configuração inicial na configuração que será manipulada
/**/                    copiaIndividuo(chaveamentoAnt->configuracao, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);
                    //aplica um par de manobra
                    indiceChav = chaveamentoAnt->paresManobras[contadorManobras];
                    noP = nosPRA[indiceChav].p;
                    noR = nosPRA[indiceChav].r;
                    noA = nosPRA[indiceChav].a;
                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
                    //determinação do intervalo correspondente a subárvore do nó P na RNP
                    //    printf("rnpP %d indiceP %d numeroNosRnpP %d \n", rnpP, indiceP, configuracao[indiceConfiguracao].rnp[rnpP].numeroNos);
                    indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
                    factibilidadeAnel = false;
                    //precisa testar também se a rnpA não é a rnp de setores sem fornecimento
                    if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA) && (rnpA != (numeroRnps - 1))) {
                        //verifica a factilidade do anel antes de aplicar a manobra
/**/                        factibilidadeAnel = verificaFactibilidadeAnel(configuracao, noR, noA, rnpP, rnpA,
                                grafoSDRParam, numeroBarras, dadosAlimentadorSDRParam,
                                dadosTrafoSDRParam, indiceConfiguracao, rnpSetoresParam,
                                SBase, numeroSetores, Z, indiceRegulador,
                                dadosRegulador, maximoCorrente, numeroTrafosParam, numeroAlimentadores);
                        //aplica a manobra
                        tamanhoTemporario = indiceL - indiceP + 1;
                        if ((listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) || (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca)) {
                            //verifica a carga do trecho
                            //recupera o setor raiz do setor P
                            indiceSetorRaiz = indiceP - 1;
                            while ((indiceSetorRaiz >= 0) && (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                                indiceSetorRaiz--;
                            }
                            carga = cargaTrecho(chaveamentoAnt->configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                        }
                        if (rnpA == rnpP) {
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                            rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                            constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                            avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                    indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                            factibilidadeManobra = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);

                        } else {
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                            rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                            rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                            //obtém as novas rnps origem e destino
/**/                            constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
/**/                        constroiRNPOrigem(configuracao[indiceConfiguracao].rnp[rnpP], &rnpOrigem, indiceP, indiceL, matrizPI, idConfiguracao, rnpP);
                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                            avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                    indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                            factibilidadeManobra = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);

                        }
                        if (!factibilidadeManobra) {
                            if (rnpA == rnpP) {
                                //a configuração obtida não é factível em relação as restrições relaxadas
                                //desfaz a operação aplicada
                                free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                //salva no vetor de pares de manobras. Nesse caso devido as restrições elétricas;
                                chaveamento->paresManobras[chaveamento->numPares] = indiceChav;
                                chaveamento->numPares++;
                            } else {
                                //a configuração obtida não é factível em relação as restrições relaxadas
                                //desfaz a operação aplicada
                                free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                                configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                                configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                                //salva no vetor de pares de manobras. Nesse caso devido as restrições elétricas;
                                chaveamento->paresManobras[chaveamento->numPares] = indiceChav;
                                chaveamento->numPares++;
                            }
                        } else {
                            if (factibilidadeAnel) {
                                //adiciona na lista de chaves manobradas

                                if (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho para fechamento de 720kVA
                                    if (carga > 720) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //fechamento da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);

                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                }
                                if (listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho de 120kVA para abertura
                                    if (carga > 120) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceP, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //abertura da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                }
                            } else {
                                //nao foi possivel operar em anel insere as manobras no modelo abre-fecha
                                //adiciona na lista de chaves manobradas
                                if (listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho de 120kVA para abertura
                                    if (carga > 120) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceP, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //abertura da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                }

                                if (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho para fechamento de 720kVA
                                    if (carga > 720) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //fechamento da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);

                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                }

                            }
                            //copia as manobras anteriores que não foram aplicadas ainda
                            for (contadorAux = 0; contadorAux < contadorManobras; contadorAux++) {
                                chaveamento->paresManobras[chaveamento->numPares] = chaveamentoAnt->paresManobras[contadorAux];
                                chaveamento->numPares++;
                            }
                            //copia os pares de manobras restante
                            for (contadorAux = contadorManobras + 1; contadorAux < chaveamentoAnt->numPares; contadorAux++) {

                                chaveamento->paresManobras[chaveamento->numPares] = chaveamentoAnt->paresManobras[contadorAux];
                                chaveamento->numPares++;
                            }
                            //tenta restabelecer os setores sem fornecimento
                            contadorFechamento = 0;
                            while (contadorFechamento < chaveamentoAnt->numChavesRestab) {
                                noP = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].p;
                                noR = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].r;
                                noA = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].a;
                                //verifica se o par de chaveamento continua valido
                                recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
                                indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);

                                if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA)) {
                                    //aplica a operação de fechamento da chave para restabelecer o setor a jusante a falta
                                    tamanhoTemporario = indiceL - indiceP + 1;
                                    if (listaChavesParam[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].subTipoChave == chaveSeca) {
                                        //verifica a carga do trecho
                                        //recupera o setor raiz do setor P
                                        indiceSetorRaiz = indiceP - 1;
                                        while ((indiceSetorRaiz >= 0) && (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                                            indiceSetorRaiz--;
                                        }
                                        carga = cargaTrecho(chaveamentoAnt->configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                                    }
                                    if (rnpA == rnpP) {
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                                        rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                                        constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                                        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                                        factibilidadeRestab = verificaFactibilidade(configuracao[0], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);
                                        if (!factibilidadeRestab) {
                                            //a configuração obtida não é factível em relação as restrições relaxadas
                                            //desfaz a operação aplicada
                                            free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                            //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                                            chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                            chaveamento->numChavesRestab++;
                                        } else {
                                            //adiciona na lista de chaves manobradas
                                            insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        }
                                    } else {
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                                        rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                                        rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                                        //obtém as novas rnps origem e destino
                                        constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                                        constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                                        configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                                        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                                        factibilidadeRestab = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafoR, maxQuedaTensaoR);
                                        if (!factibilidadeRestab) {
                                            //a configuração obtida não é factível em relação as restrições relaxadas
                                            //desfaz a operação aplicada
                                            free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                            free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                                            //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                                            chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                            chaveamento->numChavesRestab++;
                                        } else {

                                            if ((listaChavesParam[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].subTipoChave == chaveSeca) && (carga > 720)) {
                                                //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                                //será incluída a manobra de abertura e fechamento do alimentador
                                                tamanhoAlocacao += 2;
                                                chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                                chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                                chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                                idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                                //abertura do alimentador
                                                insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                                //fechamento da chave
                                                insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                                //fechamento do alimentador
                                                insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                            } else {

                                                //adiciona na lista de chaves manobradas
                                                insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                            }
                                        }
                                    }
                                } else {
                                    //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido a não garantir a radialidade;
                                    chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                    chaveamento->numChavesRestab++;
                                }

                                contadorFechamento++;
                            }
                        }
                        if (factibilidadeManobra || factibilidadeRestab) {
                            //insere a configuracao no chaveamento
                            contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
                            chaveamento->consumidoresCorteCarga = consumidoresDesligados(configuracao[indiceConfiguracao], configuracao[indiceConfiguracao].numeroRNP - 1,
                                    grafoSetoresParam, setoresFalta, numeroSetoresFalta);
                            chaveamento->configuracao = configuracao;
                            //insere o chaveamento na lista de chaveamentos
                            insercaoElemFim(listaChaveamentos, chaveamento, chaveamentoAnt);
                            //atualiza o ponteiro de fim e o contador da lista de ancestrais
                            listaAncestrais->fim = chaveamento;
                            listaAncestrais->tamanho++;
                            if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao) && chaveamento->numChavesRestab == 0) {
                                if (listaFactiveis->tamanho == 0) {
                                    insercaoListaVazia(listaFactiveis, chaveamento);
                                } else {
                                    insercaoElemFim(listaFactiveis, chaveamento, chaveamentoAnt);
                                }
                                encerra = true;
                            } else {
                                //        printf("idconfi %ld numeroChaveamentos %d\n", configuracao[indiceConfiguracao].idConfiguracao, chaveamento->numeroChaveamentos);
                                //cria um novo chaveamento
                                chaveamento = Malloc(ELEMENTO, 1);
                                chaveamento->anterior = NULL;
                                chaveamento->chavesRestabFechar = Malloc(long int, chaveamentoAnt->numChavesRestab);
                                chaveamento->estadoFinal = Malloc(ESTADOCHAVE, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                chaveamento->idChaves = Malloc(long int, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab));
                                chaveamento->reguladorTensao = Malloc(BOOL, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab));
                                chaveamento->numChavesRestab = 0;
                                chaveamento->numPares = 0;
                                chaveamento->paresManobras = Malloc(int, chaveamentoAnt->numPares);
                                idConfiguracao++;
                            }
                        }
                    }
                    contadorManobras++;
                }

                //recupera o próximo ancestral
                chaveamentoAnt = chaveamentoAnt->proximo;
                contadorAncestrais--;
            }
            chaveamentoAnt = listaAncestrais->inicio;
            contadorAncestrais = listaAncestrais->tamanho;
        }
    } else {
        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
        if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao) && chaveamento->numChavesRestab == 0) {
            if (listaFactiveis->tamanho == 0) {
                insercaoListaVazia(listaFactiveis, chaveamento);
            } else {
                insercaoElemFim(listaFactiveis, chaveamento, chaveamentoAnt);
            }
        }
    }
    indiceInd = 0;
    if (listaFactiveis->tamanho > 0) {
        chaveamento = listaFactiveis->inicio;
        configuracao = chaveamento->configuracao;

        while (chaveamento != NULL) {
            if (chaveamento->numeroChaveamentos != numeroManobras)
                salvaMelhorConfiguracao(chaveamento->configuracao[indiceConfiguracao], numeroSetoresFalta, 0, chaveamento->configuracao[indiceConfiguracao].idConfiguracao, setoresFalta);
            printf("chaveamento factivel %ld: manobras %d\n", chaveamento->configuracao[indiceConfiguracao].idConfiguracao, chaveamento->numeroChaveamentos);
            configuracoesParam[indiceAnterior].objetivo.manobrasAposChaveamento = chaveamento->numeroChaveamentos;
            for (contadorAux = 0; contadorAux < chaveamento->numeroChaveamentos; contadorAux++) {
                printf("\t %s %d", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, chaveamento->estadoFinal[contadorAux]);
                //   printf("\t %ld", chaveamento->idChaves[contadorAux]);
            }
            printf("\n\n");
            sprintf(nomeFinal, "SAIDA_sequenciaChaveamentoFactivel%s.dad", nomeSetorFalta);
            salvaChaveamento(chaveamento, nomeFinal, listaChavesParam);
            //Por Leandro: Comente aqui para não fazer a impressão do arquivo de chaves a cada execução
            //sprintf(nomeFinal, "chavesCompleto%s.dad", nomeSetorFalta);
            //gravaChavesCompleto(numeroChavesParam, nomeFinal, listaChavesParam);
            chaveamento = chaveamento->proximo;
        }
        //salva os individuos
        chaveamento = listaFactiveis->inicio;
        sprintf(nomeFinal, "Factivel_RNP_setorFalta%s.dad", nomeSetorFalta);
        gravaIndividuo(nomeFinal, chaveamento->configuracao[0]);
        sprintf(nomeFinal, "SAIDA_individuoFactivel_CaracteristicasEletricas_setorFalta%s.dad", nomeSetorFalta);
        gravaDadosEletricos(nomeFinal, chaveamento->configuracao[0]);
        FILE *arquivo;
        arquivo = fopen(nomeFinal, "a");
        fprintf(arquivo, "CONSUMIDORES CORTE CARGA %ld\n", chaveamento->consumidoresCorteCarga);
        //configuracoesParam[indiceAnterior].objetivo.consumidoresDesligadosEmCorteDeCarga = chaveamento->consumidoresCorteCarga;
        configuracoesParam[indiceAnterior].objetivo.consumidoresDesligadosEmCorteDeCarga = 0;
        fclose(arquivo);
        //recupera os individuos intermediarios
        /*
                      configuracoes = Malloc(CONFIGURACAO, 2 * totalConfiguracoes);

                      while (chaveamento != NULL) {
                          //      printf("indiceInd %ld totalConfiguracoes %d \n", indiceInd, totalConfiguracoes);
                          configuracoes[indiceInd] = chaveamento->configuracao[0];
                          indiceInd++;
                          chaveamento = chaveamento->anterior;
                      }
                      contador = 0;
                      while (indiceInd > 0) {
                          sprintf(nomeFinal, "Factivel_%d_RNP_setorFalta%s.dad", contador, nomeSetorFalta);
                          gravaIndividuo(nomeFinal, configuracoes[indiceInd - 1]);
                          sprintf(nomeFinal, "individuoFactivel_%d_CaracteristicasEletricas_setorFalta%s.dad", contador, nomeSetorFalta);
                          gravaDadosEletricos(nomeFinal, configuracoes[indiceInd - 1]);
                          indiceInd--;
                          contador++;
                      }*/

    } else {
        /* chaveamento = listaChaveamentos->inicio;
         chaveamento = chaveamento->proximo;
         contador = 1;
         while (chaveamento != NULL) {
             if (chaveamento->numeroChaveamentos != numeroManobras)
                 salvaMelhorConfiguracao(chaveamento->configuracao[indiceConfiguracao], numeroSetoresFalta, 0, chaveamento->configuracao[indiceConfiguracao].idConfiguracao, setoresFalta);
             printf("chaveamento Infactivel %ld: \n", chaveamento->configuracao[indiceConfiguracao].idConfiguracao);
             for (contadorAux = 0; contadorAux < chaveamento->numeroChaveamentos; contadorAux++) {
                 printf("\t %s %d", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, chaveamento->estadoFinal[contadorAux]);
                 //   printf("\t %ld", chaveamento->idChaves[contadorAux]);
             }
             printf("\n\n");

             sprintf(nomeFinal, "sequenciaChaveamento%s.dad", nomeSetorFalta);
             salvaChaveamento(chaveamento, nomeFinal, listaChavesParam);

             sprintf(nomeFinal, "Infactivel_%d_RNP_setorFalta%s.dad", contador, nomeSetorFalta);
             gravaIndividuo(nomeFinal, chaveamento->configuracao[0]);
             sprintf(nomeFinal, "individuoInfactivel_%d_CaracteristicasEletricas_setorFalta%s.dad", contador, nomeSetorFalta);
             gravaDadosEletricos(nomeFinal, chaveamento->configuracao[0]);
             contador++;
             chaveamento = chaveamento->proximo;
         }*/
        otimizaChaveamentoCorteCarga(dadosAlimentadorSDRParam, dadosTrafoSDRParam,
                maximoCorrente, Z, rnpSetoresParam, SBase, grafoSetoresParam,
                numeroSetores, setoresFalta, numeroSetoresFalta, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, configuracaoInicial, vetorPiChav, idConfiguracaoParam,
                grafoSDRParam, numeroBarras, listaChavesParam, numeroManobras,
                consumidores, consumidoresEspeciais, seed, indiceAnterior, configuracoesParam, numeroChavesParam);
    }
    desalocaMatrizPI(matrizPI, numeroSetores);
    free(matrizPI);
    free(posChavesFecharRest);
    free(valorPrioridade);
    free(paresManobrasAbrir);
    free(paresManobrasFechar);
}

void gravaChavesCompleto(long int numeroChavesParam, char *nomeArquivo, LISTACHAVES *listaChavesParam)
{
    FILE *arquivo;
    long int contador;
    
    arquivo = fopen(nomeArquivo,"w");
    fprintf(arquivo,"%ld\n", numeroChavesParam);
    for(contador = 1; contador <=numeroChavesParam; contador++)
    {
        fprintf(arquivo, "%ld\t%s\t%d\t%ld\t%ld\t%d\t%d\t%d\n", listaChavesParam[contador].identificador, listaChavesParam[contador].codOperacional, listaChavesParam[contador].estadoChave, listaChavesParam[contador].idNoDe, listaChavesParam[contador].idNoPara, listaChavesParam[contador].tipoChave, listaChavesParam[contador].subTipoChave, listaChavesParam[contador].condicao);
    }
    fclose(arquivo);

}

void insereManobraChaveamento(ELEMENTO *chaveamento, long int idChave, BOOL temReguladorTensao, ESTADOCHAVE estado) {
    chaveamento->idChaves[chaveamento->numeroChaveamentos] = idChave;
    chaveamento->reguladorTensao[chaveamento->numeroChaveamentos] = temReguladorTensao;
    chaveamento->estadoFinal[chaveamento->numeroChaveamentos] = estado;
    chaveamento->numeroChaveamentos++;
}

long int buscaChaveAlimentador(RNP rnpParam, GRAFOSETORES *grafoSetoresParam) {
    long int setor1, setor2;
    int contador = 0;
    setor1 = rnpParam.nos[0].idNo;
    setor2 = rnpParam.nos[1].idNo;
    while (contador < grafoSetoresParam[setor1].numeroAdjacentes && grafoSetoresParam[setor1].setoresAdjacentes[contador] != setor2) {
        contador++;
    }
    if (contador < grafoSetoresParam[setor1].numeroAdjacentes) {
        return grafoSetoresParam[setor1].idChavesAdjacentes[contador];
    } else
        return -1;
}

//dado o identificador de dois setores retorna a chave entre eles se houver ou -1 caso contrário

long int buscaChaveSetor(long int setor1, long int setor2, GRAFOSETORES *grafoSetoresParam) {

    int contador = 0;
    while ((contador < grafoSetoresParam[setor1].numeroAdjacentes) && (grafoSetoresParam[setor1].setoresAdjacentes[contador] != setor2)) {
        contador++;
    }
    if (contador < grafoSetoresParam[setor1].numeroAdjacentes) {
        return grafoSetoresParam[setor1].idChavesAdjacentes[contador];
    } else
        return -1;
}

//busca pela chave automática mais próxima na rnp para isolar um trecho a ser manobrado. 
//Recebe como parâmetros o indice do setor inicial do trecho a ser isolado, a rnp do alimentador, o grafo de setores e a lista de chaves

long int buscaChaveDesligaTrecho(long int indiceSetor, RNP rnpParam, GRAFOSETORES *grafoSetores, LISTACHAVES *listaChavesParam) {
    int contador1, contador2;
    long int setor1, setor2, idChave;
    BOOL encontrado = false;
    contador1 = indiceSetor - 1;
    contador2 = indiceSetor;

    while (!encontrado && (contador1 >= 0)) {
        while ((contador1 >= 0) && (rnpParam.nos[contador1].profundidade != (rnpParam.nos[contador2].profundidade - 1))) {
            contador1--;
        }
        if (rnpParam.nos[contador1].profundidade == (rnpParam.nos[contador2].profundidade - 1)) {
            setor1 = rnpParam.nos[contador1].idNo;
            setor2 = rnpParam.nos[contador2].idNo;
            idChave = buscaChaveSetor(setor1, setor2, grafoSetores);
            if (idChave > 0) {
                if (listaChavesParam[idChave].tipoChave == chaveAutomatica) {
                    encontrado = true;
                }
            }
        }
        if (contador1 >= 0 && !encontrado) {
            contador2 = contador1;
            contador1 = contador2 - 1;
        }
    }
    if (encontrado) {
        return idChave;
    } else {
        return -1;
    }

}

//verifica se um conjunto de nós p, r,a consistem em uma aplicação dos operadores da RNP

BOOL verificaOperadorRNP(int indiceP, int indiceR, int indiceA, int indiceL, int rnpP, int rnpR, int rnpA) {
    if (rnpP == rnpR) //os nós p e r pertencem a mesma rnp
    {
        if ((indiceR >= indiceP) && (indiceR <= indiceL)) {
            if (rnpA == rnpP) {
                if ((indiceA < indiceP) || (indiceA > indiceL)) //para ser uma aplicação de PAO ou CAO o nó A pode estar na mesma rnp mas não pode estar na subárvore podada
                {
                    return true;
                } else
                    return false;
            } else {
                return true;
            }
        } else {
            return false; //o nó r não pertence a subárvore do nó p
        }
    } else
        return false;
}

BOOL verificaPresencaRT(int indiceNo, RNP vetorRNP, RNPSETORES *matrizB, int *indiceRegulador) {

    long int indice, idSetorS, idSetorR, idBarra1, idBarra2, indice1, indice2, indice3;
    RNPSETOR rnpSetorSR;
    int contadorRegulador = 0;
    indice = indiceNo;
    //printf("indice %ld numeronos %d raiz %ld \n", indice, vetorRNP.numeroNos,vetorRNP.nos[0].idNo);
    while (indice > 0) {
        indice1 = indice - 1;
        //busca pelo nó raiz
        while (indice1 >= 0 && vetorRNP.nos[indice1].profundidade != (vetorRNP.nos[indice].profundidade - 1)) {
            //   printf("indice1 %ld \n", indice1);
            indice1--;
        }

        idSetorS = vetorRNP.nos[indice].idNo;
        idSetorR = vetorRNP.nos[indice1].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, idSetorS, idSetorR);
        for (indice2 = rnpSetorSR.numeroNos - 1; indice2 > 0; indice2--) {
            indice3 = indice2 - 1;
            while (indice3 >= 0 && rnpSetorSR.nos[indice3].profundidade != (rnpSetorSR.nos[indice2].profundidade - 1)) {
                indice3--;
            }
            idBarra1 = rnpSetorSR.nos[indice3].idNo;
            idBarra2 = rnpSetorSR.nos[indice2].idNo;

            if ((indiceRegulador[idBarra1] != 0) && (indiceRegulador[idBarra2] != 0) && (indiceRegulador[idBarra1] == indiceRegulador[idBarra2])) {

                contadorRegulador++;
            }
        }
        indice = indice1;
    }
    if (contadorRegulador > 0) {
        return true;
    } else {
        return false;
    }
}

void inicializacao(LISTA *lista) {
    lista->inicio = NULL;
    lista->fim = NULL;
    lista->tamanho = 0;
}

void insercaoListaVazia(LISTA * lista, ELEMENTO *elem) {

    elem->proximo = lista->fim;
    lista->inicio = elem;
    lista->fim = elem;
    lista->tamanho++;

}

void insercaoElemFim(LISTA *lista, ELEMENTO *elem, ELEMENTO *elemAnt) {
    elem->anterior = elemAnt;
    lista->fim->proximo = elem;
    elem->proximo = NULL;
    lista->fim = elem;
    lista->tamanho++;
}

void destruir(LISTA *lista) {
    ELEMENTO *elemento1 = lista->inicio;
    ELEMENTO *elemento2;
    while (lista->tamanho > 0) {
        elemento2 = elemento1;
        elemento1 = elemento1->proximo;
        free(elemento2);
        lista->tamanho--;
    }
}

void contaManobrasChaveamento(CONFIGURACAO *configuracao, ELEMENTO *elem, LISTACHAVES *listaChavesParam) {
    int contador;
    configuracao[0].objetivo.manobrasAutomaticas = 0;
    configuracao[0].objetivo.manobrasManuais = 0;

    for (contador = 0; contador < elem[0].numeroChaveamentos; contador++) {
        if (listaChavesParam[elem[0].idChaves[contador]].tipoChave == chaveManual) {
            configuracao[0].objetivo.manobrasManuais++;
        } else {
            if (listaChavesParam[elem[0].idChaves[contador]].tipoChave == chaveAutomatica) {
                configuracao[0].objetivo.manobrasAutomaticas++;
            }
        }

    }
}
/**
 * O presente método define em que ordem as manobras dos ancestrais da configuração final serão executadas considerando o corte de carga.
 * Essa rotina é utilizada quando a configuração final não é factível ou quando não é possível encontrar uma sequência de manobras para uma configuração factivel.
 * O processo de execução é o mesmo do método otimiza chaveamento, com a alteração de que para verificação da factibilidade intermediária são utilizdas as restrições normais de operaçao.
 * Para cada manobra realizada verifica as características elétricas de forma a obter configurações intermediárias factíveis.
 * O processo inicia construíndo uma lista com todas as manobras necessárias para restabelecer os setores sãos e outra lista para as manobras de alívio.
 * As duas listas acima são criadas com ordem de prioridade definida por uma métrica relacionada ao número de consumidores manobrados.
 * A sequência de manobras é iniciada com as manobras para isolar a falta. A seguir cada uma das manobras de fechar para restabelecimento é analisada e são executadas/inseridas as que não violam as características elétricas.
 * As manobras de fechar do restabelecimento que não puderem ser aplicadas são mantidas na lista de chaves a serem manobradas.
 * A seguir inicia-se um processo iterativo de análise de cada uma das sub-sequências possíveis de manobras de alívio até que seja possível restabelecer todos os setores sãos da configuração final.
 * Para cada chave inserida na sequência de manobras é verificada as restrições operacionais de chave seca. Casos essas sejam violadas insere-se a manobra de abertura e fechamento da primeira chave automática a montante.
 * Além disso, para as manobras de alívio é verifica a operação em anel do sistema.
 * @param dadosAlimentadorSDRParam
 * @param dadosTrafoSDRParam
 * @param maximoCorrente
 * @param Z
 * @param rnpSetoresParam
 * @param SBase
 * @param grafoSetoresParam
 * @param numeroSetores
 * @param setoresFalta
 * @param numeroSetoresFalta
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param configuracaoInicial
 * @param vetorPiChav
 * @param idConfiguracaoParam
 * @param grafoSDRParam
 * @param numeroBarras
 * @param listaChavesParam
 * @param numeroManobras
 * @param consumidores
 * @param consumidoresEspeciais
 * @param seed
 */
void otimizaChaveamentoCorteCarga(DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, int SBase, GRAFOSETORES *grafoSetoresParam,
        long int numeroSetores, long int *setoresFalta, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
        int *indiceRegulador, DADOSREGULADOR *dadosRegulador, CONFIGURACAO configuracaoInicial, VETORPI *vetorPiChav, long int idConfiguracaoParam,
        GRAFO *grafoSDRParam, long int numeroBarras, LISTACHAVES *listaChavesParam, int numeroManobras, int consumidores, int consumidoresEspeciais, int seed,
        long int indiceAnterior, CONFIGURACAO *configuracoesParam, long int numeroChavesParam) {
    LISTA *listaChaveamentos = Malloc(LISTA, 1); //lista dos chaveamentos realizados
    LISTA *listaAncestrais = Malloc(LISTA, 1); //lista de chaveamentos anteriores
    LISTA *listaFactiveis = Malloc(LISTA, 1); //lista de chaveamentos que geram configuração factível
    BOOL encerra;
    ELEMENTO *chaveamento;
    ELEMENTO *chaveamentoAnt;
    int totalConfiguracoes;
    long int *posChavesFecharRest;
    long int *valorPrioridade;
    long int *paresManobrasAbrir;
    long int *paresManobrasFechar;
    NOSPRA *nosPRA;
    long int *noPRestabelecimento;
    int contador, contadorAux, rnpA, rnpP, rnpR, contadorManobras, contadorFechamento, contFechar;
    CONFIGURACAO *configuracao;
    CONFIGURACAO *configuracoes;
    long int idConfiguracao = 0;
    long int indiceInd, noP, noR, noA;
    int indiceP, indiceR, indiceA, indiceL;
    long int *indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam, &totalConfiguracoes);
    long int indiceConfiguracao = 0;
    VETORPI *vetorPi;
    MATRIZPI *matrizPI;
    RNP rnpOrigem, rnpOrigemAnterior;
    RNP rnpDestino, rnpDestinoAnterior;
    int contadorAncestrais, indiceChav;
    int numeroRnps = configuracaoInicial.numeroRNP + 1;
    int tamanhoTemporario, tamanhoAlocacao;
    long int colunaPI;
    BOOL factibilidadeManobra = false;
    BOOL factibilidadeRestab = false;
    BOOL factibilidadeAnel = false;
    char nomeFinal[100];
    double carga;
    int indiceSetorRaiz;
    long int idChaveAlimentador;
    char setorF[10];
    char nomeSetorFalta[100];
    //cria o nome do arquivo de saída
    sprintf(nomeSetorFalta, "_");
    for (contador = 0; contador < numeroSetoresFalta; contador++) {
        sprintf(setorF, "%ld_", setoresFalta[contador]);
        strcat(nomeSetorFalta, setorF);
    }
    //para colocar o seed
    sprintf(setorF, "%d", seed);
    strcat(nomeSetorFalta, setorF);

    printf("corte de carga... \n");
    //inicializa as estruturas de dados da rnp
    inicializaVetorPi(vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras + totalConfiguracoes * 10, &vetorPi);
    inicializaMatrizPI(grafoSetoresParam, &matrizPI, (vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras + totalConfiguracoes * 10), numeroSetores);
    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
    configuracao = alocaIndividuo(configuracaoInicial.numeroRNP + 1, idConfiguracao, 1, numeroTrafosParam);
    //copia a configuração inicial na configuração que será manipulada
    copiaIndividuo(&configuracaoInicial, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);

    configuracao[indiceConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciais;
    configuracao[indiceConfiguracao].objetivo.consumidoresSemFornecimento = consumidores;

    //inicia a lista de configurações de chaveamento
    inicializacao(listaChaveamentos);
    inicializacao(listaFactiveis);
    //aloca as estruturas de dados auxiliares para realizar o chaveamento
    posChavesFecharRest = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    valorPrioridade = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    noPRestabelecimento = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras);
    paresManobrasAbrir = Malloc(long int, totalConfiguracoes - 1);
    paresManobrasFechar = Malloc(long int, totalConfiguracoes - 1);
    nosPRA = Malloc(NOSPRA, totalConfiguracoes - 1);

    //aloca o chaveamento pré-falta
    chaveamento = Malloc(ELEMENTO, 1);
    chaveamento->anterior = NULL;
    chaveamento->chavesRestabFechar = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras - 1);
    chaveamento->estadoFinal = Malloc(ESTADOCHAVE, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->idChaves = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->reguladorTensao = Malloc(BOOL, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2);
    chaveamento->numeroChaveamentos = 0;
    chaveamento->numChavesRestab = 0;
    chaveamento->numPares = totalConfiguracoes - 1;
    chaveamento->paresManobras = Malloc(int, totalConfiguracoes - 1);
    insercaoListaVazia(listaChaveamentos, chaveamento);
    //aloca a rnp responsavel por armazenar os setores em falta e sem fornecimento
    int tamanho = 1;
    alocaRNP(tamanho, &configuracao[indiceConfiguracao].rnp[configuracao[indiceConfiguracao].numeroRNP - 1]);
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    //cria a raiz do alimentador ficticio
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].idNo = 0;
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].profundidade = 0;
    avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
            indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);

    contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
    chaveamento->consumidoresCorteCarga = 0;
    chaveamento->configuracao = configuracao;
    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
    configuracao = alocaIndividuo(configuracaoInicial.numeroRNP + 1, idConfiguracao, 1, numeroTrafosParam);
    //copia a configuração inicial na configuração que será manipulada
    copiaIndividuo(&configuracaoInicial, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);
    configuracao[indiceConfiguracao].objetivo.consumidoresEspeciaisSemFornecimento = consumidoresEspeciais;
    configuracao[indiceConfiguracao].objetivo.consumidoresSemFornecimento = consumidores;
    
    chaveamentoAnt = chaveamento;
    //aloca o primeiro chaveamento
    chaveamento = Malloc(ELEMENTO, 1);
    chaveamento->anterior = NULL;
    tamanhoAlocacao = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras * 2;
    chaveamento->chavesRestabFechar = Malloc(long int, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras - 1);
    chaveamento->estadoFinal = Malloc(ESTADOCHAVE, tamanhoAlocacao);
    chaveamento->idChaves = Malloc(long int, tamanhoAlocacao);
    chaveamento->reguladorTensao = Malloc(BOOL, tamanhoAlocacao);
    chaveamento->numeroChaveamentos = 0;
    chaveamento->numChavesRestab = 0;
    chaveamento->numPares = totalConfiguracoes - 1;
    chaveamento->paresManobras = Malloc(int, totalConfiguracoes - 1);

    contadorManobras = 0;
    printf("chaves restabelecimento: \n");
    configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento = 0;
    contadorFechamento = 0;
    contFechar = 0;
    //copia as chaves utilizadas no restabelecimento para isolar a falta
    for (contador = 0; contador < vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].numeroManobras; contador++) {
        //insere na sequencia de chaveamento as chaves para isolar a falta e verifica a presença de regulador de tensão
        noP = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].nos[contador].p;
        colunaPI = retornaColunaPi(matrizPI, vetorPiChav, noP, idConfiguracao);
        indiceP = matrizPI[noP].colunas[colunaPI].posicao;
        rnpP = matrizPI[noP].colunas[colunaPI].idRNP;
        indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveAberta[contador], verificaPresencaRT(indiceL, configuracaoInicial.rnp[rnpP], rnpSetoresParam, indiceRegulador), normalmenteAberta);
        printf("\tabrir %s ", listaChavesParam[vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveAberta[contador]].codOperacional);
        configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento++;

        //monta a lista com as chaves que precisam ser fechadas no restabelecimento
        if (vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contador] > 0) {
            posChavesFecharRest[contFechar] = contador;
            noPRestabelecimento[contFechar] = vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].nos[contador].p;
            printf("\tfechar %s", listaChavesParam[vetorPiChav[indiceConfiguracoes[indiceConfiguracao]].idChaveFechada[contador]].codOperacional);
            contFechar++;
            configuracoesParam[indiceAnterior].objetivo.manobrasRestabelecimento++;
        }

        printf("\n");
        contadorManobras++;
    }

    //calcula o valor de prioridade para cada chave
    long int colunaPi;
    long int prioridade;
    for (contador = 0; contador < contFechar; contador++) {
        noP = noPRestabelecimento[contador];
        colunaPi = retornaColunaPi(matrizPI, vetorPi, noPRestabelecimento[contador], indiceConfiguracao);
        indiceP = matrizPI[noPRestabelecimento[contador]].colunas[colunaPi].posicao;
        rnpP = matrizPI[noPRestabelecimento[contador]].colunas[colunaPi].idRNP;
        contadorAux = indiceP + 1;
        prioridade = 100 * grafoSetoresParam[noP].numeroConsumidoresEspeciais + (grafoSetoresParam[noP].numeroConsumidores - grafoSetoresParam[noP].numeroConsumidoresEspeciais);
        while (contadorAux < configuracao[indiceConfiguracao].rnp[rnpP].numeroNos && configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].profundidade > configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade) {
            prioridade += 100 * grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais + (grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidores - grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais);
            contadorAux++;
        }
        valorPrioridade[contador] = prioridade;
    }
    //ordena as chaves a serem fechadas de acordo com o valor de prioridade definido
    long int auxiliar;
    for (contador = 0; contador < (contFechar - 1); contador++) {
        for (contadorAux = contador + 1; contadorAux < (contFechar); contadorAux++) {
            if (valorPrioridade[contador] < valorPrioridade[contadorAux]) {
                auxiliar = valorPrioridade[contador];
                valorPrioridade[contador] = valorPrioridade[contadorAux];
                valorPrioridade[contadorAux] = auxiliar;
                auxiliar = posChavesFecharRest[contador];
                posChavesFecharRest[contador] = posChavesFecharRest[contadorAux];
                posChavesFecharRest[contadorAux] = auxiliar;
            }
        }

    }

    printf("manobras alivio: \n");
    configuracoesParam[indiceAnterior].objetivo.manobrasAlivio = 0;
    //copia o indice das chaves utilizadas para obter a configuração final
    for (contador = 1; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        paresManobrasAbrir[contador - 1] = vetorPiChav[indiceInd].idChaveAberta[0];
        paresManobrasFechar[contador - 1] = vetorPiChav[indiceInd].idChaveFechada[0];
        nosPRA[contador - 1].p = vetorPiChav[indiceInd].nos[0].p;
        nosPRA[contador - 1].r = vetorPiChav[indiceInd].nos[0].r;
        nosPRA[contador - 1].a = vetorPiChav[indiceInd].nos[0].a;
        chaveamento->paresManobras[contador - 1] = contador - 1;
        printf("\t abrir %s ", listaChavesParam[vetorPiChav[indiceInd].idChaveAberta[0]].codOperacional);
        printf("\t fechar %s\n", listaChavesParam[vetorPiChav[indiceInd].idChaveFechada[0]].codOperacional);
        configuracoesParam[indiceAnterior].objetivo.manobrasAlivio = configuracoesParam[indiceAnterior].objetivo.manobrasAlivio + 2;
    }
    //calcula a prioridade dos pares de manobras de alivio
    for (contador = 0; contador < totalConfiguracoes - 1; contador++) {
        noP = nosPRA[contador].p;
        colunaPi = retornaColunaPi(matrizPI, vetorPi, noP, indiceConfiguracao);
        indiceP = matrizPI[noP].colunas[colunaPi].posicao;
        rnpP = matrizPI[noP].colunas[colunaPi].idRNP;
        contadorAux = indiceP + 1;
        prioridade = 100 * grafoSetoresParam[noP].numeroConsumidoresEspeciais + (grafoSetoresParam[noP].numeroConsumidores - grafoSetoresParam[noP].numeroConsumidoresEspeciais);
        while (contadorAux < configuracao[indiceConfiguracao].rnp[rnpP].numeroNos && configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].profundidade > configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade) {
            prioridade += 100 * grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais + (grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidores - grafoSetoresParam[configuracao[indiceConfiguracao].rnp[rnpP].nos[contadorAux].idNo].numeroConsumidoresEspeciais);
            contadorAux++;
        }
        valorPrioridade[contador] = prioridade;
    }

    //ordena os pares de manobras de alivio
    for (contador = 0; contador < (totalConfiguracoes - 2); contador++) {
        for (contadorAux = contador + 1; contadorAux < (totalConfiguracoes - 1); contadorAux++) {
            if (valorPrioridade[contador] < valorPrioridade[contadorAux]) {
                auxiliar = valorPrioridade[contador];
                valorPrioridade[contador] = valorPrioridade[contadorAux];
                valorPrioridade[contadorAux] = auxiliar;
                auxiliar = chaveamento->paresManobras[contador];
                chaveamento->paresManobras[contador] = chaveamento->paresManobras[contadorAux];
                chaveamento->paresManobras[contadorAux] = auxiliar;
            }
        }

    }


    //simula o fechamento das chaves que são possíveis e transfere os setores em falta e demais setores a jusante para a rnp de desligados
    //aloca a rnp responsavel por armazenar os setores em falta e sem fornecimento
    alocaRNP(tamanho, &configuracao[indiceConfiguracao].rnp[configuracao[indiceConfiguracao].numeroRNP - 1]);
    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    //cria a raiz do alimentador ficticio
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].idNo = 0;
    configuracao[indiceConfiguracao].rnp[rnpA].nos[0].profundidade = 0;
    avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
            indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);

    //imprimeIndividuo(configuracao[indiceConfiguracao]);
    //Inicia a tentativa de fechamento das chaves para restabelecer o sistema
    contador = 0;
    while (contador < contFechar) {
        contadorFechamento = posChavesFecharRest[contador];
        if (vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento] > 0) {
            noP = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].p;
            noR = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].r;
            noA = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].a;
            //verifica se o par de chaveamento continua valido
            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
            //determinação do intervalo correspondente a subárvore do nó P na RNP
            indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
            if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA)) {
                //aplica a operação de fechamento da chave para restabelecer o setor a jusante a falta
                tamanhoTemporario = indiceL - indiceP + 1;
                if (listaChavesParam[vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento]].subTipoChave == chaveSeca) {
                    //verifica a carga do trecho
                    //recupera o setor raiz do setor P
                    indiceSetorRaiz = indiceP - 1;
                    while ((indiceSetorRaiz >= 0) && (configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                        indiceSetorRaiz--;
                    }
                    carga = cargaTrecho(configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                }
                if (rnpA == rnpP) {
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                } else {
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                    //obtém as novas rnps origem e destino
                    constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                    constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                    configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                }
                avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                        indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                if (!verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
                    //a configuração obtida não é factível em relação as restrições
                    //desfaz a operação aplicada
                    if (rnpA == rnpP) {
                        free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                        configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                    } else {
                        free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                        configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                        free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                        configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                        configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                    }
                    //transfere para a rnp de setores em fornecimento, aplicando o operador correspondente
                    noA = 0;
                    indiceA = 0;
                    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                    alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                    rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                    rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                    //obtém as novas rnps origem e destino
                    constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                    constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                    configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                    configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;

                    //faz o no p ter o mesmo valor do nó r
                    vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].p = vetorPiChav[indiceConfiguracoes[0]].nos[contadorFechamento].r;
                    //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                    chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = contadorFechamento;
                    chaveamento->numChavesRestab++;
                } else {
                    if ((listaChavesParam[vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento]].subTipoChave == chaveSeca) && (carga > 720)) {
                        tamanhoAlocacao += 2;
                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                        //abertura do alimentador
                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                        //fechamento da chave
                        //adiciona na lista de chaves manobradas
                        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                        //fechamento do alimentador
                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                    } else {
                        //adiciona na lista de chaves manobradas
                        insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[contadorFechamento], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                    }

                }

            } else {
                //salva no vetor de elementos do chaveamento o idP da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido a não garantir a radialidade;
                chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = contadorFechamento;
                chaveamento->numChavesRestab++;
            }
        }
        contador++;
    }

    //os setores em falta e seus ajusante serão transferidos para a RNP de setores desenergizados e ligados a raiz dessa rnp.
    rnpA = configuracao[indiceConfiguracao].numeroRNP - 1;
    indiceA = 0;
    for (contador = 0; contador < numeroSetoresFalta; contador++) {
        colunaPI = retornaColunaPi(matrizPI, vetorPi, setoresFalta[contador], idConfiguracao);
        indiceP = matrizPI[setoresFalta[contador]].colunas[colunaPI].posicao;
        rnpP = matrizPI[setoresFalta[contador]].colunas[colunaPI].idRNP;
        if (rnpP != rnpA) {
            indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
            indiceR = indiceP;
            tamanhoTemporario = indiceL - indiceP + 1;
            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
            //obtém as novas rnps origem e destino
            constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
            constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);

            //###############   ARMAZENA O PONTEIRO PARA FLORESTA E LIBERA A MEMORIA DAS RNPS SUBSTITUÍDAS   ###############
            // free(configuracao[0].rnp[rnpA].nos);
            //free(configuracao[0].rnp[rnpP].nos);
            rnpOrigem.fitnessRNP = configuracao[indiceConfiguracao].rnp[rnpP].fitnessRNP;
            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
            //até esse ponto o setor em falta foi isolado do alimentador. 
            //Já foi inserido na sequencia de chaveamento a abertura de todas as chaves para isolar a falta
        }
    }

    //   imprimeIndividuo(configuracao[indiceConfiguracao]);
    contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
    chaveamento->consumidoresCorteCarga = consumidoresDesligados(configuracao[indiceConfiguracao], configuracao[indiceConfiguracao].numeroRNP - 1,
            grafoSetoresParam, setoresFalta, numeroSetoresFalta);
    // printf("consumidores em corte de carga %ld\n", chaveamento->consumidoresCorteCarga);
    chaveamento->configuracao = configuracao;
    insercaoElemFim(listaChaveamentos, chaveamento, chaveamentoAnt);
    //salva o ponteiro do chaveamento anterior
    listaAncestrais->inicio = chaveamento;
    listaAncestrais->fim = chaveamento;
    listaAncestrais->tamanho = 1;
    chaveamentoAnt = listaAncestrais->inicio;
    contadorAncestrais = listaAncestrais->tamanho;
    idConfiguracao++;
    encerra = false;
    if (chaveamentoAnt->numPares > 0) {
        while ((chaveamentoAnt != NULL) && (chaveamentoAnt->numPares > 0)) { //verifica se já encontrou um chaveamento factivel e se ainda existem pares para ser manobrados    
            //cria o primeiro novo chaveamento
            chaveamento = Malloc(ELEMENTO, 1);
            chaveamento->anterior = chaveamentoAnt;
            tamanhoAlocacao = chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab;
            chaveamento->chavesRestabFechar = Malloc(long int, chaveamentoAnt->numChavesRestab);
            chaveamento->estadoFinal = Malloc(ESTADOCHAVE, (tamanhoAlocacao)); //aplica um par e tenta fechar todas as chaves do restabelecimento
            chaveamento->idChaves = Malloc(long int, (tamanhoAlocacao));
            chaveamento->reguladorTensao = Malloc(BOOL, (tamanhoAlocacao));
            chaveamento->numChavesRestab = 0;
            chaveamento->numPares = 0;
            chaveamento->paresManobras = Malloc(int, chaveamentoAnt->numPares);
            listaAncestrais->inicio = chaveamento;
            listaAncestrais->tamanho = 0;
            while (contadorAncestrais > 0) {
                //para cada possível ancestral percorre a lista de pares de manobras disponíveis
                contadorManobras = 0;
                //printf("numero pares manobras restante %d \n",chaveamentoAnt->numPares);
                while (contadorManobras < chaveamentoAnt->numPares) {
                    chaveamento->numeroChaveamentos = chaveamentoAnt->numeroChaveamentos;
                    for (contadorAux = 0; contadorAux < chaveamentoAnt->numeroChaveamentos; contadorAux++) {
                        chaveamento->estadoFinal[contadorAux] = chaveamentoAnt->estadoFinal[contadorAux];
                        chaveamento->idChaves[contadorAux] = chaveamentoAnt->idChaves[contadorAux];
                        chaveamento->reguladorTensao[contadorAux] = chaveamentoAnt->reguladorTensao[contadorAux];
                    }
                    //aloca memória da configuração para simular a otimização da sequencia de chaveamento
                    configuracao = alocaIndividuo(numeroRnps, indiceConfiguracao, 1, numeroTrafosParam);
                    configuracao[indiceConfiguracao].idConfiguracao = idConfiguracao;
                    //copia a configuração inicial na configuração que será manipulada
                    copiaIndividuo(chaveamentoAnt->configuracao, configuracao, indiceConfiguracao, indiceConfiguracao, matrizPI);
                    //aplica um par de manobra
                    indiceChav = chaveamentoAnt->paresManobras[contadorManobras];
                    noP = nosPRA[indiceChav].p;
                    noR = nosPRA[indiceChav].r;
                    noA = nosPRA[indiceChav].a;
                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
                    //determinação do intervalo correspondente a subárvore do nó P na RNP
                    //    printf("rnpP %d indiceP %d numeroNosRnpP %d \n", rnpP, indiceP, configuracao[indiceConfiguracao].rnp[rnpP].numeroNos);
                    indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);
                    factibilidadeAnel = false;
                    //precisa testar também se a rnpA não é a rnp de setores sem fornecimento
                    if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA) && (rnpA != (numeroRnps - 1))) {
                        //verifica a factilidade do anel antes de aplicar a manobra
                        factibilidadeAnel = verificaFactibilidadeAnel(configuracao, noR, noA, rnpP, rnpA,
                                grafoSDRParam, numeroBarras, dadosAlimentadorSDRParam,
                                dadosTrafoSDRParam, indiceConfiguracao, rnpSetoresParam,
                                SBase, numeroSetores, Z, indiceRegulador,
                                dadosRegulador, maximoCorrente, numeroTrafosParam, numeroAlimentadores);
                        //aplica a manobra
                        tamanhoTemporario = indiceL - indiceP + 1;
                        if ((listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) || (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca)) {
                            //verifica a carga do trecho
                            //recupera o setor raiz do setor P
                            indiceSetorRaiz = indiceP - 1;
                            while ((indiceSetorRaiz >= 0) && (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                                indiceSetorRaiz--;
                            }
                            carga = cargaTrecho(chaveamentoAnt->configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                        }
                        if (rnpA == rnpP) {
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                            rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                            constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                            avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                    indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                            factibilidadeManobra = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao);

                        } else {
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                            alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                            rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                            rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                            //obtém as novas rnps origem e destino
                            constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                            constroiRNPOrigem(configuracao[indiceConfiguracao].rnp[rnpP], &rnpOrigem, indiceP, indiceL, matrizPI, idConfiguracao, rnpP);
                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                            avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                    indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                            factibilidadeManobra = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao);

                        }
                        if (!factibilidadeManobra) {
                            if (rnpA == rnpP) {
                                //a configuração obtida não é factível em relação as restrições relaxadas
                                //desfaz a operação aplicada
                                free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                //salva no vetor de pares de manobras. Nesse caso devido as restrições elétricas;
                                chaveamento->paresManobras[chaveamento->numPares] = indiceChav;
                                chaveamento->numPares++;
                            } else {
                                //a configuração obtida não é factível em relação as restrições relaxadas
                                //desfaz a operação aplicada
                                free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                                configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                                configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                                //salva no vetor de pares de manobras. Nesse caso devido as restrições elétricas;
                                chaveamento->paresManobras[chaveamento->numPares] = indiceChav;
                                chaveamento->numPares++;
                            }
                        } else {
                            if (factibilidadeAnel) {
                                //adiciona na lista de chaves manobradas

                                if (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho para fechamento de 720kVA
                                    if (carga > 720) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //fechamento da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);

                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                }
                                if (listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho de 120kVA para abertura
                                    if (carga > 120) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceP, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //abertura da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                }
                            } else {
                                //nao foi possivel operar em anel insere as manobras no modelo abre-fecha
                                //adiciona na lista de chaves manobradas
                                if (listaChavesParam[paresManobrasAbrir[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho de 120kVA para abertura
                                    if (carga > 120) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceP, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //abertura da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasAbrir[indiceChav], verificaPresencaRT(indiceL, rnpOrigemAnterior, rnpSetoresParam, indiceRegulador), normalmenteAberta);
                                }

                                if (listaChavesParam[paresManobrasFechar[indiceChav]].subTipoChave == chaveSeca) {
                                    //verifica a restricao de carga do trecho para fechamento de 720kVA
                                    if (carga > 720) {
                                        //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                        //será incluída a manobra de abertura e fechamento do alimentador
                                        tamanhoAlocacao += 2;
                                        chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                        chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                        chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                        idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                        //abertura do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                        //fechamento da chave
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        //fechamento do alimentador
                                        insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);

                                    } else {
                                        //a carga da chave seca não viola a restrição de manobra
                                        insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                    }
                                } else {
                                    //não é chave seca
                                    insereManobraChaveamento(chaveamento, paresManobrasFechar[indiceChav], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                }

                            }
                            //tenta restabelecer os setores sem fornecimento
                            contadorFechamento = 0;
                            while (contadorFechamento < chaveamentoAnt->numChavesRestab) {
                                noP = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].p;
                                noR = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].r;
                                noA = vetorPiChav[indiceConfiguracoes[0]].nos[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].a;
                                //verifica se o par de chaveamento continua valido
                                recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracao, matrizPI, vetorPi);
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
                                indiceL = limiteSubArvore(configuracao[indiceConfiguracao].rnp[rnpP], indiceP);

                                if (verificaOperadorRNP(indiceP, indiceR, indiceA, indiceL, rnpP, rnpR, rnpA)) {
                                    //aplica a operação de fechamento da chave para restabelecer o setor a jusante a falta
                                    tamanhoTemporario = indiceL - indiceP + 1;
                                    if (listaChavesParam[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].subTipoChave == chaveSeca) {
                                        //verifica a carga do trecho
                                        //recupera o setor raiz do setor P
                                        indiceSetorRaiz = indiceP - 1;
                                        while ((indiceSetorRaiz >= 0) && (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceSetorRaiz].profundidade != (chaveamentoAnt->configuracao[indiceConfiguracao].rnp[rnpP].nos[indiceP].profundidade - 1))) {
                                            indiceSetorRaiz--;
                                        }
                                        carga = cargaTrecho(chaveamentoAnt->configuracao[indiceConfiguracao], rnpP, indiceSetorRaiz, indiceP, indiceL, rnpSetoresParam);
                                    }
                                    if (rnpA == rnpP) {
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos, &rnpOrigem);
                                        rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                                        constroiRNPOrigemDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                                        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                                        factibilidadeRestab = verificaFactibilidade(configuracao[0], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao);
                                        if (!factibilidadeRestab) {
                                            //a configuração obtida não é factível em relação as restrições relaxadas
                                            //desfaz a operação aplicada
                                            free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                            //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                                            chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                            chaveamento->numChavesRestab++;
                                        } else {
                                            //adiciona na lista de chaves manobradas
                                            insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                        }
                                    } else {
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpP].numeroNos - tamanhoTemporario, &rnpOrigem);
                                        alocaRNP(configuracao[indiceConfiguracao].rnp[rnpA].numeroNos + tamanhoTemporario, &rnpDestino);
                                        rnpOrigemAnterior = configuracao[indiceConfiguracao].rnp[rnpP];
                                        rnpDestinoAnterior = configuracao[indiceConfiguracao].rnp[rnpA];
                                        //obtém as novas rnps origem e destino
                                        constroiRNPDestino(configuracao, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPI, indiceConfiguracao, &rnpDestino, idConfiguracao);
                                        constroiRNPOrigemRestabelecimento(configuracao, rnpP, indiceL, indiceP, matrizPI, indiceConfiguracao, &rnpOrigem, idConfiguracao);
                                        configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigem;
                                        configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestino;
                                        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                                                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
                                        factibilidadeRestab = verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao);
                                        if (!factibilidadeRestab) {
                                            //a configuração obtida não é factível em relação as restrições relaxadas
                                            //desfaz a operação aplicada
                                            free(configuracao[indiceConfiguracao].rnp[rnpP].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpP].nos = 0;
                                            free(configuracao[indiceConfiguracao].rnp[rnpA].nos);
                                            configuracao[indiceConfiguracao].rnp[rnpA].nos = 0;
                                            configuracao[indiceConfiguracao].rnp[rnpP] = rnpOrigemAnterior;
                                            configuracao[indiceConfiguracao].rnp[rnpA] = rnpDestinoAnterior;
                                            //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido as restrições elétricas;
                                            chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                            chaveamento->numChavesRestab++;
                                        } else {

                                            if ((listaChavesParam[chaveamentoAnt->chavesRestabFechar[contadorFechamento]].subTipoChave == chaveSeca) && (carga > 720)) {
                                                //nesse caso a chave viola a restrição de carga para manobra e é necessário abrir o alimentador
                                                //será incluída a manobra de abertura e fechamento do alimentador
                                                tamanhoAlocacao += 2;
                                                chaveamento->estadoFinal = (ESTADOCHAVE *) realloc(chaveamento->estadoFinal, (tamanhoAlocacao) * sizeof (ESTADOCHAVE)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                                chaveamento->idChaves = (long int *) realloc(chaveamento->idChaves, (tamanhoAlocacao) * sizeof (long int));
                                                chaveamento->reguladorTensao = (BOOL *) realloc(chaveamento->reguladorTensao, (tamanhoAlocacao) * sizeof (BOOL));
                                                idChaveAlimentador = buscaChaveDesligaTrecho(indiceA, configuracao[indiceConfiguracao].rnp[rnpA], grafoSetoresParam, listaChavesParam);
                                                //abertura do alimentador
                                                insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteAberta);
                                                //fechamento da chave
                                                insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                                //fechamento do alimentador
                                                insereManobraChaveamento(chaveamento, idChaveAlimentador, false, normalmenteFechada);
                                            } else {

                                                //adiciona na lista de chaves manobradas
                                                insereManobraChaveamento(chaveamento, vetorPiChav[indiceConfiguracoes[0]].idChaveFechada[chaveamentoAnt->chavesRestabFechar[contadorFechamento]], verificaPresencaRT(indiceA, configuracao[0].rnp[rnpA], rnpSetoresParam, indiceRegulador), normalmenteFechada);
                                            }
                                        }
                                    }
                                } else {
                                    //salva no vetor de elementos do chaveamento o id da posição que não foi possível aplicar a manobra de fechamento. Nesse caso devido a não garantir a radialidade;
                                    chaveamento->chavesRestabFechar[chaveamento->numChavesRestab] = chaveamentoAnt->chavesRestabFechar[contadorFechamento];
                                    chaveamento->numChavesRestab++;
                                }

                                contadorFechamento++;
                            }
                        }
                        if ((factibilidadeManobra || factibilidadeRestab) && ((chaveamentoAnt->numeroChaveamentos != chaveamento->numeroChaveamentos))) {
                            //insere a configuracao no chaveamento
                            contaManobrasChaveamento(configuracao, chaveamento, listaChavesParam);
                            chaveamento->consumidoresCorteCarga = consumidoresDesligados(configuracao[indiceConfiguracao], configuracao[indiceConfiguracao].numeroRNP - 1,
                                    grafoSetoresParam, setoresFalta, numeroSetoresFalta);
                            chaveamento->configuracao = configuracao;
                            //insere o chaveamento na lista de chaveamentos
                            insercaoElemFim(listaChaveamentos, chaveamento, chaveamentoAnt);
                            //atualiza o ponteiro de fim e o contador da lista de ancestrais
                            listaAncestrais->fim = chaveamento;
                            listaAncestrais->tamanho++;
                            /*como fazer esse if.... 
                                          if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao) && chaveamento->numChavesRestab == 0) {
                                              if (listaFactiveis->tamanho == 0) {
                                                  insercaoListaVazia(listaFactiveis, chaveamento);
                                              } else {
                                                  insercaoElemFim(listaFactiveis, chaveamento, chaveamentoAnt);
                                              }
                                              encerra = true;
                                          } else */
                            {
                                //        printf("idconfi %ld numeroChaveamentos %d\n", configuracao[indiceConfiguracao].idConfiguracao, chaveamento->numeroChaveamentos);
                                //cria um novo chaveamento
                                chaveamento = Malloc(ELEMENTO, 1);
                                chaveamento->anterior = NULL;
                                chaveamento->chavesRestabFechar = Malloc(long int, chaveamentoAnt->numChavesRestab);
                                chaveamento->estadoFinal = Malloc(ESTADOCHAVE, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab)); //aplica um par e tenta fechar todas as chaves do restabelecimento
                                chaveamento->idChaves = Malloc(long int, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab));
                                chaveamento->reguladorTensao = Malloc(BOOL, (chaveamentoAnt->numeroChaveamentos + 2 + chaveamentoAnt->numChavesRestab));
                                chaveamento->numChavesRestab = 0;
                                chaveamento->numPares = 0;
                                chaveamento->paresManobras = Malloc(int, chaveamentoAnt->numPares);
                                idConfiguracao++;
                            }
                        }

                    }
                    contadorManobras++;
                }
                //recupera o próximo ancestral
                chaveamentoAnt = chaveamentoAnt->proximo;
                contadorAncestrais--;
            }
            if (listaAncestrais->tamanho > 0) {
                chaveamentoAnt = listaAncestrais->inicio;
                contadorAncestrais = listaAncestrais->tamanho;
            } else {
                chaveamentoAnt = NULL;
            }
        }
        //recuperar o melhor chaveamento
        chaveamento = listaChaveamentos->inicio;
        chaveamento = chaveamento->proximo;
        contador = 1;
        while (chaveamento != NULL) {
            chaveamentoAnt = chaveamento;
            chaveamento = chaveamento->proximo;
        }
        configuracao = chaveamentoAnt->configuracao;
        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
        if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
            insercaoListaVazia(listaFactiveis, chaveamentoAnt);
        }
    } else {
        avaliaConfiguracaoChaveamento(configuracao, indiceConfiguracao, dadosTrafoSDRParam, numeroTrafosParam, numeroAlimentadoresParam,
                indiceRegulador, dadosRegulador, dadosAlimentadorSDRParam, rnpSetoresParam, Z, maximoCorrente, numeroBarras, grafoSDRParam, SBase);
        if (verificaFactibilidade(configuracao[indiceConfiguracao], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
            if (listaFactiveis->tamanho == 0) {
                insercaoListaVazia(listaFactiveis, chaveamento);
            } else {
                insercaoElemFim(listaFactiveis, chaveamento, chaveamentoAnt);
            }
        }
    }

    indiceInd = 0;
    if (listaFactiveis->tamanho > 0) {
        chaveamento = listaFactiveis->inicio;
        configuracao = chaveamento->configuracao;

        while (chaveamento != NULL) {
            if (chaveamento->numeroChaveamentos != numeroManobras)
                salvaMelhorConfiguracao(chaveamento->configuracao[indiceConfiguracao], numeroSetoresFalta, 0, chaveamento->configuracao[indiceConfiguracao].idConfiguracao, setoresFalta);
            printf("chaveamento factivel %ld: manobras %d\n", chaveamento->configuracao[indiceConfiguracao].idConfiguracao, chaveamento->numeroChaveamentos);
            configuracoesParam[indiceAnterior].objetivo.manobrasAposChaveamento = chaveamento->numeroChaveamentos;
            for (contadorAux = 0; contadorAux < chaveamento->numeroChaveamentos; contadorAux++) {
                printf("\t %s %d", listaChavesParam[chaveamento->idChaves[contadorAux]].codOperacional, chaveamento->estadoFinal[contadorAux]);
                //   printf("\t %ld", chaveamento->idChaves[contadorAux]);
            }
            printf("\n\n");
            sprintf(nomeFinal, "SAIDA_sequenciaChaveamentoFactivel%s.dad", nomeSetorFalta);
            salvaChaveamento(chaveamento, nomeFinal, listaChavesParam);
            sprintf(nomeFinal, "SAIDA_chavesCompleto%s.dad", nomeSetorFalta);
            gravaChavesCompleto(numeroChavesParam, nomeFinal, listaChavesParam);
            chaveamento = chaveamento->proximo;
        }
        //salva os individuos
        chaveamento = listaFactiveis->inicio;
        sprintf(nomeFinal, "SAIDA_Factivel_RNP_setorFalta%s.dad", nomeSetorFalta);
        gravaIndividuo(nomeFinal, chaveamento->configuracao[0]);
        sprintf(nomeFinal, "SAIDA_individuoFactivel_CaracteristicasEletricas_setorFalta%s.dad", nomeSetorFalta);
        gravaDadosEletricos(nomeFinal, chaveamento->configuracao[0]);
        FILE *arquivo;
        arquivo = fopen(nomeFinal, "a");
        fprintf(arquivo, "CONSUMIDORES CORTE CARGA %ld\n", chaveamento->consumidoresCorteCarga);
        configuracoesParam[indiceAnterior].objetivo.consumidoresDesligadosEmCorteDeCarga = chaveamento->consumidoresCorteCarga;
        fclose(arquivo);
        //recupera os individuos intermediarios
        /*       
               configuracoes = Malloc(CONFIGURACAO, 2 * totalConfiguracoes);

               while (chaveamento != NULL) {
                   //      printf("indiceInd %ld totalConfiguracoes %d \n", indiceInd, totalConfiguracoes);
                   configuracoes[indiceInd] = chaveamento->configuracao[0];
                   indiceInd++;
                   chaveamento = chaveamento->anterior;
               }
               contador = 0;
               while (indiceInd > 0) {
                   sprintf(nomeFinal, "Factivel_%d_RNP_setorFalta%s.dad", contador, nomeSetorFalta);
                   gravaIndividuo(nomeFinal, configuracoes[indiceInd - 1]);
                   sprintf(nomeFinal, "individuoFactivel_%d_CaracteristicasEletricas_setorFalta%s.dad", contador, nomeSetorFalta);
                   gravaDadosEletricos(nomeFinal, configuracoes[indiceInd - 1]);
                   indiceInd--;
                   contador++;
               }*/

    }
    desalocaMatrizPI(matrizPI, numeroSetores);
    free(matrizPI);
    free(posChavesFecharRest);
    free(valorPrioridade);
    free(paresManobrasAbrir);
    free(paresManobrasFechar);

}
/**
 * Realiza a contagem do número de consumidores desligados do sistema.
 * @param configuracaoParam
 * @param indiceRNP
 * @param grafoSetoresParam
 * @param setoresFalta
 * @param numeroSetoresFalta
 * @return 
 */
long int consumidoresDesligados(CONFIGURACAO configuracaoParam, int indiceRNP,
        GRAFOSETORES *grafoSetoresParam, long int *setoresFalta,
        int numeroSetoresFalta) {
    long int totalConsumidores = 0;
    long int indice;
    for (indice = 1; indice < configuracaoParam.rnp[indiceRNP].numeroNos; indice++) {
        totalConsumidores += grafoSetoresParam[configuracaoParam.rnp[indiceRNP].nos[indice].idNo].numeroConsumidores;
    }
    if (totalConsumidores > 0) {
//        for (indice = 0; indice < numeroSetoresFalta; indice++) {
  //          totalConsumidores -= grafoSetoresParam[setoresFalta[indice]].numeroConsumidores;
    //    }
        totalConsumidores -= configuracaoParam.objetivo.consumidoresSemFornecimento;
    }
    return totalConsumidores;
} 

/**
 * Por Leandro: durante a verificação e correção de sequência de chaveamento com chave que retornou
 * ao seu estado inicial, ao longo do PROCESSO EVOLUTIVO, faz necessário recontar o número de manobras
 * em chaves.
 * Sabe-se que as funções próprias tentarão encontrar uma sequẽncia de chaveamento sem a presença da(s) que
 * retornou(am) ao seu estado inicial. Contudo, quando tal ação não for possível para obtenção de uma sequência factível,
 * a chave que retornou ao seu estado inicial será mantida repetida, e, por consequência, esta segunda manobra deverá
 * ser contabilizada pois, uma vez que tal indivíduo seja escolhido como solução, tal sequência com chave repetida será
 * executada. Consequentemente, durante a contagem das manobras em chaves durante a execução das rotinas
 * para tratamento de sequência de chaveamento, não poderá ser decrementado o contador de manobras para chaves
 * que retornaram ao seu estado inicial.
 * Este método realização tal ação de contar manobras sem decrementar o contador na ocorrência de chaves repetidas.
 * Contudo, na ocorrência de manobras repetidas, mesmo não decrementando o contador, este método, assim como a
 * função "numeroManobrasModificada()", determina o caso de manobra e outros parâmentros necessário a atualização
 * posterior do vetor Pi.
 *
 * Em outras palavras, consiste na função "numeroManobrasModificada()" modificada para não decrementar o contador de manobras.
 *
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
void numeroManobrasChaveamento(CONFIGURACAO *configuracoesParam, long int idChaveAberta, long int idChaveFechada,
		LISTACHAVES *listaChavesParam, int idConfiguracaoParam, ESTADOCHAVE *estadoInicialChaves,
		BOOL *estadoInicialCA, BOOL *estadoInicialCF, int idNovaConfiguracaoParam, int *casoManobra) {
	int contaMudancaChave = 0;
    int seca = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca;
    int comCargaManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual;
    int comCargaAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaAutomatica;
    int curtoManual = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual;
    int curtoAutomatica = configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoAutomatica;

    int manobrasAutomatica, manobrasManual;
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
		}
		else {
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
		//Para definir o caso de manobra
        if (estadoInicialChaves[idChaveAberta] == normalmenteAberta) {
            estadoInicialCA[0] = true;
            contaMudancaChave--;
        }
        else{
            estadoInicialCA[0] = false;
            contaMudancaChave++;
        }
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
        }
        else {
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
        //Para definir o caso de manobra
        if (estadoInicialChaves[idChaveFechada] == normalmenteFechada) {
            estadoInicialCF[0] = true;
            contaMudancaChave--;
        }
        else{
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

/* Por Leandro:
 * Descrição:  esta função compara a sequência de chaveamento que possui chaves que retornaram ao seu estado inicial com a sequência de chaveamento
 * resultante da aplicação da função destinada a excluir tais chaves repedtidas, a fim de determinar se as mesmas foram excluídas. Em outras palavras,
 * determina se a nova sequência de chaveamento é igual a sequência original, com chaves repetidas.
 *
 */

BOOL verificaRemocaoChaves(VETORPI *vetorPiParam, long int idNovaConfiguracaoParam, VETORPI *vetorPiOtimizado, long int idConfiguracaoOtimizadaParam){
    long int *idConfiguracoesInterOriginal, *idConfiguracoesInterOtimizado, idConfiguracao1, idConfiguracao2;
    int totalConfiguracoesInterOriginal, totalConfiguracoesInterOtimizado,indice, indice2;
    BOOL sequenciaIgual;

    //Recupera a relação de ancestralidade Original e aquela obtida após a Tentativa de remoção das chaves repetidas
    idConfiguracoesInterOriginal = recuperaConfiguracoesIntermediarias(vetorPiParam, idNovaConfiguracaoParam, &totalConfiguracoesInterOriginal);
    idConfiguracoesInterOtimizado = recuperaConfiguracoesIntermediarias(vetorPiOtimizado, idConfiguracaoOtimizadaParam, &totalConfiguracoesInterOtimizado);

    sequenciaIgual = true;
    if(totalConfiguracoesInterOriginal != totalConfiguracoesInterOtimizado) //Compara o número de config. intermediárias da sequência original com o da sequência após a tentativa de remoção de chaves repetidas
        sequenciaIgual = false;
    else{ //Senão, compara as sequência de chaveamento anterior e posterior a tentativa de remoção das chaves repetidas
		indice = 0;
		while(sequenciaIgual && indice < totalConfiguracoesInterOriginal && indice < totalConfiguracoesInterOtimizado){
			idConfiguracao1 = idConfiguracoesInterOriginal[indice];
			idConfiguracao2 = idConfiguracoesInterOtimizado[indice];

			if(vetorPiParam[idConfiguracao1].numeroManobras == vetorPiOtimizado[idConfiguracao2].numeroManobras){
				indice2 = 0;
				while( (indice2 < vetorPiParam[idConfiguracao1].numeroManobras) && sequenciaIgual){
					if( (vetorPiParam[idConfiguracao1].idChaveAberta[indice2] != vetorPiOtimizado[idConfiguracao2].idChaveAberta[indice2]) || (vetorPiParam[idConfiguracao1].idChaveFechada[indice2] != vetorPiOtimizado[idConfiguracao2].idChaveFechada[indice2]) )
						sequenciaIgual = false;
					indice2++;
				}
			}
			else
				sequenciaIgual = false;

			indice++;
		}
	}

    free(idConfiguracoesInterOriginal);
    idConfiguracoesInterOriginal = NULL;
	free(idConfiguracoesInterOtimizado);
	idConfiguracoesInterOtimizado = NULL;
    if(sequenciaIgual) //Se as sequência são iguais, então não houve remoção de chaves repetidas
    	return false;
    else
    	return true;
}

/* Por Leandro: está função determina o índice das Chaves a serem abertas e fechadas usando, em vez do vetorPi,
 * as informações salvas em "configuracaoParam" e "grafoSetoresParam".
 *
 * Dados os índices dos Nós P, R e A, busca o nó mais próximo do nó P à montante e; com as informações nó P e do seu adjacente à montante, determina,
 * em "grafoSetoresParam", o índice da chave entre estes dois nós; dados os nós R e A, determina, em "grafoSetoresParam", o índice da chave entre estes dois nós.
 * O primeiro será o índice da chave a ser aberta e o segundo é o índice da chave a ser fechada
 *
 *@param idChaveAbertaParam - salvará o índice da chave a ser aberta
 *@param idChaveFechadaParam - salvará o índice da cahves a ser fechada
 *@param indicePParam - índice do nó P
 *@param indiceRParam - índice do nó R
 *@param indiceAParam - índice do nó A
 *@param rnpPParam - índice da rnp na qual encontra-se o nó P
 *@param rnpRParam - índice da rnp na qual encontra-se o nó R
 *@param rnpAParam - índice da rnp na qual encontra-se o nó A
 *@param configuracaoParam -
 *@param idConfiguracaoParam - é o índice do indivíuo no qual os nós PRA serão pesquisados
 *@param grafoSetoresParam
 */
void determinaChaves(long int *idChaveAbertaParam, long int *idChaveFechadaParam, int indicePParam, int indiceRParam, int indiceAParam,
		int rnpPParam, int rnpRParam, int rnpAParam, CONFIGURACAO *configuracaoParam, long int idConfiguracaoParam, GRAFOSETORES *grafoSetoresParam){
	long int noP, noR, noA, noRaizP;
	int indice;

    if (rnpPParam != rnpRParam || rnpAParam >= configuracaoParam[idConfiguracaoParam].numeroRNP) {
    	idChaveAbertaParam[0] = -999;
		idChaveFechadaParam[0] = -999;
    }
    else{
    	if(rnpPParam >= configuracaoParam[idConfiguracaoParam].numeroRNP && rnpAParam < configuracaoParam[idConfiguracaoParam].numeroRNP){
    		rnpPParam = rnpPParam - configuracaoParam[idConfiguracaoParam].numeroRNP;
    		if(indicePParam > 1){
    			indice = indicePParam-1;
    			while(indice>0 && configuracaoParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indice].profundidade >= configuracaoParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indicePParam].profundidade)
    				indice--;
    			noRaizP = configuracaoParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indice].idNo;
				noP = configuracaoParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indicePParam].idNo;

    			idChaveAbertaParam[0] = -999;
    			for(indice = 0; indice< grafoSetoresParam[noP].numeroAdjacentes && idChaveAbertaParam[0] == -999; indice++){
    				if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
    					idChaveAbertaParam[0] = grafoSetoresParam[noP].idChavesAdjacentes[indice];
    			}

    		}
    		else
    			idChaveAbertaParam[0] = -1; //Se indiceP for igual a 1, então a chave que teria que ser aberta seria aquela que conecta o nóP ao nó raiza da RNP Fictícia. Contudo, tanto este nó, quanto esta suposta chave são fictícios, isto é, apenas artifícios de programação.

			noR = configuracaoParam[idConfiguracaoParam].rnpFicticia[rnpPParam].nos[indiceRParam].idNo;
			noA = configuracaoParam[idConfiguracaoParam].rnp[rnpAParam].nos[indiceAParam].idNo;
    		idChaveFechadaParam[0] = -999;
    		for(indice = 0; indice< grafoSetoresParam[noR].numeroAdjacentes && idChaveFechadaParam[0] == -999; indice++){
				if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
					idChaveFechadaParam[0] = grafoSetoresParam[noR].idChavesAdjacentes[indice];
    		}
    	}
    	else{
    		if(rnpPParam < configuracaoParam[idConfiguracaoParam].numeroRNP && rnpAParam < configuracaoParam[idConfiguracaoParam].numeroRNP){
    			indice = indicePParam-1;
				while(indice>0 && configuracaoParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice].profundidade >= configuracaoParam[idConfiguracaoParam].rnp[rnpPParam].nos[indicePParam].profundidade)
					indice--;
				noRaizP = configuracaoParam[idConfiguracaoParam].rnp[rnpPParam].nos[indice].idNo;
				noP = configuracaoParam[idConfiguracaoParam].rnp[rnpPParam].nos[indicePParam].idNo;
				noR = configuracaoParam[idConfiguracaoParam].rnp[rnpPParam].nos[indiceRParam].idNo;
				noA = configuracaoParam[idConfiguracaoParam].rnp[rnpAParam].nos[indiceAParam].idNo;

				idChaveAbertaParam[0] = -999;
				for(indice = 0; indice < grafoSetoresParam[noP].numeroAdjacentes && idChaveAbertaParam[0] == -999; indice++){
					if(grafoSetoresParam[noP].setoresAdjacentes[indice] == noRaizP)
						idChaveAbertaParam[0] = grafoSetoresParam[noP].idChavesAdjacentes[indice];
				}

				idChaveFechadaParam[0] = -999;
				for(indice = 0; indice < grafoSetoresParam[noR].numeroAdjacentes && idChaveFechadaParam[0] == -999; indice++){
						if(grafoSetoresParam[noR].setoresAdjacentes[indice] == noA)
							idChaveFechadaParam[0] = grafoSetoresParam[noR].idChavesAdjacentes[indice];
				}
    		}
    	}
    }
}



/**
 * Por Leandro: consiste na função "simulaChaveamentoPosicaoInicial()" modificada para:
 * a) considerar a possibilidade de que a configuração análisada tenha sido gerada pelo LRO, o qual faz poda em rnp Fictícia e
 * transferência para RNPs Real.
 *
 * b) PARA IMPEDIR SEQUÊNCIAS QUE LEVEM AO DESLIGAMENTO DE SETORES SAUDÁVEIS NÃO AFETADOS PELA FALTA. Em outras palavras, para fazer com que sejam
 * impedida ações que levem a transferência de setores para uma RNP fictícia. Para tal, optou-se classificar esta sequência como sendo infactível.
 * Assim, poder-se-á testar o par rearranjado numa outra posição ou obter uma sequência com chave repetida mas sem desligamento de setores
 *
 * c) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
 *
 * d) para corrigir uma falha no "if" que verifica se o Nó R está presente na subárvore transferida. Para tal, mudou-se o operador lógido,
 * tirando o "&&" (operador E) e colocando em seu lugar o "||" (operador OU)
 *
 *
 * Simula a inserção do novo par de manobra na primeira posição executando os pares de manobras posteriores.
 * @param indiceConfiguracoes
 * @param contador
 * @param vetorPiChav
 * @param matrizPIChav
 * @param idProxConfiguracao
 * @param configuracoesChav
 * @param noPNovo
 * @param noANovo
 * @param noRNovo
 * @param posicaoChave
 * @param posicaoConfiguracao
 * @param idConfiguracaoAncestral
 * @param idChaveAbertaR
 * @param idChaveFechadaR
 * @param idNovoAncestral
 * @param idConfiguracaoIni
 * @return
 */
BOOL simulaChaveamentoPosicaoInicialModificada(int *indiceConfiguracoes, int contador, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav,
        int *idProxConfiguracao, CONFIGURACAO *configuracoesChav, long int noPNovo, long int noANovo, long int noRNovo,
        int posicaoChave, int posicaoConfiguracao, int idConfiguracaoAncestral, long int idChaveAbertaR,
        long int idChaveFechadaR, int *idNovoAncestral, int idConfiguracaoIni, GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParamInicial) {
    BOOL chaveamentoCorreto = true;
    int configuracao;
    int contadorChav;
    long int noP, noR, noA;
    int indiceA, indiceR, indiceP, indiceL, indice1;
    int rnpP, rnpR, rnpA;
    int idNovaConfiguracao = idProxConfiguracao[0];
    long int idChaveAberta, idChaveFechada, idChaveAbertaPRA, idChaveFechadaPRA;

    while (contador > 0 && chaveamentoCorreto) {
        configuracao = indiceConfiguracoes[contador];
        contadorChav = 0;

        if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
        {
            noP = noPNovo;
            noR = noRNovo;
            noA = noANovo;
            idChaveAberta = idChaveAbertaR;
            idChaveFechada = idChaveFechadaR;
        } else {
            //aplica a tripla de nos anterior
            noP = vetorPiChav[configuracao].nos[contadorChav].p;
            noR = vetorPiChav[configuracao].nos[contadorChav].r;
            noA = vetorPiChav[configuracao].nos[contadorChav].a;
            idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
            idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];
        }
        //verifica se o par de chaveamento continua valido
        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
        // Por Leandro: Modificado aqui para
        // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
        // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
        determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

        if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
            chaveamentoCorreto = false;
        } else {
            //determinação do intervalo correspondente a subárvore do nó P na RNP
//            indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
        	if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
        		indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
        	else{
        		if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
        			indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
        	}

            //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
            if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                chaveamentoCorreto = false;
            } else {
                if (rnpA == rnpP) {
                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                    if (indiceA >= indiceP && indiceA <= indiceL) {
                        chaveamentoCorreto = false;
                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                    {
                        //realiza a alocação da RNP que será alterada
                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);

                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                            if (indice1 != rnpP) {
                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                            }
                        }
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) //Por Leandro: copia também as RNPs Fictícias
                        	configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                        configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;
                    }
                } else //aplica o chaveamento e gera uma nova configuracao simulada
                {
					obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                }
                copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                {
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                    vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                }
                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                idConfiguracaoAncestral = idNovaConfiguracao;
                idNovaConfiguracao++;
            }
        }
        contador--;
    }
    if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
    {
        if (idProxConfiguracao[0] == idNovaConfiguracao)
            idNovoAncestral[0] = vetorPiChav[idConfiguracaoIni].idAncestral;
        else
            idNovoAncestral[0] = idNovaConfiguracao - 1;
    }
    idProxConfiguracao[0] = idNovaConfiguracao;
    return chaveamentoCorreto;
}

/**
 * Por Leandro: consiste na função "simulaChaveamentoPosicaoFinal()" modificada para:
 * a) considerar a possibilidade de que a configuração análisada tenha sido gerada pelo LRO, o qual faz poda em rnp Fictícia e
 * transferência para RNPs Real
 *
 * b) PARA IMPEDIR SEQUÊNCIAS QUE LEVEM AO DESLIGAMENTO DE SETORES SAUDÁVEIS NÃO AFETADOS PELA FALTA. Em outras palavras, para fazer com que sejam
 * impedida ações que levem a transferência de setores para uma RNP fictícia. Para tal, optou-se classificar esta sequência como sendo infactível.
 * Assim, poder-se-á testar o par rearranjado numa outra posição ou obter uma sequência com chave repetida mas sem desligamento de setores
 *
 * c) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
 *
 * d) para corrigir uma falha no "if" que verifica se o Nó R está presente na subárvore transferida. Para tal, mudou-se o operador lógido,
 * tirando o "&&" (operador E) e colocando em seu lugar o "||" (operador OU)
 *
 * Simula a insercão do novo par de manobra na segunda posição possível, ou seja no final da cadeia. Para tal aplica os pares de manobras intermediários a partir do ancestral da posição 1.
 * @param indiceConfiguracoes
 * @param contador
 * @param vetorPiChav
 * @param matrizPIChav
 * @param idProxConfiguracao
 * @param configuracoesChav
 * @param noPNovo
 * @param noANovo
 * @param noRNovo
 * @param posicaoChave
 * @param posicaoConfiguracao
 * @param idConfiguracaoAncestral
 * @param idChaveAbertaR
 * @param idChaveFechadaR
 * @param idNovoAncestral
 * @param idConfiguracaoIni
 * @return
 */
BOOL simulaChaveamentoPosicaoFinalModificada(int *indiceConfiguracoes, int contador, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav,
        int *idProxConfiguracao, CONFIGURACAO *configuracoesChav, long int noPNovo, long int noANovo, long int noRNovo,
        int posicaoChave, int posicaoConfiguracao, int idConfiguracaoAncestral, long int idChaveAbertaR,
        long int idChaveFechadaR, int *idNovoAncestral, int idConfiguracaoIni, GRAFOSETORES *grafoSetoresParam) {
    BOOL chaveamentoCorreto = true;
    int configuracao;
    int contadorChav;
    long int noP, noR, noA, idChaveAberta, idChaveFechada, idChaveAbertaPRA, idChaveFechadaPRA;
    int indiceA, indiceR, indiceP, indiceL, indice1;
    int rnpP, rnpR, rnpA;
    int idNovaConfiguracao = idProxConfiguracao[0];

    while (contador >= 0 && chaveamentoCorreto) {
        configuracao = indiceConfiguracoes[contador];
        contadorChav = 0;
        if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
        {
            noP = noPNovo;
            noR = noRNovo;
            noA = noANovo;
            idChaveAberta = idChaveAbertaR;
            idChaveFechada = idChaveFechadaR;
        } else //aplica a tripla de nos anterior
        {
            noP = vetorPiChav[configuracao].nos[contadorChav].p;
            noR = vetorPiChav[configuracao].nos[contadorChav].r;
            noA = vetorPiChav[configuracao].nos[contadorChav].a;
            idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
            idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];
        }
        //verifica se o par de chaveamento continua valido
        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
        // Por Leandro: Modificado aqui para
        // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
        // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste

        determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

        if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
            chaveamentoCorreto = false;
        } else {
            //determinação do intervalo correspondente a subárvore do nó P na RNP
//            indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
        	if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)
        		indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
        	else{
        		if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)
        			indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
        	}
			if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                chaveamentoCorreto = false;
            } else {
                if (rnpA == rnpP) {
                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                    if (indiceA >= indiceP && indiceA <= indiceL) {
                        chaveamentoCorreto = false;
                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                    {
                        //realiza a alocação da RNP que será alterada
                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                            if (indice1 != rnpP) {
                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                            }
                        }
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) //Por Leandro: copia também as RNPs Fictícias
                        	configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                        configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;

                    }
                } else //aplica o chaveamento e gera uma nova configuracao simulada
                {
                    //calcula o tamanho da subárvore podada
                    obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                }
                copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                if (configuracao == posicaoConfiguracao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                {
                    //insere dados no vetorPi
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                    vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                }
                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                idConfiguracaoAncestral = idNovaConfiguracao;
                idNovaConfiguracao++;
            }
        }
        contador--;
    }
    if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
    {
        if (idProxConfiguracao[0] == idNovaConfiguracao) {
            idNovoAncestral[0] = vetorPiChav[idConfiguracaoIni].idAncestral;
            // printf("idNovoAncestral %d \n", idNovoAncestral[0]);
        } else
            idNovoAncestral[0] = idNovaConfiguracao - 1;
    }
    idProxConfiguracao[0] = idNovaConfiguracao;
    return chaveamentoCorreto;
}

/**
 * Por Leandro: consiste na função "removeChaveEstadoInicialCaso2()" modificada para:
 * a) considerar a possibilidade de que a configuração análisada tenha sido gerada pelo LRO, o qual faz poda em rnp Fictícia e
 * transferência para RNPs Real.
 *
 * b) passar novos parâmentros, necessários à execução de funções que foram modificadas e que são chamadas aqui.
 *
 * c) PARA IMPEDIR SEQUÊNCIAS QUE LEVEM AO DESLIGAMENTO DE SETORES SAUDÁVEIS NÃO AFETADOS PELA FALTA. Em outras palavras, para fazer com que sejam
 * impedida ações que levem a transferência de setores para uma RNP fictícia. Para tal, optou-se classificar esta sequência como sendo infactível.
 * Assim, poder-se-á testar o par rearranjado numa outra posição ou obter uma sequência com chave repetida mas sem desligamento de setores
 *
 * d)  verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
 *
 * Esse método realiza a remoção da chave que retornou ao estado inicial para o caso 2. Nesse caso as duas chaves da manobra retornaram ao estado inicial.
 * A seguir busca-se a posição em que cada uma das chaves retornou ao seu estado inicial.
 * Se as duas chaves foram anteriormente manobradas no mesmo par, simplemente remove-se as manobras.
 * Caso elas tenham sido manobradas em ancestrais diferentes é obtido um novo par de manobra combinando as chaves diferentes das duas configurações.
 * Obtido o novo par verifica em que posição o novo par será inserido, existem duas posições possíveis. No ponto onde a chave aberta foi alterada ou no lugar em que a chave fechada foi alterada.
 * Para definir essa posição simula toda a sequência de manobras à partir do ancestral para cada uma das inserções.
 * Se todos as configurações intermediárias mantiverem a característica radial, a nova sequência é informada.
 * @param idConfiguracaoParam
 * @param vetorPiChav
 * @param matrizPIChav
 * @param configuracoesChav
 * @param idProxConfiguracao
 * @param totalConfiguracoes
 * @param idNovoAncestral
 * @return
 */
BOOL removeChaveEstadoInicialCaso2Modificada(int idConfiguracaoParam, VETORPI *vetorPiChav, MATRIZPI *matrizPIChav, CONFIGURACAO *configuracoesChav, int *idProxConfiguracao,
		int totalConfiguracoes, int *idNovoAncestral, int numeroTrafosParam, GRAFOSETORES *grafoSetoresParam) {
    /*As variáveis xxIni serão utilizadas para armazenar os dados da configuração onde ocorreu a primeira mudança na chave
     As variáveis xxFinal contém os dados na configuração onde a chave retornou ao seu estado inicial.*/
    int contador, contadorChav, novaPosicao, contadorConfiguracoes, indice1;
    BOOL encontradoCA = false;
    BOOL encontradoCF = false;
    int idConfiguracaoIniCA = -1, posIniCA = -1, idConfiguracaoIniCF = -1, posIniCF = -1, idConfiguracaoAncestral;
    long int noA, noP, noR, noPIniA, noRIniA, noAIniA, noPIniF, noRIniF, noAIniF, noPNovo, noRNovo, noANovo;
    long int idChaveAbertaR, idChaveFechadaR; //armazena o id das chaves que serao rearranjadas
    BOOL chaveamentoCorreto = true;
    int *indiceConfiguracoes;
    indiceConfiguracoes = Malloc(int, totalConfiguracoes);
    int configuracao, rnpA, rnpP, rnpR, indiceL, indiceA, indiceP, indiceR;
    long int idChaveAberta, idChaveFechada, idChaveAbertaPRA, idChaveFechadaPRA;

    //localiza no vetorPi a alteração anterior da chave
    contador = vetorPiChav[idConfiguracaoParam].idAncestral;
    contadorConfiguracoes = 0;
    indiceConfiguracoes[contadorConfiguracoes] = idConfiguracaoParam;
    int idNovaConfiguracao = idProxConfiguracao[0];
    while (contador > 0 && (!encontradoCA || !encontradoCF)) {
        contadorChav = 0;
        while (contadorChav < vetorPiChav[contador].numeroManobras && (!encontradoCA || !encontradoCF)) {
            if (vetorPiChav[contador].idChaveFechada[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveAberta[0]) {
                encontradoCA = true;
                idConfiguracaoIniCA = contador;
                posIniCA = contadorChav;
            }
            if (vetorPiChav[contador].idChaveAberta[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveFechada[0]) {
                encontradoCF = true;
                idConfiguracaoIniCF = contador;
                posIniCF = contadorChav;
            }
            contadorChav++;
        }
        //salva os identificadores das configuracoes entre a repetição de chaves
        contadorConfiguracoes++;
        indiceConfiguracoes[contadorConfiguracoes] = contador;
        contador = vetorPiChav[contador].idAncestral;
    }
    //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
    idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
    //nesse caso as chaves se repetem na mesma configuração e deve ser simulado somente as configurações intermediárias entre a repetição
    if ((idConfiguracaoIniCA == idConfiguracaoIniCF) && (posIniCA == posIniCF)) {
        contador = contadorConfiguracoes - 1;
        while (contador > 0 && chaveamentoCorreto) {
            configuracao = indiceConfiguracoes[contador];
            contadorChav = 0;
            copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
            noP = vetorPiChav[configuracao].nos[contadorChav].p;
            noR = vetorPiChav[configuracao].nos[contadorChav].r;
            noA = vetorPiChav[configuracao].nos[contadorChav].a;
            idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
            idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];

            //verifica se o par de chaveamento continua valido
            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
            // Por Leandro: Modificado aqui para
            // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
            // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
            determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

            if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                chaveamentoCorreto = false;
            } else {
                //determinação do intervalo correspondente a subárvore do nó P na RNP
//                indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
                	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
        		else{
        			if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
        				indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
        		}

                if (rnpA == rnpP) {
                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                    if (indiceA >= indiceP && indiceA <= indiceL) {
                        chaveamentoCorreto = false;
                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                    {
                        //realiza a alocação da RNP que será alterada
                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                            if (indice1 != rnpP) {
                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                            }
                        }
                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) //Por Leandro: Copia também as RNPs Fictícias
                        	configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                        configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;
                    }
                } else //aplica o chaveamento e gera uma nova configuracao simulada
                {
                    obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                }
                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                idConfiguracaoAncestral = idNovaConfiguracao;
                idNovaConfiguracao++;
            }
            contador--;
        }
        if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
        {
            if (idProxConfiguracao[0] == idNovaConfiguracao)
                idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
            else
                idNovoAncestral[0] = idNovaConfiguracao - 1;
        }
        idProxConfiguracao[0] = idNovaConfiguracao;

    } else {
        //nesse caso as duas chaves que retornaram ao estado inicial estão em configurações anteriores diferentes.
        //O novo chaveamento será composto pela chave que abre na configuração que repete a chave que abriu e pela que chave que fecha na configuração que repete a chave que fechou.
        //recupera os nós P, R, A da primeira mudança na chave aberta, nesse ponto a chave foi fechada. A chave que abriu nesse par será utilizada no par rearranjado
        noPIniA = vetorPiChav[idConfiguracaoIniCA].nos[posIniCA].p;
        noRIniA = vetorPiChav[idConfiguracaoIniCA].nos[posIniCA].r;
        noAIniA = vetorPiChav[idConfiguracaoIniCA].nos[posIniCA].a;
        //printf("1 alteracao noP %ld noR %ld noA %ld \n", noPIni, noRIni, noAIni);
        //imprimeIndividuo(configuracoesChav[idConfiguracaoIni]);
        //recupera os nós P, R, A da primeira mudança na chave fechada, nesse ponto a chave foi aberta. A chave que fechou nesse par será utilizada no par rearranjado
        noPIniF = vetorPiChav[idConfiguracaoIniCF].nos[posIniCF].p;
        noRIniF = vetorPiChav[idConfiguracaoIniCF].nos[posIniCF].r;
        noAIniF = vetorPiChav[idConfiguracaoIniCF].nos[posIniCF].a;

        //definição do novo par de chaveamento. Esse par será composto pelas chaves diferentes presentes nos chaveamentos que contém as chaves que retornaram ao estado inicial.
        //o nó noP tem origem na chave que será aberta no par de chaveamento.
        //os nós nR e noA tem origem na chave que será fechada no par de chaveamento
        noPNovo = noPIniA;
        noRNovo = noRIniF;
        noANovo = noAIniF;
        idChaveAbertaR = vetorPiChav[idConfiguracaoIniCA].idChaveAberta[posIniCA];
        idChaveFechadaR = vetorPiChav[idConfiguracaoIniCF].idChaveFechada[posIniCF];

        //verifica se o par rearranjado é composto por chaves diferentes
        if (idChaveAbertaR != idChaveFechadaR) {
            //O par de chaveamento rearranjado pode ser colocado em 3 posições.
            if (vetorPiChav[idConfiguracaoIniCA].numeroManobras<=1) {
                //1)No local que a chave que fechou teve sua primeira alteração de estado.
                novaPosicao = idConfiguracaoIniCF;
                contador = contadorConfiguracoes;
                //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                while (contador > 0 && chaveamentoCorreto) {
                    configuracao = indiceConfiguracoes[contador];
                    contadorChav = 0;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    if (configuracao != idConfiguracaoIniCA) { //é necessário pular a configuração que teve a primeira alteração da chave para chave aberta
                        if (configuracao == novaPosicao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                        {
                            noP = noPNovo;
                            noR = noRNovo;
                            noA = noANovo;
                            idChaveAberta = idChaveAbertaR;
                            idChaveFechada = idChaveFechadaR;
                            //insere dados no vetorPi
                            if (contadorChav == posIniCF) {
                                vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                            }

                        } else //aplica a tripla de nos anterior
                        {
                            noP = vetorPiChav[configuracao].nos[contadorChav].p;
                            noR = vetorPiChav[configuracao].nos[contadorChav].r;
                            noA = vetorPiChav[configuracao].nos[contadorChav].a;
                            idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
                            idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];
                        }

                        //  printf("nop %ld, noA %ld noR %ld \n",noP, noA, noR);
                        //verifica se o par de chaveamento continua valido
                        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                        // Por Leandro: Modificado aqui para
                        // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                        // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                        determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

                        if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                            chaveamentoCorreto = false;
                        } else {
                            //determinação do intervalo correspondente a subárvore do nó P na RNP
//                        	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                            if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
                            	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
                    		else{
                    			if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
                    				indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                    		}

                            //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                            if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                                chaveamentoCorreto = false;
                            } else {
                                if (rnpA == rnpP) {
                                    //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                    if (indiceA >= indiceP && indiceA <= indiceL) {
                                        chaveamentoCorreto = false;
                                    } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                    {
                                        //realiza a alocação da RNP que será alterada
                                        alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                        constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                            if (indice1 != rnpP) {
                                                configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                            }
                                        }
                                        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) { //Por Leandro: copia também as RNPs Ficticias
                                                configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                                        }
                                        configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;

                                    }
                                } else //aplica o chaveamento e gera uma nova configuracao simulada
                                {
                                    obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                            indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav); //Por Leandro
                                }
                                vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                idConfiguracaoAncestral = idNovaConfiguracao;
                                idNovaConfiguracao++;
                            }
                        }
                    }
                    contador--;
                }
                //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                {
                    if (idProxConfiguracao[0] == idNovaConfiguracao) {
                        idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                    } else {
                        idNovoAncestral[0] = idNovaConfiguracao - 1;
                    }
                    idProxConfiguracao[0] = idNovaConfiguracao;
                } else {

                    //2)No local que a chave que abriu teve sua primeira alteração de estado
                    novaPosicao = idConfiguracaoIniCA;
                    contador = contadorConfiguracoes;
                    //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                    idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                    while (contador > 0 && chaveamentoCorreto) {
                        configuracao = indiceConfiguracoes[contador];
                        contadorChav = 0;
                        if (configuracao != idConfiguracaoIniCF) {
                            copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                            if (configuracao == novaPosicao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                            {
                                noP = noPNovo;
                                noR = noRNovo;
                                noA = noANovo;
                                idChaveAberta = idChaveAbertaR;
                                idChaveFechada = idChaveFechadaR;
                                //insere dados no vetorPi
                                if (contadorChav == posIniCA) {
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                                }

                            } else //aplica a tripla de nos anterior
                            {
                                noP = vetorPiChav[configuracao].nos[contadorChav].p;
                                noR = vetorPiChav[configuracao].nos[contadorChav].r;
                                noA = vetorPiChav[configuracao].nos[contadorChav].a;
                                idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
                                idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];
                            }
                            //verifica se o par de chaveamento continua valido
                            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                            // Por Leandro: Modificado aqui para
                            // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                            // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                            determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

                            if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                                chaveamentoCorreto = false;
                            } else {
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
//                            	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                                if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
                                	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
                        		else{
                        			if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
                        				indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                        		}

                                //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                                if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                                    chaveamentoCorreto = false;
                                } else {
                                    if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                                        chaveamentoCorreto = false;
                                    } else {
                                        if (rnpA == rnpP) {
                                            //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                            if (indiceA >= indiceP && indiceA <= indiceL) {
                                                chaveamentoCorreto = false;
                                            } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                            {
                                                //realiza a alocação da RNP que será alterada
                                                alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                                constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                                    if (indice1 != rnpP) {
                                                        configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                                    }
                                                }
                                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) //Por Leandro
                                                        configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                                                configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;
                                            }
                                        } else //aplica o chaveamento e gera uma nova configuracao simulada
                                        {
                                            obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                                    indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                                        }
                                        vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                        idConfiguracaoAncestral = idNovaConfiguracao;
                                        idNovaConfiguracao++;
                                    }
                                }
                            }
                        }
                        contador--;
                    }
                    //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                    if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                    {
                        if (idProxConfiguracao[0] == idNovaConfiguracao) {
                            idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                        } else {
                            idNovoAncestral[0] = idNovaConfiguracao - 1;
                        }
                        idProxConfiguracao[0] = idNovaConfiguracao;
                    } else {

                        //3)No local onde houve a repetição das duas chaves
                        novaPosicao = idConfiguracaoParam;
                        contador = contadorConfiguracoes;
                        //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                        idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                        while (contador >= 0 && chaveamentoCorreto) {
                            configuracao = indiceConfiguracoes[contador];
                            contadorChav = 0;
                            if (configuracao != idConfiguracaoIniCF && configuracao != idConfiguracaoIniCA) {
                                copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);

                                if (configuracao == novaPosicao) //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                                {
                                    noP = noPNovo;
                                    noR = noRNovo;
                                    noA = noANovo;
                                    idChaveAberta = idChaveAbertaR;
                                    idChaveFechada = idChaveFechadaR;
                                    //insere dados no vetorPi
                                    if (contadorChav == posIniCA) {
                                        vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                        vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                        vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                        vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                        vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                        vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                        vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                                    }

                                } else //aplica a tripla de nos anterior
                                {

                                    noP = vetorPiChav[configuracao].nos[contadorChav].p;
                                    noR = vetorPiChav[configuracao].nos[contadorChav].r;
                                    noA = vetorPiChav[configuracao].nos[contadorChav].a;
                                    idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
                                    idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];

                                }
                                //verifica se o par de chaveamento continua valido
                                recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                                // Por Leandro: Modificado aqui para
                                // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                                // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                                determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

                                if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                                    chaveamentoCorreto = false;
                                } else {
                                    //determinação do intervalo correspondente a subárvore do nó P na RNP
//                                	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                                    if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
                                    	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
                            		else{
                            			if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
                            				indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                            		}

                                    //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                                    if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                                        chaveamentoCorreto = false;
                                    } else {
                                        if (rnpA == rnpP) {
                                            //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                            if (indiceA >= indiceP && indiceA <= indiceL) {
                                                chaveamentoCorreto = false;
                                            } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                            {
                                                //realiza a alocação da RNP que será alterada
                                                alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                                constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                                    if (indice1 != rnpP) {
                                                        configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                                    }
                                                }
                                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) { //Por Leandro
                                                        configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                                                }
                                                configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;
                                            }
                                        } else //aplica o chaveamento e gera uma nova configuracao simulada
                                        {
                                            obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                                    indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                                        }
                                        vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                        idConfiguracaoAncestral = idNovaConfiguracao;
                                        idNovaConfiguracao++;
                                    }
                                }
                            }
                            contador--;
                        }
                        //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                        if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                        {
                            if (idProxConfiguracao[0] == idNovaConfiguracao) {
                                idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                            } else {
                                idNovoAncestral[0] = idNovaConfiguracao - 1;
                            }
                        }
                        idProxConfiguracao[0] = idNovaConfiguracao;
                    }

                } //else da falha na primeira tentativa de chaveamento
            } else {
                //o novo par tem que ser inserido no processo de restabelecimento,
                //pois consiste na troca da chave de restabelecimento
                novaPosicao = idConfiguracaoIniCA;
                contador = contadorConfiguracoes;
                //recupera o identificador da configuração ancestral para iniciar a simulação do chaveamento.
                idConfiguracaoAncestral = vetorPiChav[indiceConfiguracoes[contadorConfiguracoes]].idAncestral;
                while (contador > 0 && chaveamentoCorreto) {
                    configuracao = indiceConfiguracoes[contador];
                    contadorChav = 0;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    if (configuracao != idConfiguracaoIniCF) { //é necessário pular a configuração que teve a primeira alteração da chave para chave aberta
                        if (configuracao == novaPosicao) {//deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                            noP = noPNovo;
                            noR = noRNovo;
                            noA = noANovo;
                            idChaveAberta = idChaveAbertaR;
                            idChaveFechada = idChaveFechadaR;
                            //insere dados no vetorPi
                            while (contadorChav < vetorPiChav[idConfiguracaoIniCA].numeroManobras) {
                                if (contadorChav == posIniCA) {
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                                    vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                                    vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                                    vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                                }
                                contadorChav++;
                            }
                            //altera o individuo para conter o chaveamento
//                            copiaIndividuoMelhorada(configuracoesChav, configuracoesChav, idConfiguracaoIniCA, idNovaConfiguracao, matrizPIChav, rnpSetoresParam, numeroBarrasParam, numeroTrafosParam);//Por Leandro
                            copiaIndividuoModificada(configuracoesChav, configuracoesChav, idConfiguracaoIniCA, idNovaConfiguracao, matrizPIChav);//Por Leandro
                            //verifica se o par de chaveamento continua valido
                            recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idNovaConfiguracao, matrizPIChav, vetorPiChav);
                            // Por Leandro: Modificado aqui para
                            // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                            // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                            determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idNovaConfiguracao, grafoSetoresParam);

                            if (rnpP != rnpR || rnpA >= configuracoesChav[idNovaConfiguracao].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                                chaveamentoCorreto = false;
                            } else {
                                //determinação do intervalo correspondente a subárvore do nó P na RNP
                                //indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP);
                                if(rnpP >= configuracoesChav[idNovaConfiguracao].numeroRNP && rnpA < configuracoesChav[idNovaConfiguracao].numeroRNP)//Operador LRO
                                	indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP - configuracoesChav[idNovaConfiguracao].numeroRNP], indiceP);
                        		else{
                        			if(rnpP < configuracoesChav[idNovaConfiguracao].numeroRNP && rnpA < configuracoesChav[idNovaConfiguracao].numeroRNP)//Operador PAO ou CAO
                        				indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP);
                        		}

                                //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                                if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: aqui a implementação da Telma foi correta
                                    chaveamentoCorreto = false;
                                } else {
                                    if (rnpA == rnpP) {
                                        //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                        if (indiceA >= indiceP && indiceA <= indiceL) {
                                            chaveamentoCorreto = false;
                                        } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                        {
                                            //realiza a alocação da RNP que será alterada
                                            alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                            constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idNovaConfiguracao, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                        }
                                    } else //aplica o chaveamento e gera uma nova configuracao simulada
                                    {

                                        int tamanhoSubarvore;
                                        tamanhoSubarvore = indiceL - indiceP + 1;
                                        if(rnpP >= configuracoesChav[idConfiguracaoIniCA].numeroRNP && rnpA < configuracoesChav[idConfiguracaoIniCA].numeroRNP){//Operador LRO //Por Leandro
                                    		//realiza a alocação das que serão alteradas RNPs
                                    		alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpA]);
                                    		alocaRNP(configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIniCA].numeroRNP].numeroNos - tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIniCA].numeroRNP]);
                                    		//constroi as rnps origem e destino novas
                                    		constroiRNPOrigem(configuracoesChav[idConfiguracaoIniCA].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIniCA].numeroRNP], &configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIniCA].numeroRNP], indiceP, indiceL, matrizPIChav, idNovaConfiguracao, rnpP);
                                    		constroiRNPDestinoCAO(configuracoesChav[idConfiguracaoIniCA].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIniCA].numeroRNP], configuracoesChav[idConfiguracaoIniCA].rnp[rnpA], &configuracoesChav[idNovaConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPIChav, idNovaConfiguracao, rnpA);
                                    		configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoIniCA].numeroRNPFicticia;

                                        }
                                        else{
                                        	if(rnpP < configuracoesChav[idConfiguracaoIniCA].numeroRNP && rnpA < configuracoesChav[idConfiguracaoIniCA].numeroRNP){ //Operador PAO ou CAO
												//realiza a alocação das que serão alteradas RNPs
												alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpA]);
												alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
												//constroi a rnp de origem nova
												constroiRNPOrigem(configuracoesChav[idConfiguracaoIniCA].rnp[rnpP], &configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idNovaConfiguracao, rnpP);
												constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoIniCA, &configuracoesChav[idNovaConfiguracao].rnp[rnpA], idNovaConfiguracao);
                                        	}
                                        }
                                    }
                                    vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                                    vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                                    idConfiguracaoAncestral = idNovaConfiguracao;
                                    idNovaConfiguracao++;
                                }
                            }

                        } else {
							noP = vetorPiChav[configuracao].nos[contadorChav].p;
							noR = vetorPiChav[configuracao].nos[contadorChav].r;
							noA = vetorPiChav[configuracao].nos[contadorChav].a;
							idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
							idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];

							//  printf("nop %ld, noA %ld noR %ld \n",noP, noA, noR);
							//verifica se o par de chaveamento continua valido
							recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
							// Por Leandro: Modificado aqui para
							// (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
							// (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
							determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

							if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
								chaveamentoCorreto = false;
							} else {
								//determinação do intervalo correspondente a subárvore do nó P na RNP
								//indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
								if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
									indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
								else{
									if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
										indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
								}

                                //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                                if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
									chaveamentoCorreto = false;
								} else {
									if (rnpA == rnpP) {
										//o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
										if (indiceA >= indiceP && indiceA <= indiceL) {
											chaveamentoCorreto = false;
										} else //aplica o chaveamento considerando a alteração de somente uma rnp
										{
											//realiza a alocação da RNP que será alterada
											alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
											constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
											for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
												if (indice1 != rnpP) {
													configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
												}
											}
											for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) { //Por Leandro
													configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
											}
											configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;
										}
									} else //aplica o chaveamento e gera uma nova configuracao simulada
									{
										obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
												indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
									}
								}
								vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
								idConfiguracaoAncestral = idNovaConfiguracao;
								idNovaConfiguracao++;
							}
                        }
                    }

                    contador--;
                }
                //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
                if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
                {
                    if (idProxConfiguracao[0] == idNovaConfiguracao) {
                        idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                    } else {
                        idNovoAncestral[0] = idNovaConfiguracao - 1;
                    }
                    idProxConfiguracao[0] = idNovaConfiguracao;
                }
            }
        } else { //o novo par de chaveamento é composto pela abertura e fechamento da mesma chave
            contador = contadorConfiguracoes - 1;
            while (contador > 0 && chaveamentoCorreto) {
                configuracao = indiceConfiguracoes[contador];
                if (configuracao != idConfiguracaoIniCF || configuracao != idConfiguracaoIniCF) {
                    contadorChav = 0;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    noP = vetorPiChav[configuracao].nos[contadorChav].p;
                    noR = vetorPiChav[configuracao].nos[contadorChav].r;
                    noA = vetorPiChav[configuracao].nos[contadorChav].a;
                    idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
                    idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];
                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                    // Por Leandro: Modificado aqui para
                    // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                    // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                    determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

                    if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                        chaveamentoCorreto = false;
                    } else {
                        //determinação do intervalo correspondente a subárvore do nó P na RNP
//                        indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                        if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
                        	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
                		else{
                			if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
                				indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                		}

                        if (rnpA == rnpP) {
                            //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                            if (indiceA >= indiceP && indiceA <= indiceL) {
                                chaveamentoCorreto = false;
                            } else //aplica o chaveamento considerando a alteração de somente uma rnp
                            {
                                //realiza a alocação da RNP que será alterada
                                alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                    if (indice1 != rnpP) {
                                        configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                    }
                                }
                                for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) { //Por Leandro
                                        configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                                }
                                configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia;
                            }
                        } else //aplica o chaveamento e gera uma nova configuracao simulada
                        {
                            obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                    indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                        }
                        vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                        idConfiguracaoAncestral = idNovaConfiguracao;
                        idNovaConfiguracao++;
                    }
                }
                contador--;
            }
            if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
            {
                if (idProxConfiguracao[0] == idNovaConfiguracao)
                    idNovoAncestral[0] = vetorPiChav[idConfiguracaoIniCA].idAncestral;
                else
                    idNovoAncestral[0] = idNovaConfiguracao - 1;
            }
        }
    }//a repetição das chaves acontece em indivíduos diferentes
    free(indiceConfiguracoes);
    indiceConfiguracoes = 0;
    return chaveamentoCorreto;
}

/**
 * Por Leandro: consiste na função "removeChaveEstadoInicialCaso3()" modificada para:
 * a) considerar a possibilidade de que a configuração análisada tenha sido gerada pelo LRO, o qual faz poda em rnp Fictícia e
 * transferência para RNPs Real.
 *
 * b) passar novos parâmentros, necessários à execução de funções que foram modificadas e que são chamadas aqui.
 *
 *  c) PARA IMPEDIR SEQUÊNCIAS QUE LEVEM AO DESLIGAMENTO DE SETORES SAUDÁVEIS NÃO AFETADOS PELA FALTA. Em outras palavras, para fazer com que sejam
 * impedida ações que levem a transferência de setores para uma RNP fictícia. Para tal, optou-se classificar esta sequência como sendo infactível.
 * Assim, poder-se-á testar o par rearranjado numa outra posição ou obter uma sequência com chave repetida mas sem desligamento de setores
 *
 * d)  verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste,
 *
 * e) para corrigir uma falha no "if" que verifica se o Nó R está presente na subárvore transferida. Para tal, mudou-se o operador lógido,
* tirando o "&&" (operador E) e colocando em seu lugar o "||" (operador OU)
 *
 * Esse método realiza a remoção da chave que retornou ao estado inicial para o caso 3. Nesse caso somente uma das chaves da manobra retornou ao estado inicial.
 * Primeiramente verifica se a chave que retornou ao estado inicial foi com manobra para abrir ou fechar.
 * Se foi com abrir faz a busca nos ancestrais pela posição onde ela foi fechada, inicialmente, e vice e versa.
 * Encontrado esse ancestral obtém um novo par de manobra combinando as chaves diferentes das duas configurações.
 * Obtido o novo par verifica em que posição o novo par será inserido, existem duas posições possíveis. No ponto onde a chave foi alterada ou no lugar que ela voltou ao estado inicial.
 * Para definir essa posição simula toda a sequência de manobras à partir do ancestral para cada uma das inserções.
 * Se todos as configurações intermediárias mantiverem a característica radial, a nova sequência é informada.
 * @param vetorPiChav
 * @param idConfiguracaoParam
 * @param matrizPIChav
 * @param configuracoesChav
 * @param idProxConfiguracao
 * @param totalConfiguracoes
 * @param idNovoAncestral
 * @return
 */
BOOL removeChaveEstadoInicialCaso3Modificada(VETORPI *vetorPiChav, int idConfiguracaoParam, MATRIZPI *matrizPIChav, CONFIGURACAO *configuracoesChav, int *idProxConfiguracao,
		int totalConfiguracoes, int *idNovoAncestral, int numeroTrafosParam, GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParamInicial) {
    /*As variáveis xxIni serão utilizadas para armazenar os dados da configuração onde ocorreu a primeira mudança na chave
     As variáveis xxFinal contém os dados na configuração onde a chave retornou ao seu estado inicial.*/
    int contador, contadorChav, novaPosicao, contadorConfiguracoes, indice1;
    BOOL encontrado = false;
    int idConfiguracaoIni, posIni, idConfiguracaoAncestral;
    long int noPIni, noRIni, noAIni, noPFinal, noRFinal, noAFinal, noPNovo, noRNovo, noANovo;
    long int idChaveAbertaR, idChaveFechadaR; //armazena o id das chaves que serao rearranjadas
    BOOL chaveamentoCorreto = true;
    int *indiceConfiguracoes;
    long int idChaveAberta, idChaveFechada, idChaveAbertaPRA, idChaveFechadaPRA;
    indiceConfiguracoes = Malloc(int, totalConfiguracoes);

    int configuracao, noA, noP, noR, rnpA, rnpP, rnpR, indiceL, indiceA, indiceP, indiceR, colunaPi;
    //localiza no vetorPi a alteração anterior da chave
    contador = vetorPiChav[idConfiguracaoParam].idAncestral;
    contadorConfiguracoes = 0;
    indiceConfiguracoes[contadorConfiguracoes] = idConfiguracaoParam;
    int idNovaConfiguracao = idProxConfiguracao[0];
    while (contador > 0 && !encontrado) {

        if (vetorPiChav[idConfiguracaoParam].estadoInicialChaveAberta[0]) //nesse caso é a chave que deve ser aberta que retornou ao estado inicial. A busca pelo configuração onde ocorreu a alteração de estado da chave deve ser no vetor de chave fechada
        {
            contadorChav = 0;
            while (contadorChav < vetorPiChav[contador].numeroManobras && !encontrado) {
                if (vetorPiChav[contador].idChaveFechada[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveAberta[0]) {
                    encontrado = true;
                    idConfiguracaoIni = contador;
                    posIni = contadorChav;
                }
                contadorChav++;
            }
        } else//nesse caso é a chave que deve ser fechada que retornou ao estado inicial. A busca pelo configuração onde ocorreu a alteração de estado da chave deve ser no vetor de chave aberta
        {
            contadorChav = 0;
            while (contadorChav < vetorPiChav[contador].numeroManobras && !encontrado) {
                if (vetorPiChav[contador].idChaveAberta[contadorChav] == vetorPiChav[idConfiguracaoParam].idChaveFechada[0]) {
                    encontrado = true;
                    idConfiguracaoIni = contador;
                    posIni = contadorChav;
                }
                contadorChav++;
            }

        }
        //salva os identificadores das configuracoes entre a repetição de chaves
        contadorConfiguracoes++;
        indiceConfiguracoes[contadorConfiguracoes] = contador;
        contador = vetorPiChav[contador].idAncestral;
    }

    //recupera os nós P, R, A da primeira mudança na chave
    noPIni = vetorPiChav[idConfiguracaoIni].nos[posIni].p;
    noRIni = vetorPiChav[idConfiguracaoIni].nos[posIni].r;
    noAIni = vetorPiChav[idConfiguracaoIni].nos[posIni].a;
    //printf("1 alteracao noP %ld noR %ld noA %ld \n", noPIni, noRIni, noAIni);
    //imprimeIndividuo(configuracoesChav[idConfiguracaoIni]);
    //recupera os nós P, R, A da segunda mudança na chave
    noPFinal = vetorPiChav[idConfiguracaoParam].nos[0].p;
    noRFinal = vetorPiChav[idConfiguracaoParam].nos[0].r;
    noAFinal = vetorPiChav[idConfiguracaoParam].nos[0].a;
    // printf("2 alteracao noP %ld noR %ld noA %ld \n", noPFinal, noRFinal, noAFinal);
    //     imprimeIndividuo(configuracoesChav[idConfiguracaoParam]);
    if (vetorPiChav[idConfiguracaoParam].estadoInicialChaveAberta[0]) //nesse caso é a chave que deve ser aberta que retornou ao estado inicial. A busca pelo configuração onde ocorreu a alteração de estado da chave deve ser no vetor de chave fechada
    {
        idChaveAbertaR = vetorPiChav[idConfiguracaoIni].idChaveAberta[posIni]; //a chave a ser aberta no novo chaveamento e a chave aberta do par inicial
        idChaveFechadaR = vetorPiChav[idConfiguracaoParam].idChaveFechada[0]; //a chave a ser fechada no novo chaveamento e a chave fechada do par onde houve chave voltando ao estado original
        noPNovo = noPIni;
        noRNovo = noRFinal;
        noANovo = noAFinal;
        //primeiro tenta inserir o par de chaveamento rearranjado na posição onde a chave que retornou ao estado inicial foi fechada
        novaPosicao = idConfiguracaoIni;
        contador = contadorConfiguracoes;
        idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
        if (vetorPiChav[idConfiguracaoIni].numeroManobras <= 1) {
            chaveamentoCorreto = simulaChaveamentoPosicaoInicialModificada(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                    idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                    idChaveFechadaR, idNovoAncestral, idConfiguracaoIni, grafoSetoresParam, idConfiguracaoParamInicial);

            //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
            if (!chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
            {
                //não foi possível realizar a sequencia de chaveamento proposta e deve-se testar o par rearranjado em outra posição
                novaPosicao = idConfiguracaoParam;
                contador = contadorConfiguracoes - 1;
                idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
                chaveamentoCorreto = true;
                chaveamentoCorreto = simulaChaveamentoPosicaoFinalModificada(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                        idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                        idChaveFechadaR, idNovoAncestral, idConfiguracaoIni, grafoSetoresParam);
            }
        } else {
            //Esse caso indica que a chave que retornou ao estado original como chave aberta foi primeiramente alterada no restabelecimento.
            //Nessa situação o par rearranjado deve obrigatoriamente ser testado na posição do restabelecimento.
            //Para fazer isso será utilizada a configuração resultante do restabelecimento e simulada a aplicação dos nós p, r, a da configuracao onde
            //houve a repetição da chave.
            while (contador > 0 && chaveamentoCorreto) {
                configuracao = indiceConfiguracoes[contador];
                contadorChav = 0;
                if (configuracao == idConfiguracaoIni) {
                    //deve utilizar o conjunto de nos rearranjado para gerar a nova configuracao
                    //Nesse caso será utilizado para simular o par de chaveamento do individuo que fez a chave voltar ao estado inicial.
                    //para não ter que realizar todoo o processo do restabelecimento de novo.
                    noP = noPFinal;
                    noR = noRFinal;
                    noA = noAFinal;
                    copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                    idChaveAberta = idChaveAbertaR;
                    idChaveFechada = idChaveFechadaR;
                    //insere dados no vetorPi
                    while (contadorChav < vetorPiChav[idConfiguracaoIni].numeroManobras) {
                        if (contadorChav == posIni) {
                            vetorPiChav[idNovaConfiguracao].estadoInicialChaveAberta[contadorChav] = false;
                            vetorPiChav[idNovaConfiguracao].estadoInicialChaveFechada[contadorChav] = false;
                            vetorPiChav[idNovaConfiguracao].idChaveAberta[contadorChav] = idChaveAbertaR;
                            vetorPiChav[idNovaConfiguracao].idChaveFechada[contadorChav] = idChaveFechadaR;
                            vetorPiChav[idNovaConfiguracao].nos[contadorChav].a = noA;
                            //  vetorPiChav[idNovaConfiguracao].nos[contadorChav].p = noP;
                            vetorPiChav[idNovaConfiguracao].nos[contadorChav].r = noR;
                        }
                        contadorChav++;
                    }
                    //altera o individuo para conter o chaveamento
//                    copiaIndividuoMelhorada(configuracoesChav, configuracoesChav, idConfiguracaoIni, idNovaConfiguracao, matrizPIChav, rnpSetoresParam, numeroBarrasParam, numeroTrafosParam);
                    copiaIndividuoModificada(configuracoesChav, configuracoesChav, idConfiguracaoIni, idNovaConfiguracao, matrizPIChav);

                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noPNovo, noANovo, noRNovo, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
//                    recuperaPosicaoPRA(noPNovo, noANovo, noRNovo, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idNovaConfiguracao, matrizPIChav, vetorPiChav);
                    // Por Leandro: Modificado aqui para
                    // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                    // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                    determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);
//                    determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idNovaConfiguracao, grafoSetoresParam);

                    if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                        chaveamentoCorreto = false;
                    } else {
                        recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idNovaConfiguracao, matrizPIChav, vetorPiChav);
                        //determinação do intervalo correspondente a subárvore do nó P na RNP
//                        indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP);
                        if(rnpP >= configuracoesChav[idNovaConfiguracao].numeroRNP && rnpA < configuracoesChav[idNovaConfiguracao].numeroRNP)//Operador LRO
                        	indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP - configuracoesChav[idNovaConfiguracao].numeroRNP], indiceP);
                		else{
                			if(rnpP < configuracoesChav[idNovaConfiguracao].numeroRNP && rnpA < configuracoesChav[idNovaConfiguracao].numeroRNP)//Operador PAO ou CAO
                				indiceL = limiteSubArvore(configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP);
                		}
                        //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                        if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: aqui a Telma implementou de maneira correta
                            chaveamentoCorreto = false;
                        } else {
                            if (rnpA == rnpP) {
                                //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                if (indiceA >= indiceP && indiceA <= indiceL) {
                                    chaveamentoCorreto = false;
                                } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                {
                                    //realiza a alocação da RNP que será alterada
                                    alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                    constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idNovaConfiguracao, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                }
                            } else //aplica o chaveamento e gera uma nova configuracao simulada
                            {

                                int tamanhoSubarvore;
                                tamanhoSubarvore = indiceL - indiceP + 1;

                                if(rnpP >= configuracoesChav[idNovaConfiguracao].numeroRNP && rnpA < configuracoesChav[idNovaConfiguracao].numeroRNP){//Operador LRO
                            		//realiza a alocação das que serão alteradas RNPs
                            		alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpA]);
                            		alocaRNP(configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIni].numeroRNP].numeroNos - tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIni].numeroRNP]);
                            		//constroi as rnps origem e destino novas
                            		constroiRNPOrigem(configuracoesChav[idConfiguracaoIni].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIni].numeroRNP], &configuracoesChav[idNovaConfiguracao].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIni].numeroRNP], indiceP, indiceL, matrizPIChav, idNovaConfiguracao, rnpP);
                            		constroiRNPDestinoCAO(configuracoesChav[idConfiguracaoIni].rnpFicticia[rnpP -  configuracoesChav[idConfiguracaoIni].numeroRNP], configuracoesChav[idConfiguracaoIni].rnp[rnpA], &configuracoesChav[idNovaConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPIChav, idNovaConfiguracao, rnpA);
                            		configuracoesChav[idNovaConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracaoIni].numeroRNPFicticia;
                                }
                        		else{
                        			if(rnpP < configuracoesChav[idNovaConfiguracao].numeroRNP && rnpA < configuracoesChav[idNovaConfiguracao].numeroRNP){//Operador PAO ou CAO
                                        //realiza a alocação das que serão alteradas RNPs
                                        alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpA]);
                                        alocaRNP(configuracoesChav[idNovaConfiguracao].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                        //constroi a rnp de origem nova
                                        constroiRNPOrigem(configuracoesChav[idConfiguracaoIni].rnp[rnpP], &configuracoesChav[idNovaConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idNovaConfiguracao, rnpP);
                                        constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoIni, &configuracoesChav[idNovaConfiguracao].rnp[rnpA], idNovaConfiguracao);
                        			}
                        		}
                            }
                            vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                            vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                            idConfiguracaoAncestral = idNovaConfiguracao;
                            idNovaConfiguracao++;
                        }
                    }

                } else //aplica a tripla de nos anterior
                {
                    noP = vetorPiChav[configuracao].nos[contadorChav].p;
                    noR = vetorPiChav[configuracao].nos[contadorChav].r;
                    noA = vetorPiChav[configuracao].nos[contadorChav].a;
                    idChaveAberta = vetorPiChav[configuracao].idChaveAberta[contadorChav];
                    idChaveFechada = vetorPiChav[configuracao].idChaveFechada[contadorChav];

                    //verifica se o par de chaveamento continua valido
                    recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idConfiguracaoAncestral, matrizPIChav, vetorPiChav);
                    // Por Leandro: Modificado aqui para
                    // (a) evitar a transferência de setores para RNPs Fictícias, isto é, desligar setores
                    // (b) verificar se os nós PRA que gerarão uma nova configurações operarão de fato as chaves que constam na sequência sob teste
                    determinaChaves(&idChaveAbertaPRA, &idChaveFechadaPRA, indiceP, indiceR, indiceA, rnpP, rnpR, rnpA, configuracoesChav, idConfiguracaoAncestral, grafoSetoresParam);

                    if (rnpP != rnpR || rnpA >= configuracoesChav[idConfiguracaoAncestral].numeroRNP || idChaveAberta != idChaveAbertaPRA || idChaveFechada != idChaveFechadaPRA) {
                        chaveamentoCorreto = false;
                    } else {
                        //determinação do intervalo correspondente a subárvore do nó P na RNP
//                        indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                        if(rnpP >= configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador LRO
                        	indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnpFicticia[rnpP - configuracoesChav[idConfiguracaoAncestral].numeroRNP], indiceP);
                		else{
                			if(rnpP < configuracoesChav[idConfiguracaoAncestral].numeroRNP && rnpA < configuracoesChav[idConfiguracaoAncestral].numeroRNP)//Operador PAO ou CAO
                				indiceL = limiteSubArvore(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP], indiceP);
                		}
                        //verifica se no noR não pertence a subárvore de noP, nesse caso não é possível garantir a factibilidade
                        if (indiceR < indiceP || indiceR > indiceL) { //Por Leandro: tirei o "&&" (operador E) e coloquei em seu lugar o "||" (operador OU)
                            chaveamentoCorreto = false;
                        } else {
                            if (rnpA == rnpP) {
                                //o no A esta na mesma rnp do noP. Nesse caso o chaveamento nao sera possivel se o noA estiver no intervalor de indices da rnp que contem a subarvore a ser podada.
                                if (indiceA >= indiceP && indiceA <= indiceL) {
                                    chaveamentoCorreto = false;
                                } else //aplica o chaveamento considerando a alteração de somente uma rnp
                                {
                                    //realiza a alocação da RNP que será alterada
                                    alocaRNP(configuracoesChav[idConfiguracaoAncestral].rnp[rnpP].numeroNos, &configuracoesChav[idNovaConfiguracao].rnp[rnpP]);
                                    constroiRNPOrigemDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracaoAncestral, &configuracoesChav[idNovaConfiguracao].rnp[rnpP], idNovaConfiguracao);
                                    for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNP; indice1++) {
                                        if (indice1 != rnpP) {
                                            configuracoesChav[idNovaConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracaoAncestral].rnp[indice1];
                                        }
                                    }
                                    for (indice1 = 0; indice1 < configuracoesChav[idConfiguracaoAncestral].numeroRNPFicticia; indice1++) { //Por Leandro
                                            configuracoesChav[idNovaConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracaoAncestral].rnpFicticia[indice1];
                                    }
                                }
                            } else //aplica o chaveamento e gera uma nova configuracao simulada
                            {
                                obtemConfiguracaoModificada(configuracoesChav, idConfiguracaoAncestral, idNovaConfiguracao,
                                        indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPIChav);
                            }
                            copiaDadosVetorPI(vetorPiChav, vetorPiChav, configuracao, idNovaConfiguracao);
                            vetorPiChav[idNovaConfiguracao].idAncestral = idConfiguracaoAncestral;
                            vetorPiChav[idNovaConfiguracao].casoManobra = 1;
                            idConfiguracaoAncestral = idNovaConfiguracao;
                            idNovaConfiguracao++;
                        }
                    }
                }
                contador--;
            }
            if (chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
            {
                if (idProxConfiguracao[0] == idNovaConfiguracao)
                    idNovoAncestral[0] = vetorPiChav[idConfiguracaoIni].idAncestral;
                else
                    idNovoAncestral[0] = idNovaConfiguracao - 1;
            }
            idProxConfiguracao[0] = idNovaConfiguracao;
        }
    } else { //nesse caso a chave fechou é a que está repetida.
        idChaveAbertaR = vetorPiChav[idConfiguracaoParam].idChaveAberta[0]; //a chave a ser aberta no novo chaveamento e a chave aberta do par onde houve chave voltando ao estado original
        idChaveFechadaR = vetorPiChav[idConfiguracaoIni].idChaveFechada[posIni]; //a chave a ser fechada no novo chaveamento e a chave fechada do par inicial
        noPNovo = noPFinal;
        noRNovo = noRIni;
        noANovo = noAIni;
        //primeiro tenta inserir o par de chaveamento rearranjado na posição onde a chave que retornou ao estado inicial foi fechada

        //não foi possível realizar a sequencia de chaveamento proposta e deve-se testar o par rearranjado em outra posição
        novaPosicao = idConfiguracaoParam;
        contador = contadorConfiguracoes - 1;
        idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
        chaveamentoCorreto = simulaChaveamentoPosicaoFinalModificada(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                idChaveFechadaR, idNovoAncestral, idConfiguracaoIni, grafoSetoresParam);

        if (!chaveamentoCorreto)// a nova sequencia de chaveamento deu certo atualizar o identificador da próxima configuracao
        {
            //se o chaveamento não der certo tem que tentar a outra posição de inserção da chave
            novaPosicao = idConfiguracaoIni;
            contador = contadorConfiguracoes;
            idConfiguracaoAncestral = vetorPiChav[idConfiguracaoIni].idAncestral;
            chaveamentoCorreto = simulaChaveamentoPosicaoInicialModificada(indiceConfiguracoes, contador, vetorPiChav, matrizPIChav,
                    idProxConfiguracao, configuracoesChav, noPNovo, noANovo, noRNovo, posIni, novaPosicao, idConfiguracaoAncestral, idChaveAbertaR,
                    idChaveFechadaR, idNovoAncestral, idConfiguracaoIni, grafoSetoresParam, idConfiguracaoParamInicial);
        }

    }
    free(indiceConfiguracoes);
    indiceConfiguracoes = 0;
    return chaveamentoCorreto;
}

/**
 * Por Leandro: consiste na função "removeChavesEstadoInicial()" modificada para:
 * a) considerar a possibilidade de que a configuração análisada tenha sido gerada pelo LRO, o qual faz poda em rnp Fictícia e
 * transferência para RNPs Real.
 * b) considerar a possibilidade de que a configuração análisada tenha sido gerada pela rotina que obtem a "configuracão inicial", a qual
 * faz podas em RNP reais e transferência para RNPs fictícias.
 *
 * Recupera utilizando o VETORPI da RNP todas as configuracões geradas pelo algoritmo evolutivo em tabelas até obter a configuracao final.
 * @param vetorPiParam
 * @param idConfiguracaoParam
 * @param totalConfiguracoes
 * @return
 */
//VETORPI *removeChavesEstadoInicialModificada(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,
//		  long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam, long int numeroBarrasParam) {
//    int totalConfiguracoes, contador, somaChaveamentos = 0;
//    long int indiceInd;
//    long int *indiceConfiguracoes;// = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
//    VETORPI *vetorPiChav;
//    CONFIGURACAO *configuracoesChav;
//    MATRIZPI *matrizPIChav;
//    int tamanhoAlocacao;
//    int idConfiguracao = 0;
//    long int noP, noR, noA, colunaPI;
//    int indice1, indiceP, indiceA, indiceR, rnpP, rnpA, indiceL;
//    int tamanhoSubarvore;
//    int idNovoAncestral;
//    long int *indicesFalta;
//    int numeroConfiguracoesFalta;
//    BOOL chaveamentoCorreto = false;
//    //soma o total de pares de chaveamento para obter a configuração no AEMT
//    // printf("totalConfiguracoes %d \n", totalConfiguracoes);
//    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
//    for (contador = 0; contador < totalConfiguracoes; contador++) {
//        indiceInd = indiceConfiguracoes[contador];
//        somaChaveamentos += vetorPiParam[indiceInd].numeroManobras;
//    }
//    tamanhoAlocacao = somaChaveamentos * (somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
//    //aloca o vetorPi, MatrizPI e Vetor de configurações para realizar o processo de remoção de chaves repetidas
//    inicializaVetorPi(tamanhoAlocacao, &vetorPiChav);
//    inicializaMatrizPI(grafoSetoresParam, &matrizPIChav, (tamanhoAlocacao * 100), numeroSetores + configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia*2);
//    configuracoesChav = alocaIndividuoModificada(configuracoesParam[idConfiguracaoParam[0]].numeroRNP, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, idConfiguracao, (tamanhoAlocacao), numeroTrafos, numeroSetores); //Por Leandro
//    inicializaRNPsFicticias(configuracoesChav, idConfiguracao, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, matrizPIChav, numeroSetores);    //Por Leandro: Aloca as RNPs Fictícias para a configuração "idConfiguracao"
//
//    //copia a configuração inicial pré falta para iniciar a montagem auxiliar das soluções do AEMT para realizar a redução de chaves
//    copiaIndividuoMelhorada(configuracoesParam, configuracoesChav, 0, idConfiguracao, matrizPIChav, rnpSetoresParam, numeroBarrasParam, numeroTrafos); //Por Leandro
//
//    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiParam, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
//    //copia as configuracoes de falta
//    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
//        indiceInd = indicesFalta[contador];
//        idConfiguracao++;
//        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        copiaIndividuoMelhorada(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav, rnpSetoresParam, numeroBarrasParam, numeroTrafos); //Por Leandro
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//    }
//
//    //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
//    for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
//        indiceInd = indiceConfiguracoes[contador];
//        idConfiguracao++;
//        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        noP = vetorPiParam[indiceInd].nos[0].p;
//        noA = vetorPiParam[indiceInd].nos[0].a;
//        noR = vetorPiParam[indiceInd].nos[0].r;
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, idConfiguracao - 1);
//        indiceA = matrizPIChav[noA].colunas[colunaPI].posicao;
//        rnpA = matrizPIChav[noA].colunas[colunaPI].idRNP;
//        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noP, idConfiguracao - 1);
//        indiceP = matrizPIChav[noP].colunas[colunaPI].posicao;
//        rnpP = matrizPIChav[noP].colunas[colunaPI].idRNP;
//        colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noR, idConfiguracao - 1);
//        indiceR = matrizPIChav[noR].colunas[colunaPI].posicao;
//
//        //realiza a alocação das RNPs que serão alteradas e Construção das RNPs Origem e Destino Novas
//        if(rnpP >= configuracoesChav[idConfiguracao - 1].numeroRNP && rnpA < configuracoesChav[idConfiguracao - 1].numeroRNP){//Operador LRO
//			rnpP = rnpP - configuracoesChav[idConfiguracao - 1].numeroRNP;
//	        indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//	        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//			alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos+tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
//			alocaRNP(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnpFicticia[rnpP]);
//			//Obtém a nova rnp de origem, sem a subárvore podada
//			constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP], &configuracoesChav[idConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP + configuracoesChav[idConfiguracao - 1].numeroRNP);
//			//Obtém a nova rnp de destino, inserindo a subárvore podada
//			constroiRNPDestinoCAO(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP], configuracoesChav[idConfiguracao - 1].rnp[rnpA], &configuracoesChav[idConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPIChav, idConfiguracao, rnpA);
//
//			for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++ ){
//				if (indice1 != rnpA)
//					configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
//			}
//			for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//				if (indice1 != rnpP)
//					configuracoesChav[idConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracao - 1].rnpFicticia[indice1];
//			}
//			configuracoesChav[idConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracao - 1].numeroRNPFicticia;
//
//
//        }
//		else{
//			if(rnpP < configuracoesChav[idConfiguracao - 1].numeroRNP && rnpA < configuracoesChav[idConfiguracao - 1].numeroRNP){//Operador PAO ou CAO
//		        indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnp[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//		        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//				alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
//				alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpP]);
//
//				constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnp[rnpP], &configuracoesChav[idConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP);
//				constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracao - 1, &configuracoesChav[idConfiguracao].rnp[rnpA], idConfiguracao);
//
//		        for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++) {
//		            if (indice1 != rnpP && indice1 != rnpA)
//		                configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
//		        }
//		        configuracoesChav[idConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracao - 1].numeroRNPFicticia;
//				for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//					configuracoesChav[idConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracao - 1].rnpFicticia[indice1];
//				}
//			}
//
//		}
//    }
//
//    //localiza no vetorPi o ancestral que possui chave que retornou ao estado inicial
//    idConfiguracaoParam[0] = idConfiguracao;
//    idConfiguracao++;
//    //imprimeVetorPi(vetorPiChav, totalConfiguracoes + 1, listaChavesParam);
//    for (contador = 1; contador <= totalConfiguracoes; contador++) {
//        //printf("contador %d idConfiguracaoancestral %ld casoManobra %d\n", contador, vetorPiChav[contador].idAncestral, vetorPiChav[contador].casoManobra);
//        chaveamentoCorreto = false;
//        if (vetorPiChav[contador].casoManobra == 2) {//as duas chaves retornaram ao estado inicial
//            chaveamentoCorreto = removeChaveEstadoInicialCaso2Modificada(contador, vetorPiChav, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral, numeroTrafos, grafoSetoresParam);
//        } else {
//            if (vetorPiChav[contador].casoManobra == 3) { //somente uma das chaves retornou ao estadoInicial
//                chaveamentoCorreto = removeChaveEstadoInicialCaso3Modificada(vetorPiChav, contador, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral, rnpSetoresParam, grafoSetoresParam);
//            }
//        }
//        if (chaveamentoCorreto) {
//            //localiza no vetorPI a configuracao que tem como ancestral a configuracao que apresentou chave repetida e atualiza para ter como ancestral a configuracao resultante do novo chaveamento
//            indice1 = contador + 1;
//            while (indice1 <= totalConfiguracoes && vetorPiChav[indice1].idAncestral != contador) {
//                indice1++;
//            }
//            if (contador == totalConfiguracoes) {
//                idConfiguracaoParam[0] = idNovoAncestral;
//            }
//            if (indice1 <= totalConfiguracoes) {
//                vetorPiChav[indice1].idAncestral = idNovoAncestral;
//            }
//        }
//    }
//
//    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam[0], &totalConfiguracoes);
//    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiChav, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
//    if (numeroConfiguracoesFalta > 1) {
//        idConfiguracao++;
//        copiaDadosVetorPIMultiplasFaltas(vetorPiChav, vetorPiChav, idConfiguracao, numeroConfiguracoesFalta, indicesFalta);
//        //replica a primeira configuração obtida na etapa de restabelecimento
//        indiceInd = indicesFalta[numeroConfiguracoesFalta - 1];
//        // copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        vetorPiChav[idConfiguracao].idAncestral = 0;
//        copiaIndividuoMelhorada(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav, rnpSetoresParam, numeroBarrasParam, numeroTrafos);
//        //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
//        for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
//            indiceInd = indiceConfiguracoes[contador];
//            idConfiguracao++;
//            copiaDadosVetorPI(vetorPiChav, vetorPiChav, indiceInd, idConfiguracao);
//            noP = vetorPiChav[indiceInd].nos[0].p;
//            noA = vetorPiChav[indiceInd].nos[0].a;
//            noR = vetorPiChav[indiceInd].nos[0].r;
//            vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//            colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, idConfiguracao - 1);
//            indiceA = matrizPIChav[noA].colunas[colunaPI].posicao;
//            rnpA = matrizPIChav[noA].colunas[colunaPI].idRNP;
//            colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noP, idConfiguracao - 1);
//            indiceP = matrizPIChav[noP].colunas[colunaPI].posicao;
//            rnpP = matrizPIChav[noP].colunas[colunaPI].idRNP;
//            colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noR, idConfiguracao - 1);
//            indiceR = matrizPIChav[noR].colunas[colunaPI].posicao;
//
//            if(rnpP >= configuracoesChav[idConfiguracao - 1].numeroRNP && rnpA < configuracoesChav[idConfiguracao - 1].numeroRNP){//Operador LRO
//    			rnpP = rnpP - configuracoesChav[idConfiguracao - 1].numeroRNP;
//    	        indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//    	        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//    			alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos+tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
//    			alocaRNP(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnpFicticia[rnpP]);
//    			//Obtém a nova rnp de origem, sem a subárvore podada
//    			constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP], &configuracoesChav[idConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP + configuracoesChav[idConfiguracao - 1].numeroRNP);
//    			//Obtém a nova rnp de destino, inserindo a subárvore podada
//    			constroiRNPDestinoCAO(configuracoesChav[idConfiguracao - 1].rnpFicticia[rnpP], configuracoesChav[idConfiguracao - 1].rnp[rnpA], &configuracoesChav[idConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, matrizPIChav, idConfiguracao, rnpA);
//
//    			for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++ ){
//    				if (indice1 != rnpA)
//    					configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
//    			}
//    			for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//    				if (indice1 != rnpP)
//    					configuracoesChav[idConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracao - 1].rnpFicticia[indice1];
//    			}
//    			configuracoesChav[idConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracao - 1].numeroRNPFicticia;
//
//
//            }
//    		else{
//    			if(rnpP < configuracoesChav[idConfiguracao - 1].numeroRNP && rnpA < configuracoesChav[idConfiguracao - 1].numeroRNP){//Operador PAO ou CAO
//					indiceL = limiteSubArvore(configuracoesChav[idConfiguracao - 1].rnp[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//    		        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//    	            alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpA]);
//    	            alocaRNP(configuracoesChav[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &configuracoesChav[idConfiguracao].rnp[rnpP]);
//
//    	            constroiRNPOrigem(configuracoesChav[idConfiguracao - 1].rnp[rnpP], &configuracoesChav[idConfiguracao].rnp[rnpP], indiceP, indiceL, matrizPIChav, idConfiguracao, rnpP);
//    	            constroiRNPDestino(configuracoesChav, rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, matrizPIChav, idConfiguracao - 1, &configuracoesChav[idConfiguracao].rnp[rnpA], idConfiguracao);
//
//    	            for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNP; indice1++) {
//    	                if (indice1 != rnpP && indice1 != rnpA)
//    	                    configuracoesChav[idConfiguracao].rnp[indice1] = configuracoesChav[idConfiguracao - 1].rnp[indice1];
//    	            }
//    		        configuracoesChav[idConfiguracao].numeroRNPFicticia = configuracoesChav[idConfiguracao - 1].numeroRNPFicticia;
//    				for (indice1 = 0; indice1 < configuracoesChav[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//    					configuracoesChav[idConfiguracao].rnpFicticia[indice1] = configuracoesChav[idConfiguracao - 1].rnpFicticia[indice1];
//    				}
//    			}
//
//    		}
//        }
//
//        idConfiguracaoParam[0] = idConfiguracao;
//    }
//    desalocaMatrizPI(matrizPIChav, numeroSetores);
//    free(matrizPIChav);
//    matrizPIChav = 0;
//    free(configuracoesChav);
//    configuracoesChav = 0;
//
//    return vetorPiChav;
//}
//
///**
// * Por Leandro: Diferentemente da função "removeChavesEstadoInicialModificada()", nesta função
// * a) as variáveis "configuracoesChav", "matrizPIChav" são passadas como parâmentros
// * b) as variáveis "configuracoesChav", "matrizPIChav" são passadas por referência a fim de que os valores que elas
// * assumirem dentro desta função sejam conhecidos também pela função que a chamou. Isto é necessário para que as RNPs armazenadas em
// * "configuracoesChav" sejam usada na avaliação das configurações intermediárias salvas em "configuracoesChav".
// *
// * Assim como a "removeChavesEstadoInicialModificada()", esta função "removeChavesEstadoInicialModificadaV2()"
// * está modificada em relação a "removeChavesEstadoInicial()"para:
// * a) considerar a possibilidade de que a configuração análisada tenha sido gerada pelo LRO, o qual faz poda em rnp Fictícia e
// * transferência para RNPs Real.
// * b) considerar a possibilidade de que a configuração análisada tenha sido gerada pela rotina que obtem a "configuracão inicial", a qual
// * faz podas em RNP reais e transferência para RNPs fictícias.
// *
// * Recupera utilizando o VETORPI da RNP todas as configuracões geradas pelo algoritmo evolutivo em tabelas até obter a configuracao final.
// * @param vetorPiParam
// * @param idConfiguracaoParam
// * @param totalConfiguracoes
// * @return
// */
//VETORPI *removeChavesEstadoInicialModificadaV2(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,
//		  long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam,
//		  long int numeroBarrasParam, CONFIGURACAO **configuracoesChav, MATRIZPI **matrizPIChav, int *tamanhoAlocacaoParam) {
//    int totalConfiguracoes, contador, somaChaveamentos = 0;
//    long int indiceInd;
//    long int *indiceConfiguracoes;// = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
//    VETORPI *vetorPiChav;
////    CONFIGURACAO *configuracoesChav;
////    MATRIZPI *matrizPIChav;
//    int tamanhoAlocacao;
//    int idConfiguracao = 0;
//    long int noP, noR, noA, colunaPI;
//    int indice1, indiceP, indiceA, indiceR, rnpP, rnpA, indiceL;
//    int tamanhoSubarvore;
//    int idNovoAncestral;
//    long int *indicesFalta;
//    indicesFalta = 0;
//    int numeroConfiguracoesFalta;
//    BOOL chaveamentoCorreto = false;
//
//    //soma o total de pares de chaveamento para obter a configuração no AEMT
//    // printf("totalConfiguracoes %d \n", totalConfiguracoes);
//    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
//    for (contador = 0; contador < totalConfiguracoes; contador++) {
//        indiceInd = indiceConfiguracoes[contador];
//        somaChaveamentos += vetorPiParam[indiceInd].numeroManobras;
//    }
////    tamanhoAlocacao = somaChaveamentos * (somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
//    tamanhoAlocacao =  (somaChaveamentos/3)*(somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
//    //aloca o vetorPi, MatrizPI e Vetor de configurações para realizar o processo de remoção de chaves repetidas
//    inicializaVetorPi(tamanhoAlocacao, &vetorPiChav);
//    inicializaMatrizPI(grafoSetoresParam, &(*matrizPIChav), (tamanhoAlocacao *10), numeroSetores + configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia);
//    (*configuracoesChav) = alocaIndividuoModificada(configuracoesParam[idConfiguracaoParam[0]].numeroRNP, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, idConfiguracao, (tamanhoAlocacao), numeroTrafos, numeroSetores); //Por Leandro
//    inicializaRNPsFicticias((*configuracoesChav), idConfiguracao, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, (*matrizPIChav), numeroSetores);    //Por Leandro: Aloca as RNPs Fictícias para a configuração "idConfiguracao"
//
//    tamanhoAlocacaoParam[0] = tamanhoAlocacao;
//    //copia a configuração inicial pré falta para iniciar a montagem auxiliar das soluções do AEMT para realizar a redução de chaves
//    copiaIndividuoMelhorada(configuracoesParam, (*configuracoesChav), 0, idConfiguracao, (*matrizPIChav), rnpSetoresParam, numeroBarrasParam, numeroTrafos); //Por Leandro
//
//    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiParam, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
//    //copia as configuracoes de falta
//    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
//        indiceInd = indicesFalta[contador];
//        idConfiguracao++;
//        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        copiaIndividuoMelhorada(configuracoesParam, (*configuracoesChav), indiceInd, idConfiguracao, (*matrizPIChav), rnpSetoresParam, numeroBarrasParam, numeroTrafos); //Por Leandro
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//    }
//
//    //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
//    for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
//        indiceInd = indiceConfiguracoes[contador];
//        idConfiguracao++;
//        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        noP = vetorPiParam[indiceInd].nos[0].p;
//        noA = vetorPiParam[indiceInd].nos[0].a;
//        noR = vetorPiParam[indiceInd].nos[0].r;
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//        colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noA, idConfiguracao - 1);
//        indiceA = (*matrizPIChav)[noA].colunas[colunaPI].posicao;
//        rnpA = (*matrizPIChav)[noA].colunas[colunaPI].idRNP;
//        colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noP, idConfiguracao - 1);
//        indiceP = (*matrizPIChav)[noP].colunas[colunaPI].posicao;
//        rnpP = (*matrizPIChav)[noP].colunas[colunaPI].idRNP;
//        colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noR, idConfiguracao - 1);
//        indiceR = (*matrizPIChav)[noR].colunas[colunaPI].posicao;
//
//        //realiza a alocação das RNPs que serão alteradas e Construção das RNPs Origem e Destino Novas
//        if(rnpP >= (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador LRO
//			rnpP = rnpP - (*configuracoesChav)[idConfiguracao - 1].numeroRNP;
//	        indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//	        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos+tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP]);
//			//Obtém a nova rnp de origem, sem a subárvore podada
//			constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP + (*configuracoesChav)[idConfiguracao - 1].numeroRNP);
//			//Obtém a nova rnp de destino, inserindo a subárvore podada
//			constroiRNPDestinoCAO((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], (*configuracoesChav)[idConfiguracao - 1].rnp[rnpA], &(*configuracoesChav)[idConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, (*matrizPIChav), idConfiguracao, rnpA);
//
//			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++ ){
//				if (indice1 != rnpA)
//					(*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//			}
//			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//				if (indice1 != rnpP)
//					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//			}
//			(*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//
//
//        }
//		else{
//			if(rnpP < (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador PAO ou CAO
//		        indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//		        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//				alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//				alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpP]);
//
//				constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], &(*configuracoesChav)[idConfiguracao].rnp[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP);
//				constroiRNPDestino((*configuracoesChav), rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, (*matrizPIChav), idConfiguracao - 1, &(*configuracoesChav)[idConfiguracao].rnp[rnpA], idConfiguracao);
//
//		        for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++) {
//		            if (indice1 != rnpP && indice1 != rnpA)
//		                (*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//		        }
//		        (*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//				for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//				}
//			}
//
//		}
//    }
//
//    //localiza no vetorPi o ancestral que possui chave que retornou ao estado inicial
//    idConfiguracaoParam[0] = idConfiguracao;
//    idConfiguracao++;
//    //imprimeVetorPi(vetorPiChav, totalConfiguracoes + 1, listaChavesParam);
//    for (contador = 1; contador <= totalConfiguracoes; contador++) {
//        //printf("contador %d idConfiguracaoancestral %ld casoManobra %d\n", contador, vetorPiChav[contador].idAncestral, vetorPiChav[contador].casoManobra);
//        chaveamentoCorreto = false;
//        if (vetorPiChav[contador].casoManobra == 2) {//as duas chaves retornaram ao estado inicial
//            chaveamentoCorreto = removeChaveEstadoInicialCaso2Modificada(contador, &vetorPiChav, &(*matrizPIChav), &(*configuracoesChav), &idConfiguracao, totalConfiguracoes, &idNovoAncestral, numeroTrafos, grafoSetoresParam);
//        } else {
//            if (vetorPiChav[contador].casoManobra == 3) { //somente uma das chaves retornou ao estadoInicial
//                chaveamentoCorreto = removeChaveEstadoInicialCaso3Modificada(&vetorPiChav, contador, &(*matrizPIChav), &(*configuracoesChav), &idConfiguracao, totalConfiguracoes, &idNovoAncestral, rnpSetoresParam, grafoSetoresParam);
//            }
//        }
//        if (chaveamentoCorreto) {
//            //localiza no vetorPI a configuracao que tem como ancestral a configuracao que apresentou chave repetida e atualiza para ter como ancestral a configuracao resultante do novo chaveamento
//            indice1 = contador + 1;
//            while (indice1 <= totalConfiguracoes && vetorPiChav[indice1].idAncestral != contador) {
//                indice1++;
//            }
//            if (contador == totalConfiguracoes) {
//                idConfiguracaoParam[0] = idNovoAncestral;
//            }
//            if (indice1 <= totalConfiguracoes) {
//                vetorPiChav[indice1].idAncestral = idNovoAncestral;
//            }
//        }
//    }
//
//    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam[0], &totalConfiguracoes);
//    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiChav, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
//    if (numeroConfiguracoesFalta > 1) {
//        idConfiguracao++;
//        copiaDadosVetorPIMultiplasFaltas(vetorPiChav, vetorPiChav, idConfiguracao, numeroConfiguracoesFalta, indicesFalta);
//        //replica a primeira configuração obtida na etapa de restabelecimento
//        indiceInd = indicesFalta[numeroConfiguracoesFalta - 1];
//        // copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        vetorPiChav[idConfiguracao].idAncestral = 0;
//        copiaIndividuoMelhorada(configuracoesParam, (*configuracoesChav), indiceInd, idConfiguracao, (*matrizPIChav), rnpSetoresParam, numeroBarrasParam, numeroTrafos);
//        //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
//        for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
//            indiceInd = indiceConfiguracoes[contador];
//            idConfiguracao++;
//            copiaDadosVetorPI(vetorPiChav, vetorPiChav, indiceInd, idConfiguracao);
//            noP = vetorPiChav[indiceInd].nos[0].p;
//            noA = vetorPiChav[indiceInd].nos[0].a;
//            noR = vetorPiChav[indiceInd].nos[0].r;
//            vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//            colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noA, idConfiguracao - 1);
//            indiceA = (*matrizPIChav)[noA].colunas[colunaPI].posicao;
//            rnpA = (*matrizPIChav)[noA].colunas[colunaPI].idRNP;
//            colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noP, idConfiguracao - 1);
//            indiceP = (*matrizPIChav)[noP].colunas[colunaPI].posicao;
//            rnpP = (*matrizPIChav)[noP].colunas[colunaPI].idRNP;
//            colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noR, idConfiguracao - 1);
//            indiceR = (*matrizPIChav)[noR].colunas[colunaPI].posicao;
//
//            if(rnpP >= (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador LRO
//    			rnpP = rnpP - (*configuracoesChav)[idConfiguracao - 1].numeroRNP;
//    	        indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//    	        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//    			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos+tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//    			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP]);
//    			//Obtém a nova rnp de origem, sem a subárvore podada
//    			constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP + (*configuracoesChav)[idConfiguracao - 1].numeroRNP);
//    			//Obtém a nova rnp de destino, inserindo a subárvore podada
//    			constroiRNPDestinoCAO((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], (*configuracoesChav)[idConfiguracao - 1].rnp[rnpA], &(*configuracoesChav)[idConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, (*matrizPIChav), idConfiguracao, rnpA);
//
//    			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++ ){
//    				if (indice1 != rnpA)
//    					(*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//    			}
//    			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//    				if (indice1 != rnpP)
//    					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//    			}
//    			(*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//
//
//            }
//    		else{
//    			if(rnpP < (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador PAO ou CAO
//					indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//    		        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//    	            alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//    	            alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpP]);
//
//    	            constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], &(*configuracoesChav)[idConfiguracao].rnp[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP);
//    	            constroiRNPDestino((*configuracoesChav), rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, (*matrizPIChav), idConfiguracao - 1, &(*configuracoesChav)[idConfiguracao].rnp[rnpA], idConfiguracao);
//
//    	            for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++) {
//    	                if (indice1 != rnpP && indice1 != rnpA)
//    	                    (*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//    	            }
//    		        (*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//    				for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//    					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//    				}
//    			}
//
//    		}
//        }
//
//        idConfiguracaoParam[0] = idConfiguracao;
//    }
//
//    if(indicesFalta != NULL){
//    	free(indicesFalta); indicesFalta = 0;}
//    free(indiceConfiguracoes);
//    indiceConfiguracoes = 0;
//
//    return vetorPiChav;
//}
//
///*
// * Por Leandro:
// * DIferentemente da função "removeChavesEstadoInicialModificadaV2()", nesta função o vetor Pi Temporário ("vetorPiChav" ou "vetorPiOtimizado") é
// * passada como parâmetro e não criada dentro da própria função, como já ocorria em "removeChavesEstadoInicialModificadaV2()" com as variáveis
// * "configuracoesChav" e "vetorPiChav".
// * Ademais, para esta função estas três váriáveis temporárias são passados com memórias já alocadas, diferentemnte de "removeChavesEstadoInicialModificadaV2()".
// */
//
//void *removeChavesEstadoInicialModificadaV3(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,
//		  long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam, RNPSETORES *rnpSetoresParam,
//		  long int numeroBarrasParam, CONFIGURACAO **configuracoesChav, MATRIZPI **matrizPIChav, VETORPI *vetorPiChav, int *numeroConfiguracoesCopiadasParam) {
//    int totalConfiguracoes, contador, somaChaveamentos = 0;
//    long int indiceInd;
//    long int *indiceConfiguracoes;// = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
//    //VETORPI *vetorPiChav;
////    CONFIGURACAO *configuracoesChav;
////    MATRIZPI *matrizPIChav;
//    int tamanhoAlocacao;
//    int idConfiguracao = 0;
//    long int noP, noR, noA, colunaPI;
//    int indice1, indiceP, indiceA, indiceR, rnpP, rnpA, indiceL;
//    int tamanhoSubarvore;
//    int idNovoAncestral;
//    long int *indicesFalta;
//    indicesFalta = 0;
//    int numeroConfiguracoesFalta;
//    BOOL chaveamentoCorreto = false;
//    long int *idChaveAberta, *idChaveFechada;
//    BOOL *estadoInicialCA;
//    BOOL *estadoInicialCF;
//    NOSPRA *nosPRA;
//
//
//    //soma o total de pares de chaveamento para obter a configuração no AEMT
//    // printf("totalConfiguracoes %d \n", totalConfiguracoes);
//    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
///*
//    for (contador = 0; contador < totalConfiguracoes; contador++) {
//        indiceInd = indiceConfiguracoes[contador];
//        somaChaveamentos += vetorPiParam[indiceInd].numeroManobras;
//    }
//    //tamanhoAlocacao = somaChaveamentos * (somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
//    tamanhoAlocacao =  (somaChaveamentos/3)*(somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
//    //aloca o vetorPi, MatrizPI e Vetor de configurações para realizar o processo de remoção de chaves repetidas
//    inicializaVetorPi(tamanhoAlocacao, &vetorPiChav);
//    inicializaMatrizPI(grafoSetoresParam, &(*matrizPIChav), (tamanhoAlocacao * 100), numeroSetores + configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia*2);
//    (*configuracoesChav) = alocaIndividuoModificada(configuracoesParam[idConfiguracaoParam[0]].numeroRNP, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, idConfiguracao, (tamanhoAlocacao), numeroTrafos, numeroSetores); //Por Leandro
//    inicializaRNPsFicticias((*configuracoesChav), idConfiguracao, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, (*matrizPIChav), numeroSetores);    //Por Leandro: Aloca as RNPs Fictícias para a configuração "idConfiguracao"
//*/
//
//    //copia a configuração inicial pré falta para iniciar a montagem auxiliar das soluções do AEMT para realizar a redução de chaves
//    copiaIndividuoMelhorada(configuracoesParam, (*configuracoesChav), 0, idConfiguracao, (*matrizPIChav), rnpSetoresParam, numeroBarrasParam, numeroTrafos); //Por Leandro
//    copiaDadosVetorPIModificada(vetorPiParam, vetorPiChav, 0, idConfiguracao);
//    vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//
//    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiParam, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
//    numeroConfiguracoesCopiadasParam[0] = numeroConfiguracoesFalta;
//    //copia as configuracoes de falta
//    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
//        indiceInd = indicesFalta[contador];
//        idConfiguracao++;
//        copiaDadosVetorPIModificada(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//
//        copiaIndividuoMelhorada(configuracoesParam, (*configuracoesChav), indiceInd, idConfiguracao, (*matrizPIChav), rnpSetoresParam, numeroBarrasParam, numeroTrafos); //Por Leandro
//        numeroConfiguracoesCopiadasParam[0]++;
//    }
//
//    //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
//    for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
//        indiceInd = indiceConfiguracoes[contador];
//        idConfiguracao++;
//        noP = vetorPiParam[indiceInd].nos[0].p;
//        noA = vetorPiParam[indiceInd].nos[0].a;
//        noR = vetorPiParam[indiceInd].nos[0].r;
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//        colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noA, idConfiguracao - 1);
//        indiceA = (*matrizPIChav)[noA].colunas[colunaPI].posicao;
//        rnpA = (*matrizPIChav)[noA].colunas[colunaPI].idRNP;
//        colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noP, idConfiguracao - 1);
//        indiceP = (*matrizPIChav)[noP].colunas[colunaPI].posicao;
//        rnpP = (*matrizPIChav)[noP].colunas[colunaPI].idRNP;
//        colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noR, idConfiguracao - 1);
//        indiceR = (*matrizPIChav)[noR].colunas[colunaPI].posicao;
//
//
//        //realiza a alocação das RNPs que serão alteradas e Construção das RNPs Origem e Destino Novas
//        if(rnpP >= (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador LRO
//			rnpP = rnpP - (*configuracoesChav)[idConfiguracao - 1].numeroRNP;
//	        indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//	        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos+tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP]);
//			//Obtém a nova rnp de origem, sem a subárvore podada
//			constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP + (*configuracoesChav)[idConfiguracao - 1].numeroRNP);
//			//Obtém a nova rnp de destino, inserindo a subárvore podada
//			constroiRNPDestinoCAO((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], (*configuracoesChav)[idConfiguracao - 1].rnp[rnpA], &(*configuracoesChav)[idConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, (*matrizPIChav), idConfiguracao, rnpA);
//
//			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++ ){
//				if (indice1 != rnpA)
//					(*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//			}
//			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//				if (indice1 != rnpP)
//					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//			}
//			(*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//        }
//		else{
//			if(rnpP < (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador PAO ou CAO
//		        indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//		        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//				alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//				alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpP]);
//
//				constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], &(*configuracoesChav)[idConfiguracao].rnp[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP);
//				constroiRNPDestino((*configuracoesChav), rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, (*matrizPIChav), idConfiguracao - 1, &(*configuracoesChav)[idConfiguracao].rnp[rnpA], idConfiguracao);
//
//		        for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++) {
//		            if (indice1 != rnpP && indice1 != rnpA)
//		                (*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//		        }
//		        (*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//				for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//				}
//			}
//		}
//
//        copiaDadosVetorPIModificada(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//    }
//
//    //localiza no vetorPi o ancestral que possui chave que retornou ao estado inicial
//    idConfiguracaoParam[0] = idConfiguracao;
//    idConfiguracao++;
//    //imprimeVetorPi(vetorPiChav, totalConfiguracoes + 1, listaChavesParam);
//    for (contador = 1; contador <= totalConfiguracoes; contador++) {
//        //printf("contador %d idConfiguracaoancestral %ld casoManobra %d\n", contador, vetorPiChav[contador].idAncestral, vetorPiChav[contador].casoManobra);
//        chaveamentoCorreto = false;
//        if (vetorPiChav[contador].casoManobra == 2) {//as duas chaves retornaram ao estado inicial
//            chaveamentoCorreto = removeChaveEstadoInicialCaso2Modificada(contador, &vetorPiChav, &(*matrizPIChav), &(*configuracoesChav), &idConfiguracao, totalConfiguracoes, &idNovoAncestral, numeroTrafos, grafoSetoresParam);
//        } else {
//            if (vetorPiChav[contador].casoManobra == 3) { //somente uma das chaves retornou ao estadoInicial
//                chaveamentoCorreto = removeChaveEstadoInicialCaso3Modificada(&vetorPiChav, contador, &(*matrizPIChav), &(*configuracoesChav), &idConfiguracao, totalConfiguracoes, &idNovoAncestral, rnpSetoresParam, grafoSetoresParam);
//            }
//        }
//        if (chaveamentoCorreto) {
//            //localiza no vetorPI a configuracao que tem como ancestral a configuracao que apresentou chave repetida e atualiza para ter como ancestral a configuracao resultante do novo chaveamento
//            indice1 = contador + 1;
//            while (indice1 <= totalConfiguracoes && vetorPiChav[indice1].idAncestral != contador) {
//                indice1++;
//            }
//            if (contador == totalConfiguracoes) {
//                idConfiguracaoParam[0] = idNovoAncestral;
//            }
//            if (indice1 <= totalConfiguracoes) {
//                vetorPiChav[indice1].idAncestral = idNovoAncestral;
//            }
//        }
//    }
//
//    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam[0], &totalConfiguracoes);
//    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiChav, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
//    if (numeroConfiguracoesFalta > 1) {
//        idConfiguracao++;
//        copiaDadosVetorPIMultiplasFaltas(vetorPiChav, vetorPiChav, idConfiguracao, numeroConfiguracoesFalta, indicesFalta);
//        //replica a primeira configuração obtida na etapa de restabelecimento
//        indiceInd = indicesFalta[numeroConfiguracoesFalta - 1];
//        // copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
//        vetorPiChav[idConfiguracao].idAncestral = 0;
//        copiaIndividuoMelhorada(configuracoesParam, (*configuracoesChav), indiceInd, idConfiguracao, (*matrizPIChav), rnpSetoresParam, numeroBarrasParam, numeroTrafos);
//        //aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
//        for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
//            indiceInd = indiceConfiguracoes[contador];
//            idConfiguracao++;
//            copiaDadosVetorPIModificada(vetorPiChav, vetorPiChav, indiceInd, idConfiguracao);
//            vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//            noP = vetorPiChav[indiceInd].nos[0].p;
//            noA = vetorPiChav[indiceInd].nos[0].a;
//            noR = vetorPiChav[indiceInd].nos[0].r;
//            vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
//            colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noA, idConfiguracao - 1);
//            indiceA = (*matrizPIChav)[noA].colunas[colunaPI].posicao;
//            rnpA = (*matrizPIChav)[noA].colunas[colunaPI].idRNP;
//            colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noP, idConfiguracao - 1);
//            indiceP = (*matrizPIChav)[noP].colunas[colunaPI].posicao;
//            rnpP = (*matrizPIChav)[noP].colunas[colunaPI].idRNP;
//            colunaPI = retornaColunaPi((*matrizPIChav), vetorPiChav, noR, idConfiguracao - 1);
//            indiceR = (*matrizPIChav)[noR].colunas[colunaPI].posicao;
//
//            if(rnpP >= (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador LRO
//    			rnpP = rnpP - (*configuracoesChav)[idConfiguracao - 1].numeroRNP;
//    	        indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//    	        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//    			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos+tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//    			alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP].numeroNos-tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP]);
//    			//Obtém a nova rnp de origem, sem a subárvore podada
//    			constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], &(*configuracoesChav)[idConfiguracao].rnpFicticia[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP + (*configuracoesChav)[idConfiguracao - 1].numeroRNP);
//    			//Obtém a nova rnp de destino, inserindo a subárvore podada
//    			constroiRNPDestinoCAO((*configuracoesChav)[idConfiguracao - 1].rnpFicticia[rnpP], (*configuracoesChav)[idConfiguracao - 1].rnp[rnpA], &(*configuracoesChav)[idConfiguracao].rnp[rnpA], indiceP, indiceL,indiceR, indiceA, (*matrizPIChav), idConfiguracao, rnpA);
//
//    			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++ ){
//    				if (indice1 != rnpA)
//    					(*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//    			}
//    			for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//    				if (indice1 != rnpP)
//    					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//    			}
//    			(*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//
//
//            }
//    		else{
//    			if(rnpP < (*configuracoesChav)[idConfiguracao - 1].numeroRNP && rnpA < (*configuracoesChav)[idConfiguracao - 1].numeroRNP){//Operador PAO ou CAO
//					indiceL = limiteSubArvore((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], indiceP); //determinação do intervalo correspondente a subárvore do nó P na RNP
//    		        tamanhoSubarvore = indiceL - indiceP + 1; //calcula o tamanho da subárvore podada
//
//    	            alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpA].numeroNos + tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpA]);
//    	            alocaRNP((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP].numeroNos - tamanhoSubarvore, &(*configuracoesChav)[idConfiguracao].rnp[rnpP]);
//
//    	            constroiRNPOrigem((*configuracoesChav)[idConfiguracao - 1].rnp[rnpP], &(*configuracoesChav)[idConfiguracao].rnp[rnpP], indiceP, indiceL, (*matrizPIChav), idConfiguracao, rnpP);
//    	            constroiRNPDestino((*configuracoesChav), rnpP, rnpA, indiceL, indiceP, indiceA, indiceR, (*matrizPIChav), idConfiguracao - 1, &(*configuracoesChav)[idConfiguracao].rnp[rnpA], idConfiguracao);
//
//    	            for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNP; indice1++) {
//    	                if (indice1 != rnpP && indice1 != rnpA)
//    	                    (*configuracoesChav)[idConfiguracao].rnp[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnp[indice1];
//    	            }
//    		        (*configuracoesChav)[idConfiguracao].numeroRNPFicticia = (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia;
//    				for (indice1 = 0; indice1 < (*configuracoesChav)[idConfiguracao - 1].numeroRNPFicticia; indice1++ ){
//    					(*configuracoesChav)[idConfiguracao].rnpFicticia[indice1] = (*configuracoesChav)[idConfiguracao - 1].rnpFicticia[indice1];
//    				}
//    			}
//
//    		}
//        }
//
//        idConfiguracaoParam[0] = idConfiguracao;
//    }
//
//    if(indicesFalta != NULL){
//    	free(indicesFalta); indicesFalta = 0;}
//    free(indiceConfiguracoes); indiceConfiguracoes = 0;
//
//    //return vetorPiChav;
//}

/**
 * Por Leandro: consiste na função "removeChavesEstadoInicialModificada()" modificada para:
 * a) realizar a desalocação de todos a memória alocada e, assim, evitar vazamento de memória
 * b) evitar que a sequência de chaveamento de um indivíduo seja verificada mais de uma vez;
 * c) verificar se é necessário alocar mais memória para as variáveis "configuracoesChav" e "vetorPiChav". Caso necessário, a alocação de novas posições é realizadas;
 * d) passar dois novos parâmentros, os quais são necessários à realocação de memória
 *
 * Recupera utilizando o VETORPI da RNP todas as configuracões geradas pelo algoritmo evolutivo em tabelas até obter a configuracao final.
 * @param vetorPiParam
 * @param idConfiguracaoParam
 * @param totalConfiguracoes
 * @return
 */
VETORPI *removeChavesEstadoInicialModificadaV4(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam,
		  long int *idConfiguracaoParam, int numeroTrafos, int numeroSetoresFalta, LISTACHAVES *listaChavesParam, int **rnpANovaIdConfiguracaoParam, long int idPrimeiraConfiguracaoParam, int numeroTrafosParam) {
    int totalConfiguracoes, contador, somaChaveamentos = 0;
    long int indiceInd;
    long int *indiceConfiguracoes;// = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
    VETORPI *vetorPiChav;
    CONFIGURACAO *configuracoesChav;
    MATRIZPI *matrizPIChav;
    int tamanhoAlocacao, idConfiguracao = 0, indice;
    long int noP, noA, colunaPI, idUltimaConfiguracaoCopiada = 0, idUltimaConfiguracao = 0, idConfiguracaoParamInicial = idConfiguracaoParam[0];
    int indice1, rnpP, rnpA;
    int idNovoAncestral;
    long int *indicesFalta;
    int numeroConfiguracoesFalta;
    BOOL chaveamentoCorreto = false;


//PARTE 1 - INICIAlIZAÇÃO DE VARIÁVEIS E EXECUÇÃO DE PROCEDIMENTOS PRÉVIOS À RERIFICAÇÃO E EXCLUSÃO DE CHAVES QUE RETORNARAM AO SEU ESTADO INICIAL

    //soma o total de pares de chaveamento para obter a configuração no AEMT
    // printf("totalConfiguracoes %d \n", totalConfiguracoes);
    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiParam, idConfiguracaoParam[0], &totalConfiguracoes);
    for (contador = 0; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        somaChaveamentos += vetorPiParam[indiceInd].numeroManobras;
    }
    tamanhoAlocacao = somaChaveamentos * (somaChaveamentos * 2 - (configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasAutomaticas + configuracoesParam[idConfiguracaoParam[0]].objetivo.manobrasManuais)) + 1;
//    tamanhoAlocacao = somaChaveamentos * somaChaveamentos;
    //aloca o vetorPi, MatrizPI e Vetor de configurações para realizar o processo de remoção de chaves repetidas
    inicializaVetorPi(tamanhoAlocacao, &vetorPiChav);
    inicializaMatrizPI(grafoSetoresParam, &matrizPIChav, (tamanhoAlocacao/0.2), numeroSetores + configuracoesParam[idConfiguracaoParamInicial].numeroRNPFicticia);
    configuracoesChav = alocaIndividuoModificada(configuracoesParam[idConfiguracaoParam[0]].numeroRNP, configuracoesParam[idConfiguracaoParam[0]].numeroRNPFicticia, idConfiguracao, tamanhoAlocacao, numeroTrafos, numeroSetores); //Por Leandro

    //copia os ponteiros da configuração inicial pré falta para iniciar a montagem auxiliar das soluções do AEMT para realizar a redução de chaves
    copiaPonteirosIndividuo(configuracoesParam, configuracoesChav, 0, idConfiguracao, matrizPIChav);

    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiParam, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
    //copia as configuracoes de falta
    for (contador = 0; contador < numeroConfiguracoesFalta; contador++) {
        indiceInd = indicesFalta[contador];
        idConfiguracao++;
        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
        copiaPonteirosIndividuo(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav); //Por Leandro
        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
    }

    //Em vez de aplica os pares de manobras para modificações na floresta e obtenção da configuração final, copia os PONTEIROS DA configuração final e seus ancestais para "configuracaoChav"
    for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
        indiceInd = indiceConfiguracoes[contador];
        idConfiguracao++;
        copiaDadosVetorPI(vetorPiParam, vetorPiChav, indiceInd, idConfiguracao);
        copiaPonteirosIndividuo(configuracoesParam, configuracoesChav, indiceInd, idConfiguracao, matrizPIChav); //Por Leandro
        vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
    }

//PARTE 2 - INICIA-SE O PROCESSO DE PROCURA E EXCLUSÃO DE CHAVES QUE RETORNARAM AO SEU ESTADO INICIAL

    //localiza no vetorPi o ancestral que possui chave que retornou ao estado inicial
    idConfiguracaoParam[0] = idConfiguracao;
    idConfiguracao++;
    idUltimaConfiguracaoCopiada = idConfiguracao;

	//Verifica se é necessário alocar mais memória para as variáveis "configuracoesChav" e "vetorPiChav"
  	if(idConfiguracao >= tamanhoAlocacao - 10){ //Se for verdade, então a grande maioria das posições alocadas já foi utilizada. Logo, é necessário alocar mais posições
		realocaVetorPi(tamanhoAlocacao, (tamanhoAlocacao*1.5), &vetorPiChav);
		realocaIndividuo(tamanhoAlocacao, (tamanhoAlocacao*1.5), &configuracoesChav, idPrimeiraConfiguracaoParam, numeroTrafosParam);
		tamanhoAlocacao = tamanhoAlocacao*1.5;
	}

    for (contador = 1; contador <= totalConfiguracoes; contador++) {
        chaveamentoCorreto = false;
        if (vetorPiChav[contador].casoManobra == 2 && vetorPiChav[contador].sequenciaVerificada == false) {//as duas chaves retornaram ao estado inicial //Por Leandro: modificado aqui para evitar que uma sequência seja verificada mais de uma vez
            chaveamentoCorreto = removeChaveEstadoInicialCaso2Modificada(contador, vetorPiChav, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral, numeroTrafos, grafoSetoresParam);
        } else {
            if (vetorPiChav[contador].casoManobra == 3 && vetorPiChav[contador].sequenciaVerificada == false) { //somente uma das chaves retornou ao estadoInicial //Por Leandro: modificado aqui para evitar que uma sequência seja verificada mais de uma vez
                chaveamentoCorreto = removeChaveEstadoInicialCaso3Modificada(vetorPiChav, contador, matrizPIChav, configuracoesChav, &idConfiguracao, totalConfiguracoes, &idNovoAncestral, numeroTrafos, grafoSetoresParam, idConfiguracaoParamInicial);
            }
        }
        if (chaveamentoCorreto) {
            //localiza no vetorPI a configuracao que tem como ancestral a configuracao que apresentou chave repetida e atualiza para ter como ancestral a configuracao resultante do novo chaveamento
            indice1 = contador + 1;
            while (indice1 <= totalConfiguracoes && vetorPiChav[indice1].idAncestral != contador) {
                indice1++;
            }
            if (contador == totalConfiguracoes) {
                idConfiguracaoParam[0] = idNovoAncestral;
            }
            if (indice1 <= totalConfiguracoes) {
                vetorPiChav[indice1].idAncestral = idNovoAncestral;
            }
        }
    }

    //Por Leandro: adicionada esta linha \para marcar que já houve uma tentativa de correçaõ da sequência de chaveamento deste indivíduo
    vetorPiParam[idConfiguracaoParamInicial].sequenciaVerificada = true;

    indiceConfiguracoes = recuperaConfiguracoesIntermediarias(vetorPiChav, idConfiguracaoParam[0], &totalConfiguracoes);
    indicesFalta = recuperaVetorPiMultiplasFaltas(vetorPiChav, numeroSetoresFalta, totalConfiguracoes, indiceConfiguracoes, &numeroConfiguracoesFalta);
    //Este trecho de código a seguir visa executar a mesma tarefa do trecho comentado abaixo, mas de maneira computacionalmente mais eficiente, isto é,
	//sem realizar a alocação e criação de novas RNP, dado que ele visa apenas atualizar o vetorPi e a atualização da variável configuracaoChav não seria utilizado para nada e ela será desalocada na sequência
	if (numeroConfiguracoesFalta > 1) {
		idConfiguracao++;
		copiaDadosVetorPIMultiplasFaltas(vetorPiChav, vetorPiChav, idConfiguracao, numeroConfiguracoesFalta, indicesFalta);
		//replica a primeira configuração obtida na etapa de restabelecimento
		indiceInd = indicesFalta[numeroConfiguracoesFalta - 1];
		vetorPiChav[idConfiguracao].idAncestral = 0;
		//aplica os pares de manobras para obtenção da configuração final realizadas pelo AEMT.
		for (contador = numeroConfiguracoesFalta; contador < totalConfiguracoes; contador++) {
			indiceInd = indiceConfiguracoes[contador];
			idConfiguracao++;
			copiaDadosVetorPI(vetorPiChav, vetorPiChav, indiceInd, idConfiguracao);
			vetorPiChav[idConfiguracao].idAncestral = idConfiguracao - 1;
		}
		idConfiguracaoParam[0] = idConfiguracao;
	}

//PARTE 3 - DETERMINAÇÃO DA ÁRVORE DESTINO NAS OPERAÇÕES QUE GERARAM A ÚLTIMA CONFIGURAÇÃO OBTIDA NOS PASSOS ANTERIORES
//NOTA: esta "PARTE 3" não seja necessária para a removação das chaves que retornaram ao seu estado inicial, mas ela
// permitirá a obtenção de uma informação que será necessária para a execução da função "corrigeSequenciaChaveamento()",
// a qual atualizará a lista de RNPs em Falta (lista das RNPs envolvidas na ocorrência da falta ou que receberam setores por transferência)
// do último individuo gerado no processo evolutivo (aquele que teve chaves repetida e que cuja sequência de chaveamento está sendo corrigida).
// P.S.: esta informação não pode ser obtida fora desta função atual porque a "matrizPIChav" é desconhecida fora dela

	(*rnpANovaIdConfiguracaoParam) = malloc(vetorPiChav[idConfiguracaoParam[0]].numeroManobras*sizeof(int));
	for(contador = 0; contador < vetorPiChav[idConfiguracaoParam[0]].numeroManobras; contador++){
		noA = vetorPiChav[idConfiguracaoParam[0]].nos[contador].a;
		if(noA > 0){
			colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, vetorPiChav[idConfiguracaoParam[0]].idAncestral);
			(*rnpANovaIdConfiguracaoParam)[contador] = matrizPIChav[noA].colunas[colunaPI].idRNP;
		}
		else
			(*rnpANovaIdConfiguracaoParam)[contador] = -1;
	}

//PARTE 4 - LIBERAÇÃO DA MEMÓRIA ALOCADA

    idUltimaConfiguracao = idConfiguracao;
    //Desaloca os NÓS das RNPs que foram alocadas. Somente algumas configurações tiverem memória alocada para armazenamento de NÓS de RNPs Origem e Destinos. Estes comandos determinam as RNPs nas quais isto ocorreu e desaloca os seus NÓS
    for(contador = idUltimaConfiguracaoCopiada; contador < idUltimaConfiguracao; contador++){
		idConfiguracao = configuracoesChav[contador].idConfiguracao;
		indice = vetorPiChav[idConfiguracao].numeroManobras - 1;

		noP = vetorPiChav[idConfiguracao].nos[indice].p;
		colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noP, vetorPiChav[idConfiguracao].idAncestral);
		rnpP = matrizPIChav[noP].colunas[colunaPI].idRNP;

		noA = vetorPiChav[idConfiguracao].nos[indice].a;
		colunaPI = retornaColunaPi(matrizPIChav, vetorPiChav, noA, vetorPiChav[idConfiguracao].idAncestral);
		rnpA = matrizPIChav[noA].colunas[colunaPI].idRNP;

		if(rnpP >= configuracoesChav[idConfiguracao].numeroRNP && rnpA < configuracoesChav[idConfiguracao].numeroRNP){//Operador LRO
			rnpP = rnpP - configuracoesChav[idConfiguracao].numeroRNP;
			if(configuracoesChav[idConfiguracao].rnpFicticia[rnpP].nos != NULL){
				free(configuracoesChav[idConfiguracao].rnpFicticia[rnpP].nos);
				configuracoesChav[idConfiguracao].rnpFicticia[rnpP].nos = NULL;
			}
		}
		else{
			if(rnpP < configuracoesChav[idConfiguracao].numeroRNP && rnpA < configuracoesChav[idConfiguracao].numeroRNP){//Operador PAO ou CAO
				if(configuracoesChav[idConfiguracao].rnp[rnpP].nos != NULL){
					free(configuracoesChav[idConfiguracao].rnp[rnpP].nos);
					configuracoesChav[idConfiguracao].rnp[rnpP].nos = NULL;
				}
			}
		}
		free(configuracoesChav[idConfiguracao].rnp[rnpA].nos);
		(configuracoesChav)[idConfiguracao].rnp[rnpA].nos = NULL;
    }

    //Todas as configurações alocadas tiveram memória um vetor de RNPs reais e um de RNPs Fictícias alocados. Estes comandos desaloca-os.
    for(idConfiguracao = 0; idConfiguracao < tamanhoAlocacao; idConfiguracao++){

    	if(idConfiguracao < idUltimaConfiguracaoCopiada){ //Estas configurações tiveram ponteiros de RNP copiados para elas. Logo, a fim não desalocar a memória para qual apontaram (ela é necessária), eles são apontados para NULO antes de serem desalocados
    	    for(indice1 = 0; indice1 < configuracoesChav[idConfiguracao].numeroRNP; indice1++)
    	    	configuracoesChav[idConfiguracao].rnp[indice1].nos = NULL;
    	    for(indice1 = 0; indice1 < configuracoesChav[idConfiguracao].numeroRNPFicticia; indice1++)
    	    	configuracoesChav[idConfiguracao].rnpFicticia[indice1].nos = NULL;
    	}

    	if(configuracoesChav[idConfiguracao].rnp != NULL && configuracoesChav[idConfiguracao].numeroRNP > 0){
			free(configuracoesChav[idConfiguracao].rnp);
			configuracoesChav[idConfiguracao].rnp = NULL;
    	}
    	if(configuracoesChav[idConfiguracao].rnpFicticia != NULL && configuracoesChav[idConfiguracao].numeroRNPFicticia > 0){
			free(configuracoesChav[idConfiguracao].rnpFicticia);
			configuracoesChav[idConfiguracao].rnpFicticia = NULL;
    	}
    	if(configuracoesChav[idConfiguracao].idRnpFalta != NULL && configuracoesChav[idConfiguracao].numeroRNPsFalta > 0){
			free(configuracoesChav[idConfiguracao].idRnpFalta);
			configuracoesChav[idConfiguracao].idRnpFalta = NULL;
    	}
//    	if(configuracoesChav[idConfiguracao].ranqueamentoRnpsFicticias != NULL){
//			free(configuracoesChav[idConfiguracao].ranqueamentoRnpsFicticias);
//			configuracoesChav[idConfiguracao].ranqueamentoRnpsFicticias = NULL;
//    	}
    	if(configuracoesChav[idConfiguracao].objetivo.potenciaTrafo != NULL){
    		free(configuracoesChav[idConfiguracao].objetivo.potenciaTrafo);
    		configuracoesChav[idConfiguracao].objetivo.potenciaTrafo = NULL;
    	}
    }
	free(configuracoesChav);
	configuracoesChav = NULL;

    desalocaMatrizPI(matrizPIChav, numeroSetores + configuracoesParam[idConfiguracaoParamInicial].numeroRNPFicticia);
    free(matrizPIChav);
    matrizPIChav = NULL;

    return vetorPiChav;
}

/* Por Leandro
 * Descrição: por meio da sequência de chaveamento corrigida pela exclusão das chaves que retornaram ao seu estado inicial
 * a trinca de nos p, r e a, e, consequentemente, o identificador da árvore destino, que geraram o novo indivíduo podem ter sido modificados.
 * Logo, a lista de RNPs envolvidas no problema torna-se diferente.
 * Portanto, esta função visa atualizar a lista de RNPs da heurística que restringe a região do problema.
 *
 * @param configuracaoParam é a nova configuração ou indivíduo gerado, e que teve a sua sequência de chaveamento corrigida
 * @param rnpAntigaParam é RNP destino anterior a correção da sequência de chaveamento
 * @param rnpNovaParam é RNP destino posterior a correção da sequência de chaveamento
 */
void atualizaListaRNPsFalta(CONFIGURACAO *configuracaoParam, int rnpAntigaParam, int rnpNovaParam, BOOL *flagListaRNPsFaltaAtualizadaParam){
	int contador;
	flagListaRNPsFaltaAtualizadaParam[0] = false;

	if(rnpNovaParam >= configuracaoParam[0].numeroRNP + configuracaoParam[0].numeroRNPFicticia)
		printf("\n%d", rnpNovaParam);

	if(rnpNovaParam >= 0){
		if(rnpAntigaParam != rnpNovaParam){ //Se as duas rnp são iguais, então nenhuma atualização é necessária
			flagListaRNPsFaltaAtualizadaParam[0] = true;
			contador = 0;
			while (contador < configuracaoParam[0].numeroRNPsFalta && configuracaoParam[0].idRnpFalta[contador] != rnpAntigaParam)     	//Procura pela posição em que está salva a "rnpAntigaParam"
				contador++;

			if(contador < configuracaoParam[0].numeroRNPsFalta)         //Salva a "rnpNovaParam" na posição em que estava a "rnpAntigaParam"
				configuracaoParam[0].idRnpFalta[contador] = rnpNovaParam;
			else{
				if(contador == configuracaoParam[0].numeroRNPsFalta){ //Se, por algum motivo, a "rnpAntigaParam" não estiver salva, então adiciona "rnpNovaParam" à lista
					configuracaoParam[0].idRnpFalta[contador] = rnpNovaParam;
					configuracaoParam[0].numeroRNPsFalta++;
				}
			}
		}
	}
}

/* Por Leandro:
 * Em virtude da correção de sequência de chavemento, a lista das RNPs da Heurística Região do Problema pode ser modificada pela
 * inserção ou substituição de RNPs. Todavia, os valores de máximo carregamento de rede, máximo carregamento de transformador e queda
 * máxima de tensão são dependentes desta lista. Logo, uma vez que a mesma foi alterada, os valores destes parâmetros precisam ser recalculados.
 * Ademais, o valor da função ponderação e dependente dos valores de máximo carregamento de rede, máximo carregamento de transformador e queda
 * máxima de tensão e também precisa ser recalculada. Esta função realiza tais atualização de valores.
 *
 */
void atualizaValoresRestricoesOperacionaisEFuncaoPonderacao(BOOL todosAlimentadores, CONFIGURACAO *configuracoesParam,
		VETORPI *vetorPiParam, MATRIZPI *matrizPiParam, long int idNovaConfiguracaoParam, DADOSTRAFO *dadosTrafoParam, int numeroTrafosParam,
		int numeroAlimentadoresParam, DADOSALIMENTADOR *dadosAlimentadorParam, int idAntigaConfiguracaoParam){

    long int contador, noMenorTensaoRNP, noMaiorCarregamentoRNP, noP, noR, noA;
    int idRnp, rnpA, rnpP, rnpR, indiceP, indiceR, indiceA;
    double quedaMaxima, menorTensao, VF;

    //Determina os valore de rnpP e rnpA a serem utilizados no cálculo do carregamento de trafo heuristica
	noP = vetorPiParam[idNovaConfiguracaoParam].nos[0].p;
	noR = vetorPiParam[idNovaConfiguracaoParam].nos[0].r;
	noA = vetorPiParam[idNovaConfiguracaoParam].nos[0].a;
	recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAntigaConfiguracaoParam, matrizPiParam, vetorPiParam);
	if(rnpP >= configuracoesParam[idAntigaConfiguracaoParam].numeroRNP)
		rnpP = -1;
	if(rnpA >= configuracoesParam[idAntigaConfiguracaoParam].numeroRNP)
		rnpA = -1;

	//Zera algumas variáveis que serão atualizadas
    configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = 100000;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoTrafo = 0;
    configuracoesParam[idNovaConfiguracaoParam].objetivo.ponderacao = 0;


    //ATUALIZA OS VALORES DE MAXIMO CARREGAMENTO DE REDE, MAX. CAR. DE TRAFO E MÁXIMA QUEDA DE TENSÃO
    for (contador = 0; contador < configuracoesParam[idNovaConfiguracaoParam].numeroRNPsFalta; contador++) {
        idRnp = configuracoesParam[idNovaConfiguracaoParam].idRnpFalta[contador];
        quedaMaxima = 100000;
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[idRnp+1].idTrafo].tensaoReal/sqrt(3);
        menorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.menorTensao;
        noMenorTensaoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMenorTensao;
        noMaiorCarregamentoRNP = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMaiorCarregamentoRede;

        if(quedaMaxima > menorTensao) // encontra a menor tensão do sistema
            quedaMaxima = menorTensao;
        //atualiza a menor tensão do individuo
        if(configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao > menorTensao){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.menorTensao = menorTensao;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMenorTensao = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMenorTensao; //Por Leandro
        }
        quedaMaxima = ((VF - quedaMaxima) / VF)*100;
        if(configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima < quedaMaxima)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.quedaMaxima = quedaMaxima;
        //potencia
        if(configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador < configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.demandaAlimentador)
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorDemandaAlimentador = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.demandaAlimentador;
        //máximo de corrente
        if(configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.maiorCarregamentoRede){
            configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.maiorCarregamentoRede;
            configuracoesParam[idNovaConfiguracaoParam].objetivo.noMaiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.noMaiorCarregamentoRede;//Por Leandro:
            configuracoesParam[idNovaConfiguracaoParam].objetivo.sobrecargaRede = configuracoesParam[idNovaConfiguracaoParam].rnp[idRnp].fitnessRNP.sobrecargaRede;//Por Leandro:
        }
    }

    configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede = configuracoesParam[idNovaConfiguracaoParam].objetivo.maiorCarregamentoRede * 100;
    carregamentoTrafoHeuristicaModificada(dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, configuracoesParam, idNovaConfiguracaoParam, idAntigaConfiguracaoParam, todosAlimentadores, rnpP, rnpA);

    //ATUALIZA O VALOR DE FUNÇÃO PONDERAÇÃO NORMALIZADA
    calculaPonderacao(configuracoesParam, idNovaConfiguracaoParam, VF);
}

/* Por Leandro
 * Descrição: esta função adiciona uma coluna para os nós que serão transferidos após a definição dos novos nós P, R e A.
 *
 */

void insereColunasV1(MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, GRAFOSETORES *grafoSetoresParam){
	int indice,contador, indiceL, rnpP, rnpA, rnpR, indiceP, indiceA, indiceR, posicaoNo, posicao, tamanhoTemporario;
	long int idAncestral, noP, noA, noR, colunaPI, idNo;

	idAncestral = vetorPiParam[idConfiguracaoParam].idAncestral;

	for(contador = 0; contador < vetorPiParam[idConfiguracaoParam].numeroManobras; contador++){
		noP = vetorPiParam[idConfiguracaoParam].nos[contador].p;
		noA = vetorPiParam[idConfiguracaoParam].nos[contador].a;
		noR = vetorPiParam[idConfiguracaoParam].nos[contador].r;

		if(noP > 0 && noA > 0){

			recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);

			//Atualiza a Matriz dos setores que tiveram sua posição e/ou árvore modificados para a geração de "idConfiguracaoParam" a partir de "idAncestral" por meio da sequência de chaveamento corrigida
			if(rnpP >= configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP){ //Operações de Reconexão de Cargas (LRO)

				rnpP = rnpP - configuracoesParam[idAncestral].numeroRNP;
				indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnpFicticia[rnpP], indiceP);

				//Atualiza a Matriz Pi dos Setores que mudaram de Árvore (foram transferidos)
				for(indice = indiceP; indice <= indiceL; indice++){
					idNo = configuracoesParam[idAncestral].rnpFicticia[rnpP].nos[indice].idNo;

					if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
						posicaoNo = -1; //Busca a posição (índice) de "idNo" em "rnpA" de "idConfiguracaoParam"
						for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos && posicaoNo < 0; posicao++){
							if(configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[posicao].idNo == idNo)
								posicaoNo = posicao;
						}
						if(posicaoNo != -1)
							adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
						else{
							printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpA);
							printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
							printf("\n RNP ORIGEM ORIGINAL - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
							imprimeRNP(configuracoesParam[idAncestral].rnpFicticia[rnpP]);
							printf("\n RNP ORIGEM NOVA - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
							imprimeRNP(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP]);

							printf("\n RNP DESTINO ORIGINAL - %d", rnpA);
							imprimeRNP(configuracoesParam[idAncestral].rnp[rnpA]);
							printf("\n RNP DESTINO NOVA - %d", rnpA);
							imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA]);
							//exit(1);
						}
					}
				}

				//Atualiza a Matriz Pi dos Setores que permaneceram na RNP Origem, mas que tiveram sua posição alterada dentro do array da mesma
				for(indice = indiceL + 1; indice < configuracoesParam[idAncestral].rnpFicticia[rnpP].numeroNos; indice++){
					idNo = configuracoesParam[idAncestral].rnpFicticia[rnpP].nos[indice].idNo;

					if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
						posicaoNo = -1; 		//Determina a posição (índice) de "idNo" em "rnpP" de "idConfiguracaoParam"
						for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].numeroNos && posicaoNo < 0; posicao++){
							if(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[posicao].idNo == idNo)
								posicaoNo = posicao;
						}
						if(posicaoNo != -1)
							adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpP + configuracoesParam[idConfiguracaoParam].numeroRNP, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
						else{
							printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpP);
							printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
							printf("\n RNP ORIGEM ORIGINAL - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
							imprimeRNP(configuracoesParam[idAncestral].rnpFicticia[rnpP]);
							printf("\n RNP ORIGEM NOVA - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
							imprimeRNP(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP]);

							printf("\n RNP DESTINO ORIGINAL - %d", rnpA);
							imprimeRNP(configuracoesParam[idAncestral].rnp[rnpA]);
							printf("\n RNP DESTINO NOVA - %d", rnpA);
							imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA]);
							//exit(1);
						}
					}
				}

				//Atualiza a MatrizPi dos Setores da RNP Destino Original que mudaram de posição no array da RNP Destino Nova
				for(indice = indiceA + 1; indice < configuracoesParam[idAncestral].rnp[rnpA].numeroNos; indice++){
					idNo = configuracoesParam[idAncestral].rnp[rnpA].nos[indice].idNo;

					if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
						posicaoNo = -1; //Determina a posição (índice) de "idNo" em "rnpA" de "idConfiguracaoParam"
						for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos && posicaoNo < 0; posicao++){
							if(configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[posicao].idNo == idNo)
								posicaoNo = posicao;
						}
						if(posicaoNo != -1)
							adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
						else{
							printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpA);
							printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
							printf("\n RNP ORIGEM ORIGINAL - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
							imprimeRNP(configuracoesParam[idAncestral].rnpFicticia[rnpP]);
							printf("\n RNP ORIGEM NOVA - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
							imprimeRNP(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP]);

							printf("\n RNP DESTINO ORIGINAL - %d", rnpA);
							imprimeRNP(configuracoesParam[idAncestral].rnp[rnpA]);
							printf("\n RNP DESTINO NOVA - %d", rnpA);
							imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA]);
							//exit(1);
						}
					}
				}
			}
			else{
				if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP){ //Operações de Transferência de cargas (PAO e CAO)

					indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnp[rnpP], indiceP);

					//Atualiza a Matriz Pi dos Setores que mudaram de Árvore (foram transferidos)
					for(indice = indiceP; indice <= indiceL; indice++){
						idNo = configuracoesParam[idAncestral].rnp[rnpP].nos[indice].idNo;

						if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
							posicaoNo = -1; //Determina a posição (índice) de "idNo" em "rnpA" de "idConfiguracaoParam"
							for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos && posicaoNo < 0; posicao++){
								if(configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[posicao].idNo == idNo)
									posicaoNo = posicao;
							}
							if(posicaoNo != -1)
								adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
							else{
								printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpA);
								printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
								printf("\n RNP ORIGEM ORIGINAL - %d", rnpP);
								imprimeRNP(configuracoesParam[idAncestral].rnp[rnpP]);
								printf("\n RNP ORIGEM NOVA - %d", rnpP);
								imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP]);

								printf("\n RNP DESTINO ORIGINAL - %d", rnpA);
								imprimeRNP(configuracoesParam[idAncestral].rnp[rnpA]);
								printf("\n RNP DESTINO NOVA - %d", rnpA);
								imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA]);
								//exit(1);
							}
						}
					}

					//Atualiza a Matriz Pi dos Setores que permaneceram na RNP Origem, mas que tiveram sua posição alterada dentro do array da mesma
					for(indice = indiceL + 1; indice < configuracoesParam[idAncestral].rnp[rnpP].numeroNos; indice++){
						idNo = configuracoesParam[idAncestral].rnp[rnpP].nos[indice].idNo;

						if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
							posicaoNo = -1;		//Determina a posição (índice) de "idNo" em "rnpP" de "idConfiguracaoParam"
							for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && posicaoNo < 0; posicao++){
								if(configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[posicao].idNo == idNo)
									posicaoNo = posicao;
							}
							if(posicaoNo != -1)
								adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpP, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
							else{
								printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpP);
								printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
								printf("\n RNP ORIGEM ORIGINAL - %d", rnpP);
								imprimeRNP(configuracoesParam[idAncestral].rnp[rnpP]);
								printf("\n RNP ORIGEM NOVA - %d", rnpP);
								imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP]);

								printf("\n RNP DESTINO ORIGINAL - %d", rnpA);
								imprimeRNP(configuracoesParam[idAncestral].rnp[rnpA]);
								printf("\n RNP DESTINO NOVA - %d", rnpA);
								imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA]);
								//exit(1);
							}
						}
					}

					//Atualiza a MatrizPi dos Setores da RNP Destino Original que mudaram de posição no array da RNP Destino Nova
					for(indice = indiceA + 1; indice < configuracoesParam[idAncestral].rnp[rnpA].numeroNos; indice++){
						idNo = configuracoesParam[idAncestral].rnp[rnpA].nos[indice].idNo;

						if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
							posicaoNo = -1; //Determina a posição (índice) de "idNo" em "rnpA" de "idConfiguracaoParam"
							for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos && posicaoNo < 0; posicao++){
								if(configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[posicao].idNo == idNo)
									posicaoNo = posicao;
							}
							if(posicaoNo != -1)
								adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
							else{
								printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpA);
								printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
								printf("\n RNP ORIGEM ORIGINAL - %d", rnpP);
								imprimeRNP(configuracoesParam[idAncestral].rnp[rnpP]);
								printf("\n RNP ORIGEM NOVA - %d", rnpP);
								imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP]);

								printf("\n RNP DESTINO ORIGINAL - %d", rnpA);
								imprimeRNP(configuracoesParam[idAncestral].rnp[rnpA]);
								printf("\n RNP DESTINO NOVA - %d", rnpA);
								imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpA]);
								//exit(1);
							}
						}
					}
				}
				else{
					if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA >= configuracoesParam[idAncestral].numeroRNP){ //Operações de corte de carga (transferência de carga para RNPs Fictícias

						rnpA = rnpA - configuracoesParam[idAncestral].numeroRNP;
						indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnp[rnpP], indiceP);

						//Atualiza a Matriz Pi dos Setores que mudaram de Árvore (foram transferidos)
						for(indice = indiceP; indice <= indiceL; indice++){
							idNo = configuracoesParam[idAncestral].rnp[rnpP].nos[indice].idNo;

							if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
								posicaoNo = -1; //Determina a posição (índice) de "idNo" em "rnpA" de "idConfiguracaoParam"
								for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].numeroNos && posicaoNo < 0; posicao++){
									if(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[posicao].idNo == idNo)
										posicaoNo = posicao;
								}
								if(posicaoNo != -1)
									adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA + configuracoesParam[idAncestral].numeroRNP, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
								else{
									printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpA);
									printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
									printf("\n RNP ORIGEM ORIGINAL - %d", rnpP);
									imprimeRNP(configuracoesParam[idAncestral].rnp[rnpP]);
									printf("\n RNP ORIGEM NOVA - %d", rnpP);
									imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP]);

									printf("\n RNP DESTINO ORIGINAL - %d", rnpA + configuracoesParam[idAncestral].numeroRNP);
									imprimeRNP(configuracoesParam[idAncestral].rnpFicticia[rnpA]);
									printf("\n RNP DESTINO NOVA - %d", rnpA + configuracoesParam[idAncestral].numeroRNP);
									imprimeRNP(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA]);
									//exit(1);
								}
							}
						}

						//Atualiza a Matriz Pi dos Setores que permaneceram na RNP Origem, mas que tiveram sua posição alterada dentro do array da mesma
						for(indice = indiceL + 1; indice < configuracoesParam[idAncestral].rnp[rnpP].numeroNos; indice++){
							idNo = configuracoesParam[idAncestral].rnp[rnpP].nos[indice].idNo;

							if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
								posicaoNo = -1; 		//Determina a posição (índice) de "idNo" em "rnpP" de "idConfiguracaoParam"
								for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && posicaoNo < 0; posicao++){
									if(configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[posicao].idNo == idNo)
										posicaoNo = posicao;
								}
								if(posicaoNo != -1)
									adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpP, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
								else{
									printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpP);
									printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
									printf("\n RNP ORIGEM ORIGINAL - %d", rnpP);
									imprimeRNP(configuracoesParam[idAncestral].rnp[rnpP]);
									printf("\n RNP ORIGEM NOVA - %d", rnpP);
									imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP]);

									printf("\n RNP DESTINO ORIGINAL - %d", rnpA + configuracoesParam[idAncestral].numeroRNP);
									imprimeRNP(configuracoesParam[idAncestral].rnpFicticia[rnpA]);
									printf("\n RNP DESTINO NOVA - %d", rnpA + configuracoesParam[idAncestral].numeroRNP);
									imprimeRNP(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA]);
									//exit(1);
								}
							}
						}

						//Atualiza a MatrizPi dos Setores da RNP Destino Original que mudaram de posição no array da RNP Destino Nova
						for(indice = indiceA + 1; indice < configuracoesParam[idAncestral].rnpFicticia[rnpA].numeroNos; indice++){
							idNo = configuracoesParam[idAncestral].rnpFicticia[rnpA].nos[indice].idNo;

							if(grafoSetoresParam[idNo].setorFalta == false){ //Se "idNo" não estiver entre os setores em falta ou entre setores sem possibilidade de reconexão
								posicaoNo = -1; //Determina a posição (índice) de "idNo" em "rnpA" de "idConfiguracaoParam"
								for(posicao = 0; posicao < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].numeroNos && posicaoNo < 0; posicao++){
									if(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[posicao].idNo == idNo)
										posicaoNo = posicao;
								}
								if(posicaoNo != -1)
									adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP, posicaoNo); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
								else{
									printf("\nFunção 'insereColunasV1()'\nO nó %ld não foi encontrado na RNP Destino\nidConfiguracao: %ld\nRNP Origem: %d\nRNP Destino: %d", idNo, idConfiguracaoParam, rnpP, rnpA);
									printf("\n noP: %ld - noR: %ld - noA: %ld \nindiceP: %d - indiceR: %d - indiceA: %d - indiceL: %d", noP, noR, noA, indiceP, indiceR, indiceA, indiceL);
									printf("\n RNP ORIGEM ORIGINAL - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
									imprimeRNP(configuracoesParam[idAncestral].rnp[rnpP]);
									printf("\n RNP ORIGEM NOVA - %d", rnpP + configuracoesParam[idAncestral].numeroRNP);
									imprimeRNP(configuracoesParam[idConfiguracaoParam].rnp[rnpP]);

									printf("\n RNP DESTINO ORIGINAL - %d", rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP);
									imprimeRNP(configuracoesParam[idAncestral].rnpFicticia[rnpA]);
									printf("\n RNP DESTINO NOVA - %d", rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP);
									imprimeRNP(configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA]);
									//exit(1);
								}
							}
						}
					}
				}
			}
		}
	}
}

/* Por Leandro
 * Descrição: esta função adiciona uma coluna para os nós que serão transferidos após a definição dos novos nós P, R e A.
 *
 */

void insereColunasV3(MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam){
	int indice,contador, rnpP, rnpA, indiceP, indiceA, indiceIguais;
	long int idAncestral, noP, noA, colunaPI, idNo;

	idAncestral = vetorPiParam[idConfiguracaoParam].idAncestral;

	for(contador = 0; contador < vetorPiParam[idConfiguracaoParam].numeroManobras; contador++){
		noP = vetorPiParam[idConfiguracaoParam].nos[contador].p;
		noA = vetorPiParam[idConfiguracaoParam].nos[contador].a;

		if(noP > 0 && noA > 0){
			colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, noP, idAncestral);
			indiceP = matrizPiParam[noP].colunas[colunaPI].posicao;
			rnpP = matrizPiParam[noP].colunas[colunaPI].idRNP;

			colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, noA, idAncestral);
			indiceA = matrizPiParam[noA].colunas[colunaPI].posicao;
			rnpA = matrizPiParam[noA].colunas[colunaPI].idRNP;

			//Atualiza a Matriz dos setores que tiveram sua posição e/ou árvore modificados para a geração de "idConfiguracaoParam" a partir de "idAncestral" por meio da sequência de chaveamento corrigida
			if(rnpP >= configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP){ //Operações de Reconexão de Cargas (LRO)

				rnpP = rnpP - configuracoesParam[idConfiguracaoParam].numeroRNP;
				//Compara a RNP ORIGEM Antiga com a RNP ORIGEM Nova e determina até ponto elas são iguais
				indiceIguais = 0;
				for(indice = 0; indice < configuracoesParam[idAncestral].rnpFicticia[rnpP].numeroNos && indice < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].numeroNos && indiceIguais == 0; indice++){
					if(!(configuracoesParam[idAncestral].rnpFicticia[rnpP].nos[indice].idNo         == configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice].idNo &&
					     configuracoesParam[idAncestral].rnpFicticia[rnpP].nos[indice].profundidade == configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice].profundidade))
						indiceIguais = indice;
				}
				if(indiceIguais > 0)
					for(indice = indiceIguais; indice < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].numeroNos; indice++){
						idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpP].nos[indice].idNo;
						adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpP + configuracoesParam[idConfiguracaoParam].numeroRNP, indice); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
					}

				//Compara a RNP DESTINO Antiga com a RNP DESTINO Nova e determina até ponto elas são iguais
				indiceIguais = 0;
				for(indice = 0; indice < configuracoesParam[idAncestral].rnp[rnpA].numeroNos && indice < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos && indiceIguais == 0; indice++){
					if(!(configuracoesParam[idAncestral].rnp[rnpA].nos[indice].idNo         == configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice].idNo &&
					     configuracoesParam[idAncestral].rnp[rnpA].nos[indice].profundidade == configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice].profundidade))
						indiceIguais = indice;
				}
				if(indiceIguais > 0)
					for(indice = indiceIguais; indice < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos; indice++){
						idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice].idNo;
						adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA, indice); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
					}

			}
			else{
				if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP){ //Operações de Transferência de cargas (PAO e CAO)

					//Compara a RNP ORIGEM Antiga com a RNP ORIGEM Nova e determina até ponto elas são iguais
					indiceIguais = 0;
					for(indice = 0; indice < configuracoesParam[idAncestral].rnp[rnpP].numeroNos && indice < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && indiceIguais == 0; indice++){
						if(!(configuracoesParam[idAncestral].rnp[rnpP].nos[indice].idNo         == configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].idNo &&
						     configuracoesParam[idAncestral].rnp[rnpP].nos[indice].profundidade == configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].profundidade))
							indiceIguais = indice;
					}
					if(indiceIguais > 0)
						for(indice = indiceIguais; indice < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos; indice++){
							idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].idNo;
							adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpP, indice); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
						}

					//Compara a RNP DESTINO Antiga com a RNP DESTINO Nova e determina até ponto elas são iguais
					indiceIguais = 0;
					for(indice = 0; indice < configuracoesParam[idAncestral].rnp[rnpA].numeroNos && indice < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos && indiceIguais == 0; indice++){
						if(!(configuracoesParam[idAncestral].rnp[rnpA].nos[indice].idNo         == configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice].idNo &&
						     configuracoesParam[idAncestral].rnp[rnpA].nos[indice].profundidade == configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice].profundidade))
							indiceIguais = indice;
					}
					if(indiceIguais > 0)
						for(indice = indiceIguais; indice < configuracoesParam[idConfiguracaoParam].rnp[rnpA].numeroNos; indice++){
							idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpA].nos[indice].idNo;
							adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA, indice); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
						}
				}
				else{
					if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA >= configuracoesParam[idAncestral].numeroRNP){ //Operações de corte de carga (transferência de carga para RNPs Fictícias

						rnpA = rnpA - configuracoesParam[idConfiguracaoParam].numeroRNP;

						//Compara a RNP ORIGEM Antiga com a RNP ORIGEM Nova e determina até ponto elas são iguais
						indiceIguais = 0;
						for(indice = 0; indice < configuracoesParam[idAncestral].rnp[rnpP].numeroNos && indice < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos && indiceIguais == 0; indice++){
							if(!(configuracoesParam[idAncestral].rnp[rnpP].nos[indice].idNo         == configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].idNo &&
							     configuracoesParam[idAncestral].rnp[rnpP].nos[indice].profundidade == configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].profundidade))
								indiceIguais = indice;
						}
						if(indiceIguais > 0)
							for(indice = indiceIguais; indice < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos; indice++){
								idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice].idNo;
								adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpP, indice); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
							}

						//Compara a RNP DESTINO Antiga com a RNP DESTINO Nova e determina até ponto elas são iguais
						indiceIguais = 0;
						for(indice = 0; indice < configuracoesParam[idAncestral].rnpFicticia[rnpA].numeroNos && indice < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].numeroNos && indiceIguais == 0; indice++){
							if(!(configuracoesParam[idAncestral].rnpFicticia[rnpA].nos[indice].idNo         == configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice].idNo &&
							     configuracoesParam[idAncestral].rnpFicticia[rnpA].nos[indice].profundidade == configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice].profundidade))
								indiceIguais = indice;
						}
						if(indiceIguais > 0)
							for(indice = indiceIguais; indice < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].numeroNos; indice++){
								idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpA].nos[indice].idNo;
								adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpA + configuracoesParam[idConfiguracaoParam].numeroRNP, indice); //Adiciona ou atualiza uma coluna na matriz Pi de "idNo"
							}
					}
				}
			}
		}
	}
}

/*
 * Por Leandro: retira da matriz PI de todos os setores transferidos para a obtenção de "idConfiguracaoParam" a coluna que foi inserida
 * após a criação de deste indivíduo. Isto é feito porque, uma vez que a trinca de PRA de um indivíduo muda após a correção de sua sequência de
 * chaveamento, os nós a serem transferidos para a sua obtenção também muda. Logo, pode ocorrer que alguns nós que foram transferidos pela trinca PRA
 * original de "idConfiguracaoParam" passem a não ser mais transferidos pelo trica PRA atualizada pela exclusão de chaves repetidas.
 * Assim, faz necessário corrigir (excluir colunas) as matrizes Pi dos nós que deixaram de ser transferidos para a obtenção de "idConfiguracaoParam" em
 * virtude da modificação da trinca PRA.
 */
void retiraColunas(MATRIZPI *matrizPiParam,  int *indicePParam, int *indiceAParam, int *rnpPParam, int *rnpAParam, long int idAncestralParam,
		CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, int numeroManobrasParam){
	int indice, contadorChav, indiceP, indiceA, rnpP, rnpA;//, indiceL, tamanhoTemporario;
	long int idNo;

	for(contadorChav = 0; contadorChav < numeroManobrasParam;contadorChav++){
		indiceP = indicePParam[contadorChav];
		indiceA = indiceAParam[contadorChav];
		rnpP = rnpPParam[contadorChav];
		rnpA = rnpAParam[contadorChav];

		if(indiceP > 0 && indiceA >= 0){
			//Exclui da Matriz Pi as colunas dos setores que mudaram de posição e/ou de árvore para a obtenção de "idConfiguracaoParam" a partir de "idAncestral"
			if(rnpP >= configuracoesParam[idAncestralParam].numeroRNP && rnpA < configuracoesParam[idAncestralParam].numeroRNP){ //Operações de Reconexão de Cargas (LRO)
				//Para os nós modificados da RNP Origem
				rnpP = rnpP - configuracoesParam[idAncestralParam].numeroRNP;
				for(indice = indiceP; indice < configuracoesParam[idAncestralParam].rnpFicticia[rnpP].numeroNos; indice++){
					idNo = configuracoesParam[idAncestralParam].rnpFicticia[rnpP].nos[indice].idNo;
					excluiColuna(matrizPiParam, idNo, idConfiguracaoParam); //Exclui da matriz Pi a coluna de "idNo" relacionada a configuração "idConfiguracaoParam"
				}

				//Para os nós que mudaram de posição na RNP Destino
				for(indice = indiceA + 1; indice < configuracoesParam[idAncestralParam].rnp[rnpA].numeroNos; indice++){
					idNo = configuracoesParam[idAncestralParam].rnp[rnpA].nos[indice].idNo;
					excluiColuna(matrizPiParam, idNo, idConfiguracaoParam); //Exclui da matriz Pi a coluna de "idNo" relacionada a configuração "idConfiguracaoParam"
				}
			}
			else{
				if(rnpP < configuracoesParam[idAncestralParam].numeroRNP && rnpA < configuracoesParam[idAncestralParam].numeroRNP){ //Operações de Transferência de cargas (PAO e CAO)

					//Para os nós modificados da RNP Origem
					for(indice = indiceP; indice < configuracoesParam[idAncestralParam].rnp[rnpP].numeroNos; indice++){
						idNo = configuracoesParam[idAncestralParam].rnp[rnpP].nos[indice].idNo;
						excluiColuna(matrizPiParam, idNo, idConfiguracaoParam); //Exclui da matriz Pi a coluna de "idNo" relacionada a configuração "idConfiguracaoParam"
					}

					//Para os nós que mudaram de posição na RNP Destino
					for(indice = indiceA + 1; indice < configuracoesParam[idAncestralParam].rnp[rnpA].numeroNos; indice++){
						idNo = configuracoesParam[idAncestralParam].rnp[rnpA].nos[indice].idNo;
						excluiColuna(matrizPiParam, idNo, idConfiguracaoParam); //Exclui da matriz Pi a coluna de "idNo" relacionada a configuração "idConfiguracaoParam"
					}
				}
				else{
					if(rnpP < configuracoesParam[idAncestralParam].numeroRNP && rnpA >= configuracoesParam[idAncestralParam].numeroRNP){ //Operações de corte de carga (transferência de carga para RNPs Fictícias
						//Para os nós modificados da RNP Origem
						for(indice = indiceP; indice < configuracoesParam[idAncestralParam].rnp[rnpP].numeroNos; indice++){
							idNo = configuracoesParam[idAncestralParam].rnp[rnpP].nos[indice].idNo;
							excluiColuna(matrizPiParam, idNo, idConfiguracaoParam); //Exclui da matriz Pi a coluna de "idNo" relacionada a configuração "idConfiguracaoParam"
						}

						//Para os nós que mudaram de posição na RNP Destino
						rnpA = rnpA - configuracoesParam[idAncestralParam].numeroRNP;
						for(indice = indiceA + 1; indice < configuracoesParam[idAncestralParam].rnpFicticia[rnpA].numeroNos; indice++){
							idNo = configuracoesParam[idAncestralParam].rnpFicticia[rnpA].nos[indice].idNo;
							excluiColuna(matrizPiParam, idNo, idConfiguracaoParam); //Exclui da matriz Pi a coluna de "idNo" relacionada a configuração "idConfiguracaoParam"
						}
					}
				}
			}
		}
	}
}

/*Por Leandro:
 * Decrição: esta função percorre os nós de uma Árvore e verifica se as informações salvas na Matriz Pi dos mesmos
 * expressam exatamente as informações deste nó nesta árvore (posição do nó e índice da árvore). Caso haja informações incorretas,
 * as mesmas são corrigidas
 *
 *@param rnpParam é a RNP a ser percorrida
 *@param idConfiguracaoParam é a floresta (configuração) na qual a árvore "rnpParam" será percorrida
 *@param matrizPiParam é a matrizPi
 *@param vetorPiParam é o vetorPi
 *@param configuracoesParam é a variável que armazena as informações de todas as florestas
 */
void atualizaMatrizPiArvore(int rnpParam, long int idConfiguracaoParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam){
	int indice, posicaoPi, rnpPi;
	long int idNo, colunaPi, idConfiguracaoPi;

	if(rnpParam >= 0){
		if(rnpParam >= configuracoesParam[idConfiguracaoParam].numeroRNP){  //Se a RNP é Fictícia
			rnpParam = rnpParam - configuracoesParam[idConfiguracaoParam].numeroRNP;

			for(indice = 1; indice < configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpParam].numeroNos; indice++){
				idNo = configuracoesParam[idConfiguracaoParam].rnpFicticia[rnpParam].nos[indice].idNo;

				//recupera as informações de "idNo" salvas na Matriz Pi
				colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, idNo, idConfiguracaoParam);
				posicaoPi = matrizPiParam[idNo].colunas[colunaPi].posicao;
				rnpPi = matrizPiParam[idNo].colunas[colunaPi].idRNP;
				idConfiguracaoPi = matrizPiParam[idNo].colunas[colunaPi].idConfiguracao;

				//Testa se as informações salvas na Matriz Pi estão incorretas. Se sim, então elas devem ser corrigidas
				if(posicaoPi != indice || rnpPi != (rnpParam + configuracoesParam[idConfiguracaoParam].numeroRNP)){
					if(idConfiguracaoPi == idConfiguracaoParam){ //Se verdade, então já existe uma coluna relativa a "idConfiguracaoParam". Logo, basta apenas corrigir as informações salvas na mesma
						matrizPiParam[idNo].colunas[colunaPi].posicao = indice;
						matrizPiParam[idNo].colunas[colunaPi].idRNP = rnpParam  + configuracoesParam[idConfiguracaoParam].numeroRNP;
					}
					else //Caso as informações salvas na Matriz Pi estejam incorretas E não exista na Matriz Pi uma coluna referente a "idConfiguracaoParam", então adiciona-se uma
						adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpParam + configuracoesParam[idConfiguracaoParam].numeroRNP, indice);
				}
			}
		}
		else{ //Se RNP é Real
			for(indice = 1; indice < configuracoesParam[idConfiguracaoParam].rnp[rnpParam].numeroNos; indice++){
				idNo = configuracoesParam[idConfiguracaoParam].rnp[rnpParam].nos[indice].idNo;

				//recupera as informações de "idNo" salvas na Matriz Pi
				colunaPi = retornaColunaPi(matrizPiParam, vetorPiParam, idNo, idConfiguracaoParam);
				posicaoPi = matrizPiParam[idNo].colunas[colunaPi].posicao;
				rnpPi = matrizPiParam[idNo].colunas[colunaPi].idRNP;
				idConfiguracaoPi = matrizPiParam[idNo].colunas[colunaPi].idConfiguracao;

				//Testa se as informações salvas na Matriz Pi estão incorretas. Se sim, então elas devem ser corrigidas
				if(posicaoPi != indice || rnpPi != rnpParam){
					if(idConfiguracaoPi == idConfiguracaoParam){ //Se verdade, então já existe uma coluna relativa a "idConfiguracaoParam". Logo, basta apenas corrigir as informações salvas na mesma
						matrizPiParam[idNo].colunas[colunaPi].posicao = indice;
						matrizPiParam[idNo].colunas[colunaPi].idRNP = rnpParam;
					}
					else //Caso as informações salvas na Matriz Pi estejam incorretas E não exista na Matriz Pi uma coluna referente a "idConfiguracaoParam", então adiciona-se uma
						adicionaColunaMelhorada(matrizPiParam, idNo, idConfiguracaoParam, rnpParam, indice);
				}
			}
		}
	}
}

/*
 * Por Leandro:
 */
void insereColunasV2(MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam){
	int contador, indice, rnpP, rnpA, rnpR, indiceP, indiceA, indiceR, *rnpsAtualizadas, numeroRNPsAtualizadas;
	long int idAncestral, noP, noA, noR;
	BOOL flagRNPVerificada;

	rnpsAtualizadas = malloc(vetorPiParam[idConfiguracaoParam].numeroManobras * sizeof(int));
	numeroRNPsAtualizadas = 0;

	//Atualiza a Matriz Pi do Nós presentes nas Árvores Origem e Destino em "idConfiguracaoParam"
	idAncestral = vetorPiParam[idConfiguracaoParam].idAncestral;
	for(contador = 0; contador < vetorPiParam[idConfiguracaoParam].numeroManobras; contador++){
		noP = vetorPiParam[idConfiguracaoParam].nos[contador].p;
		noA = vetorPiParam[idConfiguracaoParam].nos[contador].a;
		noR = vetorPiParam[idConfiguracaoParam].nos[contador].r;

		//Determina as árvores origem e destino
		recuperaPosicaoPRAModificada(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);

		//Verifica se a Árvore Origem já foi atualizada
		flagRNPVerificada = false;
		for(indice = 0; indice < numeroRNPsAtualizadas && flagRNPVerificada == false; indice++)
			if(rnpsAtualizadas[indice] == rnpP)
				flagRNPVerificada = true;

		if(flagRNPVerificada == false){
			atualizaMatrizPiArvore(rnpP, idConfiguracaoParam, matrizPiParam, vetorPiParam, configuracoesParam);

			rnpsAtualizadas[numeroRNPsAtualizadas] = rnpP;
			numeroRNPsAtualizadas++;
		}

		//Verifica se a Árvore Destino já foi atualizada
		flagRNPVerificada = false;
		for(indice = 0; indice < numeroRNPsAtualizadas && flagRNPVerificada == false; indice++)
			if(rnpsAtualizadas[indice] == rnpA)
				flagRNPVerificada = true;
		if(flagRNPVerificada == false){
			atualizaMatrizPiArvore(rnpA, idConfiguracaoParam, matrizPiParam, vetorPiParam, configuracoesParam);

			rnpsAtualizadas[numeroRNPsAtualizadas] = rnpA;
			numeroRNPsAtualizadas++;
		}
	}

	free(rnpsAtualizadas);
}



/*
 * Por Leandro: esta função atualiza posições da matriz Pi em virtude da mudança dos setores a serem transferidos, o que, por sua vez, é consequencia da correção (modificação)
 * da sequência de chaveamento. Primeiramente, são excluídas as colunas que foram inseridas nas matrizes PI quando o indivíduo "idConfiguracaoParam" foi gerado por meio da
 * repetição de manobras em chaves. Em seguida, são inseridas colunas nas matrizes Pi de todos os nós que estarão envolvidos com a geração de "idConfiguracaoParam" por meio
 * da nova sequência de manobras. Estas atualizações são necessárias porque, com a correção da sequência de chaveamento, os nós de Poda (P), reconexão (R) e adjacente (A),
 * bem como o ancestral de "idConfiguracaoParam" são (ou podem ter sido) modificados. Em outras palavras, muda-se a subárvore transferida, embora as topologia da rede mantenha-se a mesma.
 *
 */
void atualizaMatrizPi(MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, GRAFOSETORES *grafoSetoresParam,
		int *indicePAntigoParam, int *indiceAAntigoParam, int *rnpPAntigoParam, int *rnpAAntigoParam, long int idAncestralAntigoParam, int numeroManobrasAntigoParam){

	retiraColunas(matrizPiParam, indicePAntigoParam, indiceAAntigoParam, rnpPAntigoParam, rnpAAntigoParam, idAncestralAntigoParam, configuracoesParam, idConfiguracaoParam, numeroManobrasAntigoParam);
//	insereColunasV1(matrizPiParam, vetorPiParam, configuracoesParam, idConfiguracaoParam, grafoSetoresParam);
	insereColunasV2(matrizPiParam, vetorPiParam, configuracoesParam, idConfiguracaoParam);
}



/* Por Leandro:
 * Motivação e descrição: após a remoção de chaves repetidas da sequência de chaveamento de um indivíudo, a topologia do mesmo mantêm-se sempre a mesma. O que muda são as manobras
 * para a sua obtenção. Todavia, uma vez que a topologia do indivíduo e os arrays de suas RNPs foram obtidos por meio da sequência original e não da sequência corrigida,
 * pode ocorrer que modificações nos arrays realizadas pela sequência original não sejam detectadas na sequência corrigida. Um exemplo disso é o de uma RNP que perdeu setores
 * e depois os recebeu de volta, desfazendo-se a primeira operação. Contudo, ao recebê-lo de volta, pode ter ocorridos que os mesmo tenha passado a possuir indices diferentes dos
 * iniciais. Uma vez que o procedimento para obtenção da sequência corrigida não atuará sobre tais setores, a matriz PI dos mesms não será atualizada com os novos índices deste
 * setores, o que incorrerá em erros quando a posição de tais setores for buscada na matriz Pi (será informado uma posição errada dentro do array).
 * Para evitar este problema, a função a seguir, faz com que sejam exatamente iguais os arrays dos alimentadores que não foram alterados para a obtenção do referido indivíduo
 * a partir do seu ancestral.
 * Caso tenha sido alocada memória para algum destes arrays por meio da sequência original de manobras, a mesma é liberada antes do ponteiros do array ser apontado para o array
 * do indivíduo ancestral
 * @param configuracoesParam
 * @param vetorPiParam
 * @param matrizPiParam
 * @param indiceInd
 */
void atualizaRNPsNaoAlteradas(CONFIGURACAO *configuracoesParam, VETORPI *vetorPiParam, MATRIZPI *matrizPiParam, long int indiceInd){
	long int noP, noR, noA, idAncestral;
	int indiceP, indiceR, indiceA, indiceRNP, *rnpP, *rnpA, rnpR, contadorChav, RNPFicticia;
	BOOL flagRNPsDiferentes, flagAlgumaRNPFicticiaFoiAlterada;

	rnpP = malloc(vetorPiParam[indiceInd].numeroManobras * sizeof(int));
	rnpA = malloc(vetorPiParam[indiceInd].numeroManobras * sizeof(int));

	flagAlgumaRNPFicticiaFoiAlterada = false;

	//1. Determinação do conjunto de árvores origem (rnpP) e destino (rnpA)
	idAncestral = vetorPiParam[indiceInd].idAncestral;
	for(contadorChav = 0; contadorChav < vetorPiParam[indiceInd].numeroManobras; contadorChav++){
		noP = vetorPiParam[indiceInd].nos[contadorChav].p;
		noR = vetorPiParam[indiceInd].nos[contadorChav].r;
		noA = vetorPiParam[indiceInd].nos[contadorChav].a;

		recuperaPosicaoPRAModificada(noP, noA, noR, &rnpP[contadorChav], &rnpA[contadorChav], &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);
		if(rnpP[contadorChav] >= configuracoesParam[indiceInd].numeroRNP || rnpA[contadorChav] >= configuracoesParam[indiceInd].numeroRNP)
			flagAlgumaRNPFicticiaFoiAlterada = true;
	}

	//2. Copia as RNPs REAIS não alteradas e desaloca a memória daquelas não alteradas que tiveram posição alocada
	for(indiceRNP = 0; indiceRNP < configuracoesParam[indiceInd].numeroRNP; indiceRNP++){
		flagRNPsDiferentes = true;
		for(contadorChav = 0; contadorChav < vetorPiParam[indiceInd].numeroManobras && flagRNPsDiferentes == true; contadorChav++){ //Verifica se a RNP "indiceRNP" é uma das RNPs que foram modificadas para obter "indiceInd" após a correção da seq. de chaveamento
			if(indiceRNP == rnpP[contadorChav] || indiceRNP == rnpA[contadorChav]) //Se a RNP "indiceRNP" é uma das RNPs que foram modificadas para obter "indiceInd" após a correção da seq. de chaveamento, então a mesma não deve ter seu ponteiro apontado para a RNP de mesmo índice na config. Ancestral
				flagRNPsDiferentes = false;
//							if(indiceRNP == rnpPAntigo[contadorChav] || indiceRNP == rnpAAntigo[contadorChav)
//								free(configuracoesParam[indiceInd].rnp[indiceRNP].nos);
		}
		if(flagRNPsDiferentes)
			configuracoesParam[indiceInd].rnp[indiceRNP] = configuracoesParam[idAncestral].rnp[indiceRNP];
	}

	//3. Copia as RNPs FICTÍCIAS não alteradas e desaloca a memória daquelas não alteradas que tiveram posição alocada
	if(flagAlgumaRNPFicticiaFoiAlterada){
		for(indiceRNP = 0; indiceRNP < configuracoesParam[indiceInd].numeroRNPFicticia; indiceRNP++){
			RNPFicticia = indiceRNP + configuracoesParam[indiceInd].numeroRNP;

			flagRNPsDiferentes = true;
			for(contadorChav = 0; contadorChav < vetorPiParam[indiceInd].numeroManobras && flagRNPsDiferentes == true; contadorChav++){ //Verifica se a RNP "indiceRNP" é uma das RNPs que foram modificadas para obter "indiceInd" após a correção da seq. de chaveamento
				if(RNPFicticia == rnpP[contadorChav] || RNPFicticia == rnpA[contadorChav]) //Se a RNP "indiceRNP" é uma das RNPs que foram modificadas para obter "indiceInd" após a correção da seq. de chaveamento, então a mesma não deve ter seu ponteiro apontado para a RNP de mesmo índice na config. Ancestral
					flagRNPsDiferentes = false;
			}
			if(flagRNPsDiferentes)
				configuracoesParam[indiceInd].rnpFicticia[indiceRNP] = configuracoesParam[idAncestral].rnpFicticia[indiceRNP];
		}
	}

	free(rnpP); rnpP = NULL;
	free(rnpA); rnpA = NULL;
}

/**
 * Por Leandro: quando há uma ou duas chaves que retornaram ao seu estado inicial para a geração de um novo indivíduo, tal chave apareceria mais de uma vez na sequência
 * de chaveamento. A fim de evitar tais manobras repetitivas, esta função tenta retirar tal(is) chave(s).
 * Primeiramente é obtida uma sequência de manobras sem a presença de chaves repetidas e que respeita a restrição de radialidade após cada par de manobras.
 * Depois, verifica-se as restrições relaxadas em cada um das configurações intermediárias provida pela nova sequência de manobras.
 * Caso, em alguma configuração intermediária, alguma restrição relaxada seja violada, a sequência de manobras original (com chave(s) repetida(s)) volta a
 * ser associada ao novo indivíduo gerado. Caso contrário, a nova sequência é armazenada na variável "configurações" (isto para não seja necessário obtê-la novamente
 * ao final do processo evolutivo no caso de tal indivíduo estar dentre as soluções da fronteira de Pareto.
 *
 * @param grafoSDRParam
 * @param numeroBarras
 * @param dadosAlimentadorSDRParam
 * @param dadosTrafoSDRParam
 * @param maximoCorrente
 * @param Z
 * @param rnpSetoresParam
 * @param SBase
 * @param grafoSetoresParam
 * @param numeroSetores
 * @param setoresFalta
 * @param numeroSetoresFalta
 * @param numeroTrafosParam
 * @param numeroAlimentadoresParam
 * @param indiceRegulador
 * @param dadosRegulador
 * @param vetorPiParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param listaChavesParam
 * @param seed
 */
void corrigeSequenciaChaveamento(GRAFO *grafoSDRParam, long int numeroBarrasParam, DADOSALIMENTADOR *dadosAlimentadorParam, DADOSTRAFO *dadosTrafoParam,
MATRIZMAXCORRENTE *maximoCorrenteParam, MATRIZCOMPLEXA *ZParam, RNPSETORES *rnpSetoresParam, GRAFOSETORES *grafoSetoresParam,
long int numeroSetoresParam, int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam,
int *indiceReguladorParam, DADOSREGULADOR *dadosReguladorParam, VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam,
long int *idConfiguracaoParam, LISTACHAVES *listaChavesParam, ESTADOCHAVE *estadoInicialParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam,
long int *numeroPosicoesAlocadasParam, long int idPrimeiraConfiguracaoParam, BOOL *sequenciaChaveamentoFactivelParam) {

    long int indiceInd, idChaveAberta, idChaveFechada;
    VETORPI *vetorPiOtimizado;
    int totalConfiguracoesOriginal, totalConfiguracoesOtimizado, indice, contadorChav, indice2, indiceConfiguracoesIguais, indiceP, indiceA, indiceR, indiceL, rnpP, rnpR, rnpA;
    int casoManobra, rnpAAntiga, *rnpANovaIdConfiguracao = NULL, numeroNovasConfiguracoesGeradas = 0, numeroManobrasAntigo, *indicePAntigo = NULL, *indiceAAntigo = NULL, *rnpPAntigo = NULL, *rnpAAntigo = NULL, idAncestralAntigo;
    long int *idConfiguracoesOriginal, *idConfiguracoesCorrigido, idConfiguracao1, idConfiguracao2, idConfiguracao, idAncestral, idConfiguracaoCorrigida, noP, noA, noR, colunaPI, idConfiguracaoParamCorrigida;
    BOOL sequenciaIgual, flagManobrasAlivio, estadoInicialCA, estadoInicialCF, remocaoChavesEstadoInicial, flagListaRNPsFaltaAtualizada;

    indiceInd = idConfiguracaoParam[0];  //Esta variável armazenará, para toda esta função, o identificador da configuração "idConfiguracaoParam"
    idConfiguracaoParamCorrigida = idConfiguracaoParam[0]; //Esta variável armazenará, após a execução da função "removeChavesEstadoInicialModificadaV4()", o id da configuração da sequência corrigida que é igual a configuração "idConfiguracaoParam"
    numeroNovasConfiguracoesGeradas = 0;

    sequenciaChaveamentoFactivelParam[0] = true;

    //Procura remover da sequência de chaveamento as chaves que retornaram ao seu estado inicial
    vetorPiOtimizado = removeChavesEstadoInicialModificadaV4(grafoSetoresParam, numeroSetores, vetorPiParam, configuracoesParam, &idConfiguracaoParamCorrigida, numeroTrafosParam,
    		numeroSetoresFalta, listaChavesParam, &rnpANovaIdConfiguracao, idPrimeiraConfiguracaoParam, numeroTrafosParam);

    remocaoChavesEstadoInicial = verificaRemocaoChaves(vetorPiParam, indiceInd, vetorPiOtimizado, idConfiguracaoParamCorrigida);

    if(remocaoChavesEstadoInicial){ //Se houve remoção das chaves que retornaram ao seu estado inicial, deve-se verifica a factibilidade da sequência em relação às restrições relaxadas
    	//Determinar até que ponto a sequência corrigida coincide com a sequência sem correção. Somente a partir deste ponto é que surgirão configurações inéditas, as quais deverão ser avaliadas quanto a adequação das restrições relaxadas

    	idConfiguracoesOriginal = recuperaConfiguracoesIntermediarias(vetorPiParam, indiceInd, &totalConfiguracoesOriginal); //Recupera a relação de ancestralidade Original
        idConfiguracoesCorrigido = recuperaConfiguracoesIntermediarias(vetorPiOtimizado, idConfiguracaoParamCorrigida, &totalConfiguracoesOtimizado); //Recupera a relação de ancestralidade da sequência aquela obtida após a remoção das chaves repetidas

        sequenciaIgual = true;
        indice = 0;
        while(sequenciaIgual && indice < totalConfiguracoesOriginal && indice < totalConfiguracoesOtimizado){ //Localiza até qual configuração intermediária as sequências otimizada e original são iguais
        	idConfiguracao1 = idConfiguracoesOriginal[indice];
        	idConfiguracao2 = idConfiguracoesCorrigido[indice];

        	if(vetorPiParam[idConfiguracao1].numeroManobras == vetorPiOtimizado[idConfiguracao2].numeroManobras){
        		indice2 = 0;
        		while( (indice2 < vetorPiParam[idConfiguracao1].numeroManobras) && sequenciaIgual){
        			if( (vetorPiParam[idConfiguracao1].idChaveAberta[indice2] != vetorPiOtimizado[idConfiguracao2].idChaveAberta[indice2]) || (vetorPiParam[idConfiguracao1].idChaveFechada[indice2] != vetorPiOtimizado[idConfiguracao2].idChaveFechada[indice2]) )
        				sequenciaIgual = false;
        			indice2++;
        		}
        	}
        	else
        		sequenciaIgual = false;
        	indice++;
        }
        indiceConfiguracoesIguais = indice - 2;

        //Algumas configurações são inéditas. Logo, elas serão criadas, salvas e avaliadas
        idConfiguracao = indiceInd;
        sequenciaChaveamentoFactivelParam[0] = true;
        numeroNovasConfiguracoesGeradas = 0;
        for(indice = indiceConfiguracoesIguais + 1; indice < totalConfiguracoesOtimizado && sequenciaChaveamentoFactivelParam[0] == true; indice++){
        	idConfiguracaoCorrigida = idConfiguracoesCorrigido[indice]; //id da configuração em vetorPiOtimizado
        	idConfiguracao++; //id da Configuração em relação aos indivíduos de todo_ o AEMO

        	//Define o ancestral de "idConfiguracao"
        	if(indice == 0)
        		idAncestral = vetorPiParam[idConfiguracoesOriginal[indice]].idAncestral;
			else{
				if(indice == indiceConfiguracoesIguais + 1)
					idAncestral = idConfiguracoesOriginal[indice - 1];
				else
					idAncestral = idConfiguracao - 1;
			}

			if(indice != totalConfiguracoesOtimizado - 1){ //Se verdade, então "idConfiguracao" é uma configuração inédita e intermediária. Logo, por ser inédita, precisa ser obtida e avaliada, o que é feito pelas linhas abaixo

				//Preenche as informações de "idConfiguracao" em "vetorPiParam"
				copiaDadosVetorPIModificada(vetorPiOtimizado, vetorPiParam, idConfiguracaoCorrigida, idConfiguracao);
            	vetorPiParam[idConfiguracao].idAncestral = idAncestral;
            	if(indice == 0 && vetorPiParam[idConfiguracao].numeroManobras > 1){
            		/*Nesta situação uma das chaves envolvidas na correção da sequência, está no primeiro indivíduo da sequência, o qual foi gerado pela Busca Heurística (Exaustiva).
            		 * A reconstrução de um novo primeiro indivíduo da sequência exige uma função especial porque, para a sua obtenção, é excepcionalmente necessário isolar o setor em falta,
            		 * detectar os setores que não podem restaurados e reconectar em alguma árvore os setores possíveis de serem restaurados.*/
            		obtemAvaliaConfiguracaoIsolaRestabelece(configuracoesParam, idConfiguracao, matrizPiParam,
            				vetorPiParam, grafoSetoresParam, listaChavesParam, estadoInicialParam, dadosTrafoParam,
            				numeroTrafosParam, indiceReguladorParam, dadosReguladorParam, dadosAlimentadorParam, rnpSetoresParam,
            				ZParam, maximoCorrenteParam, numeroBarrasParam, grafoSDRParam, sequenciaManobrasAlivioParam);
            	}
            	else{

					for(contadorChav = 0; contadorChav < vetorPiParam[idConfiguracao].numeroManobras; contadorChav++){
						noP = vetorPiParam[idConfiguracao].nos[contadorChav].p;
						noA = vetorPiParam[idConfiguracao].nos[contadorChav].a;
						noR = vetorPiParam[idConfiguracao].nos[contadorChav].r;
						if(contadorChav > 0)
							idAncestral = idConfiguracao; //No caso de uma configuração "idConfiguracao" precisar de mais de um par de manobras para ser obtida, "idAncestral" assume "idConfiguracao" apenas para a execução adequada das modificações necessárias para obter tal configuração ("idConfiguracao")

						recuperaPosicaoPRAModificada(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);

						if(rnpP >= configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP)  //Operador LRO
							indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnpFicticia[rnpP - configuracoesParam[idAncestral].numeroRNP], indiceP);
						else{
							if(rnpP < configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP) //Operador PAO ou CAO
								indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnp[rnpP], indiceP);
						}

						//Verifica se é necessário alocar mais posições para "configuracoesParam" e "vetorPiParam"
						if(idConfiguracao == numeroPosicoesAlocadasParam[0]){ //Se for verdade, então todas as posições alocadas em "configuracoesParam" e "vetorPiParam" já foram utilizadas. Logo, é necessário alocar mais posições
							realocaVetorPi(numeroPosicoesAlocadasParam[0], (numeroPosicoesAlocadasParam[0] + (totalConfiguracoesOtimizado - indice) + 1000), &vetorPiParam);
							realocaIndividuo(numeroPosicoesAlocadasParam[0], (numeroPosicoesAlocadasParam[0] + (totalConfiguracoesOtimizado - indice) + 1000), &configuracoesParam, idPrimeiraConfiguracaoParam, numeroTrafosParam);
							numeroPosicoesAlocadasParam[0] = numeroPosicoesAlocadasParam[0] + (totalConfiguracoesOtimizado - indice) + 1000;
						}

						//Obtém a nova configuração intermediária
						if(rnpP == rnpA)
							obtemConfiguracaoOrigemDestino(configuracoesParam, idAncestral, idConfiguracao, indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPiParam);
						else{
							obtemConfiguracaoModificada(configuracoesParam, idAncestral, idConfiguracao, indiceL, indiceP, indiceA, indiceR, rnpA, rnpP, matrizPiParam);
						}
						//Atualiza a lista de RNPs da Região do problema
						copiaListaRnps(configuracoesParam, idConfiguracao, idAncestral);
						insereRNPLista(&configuracoesParam[idConfiguracao], rnpA);

						//Avalia a Nova Configuração
						idChaveAberta = vetorPiParam[idConfiguracao].idChaveAberta[contadorChav];
						idChaveFechada = vetorPiParam[idConfiguracao].idChaveFechada[contadorChav];
						numeroManobrasChaveamento(configuracoesParam, idChaveAberta, idChaveFechada, listaChavesParam,
								idAncestral, estadoInicialParam, &estadoInicialCA, &estadoInicialCF, idConfiguracao, &casoManobra);
						//Atualiza o vetor Pi com informações determinadas pela função que calcula manobras
						vetorPiParam[idConfiguracao].casoManobra = casoManobra;
						vetorPiParam[idConfiguracao].estadoInicialChaveAberta[contadorChav] = estadoInicialCA;
						vetorPiParam[idConfiguracao].estadoInicialChaveFechada[contadorChav] = estadoInicialCF;


						flagManobrasAlivio = determinaFlagManobrasAlivio(configuracoesParam, matrizPiParam, vetorPiParam, idConfiguracao);
						if(rnpP >= configuracoesParam[idAncestral].numeroRNP && rnpA < configuracoesParam[idAncestral].numeroRNP)  //Se a rnpP for fictícia, então rnpP assume -1 para que o fluxo de carga não seja calculado em tal árvore
							rnpP = -1;
						if(flagManobrasAlivio == true) //Se a nova configuração intermediária foi obtida por meio de transferência de cargas entre alimentadores energizados, calcula a potência ativa transferida (por nível de prioridade de atendimento)
							configuracoesParam[idConfiguracao].potenciaTransferida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idAncestral, indiceP, indiceL, rnpP, rnpSetoresParam, grafoSDRParam);

						avaliaConfiguracaoHeuristicaModificada(false, configuracoesParam, rnpA, rnpP, idConfiguracao,
							dadosTrafoParam, numeroTrafosParam, configuracoesParam[idConfiguracao].numeroRNP,
							indiceReguladorParam, dadosReguladorParam, dadosAlimentadorParam, idAncestral, rnpSetoresParam,
							ZParam, maximoCorrenteParam, numeroBarrasParam, false, grafoSDRParam,
							sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, flagManobrasAlivio);

						if(!verificaFactibilidade(configuracoesParam[idConfiguracao], maxCarregamentoRedeR, maxCarregamentoTrafoR, maxQuedaTensaoR))
							sequenciaChaveamentoFactivelParam[0] = false;
					}
				}
				numeroNovasConfiguracoesGeradas++;
			}
			else{ //Quando "indice" é igual a "totalConfiguracoesOtimizado - 1" significa a a configuração que seria obtida é a própria nova configuração obtida pelo AEMO. Logo, faz-se necessário tão somente atualizar algumas informações do seu vetorPi

				//ATUALIZA O VETOR PI
				// Recupera, antes de atualizar o vetorPi, e salva a trinca PRA que originalmente levou à obtenção informações de "indiceInd", pois
				// esta será utilizada posteriormente para excluir colunas da Matriz Pi dos nós que originalmente foram transferidos para a obtenção de "indiceInd". Se, posteriormente a atualização do par de manobras, tais nós ainda sejam transferidos, então uma nova coluna na Matriz Pi é inserida com informações atualizadas
				indicePAntigo = malloc(vetorPiParam[indiceInd].numeroManobras * sizeof(int));
				indiceAAntigo = malloc(vetorPiParam[indiceInd].numeroManobras * sizeof(int));
				rnpPAntigo    = malloc(vetorPiParam[indiceInd].numeroManobras * sizeof(int));
				rnpAAntigo    = malloc(vetorPiParam[indiceInd].numeroManobras * sizeof(int));
				idAncestralAntigo = vetorPiParam[indiceInd].idAncestral;
				numeroManobrasAntigo = vetorPiParam[indiceInd].numeroManobras;
				for(contadorChav = 0; contadorChav < vetorPiParam[indiceInd].numeroManobras;contadorChav++){
					noP = vetorPiParam[indiceInd].nos[contadorChav].p;
					noR = vetorPiParam[indiceInd].nos[contadorChav].r;
					noA = vetorPiParam[indiceInd].nos[contadorChav].a;
					recuperaPosicaoPRA(noP, noA, noR, &rnpPAntigo[contadorChav], &rnpAAntigo[contadorChav], &rnpR, &indicePAntigo[contadorChav], &indiceR, &indiceAAntigo[contadorChav], idAncestralAntigo, matrizPiParam, vetorPiParam);
				}

            	copiaDadosVetorPIPosicaoJaAlocada(vetorPiOtimizado, vetorPiParam, idConfiguracaoCorrigida, indiceInd); //atualiza o vetor Pi de "indiceInd
    			vetorPiParam[indiceInd].idAncestral = idAncestral; //atualiza o vetor Pi de "indiceInd

    			//ATUALIZA OS ARRAYS DAS RNPs QUE NÃO FORAM MODIFCADAS PARA A OBTENÇÃO DE "indiceInd" PELA SEQUÊNCIA CORRIDA
            	atualizaRNPsNaoAlteradas(configuracoesParam, vetorPiParam, matrizPiParam, indiceInd);

            	//ATUALIZA A MATRIZ PI DOS SETORES ENVOLVIDOS NA GERAÇÃO DE "indiceInd" ANTES E DEPOIS DA CORREÇÃO DA SEQUÊNCIA DE CHAVEAMENTO
    			atualizaMatrizPi(matrizPiParam, vetorPiParam, configuracoesParam, indiceInd, grafoSetoresParam,
            			indicePAntigo, indiceAAntigo, rnpPAntigo, rnpAAntigo, idAncestralAntigo, numeroManobrasAntigo); //Esta função só pode ser chamada após o vetorPiParam tiver sido atualizado

				//ATUALIZA OUTRAS INFORMAÇÕES
				if(idAncestral == 0){
					// SE (a) "indice" é igual "totalConfiguracoesOtimizado - 1" E (b) "idAncestral" é igual a ZERO, então, "indiceInd" é uma
					// configuração que, após a exclusão das manobras repetidas, passou a ser obtida diretamente da configuração pré-falta. Em virtude disto,
					// ao seu tratamento é especial, no que tange ao modo como algumas informações devem ser atualizadas
					// Neste caso, "indiceInd" torna-se igual ao primeiro indivíduo da sequência de chaveamento que possuia antes da exclusão de chaves repetidas

					//ATUALIZA A LISTA DAS RNPs DA HEURÍSTICA REGIÃO DO PROBLEMA
					configuracoesParam[indiceInd].numeroRNPsFalta = configuracoesParam[idConfiguracoesOriginal[0]].numeroRNPsFalta;
					free(configuracoesParam[indiceInd].idRnpFalta);
					configuracoesParam[indiceInd].idRnpFalta = configuracoesParam[idConfiguracoesOriginal[0]].idRnpFalta; //Neste caso, a lista de RNP da região do problema é a mesma do primeiro indivíduo da sequência.

					//ATUALIZA OS VALORES DE MÁXIMO CARREGAMENTO DE REDE, MÁXIMO CAR. DE SUBESTÃO, QUEDA MÁXIMA DE TENSÃO, FUNÇÃO PONDERAÇÃO NORMALIZADA, NUMERO DE MANOBRAS, TEMPO DE EXECUÇÃO DE MANOBRAS E ENERGIA NÃO SUPRIDA DE "indiceInd"
					configuracoesParam[indiceInd].objetivo = configuracoesParam[idConfiguracoesOriginal[0]].objetivo; //Neste caso, os valores serão todos iguais aos do  primeiro indivíduo da sequência de chaveamento anterior a exclusão de chaves repetidas

					//ATUALIZA OS VALORES DE POTÊNCIA TRANSFERIDA (porque aqueles salvos para "indiceInd" foram obtidos considerando as manobras que foram excluídas)
					configuracoesParam[indiceInd].potenciaTransferida = configuracoesParam[idConfiguracoesOriginal[0]].potenciaTransferida; //Neste caso, os valores serão todos iguais aos do  primeiro indivíduo da sequência de chaveamento anterior a exclusão de chaves repetidas
				}
				else{
					//ATUALIZA A LISTA DAS RNPs DA HEURÍSTICA REGIÃO DO PROBLEMA
					for(contadorChav = 0; contadorChav < vetorPiOtimizado[idConfiguracoesCorrigido[totalConfiguracoesOtimizado - 1]].numeroManobras;contadorChav++){
						if(rnpANovaIdConfiguracao[contadorChav] >= 0 && rnpANovaIdConfiguracao[contadorChav] < configuracoesParam[indiceInd].numeroRNP){
							if(contadorChav <  vetorPiParam[indiceInd].numeroManobras){
								noA = vetorPiParam[indiceInd].nos[contadorChav].a;
								colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, noA, vetorPiParam[indiceInd].idAncestral);
								rnpAAntiga = matrizPiParam[noA].colunas[colunaPI].idRNP;
							}
							atualizaListaRNPsFalta(&configuracoesParam[indiceInd], rnpAAntiga, rnpANovaIdConfiguracao[contadorChav], &flagListaRNPsFaltaAtualizada);
						}
					}

					//ATUALIZA OS VALORES DE MÁXIMO CARREGAMENTO DE REDE, MÁXIMO CAR. DE SUBESTÃO, QUEDA MÁXIMA DE TENSÃO E FUNÇÃO PONDERAÇÃO NORMALIZADA DE "indiceInd" (porque estes são dependentes das rnps presentes na lista de RNPS da Heurística Região do Problema, a qual foi atualizada
					if(flagListaRNPsFaltaAtualizada == true)
						atualizaValoresRestricoesOperacionaisEFuncaoPonderacao(false, configuracoesParam, vetorPiParam, matrizPiParam, indiceInd,
								dadosTrafoParam, numeroTrafosParam, numeroAlimentadoresParam, dadosAlimentadorParam, idAncestral);

					//ATUALIZA OS VALORES DE NUMERO DE MANOBRAS EM CHAVES E DE TEMPO NECESSÁRIO PARA EXECUTÁ-LAS (porque existem situações, embora raras, nas quais o par rearranjado é formado por duas chaves que já se encontravam repetidas na sequência de manobras de indiceInd. Nestes casos, a função "removeChavesEstadoInicialModificadaV4()" irá excluir tanto as manobras que se repetiram para a geração de "indiceInd" quanto as chaves repetidas que formaram par rearranjado)
					idChaveAberta = vetorPiParam[indiceInd].idChaveAberta[0];
					idChaveFechada = vetorPiParam[indiceInd].idChaveFechada[0];
					numeroManobrasChaveamento(configuracoesParam, idChaveAberta, idChaveFechada, listaChavesParam,
							idAncestral, estadoInicialParam, &estadoInicialCA, &estadoInicialCF, indiceInd, &casoManobra);
					//Atualiza o vetor Pi com informações determinadas pela função que calcula manobras
					vetorPiParam[indiceInd].casoManobra = casoManobra;
					vetorPiParam[indiceInd].estadoInicialChaveAberta[contadorChav] = estadoInicialCA;
					vetorPiParam[indiceInd].estadoInicialChaveFechada[contadorChav] = estadoInicialCF;

					//ATUALIZAÇÃO DOS VALORES DE POTÊNCIA TRANSFERIDA (porque o par de manobras para a obtenção de "indiceInd" pode ter sido corrigido/modificado)
					flagManobrasAlivio = determinaFlagManobrasAlivio(configuracoesParam, matrizPiParam, vetorPiParam, indiceInd);
					if(flagManobrasAlivio == true){
						noP = vetorPiParam[indiceInd].nos[0].p;
						noR = vetorPiParam[indiceInd].nos[0].r;
						noA = vetorPiParam[indiceInd].nos[0].a;
						recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);
						indiceL = limiteSubArvore(configuracoesParam[idAncestral].rnp[rnpP], indiceP);
						configuracoesParam[indiceInd].potenciaTransferida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridadeV2(configuracoesParam, idAncestral, indiceP, indiceL, rnpP, rnpSetoresParam, grafoSDRParam);
					}
					else{
						configuracoesParam[indiceInd].potenciaTransferida.consumidoresSemPrioridade   	 	   = 0;
						configuracoesParam[indiceInd].potenciaTransferida.consumidoresPrioridadeBaixa 		   = 0;
						configuracoesParam[indiceInd].potenciaTransferida.consumidoresPrioridadeIntermediaria  = 0;
						configuracoesParam[indiceInd].potenciaTransferida.consumidoresPrioridadeAlta 		   = 0;
					}

					//ATUALIZAÇÃO DOS VALORES DE ENERGIA NÃO SUPRIDA (porque o ancestral mudou e, consequentemente, o valor de ENS do Ancestral, assim como pode ter mudado também os valores de potência transferida e de tempo de execução podem ter sido alterados)
					calculaEnergiaAtivaNaoSupridaPorNivelPrioridade(configuracoesParam, indiceInd, idAncestral, sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, flagManobrasAlivio);
				}

            	free(indicePAntigo); indicePAntigo = NULL;
            	free(indiceAAntigo); indiceAAntigo = NULL;
            	free(rnpPAntigo);    rnpPAntigo = NULL;
            	free(rnpAAntigo);    rnpAAntigo = NULL;
            }
        }
        free(idConfiguracoesOriginal); idConfiguracoesOriginal = NULL;
        free(idConfiguracoesCorrigido); idConfiguracoesCorrigido = NULL;
    }

    if(remocaoChavesEstadoInicial == false || sequenciaChaveamentoFactivelParam[0] == false){
// INFORMAÇÃO: Se não foi possível remover as chaves que retornaram ao seu estado inicial OU se, mesmo com a sua remoção, a sequência obtida
// foi infactível (i,e, possui alguma configuração intermediária infactível), então deve-se manter a sequência original (obtida pelo operadores
// PAO, CAO e/ou LRO). Mas, para tal, deve-se corrigir: (a) os número de manobras em chaves e (b) os valores de energia não suprida, dado que na
// obtenção destes assumiu-se que a ausência de chaves repetidas. Uma vez feito isso, a sequência de chaveamento volta a ser factível

    	for(contadorChav = 0; contadorChav < vetorPiParam[indiceInd].numeroManobras;contadorChav++){
			// Correção dos valores de número de manobras e tempo para operação das chaves
			idChaveAberta = vetorPiParam[indiceInd].idChaveAberta[contadorChav];
			idChaveFechada = vetorPiParam[indiceInd].idChaveFechada[contadorChav];
			idAncestral = vetorPiParam[indiceInd].idAncestral;

			numeroManobrasChaveamento(configuracoesParam, idChaveAberta, idChaveFechada, listaChavesParam, idAncestral,
					estadoInicialParam, &estadoInicialCA, &estadoInicialCF, indiceInd, &casoManobra);
			//Atualiza o vetor Pi com informações determinadas pela função que calcula manobras
			vetorPiParam[indiceInd].casoManobra = casoManobra;
			vetorPiParam[indiceInd].estadoInicialChaveAberta[contadorChav] = estadoInicialCA;
			vetorPiParam[indiceInd].estadoInicialChaveFechada[contadorChav] = estadoInicialCF;

			// Correção dos valores de energia não suprida, uma vez que houve alteração no número e no tempo para operação das chaves
			noP = vetorPiParam[indiceInd].nos[contadorChav].p;
			noR = vetorPiParam[indiceInd].nos[contadorChav].r;
			noA = vetorPiParam[indiceInd].nos[contadorChav].a;
			recuperaPosicaoPRA(noP, noA, noR, &rnpP, &rnpA, &rnpR, &indiceP, &indiceR, &indiceA, idAncestral, matrizPiParam, vetorPiParam);
			flagManobrasAlivio = determinaFlagManobrasAlivio(configuracoesParam, matrizPiParam, vetorPiParam, indiceInd);

			calculaEnergiaAtivaNaoSupridaPorNivelPrioridade(configuracoesParam, indiceInd, idAncestral, sequenciaManobrasAlivioParam, listaChavesParam, vetorPiParam, flagManobrasAlivio);

			// Uma vez reinserida na sequência de chaveamento a chave que retornou ao seu estado inicial, a sequência de chaveamento volta
			// a ser factível, pois volta a não possuir violação de restrição relaxada nas configurações intermediárias
			sequenciaChaveamentoFactivelParam[0] = true;
        }
    }

    idConfiguracaoParam[0] = indiceInd + numeroNovasConfiguracoesGeradas;

    if(idConfiguracaoParamCorrigida > 1){
		desalocaVetorPi(idConfiguracaoParamCorrigida, vetorPiOtimizado);
		free(vetorPiOtimizado);
    }
    vetorPiOtimizado = NULL;
}


void verificaSequenciaChaveamento(GRAFO *grafoSDRParam, long int numeroBarras, DADOSALIMENTADOR *dadosAlimentadorSDRParam, DADOSTRAFO *dadosTrafoSDRParam,
        MATRIZMAXCORRENTE *maximoCorrente, MATRIZCOMPLEXA *Z, RNPSETORES *rnpSetoresParam, GRAFOSETORES *grafoSetoresParam, long int numeroSetoresParam,
		int numeroSetoresFalta, int numeroTrafosParam, int numeroAlimentadoresParam, int *indiceRegulador, DADOSREGULADOR *dadosRegulador,
		VETORPI *vetorPiParam, CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, long int *idConfiguracaoParam, LISTACHAVES *listaChavesParam,
		ESTADOCHAVE *estadoInicial, SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivioParam, long int *numeroPosicoesAlocadasParam,
		long int idPrimeiraConfiguracaoParam, BOOL *sequenciaChaveamentoFactivelParam){
	long int idConfiguracaoAuxiliar = idConfiguracaoParam[0];

	sequenciaChaveamentoFactivelParam[0] = true;
	if(vetorPiParam[idConfiguracaoParam[0]].casoManobra == 2 || vetorPiParam[idConfiguracaoParam[0]].casoManobra == 3){ //Verifica se algumas das chaves operadas retornou ao seu estado inicial
//		idConfiguracaoAuxiliar = idConfiguracaoParam[0];
//		mostraSequenciaChaveamentoIndividuo(idConfiguracaoAuxiliar, vetorPiParam, configuracoesParam, sequenciaManobrasAlivioParam);

		corrigeSequenciaChaveamento(grafoSDRParam, numeroBarras, dadosAlimentadorSDRParam, dadosTrafoSDRParam,
		        maximoCorrente, Z, rnpSetoresParam, grafoSetoresParam, numeroSetoresParam, numeroSetoresFalta, numeroTrafosParam, numeroAlimentadoresParam,
		        indiceRegulador, dadosRegulador, vetorPiParam, configuracoesParam, matrizPiParam, &(*idConfiguracaoParam),
				listaChavesParam, estadoInicial, sequenciaManobrasAlivioParam, &numeroPosicoesAlocadasParam[0], idPrimeiraConfiguracaoParam, &(*sequenciaChaveamentoFactivelParam));

//		mostraSequenciaChaveamentoIndividuo(idConfiguracaoAuxiliar, vetorPiParam, configuracoesParam, sequenciaManobrasAlivioParam);

	}
}
