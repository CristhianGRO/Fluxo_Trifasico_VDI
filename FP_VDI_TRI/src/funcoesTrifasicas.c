/**
 * @file funcoesTrifasicas.c
 * @author Cristhian Gabriel da Rosa de Oliveira
 * @date 19 Jul 2023
 * @brief Contém a implementação das funções utilizadas para tornar o algoritmo de restabelecimento trifásico.
 * Este arquivo implementa as funções definidas na biblioteca funcoesTrifasicas.h
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include "funcoesLeituraDados.h"
#include "funcoesRNP.h"
#include "funcoesSetor.h"

extern long int numeroExecucoesFluxoCarga;
extern BOOL flagMudancaTap;
extern __complex__ double ij;
/**
 * 
 * Adaptada por Cristhian
 * @brief Função que lê as informações relativas as barras do SDR, disponiveis no arquivo barras_m.dad, e armazena na estrutura de dados do tipo GRAFO
 * 
 * @param grafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do GRAFO, onde é retornado o SDR na forma de um grafo.
 * @param numeroBarras
 * @return void.
 */
void leituraBarrasSimplicadoModificadaTrifasico(GRAFO **grafoSDRParam, long int *numeroBarras)
{
    FILE *arquivo;
    int tipo;
    double p, q;
    long int contador;
    long int barra, setor;
    char buffer[5000];
    int ignorar = 0, contadorBarras = 0;

    arquivo = fopen("barras_m.dad","r+" );
    
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo barras_m.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", numeroBarras);
     //realiza a alocação da estrutura em grafo para armazenar o SDR e verifica se o processo foi bem sucedido
    //caso não seja possível realizar a alocação o programa é encerrado
    if (((*grafoSDRParam) = (GRAFO *)malloc( (numeroBarras[0]+1) * sizeof(GRAFO)))==NULL)
    {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para o grafo do SDR !!!!");
        exit(1); 
    }    
    for (contador = 1; contador <= numeroBarras[0]; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %ld %d %lf %lf %d", &barra,  &setor, &tipo, &p, &q, &ignorar);

        (*grafoSDRParam)[barra].idNo = barra;
        (*grafoSDRParam)[barra].idSetor = setor;
        (*grafoSDRParam)[barra].tipoNo = tipo;
        (*grafoSDRParam)[barra].valorPQ.p = p;
        (*grafoSDRParam)[barra].valorPQ.q = q;
    }
    fclose(arquivo);
}
/**
 * 
 * Adaptada por Cristhian
 * @brief Função que lê as informações relativas as barras do SDR, disponiveis no arquivo barras_m.dad, e armazena na estrutura de dados do tipo GRAFO
 * 
 * @param grafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do GRAFO, onde é retornado o SDR na forma de um grafo.
 * @param numeroBarras
 * @return void.
 */
void leituraBarrasTrifasico(GRAFO **grafoSDRParam, long int *numeroBarras)
{
    FILE *arquivo;
    double pA, qA, pB, qB, pC, qC;
    long int contador;
    long int barra, setor;
    int fases;
    char buffer[5000];
    int contadorBarras = 0;

    arquivo = fopen("barrasTrifasicas.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo barrasTrifasicas.dad");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);
    sscanf(buffer, "%ld", numeroBarras);

    for (contador = 1; contador <= numeroBarras[0]; contador++) {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %ld %d %lf %lf %lf %lf %lf %lf" , &barra,  &setor, &fases, &pA, &qA, &pB, &qB, &pC, &qC);
        (*grafoSDRParam)[barra].idNo = barra;
        (*grafoSDRParam)[barra].idSetor = setor;

        //Verifica quais fases conectam-se ao nó e atribui o objeto TIPOFASES correspondente
        TIPOFASES enumFases;
        switch(fases){
            case 100:
                enumFases = A;
            break;
            //Esse valor é equivalente a 010
            case 10:
                enumFases = B;
            break;
            case 001:
                enumFases = C;
            break;
            case 110:
                enumFases= AB;
            break;
            case 101:
                enumFases = AC;
            break;
            //Esse valor é equivalente a 011
            case 11:
                enumFases = BC;
            break;
            case 111:
                enumFases = ABC;
            break;
            default:
                printf("Erro na leitura das fases - Valor invalido na barra %ld do setor %ld\n",barra,setor);
            break;
        }
        (*grafoSDRParam)[barra].tipoFases = enumFases;
        //printf("Fases da barra %ld do setor %ld: %d\n",barra,setor,fases);
        //Salva os dados de demanda de potência por fase, para cada barra.
        (*grafoSDRParam)[barra].pqTrifasico.p[0] = pA;
        (*grafoSDRParam)[barra].pqTrifasico.p[1] = pB;
        (*grafoSDRParam)[barra].pqTrifasico.p[2] = pC;

        (*grafoSDRParam)[barra].pqTrifasico.q[0] = qA;
        (*grafoSDRParam)[barra].pqTrifasico.q[1] = qB;
        (*grafoSDRParam)[barra].pqTrifasico.q[2] = qC;
    }

    fclose(arquivo);
}

/**
 * Adaptada por Cristhian
 * @brief Função para a leitura do arquivo dadosTrafoTrifasicos.dad, relativo as informações trifasicas dos trafos. 
 *
 * Essa função realiza a leitura dos dados trifasicos relativos aos trafos do sistema. Nessa função é alocada a estrutura que armazena os dados dos trafos.
 * Para isso ao função recebe como parâmetro de entrada e saída e um ponteiro para o ponteiro do tipo DADOSTRAFO, @p **dadosTrafoSDRParam .
 * A função retorna @c void.
 * 
 * @param dadosTrafoSDRParam é um ponteiro para o ponteiro da estrutura do tipo do DADOSTRAFO, onde os dados do trafo são retornados.
 * @param numeroTrafos inteiro contendo o número total de trafos do SDR.
 * @return void
  * @note
 */

void leituraTrafosTrifasicos(DADOSTRAFO **dadosTrafoSDRParam, long int *numeroTrafos)
{
    char blocoLeitura[300];  //Variável para leitura do bloco de caracteres do arquivo
    long int tamTrafos; //Variável para ler o número de trafos do SDR
    long int contador; //Variável contador para controlar a leitura dos dados de trafos
    int identificadorTrafo; //Variável para o identificador do trafo
    int identificadorSubEstacao; //Variável para o identificador da subestação
    char tipoTrafo[10];
    FILE *arquivo;
    arquivo = fopen("dadosTrafosTrifasicos.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo dadosTrafosTrifasicos.dad");
        exit(1);
    }
    //Leitura do bloco de caracteres do arquivo
    fgets(blocoLeitura, 300, arquivo);
     //Leitura do número de trafos do SDR
    sscanf(blocoLeitura, "%ld",&tamTrafos);
    //Leitura dos dados trifasicos de cada trafo
    for(contador = 1; contador <= numeroTrafos[0]; contador++)
    {
         //Leitura do bloco de caracteres do arquivo
        fgets(blocoLeitura, 300, arquivo);
         //Leitura de cada informação dos dados dos trafos na string blocoLeitura
        sscanf(blocoLeitura,"%d %d %s", &identificadorTrafo, &identificadorSubEstacao, tipoTrafo);
        //Inserção na estrutura de dados dos trafos das informações lidas.
        (*dadosTrafoSDRParam)[contador].idTrafo = identificadorTrafo;
        (*dadosTrafoSDRParam)[contador].idSubEstacao = identificadorSubEstacao;

        TIPOSTRAFOS enumTipo;
        //Define o TIPOTRAFO da barra baseado na string lida no arquivo
        if     (strcmp(tipoTrafo, "YYat"    ) == 0)   enumTipo = YYat;
        else if(strcmp(tipoTrafo, "YatY"    ) == 0)   enumTipo = YatY;
        else if(strcmp(tipoTrafo, "YatYat"  ) == 0)   enumTipo = YatYat;
        else if(strcmp(tipoTrafo, "YY"      ) == 0)   enumTipo = YY;
        else if(strcmp(tipoTrafo, "DD"      ) == 0)   enumTipo = DD;
        else if(strcmp(tipoTrafo, "DY"      ) == 0)   enumTipo = DY;
        else if(strcmp(tipoTrafo, "DYat"    ) == 0)   enumTipo = DYat;
        else if(strcmp(tipoTrafo, "YatD"    ) == 0)   enumTipo = YatD;
        else if(strcmp(tipoTrafo, "YD"      ) == 0)   enumTipo = YD;

        (*dadosTrafoSDRParam)[contador].tipoTransformador = enumTipo;
    }
}


void leituraMatrizImpedanciaCorrenteTrifasicos(MATRIZCOMPLEXA **ZParam,  MATRIZMAXCORRENTE **maximoCorrenteParam, long int numeroBarrasParam, 
        DADOSALIMENTADOR *dadosAlimentador, long int numeroAlimentadores)
{
    FILE *arquivo;
    char buffer[5000];
    long int indiceBarras, barra, barraAdj;
    int indiceAdjacente, numeroAdjacentes, indiceAlimentador;
    double resistencias[6], reatancias[6],ampacidade;
    BOOL barraAlimentador;
    int iterador,iteradorAuxiliar,contador=0;
    arquivo = fopen("matrizImpedanciaCorrenteTrifasicos.dad","r+" );
    if (arquivo == NULL) {
        printf("Erro na abertura do arquivo matrizImpedanciaCorrenteTrifasicos.dad");
        exit(1);
    }

    if (((*maximoCorrenteParam) = (MATRIZMAXCORRENTE *) malloc((numeroBarrasParam + 4) * sizeof (MATRIZMAXCORRENTE))) == NULL) {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para a matriz de máximo de corrente !!!!");
        exit(1);
    }

    //aloca a memória para a matriz Z 
    if (((*ZParam) = (MATRIZCOMPLEXA *) malloc((numeroBarrasParam + 4) * sizeof (MATRIZCOMPLEXA))) == NULL) {
        printf("Erro -- Nao foi possivel alocar espaco de memoria para a matriz Z !!!!");
        exit(1);
    }
    fgets(buffer, 5000, arquivo);

    (*ZParam)[0].idNo = 0;
    (*ZParam)[0].numeroAdjacentes = 1;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[0].noAdjacentes = (CELULACOMPLEXA *) malloc(1 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[0].idNo = 0;
    (*maximoCorrenteParam)[0].numeroAdjacentes = 1;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[0].noAdjacentes = (CELULACORRENTE *) malloc(1 * sizeof (CELULACORRENTE));

    //barra ficticia para o regulador do alimentador 1 no fluxo em anel
    (*ZParam)[numeroBarrasParam+1].idNo = numeroBarrasParam+1;
    (*ZParam)[numeroBarrasParam+1].numeroAdjacentes = 2;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[numeroBarrasParam+1].noAdjacentes = (CELULACOMPLEXA *) malloc(2 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[numeroBarrasParam+1].idNo = numeroBarrasParam+1;
    (*maximoCorrenteParam)[numeroBarrasParam+1].numeroAdjacentes = 2;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[numeroBarrasParam+1].noAdjacentes = (CELULACORRENTE *) malloc(2 * sizeof (CELULACORRENTE));

    //barra ficticia para o regulador do alimentador 2 no fluxo em anel
    (*ZParam)[numeroBarrasParam+2].idNo = numeroBarrasParam+2;
    (*ZParam)[numeroBarrasParam+2].numeroAdjacentes = 2;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[numeroBarrasParam+2].noAdjacentes = (CELULACOMPLEXA *) malloc(2 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[numeroBarrasParam+2].idNo = numeroBarrasParam+2;
    (*maximoCorrenteParam)[numeroBarrasParam+2].numeroAdjacentes = 2;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[numeroBarrasParam+2].noAdjacentes = (CELULACORRENTE *) malloc(2 * sizeof (CELULACORRENTE));


    //barra ficticia para ligar a subestacao ao modelo do anel
    (*ZParam)[numeroBarrasParam+3].idNo = numeroBarrasParam+3;
    (*ZParam)[numeroBarrasParam+3].numeroAdjacentes = 3;
    //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
    (*ZParam)[numeroBarrasParam+3].noAdjacentes = (CELULACOMPLEXA *) malloc(3 * sizeof (CELULACOMPLEXA));

    (*maximoCorrenteParam)[numeroBarrasParam+3].idNo = numeroBarrasParam+3;
    (*maximoCorrenteParam)[numeroBarrasParam+3].numeroAdjacentes = 3;
    //aloca o espaço para os dados dos adjacentes.
    (*maximoCorrenteParam)[numeroBarrasParam+3].noAdjacentes = (CELULACORRENTE *) malloc(3 * sizeof (CELULACORRENTE));

    for(indiceBarras = 1; indiceBarras <= numeroBarrasParam; indiceBarras++)
    {
        fgets(buffer, 5000, arquivo);
        sscanf(buffer, "%ld %d", &barra, &numeroAdjacentes);
        //verifica se a barra é de um alimentador
        barraAlimentador = false;
        indiceAlimentador = 1;
        while ((indiceAlimentador <= numeroAlimentadores) && !barraAlimentador) {
            if (dadosAlimentador[indiceAlimentador].barraAlimentador == barra)
            {
                barraAlimentador = true;
            }
            indiceAlimentador++;
        }

        (*ZParam)[barra].idNo = barra;
        (*ZParam)[barra].numeroAdjacentes = numeroAdjacentes;
        (*maximoCorrenteParam)[barra].idNo = barra;
        (*maximoCorrenteParam)[barra].numeroAdjacentes = numeroAdjacentes;
        if (barraAlimentador) {
            //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
            (*ZParam)[barra].noAdjacentes = (CELULACOMPLEXA *) malloc((numeroAdjacentes + 1) * sizeof (CELULACOMPLEXA));


            //aloca o espaço para os dados dos adjacentes.
            (*maximoCorrenteParam)[barra].noAdjacentes = (CELULACORRENTE *) malloc((numeroAdjacentes + 1) * sizeof (CELULACORRENTE));
        } else {
            //aloca o vetor de adjacente do tindicePo celulacomplexa para conter os valores necessários
            (*ZParam)[barra].noAdjacentes = (CELULACOMPLEXA *) malloc(numeroAdjacentes * sizeof (CELULACOMPLEXA));

            //aloca o espaço para os dados dos adjacentes.
            (*maximoCorrenteParam)[barra].noAdjacentes = (CELULACORRENTE *) malloc(numeroAdjacentes * sizeof (CELULACORRENTE));
        }
        for(indiceAdjacente = 0; indiceAdjacente < numeroAdjacentes; indiceAdjacente++)
        {
            fgets(buffer, 5000, arquivo);
            sscanf(buffer, "%ld %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &barraAdj, &ampacidade, &resistencias[0],&reatancias[0],&resistencias[1],&reatancias[1],&resistencias[2],&reatancias[2],&resistencias[3],&reatancias[3],&resistencias[4],&reatancias[4],&resistencias[5],&reatancias[5]);

            (*ZParam)[barra].noAdjacentes[indiceAdjacente].idNo = barraAdj;
            //Adicionando os elementos da matriz impedancia da triangular superior
            contador=0;
            for(iterador=0;iterador<3;iterador++){
                for(iteradorAuxiliar=iterador;iteradorAuxiliar<3;iteradorAuxiliar++){
                    (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[iterador][iteradorAuxiliar] = resistencias[contador] + ij*reatancias[contador];
                    contador++;
                }
            }
            //Adicionando os elementos da matriz impedancia da triangular inferior
            (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[1][0] = resistencias[1] + ij*reatancias[1];
            (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[2][0] = resistencias[2] + ij*reatancias[2];
            (*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[2][1] = resistencias[4] + ij*reatancias[4];

            (*maximoCorrenteParam)[barra].noAdjacentes[indiceAdjacente].valor = ampacidade;
            /*CRISTHIAN: printa a matriz Zabc*/
            /*int i,j;
            printf("\n\nZABC\n=================\n");
            for(i=0;i<3;i++){
                for(j=0;j<3;j++){
                    printf("%.3lf+j%.3f ", creal((*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[i][j]),cimag((*ZParam)[barra].noAdjacentes[indiceAdjacente].valor[i][j]));
                }
                printf("\n");
            }
            */
        }
    }
    fclose(arquivo);

}

/**
 * Cristhian: Inicializa os dados elétricos da configuração para o alimentador trifasico informado como parâmetro.
 * @param grafoSDRParam
 * @param configuracoesParam
 * @param indiceConfiguracaoParam
 * @param numeroBarrasParam
 * @param sBaseParam
 * @param dadosTrafoParam
 * @param dadosAlimentadorParam
 * @param matrizB
 */
void inicializaDadosEletricosPorAlimentadorTrifasico(GRAFO *grafoSDRParam, CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam, int sBaseParam, DADOSTRAFO *dadosTrafoParam, DADOSALIMENTADOR *dadosAlimentadorParam, RNPSETORES *matrizB) {
    long int noS, noR, noN;
    //Cristhian: assume-se, inicialmente, que todas as fases deverão ser percorridas na varredura 
    BOOL presencaDeFases[3] = {true, true, true};
   __complex__ double VF;
    int indiceRNP, indice, indice1, iterador;
    long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    RNPSETOR rnpSetorSR;
    TIPOFASES tipoFases;
    //printf("SBase %d \n", sBaseParam);
    for(iterador=0;iterador<3;iterador++){
        configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante[iterador] = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
        configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[iterador] = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
        configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador] = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
        configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador] = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    }
    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.tipoFases = malloc((numeroBarrasParam + 1) * sizeof (TIPOFASES));
    for (indiceRNP = 0; indiceRNP < configuracoesParam[indiceConfiguracaoParam].numeroRNP; indiceRNP++) {
       // printf("trafo %d \n",dadosAlimentadorParam[indiceRNP+1].idTrafo);
       //Cristhian: Atualiza o tipo de fase de cada um dos nos
        VF = 1000*dadosTrafoParam[dadosAlimentadorParam[indiceRNP+1].idTrafo].tensaoReal/ sqrt(3.0);

        noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            for(indice1 = 0; indice1 < rnpSetorSR.numeroNos; indice1++){
                noN = rnpSetorSR.nos[indice1].idNo;
                configuracoesParam[indiceConfiguracaoParam].dadosEletricos.tipoFases[noN] =  grafoSDRParam[noN].tipoFases;
            }

            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                //Cristhian: atribuindo as potencias por fase da estrutura GRAFO para a estrutura CONFIGURACAO
                for(iterador=0;iterador<3;iterador++){
                    __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][noN] = grafoSDRParam[noN].pqTrifasico.p[iterador];
                    __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][noN] = grafoSDRParam[noN].pqTrifasico.q[iterador];

                    //Cristhian: atualizando a tensão de todas as fases para a tensão de fase do transformador
                    switch(iterador){
                        case 0:
                            configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][noN] = VF;
                        break;
                        case 1:
                            configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][noN] = VF*cos(-120*M_PI/180) + ij*sin(-120*M_PI/180);
                        break;
                        case 2:
                            configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][noN] = VF*cos(120*M_PI/180) + ij*sin(120*M_PI/180);
                        break;
                    }
                }

                //Cristhian: avalia a necessidade de calcular a corrente para a fase. Caso a fase não esteja conectada no nó à jusante, a corrente é zero.
                //Cristhian: inicialmente, assume-se que todas as fases deverão ser percorridas.
                for(iterador = 0; iterador < 3; iterador++){
                    presencaDeFases[iterador] = true;
                }

                tipoFases = configuracoesParam[indiceConfiguracaoParam].dadosEletricos.tipoFases[noN];
                verificaPresencaDeFaseNo(&presencaDeFases, tipoFases);
                //Cristhian: calcula a corrente para cada uma das fases, se o nó possuir essa fase
                if(presencaDeFases[0] == true){
                    //Para a Fase A:
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[0][noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[0][noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[0][noN]);
                }else{
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[0][noN] = 0;
                }

                if(presencaDeFases[1] == true){
                    //Para a Fase B:
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[1][noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[1][noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[1][noN]);
                }
                else{
                     configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[1][noN] = 0;
                }

                if(presencaDeFases[2] == true){
                    //Para a Fase C:
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[2][noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[2][noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[2][noN]);
                }
                else{
                     configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[2][noN] = 0;
                }

            }
             //armazena o nó setor na sua profundidade
            noProf[configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[indice].idNo;
        }

        //===============================================================================================================================
        //Calcula o setor do Alimentador
        if (configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].numeroNos > 1) {
            noS = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[0].idNo;
            noR = configuracoesParam[indiceConfiguracaoParam].rnp[indiceRNP].nos[1].idNo;
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);

            for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
                noN = rnpSetorSR.nos[indice1].idNo;
                for(iterador=0;iterador<3;iterador++){
                    __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][noN] = grafoSDRParam[noN].pqTrifasico.p[iterador];
                    __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][noN] = grafoSDRParam[noN].pqTrifasico.q[iterador];

                    switch(iterador){
                        case 0:
                            configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][noN] = VF;
                        break;
                        case 1:
                            configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][noN] = VF*cos(-120*M_PI/180) + ij*sin(-120*M_PI/180);
                        break;
                        case 2:
                            configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][noN] = VF*cos(120*M_PI/180) + ij*sin(120*M_PI/180);
                        break;
                    }
                }

                for(iterador = 0; iterador < 3; iterador++){
                    presencaDeFases[iterador] = true;
                }

                tipoFases = configuracoesParam[indiceConfiguracaoParam].dadosEletricos.tipoFases[noN];
                verificaPresencaDeFaseNo(&presencaDeFases, tipoFases);
                //Cristhian: calcula a corrente para cada uma das fases, se o nó possuir essa fase
                if(presencaDeFases[0] == true){
                    //Para a Fase A:
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[0][noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[0][noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[0][noN]);
                }else{
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[0][noN] = 0;
                }

                if(presencaDeFases[1] == true){
                    //Para a Fase B:
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[1][noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[1][noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[1][noN]);
                }
                else{
                     configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[1][noN] = 0;
                }

                if(presencaDeFases[2] == true){
                    //Para a Fase C:
                    configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[2][noN] = conj(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[2][noN] / configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[2][noN]);
                }
                else{
                     configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[2][noN] = 0;
                }
            }
        }
    }
}

/**
 * Cristhian: essa função calcula o fluxo de carga trifasico de cada alimentador no modelo de varredura direta inversa.
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
void fluxoCargaAlimentadorTrifasico(int numeroBarrasParam, CONFIGURACAO *configuracoesParam,
        __complex__ double VFParam, int indiceRNP, int indiceConfiguracao, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam, int *indiceRegulador,
        DADOSREGULADOR *dadosRegulador, int maximoIteracoes) {
    long int indice, noS, noR, noN, indiceBarra;
    int iteracoes = 0, carga, iterador;
    double erroV, erroA, tol, difV, difVAux, difA, difAAux;
    __complex__ float *V_barra_ant[3];
    long int pior[3]; //Cristhian: armazena a barra com pior perfil de tensão em cada fase do alimentador
    long int *ptrPior;
    ptrPior = &pior;
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = (long int*) malloc(400 * sizeof(long int));
    RNPSETOR rnpSetorSR;
    BOOL tipoFases[3] = {true, true, true};
    //Cristhian: armazena memoria para a tensão da barra anterior para cada uma das fases

    for(iterador=0;iterador<3;iterador++){
        V_barra_ant[iterador] = malloc((numeroBarrasParam + 1) * sizeof (__complex__ double));
    }

    tol = 0.001;

    numeroExecucoesFluxoCarga++;  /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/

    carga = 0; //0 --> corrente constante, 1--> potencia constante
    //  printf("\nALIMENTADOR %d \n", indiceRNP+1);
    do // varredura backward/forward
    {
        printf("\n----------------iteracao %d -----------------\n", iteracoes);
        indice = 0;
        noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        for (indice = 1; indice < configuracoesParam[indiceConfiguracao].rnp[indiceRNP].numeroNos; indice++) {
            noS = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
            noR = noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade - 1];
            rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
            //printf("\nnumeroNos = %d", rnpSetorSR.numeroNos);
            for (indiceBarra = 0; indiceBarra < rnpSetorSR.numeroNos; indiceBarra++) {
                noN = rnpSetorSR.nos[indiceBarra].idNo;

                //Cristhian: verifica as barras conectadas ao no
                verificaPresencaDeFaseNo(&tipoFases, configuracoesParam[indiceConfiguracao].dadosEletricos.tipoFases[noN]);
                if (iteracoes == 0) {
                    //Atualiza as tensões para todas as barras, nas fases conectadas ao nó, assumindo sequencia de fase positiva.
                    //Para a fase A:
                    if(tipoFases[0]==true){
                        configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[0][noN] = VFParam;
                        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[0][noN] = conj(configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[0][noN] / configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[0][noN]);
                    }else{
                        configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[0][noN]    = 0;
                        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[0][noN]  = 0;
                    }
                    //Para a fase B:
                    if(tipoFases[1]==true){
                        configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[1][noN] = VFParam*cos(-120*M_PI/180) + ij * VFParam*sin(-120*M_PI/180);
                        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[1][noN] = conj(configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[1][noN] / configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[1][noN]);
                    }else{
                        configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[1][noN]    = 0;
                        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[1][noN]  = 0;
                    }
                    //Para a fase C:
                    if(tipoFases[2]==true){
                        configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[2][noN] = VFParam*cos(120*M_PI/180) + ij * VFParam*sin(120*M_PI/180);
                        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[2][noN] = conj(configuracoesParam[indiceConfiguracao].dadosEletricos.potencia[2][noN] / configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[2][noN]);
                    }else{
                        configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[2][noN]    = 0;
                        configuracoesParam[indiceConfiguracao].dadosEletricos.corrente[2][noN]  = 0;
                    }
                }

                for(iterador=0;iterador<3;iterador++){
                    V_barra_ant[iterador][noN] = configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[iterador][noN];
                }
            }

            noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        }

        correnteJusanteTrifasico(indiceRNP, carga, iteracoes, configuracoesParam[indiceConfiguracao], matrizB, indiceRegulador, dadosRegulador);
        tensaoQuedaTensaoModificadaTrifasico(ptrPior, indiceRNP, configuracoesParam[indiceConfiguracao], VFParam, matrizB, ZParam, indiceRegulador, dadosRegulador); // etapa forward - tensões

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
                difV = 0;
                difA = 0;
                //Cristhian: calcula a diferença de tensao, entre iteracoes, para cada fase
                //Assume-se que o erro sera calculada considerando a maior diferenca de tensao
                //printf("\nANTES ===========>\n");
                //imprimeDadosEletricosTrifasicos(configuracoesParam, 0, 4);
                for(iterador=0;iterador<3;iterador++){
                    difVAux = cabs(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[iterador][noN]) - cabs(V_barra_ant[iterador][noN]); //calcula a diferença de módulos das tensões entre duas iterações
                    difV = difVAux > difV || difVAux < -difV ? difVAux : difV; //Atualiza o maior desvio de modulo de tensao, entre as fases.

                    difAAux = carg(configuracoesParam[indiceConfiguracao].dadosEletricos.vBarra[iterador][noN]) - carg(V_barra_ant[iterador][noN]); //calcula a diferença de ângulos das tensões entre duas iterações
                    difA = difAAux > difA || difAAux < -difA ? difAAux : difA; //Atualiza o maior desvio de angulo de tensao, entre as fases.
                }
                if (difV < 0)
                    difV = -1 * difV; //faz mÃ³dulo
                if (difV > erroV) //encontra o maior desvio
                    erroV = difV;

                if (difA < 0)
                    difA = -1 * difA; //faz módulo
                if (difA > erroA) //encontra o maior desvio
                    erroA = difA;
            }
        
            noProf[configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].profundidade] = configuracoesParam[indiceConfiguracao].rnp[indiceRNP].nos[indice].idNo;
        }
        iteracoes++;
    } while ((erroV > tol || erroA > tol || flagMudancaTap) && iteracoes < maximoIteracoes);
    //exit(0);
    carregamentoPerdasModificadaTrifasico(&configuracoesParam[indiceConfiguracao], indiceRNP, matrizB, ZParam, maximoCorrenteParam);
    free(*V_barra_ant);
    free(noProf);
    
}

void verificaPresencaDeFaseNo(int* presencaDeFases, TIPOFASES tipoFases){
    //Cristhian: verifica se a varredura deverá percorrer a fase A no próximo nó
    //printf("\n\nTIPO FASES = %d",tipoFases);
    if(tipoFases == A || tipoFases == AB || tipoFases == AC || tipoFases == ABC){
        presencaDeFases[0] = 1;
    }else{
        presencaDeFases[0] = 0;
    }
    //Cristhian: verifica se a varredura deverá percorrer a fase B no próximo nó
    if(tipoFases == B || tipoFases == AB || tipoFases == BC || tipoFases == ABC){
        presencaDeFases[1] = 1;
    }else{
        presencaDeFases[1] = 0;
    }
    //Cristhian: verifica se a varredura deverá percorrer a fase C no próximo nó
    if(tipoFases == C || tipoFases == AC || tipoFases ==BC || tipoFases == ABC){
        presencaDeFases[2] = 1;
    }else{
        presencaDeFases[2] = 0;
    }
}

/**
 * Cristhian: Método utilizado para cálculo do fluxo de potência trifásico de varredura direta inversa.
 * Percorre o alimentador informado na representação nó-profundidade calculando as correntes jusantes
 * no processo de varredura inversa do alimentador, para cada uma das fases que estiverem conectados ao nó (ETAPA BACKWARD).
 * @param idRNP
 * @param carga informa se é utilizado o modelo de corrente constante (0) ou potência constante (1).
 * @param iteracao
 * @param configuracaoParam
 * @param matrizB
 * @param indiceRegulador
 * @param dadosRegulador
 */
void correnteJusanteTrifasico(int idRNP, int carga, int iteracao,
        CONFIGURACAO configuracaoParam, RNPSETORES *matrizB, int *indiceRegulador, DADOSREGULADOR *dadosRegulador) {
    long int indice, indice1, noS, noR, noN, idSetorS, idSetorR, idBarra1, idBarra2, indice2, indice3;
    double IMod, IAng;
    int iteradorFase;
    BOOL  *presencaFase[3] = {true, true, true};
    //long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = malloc(400 * sizeof(long int));
    __complex__ double iAcumulada;
    RNPSETOR rnpSetorSR;
    //=========================================================================================
    //Cristhian: calcula as correntes que fluem para as cargas no nó, em módulo e ângulo, por fase

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
                //Cristhian: Verifica o valor da variavel tipoFases do nó avaliado, para saber quais correntes precisarão ser calculadas
                verificaPresencaDeFaseNo(&presencaFase, configuracaoParam.dadosEletricos.tipoFases[noN]);
                if (carga == 0)
                //============================================================================================================== 
                //Modelo de corrente constante
                {
                    //Cristhian: calcula as correntes nas fases
                    if((int) presencaFase[0] == 1){
                        //Para a fase A:
                        IMod = cabs(configuracaoParam.dadosEletricos.corrente[0][noN]);
                        IAng = carg(configuracaoParam.dadosEletricos.vBarra[0][noN]) - carg(configuracaoParam.dadosEletricos.potencia[0][noN]);

                        configuracaoParam.dadosEletricos.corrente[0][noN] = IMod * cos(IAng) + ij * IMod * sin(IAng);
                    }else{
                        configuracaoParam.dadosEletricos.corrente[0][noN] = 0 + ij * 0;
                    }
                    if((int) presencaFase[1]==1){
                        //Para a fase B:
                        IMod = cabs(configuracaoParam.dadosEletricos.corrente[1][noN]);
                        IAng = carg(configuracaoParam.dadosEletricos.vBarra[1][noN]) - carg(configuracaoParam.dadosEletricos.potencia[1][noN]);
                        configuracaoParam.dadosEletricos.corrente[1][noN] = IMod * cos(IAng) + ij * IMod * sin(IAng);
                    }else{
                        configuracaoParam.dadosEletricos.corrente[1][noN] = 0 + ij * 0;
                    }
                    if((int) presencaFase[2]==1){
                        //Para a fase C:
                        IMod = cabs(configuracaoParam.dadosEletricos.corrente[2][noN]);
                        IAng = carg(configuracaoParam.dadosEletricos.vBarra[2][noN]) - carg(configuracaoParam.dadosEletricos.potencia[2][noN]);
                        configuracaoParam.dadosEletricos.corrente[2][noN] = IMod * cos(IAng) + ij * IMod * sin(IAng);
                    }else{
                        configuracaoParam.dadosEletricos.corrente[2][noN] = 0 + ij * 0;
                    }
                } else
                //==============================================================================================================
                //Modelo de potencia constante
                {
                    //Cristhian: Calcula a corrente para as fases conectadas ao nó pelo modelo de potencia constante
                    if((int) presencaFase[0]==1){
                        //Para a fase A:
                         configuracaoParam.dadosEletricos.corrente[0][noN] = conj(configuracaoParam.dadosEletricos.potencia[0][noN] / configuracaoParam.dadosEletricos.vBarra[0][noN]);
                    }else{
                        configuracaoParam.dadosEletricos.corrente[0][noN] = 0;
                    }
                    if((int) presencaFase[1]==1){
                        //Para a fase B:
                        configuracaoParam.dadosEletricos.corrente[1][noN] = conj(configuracaoParam.dadosEletricos.potencia[1][noN] / configuracaoParam.dadosEletricos.vBarra[1][noN]);
                    }else{
                        configuracaoParam.dadosEletricos.corrente[1][noN] = 0;
                    }
                    if((int) presencaFase[2]==1){
                        //Para a fase c:
                        configuracaoParam.dadosEletricos.corrente[2][noN] = conj(configuracaoParam.dadosEletricos.potencia[2][noN] / configuracaoParam.dadosEletricos.vBarra[2][noN]);
                    }else{
                        configuracaoParam.dadosEletricos.corrente[2][noN] = 0;
                    }
                }
            }
            noProf[configuracaoParam.rnp[idRNP].nos[indice].profundidade] = configuracaoParam.rnp[idRNP].nos[indice].idNo;
        }
    }

    //=========================================================================================
    //Cristhian: calcula as correntes nos trechos do modelo
    for (indice = configuracaoParam.rnp[idRNP].numeroNos - 1; indice > 0; indice--) {
        indice2 = indice - 1;
        //busca pelo nó raiz
        while (indice2 >= 0 && configuracaoParam.rnp[idRNP].nos[indice2].profundidade != (configuracaoParam.rnp[idRNP].nos[indice].profundidade - 1))
            indice2--;
        idSetorS = configuracaoParam.rnp[idRNP].nos[indice].idNo;
        idSetorR = configuracaoParam.rnp[idRNP].nos[indice2].idNo;
        rnpSetorSR = buscaRNPSetor(matrizB, idSetorS, idSetorR);
        for (indice2 = 0; indice2 < rnpSetorSR.numeroNos; indice2++) {
            //Zera as variáveias iJusante, para todas as fases
            idBarra1 = rnpSetorSR.nos[indice2].idNo;
            for(iteradorFase = 0;iteradorFase < 3;iteradorFase++){
                configuracaoParam.dadosEletricos.iJusante[iteradorFase][idBarra1] = 0 + ij * 0;
            }
        }
    }

    for (indice = configuracaoParam.rnp[idRNP].numeroNos - 1; indice > 0; indice--) {
        indice2 = indice - 1;
        //Busca pelo nó raiz
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

            //Calcula a corrente acumulada (soma da corrente que flui pelo trecho + corrente demandada pela carga), para cada fase
            for(iteradorFase=0;iteradorFase<3;iteradorFase++){
                iAcumulada = configuracaoParam.dadosEletricos.iJusante[iteradorFase][idBarra2] + configuracaoParam.dadosEletricos.corrente[iteradorFase][idBarra2];
                configuracaoParam.dadosEletricos.iJusante[iteradorFase][idBarra1] = configuracaoParam.dadosEletricos.iJusante[iteradorFase][idBarra1] + iAcumulada;
                //printf("\nCorrente calculada = %.3f", creal(configuracaoParam.dadosEletricos.corrente[iteradorFase][idBarra1]));
            }


            // if (eRegulador(idBarra1, idBarra2,)) //existe um regulador de tensão entre as barras 1 e 2.
            //printf("idBarra %ld %ld\n", idBarra1, idBarra2);

            //Verifica a presença de reguladores de tensão nos trechos
            if ((indiceRegulador[idBarra1] != 0) && (indiceRegulador[idBarra2] != 0) && (indiceRegulador[idBarra1] == indiceRegulador[idBarra2])) {
                //Cristhian: considera-se que o regulador de tensão é trifásico
                for(iteradorFase=0;iteradorFase<3;iteradorFase++){
                    configuracaoParam.dadosEletricos.iJusante[iteradorFase][idBarra1] = configuracaoParam.dadosEletricos.iJusante[iteradorFase][idBarra1]*(1 + 0.00625 * dadosRegulador[indiceRegulador[idBarra1]].numeroTaps);
                }
            }
        }
    }
    free(noProf); noProf= NULL;
}

/*
 * Por Leandro: consiste na função "tensaoQuedaTensao()" modificada para armazenas o nó (barra) na qual está ocorrendo a menor queda
 * de tensão na rede
 *
 * Portanto, calcula a tensão em cada barra, a menor tensão das barras e retorna a barra com menor tensão
 *
 * Cristhian: alterada a função para alocar as piores quedas de tensão em um ponteiro, que indica a menor tensão para cada fase, ao invés de retornar um inteiro único.
 */
void tensaoQuedaTensaoModificadaTrifasico(long int* pior, int indiceRNP, CONFIGURACAO configuracaoParam, __complex__ double VFParam, RNPSETORES *matrizB, MATRIZCOMPLEXA *ZParam, int *indiceRegulador, DADOSREGULADOR *dadosReguladorSDR) {
    int indice, indice1;
    long int noS, noR, noM, noN;
    int tapAnterior;
    double limiteInferior = 13491 / sqrt(3); //Cristhian: Tensao minima (tensao de fase - V) em uma barra regulada pelo RT;
    double limiteSuperior = 13763 / sqrt(3); //Cristhian: Tensao maxima (tensao de fase - V) em uma barra regulada pelo RT;
    double carregamentoRT, carregamentoAux;
    double aux;
    //long int noProf[200]; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    long int *noProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    noProf = malloc(400 * sizeof(long int));
    //long int barraProf[100]; //armazena o ultimo nó do setor presente em uma profundidade, é indexado pela profundidade
    long int *barraProf; //armazena o ultimo nó presente em uma profundidade, é indexado pela profundidade
    barraProf = malloc(200 * sizeof(long int));
    __complex__ double deltaV;
    double temporario;
    int tensao[3], iterador, iteradorFase;
    double menorTensaoNo[3] = {9999, 9999, 9999}, maiorTensaoNo[3] = {0 , 0, 0}; //Cristhian: armazena no primeiro elemento o modulo da tensao, no segundo elemento o id do no e, no terceiro, a fase em que ocorre o modulo de tensao salvo.
    double menorTensaoGlobal = 9999;
    long int noMenorTensaoGlobal;
    long int noMenorTensao[3]; // Por Leandro: salvará o nó (setor) da RNP no qual está contida a barra com o menor valor de tensão dentro da RNP em questão | Por Cristhian: adicionada duas dimensões adicionais, para armazenar o nó de menor tensão, em cada fase
    long int noMaiorTensao[3];
    RNPSETOR rnpSetorSR;
    int* presencaDeFases = {true, true, true};
    __complex__ double Zno[3];
    noProf[configuracaoParam.rnp[indiceRNP].nos[0].profundidade] = configuracaoParam.rnp[indiceRNP].nos[0].idNo;
    flagMudancaTap = false;
    //printf("menorTensao %lf \n", menorTensao);
    for (indice = 1; indice < configuracaoParam.rnp[indiceRNP].numeroNos; indice++) {
        noS = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
        noR = noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade - 1];
        rnpSetorSR = buscaRNPSetor(matrizB, noS, noR);
        barraProf[rnpSetorSR.nos[0].profundidade] = rnpSetorSR.nos[0].idNo;
        //  imprimeRNPSetor(rnpSetorSR);
        for (indice1 = 1; indice1 < rnpSetorSR.numeroNos; indice1++) {
            noM = barraProf[rnpSetorSR.nos[indice1].profundidade - 1];
            noN = rnpSetorSR.nos[indice1].idNo;
            menorTensaoNo[0] = 9999;

            if (indiceRegulador[noM] != 0 && indiceRegulador[noN] != 0) //ha um regulador de tensao entre as barras m e n.
            {
                //Cristhian: assume-se que, se houver violacao de tensao em qualquer uma das fases, o tap do regulador devera ser alterado, independente da condicao das demais fases.
                //Cristhian: Alem de verificar o valor de tensao, deve-se verificar se a fase com violacao (nesse caso violacao de limite inferior), esta conectada na barra. Se nao estiver, o valor de tensao zero, abaixo do limite inferior, nao representa problema e o regulador nao devera ser afetado.
                verificaPresencaDeFaseNo(presencaDeFases, configuracaoParam.dadosEletricos.tipoFases[noN]);
                for(iterador=0;iterador<3;iterador++){
                    configuracaoParam.dadosEletricos.vBarra[iterador][noN] = configuracaoParam.dadosEletricos.vBarra[iterador][noM]*(1 + 
                    passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);

                    tensao[iterador] = (int) cabs(configuracaoParam.dadosEletricos.vBarra[iterador][noN]);

                    if(menorTensaoNo[0] > tensao[iterador] && presencaDeFases[iterador] == true){
                        menorTensaoNo[0] = tensao[iterador];    //Modulo da menor tensao
                        menorTensaoNo[1] = noN;                 //No com a menor tensao
                        menorTensaoNo[2] = iterador;            //Fase com a menor tensao
                    }else if(maiorTensaoNo[0] < tensao[iterador] && presencaDeFases[iterador] == true){
                        maiorTensaoNo[0] = tensao[iterador];
                        maiorTensaoNo[1] = noN;
                        maiorTensaoNo[2] = iterador;
                    }
                }
                tapAnterior = dadosReguladorSDR[indiceRegulador[noM]].numeroTaps; // Grava o estado do tap antes das analises

                if (maiorTensaoNo[0] > limiteSuperior || menorTensaoNo[0] < limiteInferior) { // Analises para a mudanca ou nao do tap

                    if (maiorTensaoNo[0] > limiteSuperior) {
                        aux = limiteSuperior / (configuracaoParam.dadosEletricos.vBarra[(int) maiorTensaoNo[2]][noM]) - 1;

                        // Calcula os novos taps:
                        dadosReguladorSDR[indiceRegulador[noM]].numeroTaps = (int) floor(aux / passoRegulador); //arredondamento para baixo

                        // Calcula o carregamento do RT:
                        //Assume-se que o carregamento sera igual ao maior carregamento, entre todas as fases:
                        //TODO: considerar que o carregamento é dado em razao da potencia do RT, ao inves da ampacidade
                        carregamentoRT = 0;
                        for(iterador=0;iterador<3;iterador++){
                            carregamentoAux = cabs(configuracaoParam.dadosEletricos.iJusante[iterador][noM]) / dadosReguladorSDR[indiceRegulador[noM]].ampacidade;
                            if(carregamentoAux> carregamentoRT){
                                carregamentoRT = carregamentoAux;
                            }
                        }
                        calculaNumeroMaximoTaps(carregamentoRT, dadosReguladorSDR, indiceRegulador[noM]);
                        //*********************************************************************************************

                        //    		  printf("tap[%d] = %d\n",indreg[m],regulador[indreg[m]].tap);
                        for(iterador=0;iterador<3;iterador++){
                            configuracaoParam.dadosEletricos.vBarra[iterador][noN] = configuracaoParam.dadosEletricos.vBarra[iterador][noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                        }
                    }
                }/* ===================CRISTHIAN: CONDICIONAL NUNCA ALCANÇADA!!!!!!!!=====================
                else {

                    if (tensao < limiteInferior) {
                        aux = limiteInferior / (configuracaoParam.dadosEletricos.vBarra[noM] * sqrt(3)) - 1;

                        // Calcula os novos taps:
                        dadosReguladorSDR[indiceRegulador[noM]].numeroTaps = (int) ceil(aux / passoRegulador); //arredondamento para cima

                        // Calcula o carregamento do RT:
                        carregamentoRT = cabs(configuracaoParam.dadosEletricos.iJusante[noM]) / dadosReguladorSDR[indiceRegulador[noM]].ampacidade;

                        calculaNumeroMaximoTaps(carregamentoRT, dadosReguladorSDR, indiceRegulador[noM]);

                        configuracaoParam.dadosEletricos.vBarra[noN] = configuracaoParam.dadosEletricos.vBarra[noM]*(1 + passoRegulador * dadosReguladorSDR[indiceRegulador[noM]].numeroTaps);
                    }
                }*/

                // Condicao para verificar se o tap foi alterado ou nao
                if (tapAnterior != dadosReguladorSDR[indiceRegulador[noM]].numeroTaps) {
                    flagMudancaTap = true;
                }

            } else {
                //Atualiza a nova tensao da barra
                for(iterador=0;iterador<3;iterador++){
                    //Atualiza o valor de impedancia associada ao no e salva no ponteiro Zno
                    valorZTrifasico(ZParam, noM, noN, &Zno, iterador);
                    //printf("\n Para %ld e %ld, na fase %d = [%.3lf %.3lf %.3lf] + i*[%.3lf %.3lf %.3lf]",noM,noN,iterador,creal(Zno[0]),creal(Zno[1]),creal(Zno[2]), cimag(Zno[0]), cimag(Zno[1]), cimag(Zno[2]));
                    deltaV= 0;
                    for(iteradorFase=0;iteradorFase<3;iteradorFase++){
                        deltaV += Zno[iteradorFase] * (configuracaoParam.dadosEletricos.iJusante[iteradorFase][noN] + configuracaoParam.dadosEletricos.corrente[iteradorFase][noN]);
                    }

                    configuracaoParam.dadosEletricos.vBarra[iterador][noN] = configuracaoParam.dadosEletricos.vBarra[iterador][noM] - deltaV;
                    //printf("\nTensao atual = %.2lf", configuracaoParam.dadosEletricos.vBarra[iterador][noN]);
                }
            }
            /*#########################################################
              #################    Queda de Tensão    #################*/

            //Calcula o modulo de todas as tensões, para todas as fases da RNP. Armazena a menor tensão entre todas as fases no fitnessRNP de menor tensao
            for(iterador=0;iterador<3;iterador++){
                temporario = sqrt(pow(__real__ configuracaoParam.dadosEletricos.vBarra[iterador][noN], 2) + pow(__imag__ configuracaoParam.dadosEletricos.vBarra[iterador][noN], 2));

                if (temporario < menorTensaoGlobal) {
                    menorTensaoGlobal = temporario;
                    pior = noN;
                    noMenorTensaoGlobal = noS; // Por Leandro: salva o nó (setor) da RNP no qual está contida a barra com o menor valor de tensão dentro da RNP em questão
                }
            }

            //armazena o nó barra na sua profundidade
            barraProf[rnpSetorSR.nos[indice1].profundidade] = rnpSetorSR.nos[indice1].idNo;
        }
        //armazena o nó setor na sua profundidade
        noProf[configuracaoParam.rnp[indiceRNP].nos[indice].profundidade] = configuracaoParam.rnp[indiceRNP].nos[indice].idNo;
    }
    //exit(0);

    configuracaoParam.rnp[indiceRNP].fitnessRNP.menorTensao = menorTensaoGlobal;
    configuracaoParam.rnp[indiceRNP].fitnessRNP.noMenorTensao = noMenorTensaoGlobal;
    // printf("\n");
    //exit(0);
    free(noProf); noProf = NULL;
    free(barraProf); barraProf = NULL;
}

/**
 * Por Cristhian: consiste na função "carregamentoPerdasModificada()", modificada para que considere-se a modelagem trifasica da rede
 * @param configuracaoParam
 * @param indiceRNP
 * @param matrizB
 * @param ZParam
 * @param maximoCorrenteParam
 */
void carregamentoPerdasModificadaTrifasico(CONFIGURACAO *configuracaoParam, int indiceRNP, RNPSETORES *matrizB,
        MATRIZCOMPLEXA *ZParam, MATRIZMAXCORRENTE *maximoCorrenteParam) {
    int indice, indice1, iterador, iteradorFase;
    long int noS, noR, noN, noM;
    double temporario, sobrecargaRede = 0, corrente, correnteMaximaSuportada;
    double perda = 0;
    __complex__ double ijus[3], ibarra[3];
    __complex__ double Zno[3];
    RNPSETOR rnpSetorSR;
    double perdas = 0;
    __complex__ double perdasReativas;
    double carregamentoRede = 0;
    __complex__ double carregamentoAlimentador = 0 + ij * 0;
    long int noCarregamentoRede = 0; // Por Leandro: salva o no da RNP no qual há o maior carregamento

    //fprintf(arquivog, "Alimentador %d \n", indiceRNP+1);
    temporario = 0;
    ijus[0] = 0, ijus[1] = 0, ijus[2] = 0;
    ibarra[0] = 0, ibarra[1] = 0, ibarra[2] = 0;
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

            /*#######################################################################
             * ############    Carregamento máximo da rede (em corrente)   ##########*/
            //Cristhian: avalia o carregamento em cada fase

            for(iterador=0;iterador<3;iterador++){
                ijus[iterador] = configuracaoParam->dadosEletricos.iJusante[iterador][noN];
                ibarra[iterador] = configuracaoParam->dadosEletricos.corrente[iterador][noN];

                correnteMaximaSuportada = maximoCorrente(maximoCorrenteParam, noM, noN); //Por Leandro: criou-se esta várivável para que a função "máximoCorrente" fosse chamada somente uma única vez

                corrente = cabs(ijus[iterador] + ibarra[iterador]); //Por Leandro: criou-se esta várivável para que a chamada da função "cabs"  e esta soma fossem realizados uma única vez

                temporario = corrente / correnteMaximaSuportada;

                if (temporario > carregamentoRede) {
                carregamentoRede = temporario;
                noCarregamentoRede = noS; // Por Leandro: salva o setor (nó) da RNP no qual tem ocorrido o maior carregamento desta RNP

                //Por Leandro: Cálculo da corrente que excente o limite superior de corrente
                    if(temporario > 1) //Por Leandro: se houver sobrecarga, então calcula-se a quantidade de Amperes que excede o limite superior de corrente entre as barras noM e noN
                	    sobrecargaRede = corrente - correnteMaximaSuportada;

                //printf("noM %ld noN %ld %lf\n", noM, noN, temporario);
                }

                /*#########################################################
                *#####################      Perdas     #### ##############*/
                //Atualiza o valor de impedancia associada ao no e salva no ponteiro Zno
                valorZTrifasico(ZParam, noM, noN, &Zno, iterador);

                //Por Cristhian: as perdas serao dadas pela soma das perdas nas tres fases
                //Soma as perdas devido a impedancia própria e as mutuas
                for(iteradorFase=0;iteradorFase<3;iteradorFase++){
                    perdas += (pow((cabs(ijus[iteradorFase] + ibarra[iteradorFase])), 2)) * __real__ Zno[iteradorFase];
                    perdasReativas += ij * pow((cabs(ijus[iteradorFase] + ibarra[iteradorFase])), 2) * __imag__ Zno[iteradorFase];
                }

                /*############################## ########################################
                ###########    Carregamento dos alimentadores (em potência)   ##########*/
                carregamentoAlimentador += (configuracaoParam->dadosEletricos.vBarra[iterador][noN] * conj(configuracaoParam->dadosEletricos.corrente[iterador][noN]));
            }
             //Por Cristhian: o carregamento dos alimentadores sera dado pela soma das perdas, nas tres fases, para todos os nos
            __real__ carregamentoAlimentador += perdas;
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
CONFIGURACAO* alocaIndividuoModificadaTrifasico(int numeroRNPParam,  int numeroRNPFicticiaParam, long int idIndividuoInicialParam,
		long int numeroConfiguracoesParam, int numeroTrafosParam, int numeroSetoresParam)
{
    //aloca a lista de configurações o identificador do individuo inicial é somado ao numero de configurações para permitir o uso
    //do identificador como índice da lista.
    CONFIGURACAO *individuo = Malloc(CONFIGURACAO, (numeroConfiguracoesParam+idIndividuoInicialParam));
    long int indice, contador;
    int iterador;
    //percorre a lista de configurações fazendo a alocação do vetor de RNPs da configuração
    for (indice = idIndividuoInicialParam; indice <(numeroConfiguracoesParam+idIndividuoInicialParam); indice++ )
    {
        for(iterador=0;iterador<3;iterador++){
            individuo[indice].dadosEletricos.potencia[iterador]  = 0;
            individuo[indice].dadosEletricos.corrente[iterador] = 0;
            individuo[indice].dadosEletricos.iJusante[iterador] = 0;
            individuo[indice].dadosEletricos.vBarra[iterador] = 0;
        }

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
/*Por Cristhian: essa funcao recebe uma matriz ZParam e, a partir de um no especificado, para uma fase especificada, atualiza um ponteiro enviado como argumento as impedancias da fase enviada, tanto as mutuas quanto as próprias
 * @param ZParam
 * @param idNo1
 * @param idNo2
 * @return
 */
void valorZTrifasico(MATRIZCOMPLEXA *ZParam, long int idNo1, long int idNo2, __complex__ double* Zno, int fase) {
    int indiceAdjacentes = 0, iterador;

    while (indiceAdjacentes < ZParam[idNo1].numeroAdjacentes) {
        if (ZParam[idNo1].noAdjacentes[indiceAdjacentes].idNo == idNo2){
            //Atualiza as impedancias do ponteiro Zno
            for(iterador = 0 ; iterador < 3 ; iterador++){
               Zno[iterador] = ZParam[idNo1].noAdjacentes[indiceAdjacentes].valor[fase][iterador];
            }
        }
        indiceAdjacentes++;
    }
}

/**
 * Cristhian: Esta função consiste na função alocaIndividuo, alterada para lidar com dados eletricos trifásicos
 * 
 * @param numeroRNPParam
 * @param idIndividuoInicialParam
 * @param numeroConfiguracoesParam
 * @param numeroTrafosParam
 * @return 
 */
CONFIGURACAO* alocaIndividuoTrifasico(int numeroRNPParam, long int idIndividuoInicialParam, long int numeroConfiguracoesParam, int numeroTrafosParam)
{
    //aloca a lista de configuraÃ§Ãµes o identificador do individuo inicial Ã© somado ao numero de configuraÃ§Ãµes para permitir o uso 
    //do identificador como Ã­ndice da lista.
    CONFIGURACAO *individuo = Malloc(CONFIGURACAO, (numeroConfiguracoesParam+idIndividuoInicialParam));
    long int indice;
    int iterador;
    //percorre a lista de configuraÃ§Ãµes fazendo a alocaÃ§Ã£o do vetor de RNPs da configuraÃ§Ã£o
    for (indice = idIndividuoInicialParam; indice <(numeroConfiguracoesParam+idIndividuoInicialParam); indice++ )
    {
        individuo[indice].numeroRNP = numeroRNPParam;
        individuo[indice].numeroRNPsFalta = 0;
        individuo[indice].rnp = Malloc(RNP, individuo[indice].numeroRNP);
        individuo[indice].idRnpFalta = Malloc(int, individuo[indice].numeroRNP);
        individuo[indice].idConfiguracao = indice;
        inicializaObjetivos(&individuo[indice], numeroTrafosParam);
        for(iterador=0;iterador<3;iterador++){
             individuo[indice].dadosEletricos.corrente[iterador] = NULL;
            individuo[indice].dadosEletricos.iJusante[iterador] = NULL;
            individuo[indice].dadosEletricos.potencia[iterador] = NULL;
            individuo[indice].dadosEletricos.vBarra[iterador] = NULL;
        }
        //imprimeIndicadoresEletricos(individuo[indice]);
    }
    return individuo;
}
/*
 * Por Cristhian:
 * Esta função consiste na função desalocacaoCompletaConfiguracaoModificada, alterada para lidar com dados eletricos trifásicos
 *
 *@param configuracoesParam Ã© a variÃ¡vel do tipo "CONFIGURACOES"
 *@param numeroPosicoesAlocadasParam Ã© o nÃºmero de posiÃ§Ãµes a serem desalocadas
 */

void desalocacaoCompletaConfiguracaoModificadaTrifasico(CONFIGURACAO **configuracoesParam, long int numeroPosicoesAlocadasParam){
	int contador, indice,iterador;
	for (contador = 0; contador < numeroPosicoesAlocadasParam; contador++) {

		indice = 0;
	    while (indice < (*configuracoesParam)[contador].numeroRNP) {  //Percorre o vetor de RNPs
	        if((*configuracoesParam)[contador].rnp[indice].nos!= NULL){
	        	(*configuracoesParam)[contador].rnp[indice].nos = NULL;
	            free((*configuracoesParam)[contador].rnp[indice].nos); // Libera memÃ³ria ocupada por RNPs Reais
	        }

	        if(indice < (*configuracoesParam)[contador].numeroRNPFicticia){
				if((*configuracoesParam)[contador].rnpFicticia[indice].nos!= NULL){
					(*configuracoesParam)[contador].rnpFicticia[indice].nos = NULL;
					free((*configuracoesParam)[contador].rnpFicticia[indice].nos); // Libera memÃ³ria ocupada por RNPs Ficticias
				}
	        }
	        indice++;
	    }
	    //libera a memÃ³ria utilizada pelo vetor de RNPs
	    if((*configuracoesParam)[contador].rnp != NULL){         (*configuracoesParam)[contador].rnp = NULL;		 free((*configuracoesParam)[contador].rnp);}
	    if((*configuracoesParam)[contador].rnpFicticia != NULL){ (*configuracoesParam)[contador].rnpFicticia = NULL; free((*configuracoesParam)[contador].rnpFicticia);}

	    //Libera memÃ³ria ocupada por outras variÃ¡veis
	    if((*configuracoesParam)[contador].idRnpFalta != NULL){                (*configuracoesParam)[contador].idRnpFalta                = NULL; free((*configuracoesParam)[contador].idRnpFalta);}
//	    if((*configuracoesParam)[contador].ranqueamentoRnpsFicticias != NULL){ (*configuracoesParam)[contador].ranqueamentoRnpsFicticias = NULL; free((*configuracoesParam)[contador].ranqueamentoRnpsFicticias);}
        for(iterador=0;iterador<3;iterador++){
            if((*configuracoesParam)[contador].dadosEletricos.corrente[iterador] != NULL){   (*configuracoesParam)[contador].dadosEletricos.corrente[iterador]   = NULL; free((*configuracoesParam)[contador].dadosEletricos.corrente[iterador]);}
            if((*configuracoesParam)[contador].dadosEletricos.iJusante[iterador] != NULL){   (*configuracoesParam)[contador].dadosEletricos.iJusante[iterador]   = NULL; free((*configuracoesParam)[contador].dadosEletricos.iJusante[iterador]);}
            if((*configuracoesParam)[contador].dadosEletricos.potencia[iterador] != NULL){   (*configuracoesParam)[contador].dadosEletricos.potencia[iterador]   = NULL; free((*configuracoesParam)[contador].dadosEletricos.potencia[iterador]);}
            if((*configuracoesParam)[contador].dadosEletricos.vBarra[iterador] != NULL){     (*configuracoesParam)[contador].dadosEletricos.vBarra[iterador]     = NULL; free((*configuracoesParam)[contador].dadosEletricos.vBarra[iterador]);}
        }
	    if((*configuracoesParam)[contador].objetivo.potenciaTrafo !=NULL){ 	   (*configuracoesParam)[contador].objetivo.potenciaTrafo    = NULL; free((*configuracoesParam)[contador].objetivo.potenciaTrafo);}

	}
 if((*configuracoesParam)!= NULL){ (*configuracoesParam) = NULL; free(*configuracoesParam);}
}


/**
 * Por Leandro: consiste na funÃ§Ã£o "desalocaConfiguracao()" modificada para:
 * 1) Liberar espaÃ§o alocado para outras variÃ¡veis para quais a funÃ§Ã£o anterior nÃ£o liberava
 *
 * @param configuracaoParam
 */
void desalocaConfiguracaoModificadaTrifasico(CONFIGURACAO configuracaoParam) {
    long int indice = 0;
    int iterador;
    //libera a memÃ³ria utilizada pelo vetor de RNPs
    if(configuracaoParam.rnp != NULL){         free(configuracaoParam.rnp);                 configuracaoParam.rnp = 0;}
    if(configuracaoParam.numeroRNPFicticia > 0)
    	if(configuracaoParam.rnpFicticia != NULL){ free(configuracaoParam.rnpFicticia); configuracaoParam.rnpFicticia = 0;}

    //Libera memÃ³ria ocupada por outras variÃ¡veis
    if(configuracaoParam.idRnpFalta != NULL){                free(configuracaoParam.idRnpFalta);                configuracaoParam.idRnpFalta                = 0;}

    for(iterador=0;iterador<3;iterador++){
        if(configuracaoParam.dadosEletricos.corrente[iterador] != NULL){   free(configuracaoParam.dadosEletricos.corrente[iterador]);   configuracaoParam.dadosEletricos.corrente[iterador]   = 0;}
        if(configuracaoParam.dadosEletricos.iJusante != NULL){   free(configuracaoParam.dadosEletricos.iJusante[iterador]);   configuracaoParam.dadosEletricos.iJusante[iterador]   = 0;}
        if(configuracaoParam.dadosEletricos.potencia != NULL){   free(configuracaoParam.dadosEletricos.potencia[iterador]);   configuracaoParam.dadosEletricos.potencia[iterador]   = 0;}
        if(configuracaoParam.dadosEletricos.vBarra != NULL){     free(configuracaoParam.dadosEletricos.vBarra[iterador]);     configuracaoParam.dadosEletricos.vBarra[iterador]     = 0;}
    }

    if(configuracaoParam.objetivo.potenciaTrafo !=NULL){ 	 free(configuracaoParam.objetivo.potenciaTrafo);    configuracaoParam.objetivo.potenciaTrafo    = 0;}

}

void imprimeDadosEletricosTrifasicos(CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam) 
{
    long int contadorBarras;
    int iterador;
    for(contadorBarras = 1; contadorBarras<=numeroBarrasParam; contadorBarras++){
        printf("\n------------------\nBarra [%d]\n------------------\n", contadorBarras);
        for(iterador = 0; iterador<3; iterador++){
            printf("- (%d) Potencia %.6lf j%.6lf\n", iterador, __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][contadorBarras], __imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][contadorBarras]);
            printf("- (%d) Vbarra   %.6lf  j%.6lf\n", iterador, __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][contadorBarras],__imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][contadorBarras]);
            printf("- (%d) Corrente %.6lf j%.6lf\n", iterador, __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[iterador][contadorBarras],__imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[iterador][contadorBarras]);
            printf("- (%d) Ijusante %.6lf j%.6lf\n", iterador, __real__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante[iterador][contadorBarras],__imag__ configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante[iterador][contadorBarras]);
        }
    }
}

void imprimeDadosEletricosTrifasicos_POLAR(CONFIGURACAO *configuracoesParam, long int indiceConfiguracaoParam, long int numeroBarrasParam) 
{
    long int contadorBarras;
    int iterador;
    for(contadorBarras = 1; contadorBarras<=numeroBarrasParam; contadorBarras++){
        printf("\n------------------\nBarra [%d]\n------------------\n", contadorBarras);
        for(iterador = 0; iterador<3; iterador++){
            printf("- (%d) Potencia %.6lf /_ %.6lf [deg]\n", iterador, cabs(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][contadorBarras]), 180/M_PI*cargf(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.potencia[iterador][contadorBarras]));
            printf("- (%d) Vbarra   %.6lf  /_ %.6lf [deg]\n", iterador, cabs(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][contadorBarras]) , 180/M_PI*cargf(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.vBarra[iterador][contadorBarras]));
            printf("- (%d) Corrente %.6lf /_ %.6lf [deg]\n", iterador, cabs(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[iterador][contadorBarras]) ,180/M_PI*cargf(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.corrente[iterador][contadorBarras]));
            printf("- (%d) Ijusante %.6lf /_ %.6lf [deg]\n", iterador, cabs( configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante[iterador][contadorBarras]),180/M_PI*cargf(configuracoesParam[indiceConfiguracaoParam].dadosEletricos.iJusante[iterador][contadorBarras]));
        }
    }
}