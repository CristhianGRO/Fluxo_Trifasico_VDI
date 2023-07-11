/**
 * @file funcoesSetor.h
 * @brief Este arquivo contém os protótipos das funções implementadas no arquivo funcoesSetor.c.
 * 
 */

#ifndef FUNCOESSETOR_H
#define	FUNCOESSETOR_H
#include "data_structures.h"


void adicionaNo(NOSETOR ** listaSetores, long int idNo);

void imprimeSetor(NOSETOR * setor);



void adicionaSetorNaLista(LISTASETORES ** listaSetores, NOSETOR * setor, NOSETOR *verticesAdj);

void imprimeSetores(LISTASETORES * listaSetores);



void adicionaNoNaFila(FILABARRAS ** fila, long int idNo);

int retiraNoDaFila(FILABARRAS ** fila);




BOOL filaNaoVazia(FILABARRAS * fila);


BOOL ligadoPorTrecho(NOADJACENTE adjacente);

BOOL chaveFechada(NOADJACENTE adjacente);

void buscaSetor(GRAFO *grafo, long int idNo, LISTASETORES **listaSetores, BOOL *visitado);

LISTASETORES * buscaSetores(GRAFO *grafo, DADOSALIMENTADOR *alimentadores, long int numeroBarras);


void constroiRNP(NOSETOR *inicioListaSetor, long int idNo, BOOL *visitado, RNPSETOR *rnpParam, int *indice, int profundidade, GRAFO *grafoSDRParam);

void percorreAdjacentesSetor(NOSETOR *setorParam, NOSETOR *setorAdjParam, GRAFO *grafoSDRParam, RNPSETOR *rnps,  int numeroNos, long int numeroBarras);

void constroiRNPSetores(LISTASETORES *listaSetores, GRAFO *grafoSDRParam, RNPSETORES **rnpSetores, long int numeroBarras);

void imprimeRNPSetores (RNPSETORES *rnpSetoresParam);

void constroiListaAdjacenciaSetores(GRAFO *grafoSDRParam, LISTASETORES *listaSetores, LISTACHAVES *listaChavesParam, GRAFOSETORES **grafoSetoresParam, long int numeroChaves);

void imprimeGrafoSetores(GRAFOSETORES *grafoSetoresParam);

BOOL ligadoPorTrecho(NOADJACENTE adjacente);

BOOL chaveFechada(NOADJACENTE adjacente);

BOOL estaListaAdjacencias(GRAFO *grafoSdrParam, long int idNoRaiz, long int idNoAdj);

//realiza a busca a rnp do idsetor1 sendo fornecida pelo idSetor2 e retorna essa rnp
RNPSETOR buscaRNPSetor(RNPSETORES *rnpSetoresParam, long int idSetor1, long int idSetor2);

void imprimeRNPSetor (RNPSETOR rnpSetorParam);

void gravaRNPSetores (RNPSETORES *rnpSetoresParam, long int numeroSetores);

void gravaGrafoSetores(GRAFOSETORES *grafoSetoresParam, long int numeroSetores, LISTACHAVES *listaChavesParam);

void desalocaRNPSetores(RNPSETORES *rnpSetores, long int numeroSetores);

void desalocaGrafoSetores(GRAFOSETORES *grafoSetoresParam, long int numeroSetores);

#endif	/* FUNCOESLEITURADADOS_H */

