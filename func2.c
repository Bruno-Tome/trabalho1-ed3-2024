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
        freeRegistro(newRegistro);  // Libera a memória em caso de falha
        return NULL;
    }

    return newRegistro;
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

// Função auxiliar para ler strings delimitadas e tratar o lixo
void readString(FILE *fp, char *dest) {
    char ch;
    int i = 0;
    while (1) {
        fread(&ch, sizeof(char), 1, fp);
        if (ch == DELIMITER) {  // Encontrou o delimitador '#', fim da string
            dest[i] = '\0';
            break;
        }
        if (ch == TRASH) {  // Pulando o lixo '$'
            continue;
        }
        dest[i++] = ch;
    }
}

// Função para ler um registro binário do arquivo e tratar o lixo
Registro *readRegistroBin(FILE *fp) {
    if (fp == NULL || feof(fp)) return NULL; // erro ou fim de arquivo

    Registro *r = createRegistro(); // Cria o novo registro com valores padrões
    if (r == NULL) return NULL;  // Falha na criação

    // Salva a posição inicial do registro
    long int initialPos = ftell(fp);

    // Lê o campo "removido" (primeiro byte do registro)
    if (fread(&r->removido, sizeof(char), 1, fp) != 1) {
        freeRegistro(r);
        return NULL;  // Chegou ao final do arquivo ou erro na leitura
    }

    // Se o registro foi logicamente removido (r->removido == '1')
    if (r->removido == '1') {
        // Pula os bytes restantes do registro (160 bytes no total)
        fseek(fp, initialPos + 160, SEEK_SET);
        freeRegistro(r);  // Libera o registro
        return readRegistroBin(fp);  // Tenta ler o próximo registro
    }

    // Lê os campos de tamanho fixo
    fread(&r->encadeamento, sizeof(int), 1, fp);
    fread(&r->populacao, sizeof(int), 1, fp);
    fread(&r->tamanho, sizeof(float), 1, fp);
    fread(&r->unidadeMedida, sizeof(char), 1, fp);
    fread(&r->velocidade, sizeof(int), 1, fp);

    // Lê as strings variáveis e trata o lixo
    readString(fp, r->nome);
    readString(fp, r->especie);
    readString(fp, r->habitat);
    readString(fp, r->tipo);
    readString(fp, r->dieta);
    readString(fp, r->alimento);

    // Após ler o registro, pular o lixo que sobrou até completar 160 bytes
    long int currentPos = ftell(fp);  // Verifica a posição atual
    long int endOfRecord = initialPos + 160;  // Calcula onde o próximo registro começa
    fseek(fp, endOfRecord, SEEK_SET);  // Pula o lixo

    return r;
}

// Função para ler o cabeçalho de um arquivo binário e tratar o lixo
void readBinFileHeader(FILE *fp, Cabecalho *header) {
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

void functionality2() {
    // Inicializa o cabeçalho do arquivo binário
    Cabecalho header = initializeFileHeader();
    
    char srcFileName[30]; // Nome do arquivo de origem
    scanf("%s", srcFileName);
    
    FILE *sf = fopen(srcFileName, "rb"); // Abre o arquivo binário para leitura

    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return; // Erro ao abrir o arquivo
    }

    // Lê o cabeçalho do arquivo binário
    readBinFileHeader(sf, &header);

    int flag = 0; // Usado para verificar se algum registro foi encontrado
    Registro *r = readRegistroBin(sf); // Lê o primeiro registro do arquivo

    // Enquanto houver registros no arquivo binário
    while (r != NULL) {
        if (r->removido == '0') { // Se o registro não foi removido logicamente
            flag = 1; // Define flag para indicar que pelo menos um registro foi lido
            // Imprime o registro (adaptado para ignorar campos nulos)
            if (r->nome[0] != '\0') {
                printf("Nome: %s\n", r->nome);
            }
            if (r->especie[0] != '\0') {
                printf("Especie: %s\n", r->especie);
            }
            if (r->tipo[0] != '\0') {
                printf("Tipo: %s\n", r->tipo);
            }
            if (r->dieta[0] != '\0') {
                printf("Dieta: %s\n", r->dieta);
            }
            if (r->habitat[0] != '\0') {
                printf("Lugar que habitava: %s\n", r->habitat);
            }
            if (r->tamanho != -1) {
                printf("Tamanho: %.1f m\n", r->tamanho);
            }
            if (r->velocidade != -1) {
                printf("Velocidade: %d %cm/h\n", r->velocidade, r->unidadeMedida);
            }
            printf("\n");
        }
        // Libera o registro lido
        freeRegistro(r);

        // Lê o próximo registro do arquivo
        r = readRegistroBin(sf);
    }

    // Se nenhum registro foi encontrado
    if (!flag) {
        printf("Registro inexistente.");
    }

    // Número de páginas de disco ocupadas
    printf("Numero de paginas de disco: %d\n", header.nroPagDisco);

    fclose(sf); // Fecha o arquivo
}

int main() {
    int opt;
    scanf("%d", &opt);
    functionality2();
    return 0;
}
