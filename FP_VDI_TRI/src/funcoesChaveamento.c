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
