#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "estruturas.h"

void inicializaRandom(unsigned long long seed);
double randomDouble();
node* selecaoRoleta(listaLigada *filhos, int qtaFilhos, double alfa, double beta);

#endif