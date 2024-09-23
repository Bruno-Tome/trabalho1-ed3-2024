#include <ctype.h>
#include <string.h>
#include "funcoesFornecidas.h"
#include "cabecalho.h"

#define DELIMITER '#'
#define TRASH '$'
#define PAGE_SIZE 1600

// Função para inicializar o cabeçalho
Cabecalho inicializarCabecalho() {
    Cabecalho header;
    header.status = '0';
    header.topo = -1;
    header.proxRRN = 0;
    header.nroRegRem = 0;
    header.nroPagDisco = 1;
    header.qttCompacta = 0;
    return header;
}

// Função para escrever o cabeçalho do arquivo binário
void escreverCabecalhoBin(FILE *fp, Cabecalho *header) {
    if (fp == NULL) return;

    fwrite(&header->status, sizeof(char), 1, fp);
    fwrite(&header->topo, sizeof(int), 1, fp);
    fwrite(&header->proxRRN, sizeof(int), 1, fp);
    fwrite(&header->nroRegRem, sizeof(int), 1, fp);
    fwrite(&header->nroPagDisco, sizeof(int), 1, fp);
    fwrite(&header->qttCompacta, sizeof(int), 1, fp);

    char lixo = TRASH;
    for (int i = 0; i < 1579; i++) {
        fwrite(&lixo, sizeof(char), 1, fp);
    }
}

// Função para ler o cabeçalho de um arquivo binário e tratar o lixo
void lerCabecalhoBin(FILE *fp, Cabecalho *header) {
    if (fp == NULL || header == NULL) return; // erro

    // Lê os campos do cabeçalho
    fread(&header->status, sizeof(char), 1, fp);
    fread(&header->topo, sizeof(int), 1, fp);
    fread(&header->proxRRN, sizeof(int), 1, fp);
    fread(&header->nroRegRem, sizeof(int), 1, fp);
    fread(&header->nroPagDisco, sizeof(int), 1, fp);
    fread(&header->qttCompacta, sizeof(int), 1, fp);

    // Pular o lixo no cabeçalho (restante da página de 1600 bytes)
    long int currentPos = ftell(fp);  // Posição atual no arquivo
    long int endOfHeaderPage = PAGE_SIZE;  // Tamanho da página de disco para o cabeçalho
    fseek(fp, endOfHeaderPage - currentPos, SEEK_CUR);  // Pula o lixo até o fim da página
}
