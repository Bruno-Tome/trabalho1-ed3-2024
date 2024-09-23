#include <ctype.h>
#include <string.h>
#include "funcoesFornecidas.h"
#include "registro.h"
#include "funcoesUteis.h"
#include "cabecalho.h"

#define DELIMITER '#'
#define TRASH '$'
#define PAGE_SIZE 1600

// Função para liberar a memória de um registro
void liberarRegistro(Registro *reg) {
    if (reg == NULL) return;

    // Libera cada string se não for NULL
    if (reg->nome) free(reg->nome);
    if (reg->especie) free(reg->especie);
    if (reg->habitat) free(reg->habitat);
    if (reg->tipo) free(reg->tipo);
    if (reg->dieta) free(reg->dieta);
    if (reg->alimento) free(reg->alimento);

    // Libera a estrutura principal
    free(reg);
}

// Função para criar um novo registro com valores padrão
Registro *criarRegistro() {
    Registro *newRegistro = (Registro *) malloc(sizeof(Registro));
    if (newRegistro == NULL) return NULL;

    newRegistro->removido = '0';
    newRegistro->encadeamento = -1;
    newRegistro->populacao = -1;
    newRegistro->tamanho = -1;
    newRegistro->unidadeMedida = '$';
    newRegistro->velocidade = -1;

    // Alocando memória para os campos variáveis
    newRegistro->nome = (char *) malloc(sizeof(char) * 51);
    newRegistro->especie = (char *) malloc(sizeof(char) * 51);
    newRegistro->habitat = (char *) malloc(sizeof(char) * 51);
    newRegistro->tipo = (char *) malloc(sizeof(char) * 51);
    newRegistro->dieta = (char *) malloc(sizeof(char) * 51);
    newRegistro->alimento = (char *) malloc(sizeof(char) * 51);

    // Verifica se todas as alocações deram certo
    if (!newRegistro->nome || !newRegistro->especie || !newRegistro->habitat ||
        !newRegistro->tipo || !newRegistro->dieta || !newRegistro->alimento) {
        liberarRegistro(newRegistro);  // Libera a memória em caso de falha
        return NULL;
    }

    return newRegistro;
}

// Função para ler o CSV e criar registros
Registro *lerRegistroCSV(FILE *fp) {
    if (fp == NULL || feof(fp)) return NULL;

    Registro *r = criarRegistro();

    char fileLine[256];
    fscanf(fp, "%[^\n]\n", fileLine);

    int i = 0, k = 0;
    char numberString[20];

    // Leitura do campo "nome" (não pode ser nulo, garantido pelo CSV)
    for (k = 0; fileLine[i] != ','; i++, k++) r->nome[k] = fileLine[i];
    r->nome[k] = '\0'; i++;

    // Leitura do campo "dieta" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->dieta[0] = '#'; // Representar campo nulo com '#'
        r->dieta[1] = '\0';
        i++;
    } else {
        for (k = 0; fileLine[i] != ','; i++, k++) r->dieta[k] = fileLine[i];
        r->dieta[k] = '\0';
        i++;
    }

    // Leitura do campo "habitat" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->habitat[0] = '#'; // Representar campo nulo com '#'
        r->habitat[1] = '\0';
        i++;
    } else {
        for (k = 0; fileLine[i] != ','; i++, k++) r->habitat[k] = fileLine[i];
        r->habitat[k] = '\0';
        i++;
    }

    // Leitura do campo "populacao" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->populacao = -1; // Representar valor nulo com -1
        i++;
    } else {
        for (k = 0; fileLine[i] != ','; i++, k++) numberString[k] = fileLine[i];
        numberString[k] = '\0'; 
        r->populacao = atoi(numberString); 
        i++;
    }

    // Leitura do campo "tipo" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->tipo[0] = '#'; // Representar campo nulo com '#'
        r->tipo[1] = '\0';
        i++;
    } else {
        for (k = 0; fileLine[i] != ','; i++, k++) r->tipo[k] = fileLine[i];
        r->tipo[k] = '\0';
        i++;
    }

    // Leitura do campo "velocidade" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->velocidade = -1; // Representar valor nulo com -1
        i++;
    } else {
        for (k = 0; fileLine[i] != ','; i++, k++) numberString[k] = fileLine[i];
        numberString[k] = '\0'; 
        r->velocidade = atoi(numberString);
        i++;
    }

    // Leitura do campo "unidadeMedida" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->unidadeMedida = '$'; // Representar valor nulo com '$'
        i++;
    } else {
        r->unidadeMedida = fileLine[i]; // Unidade de medida é um caractere
        i += 2; // Avançar a vírgula e o caractere
    }

    // Leitura do campo "tamanho" (pode ser nulo)
    if (fileLine[i] == ',') {
        r->tamanho = -1; // Representar valor nulo com -1
        i++;
    } else {
        for (k = 0; fileLine[i] != ','; i++, k++) numberString[k] = fileLine[i];
        numberString[k] = '\0'; 
        r->tamanho = atof(numberString); 
        i++;
    }

    // Leitura do campo "especie" (não pode ser nulo, garantido pelo CSV)
    for (k = 0; fileLine[i] != ','; i++, k++) r->especie[k] = fileLine[i];
    r->especie[k] = '\0'; 
    i++;

    // Leitura do campo "alimento" (pode ser nulo)
    if (fileLine[i] == '\n' || fileLine[i] == '\r' || fileLine[i] == '\0') {
        r->alimento[0] = '#'; // Representar campo nulo com '#'
        r->alimento[1] = '\0';
    } else {
        for (k = 0; fileLine[i] != '\n' && fileLine[i] != '\r' && fileLine[i] != '\0'; i++, k++) r->alimento[k] = fileLine[i];
        r->alimento[k] = '\0';
    }

    return r;
}

// Função para escrever um registro no arquivo binário
void escreverRegistro(FILE *fp, Registro *reg) {
    if(fp == NULL || reg == NULL) return;

    char divider = DELIMITER;
    char trash = TRASH;
    int tamanhoUsado = 0;

    // Campo 'removido'
    fwrite(&reg->removido, sizeof(char), 1, fp);
    tamanhoUsado += sizeof(char);

    // Campo 'encadeamento'
    fwrite(&reg->encadeamento, sizeof(int), 1, fp);
    tamanhoUsado += sizeof(int);

    // Campo 'populacao'
    if (reg->populacao == -1) {
        int valorNulo = -1;
        fwrite(&valorNulo, sizeof(int), 1, fp);
    } else {
        fwrite(&reg->populacao, sizeof(int), 1, fp);
    }
    tamanhoUsado += sizeof(int);

    // Campo 'tamanho'
    if (reg->tamanho == -1) {
        float valorNulo = -1;
        fwrite(&valorNulo, sizeof(float), 1, fp);
    } else {
        fwrite(&reg->tamanho, sizeof(float), 1, fp);
    }
    tamanhoUsado += sizeof(float);

    // Campo 'unidadeMedida'
    if (reg->unidadeMedida == '$') {
        fwrite(&trash, sizeof(char), 1, fp);
    } else {
        fwrite(&reg->unidadeMedida, sizeof(char), 1, fp);
    }
    tamanhoUsado += sizeof(char);

    // Campo 'velocidade'
    if (reg->velocidade == -1) {
        int valorNulo = -1;
        fwrite(&valorNulo, sizeof(int), 1, fp);
    } else {
        fwrite(&reg->velocidade, sizeof(int), 1, fp);
    }
    tamanhoUsado += sizeof(int);

    // Campo 'nome' (não pode ser nulo, garantido pelo CSV)
    for (int i = 0; reg->nome[i] != '\0'; i++) {
        fwrite(&reg->nome[i], sizeof(char), 1, fp);
        tamanhoUsado += sizeof(char);
    }
    fwrite(&divider, sizeof(char), 1, fp);
    tamanhoUsado += sizeof(char);

    // Campo 'especie' (não pode ser nulo, garantido pelo CSV)
    for (int i = 0; reg->especie[i] != '\0'; i++) {
        fwrite(&reg->especie[i], sizeof(char), 1, fp);
        tamanhoUsado += sizeof(char);
    }
    fwrite(&divider, sizeof(char), 1, fp);
    tamanhoUsado += sizeof(char);

    // Campo 'habitat'
    if (reg->habitat[0] == '#') {
        fwrite(&divider, sizeof(char), 1, fp);  // Escreve somente o delimitador '#'
        tamanhoUsado += sizeof(char);
    } else {
        for (int i = 0; reg->habitat[i] != '\0'; i++) {
            fwrite(&reg->habitat[i], sizeof(char), 1, fp);
            tamanhoUsado += sizeof(char);
        }
        fwrite(&divider, sizeof(char), 1, fp);
        tamanhoUsado += sizeof(char);
    }

    // Campo 'tipo'
    if (reg->tipo[0] == '#') {
        fwrite(&divider, sizeof(char), 1, fp);  // Escreve somente o delimitador '#'
        tamanhoUsado += sizeof(char);
    } else {
        for (int i = 0; reg->tipo[i] != '\0'; i++) {
            fwrite(&reg->tipo[i], sizeof(char), 1, fp);
            tamanhoUsado += sizeof(char);
        }
        fwrite(&divider, sizeof(char), 1, fp);
        tamanhoUsado += sizeof(char);
    }

    // Campo 'dieta' (não pode ser nulo, garantido pelo CSV)
    for (int i = 0; reg->dieta[i] != '\0'; i++) {
        fwrite(&reg->dieta[i], sizeof(char), 1, fp);
        tamanhoUsado += sizeof(char);
    }
    fwrite(&divider, sizeof(char), 1, fp);
    tamanhoUsado += sizeof(char);

    // Campo 'alimento'
    if (reg->alimento[0] == '#') {
        fwrite(&divider, sizeof(char), 1, fp);  // Escreve somente o delimitador '#'
        tamanhoUsado += sizeof(char);
    } else {
        for (int i = 0; reg->alimento[i] != '\0'; i++) {
            fwrite(&reg->alimento[i], sizeof(char), 1, fp);
            tamanhoUsado += sizeof(char);
        }
        fwrite(&divider, sizeof(char), 1, fp);
        tamanhoUsado += sizeof(char);
    }

    // Preenchendo com '$' até atingir 160 bytes
    int espacoRestante = 160 - tamanhoUsado;
    for (int i = 0; i < espacoRestante; i++) {
        fwrite(&trash, sizeof(char), 1, fp);
    }
}

// Função para ler um registro binário do arquivo e tratar o lixo
Registro *lerRegistroBin(FILE *fp) {
    if (fp == NULL || feof(fp)) return NULL; // erro ou fim de arquivo

    Registro *r = criarRegistro(); // Cria o novo registro com valores padrões
    if (r == NULL) return NULL;  // Falha na criação

    // Salva a posição inicial do registro
    long int initialPos = ftell(fp);

    // Lê o campo "removido" (primeiro byte do registro)
    if (fread(&r->removido, sizeof(char), 1, fp) != 1) {
        liberarRegistro(r);
        return NULL;  // Chegou ao final do arquivo ou erro na leitura
    }

    // Se o registro foi logicamente removido (r->removido == '1')
    if (r->removido == '1') {
        // Pula os bytes restantes do registro (160 bytes no total)
        fseek(fp, initialPos + 160, SEEK_SET);
        liberarRegistro(r);  // Libera o registro
        return lerRegistroBin(fp);  // Tenta ler o próximo registro
    }

    // Lê os campos de tamanho fixo
    fread(&r->encadeamento, sizeof(int), 1, fp);
    fread(&r->populacao, sizeof(int), 1, fp);
    fread(&r->tamanho, sizeof(float), 1, fp);
    fread(&r->unidadeMedida, sizeof(char), 1, fp);
    fread(&r->velocidade, sizeof(int), 1, fp);

    // Lê as strings variáveis e trata o lixo
    lerString(fp, r->nome);
    lerString(fp, r->especie);
    lerString(fp, r->habitat);
    lerString(fp, r->tipo);
    lerString(fp, r->dieta);
    lerString(fp, r->alimento);

    // Após ler o registro, pular o lixo que sobrou até completar 160 bytes
    long int currentPos = ftell(fp);  // Verifica a posição atual
    long int endOfRecord = initialPos + 160;  // Calcula onde o próximo registro começa
    fseek(fp, endOfRecord, SEEK_SET);  // Pula o lixo

    return r;
}

// Função para buscar registros
int buscarRegistro(Registro *r, const char *campo, const char *valor) {
    if (strcmp(campo, "nome") == 0 && strcmp(r->nome, valor) == 0) return 1;
    if (strcmp(campo, "dieta") == 0 && strcmp(r->dieta, valor) == 0) return 1;
    if (strcmp(campo, "habitat") == 0 && strcmp(r->habitat, valor) == 0) return 1;
    
    // Correção para populacao: comparar como número
    if (strcmp(campo, "populacao") == 0 && atoi(valor) == r->populacao) return 1;

    if (strcmp(campo, "tipo") == 0 && strcmp(r->tipo, valor) == 0) return 1;
    
    if (strcmp(campo, "velocidade") == 0 && atoi(valor) == r->velocidade) return 1;

    // Correção para unidadeMedida: comparar como caractere
    if (strcmp(campo, "unidadeMedida") == 0 && valor[0] == r->unidadeMedida) return 1;
    
    if (strcmp(campo, "tamanho") == 0 && atof(valor) == r->tamanho) return 1;
    
    if (strcmp(campo, "especie") == 0 && strcmp(r->especie, valor) == 0) return 1;

    // Correção: comparar alimento com r->alimento, não com dieta
    if (strcmp(campo, "alimento") == 0 && strcmp(r->alimento, valor) == 0) return 1;

    return 0;
}

// Função para marcar um registro como logicamente removido
void removeRegistro(Registro *r, int *topo, long int byteOffset) {
    r->removido = '1';  // Marca como logicamente removido
    r->encadeamento = *topo;  // Define o próximo encadeado como o topo anterior
    *topo = byteOffset;  // Atualiza o topo da lista de removidos
}
 
 void inserirRegistro(FILE *fp, Registro *reg, Cabecalho *header) {
    if (fp == NULL || reg == NULL) return; // erro

    int tamanhoUsado = 160; // Tamanho fixo do registro em bytes

    if (header->topo != -1) { 
        // Existe um espaço na lista de registros removidos
        Registro *aux = (Registro *)malloc(sizeof(Registro));

        // Posiciona o ponteiro no início do registro removido
        long int currentRecordBO = header->topo;
        long int previousRecordBO = header->topo;

        // Lê o cabeçalho do registro removido
        fseek(fp, currentRecordBO, SEEK_SET);
        fread(&aux->removido, sizeof(char), 1, fp);
        fread(&aux->encadeamento, sizeof(int), 1, fp);

        if (tamanhoUsado <= 160) { // Verifica se cabe no espaço removido
            fseek(fp, currentRecordBO, SEEK_SET); // Posiciona para sobrescrever

            // Atualiza o cabeçalho da lista de removidos
            header->topo = aux->encadeamento;

            // Escreve o novo registro no espaço do removido
            escreverRegistro(fp, reg);
            free(aux);
            return;
        }

        // Percorre a lista de registros removidos até encontrar um espaço
        while (aux->encadeamento != -1) {
            previousRecordBO = currentRecordBO;
            currentRecordBO = aux->encadeamento;

            fseek(fp, currentRecordBO, SEEK_SET);
            fread(&aux->removido, sizeof(char), 1, fp);
            fread(&aux->encadeamento, sizeof(int), 1, fp);

            if (tamanhoUsado <= 160) {
                fseek(fp, currentRecordBO, SEEK_SET);

                // Atualiza o encadeamento do registro anterior para pular o atual
                fseek(fp, previousRecordBO + sizeof(char) + sizeof(int), SEEK_SET);
                fwrite(&aux->encadeamento, sizeof(int), 1, fp);

                // Escreve o registro no espaço disponível
                escreverRegistro(fp, reg);
                free(aux);
                return;
            }
        }
    }

    // Se não houver espaço na lista de removidos ou nenhum espaço foi suficiente
    fseek(fp, 0, SEEK_END); // Posiciona o ponteiro no final do arquivo
    escreverRegistro(fp, reg); // Escreve o novo registro no final
}


void compactarArquivoBinario(FILE *arquivoOriginal, FILE *arquivoCompactado) {
    Cabecalho header;

    // Lê o cabeçalho do arquivo original
    fread(&header, sizeof(Cabecalho), 1, arquivoOriginal);

    // Escreve o cabeçalho no arquivo compactado
    fwrite(&header, sizeof(Cabecalho), 1, arquivoCompactado);

    Registro *r;

    // Ler e copiar todos os registros não removidos
    while ((r = lerRegistroBin(arquivoOriginal)) != NULL) {
        if (r->removido == '0') {
            // Copia o registro não removido para o novo arquivo
            fwrite(r, sizeof(Registro), 1, arquivoCompactado);
        }
        // Libera o registro corretamente após ser usado
        liberarRegistro(r);
    }

    // Atualiza o cabeçalho do novo arquivo com as informações de registros copiados
    header.nroRegRem = 0;  // Após a compactação, não há mais registros removidos
    header.nroPagDisco = (ftell(arquivoCompactado) + PAGE_SIZE - 1) / PAGE_SIZE;  // Atualiza as páginas de disco
    fseek(arquivoCompactado, 0, SEEK_SET);  // Volta ao início do arquivo para reescrever o cabeçalho
    fwrite(&header, sizeof(Cabecalho), 1, arquivoCompactado);
}
