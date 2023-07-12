/*
 ============================================================================
 Name        : main.c
 Author      : Cristhian (Adaptado de Leandro)
 Version     : 1
 Copyright   : Your copyright notice
 Description : Esta é a implementação do algoritmo de fluxo de carga trifásico pelo método de varredura direta inversa
 Created on 11 de Julho de 2023
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <locale.h>    /* Biblioteca necessaria para configuracoes regionais. */
//#include <gperftools/profiler.h>
#include "funcoesLeituraDados.h"
#include "funcoesSetor.h"
#include "funcoesInicializacao.h"
#include "funcoesProblema.h"
#include "funcoesRNP.h"
#include "aemt.h"
#include "funcoesAuxiliares.h"
#include "funcoesHeuristica.h"
extern long int numeroAlimentadores; /**< Variável global para armazenar o número de alimentadores do SDR. */
extern long int numeroReguladores; /**< Variável global para armazenar o número de reguladores de tensão do SDR. */

int main(int argc, char** argv) {
 //  ProfilerStart("sc1");
	clock_t  inicio, fim;
	inicio = clock();

    GRAFO *grafoSDR;
    DADOSTRAFO *dadosTrafoSDR;
    DADOSALIMENTADOR *dadosAlimentadorSDR;
    DADOSREGULADOR *dadosReguladorSDR;
    RNPSETORES *rnpSetores;
    GRAFOSETORES *grafoSetores;
    LISTACHAVES *listaChaves;
    BOOL resultadoLeitura;
    int seed = atoi(argv[1]);
    //printf("%d\n", seed);
    long int idConfiguracao = 0;
    MATRIZMAXCORRENTE *maximoCorrente;
    MATRIZCOMPLEXA *Z;
    CONFIGURACAO *configuracaoInicial;
    ESTADOCHAVE *estadoInicial;

    long int numeroBarras, numeroTrafos, numeroChaves;
    //leituraBarrasSimplicado(&grafoSDR, &numeroBarras);
    leituraBarrasSimplicadoModificada(&grafoSDR, &numeroBarras);

    leituraBarrasTrifasicas(&grafoSDR, &numeroBarras);

    resultadoLeitura =  leituraDadosReguladoresTensao(&dadosReguladorSDR);

    leituraTrafos(&dadosTrafoSDR, &numeroTrafos);

    leituraTrafosTrifasicos(&dadosTrafoSDR, &numeroTrafos);

    alimentadoresPorTrafo(dadosTrafoSDR, numeroTrafos, dadosAlimentadorSDR, numeroAlimentadores);

    leituraListaChaves(&listaChaves, &numeroChaves, &estadoInicial);

    leituraListaAdjacenciaSetores(&grafoSetores, &numeroSetores);

    leituraRNPSetores(&rnpSetores, numeroSetores);

    leituraMatrizImpedanciaCorrente(&Z, &maximoCorrente, numeroBarras, dadosAlimentadorSDR, numeroAlimentadores);

    return EXIT_SUCCESS;

    /*

    
    

    

   //imprimeListaChaves(listaChaves, numeroChaves);

   

   int numeroAlimentadoresFicticios = 0; //Por Leandro:
   (configuracaoInicial) = alocaIndividuoModificada(numeroAlimentadores, numeroAlimentadoresFicticios, idConfiguracao, 1, numeroTrafos, numeroSetores);

    leituraIndividuoInicial(numeroSetores, numeroAlimentadores, configuracaoInicial);
    reconfiguradorHeuristicaModificada(grafoSDR, numeroBarras, dadosTrafoSDR, numeroTrafos, dadosAlimentadorSDR, numeroAlimentadores, dadosReguladorSDR,
        numeroReguladores, rnpSetores, numeroSetores, grafoSetores, listaChaves, numeroChaves, configuracaoInicial, maximoCorrente,
        Z, estadoInicial, seed);


    free(configuracaoInicial);
    free(dadosReguladorSDR);
    free(dadosAlimentadorSDR);
    free(dadosTrafoSDR);
    free(estadoInicial);
    desalocaGrafoSDR(grafoSDR, numeroBarras);
    free(listaChaves);
    desalocaMatrizZ(Z, numeroBarras);
    desalocaMatrizCorrente(maximoCorrente, numeroBarras);
    desalocaRNPSetores(rnpSetores, numeroSetores);
    free(rnpSetores);
    desalocaGrafoSetores(grafoSetores, numeroSetores);
    free(grafoSetores);

    fim = clock();
//    double tempo = (double)(fim-inicio)/CLOCK_TICKS_PER_SECOND;
//    printf("\n\tTempo Total:%.4lf seg.\t\nFim!", tempo);
//    printf("\nFIM");





    //Impressão de um arquivo para validação da implementação computacional
    int numeroSetorFalta;
    long int *setorFalta;
    leituraDadosEntrada(&numeroSetorFalta, &setorFalta);
    FILE *arquivoTeste;
    char nomeArquivoTeste[120];
    sprintf(nomeArquivoTeste, "SAIDA_testeConsistencia.dad");
    arquivoTeste = fopen(nomeArquivoTeste, "a");
    fprintf(arquivoTeste, "\n%ld\t%d", setorFalta[0], seed);
    fclose(arquivoTeste);


    return (EXIT_SUCCESS);
    */
}


