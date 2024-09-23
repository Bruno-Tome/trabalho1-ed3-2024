#include <ctype.h>
#include <string.h>
#include "funcoesFornecidas.h"
#include "cabecalho.h"
#include "registro.h"

#define DELIMITER '#'
#define TRASH '$'
#define PAGE_SIZE 1600

// Função principal para a funcionalidade 1
void funcionalidade1() {
    Cabecalho header = inicializarCabecalho();

    char srcFileName[30], destinyFileName[30];
    scanf("%s %s", srcFileName, destinyFileName);

    FILE *sf = fopen(srcFileName, "r");
    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    FILE *df = fopen(destinyFileName, "wb+");
    escreverCabecalhoBin(df, &header);

    fscanf(sf, "%*[^\n]\n"); // Ignorar a primeira linha de cabeçalho do CSV

    Registro *s = lerRegistroCSV(sf);
    int contPags = 1;
    int nBytes = 0;
    while (s != NULL) {
        escreverRegistro(df, s);
        liberarRegistro(s);  // Libera o registro após escrever no arquivo
        s = lerRegistroCSV(sf);
        header.proxRRN++;
        nBytes+= 160;

        if ((nBytes % 1600) == 0){
            contPags++;
        }
    }   

    header.nroPagDisco = contPags +1;
    fseek(df, 0, SEEK_SET);
    header.status = '1';
    escreverCabecalhoBin(df, &header);

    fclose(sf);
    fclose(df);

    // Exibir o arquivo binário
    binarioNaTela(destinyFileName);
}

void funcionalidade2() {
    // Inicializa o cabeçalho do arquivo binário
    Cabecalho header = inicializarCabecalho();
    
    char srcFileName[30]; // Nome do arquivo de origem
    scanf("%s", srcFileName);
    
    FILE *sf = fopen(srcFileName, "rb"); // Abre o arquivo binário para leitura

    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return; // Erro ao abrir o arquivo
    }

    // Lê o cabeçalho do arquivo binário
    lerCabecalhoBin(sf, &header);

    int flag = 0; // Usado para verificar se algum registro foi encontrado
    Registro *r = lerRegistroBin(sf); // Lê o primeiro registro do arquivo

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
        liberarRegistro(r);

        // Lê o próximo registro do arquivo
        r = lerRegistroBin(sf);
    }

    // Se nenhum registro foi encontrado
    if (!flag) {
        printf("Registro inexistente.");
    }

    // Número de páginas de disco ocupadas
    printf("Numero de paginas de disco: %d\n\n", header.nroPagDisco);

    fclose(sf); // Fecha o arquivo
}

// Função principal da funcionalidade 3
void funcionalidade3() {
    Cabecalho header;
    char srcFileName[30];
    int n;
    
    scanf("%s", srcFileName); // Nome do arquivo binário
    FILE *sf = fopen(srcFileName, "rb"); // Abre o arquivo binário
    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    lerCabecalhoBin(sf, &header); // Lê o cabeçalho do arquivo
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
        Registro *r = lerRegistroBin(sf); // Lê o primeiro registro

        if (i==0){
            printf("Busca %d\n", i + 1); // Exibe o número da busca
        }else{
            printf("\nBusca %d\n", i + 1); // Exibe o número da busca
        }
        

        // Itera sobre os registros do arquivo
        while (r != NULL) {
            // Verifica se o registro não foi removido e atende ao critério da busca
            if (r->removido == '0' && buscarRegistro(r, campo, valor)) {
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
            liberarRegistro(r); // Libera o registro atual
            r = lerRegistroBin(sf); // Lê o próximo registro
        }

        // Caso nenhum registro tenha sido encontrado
        if (registrosEncontrados == 0) {
            printf("Registro inexistente.\n\n");
        }

        printf("Numero de paginas de disco: %d\n", header.nroPagDisco); // Mostra o número de páginas de disco
    }
    printf("\n");
    fclose(sf); // Fecha o arquivo
}


// Função principal da funcionalidade 4
void funcionalidade4() {
    Cabecalho header = inicializarCabecalho();
    char srcFileName[30];
    scanf("%s", srcFileName);

    FILE *sf = fopen(srcFileName, "rb+");
    if (sf == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    lerCabecalhoBin(sf, &header);  // Lê o cabeçalho

    int n;
    scanf("%d", &n);  // Número de tentativas de remoção

    for (int i = 0; i < n; i++) {
        char campo[30], valor[30];
        // Lê o campo a ser buscado
        scanf("%s", campo);

        // Verifica se o campo deve ser uma string (dentro de aspas)
        if (strcmp(campo, "nome") == 0 || strcmp(campo, "especie") == 0 || strcmp(campo, "habitat") == 0 ||
            strcmp(campo, "dieta") == 0 || strcmp(campo, "tipo") == 0 || strcmp(campo, "alimento") == 0) {
            scan_quote_string(valor); // Lê strings entre aspas
        } else {
            scanf("%s", valor); // Lê o valor (pode ser numérico ou string sem aspas)
        }

        fseek(sf, PAGE_SIZE, SEEK_SET);  // Reposiciona para após o cabeçalho

        Registro *r = lerRegistroBin(sf);  // Lê o primeiro registro
        long int byteOffset = ftell(sf) - sizeof(Registro);  // Calcula o offset do registro

        while (r != NULL) {
            if (r->removido == '0' && buscarRegistro(r, campo, valor)) {
                removeRegistro(r, &header.topo, byteOffset);  // Marca como removido
                fseek(sf, byteOffset, SEEK_SET);  // Volta para o início do registro
                fwrite(r, sizeof(Registro), 1, sf);  // Sobrescreve o registro removido
            }

            byteOffset = ftell(sf);  // Atualiza o byteOffset
            r = lerRegistroBin(sf);  // Lê o próximo registro
        }
    }

    // Atualiza o cabeçalho e grava no início do arquivo
    fseek(sf, 0, SEEK_SET);
    header.status = '1';  // Marca o arquivo como consistente
    fwrite(&header, sizeof(Cabecalho), 1, sf);

    fclose(sf);
    binarioNaTela(srcFileName);  // Exibe o arquivo compactado
}
void funcionalidade5() {
    Cabecalho header = inicializarCabecalho();
    char fileName[30];
    scanf("%s", fileName);

    FILE *df = fopen(fileName, "rb+");
    if (df == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    lerCabecalhoBin(df, &header);

    int numInsertions;
    scanf("%d", &numInsertions);

    Registro *r;

    for (int i = 0; i < numInsertions; i++) {
        r = (Registro *)malloc(sizeof(Registro));
        r->removido = '0'; // Define como não removido
        r->encadeamento = -1; // Define o encadeamento como -1, pois o registro não está removido

        // Leitura dos campos do registro
        scanf("%d", &r->populacao); // Leitura da população da espécie

        scanf("%f %c", &r->tamanho, &r->unidadeMedida); // Leitura do tamanho e da unidade de medida

        scanf("%d", &r->velocidade); // Leitura da velocidade do indivíduo

        r->nome = (char *)malloc(50 * sizeof(char));
        scan_quote_string(r->nome); // Leitura do nome da espécie

        r->especie = (char *)malloc(50 * sizeof(char));
        scan_quote_string(r->especie); // Leitura do nome científico da espécie

        r->habitat = (char *)malloc(50 * sizeof(char));
        scan_quote_string(r->habitat); // Leitura do habitat da espécie

        r->tipo = (char *)malloc(50 * sizeof(char));
        scan_quote_string(r->tipo); // Leitura do tipo da espécie

        r->dieta = (char *)malloc(50 * sizeof(char));
        scan_quote_string(r->dieta); // Leitura da dieta da espécie

        r->alimento = (char *)malloc(50 * sizeof(char));
        scan_quote_string(r->alimento); // Leitura do alimento da espécie

        // Inserção do registro no arquivo binário
            inserirRegistro(df, r, &header);


        // Libera a memória alocada para os campos de string
        free(r->nome);
        free(r->especie);
        free(r->habitat);
        free(r->tipo);
        free(r->dieta);
        free(r->alimento);
        free(r);
    }

    // Atualiza o cabeçalho do arquivo
    fseek(df, 0, SEEK_SET);
    header.status = '1'; // Marca o arquivo como consistente
    escreverCabecalhoBin(df, &header);

    fclose(df);
    binarioNaTela(fileName);
}

// Função principal da funcionalidade 6
void funcionalidade6() {
    
    char nomeArquivoOriginal[30];

    // Nome do arquivo original
    scanf("%s", nomeArquivoOriginal);
    
    char *nomeArquivoCompactado = "compactado.bin";  // Nome fixo para o arquivo compactado
    
    FILE *arquivoOriginal = fopen(nomeArquivoOriginal, "rb");
    if (arquivoOriginal == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arquivoCompactado = fopen(nomeArquivoCompactado, "wb");
    if (arquivoCompactado == NULL) {
        printf("Falha ao criar o arquivo compactado.\n");
        fclose(arquivoOriginal);
        return;
    }

    // Compacta o arquivo binário, removendo registros marcados como removidos
    compactarArquivoBinario(arquivoOriginal, arquivoCompactado);

    // Fechar arquivos
    fclose(arquivoOriginal);
    fclose(arquivoCompactado);

    // Exibe o arquivo compactado
    binarioNaTela(nomeArquivoCompactado);
}