/**
 * @file funcoesHeuristica.c
 * @brief Implementa a versão das operações da representação nó-profundidade com a inserção de heurísticas do problema de restabelecimento.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <complex.h>
#include <string.h>
#include "data_structures.h"
#include "funcoesRNP.h"
#include "funcoesSetor.h"
#include "funcoesProblema.h"
#include "funcoesChaveamento.h"
#include "funcoesHeuristica.h"
#include "funcoesAuxiliares.h"
#include "funcoesTrifasicas.h"
#include "aemt.h"
#include "funcoesLeituraDados.h"

long int numeroExecucoesFluxoCarga; /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/
int tempoDeRestauracaoDaFalha; /**Por Leandro: salvará o Tempo total estimado necessário para a recupeção de todas as falhas. Em outras palavras, o tempo no qual a rede operará na configuração obtida através de plano de restabelecimento obtido e informado*/

extern int maximoTentativas; /**define o número máximo de tentativas na escolha dos nós para os operadores PAO e CAO*/
extern __complex__ double ij;/** constante para converter o valor double para a parte imaginária de um número completo*/

/**
 * Por Leandro:
 * Consiste na função "reconfiguradorHeuristica()" com inumeras modificações para, em especial:
 * a) chamar funções necessárias para considerar e tratar casos em que não é possível obter soluções que restabeleçam todos os consumidores desligados;
 * b) classificar a população em fronteiras e determinar a fronteira de Pareto (no caso do AEMO ser necessário);
 * c) selecionar um conjunto de soluções finais em vez de apenas uma única solução;
 * d) exlcuir a impressão de alguns arquivos de saída existente na versão original desta funçaõ (versão "reconfiguradorHeuristica()");
 * e) realizar a impressão de novos arquivos de saída, etc.
 *
 * Esta função realizada a leitura da configuração pré-falta do sistema e dos setores em falta; aloca memória para as principais variáveis do código; determina o indivíduo que é, conceitualmente,
 * o inicial do problema (aquele no qual o(s) setor(es) em falta está(ão) isolado(s) e os setores saudáveis afetados permanecem desligados; aplica a Busca Heurística (ou Busca Hexaustiva- BE)
 * para tentar obter indivíduos que reconectem todos os consumidores possíveis de serem reconectados por chaves;  verifica se algum destes indivíduos é atendem as restrições gerais e pode
 * ser considerado uma solução para o problema; chama a rotina que executa o processo evolutivo do AEMO (na verdade não apenas o processo evolutivo como também a geração da população inicial
 * do AEMO) caso a BE não tenha obtido nenhuma solução factível geral; classifica a população em fronteiras segundo o nível de dominância de cada indivíduo; determina a fronteira de Pareto formada
 * pelas soluções não dominadas; seleciona alguns indivíduos factíveis geral para serem as Soluções Finais do problema; e, por fim, imprime todos os arquivos de Saída.
 *
 * Os arquivos de Saída, as informações impressas por eles e a ordem em que as mesmas são impressas são dados a seguir:
 *
 *
 *
 * @param grafoSDRParam
 * @param numeroBarras
 * @param dadosTrafoSDRParam
 * @param numeroTrafos
 * @param dadosAlimentadorSDRParam
 * @param numeroAlimentadores
 * @param dadosReguladorSDR
 * @param numeroReguladores
 * @param rnpSetoresParam
 * @param numeroSetores
 * @param grafoSetoresParam
 * @param listaChavesParam
 * @param numeroChaves
 * @param configuracaoInicial
 * @param maximoCorrente
 * @param Z
 * @param estadoInicial
 * @param seed
 */
void reconfiguradorHeuristicaModificada(GRAFO *grafoSDRParam, long int numeroBarras, DADOSTRAFO *dadosTrafoSDRParam, long int numeroTrafos,
        DADOSALIMENTADOR *dadosAlimentadorSDRParam, long int numeroAlimentadores, DADOSREGULADOR *dadosReguladorSDR,
        long int numeroReguladores, RNPSETORES *rnpSetoresParam, long int numeroSetores, GRAFOSETORES *grafoSetoresParam,
        LISTACHAVES *listaChavesParam, long int numeroChaves, CONFIGURACAO *configuracaoInicial, MATRIZMAXCORRENTE *maximoCorrente,
        MATRIZCOMPLEXA *Z, ESTADOCHAVE *estadoInicial, int seed) {

    long int *setorFalta, idConfiguracao = 0, idConfiguracaoInicial, idConfiguracaoBase, numeroConfiguracoesGeradas = 0;
    int numeroSetorFalta, *indiceReguladores;
    long int contador, numeroChaveAberta = 0, numeroChaveFechada = 0, maximoGeracoes, maximoGeracoesSemAtualizacaoPopulacao;
    int numeroDesejadoSolucoesFinais, numeroSolucoesFinaisObtidas = 0;
    long int melhorConfiguracao = -1, *idIndividuosFinais, *idSolucoesFinais;
    double tempo, taxaOperadorPAO = 0.5;
    int numeroTotalManobrasRestauracaoCompletaSemAlivio, indice, indiceIndividuo;
    long int numeroRNPFicticia = 0, idPrimeiraConfiguracao = -1,  numeroPosicoesAlocadas, geracoesExecutadas;
    int numeroDeFronteiras = 0, contadorRnp, contadorNos, numeroIndividuosPopulacaoInicial;
    int iterador;

    VETORPI *vetorPi;
    MATRIZPI *matrizPI;
    CONFIGURACAO *configuracoes;
    VETORTABELA *populacao;
    SEQUENCIAMANOBRASALIVIO sequenciaManobrasAlivio;
    FRONTEIRAS *fronteiras, fronteiraPareto;
    clock_t inicio, fim, inicioGeral;
    BOOL copiaDados = false, todosAlimentadores = true, flagAtualizacaoPopulacao;
    int SBase;
    int tamanhoTabela[26];
    int numeroTabelas = 26;

    idConfiguracao = 0;
    idConfiguracaoInicial = idConfiguracao;
    idConfiguracaoBase = idConfiguracao;
    numeroExecucoesFluxoCarga = 0; /**Por Leandro: à título de análise e estudo, salvará o número total de alimentadores para os quais foi executado o fluxo de carga*/
    leituraParametrosModificada(&maximoGeracoes, tamanhoTabela, &SBase, &maximoGeracoesSemAtualizacaoPopulacao, &sequenciaManobrasAlivio, &numeroDesejadoSolucoesFinais);
    leituraDadosEntrada(&numeroSetorFalta, &setorFalta);

    srand(seed);
    inicio = clock();
    indiceReguladores = Malloc(int, (numeroBarras + 5));
    numeroPosicoesAlocadas = (maximoGeracoes + 3);
    //Nota: se o operador de corte (LSO) for utilizado, então o número de RNPs Fictícia não pode ser fixo, como é neste caso.
    //numeroRNPFicticia = obtemNumeroSubarvoresDesligadas(grafoSetoresParam, numeroSetorFalta, setorFalta, numeroPosicoesAlocadas,  configuracaoInicial, idConfiguracaoInicial, rnpSetoresParam, grafoSDRParam); //Por Leandro: número de RNP Fictícia da configuração inicial do problema (aquela na qual todos os setores desligados encontram-se ainda desconectados)
	inicializaVetorPi(numeroPosicoesAlocadas, &vetorPi);
	inicializaMatrizPI(grafoSetoresParam, &matrizPI, numeroPosicoesAlocadas, numeroSetores+numeroRNPFicticia);  //Leandro: deve-se adicionar à matriz PI os setores artificiais que consistem nos nós-raiz das RNPs Fictícias.
	configuracoes = alocaIndividuoModificadaTrifasico(numeroAlimentadores, numeroRNPFicticia, idConfiguracao, numeroPosicoesAlocadas, numeroTrafos, numeroSetores);//Por Leandro: mudou a função que faz a alocação
	// Copia o Individuo Inicial para a posição 0 de "configuracoes"
	for (contadorRnp = 0; contadorRnp < configuracaoInicial[0].numeroRNP; contadorRnp++) {
		alocaRNP(configuracaoInicial[0].rnp[contadorRnp].numeroNos, &configuracoes[idConfiguracao].rnp[contadorRnp]);
        for (contadorNos = 0; contadorNos < configuracaoInicial[0].rnp[contadorRnp].numeroNos; contadorNos++) {
			configuracoes[idConfiguracao].rnp[contadorRnp].nos[contadorNos] = configuracaoInicial[0].rnp[contadorRnp].nos[contadorNos];
            adicionaColuna(matrizPI, configuracoes[idConfiguracao].rnp[contadorRnp].nos[contadorNos].idNo, idConfiguracao, contadorRnp, contadorNos);
        }
	}
	inicializaRNPsFicticias(configuracoes, idConfiguracao, numeroRNPFicticia, matrizPI, numeroSetores); //Por Leandro: Inicializa as RNPs Fictícias para a configuração "idConfiguracao"
	leituraVetorTaps(indiceReguladores, dadosReguladorSDR, numeroReguladores, numeroBarras);

	inicializaDadosEletricosPorAlimentadorTrifasico(grafoSDRParam, configuracoes, idConfiguracao, numeroBarras, SBase, dadosTrafoSDRParam, dadosAlimentadorSDRParam, rnpSetoresParam);
    avaliaConfiguracaoModificada(todosAlimentadores, configuracoes, -1, -1, idConfiguracao,
								dadosTrafoSDRParam, numeroTrafos, numeroAlimentadores, indiceReguladores,
								dadosReguladorSDR, dadosAlimentadorSDRParam, idConfiguracao,
								rnpSetoresParam, Z, maximoCorrente, numeroBarras, copiaDados,
								grafoSDRParam, sequenciaManobrasAlivio, listaChavesParam, vetorPi, false);
    //imprimeDadosEletricosTrifasicos(configuracoes, 0, numeroBarras);
    imprimeDadosEletricosTrifasicos_POLAR(configuracoes, 0, numeroBarras);
    /*Encerra a contagem de tempo */
    fim = clock();
    tempo = (double)(fim-inicio)/CLOCK_TICKS_PER_SECOND;

    printf("Tempo de execucao: %lf\n", tempo);

    // if(numeroDeFronteiras > 0){
    //     free(idIndividuosFinais);
    //     free(idSolucoesFinais);
    // }
    // free(configuracoes);
    // desalocaMatrizPI(matrizPI, numeroSetores);
    // free(matrizPI);
    // //desalocaTabelas(numeroTabelas, populacao);
    // //free(populacao);
    // desalocaVetorPi((maximoGeracoes), vetorPi);
    // free(vetorPi);
    // free(indiceReguladores);
}


/*
* Por Leandro
* DescriÃ§Ã£o: determina o nÃºmero de subÃ¡rvores Ã  jusante do setor de falta CAPAZES DE SEREM
* RESTABELECIDAS por meio de chaves normalmente abertas, a fim de saber o nÃºmero de RNPs FictÃ­cias
* a serem alocadas.
* Note que as subÃ¡rvores que nÃ£o podem ser reconectadas, devido a ausÃªncia de chaves, nÃ£o sÃ£o
* contabilizadas
*
* @param
*/
long int obtemNumeroSubarvoresDesligadas(GRAFOSETORES *grafoSetoresParam, int numeroSetorFaltaParam, long int *setoresFaltaParam, int numeroPosicoesAlocadasParam, CONFIGURACAO *configuracaoInicial, long int idConfiguracaoParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam) {

   CONFIGURACAO *configuracoesTemp;
   MATRIZPI *matrizPiTemp;
   VETORPI *vetorPiTemp;
   long int idConfiguracao, numeroRNPFicticia, numeroRNPFicticiaPorSetorFalta = 0, numeroConfiguracoesTemp;
   int contadorRnp, contadorNos, indice, contador;

   inicializaVetorPi(numeroPosicoesAlocadasParam, &vetorPiTemp);        //Por Leandro
   inicializaMatrizPI(grafoSetoresParam, &matrizPiTemp, numeroPosicoesAlocadasParam, numeroSetores); //Por Leandro
   idConfiguracao = 0;
   numeroConfiguracoesTemp = 1;
   configuracoesTemp = alocaIndividuoTrifasico(configuracaoInicial[0].numeroRNP, idConfiguracao, numeroConfiguracoesTemp, 1); //Por Leandro: mudou a funÃ§Ã£o que faz a alocaÃ§Ã£o

   //Por Leandro: Aloca somente a variÃ¡vel temporÃ¡ria "configuracoesTemp". A variÃ¡vel "configuracoes" serÃ¡ alocada somente se a BE nÃ£o retornar uma soluÃ§Ã£o factÃ­vel
    
   for (contadorRnp = 0; contadorRnp < configuracaoInicial[0].numeroRNP; contadorRnp++) {
       alocaRNP(configuracaoInicial[0].rnp[contadorRnp].numeroNos, &configuracoesTemp[idConfiguracao].rnp[contadorRnp]); //Por Leandro
       for (contadorNos = 0; contadorNos < configuracaoInicial[0].rnp[contadorRnp].numeroNos; contadorNos++) {
           configuracoesTemp[idConfiguracao].rnp[contadorRnp].nos[contadorNos] = configuracaoInicial[0].rnp[contadorRnp].nos[contadorNos];              //Por Leandro
           adicionaColuna(matrizPiTemp, configuracoesTemp[idConfiguracao].rnp[contadorRnp].nos[contadorNos].idNo, idConfiguracao, contadorRnp, contadorNos);//Por Leandro
       }
   }

   numeroRNPFicticia = 0;
   for(indice = 0; indice < numeroSetorFaltaParam; indice++){
	   numeroRNPFicticiaPorSetorFalta = determinaNumeroSubarvoresDesligadasReconectaveis(grafoSetoresParam, idConfiguracaoParam, configuracoesTemp, matrizPiTemp, vetorPiTemp, rnpSetoresParam, grafoSDRParam, setoresFaltaParam[indice]); //Por Leandro
       numeroRNPFicticia = numeroRNPFicticia + numeroRNPFicticiaPorSetorFalta;
   }
   //Desaloca as variÃ¡veis temporÃ¡rias
   for (contador = 0; contador < numeroConfiguracoesTemp; contador++) {
	   configuracoesTemp[contador].numeroRNPFicticia = 0;
   }
   
   desalocacaoCompletaConfiguracao(configuracoesTemp, numeroConfiguracoesTemp);
   desalocaMatrizPI(matrizPiTemp, numeroSetores);
   free(matrizPiTemp);
   desalocaVetorPi(numeroPosicoesAlocadasParam, vetorPiTemp);
   free(vetorPiTemp);

   return numeroRNPFicticia;
}


/*
* Por Leandro
* DescriÃ§Ã£o: determina o nÃºmero de subÃ¡rvores Ã  jusante do setor de falta CAPAZES DE SEREM
* RESTABELECIDAS por meio de chaves normalmente abertas E que possuam consumidores.
* Dada uma certa subÃ¡rvore, mesmo que seja possÃ­vel reconectÃ¡-la por meio de chaves NAs, a mesma nÃ£o serÃ¡
* contabilida caso nÃ£o possua, em seus setores, consumidores. Isto visa evitar manobras de reconexÃ£o que nÃ£o beneficiariam a nenhum consumidor
* Portanto, nÃ£o sÃ£o contabilizadas:
* a) subÃ¡rvores que nÃ£o podem ser reconectadas, devido a ausÃªncia de chaves NAs, e;
* b) subÃ¡rvores sem consumidores;
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
long int determinaNumeroSubarvoresDesligadasReconectaveis(GRAFOSETORES *grafoSetoresParam, long int idConfiguracaoParam,
       CONFIGURACAO *configuracoesParam, MATRIZPI *matrizPiParam, VETORPI *vetorPiParam, RNPSETORES *rnpSetoresParam, GRAFO *grafoSDRParam, long int setorFaltaParam) {

   long int a, p, indice, indice1, indiceP, indiceL, indiceSetorFalta, indiceA;
   long int noAnteriorFalta;
   long int indiceLFalta;
   int rnpP;
   int numeroFalta, numeroAJusante, indiceAux1;
   long int *nosFalta = Malloc(long int, 1000);//nosFalta[500]; //Ã© o nÃºmero mÃ¡ximo de setores em falta ou a jusantes
   long int *nosJusanteFalta = Malloc(long int, 500); //nosJusanteFalta[100]; //contÃ©m as posiÃ§Ãµes da rnp referente a raiz de subÃ¡rvore de nÃ³s a jusante a falta
   int blocoRestabelecimento=0;
   int colunaPI;
   NOSRESTABELECIMENTO *nos;
   NIVEISDEPRIORIDADEATENDIMENTO potenciaNaoSuprida;
   double potenciaNaoSupridaTotal;
   long int numeroSubarvoresDesligadas = 0;

   /*//marca o setor em falta na lista de adjacencias para evitar a sua escolha como setor adjacente nos operadores PAO e CAO
   grafoSetoresParam[setorFaltaParam].setorFalta = true;
   //INICIALIZA A CONTAGEM DE CONSUMIDORES SEM FORNECIMENTO
 //  consumidoresSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidores;
  // consumidoresEspeciaisSemFornecimento += grafoSetoresParam[setorFaltaParam].numeroConsumidoresEspeciais;
*/
   //RECUPERA A COLUNAPI DO SETOR EM FALTA
   //colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, setorFaltaParam, idConfiguracaoParam);
   //rnpP = matrizPiParam[setorFaltaParam].colunas[colunaPI].idRNP;
   //considera como possÃ­vel nÃ³ P o nÃ³ subsequente ao setor em falta na RNP, deve ser verificado se a posiÃ§Ã£o existe e se ela corresponde a setores a jusante a falta.
   //indiceP = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao + 1;
   //indiceSetorFalta = matrizPiParam[setorFaltaParam].colunas[colunaPI].posicao; //indice do setor em falta na RNP.

   //LOCALIZA O SETOR ANTERIOR A FALTA
   //if(indiceSetorFalta == 0){
	   //printf("\nO setor em falta informado Ã© um setor raiz, ou seja, de uma barra em uma subestaÃ§Ã£o.\n");
	   //exit(0);
   //}
   indice1 = indiceSetorFalta - 1; //Ao final do laÃ§o de repetiÃ§Ã£o, 'indice1' armazenarÃ¡ o Ã­ndice do nÃ³ antecessor ao nÃ³ sob falta
   while (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].profundidade != (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade - 1)) {
       indice1--;
   }
   noAnteriorFalta = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indice1].idNo;

   //verifica se o setor em falta nÃ£o Ã© o Ãºltimo da RNP
   if (indiceP < configuracoesParam[idConfiguracaoParam].rnp[rnpP].numeroNos) {
       numeroAJusante = 0;
       //verifica se o setor da posiÃ§Ã£o do indiceP corresponde a um setor a jusante a falta
       if (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].profundidade == (configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceSetorFalta].profundidade + 1)) {
           indice1 = indiceP + 1;
           nosJusanteFalta[numeroAJusante] = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo;
           numeroAJusante++;
           //recupera a raiz de todas as subÃ¡rvores que contÃ©m setores a jusante do setor em falta
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
           blocoRestabelecimento = 0;
           //percorre a lista de raizes de subÃ¡rvore a jusante do setor em falta verificando as opÃ§Ãµes de restabelecimento
           while (indice < numeroAJusante) {
               a = 0;
               colunaPI = retornaColunaPi(matrizPiParam, vetorPiParam, nosJusanteFalta[indice], idConfiguracaoParam);
               indiceP = matrizPiParam[nosJusanteFalta[indice]].colunas[colunaPI].posicao;
               p = configuracoesParam[idConfiguracaoParam].rnp[rnpP].nos[indiceP].idNo; // O nÃ³ 'p' Ã© o nÃ³ subsequente ao nÃ³ 'setor em falta'
               //recupera o indice final da subÃ¡rvore dos setore a jusante
               indiceL = limiteSubArvore(configuracoesParam[idConfiguracaoParam].rnp[rnpP], indiceP);
               nos[blocoRestabelecimento].noA = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].noR = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].noP = Malloc(long int, (indiceL - indiceP + 1)*6);
               nos[blocoRestabelecimento].rnpA = Malloc(int, (indiceL - indiceP + 1)*10);
               buscaTodosNosRestabelecimentoCorrigida(configuracoesParam, rnpP, indiceP, grafoSetoresParam, idConfiguracaoParam, indiceL, nosFalta, numeroFalta, matrizPiParam, vetorPiParam, &nos[blocoRestabelecimento], indiceSetorFalta);
               potenciaNaoSuprida = calculaSomatorioPotenciaAtivaSubarvorePorNivelDePrioridade(configuracoesParam, idConfiguracaoParam, p, rnpSetoresParam, matrizPiParam, vetorPiParam, grafoSDRParam);
               potenciaNaoSupridaTotal = potenciaNaoSuprida.consumidoresSemPrioridade + potenciaNaoSuprida.consumidoresPrioridadeBaixa + potenciaNaoSuprida.consumidoresPrioridadeIntermediaria + potenciaNaoSuprida.consumidoresPrioridadeAlta;

               //printf("setor a jusante %ld quantidade restabelecimento %ld \n", nosJusanteFalta[indice], nos[blocoRestabelecimento].numeroNos);
               if (nos[blocoRestabelecimento].numeroNos > 0 && potenciaNaoSupridaTotal > 0) {
                   blocoRestabelecimento++;
                   numeroSubarvoresDesligadas++; //Considera apenas as subÃ¡rvores capazes de serem restauradas. As demais sÃ£o desconsideradas
               }
               else{
                   //adiciona os setores a jusante sem opÃ§ao de restabelecimento ao grupo de setores em falta
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
           if(numeroAJusante > 0){
        	   for(indice = 0; indice < blocoRestabelecimento; indice++){
				  free(nos[indice].noA);
				  free(nos[indice].noR);
				  free(nos[indice].noP);
				  free(nos[indice].rnpA);
				}
				free(nos);
           }
       }
       else //nÃ£o existem setores a jusante para ser restabelecidos
    	   numeroSubarvoresDesligadas = 0;
   }
   else //o setor em falta Ã© o ultimo da rnp. EntÃ£o nÃ£o hÃ¡ subÃ¡vores saudÃ¡veis desligadas
	   numeroSubarvoresDesligadas = 0;
    
   free(nosFalta);
   free(nosJusanteFalta);

   return numeroSubarvoresDesligadas;
}

/* Descrição:
 * Esta função adiciona um novo alimentador (árvore ou rnp) à lista que contém os alimentadores envolvidos no problema.
 * Esta lista será utilizada na avaliação da solução, de forma que os valores das restrições serão determinados somente para
 * estes alimentadores.
 *
 * Obs.: Heurística que restringe a avaliação das configurações á região do problema.
 * 
 * @param configuracaoParam
 * @param rnp
 */
void insereRNPLista(CONFIGURACAO *configuracaoParam, int rnp)
{
    int contador;
    
    //verifica se a rnp passada como parâmetro não pertence a lista
    contador = 0;
    while (contador < configuracaoParam[0].numeroRNPsFalta && configuracaoParam[0].idRnpFalta[contador] != rnp)
        contador++;
    //a rnp não pertence a lista
    if(contador == configuracaoParam[0].numeroRNPsFalta)
    {
        configuracaoParam[0].idRnpFalta[contador] = rnp;
        configuracaoParam[0].numeroRNPsFalta++;
    }
}


