/* 
 * File:   funcoesAuxiliares.h
 * Author: sony
 *
 * Created on 20 de Novembro de 2013, 19:49
 */
/**
 * @file funcoesAuxiliares.h
 * @brief Este arquivo contém os protótipos das funções implementadas no arquivo funcoesAuxiliares.c.
 * 
 */
#ifndef FUNCOESAUXILIARES_H
#define	FUNCOESAUXILIARES_H

int inteiroAleatorio( int inferior, int superior);

void alocaMatrizInteiros(long int ***matrizParam, long int numeroLinhas, long int numeroColunas);
void alocaMatrizDouble(double ***matrizParam, long int numeroLinhas, long int numeroColunas);
void desalocaMatrizInteiros(long int **matrizParam, long int numeroLinhas);
void desalocaGrafoSDR(GRAFO *grafoSDRParam, long int numeroBarras);
void desalocaTabelas(int numeroTabelasParam, VETORTABELA *populacaoParam);
void limparString(char *str);
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* FUNCOESAUXILIARES_H */

