#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

    if (reg->nome) free(reg->nome);
    if (reg->especie) free(reg->especie);
    if (reg->habitat) free(reg->habitat);
    if (reg->tipo) free(reg->tipo);
    if (reg->dieta) free(reg->dieta);
    if (reg->alimento) free(reg->alimento);

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

    if (!newRegistro->nome || !newRegistro->especie || !newRegistro->habitat ||
        !newRegistro->tipo || !newRegistro->dieta || !newRegistro->alimento) {
        freeRegistro(newRegistro);
        return NULL;
    }

    return newRegistro;
}

// Função fornecida para ler strings entre aspas
void scan_quote_string(char *str) {
    char R;

    
    while ((R = getchar()) != EOF && isspace(R)); // Ignorar espaços
    
    if (R == 'N' || R == 'n') {
        
        getchar(); getchar(); getchar(); // Ignorar o "ULO" de NULO.
        strcpy(str, ""); // Campo nulo
        
    } else if (R == '\"') {
        
        if (scanf("%[^\"]", str) != 1) {
            strcpy(str, "");
        }
        getchar(); // Ignorar aspas fechando
    } else if (R != EOF) {
        
        str[0] = R;
        scanf("%s", &str[1]);
    } else {
        strcpy(str, "");
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

// Função auxiliar para ler strings delimitadas e tratar o lixo
void readString(FILE *fp, char *dest) {
    char ch;
    int i = 0;
    while (1) {
        fread(&ch, sizeof(char), 1, fp);
        if (ch == DELIMITER) {
            dest[i] = '\0';
            break;
        }
        if (ch == TRASH) {
            continue;
        }
        dest[i++] = ch;
    }
}

// Função para ler um registro binário do arquivo e tratar o lixo
Registro *readRegistroBin(FILE *fp) {
    if (fp == NULL || feof(fp)) return NULL;

    Registro *r = createRegistro();
    if (r == NULL) return NULL;

    long int initialPos = ftell(fp);

    if (fread(&r->removido, sizeof(char), 1, fp) != 1) {
        freeRegistro(r);
        return NULL;
    }

    if (r->removido == '1') {
        fseek(fp, initialPos + 160, SEEK_SET);
        freeRegistro(r);
        return readRegistroBin(fp);
    }

    fread(&r->encadeamento, sizeof(int), 1, fp);
    fread(&r->populacao, sizeof(int), 1, fp);
    fread(&r->tamanho, sizeof(float), 1, fp);
    fread(&r->unidadeMedida, sizeof(char), 1, fp);
    fread(&r->velocidade, sizeof(int), 1, fp);

    readString(fp, r->nome);
    readString(fp, r->especie);
    readString(fp, r->habitat);
    readString(fp, r->tipo);
    readString(fp, r->dieta);
    readString(fp, r->alimento);

    long int currentPos = ftell(fp);
    long int endOfRecord = initialPos + 160;
    fseek(fp, endOfRecord, SEEK_SET);

    return r;
}

// Função para ler o cabeçalho de um arquivo binário
void readBinFileHeader(FILE *fp, Cabecalho *header) {
    if (fp == NULL || header == NULL) return;

    fread(&header->status, sizeof(char), 1, fp);
    fread(&header->topo, sizeof(int), 1, fp);
    fread(&header->proxRRN, sizeof(int), 1, fp);
    fread(&header->nroRegRem, sizeof(int), 1, fp);
    fread(&header->nroPagDisco, sizeof(int), 1, fp);
    fread(&header->qttCompacta, sizeof(int), 1, fp);

    long int currentPos = ftell(fp);
    long int endOfHeaderPage = PAGE_SIZE;
    fseek(fp, endOfHeaderPage - currentPos, SEEK_CUR);
}

// Função para buscar registros
int buscaRegistro(Registro *r, const char *campo, const char *valor) {
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


// Função principal da funcionalidade 3
void functionality3() {
    Cabecalho header;
    char srcFileName[30];
    int n;
    
    scanf("%s", srcFileName); // Nome do arquivo binário
    FILE *sf = fopen(srcFileName, "rb"); // Abre o arquivo binário
    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    readBinFileHeader(sf, &header); // Lê o cabeçalho do arquivo
    scanf("%d", &n); // Lê o número de tentativas de busca

    char campo[30], valor[30];

    // Loop para executar 'n' buscas
    for (int i = 0; i < n; i++) {
        // Lê o campo a ser buscado
        scanf("%s", campo);

        // Verifica se o campo deve ser uma string (dentro de aspas)
        if (strcmp(campo, "nome") == 0 || strcmp(campo, "especie") == 0 || strcmp(campo, "habitat") == 0 ||
            strcmp(campo, "dieta") == 0 || strcmp(campo, "tipo") == 0 || strcmp(campo, "alimento") == 0) {
            scan_quote_string(valor); // Lê strings entre aspas
        } else {
            scanf("%s", valor); // Lê o valor (pode ser numérico ou string sem aspas)
        }

        // Reposiciona o ponteiro no arquivo para começar após o cabeçalho
        fseek(sf, PAGE_SIZE, SEEK_SET);

        int registrosEncontrados = 0; // Inicializa o contador de registros encontrados
        Registro *r = readRegistroBin(sf); // Lê o primeiro registro

        if (i==0){
            printf("Busca %d\n", i + 1); // Exibe o número da busca
        }else{
            printf("\nBusca %d\n", i + 1); // Exibe o número da busca
        }
        

        // Itera sobre os registros do arquivo
        while (r != NULL) {
            // Verifica se o registro não foi removido e atende ao critério da busca
            if (r->removido == '0' && buscaRegistro(r, campo, valor)) {
                // Exibe os campos do registro encontrado
                if (r->nome[0] != '\0') printf("Nome: %s\n", r->nome);
                if (r->especie[0] != '\0') printf("Especie: %s\n", r->especie);
                if (r->tipo[0] != '\0') printf("Tipo: %s\n", r->tipo);
                if (r->dieta[0] != '\0') printf("Dieta: %s\n", r->dieta);
                if (r->habitat[0] != '\0') printf("Lugar que habitava: %s\n", r->habitat);
                if (r->tamanho != -1) printf("Tamanho: %.1f m\n", r->tamanho);
                if (r->velocidade != -1) printf("Velocidade: %d %cm/h\n", r->velocidade, r->unidadeMedida);
                printf("\n");
                registrosEncontrados++;
            }
            free(r); // Libera o registro atual
            r = readRegistroBin(sf); // Lê o próximo registro
        }

        // Caso nenhum registro tenha sido encontrado
        if (registrosEncontrados == 0) {
            printf("Registro inexistente.\n\n");
        }

        printf("Numero de paginas de disco: %d\n", header.nroPagDisco); // Mostra o número de páginas de disco
    }

    fclose(sf); // Fecha o arquivo
}

int main() {
    int opt;
    scanf("%d", &opt);
    if (opt == 3) {
        functionality3();
    }
    return 0;
}