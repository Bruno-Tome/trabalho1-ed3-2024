#ifndef FUNCOESBD_H
#define FUNCOESBD_H

#include "registro.h"  // Inclui a estrutura de dados para registros e cabeçalhos

// Definições das constantes
#define TAM_CABECALHO 1600   // Tamanho fixo para o cabeçalho (1 página de disco)
#define TAM_REGISTRO 160     // Tamanho fixo para cada registro (160 bytes)
#define RRN_NULO -1          // Indica que não há próximo RRN (para lista de removidos)
#define TAM_PAG_DISCO 1600   // Tamanho de uma página de disco em bytes

// Funções principais de manipulação do arquivo binário

// Função para criar um arquivo binário a partir de um arquivo CSV
void criaArquivoBinario(const char *nomeArquivoCSV, const char *nomeArquivoBin);

// Função para recuperar todos os registros de um arquivo binário (SELECT FROM)
void recuperaTodosRegistros(const char *nomeArquivoBin);

// Função para recuperar registros específicos de acordo com o critério de busca (SELECT WHERE)
void buscaRegistro(const char *nomeArquivoBin, const char *campo, const char *valor);

// Função para remover logicamente um registro específico (DELETE)
void removeRegistroLogico(const char *nomeArquivoBin, int rrn);

// Função para inserir um novo registro no arquivo binário (INSERT)
void insereRegistro(const char *nomeArquivoBin, Registro registro);

// Função para compactar o arquivo, removendo registros logicamente removidos (COMPACT)
void compactaArquivo(const char *nomeArquivoBin);

// Funções auxiliares

// Função auxiliar para escrever strings de tamanho variável no arquivo binário
void escreveStringVariavel(FILE *arquivoBin, const char *str);

// Função auxiliar para ler strings de tamanho variável do arquivo binário
char* leStringVariavel(FILE *arquivoBin);

void parseCSVLine(char *line, char **fields, int *numFields);

#endif // FUNCOESBD_H
