/**
 * @file aemt.c
 * @brief Este arquivo contém a implementação das funcões para manipulação das subpopulações do algoritmo evolutivo multiobjetivo em tabelas.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "aemt.h"
#include "funcoesAuxiliares.h"
#include "data_structures.h"
#include "funcoesProblema.h"



void ordenaIndividuosEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores com Prioridade Alta e Manobras em Chaves Controladas Remotamente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se na última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se na última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores com Prioridade Intermediária e Manobras em Chaves Controladas Remotamente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores com Prioridade Baixa e Manobras em Chaves Controladas Remotamente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores sem Prioridade e Manobras em Chaves Controladas Remotamente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores com Prioridade Alta e Manobras em Chaves Controladas Manualmente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}
/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores com Prioridade Intermediária e Manobras em Chaves Controladas Manualmente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores com Prioridade Baixa e Manobras em Chaves Controladas Manualmente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade < configuracoes[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo 1
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
                        - configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade);
            } else {//utiliza o objetivo 2
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia Não Suprida aos Consumidores sem Prioridade e Manobras em Chaves Controladas Manualmente
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade >= configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade == configuracoesParam[idConfiguracao2].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaNaoSuprida.consumidoresSemPrioridade == configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosCarregamentoRedeVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++){
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {
                if (configuracoes[idConfiguracao1].objetivo.maiorCarregamentoRede < configuracoes[idConfiguracao2].objetivo.maiorCarregamentoRede) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            }
        }
    }
}


void crowndingCarregamentoRedeVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {
    int i, j, k;
    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;

    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosCarregamentoRedeVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Carregamento de Rede
                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede
                        - configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
    indicesOrdenados = NULL; free(indicesOrdenados);
}
void insereConfiguracaoTabelaDominanciaCarregamentoRedeVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ, contadorIndividuosVerificados, numIndividuos, numeroIndividuosUltimaFronteira;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;
							// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede >= configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}
				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingCarregamentoRedeVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				individuosUltimaFronteira = NULL; free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}
						}
					}
				}
			}

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}


void ordenaIndividuosCarregamentoRedeVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.maiorCarregamentoRede < configuracoes[idConfiguracao2].objetivo.maiorCarregamentoRede) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingCarregamentoRedeVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosCarregamentoRedeVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Carregamento de Rede

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede
                        - configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
}

void insereConfiguracaoTabelaDominanciaCarregamentoRedeVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {

    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    //int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;//[populacaoParam[idTabelaParam].numeroIndividuos];
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {
    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			//FRONTEIRAS fronteira[populacaoParam[idTabelaParam].tamanhoMaximo];

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}


        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;


/*
				printf("\n_______________________________________________________________");
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos + 1; contadorI++) {
					printf("\n%d | %f | %d", individuos[contadorI].idConfiguracao, configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.maiorCarregamentoRede, configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.manobrasManuais);
				}
*/



				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não foi salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;

								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede >= configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede
													&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Salva o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingCarregamentoRedeVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            //ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
		            double tmpSol = 1.0e14;
		            int menorDist;
		            long int idConfiguracaoMenorDist;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
/*								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
								}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}
                        	}
                        }

					}
		            //zera as distancias inicialmente
/*		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;*/

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
/*	                        	if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
	                        	}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
			//}
        //}
				free(fronteira);
    	}
    }

    free(individuos);
}


void ordenaIndividuosCarregamentoTrafoVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.maiorCarregamentoTrafo < configuracoes[idConfiguracao2].objetivo.maiorCarregamentoTrafo) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingCarregamentoTrafoVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosCarregamentoTrafoVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Carregamento de Rede

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo
                        - configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
}

void insereConfiguracaoTabelaDominanciaCarregamentoTrafoVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    //int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;//[populacaoParam[idTabelaParam].numeroIndividuos];
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {
    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			//FRONTEIRAS fronteira[populacaoParam[idTabelaParam].tamanhoMaximo];

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}


        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não foi salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;

								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo >= configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo
													&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}


				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Salva o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingCarregamentoTrafoVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            //ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
		            double tmpSol = 1.0e14;
		            int menorDist;
		            long int idConfiguracaoMenorDist;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
/*								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
								}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


                        	}
                        }

					}
		            //zera as distancias inicialmente
/*		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;*/

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
/*	                        	if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
	                        	}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}

	                        }
						}
					}
				}
			//}
        //}
				free(fronteira);
    	}
    }

    free(individuos);
}

void ordenaIndividuosCarregamentoTrafoVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.maiorCarregamentoTrafo < configuracoes[idConfiguracao2].objetivo.maiorCarregamentoTrafo) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingCarregamentoTrafoVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosCarregamentoTrafoVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Carregamento de Rede

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo
                        - configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
}

void insereConfiguracaoTabelaDominanciaCarregamentoTrafoVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    //int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;//[populacaoParam[idTabelaParam].numeroIndividuos];
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {
    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			//FRONTEIRAS fronteira[populacaoParam[idTabelaParam].tamanhoMaximo];

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não foi salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;

								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo >= configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo
													&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}


				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Salva o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}

					crowndingCarregamentoTrafoVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            //ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
		            double tmpSol = 1.0e14;
		            int menorDist;
		            long int idConfiguracaoMenorDist;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
/*								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
								}*/

	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


                        	}
                        }

					}
		            //zera as distancias inicialmente
/*		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;*/

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
/*	                        	if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
	                        	}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
			//}
        //}
				free(fronteira);
    	}
    }

    free(individuos);
}

void ordenaIndividuosQuedaTensaoVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.quedaMaxima < configuracoes[idConfiguracao2].objetivo.quedaMaxima) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingQuedaTensaoVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosQuedaTensaoVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Carregamento de Rede

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.quedaMaxima
                        - configuracoesParam[idConfiguracao2].objetivo.quedaMaxima);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
}

void insereConfiguracaoTabelaDominanciaQuedaTensaoVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    //int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;//[populacaoParam[idTabelaParam].numeroIndividuos];
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {
    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			//FRONTEIRAS fronteira[populacaoParam[idTabelaParam].tamanhoMaximo];

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}


			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não foi salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
							idConfiguracao2 = individuos[contadorJ].idConfiguracao;

							if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
								if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
									if (configuracoesParam[idConfiguracao1].objetivo.quedaMaxima >= configuracoesParam[idConfiguracao2].objetivo.quedaMaxima
											&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
										rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
										if (configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracao2].objetivo.quedaMaxima
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank--;
										}
									}
								}
							}
						}
					}

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}


				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}


			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Salva o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingQuedaTensaoVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				//ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
				double tmpSol = 1.0e14;
				int menorDist;
				long int idConfiguracaoMenorDist;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
/*								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
								populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
								populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
								populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
								populacaoParam[idTabelaParam].numeroIndividuos++;
							}*/
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}


						}
					}

				}
				//zera as distancias inicialmente
/*		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;*/

				free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
/*	                        	if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
								populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
								populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
								populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
								populacaoParam[idTabelaParam].numeroIndividuos++;
							}*/
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima
							   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}

						}
					}
				}
			}
			//}
        //}
			free(fronteira);
    	}
    }

    free(individuos);
}


void ordenaIndividuosQuedaTensaoVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.quedaMaxima < configuracoes[idConfiguracao2].objetivo.quedaMaxima) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingQuedaTensaoVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosQuedaTensaoVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Carregamento de Rede

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.quedaMaxima
                        - configuracoesParam[idConfiguracao2].objetivo.quedaMaxima);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
}

void insereConfiguracaoTabelaDominanciaQuedaTensaoVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    //int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {
    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			//FRONTEIRAS fronteira[populacaoParam[idTabelaParam].tamanhoMaximo];

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}


        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

/*				printf("\n_______________________________________________________________");
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos + 1; contadorI++) {
					printf("\n%d | %f | %d", individuos[contadorI].idConfiguracao, configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.quedaMaxima, configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.manobrasManuais);
				}*/

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não foi salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;

								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.quedaMaxima >= configuracoesParam[idConfiguracao2].objetivo.quedaMaxima
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracao2].objetivo.quedaMaxima
													&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira"
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}


/*				for(contadorI = 0; contadorI <= idFronteira; contadorI++){
					printf("\n\nFronteira %d\n", contadorI);
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos ; contadorJ++){
						printf("\t %d", fronteira[contadorI].individuos[contadorJ].idConfiguracao);
					}
				}*/

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Salva o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingQuedaTensaoVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            //ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
		            double tmpSol = 1.0e14;
		            int menorDist;
		            long int idConfiguracaoMenorDist;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
/*								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
								}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}



                        	}
                        }

					}
		            //zera as distancias inicialmente
/*		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;*/

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
/*	                        	if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
	                        	}*/
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
			//}
        //}
				free(fronteira);
    	}
    }

    free(individuos);
}



void ordenaIndividuosEnergiaTotalNaoSupridaVsManobrasAutomaticas(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.energiaTotalNaoSuprida < configuracoes[idConfiguracao2].objetivo.energiaTotalNaoSuprida) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingEnergiaTotalNaoSupridaVsManobrasAutomaticas(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaTotalNaoSupridaVsManobrasAutomaticas(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Energia Não Suprida

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida
                        - configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
}
void insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsManobrasAutomaticas(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    //int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;//[populacaoParam[idTabelaParam].numeroIndividuos];
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

    			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;
								// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas >= configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
													&& configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

/*				for(contadorI = 0; contadorI <= idFronteira; contadorI++){
					printf("\n\nFronteira %d\n", contadorI);
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos ; contadorJ++){
						printf("\t %d", fronteira[contadorI].individuos[contadorJ].idConfiguracao);
					}
				}*/

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsManobrasAutomaticas(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            //ordenaPorDistanciaMultidao(individuosUltimaFronteira, numeroIndividuosUltimaFronteira, indicesOrdenados);
		            double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
		            int menorDist = 0;
		            long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
/*								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
								}*/

                        		//Esta mudança abaixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}



                        	}
                        }

					}
		            //zera as distancias inicialmente
		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
/*	                        	if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
	                        	}*/

                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
			//}
        //}
				free(fronteira);
    	}
    }

    free(individuos);
}



void ordenaIndividuosEnergiaTotalNaoSupridaVsManobrasManuais(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.energiaTotalNaoSuprida < configuracoes[idConfiguracao2].objetivo.energiaTotalNaoSuprida) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingEnergiaTotalNaoSupridaVsManobrasManuais(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaTotalNaoSupridaVsManobrasManuais(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Energia Não Suprida

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida
                        - configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais));
            }
        }
    }
}
void insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsManobrasManuais(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

    			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;
								// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
												&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais >= configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
													&& configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracao2].objetivo.manobrasManuais) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsManobrasManuais(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
		            int menorDist = 0;
		            long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}



                        	}
                        }

					}
		            //zera as distancias inicialmente
		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.manobrasManuais == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
				free(fronteira);
    	}
    }
    free(individuos);
}



void ordenaIndividuosEnergiaTotalNaoSupridaVsCarregamentoRede(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.energiaTotalNaoSuprida < configuracoes[idConfiguracao2].objetivo.energiaTotalNaoSuprida) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.maiorCarregamentoRede)< (configuracoes[idConfiguracao2].objetivo.maiorCarregamentoRede)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingEnergiaTotalNaoSupridaVsCarregamentoRede(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaTotalNaoSupridaVsCarregamentoRede(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Energia Não Suprida

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida
                        - configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede)
                        - (configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede));
            }
        }
    }
}
void insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsCarregamentoRede(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

/*    typedef struct{
		long int idConfiguracao;
	 }INDIVIDUOSNAODOMINADOS;

	typedef struct
    {
		int nivelDominancia;
		INDIVIDUOSNAODOMINADOS *individuos;
		int numeroIndividuos;
    }FRONTEIRAS;*/

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

    			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;
								// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
												&& configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede >= configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
													&& configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoRede) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsCarregamentoRede(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
		            int menorDist = 0;
		            long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}



                        	}
                        }

					}
		            //zera as distancias inicialmente
		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
				free(fronteira);
    	}
    }
    free(individuos);
}





void ordenaIndividuosEnergiaTotalNaoSupridaVsCarregamentoTrafo(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.energiaTotalNaoSuprida < configuracoes[idConfiguracao2].objetivo.energiaTotalNaoSuprida) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.maiorCarregamentoTrafo)< (configuracoes[idConfiguracao2].objetivo.maiorCarregamentoTrafo)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingEnergiaTotalNaoSupridaVsCarregamentoTrafo(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaTotalNaoSupridaVsCarregamentoTrafo(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Energia Não Suprida

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida
                        - configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo)
                        - (configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo));
            }
        }
    }
}
void insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsCarregamentoTrafo(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

    			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;
								// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
												&& configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo >= configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
													&& configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracao2].objetivo.maiorCarregamentoTrafo) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsCarregamentoTrafo(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
		            int menorDist = 0;
		            long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}



                        	}
                        }

					}
		            //zera as distancias inicialmente
		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.maiorCarregamentoTrafo == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
				free(fronteira);
    	}
    }
    free(individuos);
}



void ordenaIndividuosEnergiaTotalNaoSupridaVsQuedaTensao(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.energiaTotalNaoSuprida < configuracoes[idConfiguracao2].objetivo.energiaTotalNaoSuprida) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.quedaMaxima)< (configuracoes[idConfiguracao2].objetivo.quedaMaxima)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingEnergiaTotalNaoSupridaVsQuedaTensao(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaTotalNaoSupridaVsQuedaTensao(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Energia Não Suprida

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida
                        - configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.quedaMaxima)
                        - (configuracoesParam[idConfiguracao2].objetivo.quedaMaxima));
            }
        }
    }
}
void insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsQuedaTensao(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    BOOL dominado = false;
    int rank = 0, idFronteira = 0;
    BOOL entrou = false;
    int contadorI, contadorJ, contadorIndividuosDominados = 0, contadorIndividuosVerificados;
    long int idConfiguracao, idConfiguracao1, idConfiguracao2;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    TABELA *individuosUltimaFronteira;
    int numIndividuos;
    int numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

    			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
        		for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
								idConfiguracao2 = individuos[contadorJ].idConfiguracao;
								// Compara idConfiguracao1 com todas as demais configurações a fim de determinar se esta é dominada por algum indivíduo (ou configuração)
								if(configuracoesParam[idConfiguracao2].objetivo.fronteira == 999){ //Se idConfiguracao2 ainda não foi salva em alguma fronteira
									if(idConfiguracao1 != idConfiguracao2){ //Para evitar que um indivíduo seja comparado consigo mesmo
										if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida >= configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
												&& configuracoesParam[idConfiguracao1].objetivo.quedaMaxima >= configuracoesParam[idConfiguracao2].objetivo.quedaMaxima) {
											rank++; //Incrementa se idConfiguracao1 é dominado por idConfiguracao2
											if (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida
													&& configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracao2].objetivo.quedaMaxima) {
												rank--;
											}
										}
									}
								}
							}
						}

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsQuedaTensao(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

		            double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
		            int menorDist = 0;
		            long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
		            for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
		                if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
		                    tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
		                    menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
		                    idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
		                }
		            }

		            //Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela
		            populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
                        	if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}
                        	}
                        }

					}
		            //zera as distancias inicialmente
		            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
		                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

		            free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
	                        for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

                        		//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
	                        	idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
	                        	if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
	                			   && configuracoesParam[idConfiguracao1].objetivo.quedaMaxima == configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima
	                			   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
	                        	}


	                        }
						}
					}
				}
				free(fronteira);
    	}
    }
    free(individuos);
}



void ordenaIndividuosEnergiaTotalNaoSupridaVsTotalManobras(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.energiaTotalNaoSuprida < configuracoes[idConfiguracao2].objetivo.energiaTotalNaoSuprida) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais + configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais + configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }
        }
}


void crowndingEnergiaTotalNaoSupridaVsTotalManobras(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuosEnergiaTotalNaoSupridaVsTotalManobras(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da Energia Não Suprida

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida
                        - configuracoesParam[idConfiguracao2].objetivo.energiaTotalNaoSuprida);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }
}

/** Por Leandro
 * Descrição: Após a geração de uma nova solução, esta função seleciona os indivíduo a serem salvos na tabela por meio da ordenação por não dominância em relação aos valores de
 * Energia TOTAL Não Suprida aos Consumidores e Numero TOTAL de Manobras
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */
void insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsTotalManobras(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ,contadorIndividuosVerificados;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;
    int numIndividuos, numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

		configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
		configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
		populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabelaParam].numeroIndividuos++;
		torneioEntrou[0] = true;

    } else {

    	if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
			FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
				fronteira[contadorI].numeroIndividuos = 0;
				fronteira[contadorI].nivelDominancia = 0;
				fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
			}

			individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
			//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
			for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
				individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
				individuos[contadorI].valorObjetivo = 0;
				individuos[contadorI].distanciaMultidao = 0;
				configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
			}
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

			idFronteira = 0;
			contadorIndividuosVerificados = 0;
			while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
				//Determina a qual fronteira cada indivíduo pertence
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
					idConfiguracao1 = individuos[contadorI].idConfiguracao;
					rank = 0;

					if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
						for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
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

					//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
					if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
						numIndividuos = fronteira[idFronteira].numeroIndividuos;
						fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
						fronteira[idFronteira].nivelDominancia = idFronteira;
						fronteira[idFronteira].numeroIndividuos++;
						//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
						contadorIndividuosVerificados++;
					}


				}
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
				}

				idFronteira++;
			}

			//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
			idFronteira--; //Decrementa para salvar o id da última fronteira construída
			if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
				//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

				numeroIndividuosUltimaFronteira = 0;
				individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
				//Copia somente os indivíduos que estão na última fronteira
				for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
					individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
					individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
					individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
					numeroIndividuosUltimaFronteira++;
				}
				crowndingEnergiaTotalNaoSupridaVsTotalManobras(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

				double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
				int menorDist = 0;
				long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
				for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
					if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
						tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
						menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
						idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
					}
				}

				//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

				populacaoParam[idTabelaParam].numeroIndividuos = 0;
				for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
					for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
						if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
							   && (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
										torneioEntrou[0] = true;
								}
							}



						}
					}

				}
				//zera as distancias inicialmente
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
					populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

				free(individuosUltimaFronteira);

			}
			else{
				if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

							//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
							idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
							if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
							   && (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
							   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
								if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
									populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
									populacaoParam[idTabelaParam].numeroIndividuos++;
									torneioEntrou[0] = true;
								}
							}


						}
					}
				}
			}
			for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
				fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
			fronteira  = NULL; free(fronteira);
			individuos = NULL; free(individuos);
    	}
    }
}





/**
 * Por Leandro:
 * Descrição: Realiza a inserção de uma nova configuração nas tabelas que priorizam o atendimento de consumidores prioritários
 * enquanto minimiza a energia não suprida. Em outras palavras, estas tabelas priorizam o armazenamento de indivíduos
 * que minimizam as cargas desligadas enquanto, concomitantemente, prioriza a reconexão de consumidores seguindo o nível
 * de prioridade de atendimento dos mesmos.
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 */
void insereConfiguracaoTabelasEnergiaNaoSuprida(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int idPrimeiraConfiguracaoParam) {
    int idTabela;
    int posicao;
    int contador, indiceTmp;
    long int idTmp, idConf;

    idTabela = -1;
    //identifica em qual tabela de ENS a configuração poderá ser inserida com base no valor de ENERGIA NÃO SUPRIDA TOTAL
    if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.20*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
    	idTabela = idTabelaParam;
    else{
		if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.40*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
			idTabela = idTabelaParam + 1;
		else {
			if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.60*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
				idTabela = idTabelaParam + 2;
			else {
				if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.80*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
					idTabela = idTabelaParam + 3;
				else {
					idTabela = idTabelaParam + 4;
				}
			}
		}
    }

     //Uma vez definida a tabela na qual a nova configuração poderá ser inserida, então são analisados valores de energia não suprida considerando os níveis de prioridade de atendimento
	if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {//Se a tabela não está cheia, então insere a configuração
		 posicao = populacaoParam[idTabela].numeroIndividuos;
		 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida;
		 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		 populacaoParam[idTabela].numeroIndividuos++;
	}
	else{ //a tabela está cheia
		 indiceTmp = 0;
		 //localiza a pior configuração presente na tabela
		 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			 if (     (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa))) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) ) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConf].objetivo.manobrasManuais >   configuracoesParam[idTmp].objetivo.manobrasManuais)) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConf].objetivo.manobrasManuais ==  configuracoesParam[idTmp].objetivo.manobrasManuais) && (configuracoesParam[idConf].objetivo.manobrasAutomaticas >  configuracoesParam[idTmp].objetivo.manobrasAutomaticas))
			 ){
				 indiceTmp = contador; //Indice da posição da tabela na qual está o pior indivíduo
			 }
		 }
		 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		 //verifica se a nova solução é melhor do que a pior solução encontrada
		 if ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa))) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) <   floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) ) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <  configuracoesParam[idTmp].objetivo.manobrasManuais)) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais ==  configuracoesParam[idTmp].objetivo.manobrasManuais)  && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas <  configuracoesParam[idTmp].objetivo.manobrasAutomaticas))
		 ) {
			 populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida;
		 }
	}
}

/**
 * Por Leandro:
 * Descrição: Consiste na função "insereConfiguracaoTabelasEnergiaNaoSuprida()" modificada para
 * (a) impedir o armazenamento de indivíduo quando valores analisados pelas tabela sejam iguais aos de algum indivíduo já salvo
 * (b) verificar e informar se "idConfiguracaoParam" foi adicionado a tabela
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 */
void insereConfiguracaoTabelasEnergiaNaoSupridaModificada(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, long int idPrimeiraConfiguracaoParam, BOOL *torneioEntrou) {
    int idTabela;
    int posicao;
    int contador, indiceTmp;
    long int idTmp, idConf;
    torneioEntrou[0] = false;

    idTabela = -1;
    //identifica em qual tabela de ENS a configuração poderá ser inserida com base no valor de ENERGIA NÃO SUPRIDA TOTAL
    if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.20*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
    	idTabela = idTabelaParam;
    else{
		if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.40*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
			idTabela = idTabelaParam + 1;
		else {
			if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.60*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
				idTabela = idTabelaParam + 2;
			else {
				if (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida) <= 0.80*floor(configuracoesParam[idPrimeiraConfiguracaoParam].objetivo.energiaTotalNaoSuprida))
					idTabela = idTabelaParam + 3;
				else {
					idTabela = idTabelaParam + 4;
				}
			}
		}
    }

     //Uma vez definida a tabela na qual a nova configuração poderá ser inserida, então são analisados valores de energia não suprida considerando os níveis de prioridade de atendimento
	if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {//Se a tabela não está cheia, então insere a configuração
		if(!verificaSeHaValorIgualJaSalvoTabelasEnergiaNaoSuprida(populacaoParam[idTabela], idConfiguracaoParam, configuracoesParam)){
			 posicao = populacaoParam[idTabela].numeroIndividuos;
			 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida;
			 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
			 populacaoParam[idTabela].numeroIndividuos++;
			 torneioEntrou[0] = true;
		}
	}
	else{ //a tabela está cheia
		 indiceTmp = 0;
		 //localiza a pior configuração presente na tabela
		 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			 if (     (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa))) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) >  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) ) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConf].objetivo.manobrasManuais >   configuracoesParam[idTmp].objetivo.manobrasManuais)) ||
					 ((floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConf].objetivo.manobrasManuais ==  configuracoesParam[idTmp].objetivo.manobrasManuais) && (configuracoesParam[idConf].objetivo.manobrasAutomaticas >  configuracoesParam[idTmp].objetivo.manobrasAutomaticas))
			 ){
				 indiceTmp = contador; //Indice da posição da tabela na qual está o pior indivíduo
			 }
		 }
		 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		 //verifica se a nova solução é melhor do que a pior solução encontrada
		 if ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria))) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa))) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) <   floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) ) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <  configuracoesParam[idTmp].objetivo.manobrasManuais)) ||
				 ((floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) ==  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade)) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais ==  configuracoesParam[idTmp].objetivo.manobrasManuais)  && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas <  configuracoesParam[idTmp].objetivo.manobrasAutomaticas))
		 ) {
			 if(!verificaSeHaValorIgualJaSalvoTabelasEnergiaNaoSuprida(populacaoParam[idTabela], idConfiguracaoParam, configuracoesParam)){
				 populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
				 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida;
				 torneioEntrou[0] = true;
			 }
		 }
	}
}


/**
 * Este método é responsável por fazer a alocação da estrutura para armazenar os indivíduos do AEMT nas respectivas tabelas.
 * Recebe como parâmetros o número de tabelas e quantas soluçõe serão armazenadas em cada tabela. Além disso recebe um ponteiro para a variável 
 * que armazena as tabelas.
 * 
 * @param numeroTabelasParam
 * @param numeroConfiguracoesParam
 * @param populacaoParam
 */
void alocaTabelas(int numeroTabelasParam, int *numeroConfiguracoesParam, VETORTABELA **populacaoParam) {
    int contador;
    //aloca o número de tabelas especificado
//    printf("numero tabelas %d  \n", numeroTabelasParam);
    if ((*populacaoParam = (VETORTABELA *) malloc((numeroTabelasParam + 1) * sizeof (VETORTABELA))) == NULL) {
        printf("Não foi possível alocar as tabelas");
        exit(1);
    }
    //aloca para cada tabela o número de condigurações especificado
    for (contador = 0; contador < numeroTabelasParam; contador++) 
    {
  //      printf("tabela %d tamanho %d \n", contador, numeroConfiguracoesParam[contador]);
        (*populacaoParam)[contador].tabela = (TABELA *) malloc((numeroConfiguracoesParam[contador]) * sizeof (TABELA));
        (*populacaoParam)[contador].numeroIndividuos = 0;
        (*populacaoParam)[contador].tamanhoMaximo = numeroConfiguracoesParam[contador];
        (*populacaoParam)[contador].torneioValorReferencia = 0.0;
    }
}

/**
 * Este método realiza a impressão na tela de todas as tabelas. As informações impressas são o identificador da configuração e o valor do objetivo da tabela analisada. 
 * @param numeroTabelasParam
 * @param populacaoParam
 */
void imprimeTabelas(int numeroTabelasParam, VETORTABELA *populacaoParam) {
    int contadorTabelas, contadorConfiguracoes;
    //percorre todas as tabelas
    for (contadorTabelas = 0; contadorTabelas < numeroTabelasParam; contadorTabelas++) {
        printf("==== Tabela: %d =====\n\n", contadorTabelas);
        //percorre todas as configurações da tabela realizando a impressão dos valores de identificador e objetivo
        for (contadorConfiguracoes = 0; contadorConfiguracoes < populacaoParam[contadorTabelas].numeroIndividuos; contadorConfiguracoes++) {
            printf("\t Identificador Configuracao:  %ld", populacaoParam[contadorTabelas].tabela[contadorConfiguracoes].idConfiguracao);
            printf("\t Valor Objetivo:  %.3lf \n\n", populacaoParam[contadorTabelas].tabela[contadorConfiguracoes].valorObjetivo);
        }
    }

}

/**
 * Este método realiza a impressão na tela de uma tabela especificada no parâmetro idTabelaParam. 
 * As informações impressas são o identificador da configuração e o valor do objetivo da tabela analisada.
 * 
 * @param idTabelaParam
 * @param populacaoParam
 */
void imprimeTabela(int idTabelaParam, VETORTABELA *populacaoParam) {
    int contadorConfiguracoes;

    printf("==== Tabela: %d =====\n\n", idTabelaParam);
    for (contadorConfiguracoes = 0; contadorConfiguracoes < populacaoParam[idTabelaParam].numeroIndividuos; contadorConfiguracoes++) {
        printf("\t Identificador Configuracao:  %ld", populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].idConfiguracao);
        printf("\t Valor Objetivo:  %.3lf \n\n", populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo);
    }
}

/**
 * Este método realiza a impressão na tela de todas as configurações uma tabela especificada no parâmetro idTabelaParam. 
 * As informações impressas são o identificador da configuração, o valor do objetivo da tabela analisada
 * e os dados da configuração.
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 */
void imprimeConfiguracoesTabela(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam) {
    int contadorConfiguracoes;
    int idConfiguracao;
    printf("==== Tabela: %d =====\n\n", idTabelaParam);
    for (contadorConfiguracoes = 0; contadorConfiguracoes < populacaoParam[idTabelaParam].numeroIndividuos; contadorConfiguracoes++) {
        printf("\t Identificador Configuracao:  %ld", populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].idConfiguracao);
        printf("\t Valor Objetivo:  %.3lf \n\n", populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo);
        idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].idConfiguracao;
        imprimeIndividuo(configuracoesParam[idConfiguracao]);
    }
}
/**
 * Este método recebe como parâmetros o identificador de uma tabela, o numero de elementos da tabela, o ponteiro para a lista de tabelas,
 * o ponteiro para o vetor de configurações e um flag lógico indicando se o objetivo é de maximizar(true) ou minimizar (false).
 * Dadas essas informações retorna a posição da tabela que contém a pior configuração para esse objetivo.
 * 
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param maximizar
 * @return 
 */
int retornaPiorConfiguracaoTabela(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, BOOL maximizar) {
    int contadorConfiguracoes;
    int idPior;
    double valorObjetivoPior;
    int comparacao;
    if (populacaoParam[idTabelaParam].numeroIndividuos > 0) {
        //recupera o valor do objetivo do primeiro individuo da tabela e considera como pior.
        valorObjetivoPior = populacaoParam[idTabelaParam].tabela[0].valorObjetivo;
        idPior = 0;
    } else {
        if (maximizar)
            valorObjetivoPior = 100000000.00;
        else
            valorObjetivoPior = -1000000000.00;
        idPior = -1;
    }
    //indica que o objetivo analisado na tabela refere a um problema de maximizar
    if (maximizar) {
        //percorre todas as configurações da tabela fazendo comparações em relação a pior configuração obtida até o momento
        for (contadorConfiguracoes = 1; contadorConfiguracoes < populacaoParam[idTabelaParam].numeroIndividuos; contadorConfiguracoes++) {
            //compara a configuração atual com a pior encontrada, se a atual for pior substitui
            if (valorObjetivoPior > populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo) {
                valorObjetivoPior = populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo;
                idPior = contadorConfiguracoes; //indice na tabela para proporcionar a troca de configurações na tabela
            }
        }
    } else //quando o problema for de minimizar
    {
        //percorre todas as configurações da tabela fazendo comparações em relação a pior configuração obtida até o momento
        for (contadorConfiguracoes = 1; contadorConfiguracoes < populacaoParam[idTabelaParam].numeroIndividuos; contadorConfiguracoes++) {
            //compara a configuração atual com a pior encontrada, se a atual for pior substitui
            if (valorObjetivoPior < populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo) {
                valorObjetivoPior = populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo;
                idPior = contadorConfiguracoes; //indice na tabela para proporcionar a troca de configurações na tabela

            }
        }
    }

    return idPior;
}

/**
 * Realiza a inserção de uma nova configuração na tabela especificada. Considera que a nova configuração substituirá a configuração de pior
 * objetivo para essa tabela, se ela for melhor do que esta. Para isso faz uso da função retornaPiorConfiguracaoTabela e da função comparaObjetivos
 * Além disso, leva em consideração se o objetivo analisado na tabela é de maximizar ou minimizar.
 * @param idTabelaParam
 * @param populacaoParam
 * @param idConfiguracaoParam
 * @param objetivoParam
 * @param configuracoesParam
 * @param maximizar
 * @param torneioEntrou
 */
void insereConfiguracaoTabela(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, double objetivoParam,
        CONFIGURACAO *configuracoesParam, BOOL maximizar, BOOL *torneioEntrou) {
    int idPiorConfiguracao;

    torneioEntrou[0] = false;

    //recupera o indice da pior configuração na tabela
    idPiorConfiguracao = retornaPiorConfiguracaoTabela(idTabelaParam, populacaoParam, configuracoesParam, maximizar);

    if (populacaoParam[idTabelaParam].tamanhoMaximo > 0) {
        //compara a nova solução com a pior solução considerando um objetivo de maximizar
        if (maximizar) {
            //se for melhor que a pior solução substitui
            if (populacaoParam[idTabelaParam].numeroIndividuos >= populacaoParam[idTabelaParam].tamanhoMaximo) {
                if (populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo < objetivoParam) {
                    populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo = objetivoParam;
                    populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].idConfiguracao = idConfiguracaoParam;
                    torneioEntrou[0] = true;
                }
            } else {
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = objetivoParam;
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabelaParam].numeroIndividuos++;
                torneioEntrou[0] = true;
            }
        }//compara a nova solução com a pior solução considerando um objetivo de minimizar
        else {
            //se for melhor que a pior solução substitui

            if (populacaoParam[idTabelaParam].numeroIndividuos >= populacaoParam[idTabelaParam].tamanhoMaximo) {
                if (populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo > objetivoParam) {
                    populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo = objetivoParam;
                    populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].idConfiguracao = idConfiguracaoParam;
                    torneioEntrou[0] = true;
                }
            } else {
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = objetivoParam;
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabelaParam].numeroIndividuos++;
                torneioEntrou[0] = true;
            }
        }
    }
}

/**
 * Por Leandro: consiste na função modificada para evitar que um novo indivíduo seja salvo se este possuir um valor de objetivo igual ao
 * valor de objetivo de um  indivíduo já salvo.
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param idConfiguracaoParam
 * @param objetivoParam
 * @param configuracoesParam
 * @param maximizar
 * @param torneioEntrou
 */
void insereConfiguracaoTabelaModificada(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, double objetivoParam,
        CONFIGURACAO *configuracoesParam, BOOL maximizar, BOOL *torneioEntrou) {
    int idPiorConfiguracao;

    torneioEntrou[0] = false;
    
    //recupera o indice da pior configuração na tabela
    idPiorConfiguracao = retornaPiorConfiguracaoTabela(idTabelaParam, populacaoParam, configuracoesParam, maximizar);

    if (populacaoParam[idTabelaParam].tamanhoMaximo > 0) {
        //compara a nova solução com a pior solução considerando um objetivo de maximizar
        if (maximizar) {
            //se for melhor que a pior solução substitui
            if (populacaoParam[idTabelaParam].numeroIndividuos >= populacaoParam[idTabelaParam].tamanhoMaximo) {
                if (populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo < objetivoParam) {
                	if(!verificaSeHaValorIgualJaSalvoTabela(populacaoParam[idTabelaParam], objetivoParam)){
						populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo = objetivoParam;
						populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].idConfiguracao = idConfiguracaoParam;
						torneioEntrou[0] = true;
                	}
                }
            } else {
            	if(!verificaSeHaValorIgualJaSalvoTabela(populacaoParam[idTabelaParam], objetivoParam)){
					populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = objetivoParam;
					populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
					populacaoParam[idTabelaParam].numeroIndividuos++;
					torneioEntrou[0] = true;
            	}
            }
        }//compara a nova solução com a pior solução considerando um objetivo de minimizar
        else {
            //se for melhor que a pior solução substitui

            if (populacaoParam[idTabelaParam].numeroIndividuos >= populacaoParam[idTabelaParam].tamanhoMaximo) {
                if (populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo > objetivoParam) {
                	if(!verificaSeHaValorIgualJaSalvoTabela(populacaoParam[idTabelaParam], objetivoParam)){
						populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].valorObjetivo = objetivoParam;
						populacaoParam[idTabelaParam].tabela[idPiorConfiguracao].idConfiguracao = idConfiguracaoParam;
						torneioEntrou[0] = true;
                	}
                }
            } else {
            	if(!verificaSeHaValorIgualJaSalvoTabela(populacaoParam[idTabelaParam], objetivoParam)){
					populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = objetivoParam;
					populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
					populacaoParam[idTabelaParam].numeroIndividuos++;
					torneioEntrou[0] = true;
            	}
            }
        }
    }
}

/**
 * Esse método faz a ordenação dos indivíduos de uma subpopulação pelo valor do objetivo informado no parâmetro objetivo.
 * @param objetivo indica se o objetivo é para ordenar pelo valor de ponderação (1) ou pelas manobras
 * @param numeroIndividuos
 * @param individuos
 * @param configuracoes
 * @param indicesOrdenados
 */
void ordenaIndividuos(int objetivo, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoes, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;
    long int idConfiguracao1, idConfiguracao2;

    //inicializa indexador
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificação das soluções ... Otimizar este processo (usar qsort)
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            idConfiguracao1 = individuos[indicesOrdenados[indiceJ - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[indiceJ]].idConfiguracao;
            if (objetivo == 1) {

                if (configuracoes[idConfiguracao1].objetivo.ponderacao < configuracoes[idConfiguracao2].objetivo.ponderacao) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }
            } else {
                if ((configuracoes[idConfiguracao1].objetivo.manobrasManuais + configuracoes[idConfiguracao1].objetivo.manobrasAutomaticas)< (configuracoes[idConfiguracao2].objetivo.manobrasManuais + configuracoes[idConfiguracao2].objetivo.manobrasAutomaticas)) {
                    aux = indicesOrdenados[indiceJ - 1];
                    indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                    indicesOrdenados[indiceJ] = aux;
                }

            }

        }
}
/**
 * Cálcula o valor de distância de crownding de acordo com os objetivos ponderação e número de manobras.
 * @param objetivos 
 * @param numeroIndividuos
 * @param individuos
 * @param configuracoesParam
 */
/*Calcula a distância de crownding da tabela de dominância*/
void crownding(int objetivos, int numeroIndividuos, TABELA *individuos, CONFIGURACAO *configuracoesParam) {

    int i, j, k;

    long int idConfiguracao1, idConfiguracao2;
    int *indicesOrdenados = Malloc(int, numeroIndividuos);
    float dist = 0;

    //vamos utilizar o campo do objetivo para calcular as distâncias
    //zera as distancias inicialmente
    for (i = 0; i < numeroIndividuos; i++)
        individuos[i].valorObjetivo = 0;


    for (k = 0; k < objetivos; k++) {
        ordenaIndividuos(k, numeroIndividuos, individuos, configuracoesParam, indicesOrdenados);
        individuos[indicesOrdenados[0]].distanciaMultidao = 1.0e12;
        individuos[indicesOrdenados[numeroIndividuos - 1]].distanciaMultidao = 1.0e12;

        for (j = 1; j < numeroIndividuos - 1; j++) {
            idConfiguracao1 = individuos[indicesOrdenados[j - 1]].idConfiguracao;
            idConfiguracao2 = individuos[indicesOrdenados[j + 1]].idConfiguracao;
            if (k == 0) { //utiliza o objetivo da ponderacao

                individuos[indicesOrdenados[j]].distanciaMultidao += (configuracoesParam[idConfiguracao1].objetivo.ponderacao
                        - configuracoesParam[idConfiguracao2].objetivo.ponderacao);
            } else {//utiliza o objetivo de manobras
                individuos[indicesOrdenados[j]].distanciaMultidao += ((configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas)
                        - (configuracoesParam[idConfiguracao2].objetivo.manobrasManuais + configuracoesParam[idConfiguracao2].objetivo.manobrasAutomaticas));
            }
        }
    }


}
/**
 * Ordena os indivíduos de uma subpopulação pela distância de crownding
 * @param individuos
 * @param numeroIndividuos
 * @param indicesOrdenados
 */
void ordenaPorDistanciaMultidao(TABELA *individuos, int numeroIndividuos, int *indicesOrdenados) {
    int aux;
    int indiceI, indiceJ;

    //pega os indices da fronteira relacionados ao nï¿½ node.

    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        indicesOrdenados[indiceI] = indiceI;

    //TODO Classificacao das soluoes Otimizar este processo
    for (indiceI = 0; indiceI < numeroIndividuos; indiceI++)
        for (indiceJ = numeroIndividuos - 1; indiceJ > indiceI; indiceJ--) {
            if (individuos[indicesOrdenados[indiceJ - 1]].distanciaMultidao <= individuos[indicesOrdenados[indiceJ]].distanciaMultidao) {
                aux = indicesOrdenados[indiceJ - 1];
                indicesOrdenados[indiceJ - 1] = indicesOrdenados[indiceJ];
                indicesOrdenados[indiceJ] = aux;
            }
        }
}
/**
 * Insere uma nova configuração nas subpopulações de dominância
 * @param idTabelaParam
 * @param populacaoParam
 * @param idConfiguracaoParam
 * @param configuracoesParam
 * @param nivelDominancia
 */
void insereConfiguracaoTabelaDominancia(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, int nivelDominancia) {
    BOOL dominado = false;
    int rank = 0;
    BOOL entrou = false;
    int contadorI, contadorJ;
    long int idConfiguracao;
    TABELA *individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    int *indicesOrdenados = Malloc(int, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
    //printf("ponto 0 nivel Dominancia %d \n", nivelDominancia);
    //Nesse caso a tabela não está cheia ainda
    if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) {
        //numero de individuos for 0 a população está vazia e a primeira configuração deve ser inserida diretamente
        if (populacaoParam[idTabelaParam].numeroIndividuos == 0) {
            configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
            configuracoesParam[idConfiguracaoParam].objetivo.fronteira = nivelDominancia;
            populacaoParam[idTabelaParam].tabela[0].valorObjetivo = nivelDominancia;
            populacaoParam[idTabelaParam].tabela[0].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabelaParam].numeroIndividuos++;
        } else {// a tabela possui mais de uma solução
            rank = 0;
            entrou = false;
            for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos; contadorJ++) {
                populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo = 0;
                idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorJ].idConfiguracao;
                if (configuracoesParam[idConfiguracaoParam].objetivo.ponderacao >= configuracoesParam[idConfiguracao].objetivo.ponderacao
                        && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas/fatorPonderacaoManobras) >= (configuracoesParam[idConfiguracao].objetivo.manobrasManuais + configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas/fatorPonderacaoManobras)) {
                    rank++;
                    if (configuracoesParam[idConfiguracaoParam].objetivo.ponderacao == configuracoesParam[idConfiguracao].objetivo.ponderacao
                            && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas/fatorPonderacaoManobras) == (configuracoesParam[idConfiguracao].objetivo.manobrasManuais + configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas/fatorPonderacaoManobras)) {
                        rank--;
                        populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo++;
                    }
                } else if (configuracoesParam[idConfiguracaoParam].objetivo.ponderacao <= configuracoesParam[idConfiguracao].objetivo.ponderacao
                        && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas/fatorPonderacaoManobras) <= (configuracoesParam[idConfiguracao].objetivo.manobrasManuais + configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas/fatorPonderacaoManobras)) {
                    populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo++;
                }
            }
            if (rank == 0) {
                for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos; contadorJ++) {
                    if (populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo > 0) //significa que a solucao esta sendo dominada
                    {
                        //a nova solução substitui todas as soluções dominadas por ela
                        populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.ponderacao;
                        populacaoParam[idTabelaParam].tabela[contadorJ].idConfiguracao = idConfiguracaoParam;
                        configuracoesParam[idConfiguracaoParam].objetivo.fronteira = nivelDominancia;
                        configuracoesParam[idConfiguracaoParam].objetivo.rank = rank;

                        //atualizaNaoDominadas(j, ind_pop, 0);

                        entrou = true;
                    }
                }
            }
            if (rank == 0 && entrou == false) {
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.ponderacao;
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
                configuracoesParam[idConfiguracaoParam].objetivo.fronteira = nivelDominancia;
                configuracoesParam[idConfiguracaoParam].objetivo.rank = rank;
                populacaoParam[idTabelaParam].numeroIndividuos++;
            }
            if (rank != 0) {
                nivelDominancia++;
                //   printf("ponto 1 nivel Dominancia %d \n", nivelDominancia);
                if (nivelDominancia < 3) {
                    insereConfiguracaoTabelaDominancia((idTabelaParam + 1), populacaoParam, idConfiguracaoParam, configuracoesParam, (nivelDominancia));
                }
            }
        }

    } else {
        rank = 0;
        entrou = false;
        for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos; contadorJ++) {
            populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo = 0;
            populacaoParam[idTabelaParam].tabela[contadorJ].distanciaMultidao = 0;
            idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorJ].idConfiguracao;
            if (configuracoesParam[idConfiguracaoParam].objetivo.ponderacao >= configuracoesParam[idConfiguracao].objetivo.ponderacao
                    && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) >= (configuracoesParam[idConfiguracao].objetivo.manobrasManuais + configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas)) {
                rank++;
                if (configuracoesParam[idConfiguracaoParam].objetivo.ponderacao == configuracoesParam[idConfiguracao].objetivo.ponderacao
                        && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracao].objetivo.manobrasManuais + configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas)) {
                    rank--;
                    populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo++;
                }
            } else if (configuracoesParam[idConfiguracaoParam].objetivo.ponderacao <= configuracoesParam[idConfiguracao].objetivo.ponderacao
                    && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) <= (configuracoesParam[idConfiguracao].objetivo.manobrasManuais + configuracoesParam[idConfiguracao].objetivo.manobrasAutomaticas)) {
                populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo++;
            }
        }
        if (rank == 0) {
            for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos; contadorJ++) {
                if (populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo > 0) //significa que a solucao esta sendo dominada
                {
                    //a nova solução substitui todas as soluções dominadas por ela
                    populacaoParam[idTabelaParam].tabela[contadorJ].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.ponderacao;
                    populacaoParam[idTabelaParam].tabela[contadorJ].idConfiguracao = idConfiguracaoParam;
                    populacaoParam[idTabelaParam].tabela[contadorJ].distanciaMultidao = 0;
                    configuracoesParam[idConfiguracaoParam].objetivo.fronteira = nivelDominancia;
                    configuracoesParam[idConfiguracaoParam].objetivo.rank = rank;

                    //atualizaNaoDominadas(j, ind_pop, 0);

                    entrou = true;
                }
            }
        }
        //a tabela está cheia e o novo elemento não domina nenhuma solução
        if (rank == 0 && entrou == 0) {
            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
                individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
                individuos[contadorI].valorObjetivo = populacaoParam[idTabelaParam].tabela[contadorI].valorObjetivo;
                individuos[contadorI].distanciaMultidao = 0;
            }

            individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
            individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
            individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
            crownding(2, (populacaoParam[idTabelaParam].numeroIndividuos + 1), individuos, configuracoesParam);

            ordenaPorDistanciaMultidao(individuos, (populacaoParam[idTabelaParam].numeroIndividuos + 1), indicesOrdenados);
            double tmpSol = 1.0e14;
            int menorDist;
            for (contadorI = 0; contadorI <= populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
                if (tmpSol > individuos[contadorI].distanciaMultidao) {
                    tmpSol = individuos[contadorI].distanciaMultidao;
                    menorDist = contadorI;
                }
            }
            for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos; contadorJ++) {

                contadorI = indicesOrdenados[contadorJ];

                if (contadorI == populacaoParam[idTabelaParam].numeroIndividuos) {
                    //   flag = 1;
                    populacaoParam[idTabelaParam].tabela[menorDist].idConfiguracao = individuos[contadorI].idConfiguracao;
                    populacaoParam[idTabelaParam].tabela[menorDist].valorObjetivo = individuos[contadorI].valorObjetivo;
                    populacaoParam[idTabelaParam].tabela[menorDist].distanciaMultidao = individuos[contadorI].distanciaMultidao;
                }
            }
            entrou = true;
            //zera as distancias inicialmente
            for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
                populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;
        }

        if (rank != 0) {
            nivelDominancia++;
            // printf("ponto 2 nivel Dominancia %d \n", nivelDominancia);
            if (nivelDominancia < 3) {
                insereConfiguracaoTabelaDominancia((idTabelaParam + 1), populacaoParam, idConfiguracaoParam, configuracoesParam, nivelDominancia);
            }
        }
    }
    free(individuos);
    free(indicesOrdenados);
}

/**
 * Esta função retorna o indice da tabela referente a melhor configuração presente na subpopulação.
 * Se encontra duas soluções com o mesmo objetivo para essa subpopulação verifica os demais objetivos.
 * Retorna o índice da subpopulação que contém a configuração com o melhor valor do objetivo analisado na subpopulação.
 * @param idTabelaParam
 * @param populacaoParam
 * @param maximizar
 * @return 
 */
long int retornaMelhorConfiguracaoTabela(int idTabelaParam, VETORTABELA *populacaoParam, BOOL maximizar) {
    int contadorConfiguracoes;
    int idMelhor;
    double valorObjetivoMelhor;
    //recupera o valor do objetivo do primeiro individuo da tabela e considera como melhor.
    if (populacaoParam[idTabelaParam].numeroIndividuos > 0) {
        //recupera o valor do objetivo do primeiro individuo da tabela e considera como pior.
        valorObjetivoMelhor = populacaoParam[idTabelaParam].tabela[0].valorObjetivo;
        idMelhor = 0;
    } else {
        if (maximizar)
            valorObjetivoMelhor = -100000000.00;
        else
            valorObjetivoMelhor = 1000000000.00;
        idMelhor = -1;
    }

    //realiza a busca considerando que que o objetivo analisado na tabela refere a um problema de maximizar
    if (maximizar) {
        //percorre as configurações da tabela buscando pela melhor delas.
        for (contadorConfiguracoes = 1; contadorConfiguracoes < populacaoParam[idTabelaParam].numeroIndividuos; contadorConfiguracoes++) {
            //se a configuração atual é melhor do que escolhida substitui os valores de melhor objetivo e índice
            if (valorObjetivoMelhor < populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo) {
                valorObjetivoMelhor = populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo;
                idMelhor = contadorConfiguracoes;
            }
        }

    } else //realiza a busca considerando que que o objetivo analisado na tabela refere a um problema de minimizar
    {
        for (contadorConfiguracoes = 1; contadorConfiguracoes < populacaoParam[idTabelaParam].numeroIndividuos; contadorConfiguracoes++) {
            //se a configuração atual é melhor do que escolhida substitui os valores de melhor objetivo e índice
            if (valorObjetivoMelhor > populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo) {
                valorObjetivoMelhor = populacaoParam[idTabelaParam].tabela[contadorConfiguracoes].valorObjetivo;
                idMelhor = contadorConfiguracoes;
            }
        }
    }

    return idMelhor;
}

/**
 * Função que retorna o identificador de uma configuração no vetor de configurações para ser utilizada
 * na obtenção de uma nova configuração. Realiza um processo de seleção uniforme da subpopulação e de uma configuracao da subpopulação.
 * Tem com parâmetros a lista de subpopulações, o numero de subpopulações na lista e o numero de configurações de cada subpopulação
 * 
 * @param populacaoParam
 * @param numeroTabelasParam
 * @return 
 */
long int selecionaConfiguracao(VETORTABELA *populacaoParam, int numeroTabelasParam) {
    int indiceTabela;
    int indiceConfiguracaoTabela;
    long int idConfiguracaoSelecionada;

    indiceTabela = inteiroAleatorio(0, numeroTabelasParam-1);
    while (populacaoParam[indiceTabela].numeroIndividuos <= 0)
        indiceTabela = inteiroAleatorio(0, numeroTabelasParam-1);
    indiceConfiguracaoTabela = inteiroAleatorio(0, (populacaoParam[indiceTabela].numeroIndividuos - 1));
    //  printf("indiceTabela %d indiceConfigurcao %d \n", indiceTabela, indiceConfiguracaoTabela);
    idConfiguracaoSelecionada = populacaoParam[indiceTabela].tabela[indiceConfiguracaoTabela].idConfiguracao;

    return idConfiguracaoSelecionada;
}
/**
 * Realiza a inserção de uma nova configuração na subpopulação de número de manobras correspondente.
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroMinimoManobrasManuais
 * @param torneioQtdInclusoesManobras
 */
void insereConfiguracaoTabelaManobras(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, 
        long int numeroMinimoManobrasManuais, int *torneioQtdInclusoesManobras) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    double tmpNovo, tmpAntigo, tmpAntigoContador;
    double ponderacaoManobras;
    
    torneioQtdInclusoesManobras[0] = 0;
    /*tmpNovo = funcaoDistanciaFactivel(configuracoesParam, idConfiguracaoParam);*/


    /*
     * Leandro: Esta rotina comentada abaixo consiste na rotina origirnal da “MRAN2_0_LNA_IBP_CBE”. Ela foi substituída por aquela
     * rotina logo abaixo a fim de estas 5 tabelas tenham o mesmo comportamento que tem no AEMT++.
     */
/*    //identifica em qual tabela de manobras a configuração será inserida
    //primeira tabela - sem manobras manuais
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == numeroMinimoManobrasManuais) {
        idTabela = idTabelaParam;
    } else {
        if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+2)) {
            //segunda tabela - entre [1-4] manobras manuais
            idTabela = idTabelaParam + 1;
        } else {
            if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+4)) {
                //terceira tabela - entre [5-8] manobras manuais
                idTabela = idTabelaParam + 2;
            } else {
                if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+6)) {
                    //quarta tabela - entre [9-12] manobras manuais
                    idTabela = idTabelaParam + 3;
                } else {
                    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+8)) {
                        //quinta tabela - entre [13-16] manobras manuais
                        idTabela = idTabelaParam + 4;
                    }

                }
            }
        }
    }
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= numeroMinimoManobrasManuais + 8) {
        //a tabela não está cheia insere a configuracao
        if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
            posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
            populacaoParam[idTabela].numeroIndividuos++;
            torneioQtdInclusoesManobras[0]++;
        } else //a tabela está cheia
        {
            manobrasTmp = -1;
            indiceTmp = 0;
            //localiza a pior configuração presente na tabela
            for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
                idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
                tmpAntigo = funcaoDistanciaFactivel(configuracoesParam, idTmp);
                tmpAntigoContador = funcaoDistanciaFactivel(configuracoesParam, idConf);                
                
                if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca < configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && tmpAntigoContador > tmpAntigo)) {
                    indiceTmp = contador;
                    manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;

                }
            }
            idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
            tmpAntigo = funcaoDistanciaFactivel(configuracoesParam, idTmp);
            
            //verifica se a nova solução é melhor do que a pior solução encontrada
            if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual && tmpNovo < tmpAntigo)) {
                populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                torneioQtdInclusoesManobras[0]++;
            }

        }
    }*/

    /*
     * Leandro: Esta rotina abaixo refleto o comportamente  que estas 5 tabelas teem no AEMT++.
     */
    //identifica em qual tabela de manobras a configuração será inserida
     //primeira tabela - sem manobras manuais
     torneioQtdInclusoesManobras[0] = 0;

     if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == 0) {
         idTabela = idTabelaParam;
     } else {
         if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais >= 1) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 4)) {
             //segunda tabela - entre [1-4] manobras manuais
             idTabela = idTabelaParam + 1;
         } else {
             if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 8)) {
                 //terceira tabela - entre [5-8] manobras manuais
                 idTabela = idTabelaParam + 2;
             } else {
                 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 12)) {
                     //quarta tabela - entre [9-12] manobras manuais
                     idTabela = idTabelaParam + 3;
                 } else {
                     if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 16)) {
                         //quinta tabela - entre [13-16] manobras manuais
                         idTabela = idTabelaParam + 4;
                     }
                 }
             }
         }

     }
     if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 16) {
         //a tabela não está cheia insere a configuracao
         if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
             posicao = populacaoParam[idTabela].numeroIndividuos;
             populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
             populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
             populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
             populacaoParam[idTabela].numeroIndividuos++;
             torneioQtdInclusoesManobras[0]++;
         } else //a tabela está cheia
         {
             manobrasTmp = -1;
             indiceTmp = 0;
             //localiza a pior configuração presente na tabela
             for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
                 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
                 if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
                         (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
                         (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) >  floor(configuracoesParam[idConf].objetivo.maiorCarregamentoRede)) ||
                         (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idConf].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idTmp].objetivo.menorTensao) <  floor(configuracoesParam[idConf].objetivo.menorTensao)) ||
                         (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idConf].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idTmp].objetivo.menorTensao) == floor(configuracoesParam[idConf].objetivo.menorTensao) && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo) < floor(configuracoesParam[idConf].objetivo.maiorCarregamentoTrafo))) {
                     indiceTmp = contador;
                     manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;
                 }
             }
             idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
             //verifica se a nova solução é melhor do que a pior solução encontrada
             if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
                     (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
                     (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) >  floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede)) ||
                     (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idConfiguracaoParam].objetivo.menorTensao) < floor(configuracoesParam[idTmp].objetivo.menorTensao)) ||
                     (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idConfiguracaoParam].objetivo.menorTensao) == floor(configuracoesParam[idTmp].objetivo.menorTensao) && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo) < floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo))) {
             	populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
                 //*sucessoAplicacaoOperadorParam = 1;
                 torneioQtdInclusoesManobras[0]++;
             }
         }
     }

}

/**
 * Por Leandro:
 * Consiste na função "insereConfiguracaoTabelaManobras()" modificada para tornar possível o armazenamento de
 * soluções com mais de 16 manobras em chaves manuais
 *
 * Assim como a função "insereConfiguracaoTabelaManobras()", realiza a inserção de uma nova configuração na
 * subpopulação de número de manobras correspondente.
 *
 * IMPORTANTE: TAMBÉM O TRECHO COMENTADO FOI MODIFICADO PARA NÃO IMPOR UM LIMITE SUPERIOR AO NÚMERO DE MANOBRAS
 * MANUAIS QUE INDIVÍDUO DEVE TER PARA SER SALVO NA ÚLTIMA TABELA DE MANOBRAS
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroMinimoManobrasManuais
 * @param torneioQtdInclusoesManobras
 */
void insereConfiguracaoTabelaManobrasModificada(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
        long int numeroMinimoManobrasManuais, int *torneioQtdInclusoesManobras) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    double tmpNovo, tmpAntigo, tmpAntigoContador;
    double ponderacaoManobras;

    torneioQtdInclusoesManobras[0] = 0;
    /*tmpNovo = funcaoDistanciaFactivel(configuracoesParam, idConfiguracaoParam);*/


    /*
     * Leandro: Esta rotina comentada abaixo consiste na rotina origirnal da “MRAN2_0_LNA_IBP_CBE”. Ela foi substituída por aquela
     * rotina logo abaixo a fim de estas 5 tabelas tenham um comportamento mais semelhante do AEMT++.
     *
     * IMPORTANTE: TAMBÉM O TRECHO COMENTADO FOI MODIFICADO PARA NÃO IMPOR UM LIMITE SUPERIOR AO NÚMERO DE MANOBRAS
 	   MANUAIS QUE INDIVÍDUO DEVE TER PARA SER SALVO NA ÚLTIMA TABELA DE MANOBRAS
     */

/*    //identifica em qual tabela de manobras a configuração será inserida
    //primeira tabela - sem manobras manuais
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == numeroMinimoManobrasManuais) {
        idTabela = idTabelaParam;
    } else {
        if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+2)) {
            //segunda tabela - entre [1-4] manobras manuais
            idTabela = idTabelaParam + 1;
        } else {
            if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+4)) {
                //terceira tabela - entre [5-8] manobras manuais
                idTabela = idTabelaParam + 2;
            } else {
                if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+6)) {
                    //quarta tabela - entre [9-12] manobras manuais
                    idTabela = idTabelaParam + 3;
                } else {
                        //quinta tabela - entre [13-16] manobras manuais
                        idTabela = idTabelaParam + 4;
                }
            }
        }
    }

	//a tabela não está cheia insere a configuracao
	if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		posicao = populacaoParam[idTabela].numeroIndividuos;
		populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
		populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		populacaoParam[idTabela].numeroIndividuos++;
		torneioQtdInclusoesManobras[0]++;
	} else //a tabela está cheia
	{
		manobrasTmp = -1;
		indiceTmp = 0;
		//localiza a pior configuração presente na tabela
		for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			tmpAntigo = funcaoDistanciaFactivel(configuracoesParam, idTmp);
			tmpAntigoContador = funcaoDistanciaFactivel(configuracoesParam, idConf);

			if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca < configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && tmpAntigoContador > tmpAntigo)) {
				indiceTmp = contador;
				manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;

			}
		}
		idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		tmpAntigo = funcaoDistanciaFactivel(configuracoesParam, idTmp);

		//verifica se a nova solução é melhor do que a pior solução encontrada
		if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual && tmpNovo < tmpAntigo)) {
			populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
			populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
			torneioQtdInclusoesManobras[0]++;
		}

	}
    */

    //identifica em qual tabela de manobras a configuração será inserida
     //primeira tabela - sem manobras manuais
     torneioQtdInclusoesManobras[0] = 0;

     if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == 0) {
         idTabela = idTabelaParam;
     } else {
         if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais >= 1) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 4)) {
             //segunda tabela - entre [1-4] manobras manuais
             idTabela = idTabelaParam + 1;
         } else {
             if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 8)) {
                 //terceira tabela - entre [5-8] manobras manuais
                 idTabela = idTabelaParam + 2;
             } else {
                 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 12)) {
                     //quarta tabela - entre [9-12] manobras manuais
                     idTabela = idTabelaParam + 3;
                 } else {
                         //quinta tabela -  13 ou mais manobras manuais
                         idTabela = idTabelaParam + 4;
                 }
             }
         }

     }

	 //a tabela não está cheia insere a configuracao
	 if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		 posicao = populacaoParam[idTabela].numeroIndividuos;
		 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
		 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		 populacaoParam[idTabela].numeroIndividuos++;
		 torneioQtdInclusoesManobras[0]++;
	 } else //a tabela está cheia
	 {
		 manobrasTmp = -1;
		 indiceTmp = 0;
		 //localiza a pior configuração presente na tabela
		 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			 if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) >  floor(configuracoesParam[idConf].objetivo.maiorCarregamentoRede)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idConf].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idTmp].objetivo.menorTensao) <  floor(configuracoesParam[idConf].objetivo.menorTensao)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idConf].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idTmp].objetivo.menorTensao) == floor(configuracoesParam[idConf].objetivo.menorTensao) && floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo) < floor(configuracoesParam[idConf].objetivo.maiorCarregamentoTrafo))) {
				 indiceTmp = contador;
				 manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;
			 }
		 }
		 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		 //verifica se a nova solução é melhor do que a pior solução encontrada
		 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) >  floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idConfiguracaoParam].objetivo.menorTensao) <  floor(configuracoesParam[idTmp].objetivo.menorTensao)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) == floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) && floor(configuracoesParam[idConfiguracaoParam].objetivo.menorTensao) == floor(configuracoesParam[idTmp].objetivo.menorTensao) && floor(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo) < floor(configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo))) {
			populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
			 //*sucessoAplicacaoOperadorParam = 1;
			 torneioQtdInclusoesManobras[0]++;
		 }
	 }

}




/**
 * Por Leandro:
 * Consiste na função "insereConfiguracaoTabelaManobrasModificada()" com parametros diferentes a serem avaliados para a inserção ou não de uma nova solução
 * na subpopulação de número de manobras. Assim, em vez de considerar os valores das restrições operacionais para seleção entre indivíduos no caso
 * de empate em relação ao número de manobras, esta função considera os valores de Energia Não Suprida e os múltiplos níveis de prioridade de atendimento dos consumidores
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroMinimoManobrasManuais
 * @param torneioQtdInclusoesManobras
 */
void insereConfiguracaoTabelaManobrasV2(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam,
        long int numeroMinimoManobrasManuais, int *torneioQtdInclusoesManobras) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    double tmpNovo, tmpAntigo, tmpAntigoContador;
    double ponderacaoManobras;

    torneioQtdInclusoesManobras[0] = 0;
    /*tmpNovo = funcaoDistanciaFactivel(configuracoesParam, idConfiguracaoParam);*/


    /*
     * Leandro: Esta rotina comentada abaixo consiste na rotina origirnal da “MRAN2_0_LNA_IBP_CBE”. Ela foi substituída por aquela
     * rotina logo abaixo a fim de estas 5 tabelas tenham um comportamento mais semelhante do AEMT++.
     *
     * IMPORTANTE: TAMBÉM O TRECHO COMENTADO FOI MODIFICADO PARA NÃO IMPOR UM LIMITE SUPERIOR AO NÚMERO DE MANOBRAS
 	   MANUAIS QUE INDIVÍDUO DEVE TER PARA SER SALVO NA ÚLTIMA TABELA DE MANOBRAS
     */

/*    //identifica em qual tabela de manobras a configuração será inserida
    //primeira tabela - sem manobras manuais
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == numeroMinimoManobrasManuais) {
        idTabela = idTabelaParam;
    } else {
        if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+2)) {
            //segunda tabela - entre [1-4] manobras manuais
            idTabela = idTabelaParam + 1;
        } else {
            if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+4)) {
                //terceira tabela - entre [5-8] manobras manuais
                idTabela = idTabelaParam + 2;
            } else {
                if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= (numeroMinimoManobrasManuais+6)) {
                    //quarta tabela - entre [9-12] manobras manuais
                    idTabela = idTabelaParam + 3;
                } else {
                        //quinta tabela - entre [13-16] manobras manuais
                        idTabela = idTabelaParam + 4;
                }
            }
        }
    }

	//a tabela não está cheia insere a configuracao
	if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		posicao = populacaoParam[idTabela].numeroIndividuos;
		populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
		populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		populacaoParam[idTabela].numeroIndividuos++;
		torneioQtdInclusoesManobras[0]++;
	} else //a tabela está cheia
	{
		manobrasTmp = -1;
		indiceTmp = 0;
		//localiza a pior configuração presente na tabela
		for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			tmpAntigo = funcaoDistanciaFactivel(configuracoesParam, idTmp);
			tmpAntigoContador = funcaoDistanciaFactivel(configuracoesParam, idConf);

			if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca < configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual) ||
					(manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && tmpAntigoContador > tmpAntigo)) {
				indiceTmp = contador;
				manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;

			}
		}
		idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		tmpAntigo = funcaoDistanciaFactivel(configuracoesParam, idTmp);

		//verifica se a nova solução é melhor do que a pior solução encontrada
		if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual) ||
				(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual && tmpNovo < tmpAntigo)) {
			populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
			populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
			torneioQtdInclusoesManobras[0]++;
		}

	}
    */

    //identifica em qual tabela de manobras a configuração será inserida
     //primeira tabela - sem manobras manuais
     torneioQtdInclusoesManobras[0] = 0;

     if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == 0) {
         idTabela = idTabelaParam;
     } else {
         if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais >= 1) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 4)) {
             //segunda tabela - entre [1-4] manobras manuais
             idTabela = idTabelaParam + 1;
         } else {
             if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 8)) {
                 //terceira tabela - entre [5-8] manobras manuais
                 idTabela = idTabelaParam + 2;
             } else {
                 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 12)) {
                     //quarta tabela - entre [9-12] manobras manuais
                     idTabela = idTabelaParam + 3;
                 } else {
                         //quinta tabela -  13 ou mais manobras manuais
                         idTabela = idTabelaParam + 4;
                 }
             }
         }

     }

	 //a tabela não está cheia insere a configuracao
	 if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		 posicao = populacaoParam[idTabela].numeroIndividuos;
		 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
		 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		 populacaoParam[idTabela].numeroIndividuos++;
		 torneioQtdInclusoesManobras[0]++;
	 } else //a tabela está cheia
	 {
		 manobrasTmp = -1;
		 indiceTmp = 0;
		 //localiza a pior configuração presente na tabela
		 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			 if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)  && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) < floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade))

			 ) {
				 indiceTmp = contador;
				 manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;
			 }
		 }
		 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		 //verifica se a nova solução é melhor do que a pior solução encontrada
		 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade))
		  ) {
			populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
			 //*sucessoAplicacaoOperadorParam = 1;
			 torneioQtdInclusoesManobras[0]++;
		 }
	 }

}

/**
 * Por Leandro:
 * Nesta versao das tabelas de manobras e priorização de chaves,
 * a) avalia-se os valores de número TOTAL de manobras em vez do número de manobras em MANUAIS;
 * b) os intervalos de valores de número de manobras foram alterados;
 * c) em vez de considerar os valores das restrições operacionais para a seleção entre indivíduos no caso
 * de empate em relação ao número de manobras, esta função considera os valores de Energia Não Suprida a cada nível de prioridade de atendimento dos consumidores
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroMinimoManobrasManuais
 * @param torneioQtdInclusoesManobras
 * @param numeroTotalManobrasRestauracaoCompletaSemAlivioParam armazena o número total de manobras necessárias para restaurar todos os agrupamentos de setores saudáveis possíveis de serem reconectados SEM a execução de manobras para a alívio dos alimentadores adjacentes à região da falta.
 */
void insereConfiguracaoTabelasManobrasV3(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, int numeroTotalManobrasRestauracaoCompletaSemAlivioParam) {
    int idTabela = -1;
    int posicao;
    int contador, indiceTmp, manobrasTmp, numeroTotalManobras;
    long int idTmp, idConf;

     numeroTotalManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    //identifica em qual tabela de manobras a configuração será inserida
     if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam/2)
    	 //Na primeira tabela são salvos indivíduos que promovem restauração parcial para até aproximadamente metade dos agrupamentos de setores saudáveis fora de serviço
         idTabela = idTabelaParam;
     else {
         if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam)
        	 //Na segunda tabela são salvos indivíduos que promovem restauração parcial para mais da metade dos agrupamentos de setores saudáveis fora de serviço e
        	 // restauração completa sem a execução de manobras de alívio dos aliementados que receberam/receberão os setores restauradas
             idTabela = idTabelaParam + 1;
         else {
             if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam + 6)
            	 //Na terceira tabela são salvos indivíduos que promovem restauração completa com a executação de até 3 pares de manobras de alívio dos alimentados adjacentes à região da falta
            	 idTabela = idTabelaParam + 2;
             else {
                 if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam + 12)
                	 //Na quarta tabela são salvos indivíduos que promovem restauração completa com a executação de até 6 pares de manobras de alívio dos alimentados adjacentes à região da falta
                	 idTabela = idTabelaParam + 3;
                 else {
                	 //Na quinta tabela são salvos indivíduos que promovem restauração completa com a executação de mais de 6 pares de manobras de alívio do alimentados adjacentes à região da falta
                	 idTabela = idTabelaParam + 4;
                 }
             }
         }
     }

	 //a tabela não está cheia insere diretamente a configuracao
	 if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		 posicao = populacaoParam[idTabela].numeroIndividuos;
		 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
		 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		 populacaoParam[idTabela].numeroIndividuos++;
	 } else //a tabela está cheia
	 {
		 manobrasTmp = -1;
		 indiceTmp = 0;
		 //localiza a pior configuração presente na tabela
		 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			 if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)  && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) < floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade))

			 ) {
				 indiceTmp = contador;
				 manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;
			 }
		 }
		 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		 //verifica se a nova solução é melhor do que a pior solução encontrada
		 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade))
		  ) {
			 populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
		 }
	 }

}

/**
 * Por Leandro:
 * Consiste na função "insereConfiguracaoTabelasManobrasV3()" modificada para
 * (a) impedir o armazenamento de indivíduo quando valores analisados pelas tabela sejam iguais aos de algum indivíduo já salvo
 * (b) verificar e informar se "idConfiguracaoParam" foi adicionado a tabela
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroMinimoManobrasManuais
 * @param torneioQtdInclusoesManobras
 * @param numeroTotalManobrasRestauracaoCompletaSemAlivioParam armazena o número total de manobras necessárias para restaurar todos os agrupamentos de setores saudáveis possíveis de serem reconectados SEM a execução de manobras para a alívio dos alimentadores adjacentes à região da falta.
 */
void insereConfiguracaoTabelasManobrasV3Modificada(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, int numeroTotalManobrasRestauracaoCompletaSemAlivioParam, BOOL *torneioEntrou) {
    int idTabela = -1;
    int posicao;
    int contador, indiceTmp, manobrasTmp, numeroTotalManobras;
    long int idTmp, idConf;
    torneioEntrou[0] = false;

     numeroTotalManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas;
    //identifica em qual tabela de manobras a configuração será inserida
     if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam/2)
    	 //Na primeira tabela são salvos indivíduos que promovem restauração parcial para até aproximadamente metade dos agrupamentos de setores saudáveis fora de serviço
         idTabela = idTabelaParam;
     else {
         if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam)
        	 //Na segunda tabela são salvos indivíduos que promovem restauração parcial para mais da metade dos agrupamentos de setores saudáveis fora de serviço e
        	 // restauração completa sem a execução de manobras de alívio dos aliementados que receberam/receberão os setores restauradas
             idTabela = idTabelaParam + 1;
         else {
             if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam + 6)
            	 //Na terceira tabela são salvos indivíduos que promovem restauração completa com a executação de até 3 pares de manobras de alívio dos alimentados adjacentes à região da falta
            	 idTabela = idTabelaParam + 2;
             else {
                 if (numeroTotalManobras <= numeroTotalManobrasRestauracaoCompletaSemAlivioParam + 12)
                	 //Na quarta tabela são salvos indivíduos que promovem restauração completa com a executação de até 6 pares de manobras de alívio dos alimentados adjacentes à região da falta
                	 idTabela = idTabelaParam + 3;
                 else {
                	 //Na quinta tabela são salvos indivíduos que promovem restauração completa com a executação de mais de 6 pares de manobras de alívio do alimentados adjacentes à região da falta
                	 idTabela = idTabelaParam + 4;
                 }
             }
         }
     }

	 //a tabela não está cheia insere diretamente a configuracao
	 if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		 if(!verificaSeHaValorIgualJaSalvoTabelasManobras(populacaoParam[idTabela], idConfiguracaoParam, configuracoesParam)){
			 posicao = populacaoParam[idTabela].numeroIndividuos;
			 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
			 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
			 populacaoParam[idTabela].numeroIndividuos++;
			 torneioEntrou[0] = true;
		 }
	 } else //a tabela está cheia
	 {
		 manobrasTmp = -1;
		 indiceTmp = 0;
		 //localiza a pior configuração presente na tabela
		 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			 if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) ||
					 (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)  && floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) < floor(configuracoesParam[idConf].objetivo.energiaNaoSuprida.consumidoresSemPrioridade))

			 ) {
				 indiceTmp = contador;
				 manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;
			 }
		 }
		 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
		 //verifica se a nova solução é melhor do que a pior solução encontrada
		 if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa)) ||
				 (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeAlta) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeIntermediaria) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresPrioridadeBaixa) && floor(configuracoesParam[idConfiguracaoParam].objetivo.energiaNaoSuprida.consumidoresSemPrioridade) <  floor(configuracoesParam[idTmp].objetivo.energiaNaoSuprida.consumidoresSemPrioridade))
		  ) {
			 if(!verificaSeHaValorIgualJaSalvoTabelasManobras(populacaoParam[idTabela], idConfiguracaoParam, configuracoesParam)){
				 populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
				 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
				 torneioEntrou[0] = true;
			 }
		 }
	 }

}




/**
 * Realiza a inserção de uma nova configuração na subpopulação de número de manobras correspondente.
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param numeroMinimoManobrasManuais
 * @param torneioQtdInclusoesManobras
 */
void insereConfiguracaoTabelaSolucoesFactiveis(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, int *torneioQtdInclusoesManobras) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    double tmpNovo, tmpAntigo, tmpAntigoContador;
    double ponderacaoManobras;

    torneioQtdInclusoesManobras[0] = 0;
    tmpNovo = funcaoDistanciaFactivel(configuracoesParam, idConfiguracaoParam);

    //setima tabela - solucoes factiveis
    idTabela = idTabelaParam;
    manobrasTmp = -1;
    indiceTmp = 0;

    if (verificaFactibilidade(configuracoesParam[idConfiguracaoParam],maxCarregamentoRede,maxCarregamentoTrafo,maxQuedaTensao)) {
    //a tabela não está completa ainda
        if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
            posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            populacaoParam[idTabela].numeroIndividuos++;
            torneioQtdInclusoesManobras[0]++;
        } else //a tabela está cheia
        {
            //localiza a pior configuração presente na tabela
            for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao; //pior elemento da tabela
                idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao; //elemento a ser comparado
                if ((manobrasTmp < populacaoParam[idTabela].tabela[contador].valorObjetivo) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos < configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca < configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede < configuracoesParam[idConf].objetivo.maiorCarregamentoRede) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede == configuracoesParam[idConf].objetivo.maiorCarregamentoRede && configuracoesParam[idTmp].objetivo.menorTensao > configuracoesParam[idConf].objetivo.menorTensao) ||
                        (manobrasTmp == populacaoParam[idTabela].tabela[contador].valorObjetivo && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConf].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede == configuracoesParam[idConf].objetivo.maiorCarregamentoRede && configuracoesParam[idTmp].objetivo.menorTensao == configuracoesParam[idConf].objetivo.menorTensao && configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo < configuracoesParam[idConf].objetivo.maiorCarregamentoTrafo)) {

                    indiceTmp = contador;
                    manobrasTmp = populacaoParam[idTabela].tabela[contador].valorObjetivo;
                }
            }
            idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
            //verifica se a nova solução é melhor do que a pior solução encontrada
            ponderacaoManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + ((double) configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) / fatorPonderacaoManobras;

            if ((manobrasTmp > configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos > configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca > configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual > configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual > configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas > configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede > configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede && configuracoesParam[idTmp].objetivo.menorTensao < configuracoesParam[idConfiguracaoParam].objetivo.menorTensao) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idTmp].objetivo.consumidoresEspeciaisTransferidos == configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede == configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede && configuracoesParam[idTmp].objetivo.menorTensao == configuracoesParam[idConfiguracaoParam].objetivo.menorTensao && configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo > configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo)) {


            populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;;
                torneioQtdInclusoesManobras[0]++;
            }
        }
    }


}

/** Por Leandro
 *
 * Descrição: esta função seleciona exclusivamente os indivíduos FACTÍVEIS GERAL por meio da ordenação por não dominância em relação aos valores de
 * Energia TOTAL Não Suprida aos Consumidores e Numero TOTAL de Manobras.
 * A finalidade desta tabela é permitir que sejam mantidos na população indivíduos que, embora factíveis, não tenham se qualificados para serem salvos em alguma das
 * demais tabelas, especialmente na tabela gerenciada pela função "insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsTotalManobras()".
 *
 * a) Esta função substituiu a função "insereConfiguracaoTabelaSolucoesFactiveis()", que armazena as soluções factíveis obtidas ao longo do processo evolutivo e que minimizam alguns parametros
 * relacionados ao número de consumidores especiais transferidos e ao número de manobras;
 * b) Esta função consiste na função "insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsTotalManobras()" modificada para permitir o armazenamento apenas de soluções que atendem as
 * restrições gerais do problema
 *
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */

void insereConfiguracaoTabelaSolucoesFactiveisDominanciaEnergiaTotalNaoSupridaVsTotalManobras(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ,contadorIndividuosVerificados;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;
    int numIndividuos, numeroIndividuosUltimaFronteira;

//    torneioEntrou[0] = false;

    if (verificaFactibilidade(configuracoesParam[idConfiguracaoParam], maxCarregamentoRede, maxCarregamentoTrafo, maxQuedaTensao)) {
		//Nesse caso a tabela não está cheia ainda
		if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

			configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
			populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			populacaoParam[idTabelaParam].numeroIndividuos++;
	//		torneioEntrou[0] = true;

		} else {

			if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
				FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

				for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
					fronteira[contadorI].numeroIndividuos = 0;
					fronteira[contadorI].nivelDominancia = 0;
					fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
				}

				individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
				//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
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

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsTotalManobras(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

					double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
					int menorDist = 0;
					long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
					for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
						if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
							tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
							menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
							idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
						}
					}

					//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
								//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
								idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
								if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
								   && (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
								   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
	//									torneioEntrou[0] = true;
									}
								}



							}
						}

					}
					//zera as distancias inicialmente
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
						populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

					free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
							for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

								//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
								idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
								if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
								   && (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
								   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
	//									torneioEntrou[0] = true;
									}
								}


							}
						}
					}
				}
				for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
					fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
				fronteira  = NULL; free(fronteira);
				individuos = NULL; free(individuos);
			}
		}
	}
}

/** Por Leandro
 *
 * Descrição: consiste na função "insereConfiguracaoTabelaSolucoesFactiveisDominanciaEnergiaTotalNaoSupridaVsTotalManobras()" modificada para
 * (a) verificar, além da adequação de "idConfiguracaoParam" às restrições gerais, a factibilidade da sequência de chaveamento  de "idConfiguracaoParam";
 * (b) passar como parâmentro o "vetorPiParam", necessário a execução da modificação 'a';
 * (c) verificar e informar se "idConfiguracaoParam" foi adicionado a tabela
 *
 * P.S.: a função "verificaFactibilidadeSequenciaChaveamento()" verifica tanto as restrições gerais quanto a factibilidade da sequência de chaveamento  de "idConfiguracaoParam";
 *
 *
 * @param idTabelaParam é o identificador da tabela
 * @param populacaoParam é a estrutura que armazena as informaçẽos das tabelas de subpopulaão
 * @param idConfiguracaoParam é o identificador do novo indivíduo (ou solução ou configuração) gerada
 * @param configuracoesParam é a estrutura de armazena as informações dos indivíduos
 * @param torneioEntrou é uma variável booleana que salva a informação da inserção ou não do novo indivíduo na tabela. Ela foi criada para auxiliar a execução de um tornei de tabelas. Contudo, este código não aplica-o e a informação salva nesta variável não é usada
 */

void insereConfiguracaoTabelaSolucoesFactiveisDominanciaEnergiaTotalNaoSupridaVsTotalManobrasModificada(int idTabelaParam, VETORTABELA *populacaoParam, int idConfiguracaoParam, CONFIGURACAO *configuracoesParam, VETORPI *vetorPiParam, BOOL *torneioEntrou) {
    int rank = 0, idFronteira = 0, contadorI, contadorJ,contadorIndividuosVerificados;
    long int idConfiguracao1, idConfiguracao2;
    TABELA *individuos, *individuosUltimaFronteira;
    int numIndividuos, numeroIndividuosUltimaFronteira;

    torneioEntrou[0] = false;

    if (verificaFactibilidadeSequenciaChaveamento(configuracoesParam, vetorPiParam, idConfiguracaoParam)) {
		//Nesse caso a tabela não está cheia ainda
		if (populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo) { //Se a subpopulação ainda não está completa, o novo indivíduo é diretamente armazenado

			configuracoesParam[idConfiguracaoParam].objetivo.rank = 0;
			configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 0; //Só haverá determinação do nível de dominancia de cada individuo quando a tabela estiver completa e for iniciado o processo de evolução da subopopulação
			populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
			populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
			populacaoParam[idTabelaParam].numeroIndividuos++;
			torneioEntrou[0] = true;

		} else {

			if(populacaoParam[idTabelaParam].tamanhoMaximo > 0){
				FRONTEIRAS *fronteira =  Malloc(FRONTEIRAS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));

				for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++){
					fronteira[contadorI].numeroIndividuos = 0;
					fronteira[contadorI].nivelDominancia = 0;
					fronteira[contadorI].individuos = Malloc(INDIVIDUOSNAODOMINADOS, (populacaoParam[idTabelaParam].tamanhoMaximo + 1));
				}

				individuos = Malloc(TABELA, (populacaoParam[idTabelaParam].numeroIndividuos + 1));
				//Salva numa variável temporárias as informaçẽos de todos os indivíduos já salvos na tabela e do novo indivíduo
				for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++) {
					individuos[contadorI].idConfiguracao = populacaoParam[idTabelaParam].tabela[contadorI].idConfiguracao;
					individuos[contadorI].valorObjetivo = 0;
					individuos[contadorI].distanciaMultidao = 0;
					configuracoesParam[individuos[contadorI].idConfiguracao].objetivo.fronteira = 999;
				}
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = idConfiguracaoParam;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = 0;
				individuos[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
				configuracoesParam[idConfiguracaoParam].objetivo.fronteira = 999;

				idFronteira = 0;
				contadorIndividuosVerificados = 0;
				while(contadorIndividuosVerificados < populacaoParam[idTabelaParam].numeroIndividuos + 1){ //"1" é o número de novos indivíduos gerados
					//Determina a qual fronteira cada indivíduo pertence
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorI++) {
						idConfiguracao1 = individuos[contadorI].idConfiguracao;
						rank = 0;

						if(configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){ //Se idConfiguracao1 ainda não estiver salva em alguma fronteira
							for (contadorJ = 0; contadorJ < populacaoParam[idTabelaParam].numeroIndividuos +1; contadorJ++) {
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

						//verificar se idConfiguracao1 pode ser salvo na fronteira "idFronteira" (rank igual a 0 significa que idConfiguracao1 não é dominada por nenhuma das configurações que aida não foram classificadas em alguma fronteira; configuracoesParam[idConfiguracao1].objetivo.fronteira significa que tal configuração não está em nenhuma das fronteiras já verificadas
						if(rank == 0 && configuracoesParam[idConfiguracao1].objetivo.fronteira == 999){//Se for verdade, então idConfiguracao1 ainda não foi salvo em nenhuma fronteira e não foi dominado por nenhuma outra solução. Logo, ele pertençe a primeira fronteira
							numIndividuos = fronteira[idFronteira].numeroIndividuos;
							fronteira[idFronteira].individuos[numIndividuos].idConfiguracao = idConfiguracao1;
							fronteira[idFronteira].nivelDominancia = idFronteira;
							fronteira[idFronteira].numeroIndividuos++;
							//configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
							contadorIndividuosVerificados++;
						}


					}
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						idConfiguracao1 = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						configuracoesParam[idConfiguracao1].objetivo.fronteira = idFronteira;
					}

					idFronteira++;
				}

				//Salva na tabelas os indivíduos segundo a fronteira a qual pertencem
				idFronteira--; //Decrementa para salvar o id da última fronteira construída
				if(fronteira[idFronteira].numeroIndividuos > 1){ //Se não última fronteira há mais de um indivíduo, então deve calcular a distância de multidão a fim de excluir aquele com o menor valor de distância de multidao
					//Determina distância de multidão dos indivíduos e o indivíduo da última fronteira que possui o menor valor de distancia de multida. Este não será salvo na tabela

					numeroIndividuosUltimaFronteira = 0;
					individuosUltimaFronteira = Malloc(TABELA, (fronteira[idFronteira].numeroIndividuos + 1));
					//Copia somente os indivíduos que estão na última fronteira
					for(contadorJ = 0; contadorJ < fronteira[idFronteira].numeroIndividuos; contadorJ++){
						individuosUltimaFronteira[contadorJ].idConfiguracao = fronteira[idFronteira].individuos[contadorJ].idConfiguracao;
						individuosUltimaFronteira[contadorJ].valorObjetivo  = fronteira[idFronteira].nivelDominancia;
						individuosUltimaFronteira[contadorJ].distanciaMultidao = 0;
						numeroIndividuosUltimaFronteira++;
					}
					crowndingEnergiaTotalNaoSupridaVsTotalManobras(2, numeroIndividuosUltimaFronteira, individuosUltimaFronteira, configuracoesParam);

					double tmpSol = individuosUltimaFronteira[0].distanciaMultidao;//1.0e14;
					int menorDist = 0;
					long int idConfiguracaoMenorDist = individuosUltimaFronteira[0].idConfiguracao;//;
					for (contadorI = 0; contadorI < numeroIndividuosUltimaFronteira; contadorI++) { //Pesquisa pelo indivíduo com menor distancia de multidao
						if (tmpSol > individuosUltimaFronteira[contadorI].distanciaMultidao) {
							tmpSol = individuosUltimaFronteira[contadorI].distanciaMultidao;
							menorDist = contadorI; //posição do indivíduo com menor distancia de multidaoa
							idConfiguracaoMenorDist = individuosUltimaFronteira[contadorI].idConfiguracao;
						}
					}

					//Copia para a tabelas todos os indivíduos salvos em "fronteira" exceto aquele que possui id igual a idConfiguracaoMenorDist, pois ele é o pior indivíduo e não devera estar na tabela

					populacaoParam[idTabelaParam].numeroIndividuos = 0;
					for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
						for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){
							if(fronteira[contadorI].individuos[contadorJ].idConfiguracao != idConfiguracaoMenorDist){ //Copia todos os indivíduos, exceto o pior (idConfiguracaoMenorDist)
								//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
								idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
								if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
								   && (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
								   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										if(idConfiguracaoParam == populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao)
											torneioEntrou[0] = true;
									}
								}



							}
						}

					}
					//zera as distancias inicialmente
					for (contadorI = 0; contadorI < populacaoParam[idTabelaParam].numeroIndividuos; contadorI++)
						populacaoParam[idTabelaParam].tabela[contadorI].distanciaMultidao = 0;

					free(individuosUltimaFronteira);

				}
				else{
					if(fronteira[idFronteira].numeroIndividuos == 1){ //Se não última fronteira há apenas um indivíduo, então já é, diretamente, o pior e deverá ser descartado
						populacaoParam[idTabelaParam].numeroIndividuos = 0;
						for(contadorI = 0; contadorI <= idFronteira; contadorI++){ //Copiar os individuos de "fronteira" para dentro das tabelas
							for(contadorJ = 0; contadorJ < fronteira[contadorI].numeroIndividuos; contadorJ++){

								//Esta mudança a baixo serve para impedir o armazenemento de duas soluções os mesmos valores para Objetivo 1 e 2
								idConfiguracao1 = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
								if(!(configuracoesParam[idConfiguracao1].objetivo.energiaTotalNaoSuprida == configuracoesParam[idConfiguracaoParam].objetivo.energiaTotalNaoSuprida
								   && (configuracoesParam[idConfiguracao1].objetivo.manobrasManuais + configuracoesParam[idConfiguracao1].objetivo.manobrasAutomaticas) == (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas)
								   && idConfiguracao1 != idConfiguracaoParam)){//Se a nova solução gerada possuir os mesmos valores de Objetivo 1 e Objetivo de alguma solução já salva, esta última será descartada.
									if(populacaoParam[idTabelaParam].numeroIndividuos < populacaoParam[idTabelaParam].tamanhoMaximo){
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].idConfiguracao = fronteira[contadorI].individuos[contadorJ].idConfiguracao;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].valorObjetivo = fronteira[contadorI].nivelDominancia;
										populacaoParam[idTabelaParam].tabela[populacaoParam[idTabelaParam].numeroIndividuos].distanciaMultidao = 0;
										populacaoParam[idTabelaParam].numeroIndividuos++;
										torneioEntrou[0] = true;
									}
								}


							}
						}
					}
				}
				for(contadorI = 0; contadorI < populacaoParam[idTabelaParam].tamanhoMaximo + 1; contadorI++)
					fronteira[contadorI].individuos = NULL, free(fronteira[contadorI].individuos);
				fronteira  = NULL; free(fronteira);
				individuos = NULL; free(individuos);
			}
		}
	}
}


/**
 * Insere uma nova configuração na subpopulacao com o objetivo de reduzir as manobras que envolvam consumidores especiais.
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @param torneioQtdInclusoesManobras
 */
void insereConfiguracaoTabelaConsumidoresEspeciais(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam,
        long int idConfiguracaoParam, int *torneioQtdInclusoesManobras) {
    int idTabela;
    int posicao;
    int contador, indiceTmp;
    long int idTmp, idConf;
    double ponderacaoTmp = -1;

    torneioQtdInclusoesManobras[0] = 0;

    double ponderacaoManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;

    //tabela - sem consumidores especiais transferidos e que minimiza o número ponderado de Manobras
    idTabela = idTabelaParam;
    if (configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos == 0) {

        //a tabela não está cheia insere a configuracao
        //printf("\nidTabela: %d - NumIndiv: %ld", idTabela, populacaoParam[idTabela].numeroIndividuos);
        if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
            posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = ponderacaoManobras;
            populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
            populacaoParam[idTabela].numeroIndividuos++;
            torneioQtdInclusoesManobras[0]++;
        } else //a tabela está cheia
        {
            ponderacaoTmp = -1;
            //localiza a pior configuração presente na tabela
            for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                if (ponderacaoTmp < populacaoParam[idTabela].tabela[contador].valorObjetivo) {
                    indiceTmp = contador;
                    ponderacaoTmp = populacaoParam[idTabela].tabela[contador].valorObjetivo;

                }
            }
            //verifica se a nova solução é melhor do que a pior solução encontrada
            if (ponderacaoManobras < ponderacaoTmp) {
                populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = ponderacaoManobras;
                torneioQtdInclusoesManobras[0]++;
            }
        }
    }

    //tabela - minimiza número de consumidores especiais transferidos e, com prioridade menor, o número ponderado de Manobras
    idTabela = idTabelaParam + 1;

	//a tabela não está cheia insere a configuracao
	if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
		posicao = populacaoParam[idTabela].numeroIndividuos;
		populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
		populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos;
		populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
		populacaoParam[idTabela].numeroIndividuos++;
                torneioQtdInclusoesManobras[0]++;
	} else //a tabela está cheia
	{
		int consumidoresEspeciaisTransferidosTmp = -1;
		double ponderacaoConf;
		ponderacaoTmp = -1;
		indiceTmp = 0;
		//localiza a pior configuração presente na tabela
		for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
			idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
			ponderacaoConf = configuracoesParam[idConf].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idConf].objetivo.manobrasManuais;
			if ((consumidoresEspeciaisTransferidosTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
                                (consumidoresEspeciaisTransferidosTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) &&
                                ponderacaoTmp < ponderacaoConf))
                        {
                            indiceTmp = contador;
                            consumidoresEspeciaisTransferidosTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;
                            idTmp = populacaoParam[idTabela].tabela[contador].idConfiguracao;
                            ponderacaoTmp = configuracoesParam[idTmp].objetivo.manobrasAutomaticas / fatorPonderacaoManobras + configuracoesParam[idTmp].objetivo.manobrasManuais;
			}
		}
		//verifica se a nova solução é melhor do que a pior solução encontrada
		if((configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos  < consumidoresEspeciaisTransferidosTmp) ||
                        (configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos == consumidoresEspeciaisTransferidosTmp &&
                        ponderacaoManobras < ponderacaoTmp))
                {
                    populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                    populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.consumidoresEspeciaisTransferidos;
                    torneioQtdInclusoesManobras[0]++;
		}
	}
}

/**
 * Por Leandro:
 * Realiza a inserção de uma nova configuração nas tabelas que priorizam o atendimento de consumidores prioritários
 * enquanto minimiza a potência Total Transferida entre alimentadores energizados (para alívio de carregamentos) para a geração de um novo indivíduo.
 * Esta função é executa somente se a sequência para execução das manobras em pares for ABRE-FECHA, uma vez que esta leva a um desligamento teporário de cargas
 * energizadas. Assim, esta função visa priorizar individuos que minimizam os impactos da transferência de cargas para alívio de alimentadores.
 * Esta considera os múltiplos níveis de prioridade de atendimento entre os consumidores
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 */
void insereConfiguracaoTabelaPotenciaTransferida(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    double tmpNovo, tmpAntigo, tmpAntigoContador;
    double ponderacaoManobras;

    idTabela = idTabelaParam;

    if(idTabela != -1){
		 if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {  //Se a tabela não está cheia, então insere a configuração
			 posicao = populacaoParam[idTabela].numeroIndividuos;
			 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
			 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta;
			 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
			 populacaoParam[idTabela].numeroIndividuos++;
		 } else //a tabela está cheia
		 {
			 indiceTmp = 0;
			 //localiza a pior configuração presente na tabela
			 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
				 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
				 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;

				 if (     (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) >  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) ||
						 ((floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeIntermediaria) >  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria))) ||
						 ((floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeBaixa) >  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa))) ||
						 ((floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresSemPrioridade) > floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresSemPrioridade)) )
					){
					 indiceTmp = contador; //Indice da posição da tabela na qual está o pior indivíduo
				 }
			 }
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 //verifica se a nova solução é melhor do que a pior solução encontrada
			 if (     (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) ||
					 ((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria))) ||
					 ((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa))) ||
					 ((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade) < floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresSemPrioridade)) )
				) {
				 populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
				 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta;
			 }
		 }
    }
}

/**
 * Por Leandro:
 * Consiste na função modifica para "insereConfiguracaoTabelaPotenciaTransferida()"
 * (a) para impedir o armazenamento de um novo indivíduo se este possuir o mesmo valor de objetivo de algum dos indivíduos já salvos
 * (b) verificar e informar se "idConfiguracaoParam" foi adicionado a tabela
 *
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 */
void insereConfiguracaoTabelaPotenciaTransferidaModificada(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam, BOOL *torneioEntrou) {
    int idTabela;
    int posicao;
    int contador, indiceTmp;
    long int idTmp, idConf;

    torneioEntrou[0] = false;

    idTabela = idTabelaParam;

    if(idTabela != -1){
		 if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {  //Se a tabela não está cheia, então insere a configuração
			 if(!verificaSeHaValorIgualJaSalvoTabelaPotenciaTransferida(populacaoParam[idTabela], idConfiguracaoParam, configuracoesParam)){
				 posicao = populacaoParam[idTabela].numeroIndividuos;
				 populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
				 populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta;
				 populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
				 populacaoParam[idTabela].numeroIndividuos++;
				 torneioEntrou[0] = true;
			 }
		 } else //a tabela está cheia
		 {
			 indiceTmp = 0;
			 //localiza a pior configuração presente na tabela
			 for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
				 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
				 idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;

				 if (     (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) >  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) ||
						 ((floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeIntermediaria) >  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria))) ||
						 ((floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeBaixa) >  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa))) ||
						 ((floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConf].potenciaTransferida.consumidoresSemPrioridade) > floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresSemPrioridade)) )
					){
					 indiceTmp = contador; //Indice da posição da tabela na qual está o pior indivíduo
				 }
			 }
			 idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
			 //verifica se a nova solução é melhor do que a pior solução encontrada
			 if (     (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) <  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) ||
					 ((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) <  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria))) ||
					 ((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa) <  floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa))) ||
					 ((floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeAlta)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeIntermediaria) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeIntermediaria)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeBaixa) == floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresPrioridadeBaixa)) && (floor(configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresSemPrioridade) < floor(configuracoesParam[idTmp].potenciaTransferida.consumidoresSemPrioridade)) )
				) {
				 if(!verificaSeHaValorIgualJaSalvoTabelaPotenciaTransferida(populacaoParam[idTabela], idConfiguracaoParam, configuracoesParam)){
					 populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
					 populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].potenciaTransferida.consumidoresPrioridadeAlta;
					 torneioEntrou[0] = true;
				 }
			 }
		 }
    }
}

/*
void insereConfiguracaoTabelaManobrasAntigo(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    //identifica em qual tabela de manobras a configuração será inserida
    //primeira tabela - sem manobras manuais
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == 0) {
        idTabela = idTabelaParam;
    } else {
        if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais >= 1) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 4)) {
            //segunda tabela - entre [1-4] manobras manuais
            idTabela = idTabelaParam + 1;
        } else {
            if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 8)) {
                //terceira tabela - entre [5-8] manobras manuais
                idTabela = idTabelaParam + 2;
            } else {
                if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 12)) {
                    //quarta tabela - entre [9-12] manobras manuais
                    idTabela = idTabelaParam + 3;
                } else {
                    if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 16)) {
                        //quinta tabela - entre [13-16] manobras manuais
                        idTabela = idTabelaParam + 4;
                    }

                }
            }
        }

    }
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 16) {
        //a tabela não está cheia insere a configuracao
        if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
            posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
            populacaoParam[idTabela].numeroIndividuos++;
        } else //a tabela está cheia
        {
            manobrasTmp = -1;
            indiceTmp = 0;
            //localiza a pior configuração presente na tabela
            for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
                idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
                if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.menorTensao > configuracoesParam[idConf].objetivo.menorTensao) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.menorTensao == configuracoesParam[idConf].objetivo.menorTensao && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede < configuracoesParam[idConf].objetivo.maiorCarregamentoRede) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.menorTensao == configuracoesParam[idConf].objetivo.menorTensao && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede == configuracoesParam[idConf].objetivo.maiorCarregamentoRede && configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo < configuracoesParam[idConf].objetivo.maiorCarregamentoTrafo)) {
                    indiceTmp = contador;
                    manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;

                }
            }
            idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
            //verifica se a nova solução é melhor do que a pior solução encontrada
            if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao > configuracoesParam[idTmp].objetivo.menorTensao) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao == configuracoesParam[idTmp].objetivo.menorTensao && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao == configuracoesParam[idTmp].objetivo.menorTensao && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede == configuracoesParam[idTmp].objetivo.maiorCarregamentoRede && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo < configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo)) {
                populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            }

        }
    }
    //sexta tabela - ponderação de manobras
    idTabela = idTabelaParam+5;
    double ponderacaoManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + ((double) configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) / fatorPonderacaoManobras;
    double ponderacaoTmp = -1;
    //a tabela não está completa ainda
    if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
        posicao = populacaoParam[idTabela].numeroIndividuos;
        populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
        populacaoParam[idTabela].tabela[posicao].valorObjetivo = ponderacaoManobras;
        populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
        populacaoParam[idTabela].numeroIndividuos++;
    } else //a tabela está cheia
    {
        //localiza a pior configuração presente na tabela
        for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
            if (ponderacaoTmp < populacaoParam[idTabela].tabela[contador].valorObjetivo) {
                indiceTmp = contador;
                ponderacaoTmp = populacaoParam[idTabela].tabela[contador].valorObjetivo;

            }
        }
        //verifica se a nova solução é melhor do que a pior solução encontrada
        if (ponderacaoManobras < ponderacaoTmp) {
            populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = ponderacaoManobras;
        }

    }
//setima tabela - solucoes factiveis
    idTabela = idTabelaParam+6;
    manobrasTmp = -1;
    indiceTmp = 0;
    //a tabela não está completa ainda
    if (verificaFactibilidade(configuracoesParam[idConfiguracaoParam],maxCarregamentoRede,maxCarregamentoTrafo,maxQuedaTensao)) {
        if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
            posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + ((double) configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) / fatorPonderacaoManobras;
            populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
            populacaoParam[idTabela].numeroIndividuos++;
        } else //a tabela está cheia
        {
            //localiza a pior configuração presente na tabela
            for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao; //pior elemento da tabela
                idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao; //elemento a ser comparado
                if ((((double) populacaoParam[idTabela].tabela[contador].valorObjetivo) > manobrasTmp) ||
                        (((double) populacaoParam[idTabela].tabela[contador].valorObjetivo) == manobrasTmp && configuracoesParam[idConf].objetivo.maiorCarregamentoRede > configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) ||
                        (((double) populacaoParam[idTabela].tabela[contador].valorObjetivo) == manobrasTmp && configuracoesParam[idConf].objetivo.maiorCarregamentoRede == configuracoesParam[idTmp].objetivo.maiorCarregamentoRede && configuracoesParam[idConf].objetivo.menorTensao < configuracoesParam[idTmp].objetivo.menorTensao) ||
                        (((double) populacaoParam[idTabela].tabela[contador].valorObjetivo) == manobrasTmp && configuracoesParam[idConf].objetivo.maiorCarregamentoRede == configuracoesParam[idTmp].objetivo.maiorCarregamentoRede && configuracoesParam[idConf].objetivo.menorTensao == configuracoesParam[idTmp].objetivo.menorTensao && configuracoesParam[idConf].objetivo.maiorCarregamentoTrafo > configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo)) {

                    indiceTmp = contador;
                    manobrasTmp = (double) populacaoParam[idTabela].tabela[contador].valorObjetivo; //ponderacao de manobras do pior elemento da tabela
                }
            }
            idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
            //verifica se a nova solução é melhor do que a pior solução encontrada
            ponderacaoManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + ((double) configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas) / fatorPonderacaoManobras;
            if((ponderacaoManobras < manobrasTmp) ||
                (ponderacaoManobras == manobrasTmp && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) ||
                (ponderacaoManobras == manobrasTmp && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede == configuracoesParam[idTmp].objetivo.maiorCarregamentoRede && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao > configuracoesParam[idTmp].objetivo.menorTensao) ||
                (ponderacaoManobras == manobrasTmp && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede == configuracoesParam[idTmp].objetivo.maiorCarregamentoRede && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao == configuracoesParam[idTmp].objetivo.menorTensao && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo < configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo)) {
                populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = ponderacaoManobras;
            }
        }
    }
}
*/
/**
 * Por Leandro:
 * Consiste na função "insereTabelas()" modificada para
 * 1) a exclusão de comandos relacionados ao torneio de tabelas, uma vez que este não existe mais neste AEMO
 * 2) para realizar a chamada das funções para as novas tabelas
 * 3) para que seja passado como parâmentro o indentificador da configuração que é concentualmente a inicial do problema. Este será utilizado na determinação
 * da adequação de um indivíduo G aos critérios para inserção nas tabelas que minimizam a ENS e priorizam a reconexão de consumidores prioritários
 * 4) para sejam passados por parametros a sequência de chaveamento adotada para as manobras que ocorrem em par e a informação se as manobras ocorreram em par
 * 5) efetuar a passagem de "numeroTotalManobrasRestauracaoCompletaSemAlivioParam"
 * 6) deixar de efetuar a passagem de "numeroMinimoManobrasManuais"
 *
 *
 * Realiza a inserção de uma nova configuração em cada uma das subpopulações utilizadas. 
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracao
 * @param numeroMinimoManobrasManuais
 * @param tabelaEscolhida
 * @param sucessoAplicacaoOperadorParam
 */
void insereTabelasModificada(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao, long int idPrimeiraConfiguracaoParam,
		int numeroTotalManobrasRestauracaoCompletaSemAlivioParam, int *sucessoAplicacaoOperadorParam, SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, BOOL flagManobrasAlivioParam) {
//    BOOL torneioEntrou;
//    int idTabela = 0, indice;
//    int torneioQtdInclusoesManobras = 0;
//    sucessoAplicacaoOperadorParam[0] = 0;
//
//    //1 TABELA PARA MINIMIZAÇÃO DO CARREGAMENTO DE REDE (idTabela = 0)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede,
//            configuracoesParam, false, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DO CARREGAMENTO DE TRAFO (idTabela = 1)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo,
//            configuracoesParam, false, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DA QUEDAS DE TENSÃO (idTabela = 2)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.menorTensao,
//            configuracoesParam, true, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DAS PERDAS RESISTIVAS (idTabela = 3)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.perdasResistivas,
//            configuracoesParam, false, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DA FUNÇÃO PONDERAÇÃO NORMALIZADA(idTabela = 4)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.ponderacao, configuracoesParam,
//            false, &torneioEntrou);
//    idTabela++;
//
//    //5 TABELAS PARA MINIMIZAÇÃO DO NÚMERO DE MANOBRAS E PRIORIZAÇÃO DE MANOBRAS EM CHAVES CONTROLADAS REMOTAMENTE
//    insereConfiguracaoTabelasManobrasV3(idTabela, populacaoParam, configuracoesParam, idConfiguracao, numeroTotalManobrasRestauracaoCompletaSemAlivioParam);
//    idTabela += 5;
//
//    //5 TABELAS PARA PRIORIZAÇÃO DE ATENDIMENTO DE CONSUMIDORES PRIORITÁRIOS E MINIMIZAÇÃO DA ENERGIA NÃO SUPRIDA
//    insereConfiguracaoTabelasEnergiaNaoSuprida(idTabela, populacaoParam, configuracoesParam, idConfiguracao, idPrimeiraConfiguracaoParam);
//    idTabela += 5;
//
//    //9 TABELAS QUE UTILIZAM A SELEÇÃO POR NÃO-DOMINANCIA PRESENTE NO NSGA-II PARA A FORMAÇÃO DA SUBPOPULAÇÃO
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsTotalManobras(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//	idTabela++;
//
//    //1 TABELA PARA ARMAZENAMENTO DE SOLUÇÕES FACTÍVEIS SELECIONADAS PELO GRAU DE DOMINÂNCIA EM RELAÇÃO AOS OBJETIVOS DE MINIMIZAR ENS_TOT E MAN_TOT
//	insereConfiguracaoTabelaSolucoesFactiveisDominanciaEnergiaTotalNaoSupridaVsTotalManobras(idTabela, populacaoParam, idConfiguracao, configuracoesParam);
//    idTabela++;
//
////    //Verifica se "idConfiguracao" foi inserido na tabela que armazena as sluções Factíveis selecionados pelo grau de dominância em relação a ENS_TOT e MAN_TOT
////    for(indice = 0; indice < populacaoParam[idTabela - 1].numeroIndividuos && sucessoAplicacaoOperadorParam[0] == 0; indice++){
////    	if( populacaoParam[idTabela - 1].tabela[indice].idConfiguracao == idConfiguracao)
////    		sucessoAplicacaoOperadorParam[0] = 1;
////    }
//
//    // 1 TABELA PARA MINIMIZAR POTÊNCIA ATIVA TRANSFERIDA, POR NÍVEL DE PRIORIDADE.
//    // ELA É ATIVADA SOMENTE SE A SEQUÊNCIA ABRE-FECHA FOI ESCOLHIDA E AS MANOBRAS OCORRERAM EM PAR PARA A GERAÇÃO DO NOVO INDIVÍDUO
//    if(flagManobrasAlivioParam && sequenciaManobrasEmParParam == abreFecha){
//		insereConfiguracaoTabelaPotenciaTransferida(idTabela, populacaoParam, configuracoesParam, idConfiguracao);
//		idTabela += 1;
//    }

}

/**
 * Por Leandro:
 * Consiste na função "insereTabelasModificada()" modificada para
 * 1) passar como parâmentro o "vetorPiParam", necessário a execução da função "insereConfiguracaoTabelaSolucoesFactiveisDominanciaEnergiaTotalNaoSupridaVsTotalManobrasModificada()"
 * 2) chamar as funções que verificam a existencia de indivíduos iguais já salvos nas tabelas
 * 3) deixar de passar o parâmetro "sucessoAplicacaoOperadorParam"
 * 4) para passar como parâmetro "flagAtualizacaoPopulacaoParam", para informar se o novo indivíduo foi salvo em alguma tabela
 * 5) para para verificar e salvar em "flagAtualizacaoPopulacaoParam" a informação de que o novo indivíduo foi ou não salvo em alguma tabela
 *
 * Realiza a inserção de uma nova configuração em cada uma das subpopulações utilizadas.
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracao
 * @param numeroMinimoManobrasManuais
 * @param tabelaEscolhida
 * @param sucessoAplicacaoOperadorParam
 */
void insereTabelasModificadaV2(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao, long int idPrimeiraConfiguracaoParam, int numeroTotalManobrasRestauracaoCompletaSemAlivioParam,
		SEQUENCIAMANOBRASALIVIO sequenciaManobrasEmParParam, BOOL flagManobrasAlivioParam, VETORPI *vetorPiParam, BOOL *flagAtualizacaoPopulacaoParam) {
    BOOL torneioEntrou;
    int idTabela = 0;
    flagAtualizacaoPopulacaoParam[0] = false;

    //1 TABELA PARA MINIMIZAÇÃO DO CARREGAMENTO DE REDE (idTabela = 0)
    insereConfiguracaoTabelaModificada(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede,
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true; //Verifica se "idConfiguracao" foi adicionado a tabela

    //1 TABELA PARA MINIMIZAÇÃO DO CARREGAMENTO DE TRAFO (idTabela = 1)
    insereConfiguracaoTabelaModificada(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo,
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //1 TABELA PARA MINIMIZAÇÃO DA QUEDAS DE TENSÃO (idTabela = 2)
    insereConfiguracaoTabelaModificada(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.menorTensao,
            configuracoesParam, true, &torneioEntrou);
    idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //1 TABELA PARA MINIMIZAÇÃO DAS PERDAS RESISTIVAS (idTabela = 3)
    insereConfiguracaoTabelaModificada(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.perdasResistivas,
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //1 TABELA PARA MINIMIZAÇÃO DA FUNÇÃO PONDERAÇÃO NORMALIZADA(idTabela = 4)
    insereConfiguracaoTabelaModificada(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.ponderacao, configuracoesParam,
            false, &torneioEntrou);
    idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //5 TABELAS PARA MINIMIZAÇÃO DO NÚMERO DE MANOBRAS E PRIORIZAÇÃO DE MANOBRAS EM CHAVES CONTROLADAS REMOTAMENTE (idTabela = 5 a 9)
    insereConfiguracaoTabelasManobrasV3Modificada(idTabela, populacaoParam, configuracoesParam, idConfiguracao, numeroTotalManobrasRestauracaoCompletaSemAlivioParam, &torneioEntrou);
    idTabela += 5;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //5 TABELAS PARA PRIORIZAÇÃO DE ATENDIMENTO DE CONSUMIDORES PRIORITÁRIOS E MINIMIZAÇÃO DA ENERGIA NÃO SUPRIDA (idTabela = 10 a 14)
    insereConfiguracaoTabelasEnergiaNaoSupridaModificada(idTabela, populacaoParam, configuracoesParam, idConfiguracao, idPrimeiraConfiguracaoParam, &torneioEntrou);
    idTabela += 5;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //9 TABELAS QUE UTILIZAM A SELEÇÃO POR NÃO-DOMINANCIA PRESENTE NO NSGA-II PARA A FORMAÇÃO DA SUBPOPULAÇÃO (idTabela = 15 a 23)
	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

	insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsTotalManobras(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
	idTabela++;
	if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    //1 TABELA PARA ARMAZENAMENTO DE SOLUÇÕES FACTÍVEIS SELECIONADAS PELO GRAU DE DOMINÂNCIA EM RELAÇÃO AOS OBJETIVOS DE MINIMIZAR ENS_TOT E MAN_TOT (idTabela = 24)
	insereConfiguracaoTabelaSolucoesFactiveisDominanciaEnergiaTotalNaoSupridaVsTotalManobrasModificada(idTabela, populacaoParam, idConfiguracao, configuracoesParam, vetorPiParam, &torneioEntrou);
    idTabela++;
    if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;

    // 1 TABELA PARA MINIMIZAR POTÊNCIA ATIVA TRANSFERIDA, POR NÍVEL DE PRIORIDADE. (idTabela = 25)
    // ELA É ATIVADA SOMENTE SE A SEQUÊNCIA ABRE-FECHA FOI ESCOLHIDA E AS MANOBRAS OCORRERAM EM PAR PARA A GERAÇÃO DO NOVO INDIVÍDUO
    if(flagManobrasAlivioParam && sequenciaManobrasEmParParam == abreFecha){
    	insereConfiguracaoTabelaPotenciaTransferidaModificada(idTabela, populacaoParam, configuracoesParam, idConfiguracao, &torneioEntrou);
		idTabela += 1;
		if(torneioEntrou) flagAtualizacaoPopulacaoParam[0] = true;
    }

}

//void insereTabelasModificada(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao, long int idPrimeiraConfiguracaoParam,
//		long int numeroMinimoManobrasManuais, int *sucessoAplicacaoOperadorParam) {
///*    //int nivelDominancia = 0;
//    BOOL tipoObjetivo = false;*/
//    BOOL torneioEntrou;
//    int idTabela = 0;
//    int torneioQtdInclusoesManobras = 0;
//    sucessoAplicacaoOperadorParam[0] = 0;
//
//    //1 TABELA PARA MINIMIZAÇÃO DA FUNÇÃO PONDERAÇÃO NORMALIZADA(idTabela = 0)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.ponderacao, configuracoesParam,
//            false, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DAS PERDAS RESISTIVAS (idTabela = 1)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.perdasResistivas,
//            configuracoesParam, false, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DA QUEDAS DE TENSÃO (idTabela = 2)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.menorTensao,
//            configuracoesParam, true, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DO CARREGAMENTO DE REDE (idTabela = 3)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede,
//            configuracoesParam, false, &torneioEntrou);
//    idTabela++;
//
//    //1 TABELA PARA MINIMIZAÇÃO DO CARREGAMENTO DE TRAFO (idTabela = 4)
//    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo,
//            configuracoesParam, false, &torneioEntrou);
//    idTabela++;
//
//    //5 TABELAS PARA MINIMIZAÇÃO DO NÚMERO DE MANOBRAS E PRIORIZAÇÃO DE MANOBRAS EM CHAVES CONTROLADAS REMOTAMENTE
//    insereConfiguracaoTabelaManobrasV2(idTabela, populacaoParam, configuracoesParam, idConfiguracao, numeroMinimoManobrasManuais,
//            &torneioQtdInclusoesManobras);
//    idTabela += 5;
//
//    //5 TABELAS PARA PRIORIZAÇÃO DE ATENDIMENTO DE CONSUMIDORES PRIORITÁRIOS E MINIMIZAÇÃO DA ENERGIA NÃO SUPRIDA
//    insereConfiguracaoTabelasEnergiaNaoSuprida(idTabela, populacaoParam, configuracoesParam, idConfiguracao, idPrimeiraConfiguracaoParam);
//    idTabela += 5;
//
//    //9 TABELAS QUE UTILIZAM A SELEÇÃO POR NÃO-DOMINANCIA PRESENTE NO NSGA-II PARA A FORMAÇÃO DA SUBPOPULAÇÃO
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasAutomaticas(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeAltaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeIntermediariaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaPrioridadeBaixaVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaNaoSupridaSemPrioridadeVsManobrasManuais(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//	insereConfiguracaoTabelaDominanciaEnergiaTotalNaoSupridaVsTotalManobras(idTabela, populacaoParam, idConfiguracao, configuracoesParam, &torneioEntrou);
//	idTabela++;
//
//    //1 TABELA PARA ARMAZENAMENTO DE SOLUÇÕES FACTÍVEIS
//    insereConfiguracaoTabelaSolucoesFactiveis(idTabela, populacaoParam, configuracoesParam, idConfiguracao, &torneioQtdInclusoesManobras);
//    idTabela++;
///*    if(torneioQtdInclusoesManobras > 0)
//        sucessoAplicacaoOperadorParam[0] = 1;*/
//
//    //2 TABELAS ANTIGAS PARA MINIMIZAR O NÚMERO DE TRANFERÊNCIAS DE CONSUMUIDORES ESPECIAIS
//    insereConfiguracaoTabelaConsumidoresEspeciais(idTabela, populacaoParam, configuracoesParam, idConfiguracao, &torneioQtdInclusoesManobras);
//    idTabela += 2;
//
//}


/*void processoEvolutivo(DADOSTRAFO *dadosTrafoParam,
        DADOSALIMENTADOR *dadosAlimentadorParam,
        CONFIGURACAO *configuracoesParam, VETORTABELA *populacaoParam, GRAFOSETORES *grafoSetoresParam,
        RNPSETORES *matrizSetoresParam, LISTACHAVES *listaChavesParam, MATRIZPI *matrizPiParam,
        VETORPI *vetorPiParam, MATRIZMAXCORRENTE *maximoCorrenteParam, MATRIZCOMPLEXA *ZParam,
        double VFParam, long int numeroBarras, long int numeroSetores, int numeroTrafos, ESTADOCHAVE *estadoInicial,
        long int maximoGeracoes, int numeroTabelas,
        double taxaOperadorPAO, long int idConfiguracao, int *indiceRegulador, DADOSREGULADOR *dadosRegulador, long int *numChaveAberta,
        long int *numChaveFechada, int numeroIndividuos) {
    long int geracao;
    long int idConfiguracaoSelecionada;
    double probabilidadeAplicacao;
    int contador;

    //obtém individuos iniciais
    for (contador = idConfiguracao; contador < numeroIndividuos; contador++) {
        idConfiguracaoSelecionada = selecionaConfiguracao(populacaoParam, numeroTabelas);
        operadorPAO(configuracoesParam, idConfiguracao, idConfiguracaoSelecionada,
                grafoSetoresParam, matrizPiParam, vetorPiParam, indiceRegulador,dadosRegulador,
                dadosAlimentadorParam, VFParam, dadosTrafoParam, numeroTrafos,
                matrizSetoresParam, ZParam, maximoCorrenteParam, numeroBarras, estadoInicial,
               listaChavesParam, numChaveAberta, numChaveFechada);
       // imprimeIndividuo(configuracoesParam[idConfiguracao]);
        //printf("\n");
      //  imprimeIndicadoresEletricos(configuracoesParam[idConfiguracao]);
        insereTabelas(populacaoParam, configuracoesParam, idConfiguracao);
        idConfiguracao++;
    }

    geracao = 1;
    while (geracao < maximoGeracoes) {

     //
        
      //  printf("Geracao %ld \n", geracao);
        idConfiguracaoSelecionada = selecionaConfiguracao(populacaoParam, numeroTabelas);
        probabilidadeAplicacao = (double) rand() / (double) RAND_MAX;

        //imprimeIndividuo(configuracoesParam[idConfiguracaoSelecionada]);
        //printf("probabilidade %f \n", probabilidadeAplicacao);
        if (probabilidadeAplicacao <= taxaOperadorPAO) //aplica o operador PAO
        {
             //printf("PAO \n");
            operadorPAO(configuracoesParam, idConfiguracao, idConfiguracaoSelecionada,
                    grafoSetoresParam, matrizPiParam, vetorPiParam, indiceRegulador, dadosRegulador,
                    dadosAlimentadorParam, VFParam, dadosTrafoParam, numeroTrafos,
                    matrizSetoresParam, ZParam, maximoCorrenteParam, numeroBarras,estadoInicial,
                    listaChavesParam, numChaveAberta, numChaveFechada);
            
        }
        else //aplica o operador CAO
        {
            //  printf("CAO\n");
            operadorCAO(configuracoesParam, idConfiguracao, idConfiguracaoSelecionada,
                    grafoSetoresParam, matrizPiParam, vetorPiParam, indiceRegulador, dadosRegulador,
                    dadosAlimentadorParam, VFParam, dadosTrafoParam, numeroTrafos,
                    matrizSetoresParam, ZParam, maximoCorrenteParam, numeroBarras,
                    estadoInicial,listaChavesParam, numChaveAberta, numChaveFechada);
        }
       // imprimeIndividuo(configuracoesParam[idConfiguracao]);
        //insere a nova configuracao nas tabelas
        insereTabelas(populacaoParam, configuracoesParam, idConfiguracao);
        idConfiguracao++;
        geracao++;
    }
}*/
/**
 * Realiza a inserção de uma nova configuração na subpopulação de menor número de manobras que respeita as restrições operacionais do SDR.
 * @param idTabelaParam
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracaoParam
 */
void insereConfiguracaoTabelaManobrasFactiveis(int idTabelaParam, VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam) {
    int idTabela;
    int posicao;
    int contador, indiceTmp, manobrasTmp;
    long int idTmp, idConf;
    //identifica em qual tabela de manobras a configuração será inserida
    //primeira tabela - sem manobras manuais
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais == 0) {
        idTabela = idTabelaParam;
    } else {
        if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais >= 1) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 4)) {
            //segunda tabela - entre [1-4] manobras manuais
            idTabela = idTabelaParam + 1;
        } else {
            if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 8)) {
                //terceira tabela - entre [5-8] manobras manuais
                idTabela = idTabelaParam + 2;
            } else {
                if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 12)) {
                    //quarta tabela - entre [9-12] manobras manuais
                    idTabela = idTabelaParam + 3;
                } else {
                    if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 16)) {
                        //quinta tabela - entre [13-16] manobras manuais
                        idTabela = idTabelaParam + 4;
                    }

                }
            }
        }

    }
    if (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais <= 16) {
        //a tabela não está cheia insere a configuracao
        if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
            posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
            populacaoParam[idTabela].numeroIndividuos++;
        } else //a tabela está cheia
        {
            manobrasTmp = -1;
            indiceTmp = 0;
            //localiza a pior configuração presente na tabela
            for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
                idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
                idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
                if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.menorTensao > configuracoesParam[idConf].objetivo.menorTensao) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.menorTensao == configuracoesParam[idConf].objetivo.menorTensao && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede < configuracoesParam[idConf].objetivo.maiorCarregamentoRede) ||
                        (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.manobrasAutomaticas == configuracoesParam[idConf].objetivo.manobrasAutomaticas && configuracoesParam[idTmp].objetivo.menorTensao == configuracoesParam[idConf].objetivo.menorTensao && configuracoesParam[idTmp].objetivo.maiorCarregamentoRede == configuracoesParam[idConf].objetivo.maiorCarregamentoRede && configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo < configuracoesParam[idConf].objetivo.maiorCarregamentoTrafo)) {
                    indiceTmp = contador;
                    manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;

                }
            }
            idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
            //verifica se a nova solução é melhor do que a pior solução encontrada
            if ((configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas < configuracoesParam[idTmp].objetivo.manobrasAutomaticas) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao > configuracoesParam[idTmp].objetivo.menorTensao) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao == configuracoesParam[idTmp].objetivo.menorTensao && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede < configuracoesParam[idTmp].objetivo.maiorCarregamentoRede) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas == configuracoesParam[idTmp].objetivo.manobrasAutomaticas && configuracoesParam[idConfiguracaoParam].objetivo.menorTensao == configuracoesParam[idTmp].objetivo.menorTensao && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede == configuracoesParam[idTmp].objetivo.maiorCarregamentoRede && configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo < configuracoesParam[idTmp].objetivo.maiorCarregamentoTrafo)) {
                populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
                populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            }

        }
    }
    //sexta tabela - ponderação de manobras
    idTabela = idTabelaParam+5;
    double ponderacaoManobras = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais + (double) configuracoesParam[idConfiguracaoParam].objetivo.manobrasAutomaticas / fatorPonderacaoManobras;
    double ponderacaoTmp = -1;
    //a tabela não está completa ainda
    if (populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
        posicao = populacaoParam[idTabela].numeroIndividuos;
        populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
        populacaoParam[idTabela].tabela[posicao].valorObjetivo = ponderacaoManobras;
        populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
        populacaoParam[idTabela].numeroIndividuos++;
    } else //a tabela está cheia
    {
        //localiza a pior configuração presente na tabela
        for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
            if (ponderacaoTmp < populacaoParam[idTabela].tabela[contador].valorObjetivo) {
                indiceTmp = contador;
                ponderacaoTmp = populacaoParam[idTabela].tabela[contador].valorObjetivo;

            }
        }
        //verifica se a nova solução é melhor do que a pior solução encontrada
        if (ponderacaoManobras < ponderacaoTmp) {
            populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
        }

    }
    idTabela = idTabelaParam+6;
    manobrasTmp = -1;
    //a tabela não está completa ainda
    if (verificaFactibilidade(configuracoesParam[idConfiguracaoParam],maxCarregamentoRede,maxCarregamentoTrafo,maxQuedaTensao) && populacaoParam[idTabela].numeroIndividuos < populacaoParam[idTabela].tamanhoMaximo) {
        posicao = populacaoParam[idTabela].numeroIndividuos;
            populacaoParam[idTabela].tabela[posicao].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[posicao].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
            populacaoParam[idTabela].tabela[posicao].distanciaMultidao = 0;
            populacaoParam[idTabela].numeroIndividuos++;
    } else //a tabela está cheia
    {
        //localiza a pior configuração presente na tabela
        for (contador = 0; contador < populacaoParam[idTabela].numeroIndividuos; contador++) {
            idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
            idConf = populacaoParam[idTabela].tabela[contador].idConfiguracao;
            if ((manobrasTmp < ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo)) ||
                    (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca < configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca) ||
                    (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual) ||
                    (manobrasTmp == ((int) populacaoParam[idTabela].tabela[contador].valorObjetivo) && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca == configuracoesParam[idConf].objetivo.contadorManobrasTipo.seca && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idConf].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idConf].objetivo.contadorManobrasTipo.curtoManual)) {
                indiceTmp = contador;
                manobrasTmp = (int) populacaoParam[idTabela].tabela[contador].valorObjetivo;

            }
        }
        idTmp = populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao;
        //verifica se a nova solução é melhor do que a pior solução encontrada
        if ( (verificaFactibilidade(configuracoesParam[idConfiguracaoParam],maxCarregamentoRede,maxCarregamentoTrafo,maxQuedaTensao) && (configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais < manobrasTmp) ) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual) ||
                    (manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual < configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual) ||
					(manobrasTmp == configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.seca == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.seca && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.comCargaManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.comCargaManual && configuracoesParam[idConfiguracaoParam].objetivo.contadorManobrasTipo.curtoManual == configuracoesParam[idTmp].objetivo.contadorManobrasTipo.curtoManual && configuracoesParam[idTmp].objetivo.manobrasAutomaticas < configuracoesParam[idConf].objetivo.manobrasAutomaticas)) {
            populacaoParam[idTabela].tabela[indiceTmp].idConfiguracao = idConfiguracaoParam;
            populacaoParam[idTabela].tabela[indiceTmp].valorObjetivo = configuracoesParam[idConfiguracaoParam].objetivo.manobrasManuais;
        }

    }
    
}
/**
 * Verifica a inserção de uma nova configuração em cada uma das subpopulações definidas.
 * @param populacaoParam
 * @param configuracoesParam
 * @param idConfiguracao
 */
void insereTabelasFactiveis(VETORTABELA *populacaoParam, CONFIGURACAO *configuracoesParam, long int idConfiguracao) {
int nivelDominancia = 0;
    BOOL tipoObjetivo = false;
    BOOL torneioEntrou;
    int idTabela = 0;
    //tabela de ponderação 1
    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.ponderacao, 
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    //tabela de perdas 2
    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.perdasResistivas, 
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    //tabela de menor Tensão 3
    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.menorTensao, 
            configuracoesParam, true, &torneioEntrou);
    idTabela++;
    //tabela de carregamento da rede 4
    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoRede, 
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    //tabela de carregamento da trafo 5
    insereConfiguracaoTabela(idTabela, populacaoParam, idConfiguracao, configuracoesParam[idConfiguracao].objetivo.maiorCarregamentoTrafo, 
            configuracoesParam, false, &torneioEntrou);
    idTabela++;
    //tabelas pares manobras
    insereConfiguracaoTabelaManobrasFactiveis(idTabela, populacaoParam, configuracoesParam, idConfiguracao);
    idTabela += 7;
    //tabela dominancia
    insereConfiguracaoTabelaDominancia(idTabela, populacaoParam, idConfiguracao, configuracoesParam, nivelDominancia);
}

/**
 * Função para selecionar uma configuração para aplicação dos operadores PAO e CAO. Essa seleção tem como base o método de torneio.
 * Primeiro é a realizado o torneio entre subpopulações com base no índice de sucesso da subpopulação.
 * @param populacaoParam
 * @param numeroTabelasParam
 * @param torneioTabelaSelecionada
 * @return 
 */
long int selecionaConfiguracaoTorneioTabelas(VETORTABELA *populacaoParam, int numeroTabelasParam, int *torneioTabelaSelecionada) {
    int indiceTabela, indiceTabela1, indiceTabela2;
    int indiceConfiguracaoTabela;
    long int idConfiguracaoSelecionada;

    indiceTabela1 = inteiroAleatorio(0, numeroTabelasParam-1);
    indiceTabela2 = inteiroAleatorio(0, numeroTabelasParam-1);

    while(indiceTabela1 == indiceTabela2)
    {
        indiceTabela2 = inteiroAleatorio(0, numeroTabelasParam-1);
    }
    
    if(populacaoParam[indiceTabela1].torneioValorReferencia >= populacaoParam[indiceTabela2].torneioValorReferencia)
    {
        indiceTabela = indiceTabela1;
    }
    else
    {
        indiceTabela = indiceTabela2;
    }

    while (populacaoParam[indiceTabela].numeroIndividuos <= 0)
    {
        indiceTabela1 = inteiroAleatorio(0, numeroTabelasParam-1);
        indiceTabela2 = inteiroAleatorio(0, numeroTabelasParam-1);
        while(indiceTabela1 == indiceTabela2)
        {
            indiceTabela2 = inteiroAleatorio(0, numeroTabelasParam-1);
        }

        if(populacaoParam[indiceTabela1].torneioValorReferencia >= populacaoParam[indiceTabela2].torneioValorReferencia)
        {
            indiceTabela = indiceTabela1;
        }
        else
        {
            indiceTabela = indiceTabela2;
        }
    }
    //printf("Tab1:%d(%ld)-Tab2:%d(%ld)-Tab:%d\t", indiceTabela1, vetorTorneioTabelas[indiceTabela1], indiceTabela2, vetorTorneioTabelas[indiceTabela2], indiceTabela);
    
    indiceConfiguracaoTabela = inteiroAleatorio(0, (populacaoParam[indiceTabela].numeroIndividuos - 1));
    //  printf("indiceTabela %d indiceConfigurcao %d \n", indiceTabela, indiceConfiguracaoTabela);
    idConfiguracaoSelecionada = populacaoParam[indiceTabela].tabela[indiceConfiguracaoTabela].idConfiguracao;
    
    torneioTabelaSelecionada[0] = indiceTabela;
    
    return idConfiguracaoSelecionada;
}
/**
 * Calcula a distância das configurações para os limites de factibilidade operacional
 * @param configuracoesParam
 * @param idConfiguracaoParam
 * @return 
 */
double funcaoDistanciaFactivel(CONFIGURACAO *configuracoesParam, long int idConfiguracaoParam){
    
    double tmpNovo = 0.0;
    int varA = 0;
    int varB = 0;
    int varC = 0;
    
    if(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede > 100)
        varA = 1;
    if(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo > 100)
        varB = 1;
    if(configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima > 10)
        varC = 1;
    
    tmpNovo = varA * ((configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede - 100)*(configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede - 100));
    tmpNovo = tmpNovo + varB * ((configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo - 100) * (configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo - 100));
    tmpNovo = tmpNovo + varC * ((configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima - 10) * (configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima - 10));
    tmpNovo = sqrt(tmpNovo);

    //printf("\nResultado:%lf - Config:%ld - rede:%lf - trafo:%lf - queda:%lf", tmpNovo, idConfiguracaoParam, configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoRede, configuracoesParam[idConfiguracaoParam].objetivo.maiorCarregamentoTrafo, configuracoesParam[idConfiguracaoParam].objetivo.quedaMaxima);
    return tmpNovo;
}

/**
 * Por Leandro
 * Descrição: Faz o ajuste da taxa de probabilidade de aplicação dos operadores PAO e CAO da RNP. Considera-se que a aplicação de operador teve sucesso se
 * o indivíduo gerado ATENDE AS RESTRIÇÕES RELAXADAS
 *
 * @param probabilidadeAplicacaoParam
 * @param taxaOperadorPAOParam
 * @param sucessoAplicacaoOperadorParam
 * @return nova taxa de aplicação do operador PAO
 */
double ajustarTaxaDeProbabilidadeAplicacaoOperadores(double probabilidadeAplicacaoParam, double taxaOperadorPAOParam, int sucessoAplicacaoOperadorParam){

	if (probabilidadeAplicacaoParam < taxaOperadorPAOParam) // o operador aplicado foi o PAO
	{
		if(sucessoAplicacaoOperadorParam == 1) //o indivíduo gerado pelo PAO atende as RESTRIÇÕES RELAXADAS. Logo, o 'sucesso' do PAO foi alcançado e a taxa de probabilidade de escolha do mesmo deve ser aumentada
			taxaOperadorPAOParam = taxaOperadorPAOParam + 0.01;
		else
			taxaOperadorPAOParam = taxaOperadorPAOParam - 0.01;
	}
	else //o operador aplicado foi o CAO
	{
		if(sucessoAplicacaoOperadorParam == 1) //o indivíduo gerado pelo CAO atende as RESTRIÇÕES RELAXADAS. Logo, o 'sucesso' do mesmo foi alcançado e a taxa de probabilidade de escolha do PAO deve ser diminuída
			taxaOperadorPAOParam = taxaOperadorPAOParam - 0.01;
		else
			taxaOperadorPAOParam = taxaOperadorPAOParam + 0.01;
	}
	return taxaOperadorPAOParam;
}
