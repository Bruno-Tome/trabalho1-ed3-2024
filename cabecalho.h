#ifndef CABECALHO_H
#define CABECALHO_H
#include <stdio.h>
#include <stdlib.h>

// Definição de estruturas
typedef struct {
    char status;            // '0' inconsistente, '1' consistente
    int topo;               // RRN do primeiro registro removido logicamente
    int proxRRN;            // Próximo RRN disponível para inserção
    int nroRegRem;          // Número de registros logicamente removidos
    int nroPagDisco;        // Número de páginas de disco ocupadas
    int qttCompacta;        // Quantidade de compactações
} Cabecalho;



Cabecalho inicializarCabecalho();

void escreverCabecalhoBin(FILE *fp, Cabecalho *header);

void lerCabecalhoBin(FILE *fp, Cabecalho *header);



#endif