#ifndef REGISTRO_H
#define REGISTRO_H
#include <stdio.h>
#include <stdlib.h>
#include "cabecalho.h"

typedef struct {
    char removido;          // '0' não removido, '1' removido logicamente
    int encadeamento;       // Próximo na lista de registros removidos
    int populacao;          // População da espécie
    float tamanho;          // Tamanho do indivíduo
    char unidadeMedida;     // Unidade de medida (char único)
    int velocidade;         // Velocidade do indivíduo
    char *nome;             // Nome da espécie
    char *especie;          // Nome científico da espécie
    char *habitat;          // Habitat da espécie
    char *tipo;             // Tipo da espécie
    char *dieta;            // Dieta da espécie
    char *alimento;         // Alimento da espécie
} Registro;



void liberarRegistro(Registro *reg);

Registro *criarRegistro();

void escreverRegistro(FILE *fp, Registro *reg);

// Função para ler um registro binário do arquivo e tratar o lixo
Registro *lerRegistroBin(FILE *fp);

Registro *lerRegistroCSV(FILE *fp);

int buscarRegistro(Registro *r, const char *campo, const char *valor);

void removeRegistro(Registro *r, int *topo, long int byteOffset);

void inserirRegistro(FILE *fp, Registro *reg, Cabecalho *header);


void compactarArquivoBinario(FILE *arquivoOriginal, FILE *arquivoCompactado);

#endif