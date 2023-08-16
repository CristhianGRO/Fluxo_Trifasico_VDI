/**
 * @file funcoesInicializacao.c
 * @brief Este arquivo contém a implementação de funções e métodos necessários a inicialização de algumas estruturas.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include "funcoesSetor.h"
#include "funcoesInicializacao.h"
#include "data_structures.h"

/**
 *  Este método é responsável por identificar todas as chaves do Sistema de Distribuição e armazená-las em uma estrutura.
 *  O método também atribui a cada chave um identificador único, que será utilizado para identificar as chaves manobradas.
 *  Tem como parâmetros o grafo que representa o sistema e a estrutura onde serão armazenadas as chaves
 * @param grafoSDRParam
 * @param listaChavesParam
 * @param estadoInicial
 * @param estadoAutomaticas
 * @param estadoRestabelecimento
 * @param numeroChaves
 * @param numeroBarras
 */
void constroiListaChaves(GRAFO *grafoSDRParam, LISTACHAVES **listaChavesParam, 
        ESTADOCHAVE *estadoInicial, ESTADOCHAVE *estadoAutomaticas, 
        ESTADOCHAVE *estadoRestabelecimento, long int numeroChaves, long int numeroBarras)
{
   //faz a alocação da estrutura para armazenar as chaves do sistema
    if (((*listaChavesParam) = (LISTACHAVES *)malloc((numeroChaves+1) * sizeof(LISTACHAVES)))==NULL)
    {
        printf("ERRO ao alocar memoria para lista de chaves!!!");
        exit(1);
    }
    int contador, contadorAdj;
    int identificadorChave = 1; //variável identificador das chaves, começa a sua numeração de 1 e é utilizada para indexar a nova estrutura
    
    //percorre o grafo do sistema de distribuição identificando as chaves e armazenando na nova estrutura atribuindo um identificador
    for (contador =1; contador <= numeroBarras; contador++)
    {
        for(contadorAdj = 0; contadorAdj<grafoSDRParam[contador].numeroAdjacentes; contadorAdj++)
        {
            if((grafoSDRParam[contador].adjacentes[contadorAdj].idNo >= contador)&&(grafoSDRParam[contador].adjacentes[contadorAdj].tipoAresta == chaveManual || grafoSDRParam[contador].adjacentes[contadorAdj].tipoAresta == chaveAutomatica))
            {
                //copia os dados referentes a chave
                (*listaChavesParam)[identificadorChave].idNoDe = grafoSDRParam[contador].idNo;
                (*listaChavesParam)[identificadorChave].idNoPara = grafoSDRParam[contador].adjacentes[contadorAdj].idNo;
                (*listaChavesParam)[identificadorChave].condicao = grafoSDRParam[contador].adjacentes[contadorAdj].condicao;
                (*listaChavesParam)[identificadorChave].estadoChave = grafoSDRParam[contador].adjacentes[contadorAdj].estadoChave;
                (*listaChavesParam)[identificadorChave].identificador = identificadorChave;
                (*listaChavesParam)[identificadorChave].subTipoChave = grafoSDRParam[contador].adjacentes[contadorAdj].subTipoAresta;
                (*listaChavesParam)[identificadorChave].tipoChave = grafoSDRParam[contador].adjacentes[contadorAdj].tipoAresta;
                strcpy((*listaChavesParam)[identificadorChave].codOperacional, grafoSDRParam[contador].adjacentes[contadorAdj].idAresta);       
                estadoInicial[identificadorChave] =  grafoSDRParam[contador].adjacentes[contadorAdj].estadoChave;
                if(grafoSDRParam[contador].adjacentes[contadorAdj].tipoAresta == chaveAutomatica)
                    estadoAutomaticas[identificadorChave] =  grafoSDRParam[contador].adjacentes[contadorAdj].estadoChave;
                else
                    estadoAutomaticas[identificadorChave] = normalmenteAberta;
                estadoRestabelecimento[identificadorChave] =  grafoSDRParam[contador].adjacentes[contadorAdj].estadoChave;
                //incrementa o valor do identificador da chave
                identificadorChave++;
                
            }
        }
    }   
}

/**
 * Realiza a impressão na tela de todas as chaves do sistema com o identificador e as suas barrass.
 * Recebe como parâmetro a estrutura LISTACHAVES
 * @param listaChavesParam
 * @param numeroChaves
 */
void imprimeListaChaves(LISTACHAVES *listaChavesParam, long int numeroChaves)
{
    int contador;
   //imprime o número de chaves do sistema
    printf("Numero Chaves %ld\n", numeroChaves);
    //percorre a lista realizando a impressão
    for(contador = 1; contador<=numeroChaves; contador++)
    {
        printf("Identificador chave: %ld id no origem: %ld id no destino %ld Estado Chave %d \n", listaChavesParam[contador].identificador, listaChavesParam[contador].idNoDe, listaChavesParam[contador].idNoPara, listaChavesParam[contador].estadoChave);
    }

}

/**
 * Função que retorna o identificador de uma chave dados os valores da barras correspondentes
 * Recebe como parâmetros a lista de chaves (LISTACHAVES) e o valor das duas barras
 * @param listaChavesParam
 * @param idNo1
 * @param idNo2
 * @param numeroChaves
 * @return 
 */
int buscaChave(LISTACHAVES *listaChavesParam, int idNo1, int idNo2, long int numeroChaves)
{
    int contador=1;
    BOOL encontrado = false;
    int idChave = -1;
    //percorre a lista de chaves até encontrar a chave ou chegar ao fim da lista
    while(contador<=numeroChaves && !encontrado)
    {
        //verifica a chave refere-se as barras informadas;
        if((listaChavesParam[contador].idNoDe == idNo1 && listaChavesParam[contador].idNoPara == idNo2) || (listaChavesParam[contador].idNoDe == idNo2 && listaChavesParam[contador].idNoPara == idNo1))
        {
            encontrado = true;
            idChave = contador;
        }
        contador++;
    }
    
    return idChave;
}

/**
 * Realiza uma busca em profundidade nos setores identificados do sistema de forma a constuir a RNP do alimentador.
 * Tem como parâmetros o identificador do setor, o vetor de flags para indicar os setores visitados, o ponteiro para RNP
 * que está sendo construída, a profundidade atual, o índice atual, o Grafo de setores sistema e a lista de chaves.
 * 
 * @param idSetorParam
 * @param visitadoParam
 * @param rnpParam
 * @param profundidade
 * @param indice
 * @param grafoSetoresParam
 * @param listaChavesParam
 */
void buscaProfundidadeSetores(int idSetorParam, BOOL *visitadoParam,RNP *rnpParam, int profundidade, int *indice, GRAFOSETORES *grafoSetoresParam, LISTACHAVES *listaChavesParam) 
{
    int indiceAdj;
    int idSetorAdj;
    int idChave;
    rnpParam->nos[indice[0]].idNo = idSetorParam;
    rnpParam->nos[indice[0]].profundidade = profundidade;
    visitadoParam[idSetorParam] = true;
    indice[0]++;
    profundidade++;
    //percorre a lista de adjacências do grafo de setores
    for(indiceAdj = 0; indiceAdj < grafoSetoresParam[idSetorParam].numeroAdjacentes; indiceAdj++)
    {
        idSetorAdj = grafoSetoresParam[idSetorParam].setoresAdjacentes[indiceAdj];
        idChave = grafoSetoresParam[idSetorParam].idChavesAdjacentes[indiceAdj];
        //se o setor adjacente não tiver sido visitado e a chave for fechada continua a busca em profundidade recursivamente
        if(!visitadoParam[idSetorAdj] && listaChavesParam[idChave].estadoChave==normalmenteFechada)
        {
            buscaProfundidadeSetores(idSetorAdj, visitadoParam, rnpParam, profundidade, indice, grafoSetoresParam, listaChavesParam);
        }
    }
        
}


/**
 * Percorre a lista de alimentadores do sistema com os seus repectivos setores, construindo as RNPs da configuração inicial.
 * Esse processo considera o estado das chaves. 
 * Tem como parâmentros o grafo de setores, o grafo do sistema, a lista de chaves, a lista de alimentadores, a matriz PI ra RNP e lista de configurações 
 *
 * @param grafoSetoresParam
 * @param grafoSDRParam
 * @param listaChavesParam
 * @param alimentadoresParam
 * @param populacaoParam
 */
/*
void constroiIndividuoInicial(GRAFOSETORES *grafoSetoresParam, 
        GRAFO *grafoSDRParam, LISTACHAVES *listaChavesParam, 
        DADOSALIMENTADOR *alimentadoresParam, CONFIGURACAO *populacaoParam)
{
    int indice;
    int indiceRNP = 0;
    int profundidade = 0;
    int idIndividuoInicial = 0;
    
    BOOL *visitado = Malloc(BOOL, (numeroSetores+1));
    int idSetorAlimentador;
    //inicializa o vetor flag de visitado para  busca em profundidade.
    for(indice=0;indice<=numeroSetores; indice++)
    {
        visitado[indice] = false;
    }
    
    //percorre a lista de alimentadores do sistema para construir a RNP referente a cada alimentador
    for(indice=1;indice<=numeroAlimentadores; indice++)
    {
        
        indiceRNP=0;
        idSetorAlimentador = grafoSDRParam[alimentadoresParam[indice].barraAlimentador].idSetor;
        //faz a alocação da RNP de cada alimentador
        alocaRNP(alimentadoresParam[indice].numeroSetores,&populacaoParam[idIndividuoInicial].rnp[(indice-1)]);
        //realiza a busca em profundidade para cada alimentador de forma a obter a RNP
        buscaProfundidadeSetores(idSetorAlimentador, visitado,&populacaoParam[idIndividuoInicial].rnp[(indice-1)], profundidade, &indiceRNP, grafoSetoresParam, listaChavesParam);
    }
}
*/
/**
 * Aloca memória e inicializa os arrays de dados elétricos da configuração
 * @param grafoSDRParam
 * @param configuracoesParam
 * @param indiceConfiguracaoParam
 * @param numeroBarrasParam
 * @param sBaseParam
 * @param VFParam
 * @param modeloFluxoCarga
 */
/*Cristhian: função não utilizada nos algoritmos monofásicos e trifásicos
//==============================================================================
void inicializaDadosEletricos(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, double VFParam, int modeloFluxoCarga) {
    long int contadorBarras;
    int sinal = 1;
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    for(contadorBarras = 1; contadorBarras<=numeroBarrasParam; contadorBarras++)
    {
        sinal = 1;
       __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras] = grafoSDRParam[contadorBarras].valorPQ.p;
       __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras] = grafoSDRParam[contadorBarras].valorPQ.q;
       //transforma em modelo monofásico...
       configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras] = (configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras]*sBaseParam)/3.0;
       configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[contadorBarras] = VFParam;
       configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[contadorBarras] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[contadorBarras]); 
    }
}
*/
/*
/**
 * Inicializa os dados elétricos da configuração para o alimentador informado como parâmetro.
 * @param grafoSDRParam
 * @param configuracoesParam
 * @param indiceConfiguracaoParam
 * @param numeroBarrasParam
 * @param sBaseParam
 * @param dadosTrafoParam
 * @param dadosAlimentadorParam
 * @param matrizB
 */
/*
void inicializaDadosEletricosPorAlimentador(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB) {
    long int noS, noR, noN;
   __complex__ double VF;
    int indiceRNP, indice, indice1;
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    RNPSETOR rnpSetorSR;
    //printf("SBase %d \n", sBaseParam);
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    for (indiceRNP = 0; indiceRNP < configuracoesParam[indiceConfiguracaoParam].numeroRNP; indiceRNP++) {
       // printf("trafo %d \n",dadosAlimentadorParam[indiceRNP+1].idTrafo);
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[indiceRNP+1].idTrafo].tensaoReal/ sqrt(3.0);

        noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
                __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
                //transforma em modelo monofásico...
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = (configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] * sBaseParam) / 3.0;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN] = VF;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN]);

            }
             //armazena o nó setor na sua profundidade
            noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
        }
        //Calcula o setor do Alimentador
        if (configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos > 1) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
            noR = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[1].idNo;
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
                __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
                //transforma em modelo monofásico...
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = (configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] * sBaseParam) / 3.0;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN] = VF;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN]);

            }
        }
    }
}
*/

/*void inicializaDadosDePriorizacaoPorAlimentador(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB) {
    long int noS, noR, noN;
   __complex__ double VF;
    int indiceRNP, indice, indice1;
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    RNPSETOR rnpSetorSR;
    //printf("SBase %d \n", sBaseParam);
    //configuracoesParam[indiceConfiguracaoParam].prioridade.consumidor = malloc((numeroBarrasParam + 1) * sizeof (TIPOCONSUMIDOR));

    for (indiceRNP = 0; indiceRNP < configuracoesParam[indiceConfiguracaoParam].numeroRNP; indiceRNP++) {
       // printf("trafo %d \n",dadosAlimentadorParam[indiceRNP+1].idTrafo);
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[indiceRNP+1].idTrafo].tensaoReal/ sqrt(3.0);

        noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                configuracoesParam[indiceConfiguracaoParam].prioridade.consumidor[noN] = grafoSDRParam[noN].priorizacoes.consumidor;
            }
             //armazena o nó setor na sua profundidade
            noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
        }
    }
}*/
/*
void inicializaDadosEletricosChaveamento(RNPSETORES *matrizB, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, 
        long int numeroBarrasParam, int sBaseParam) 
{
    long int noS, noR, noN;
    __complex__ double VF;
    int indiceRNP, indice, indice1;
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    RNPSETOR rnpSetorSR;
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    for (indiceRNP = 0; indiceRNP < (configuracoesParam[indiceConfiguracaoParam].numeroRNP-1); indiceRNP++) {
     //   printf("indiceRNP %d trafo %d \n",indiceRNP, dadosAlimentadorParam[indiceRNP+1].idTrafo);
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[indiceRNP+1].idTrafo].tensaoReal/ sqrt(3.0);

        noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade - 1];
            //printf("nos %ld nor %ld \n", noS, noR);
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
                __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
                //transforma em modelo monofásico...
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = (configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] * sBaseParam) / 3.0;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN] = VF;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN]);

            }
             //armazena o nó setor na sua profundidade
            noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
        }
        //Calcula o setor do Alimentador
        if (configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos > 1) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
            noR = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[1].idNo;
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.p;
                __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = grafoSDRParam[noN].valorPQ.q;
                //transforma em modelo monofásico...
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] = (configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] * sBaseParam) / 3.0;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN] = VF;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[noN]);

            }
        }
    }
}
*/
/*
void imprimeDadosEletricos(CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam) 
{
    long int contadorBarras;
    for(contadorBarras = 1; contadorBarras<=numeroBarrasParam; contadorBarras++)
    {
        printf("Potencia Barra %ld %.2lf j%.2lf\n",contadorBarras,__real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras],__imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[contadorBarras]);
        printf("Vbarra Barra %ld %.2lf j%.2lf\n",contadorBarras,__real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[contadorBarras],__imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[contadorBarras]);
        printf("Corrente Barra %ld %.2lf j%.2lf\n",contadorBarras,__real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[contadorBarras],__imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[contadorBarras]);
      //  printf("Modulo Corrente Barra %ld %.2lf\n\n",contadorBarras,cabs( configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[contadorBarras]));
     
    }
}
*/

/* Por Leandro:
 * Descrição: Aloca espaço para o novo item ".alimentador" inserido no struct "DADOSTRAFO"
 * e Salva, para cada trafo, quais são os alimentadores que atende
 */
void alimentadoresPorTrafo(DADOSTRAFO *dadosTrafoParam, long int numeroTrafosParam, DADOSALIMENTADOR *dadosAlimentadorParam, long int numeroAlimentadoresParam){
	int indice, idTrafo, indiceJ;
	DADOSTRAFO *dadosTrafoTemporario = Malloc(DADOSTRAFO, (numeroTrafosParam+1));

/*    //Caso não seja possível alocar a memória encerra o programa.
    if (((*dadosTrafoTemporario) = (DADOSTRAFO *)malloc( (numeroTrafosParam+1) * sizeof(DADOSTRAFO)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para os dados de trafos do SDR !!!!");
        exit(1);
    }*/

    
    for (indice = 1; indice <= numeroTrafosParam; indice++) {

    	dadosTrafoTemporario[indice].alimentador =  Malloc(int, numeroAlimentadoresParam+1);
    	dadosTrafoTemporario[indice].numeroAlimentadores = 0;
    }



    for (indice = 1; indice <= numeroAlimentadoresParam; indice++) {
        idTrafo = dadosAlimentadorParam[indice].idTrafo;

        dadosTrafoTemporario[idTrafo].alimentador[dadosTrafoTemporario[idTrafo].numeroAlimentadores] = indice - 1; //Por Telma: a sequencia de rnps obedece a mesma sequência de alimentadores com a diferença de uma posição
        dadosTrafoTemporario[idTrafo].numeroAlimentadores++;

    }


    for (indice = 1; indice <= numeroTrafosParam; indice++) {
    	dadosTrafoParam[indice].alimentador =  Malloc(int, dadosTrafoTemporario[indice].numeroAlimentadores);
    	/*if(((dadosTrafoParam[indice].alimentador) = (int *)malloc( (dadosTrafoTemporario[indice].numeroAlimentadores +1) * sizeof(int))) == NULL)*/

    	for(indiceJ = 0; indiceJ < dadosTrafoTemporario[indice].numeroAlimentadores; indiceJ++)
    		dadosTrafoParam[indice].alimentador[indiceJ] = dadosTrafoTemporario[indice].alimentador[indiceJ];

    	dadosTrafoParam[indice].numeroAlimentadores = dadosTrafoTemporario[indice].numeroAlimentadores;
    }

    //Desaloa a variável temporária criada
//    for (indice = 1; indice <= numeroTrafosParam; indice++)
//    	dadosTrafoTemporario[indice].alimentador =  NULL; free(dadosTrafoTemporario[indice].alimentador);
    free(dadosTrafoTemporario);
    dadosTrafoTemporario = NULL;
}
