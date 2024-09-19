#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIMITER '#'
#define TRASH '$'
#define PAGE_SIZE 1600

// Definição de estruturas

typedef struct {
    char status;            // '0' inconsistente, '1' consistente
    int topo;               // RRN do primeiro registro removido logicamente
    int proxRRN;            // Próximo RRN disponível para inserção
    int nroRegRem;          // Número de registros logicamente removidos
    int nroPagDisco;        // Número de páginas de disco ocupadas
    int qttCompacta;        // Quantidade de compactações
} Cabecalho;

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


// Função para exibir o binário na tela (usado para o run.codes)
void binarioNaTela(char *nomeArquivoBinario) {
    unsigned long i, cs;
    unsigned char *mb;
    size_t fl;
    FILE *fs;
    if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
        fprintf(stderr, "ERRO AO ESCREVER O BinARIO NA TELA: não foi possível abrir o arquivo.\n");
        return;
    }
    fseek(fs, 0, SEEK_END);
    fl = ftell(fs);
    fseek(fs, 0, SEEK_SET);
    mb = (unsigned char *) malloc(fl);
    fread(mb, 1, fl, fs);

    cs = 0;
    for(i = 0; i < fl; i++) {
        cs += (unsigned long) mb[i];
    }
    printf("%lf\n", (cs / (double) 100));
    free(mb);
    fclose(fs);
}

// Função para liberar a memória de um registro
void freeRegistro(Registro *reg) {
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
Registro *createRegistro() {
    Registro *newRegistro = (Registro *) malloc(sizeof(Registro));
    if(newRegistro == NULL) return NULL;

    newRegistro->removido = '0';
    newRegistro->encadeamento = -1;
    newRegistro->populacao = -1;
    newRegistro->tamanho = -1;
    newRegistro->unidadeMedida = '$';
    newRegistro->velocidade = -1;

    newRegistro->nome = (char *) malloc(sizeof(char) * 51);
    newRegistro->especie = (char *) malloc(sizeof(char) * 51);
    newRegistro->habitat = (char *) malloc(sizeof(char) * 51);
    newRegistro->tipo = (char *) malloc(sizeof(char) * 51);
    newRegistro->dieta = (char *) malloc(sizeof(char) * 51);
    newRegistro->alimento = (char *) malloc(sizeof(char) * 51);

    // Verifica se alguma das alocações falhou
    if (!newRegistro->nome || !newRegistro->especie || !newRegistro->habitat || !newRegistro->tipo || !newRegistro->dieta || !newRegistro->alimento) {
        freeRegistro(newRegistro);  // Libera a memória em caso de falha
        return NULL;
    }

    return newRegistro;
}

// Função para escrever um registro no arquivo binário
void writeRegistro(FILE *fp, Registro *reg) {
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

// Função para escrever o cabeçalho do arquivo binário
void writeBinFileHeader(FILE *fp, Cabecalho *header) {
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

// Função para inicializar o cabeçalho
Cabecalho initializeFileHeader() {
    Cabecalho header;
    header.status = '0';
    header.topo = -1;
    header.proxRRN = 0;
    header.nroRegRem = 0;
    header.nroPagDisco = 1;
    header.qttCompacta = 0;
    return header;
}

// Função para ler o CSV e criar registros
Registro *readRegistroCsv(FILE *fp) {
    if (fp == NULL || feof(fp)) return NULL;

    Registro *r = createRegistro();

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

// Função principal para a funcionalidade 1
void functionality1() {
    Cabecalho header = initializeFileHeader();

    char srcFileName[30], destinyFileName[30];
    scanf("%s %s", srcFileName, destinyFileName);

    FILE *sf = fopen(srcFileName, "r");
    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    FILE *df = fopen(destinyFileName, "wb+");
    writeBinFileHeader(df, &header);

    fscanf(sf, "%*[^\n]\n"); // Ignorar a primeira linha de cabeçalho do CSV

    Registro *s = readRegistroCsv(sf);
    int contPags = 1;
    int nBytes = 0;
    while (s != NULL) {
        writeRegistro(df, s);
        freeRegistro(s);  // Libera o registro após escrever no arquivo
        s = readRegistroCsv(sf);
        header.proxRRN++;
        nBytes+= 160;

        if ((nBytes % 1600) == 0){
            contPags++;
        }
    }   

    header.nroPagDisco = contPags +1;
    fseek(df, 0, SEEK_SET);
    header.status = '1';
    writeBinFileHeader(df, &header);

    fclose(sf);
    fclose(df);

    // Exibir o arquivo binário
    binarioNaTela(destinyFileName);
}



int main() {
    int opt;
    scanf("%d", &opt);

    switch (opt) {
        case 1:
            functionality1();
            break;
        default:
            break;
    }
}
