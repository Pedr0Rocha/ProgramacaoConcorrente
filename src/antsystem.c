#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "leitor.h"
#include "utils.h"
#include "estruturas.h"
#include "heuristica.h"

#define MAX_CICLOS 40
#define QTA_FORMIGAS 100

int matrizInicial[4][4];
int matrizResposta[4][4];

node raizArvore;
listaLigada *nodesInseridosArvore = NULL;

// heuristica usada, 1 - manhattan distance, 0 - order
int heuristicaUsada = 0;

// peso aplicado no feromonio
double alfa = 0.10;
// peso aplicado na heuristica
double beta = 1;
// taxa de evapocarao
double rho = 0.5;

/*
	modelagem das equações

	equacao de probabilidade
	prob do caminho = [feromonio pra depositar]^alfa   *  [(1/valorHeuristica + 0.01)]^beta 
					   ---------------------------------------------------------------------
					   Somatório([feromonio pra depositar]^alfa   *  [(1/valorHeuristica + 0.01)]^beta)

	feromonio pra depositar é a quantidade de feromonio que cada formiga depositará nos nodes 
	do caminho encontrado. este valor é alterado depois da formiga terminar de achar a solução. valor = 1 (primeira iteração)

	para calcular o novo feromonio a depositar na trilha usa-se a formula
	fer = fer * (1 - rho) + delta

	rho é a taxa de evaporação. valor = 0.5
	
	delta = (10 / numero de movimentos para chegar na soluçao)
	quanto menor o numero de movimentos, maior o delta e mais feromonio sera depositado naquele caminho

	alfa é o peso para o feromonio aplicado nos nodes. valor = 0.10

	valor da heuristica é o valor retornado pela heuristica usada. Manhattan distance
	usa 1/valorHeuristica + 0.1 e order usa apenas o valor. Quanto maior o valor, melhor.
	
	beta é o peso dado para o valor da heuristica. valor = 1

	testes: ftp://ftp.cs.princeton.edu/pub/cs226/8puzzle

*/

void atualizaFeromonioCaminho(formiga *formiga){
	double delta;
	delta = 10 / formiga->movimentos;
	listaLigada *atual = formiga->caminho;

	while (atual != NULL){
		if (formiga->resolvido)
			atual->nodeAtual->feromonio = atual->nodeAtual->feromonio * (1 - rho) + delta;
		else 
			atual->nodeAtual->feromonio *= 0.7;

		atual = atual->prev;
	}
}

void atualizaFeromonioGlobal(){
	listaLigada *atual = nodesInseridosArvore;
	while (atual != NULL){
		nodesInseridosArvore->nodeAtual->feromonio *= rho;
		atual = atual->prev;
	}
}

void inicializaFilho(node *filho){
	filho->valorHeuristica = calculaHeuristica(matrizResposta, filho->matriz, heuristicaUsada);
	filho->feromonio = 1;
	filho->filhos = NULL;
}

void geraNode(node *nodeOrigem) {
	par zeroPos;
	zeroPos = achaPosicaoZero(nodeOrigem->matriz);	
	int qtaFilhos = 0;
	node *ptNode;
	// vizinho na coluna da esquerda
	//printf("gerandoNode\n");
	if (zeroPos.y - 1 >= 0) {
		int matrizTemp[4][4];
		cloneArray(nodeOrigem->matriz, matrizTemp);
		matrizTemp[zeroPos.x][zeroPos.y] = matrizTemp[zeroPos.x][zeroPos.y - 1];
		matrizTemp[zeroPos.x][zeroPos.y - 1] = 0;
		qtaFilhos++;
		if (!(ptNode = getNoCaminhoExiste(matrizTemp, nodesInseridosArvore))){
			node *filhoEsquerda = malloc(sizeof(node));
			cloneArray(nodeOrigem->matriz, filhoEsquerda->matriz);
			filhoEsquerda->matriz[zeroPos.x][zeroPos.y] = filhoEsquerda->matriz[zeroPos.x][zeroPos.y - 1];
			filhoEsquerda->matriz[zeroPos.x][zeroPos.y - 1] = 0;
			inicializaFilho(filhoEsquerda);
			insereListaLigada(filhoEsquerda, &nodeOrigem->filhos);
		} else {
			insereListaLigada(ptNode, &nodeOrigem->filhos);
		}
	}
	// vizinho na coluna da direita
	if (zeroPos.y + 1 < 4) {
		int matrizTemp[4][4];
		cloneArray(nodeOrigem->matriz, matrizTemp);
		matrizTemp[zeroPos.x][zeroPos.y] = matrizTemp[zeroPos.x][zeroPos.y + 1];
		matrizTemp[zeroPos.x][zeroPos.y + 1] = 0;
		qtaFilhos++;
		if (!(ptNode = getNoCaminhoExiste(matrizTemp, nodesInseridosArvore))){
			node *filhoDireita = malloc(sizeof(node));
			cloneArray(nodeOrigem->matriz, filhoDireita->matriz);
			filhoDireita->matriz[zeroPos.x][zeroPos.y] = filhoDireita->matriz[zeroPos.x][zeroPos.y + 1];
			filhoDireita->matriz[zeroPos.x][zeroPos.y + 1] = 0;
			inicializaFilho(filhoDireita);
			insereListaLigada(filhoDireita, &nodeOrigem->filhos);
		} else {
			insereListaLigada(ptNode, &nodeOrigem->filhos);
		}
	}
	// vizinho na linha de cima
	if (zeroPos.x - 1 >= 0) {
		int matrizTemp[4][4];
		cloneArray(nodeOrigem->matriz, matrizTemp);
		matrizTemp[zeroPos.x][zeroPos.y] = matrizTemp[zeroPos.x - 1][zeroPos.y];
		matrizTemp[zeroPos.x - 1][zeroPos.y] = 0;
		qtaFilhos++;
		if (!(ptNode = getNoCaminhoExiste(matrizTemp, nodesInseridosArvore))){
			node *filhoCima = malloc(sizeof(node));
			cloneArray(nodeOrigem->matriz, filhoCima->matriz);
			filhoCima->matriz[zeroPos.x][zeroPos.y] = filhoCima->matriz[zeroPos.x - 1][zeroPos.y];
			filhoCima->matriz[zeroPos.x - 1][zeroPos.y] = 0;
			inicializaFilho(filhoCima);
			insereListaLigada(filhoCima, &nodeOrigem->filhos);
		} else {
			insereListaLigada(ptNode, &nodeOrigem->filhos);
		}
	}
	// vizinho na linha de baixo
	if (zeroPos.x + 1 < 4) {
		int matrizTemp[4][4];
		cloneArray(nodeOrigem->matriz, matrizTemp);
		matrizTemp[zeroPos.x][zeroPos.y] = matrizTemp[zeroPos.x + 1][zeroPos.y];
		matrizTemp[zeroPos.x + 1][zeroPos.y] = 0;
		qtaFilhos++;
		if (!(ptNode = getNoCaminhoExiste(matrizTemp, nodesInseridosArvore))){
			node *filhoBaixo = malloc(sizeof(node));
			cloneArray(nodeOrigem->matriz, filhoBaixo->matriz);
			filhoBaixo->matriz[zeroPos.x][zeroPos.y] = filhoBaixo->matriz[zeroPos.x + 1][zeroPos.y];
			filhoBaixo->matriz[zeroPos.x + 1][zeroPos.y] = 0;
			inicializaFilho(filhoBaixo);
			insereListaLigada(filhoBaixo, &nodeOrigem->filhos);
		} else {
			insereListaLigada(ptNode, &nodeOrigem->filhos);
		}
	}
	nodeOrigem->qtaFilhos = qtaFilhos;
}

void inicializaFormigas(formiga *formiga, int index, node *raiz){
	formiga->id = index;
	formiga->caminho = NULL;
	formiga->movimentos = 0;
	formiga->resolvido = 0;
	insereListaLigada(raiz, &formiga->caminho);
}

void inicializaArvore(node *raiz){
	cloneArray(matrizInicial, raiz->matriz);
	raiz->valorHeuristica = 1;
	raiz->feromonio = 1;
	raiz->filhos = NULL;
	insereListaLigada(raiz, &nodesInseridosArvore);
}

// escolhe probabilisticamente o melhor dos filhos
node* escolheFilho(node *nodeAtual){ 
	return selecaoRoleta(nodeAtual->filhos, nodeAtual->qtaFilhos, alfa, beta);
}

 // adiciona o filho escolhido gerado no caminho
void adicionaNoCaminho(formiga *formiga, node *filho){
	//printf("adicionaNoCaminho\n");
	//printf("%p\n", filho->matriz);
	if (estaNoCaminho(filho->matriz, formiga) == 0){
		insereListaLigada(filho, &formiga->caminho);
		formiga->movimentos += 1;
	} else {
		//printf("Ta no caminho\n");
	}
}

void geraSolucao(formiga *formiga, node *raiz) {
	while (matrizIgual(matrizResposta, formiga->caminho->nodeAtual->matriz) != 1){
		if (formiga->caminho->nodeAtual->filhos == NULL){
			geraNode(formiga->caminho->nodeAtual);
			if (todosNoCaminho(formiga)) break;
		}
		node *filho = escolheFilho(formiga->caminho->nodeAtual);
		adicionaNoCaminho(formiga, filho);

		if (formiga->movimentos >= 1000) {
			formiga->resolvido = 0;
			//printf("Limite de movimentos(%d) excedido..\n", 1000);
			break;
		} else {
			formiga->resolvido = 1;
		}
	}	
	//printf("movimentos finais: %d\n", formiga->movimentos);
}

int antsystem(){
	formiga formigas[QTA_FORMIGAS];
	int i, contadorCiclos = 0;
	inicializaArvore(&raizArvore);
	int melhorMovimentos = INT_MAX;
	while (contadorCiclos != MAX_CICLOS){
		for (i = 0; i < QTA_FORMIGAS; i++){
			inicializaFormigas(&formigas[i], i, &raizArvore);
			geraSolucao(&formigas[i], &raizArvore);
		}
		for (i = 0; i < QTA_FORMIGAS; i++){
			if (matrizIgual(formigas[i].caminho->nodeAtual->matriz, matrizResposta)){
				if (formigas[i].movimentos < melhorMovimentos){
					melhorMovimentos = formigas[i].movimentos;
				}			
			}
			atualizaFeromonioCaminho(&formigas[i]);
		}
		contadorCiclos++;
		atualizaFeromonioGlobal();
	}
	return melhorMovimentos;
}

int main(){
	inicializaMatrizResposta(matrizResposta);

	leEntrada("entradas/med/18mov.txt", matrizInicial);
	printf("\n\n");

	int seed = 5;
	inicializaRandom(seed);

	//antsystem();
	printf("Movimentos: %d\n", antsystem());
}