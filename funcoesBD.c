#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoesBD.h"
#include "registro.h"
#include "funcoesFornecidas.h"  // Funções fornecidas

// Definições das constantes
#define TAM_CABECALHO 1600   // Tamanho fixo para o cabeçalho (1 página de disco)
#define TAM_REGISTRO 160     // Tamanho fixo para cada registro (160 bytes)
#define RRN_NULO -1          // Indica que não há próximo RRN (para lista de removidos)
#define TAM_PAG_DISCO 1600   // Tamanho de uma página de disco em bytes
#define MAX_FIELDS 20
#define MAX_FIELD_SIZE 256

// Função para escrever uma string de tamanho variável no arquivo binário
void escreveStringVariavel(FILE *arquivoBin, const char *str) {
    if (str == NULL || strcmp(str, "") == 0) {
        int tamanhoNulo = 1;
        char delimitador = '#';
        fwrite(&tamanhoNulo, sizeof(int), 1, arquivoBin);
        fwrite(&delimitador, sizeof(char), 1, arquivoBin);
    } else {
        int tamanhoString = strlen(str);
        fwrite(&tamanhoString, sizeof(int), 1, arquivoBin);
        fwrite(str, sizeof(char), tamanhoString, arquivoBin);
    }
}

// Função para analisar uma linha CSV em campos, considerando aspas
void parseCSVLine(char *line, char **fields, int *numFields) {
    char *ptr = line;
    int fieldIndex = 0;
    while (*ptr != '\0' && fieldIndex < MAX_FIELDS) {
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') ptr++;
        if (*ptr == '"') {
            ptr++; // Pula a aspas inicial
            char *start = ptr;
            while (*ptr != '"' && *ptr != '\0') ptr++;
            int len = ptr - start;
            fields[fieldIndex] = (char *)malloc(len + 1);
            strncpy(fields[fieldIndex], start, len);
            fields[fieldIndex][len] = '\0';
            if (*ptr == '"') ptr++; // Pula a aspas final
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == ',') ptr++;
        } else {
            char *start = ptr;
            while (*ptr != ',' && *ptr != '\0' && *ptr != '\n') ptr++;
            int len = ptr - start;
            fields[fieldIndex] = (char *)malloc(len + 1);
            strncpy(fields[fieldIndex], start, len);
            fields[fieldIndex][len] = '\0';
            if (*ptr == ',') ptr++;
        }
        fieldIndex++;
    }
    *numFields = fieldIndex;
}

// Função para criar um arquivo binário a partir de um arquivo CSV
void criaArquivoBinario(const char *nomeArquivoCSV, const char *nomeArquivoBin) {
    FILE *arquivoCSV = fopen(nomeArquivoCSV, "r");
    FILE *arquivoBin = fopen(nomeArquivoBin, "wb");

    if (!arquivoCSV || !arquivoBin) {
        printf("Falha no processamento do arquivo.\n");
        if (arquivoCSV) fclose(arquivoCSV);
        if (arquivoBin) fclose(arquivoBin);
        return;
    }

    // Inicializa e escreve o cabeçalho
    Cabecalho cabecalho;
    inicializaCabecalho(&cabecalho);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);

    // Preenche o restante da página de disco com lixo
    char lixo = '$';
    int bytesEscritos = sizeof(Cabecalho);
    while (bytesEscritos < TAM_PAG_DISCO) {
        fwrite(&lixo, sizeof(char), 1, arquivoBin);
        bytesEscritos += sizeof(char);
    }

    Registro registro;
    int rrnAtual = 0;

    // Lê o cabeçalho do CSV (ignora a primeira linha)
    char linha[1024];
    fgets(linha, sizeof(linha), arquivoCSV);

    while (fgets(linha, sizeof(linha), arquivoCSV)) {
        inicializaRegistro(&registro);
        char *fields[MAX_FIELDS];
        int numFields;
        parseCSVLine(linha, fields, &numFields);

        if (numFields < 10) {
            // Trata linhas com campos insuficientes
            for (int i = 0; i < numFields; i++) {
                free(fields[i]);
            }
            continue;
        }

        // Atribui os campos ao registro
        registro.populacao = atoi(fields[0]);
        registro.tamanho = atof(fields[1]);
        registro.unidadeMedida = fields[2][0];
        registro.velocidade = atoi(fields[3]);

        registro.nome = strdup(fields[4]);
        registro.especie = strdup(fields[5]);
        registro.habitat = strdup(fields[6]);
        registro.tipo = strdup(fields[7]);
        registro.dieta = strdup(fields[8]);
        registro.alimento = strdup(fields[9]);

        // Libera os campos temporários
        for (int i = 0; i < numFields; i++) {
            free(fields[i]);
        }

        // Escreve campos de tamanho fixo
        fwrite(&registro.removido, sizeof(char), 1, arquivoBin);
        fwrite(&registro.encadeamento, sizeof(int), 1, arquivoBin);
        fwrite(&registro.populacao, sizeof(int), 1, arquivoBin);
        fwrite(&registro.tamanho, sizeof(float), 1, arquivoBin);
        fwrite(&registro.unidadeMedida, sizeof(char), 1, arquivoBin);
        fwrite(&registro.velocidade, sizeof(int), 1, arquivoBin);

        // Escreve campos de tamanho variável
        escreveStringVariavel(arquivoBin, registro.nome);
        escreveStringVariavel(arquivoBin, registro.especie);
        escreveStringVariavel(arquivoBin, registro.habitat);
        escreveStringVariavel(arquivoBin, registro.tipo);
        escreveStringVariavel(arquivoBin, registro.dieta);
        escreveStringVariavel(arquivoBin, registro.alimento);

        // Libera memória das strings alocadas
        free(registro.nome);
        free(registro.especie);
        free(registro.habitat);
        free(registro.tipo);
        free(registro.dieta);
        free(registro.alimento);

        rrnAtual++;
    }

    // Atualiza o cabeçalho
    cabecalho.status = '1';  // Marca como consistente
    cabecalho.proxRRN = rrnAtual;

    // Calcula o número de páginas de disco
    long tamanhoArquivo = ftell(arquivoBin);
    cabecalho.nroPagDisco = (tamanhoArquivo + TAM_PAG_DISCO - 1) / TAM_PAG_DISCO;

    // Reescreve o cabeçalho atualizado
    fseek(arquivoBin, 0, SEEK_SET);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);

    fclose(arquivoCSV);
    fclose(arquivoBin);
    printf("Arquivo binário criado com sucesso.\n");
}



//---------------------------------------------------------------------------------------------------------------------------

// Função para recuperar todos os registros de um arquivo binário (SELECT FROM)
void recuperaTodosRegistros(const char *nomeArquivoBin) {
    FILE *arquivoBin = fopen(nomeArquivoBin, "rb");
    if (!arquivoBin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;
    fread(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);  // Lê o cabeçalho

    if (cabecalho.status == '0') {
        printf("Arquivo inconsistente.\n");
        fclose(arquivoBin);
        return;
    }

    Registro registro;
    int rrn = 0;
    while (fread(&registro, sizeof(Registro), 1, arquivoBin)) {
        if (registro.removido == '0') {
            // Exibe o registro somente se não estiver logicamente removido
            printf("RRN %d: Nome: %s, Nome Científico: %s\n", rrn, registro.nome, registro.especie);
        }
        rrn++;
    }

    fclose(arquivoBin);
}

// Função para recuperar registros específicos de acordo com o critério de busca (SELECT WHERE)
void buscaRegistro(const char *nomeArquivoBin, const char *campo, const char *valor) {
    FILE *arquivoBin = fopen(nomeArquivoBin, "rb");
    if (!arquivoBin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;
    fread(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);

    if (cabecalho.status == '0') {
        printf("Arquivo inconsistente.\n");
        fclose(arquivoBin);
        return;
    }

    Registro registro;
    int encontrado = 0;
    while (fread(&registro, sizeof(Registro), 1, arquivoBin)) {
        if (registro.removido == '0') {
            if ((strcmp(campo, "nome") == 0 && strcmp(registro.nome, valor) == 0) ||
                (strcmp(campo, "especie") == 0 && strcmp(registro.especie, valor) == 0)) {
                // Exibe o registro se o critério for atendido
                printf("Nome: %s, Nome Científico: %s\n", registro.nome, registro.especie);
                encontrado = 1;
            }
        }
    }

    if (!encontrado) {
        printf("Registro não encontrado.\n");
    }

    fclose(arquivoBin);
}

// Função para remover logicamente um registro específico (DELETE)
void removeRegistroLogico(const char *nomeArquivoBin, int rrn) {
    FILE *arquivoBin = fopen(nomeArquivoBin, "rb+");
    if (!arquivoBin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;
    fread(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);  // Lê o cabeçalho

    if (cabecalho.status == '0') {
        printf("Arquivo inconsistente.\n");
        fclose(arquivoBin);
        return;
    }

    fseek(arquivoBin, TAM_CABECALHO + rrn * TAM_REGISTRO, SEEK_SET);
    Registro registro;
    fread(&registro, sizeof(Registro), 1, arquivoBin);

    if (registro.removido == '1') {
        printf("Registro já removido.\n");
        fclose(arquivoBin);
        return;
    }

    registro.removido = '1';  // Marca como removido
    registro.encadeamento = cabecalho.topo;  // Encadeia na lista de registros removidos
    cabecalho.topo = rrn;
    cabecalho.nroRegRem++;

    fseek(arquivoBin, -sizeof(Registro), SEEK_CUR);
    fwrite(&registro, sizeof(Registro), 1, arquivoBin);  // Regrava o registro atualizado

    fseek(arquivoBin, 0, SEEK_SET);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);  // Regrava o cabeçalho atualizado

    fclose(arquivoBin);
    printf("Registro removido logicamente.\n");
}

// Função para compactar o arquivo, removendo registros logicamente removidos (COMPACT)
void compactaArquivo(const char *nomeArquivoBin) {
    FILE *arquivoBin = fopen(nomeArquivoBin, "rb+");
    if (!arquivoBin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;
    fread(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);  // Lê o cabeçalho

    if (cabecalho.status == '0') {
        printf("Arquivo inconsistente.\n");
        fclose(arquivoBin);
        return;
    }

    int rrnAtual = 0;
    int rrnNovo = 0;
    Registro registro;

    while (fread(&registro, sizeof(Registro), 1, arquivoBin)) {
        if (registro.removido == '0') {  // Somente registros válidos
            if (rrnAtual != rrnNovo) {
                fseek(arquivoBin, TAM_CABECALHO + rrnNovo * TAM_REGISTRO, SEEK_SET);
                fwrite(&registro, sizeof(Registro), 1, arquivoBin);
            }
            rrnNovo++;
        }
        rrnAtual++;
    }

    cabecalho.proxRRN = rrnNovo;
    cabecalho.topo = RRN_NULO;
    cabecalho.nroRegRem = 0;
    cabecalho.qttCompacta++;

    fseek(arquivoBin, 0, SEEK_SET);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivoBin);  // Regrava o cabeçalho atualizado

    fclose(arquivoBin);
    printf("Arquivo compactado com sucesso.\n");
}
