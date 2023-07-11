/**
 * @file funcoesAuxiliares.c
 * @brief Este arquivo contém a implementação de funções auxiliares para o desenvolvimento do MRAN. 
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include "data_structures.h"
/**
 * @brief A função RandomInteger devolve um inteiro 
* aleatório entre inferior e superior.
 * @param inferior inteiro com o limite inferior para o intervalo em que se deseja gerar o número aleatório
 * @param superior inteiro com o limite superior para o intervalo em que se deseja gerar o número aleatório
 * @return 
 */
int inteiroAleatorio( int inferior, int superior)
{
    int k;
    int d;
   // d = inferior + rand( ) % superior;
    d=  rand() % (superior - inferior + 1) + inferior;
   // k = d * (superior - inferior + 1);
    return d;
}

/**
 * Função para realizar a alocação de memória para uma matriz de inteiros com ponteiros.
 * @param matrizParam ponteiro para a estrutura da matriz
 * @param numeroLinhas inteiro com o número de linhas da matriz
 * @param numeroColunas inteiro com o número de colunas da matriz
 * @return 
 */
void alocaMatrizInteiros(long int ***matrizParam, long int numeroLinhas, long int numeroColunas)
{
    long int contadorL;
    //faz a alocação da matriz
    if (((*matrizParam) = (long int **)malloc((numeroLinhas) * sizeof(long int *)))==NULL)
    {
        printf("ERRO ao alocar memoria para matriz de Inteiros!!!");
        exit(1);
    }
    for(contadorL= 0; contadorL< numeroLinhas; contadorL++)
    {
        (*matrizParam)[contadorL] = (long int*) malloc(numeroColunas * sizeof(long int));
    }
}

/**
 * Função para realizar a liberação de memória de uma matriz de inteiros com ponteiros.
 * @param matrizParam ponteiros da matriz
 * @param numeroLinhas inteiro com o número de linhas da matriz
 * @return 
 */
void desalocaMatrizInteiros(long int **matrizParam, long int numeroLinhas)
{
    long int contadorL;
    //faz a desalocação da matriz
     for(contadorL= 0; contadorL< numeroLinhas; contadorL++)
    {
        free(matrizParam[contadorL]);
    }
    free(matrizParam);
}

/**
 * Função para realizar a liberação de memória alocada para a estrutura de dados do tipo GRAFO.
 * @param grafoSDRParam ponteiro para a estrutura de dados do tipo GRAFO
 * @param numeroBarras inteiro com o número de Barras do grafo
 * @return 
 */
void desalocaGrafoSDR(GRAFO *grafoSDRParam, long int numeroBarras)
{
    long int contador;
    /*for(contador = 1; contador<numeroBarras; contador++)
    {
        free(grafoSDRParam[contador].adjacentes);
    }*/
    free(grafoSDRParam);

}

/**
 * Função para realizar a alocação de memória para uma matriz do tipo double com ponteiros.
 * @param matrizParam ponteiro para a estrutura da matriz
 * @param numeroLinhas inteiro com o número de linhas da matriz
 * @param numeroColunas inteiro com o número de colunas da matriz
 * @return 
 */

void alocaMatrizDouble(double ***matrizParam, long int numeroLinhas, long int numeroColunas)
{
    long int contadorL;
    //faz a alocação da matriz
    if (((*matrizParam) = (double **)malloc((numeroLinhas) * sizeof(double *)))==NULL)
    {
        printf("ERRO ao alocar memoria para matriz de Doubles!!!");
        exit(1);
    }
    for(contadorL= 0; contadorL< numeroLinhas; contadorL++)
    {
        (*matrizParam)[contadorL] = (double*) malloc(numeroColunas * sizeof(double));
    }
}

/**
 * Função para realizar a liberação da memória alocada para armazenar a estrutura de subpopulações do algoritmo evolutivo.
 * @param numeroTabelasParam inteiro com o número de subpopulações utilizadas
 * @param populacaoParam ponteiro para estrutura do tipo VETORTABELA
 * @return 
 */
void desalocaTabelas(int numeroTabelasParam, VETORTABELA *populacaoParam) {
    int contador;
    //aloca o número de tabelas especificado
    
    //aloca para cada tabela o número de condigurações especificado
    for (contador = 0; contador < numeroTabelasParam; contador++) 
    {
        free(populacaoParam[contador].tabela);
    }
}

/*Por Leandro: zera as posições não nulas de uma string
*/
void limparString(char *str){
	int indice = 0;
	while(str[indice] != NULL){
		str[indice] = NULL;
		indice++;
	}
}
