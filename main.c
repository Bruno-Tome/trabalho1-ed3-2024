#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> 
// Definição de estruturas de dados
typedef struct {
    char status;            // Consistência do arquivo ('0' para inconsistente, '1' para consistente)
    int topo;               // RRN do primeiro registro logicamente removido
    int proxRRN;            // Próximo RRN disponível
    int nroRegRem;          // Número de registros removidos logicamente
    int nroPagDisco;        // Número de páginas de disco ocupadas
    int qttCompacta;        // Quantidade de vezes que o arquivo foi compactado
    char lixo[1590];        // Preenchimento para completar uma página de 1600 bytes
} Cabecalho;

typedef struct {
    char removido;          // Indica se o registro foi removido ('1' para removido, '0' para não removido)
    int encadeamento;       // RRN do próximo registro logicamente removido
    int populacao;          // Quantidade de indivíduos
    float tamanho;          // Tamanho do indivíduo
    char unidadeMedida[1];  // Unidade de medida da velocidade
    int velocidade;         // Velocidade do indivíduo
    char nome[30];          // Nome da espécie
    char especie[30];       // Nome da espécie específica
    char habitat[30];       // Habitat da espécie
    char tipo[30];          // Tipo da espécie
    char dieta[30];         // Dieta da espécie
    char alimento[30];      // Alimento da espécie
} Registro;


void binarioNaTela(char *nomeArquivoBinario) { /* Você não precisa entender o código dessa função. */

	/* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
	*  Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela). */

	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
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

void scan_quote_string(char *str) {

	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/

	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}
void lerArquivoCSV(char *arquivoEntrada, char *arquivoSaida) {
    FILE *csvFile = fopen(arquivoEntrada, "r");
    FILE *binFile = fopen(arquivoSaida, "wb");

    if (csvFile == NULL || binFile == NULL) {
        printf("Falha ao abrir os arquivos.\n");
        return;
    }

    // Escrever o registro de cabeçalho no arquivo binário
    Cabecalho cabecalho;
    cabecalho.status = '0'; // Inicia como inconsistente
    cabecalho.topo = -1;    // Nenhum registro logicamente removido
    cabecalho.proxRRN = 0;  // Próximo RRN disponível
    cabecalho.nroRegRem = 0; // Nenhum registro removido logicamente
    cabecalho.nroPagDisco = 0; // Inicialmente, 0 páginas de disco ocupadas
    cabecalho.qttCompacta = 0;  // Nenhuma compactação realizada
    memset(cabecalho.lixo, '$', 1590); // Preencher com lixo

    fwrite(&cabecalho, sizeof(Cabecalho), 1, binFile);

    char linha[256]; // Buffer para leitura de linha do CSV

    // Ignorar a primeira linha de cabeçalho do CSV
    fgets(linha, sizeof(linha), csvFile);

    while (fgets(linha, sizeof(linha), csvFile)) {
        char buffer[160];  // Buffer para o registro de tamanho fixo de 160 bytes
        memset(buffer, '$', sizeof(buffer));  // Preencher com lixo

        // Inicializar campos de string e garantir que sejam delimitados corretamente
        Registro registro;
        memset(&registro, 0, sizeof(Registro)); // Zerar o registro para evitar lixo
        registro.removido = '0';  // Registro não removido
        registro.encadeamento = -1; // Sem encadeamento

        // Variáveis para leitura dos campos
        char nome[30], dieta[30], habitat[30], tipo[30], unidadeMedida, especie[30], alimento[30];
        int populacao, velocidade;
        float tamanho;

        // Ler os campos da linha CSV na ordem correta
        sscanf(linha, "%29[^,],%29[^,],%29[^,],%d,%29[^,],%d,%c,%f,%29[^,],%29[^\n]", 
            nome, dieta, habitat, &populacao, tipo, &velocidade, &unidadeMedida, &tamanho, especie, alimento);

        // Copiar dados para o registro, garantindo que os arrays de tamanho fixo sejam preenchidos corretamente
        strncpy(registro.nome, nome, sizeof(registro.nome) - 1);
        strncpy(registro.dieta, dieta, sizeof(registro.dieta) - 1);
        strncpy(registro.habitat, habitat, sizeof(registro.habitat) - 1);
        strncpy(registro.tipo, tipo, sizeof(registro.tipo) - 1);
        strncpy(registro.especie, especie, sizeof(registro.especie) - 1);
        strncpy(registro.alimento, alimento, sizeof(registro.alimento) - 1);
        registro.populacao = populacao;
        registro.velocidade = velocidade;
        registro.unidadeMedida[0] = unidadeMedida;
        registro.tamanho = tamanho;

        // Preencher o buffer com os campos de tamanho fixo
        int offset = 0;
        memcpy(buffer + offset, &registro.removido, sizeof(char));
        offset += sizeof(char);
        memcpy(buffer + offset, &registro.encadeamento, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, &registro.populacao, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, &registro.tamanho, sizeof(float));
        offset += sizeof(float);
        memcpy(buffer + offset, &registro.unidadeMedida, sizeof(char));
        offset += sizeof(char);
        memcpy(buffer + offset, &registro.velocidade, sizeof(int));
        offset += sizeof(int);

        // Preencher o buffer com os campos de tamanho variável seguidos do delimitador '#'
        int len;

        len = strlen(registro.nome);
        memcpy(buffer + offset, registro.nome, len);
        offset += len;
        buffer[offset++] = '#';

        len = strlen(registro.especie);
        memcpy(buffer + offset, registro.especie, len);
        offset += len;
        buffer[offset++] = '#';

        len = strlen(registro.habitat);
        memcpy(buffer + offset, registro.habitat, len);
        offset += len;
        buffer[offset++] = '#';

        len = strlen(registro.tipo);
        memcpy(buffer + offset, registro.tipo, len);
        offset += len;
        buffer[offset++] = '#';

        len = strlen(registro.dieta);
        memcpy(buffer + offset, registro.dieta, len);
        offset += len;
        buffer[offset++] = '#';

        len = strlen(registro.alimento);
        memcpy(buffer + offset, registro.alimento, len);
        offset += len;
        buffer[offset++] = '#';

        // Gravar o buffer de 160 bytes no arquivo binário
        fwrite(buffer, sizeof(char), 160, binFile);

        cabecalho.proxRRN++; // Incrementa o próximo RRN disponível
    }

    // Atualizar o status do cabeçalho para consistente
    fseek(binFile, 0, SEEK_SET);
    cabecalho.status = '1';
    fwrite(&cabecalho, sizeof(Cabecalho), 1, binFile);

    fclose(csvFile);
    fclose(binFile);
    binarioNaTela(arquivoSaida);
}




// Função auxiliar para ler strings de tamanho variável até o delimitador '#'
void lerStringComDelimitador(FILE *file, char *str, int maxLen) {
    char ch;
    int i = 0;
    while (i < maxLen - 1 && fread(&ch, sizeof(char), 1, file) && ch != '#') {
        str[i++] = ch;
    }
    str[i] = '\0';  // Terminar a string
}
void exibirRegistros(char *arquivoEntrada) {
    FILE *binFile = fopen(arquivoEntrada, "rb");

    if (binFile == NULL) {
        printf("Falha ao abrir o arquivo.\n");
        return;
    }

    // Pular o registro de cabeçalho
    fseek(binFile, 1600, SEEK_SET);

    char removido;
    int encadeamento;
    int populacao;
    float tamanho;
    char unidadeMedida;
    int velocidade;
    char nome[30], especie[30], habitat[30], tipo[30], dieta[30];

    int registrosLidos = 0;

    // Ler cada registro de 160 bytes
    while (fread(&removido, sizeof(char), 1, binFile)) {
        fread(&encadeamento, sizeof(int), 1, binFile);
        fread(&populacao, sizeof(int), 1, binFile);
        fread(&tamanho, sizeof(float), 1, binFile);
        fread(&unidadeMedida, sizeof(char), 1, binFile);
        fread(&velocidade, sizeof(int), 1, binFile);

        // Ler campos de tamanho variável até o delimitador '#'
        lerStringComDelimitador(binFile, nome, 30);
        lerStringComDelimitador(binFile, especie, 30);
        lerStringComDelimitador(binFile, habitat, 30);
        lerStringComDelimitador(binFile, tipo, 30);
        lerStringComDelimitador(binFile, dieta, 30);

        if (removido == '0') { // Apenas exibir registros não removidos
            if (strlen(nome) > 0) {
                printf("Nome: %s\n", nome);
            }
            if (strlen(especie) > 0) {
                printf("Especie: %s\n", especie);
            }
            if (strlen(tipo) > 0) {
                printf("Tipo: %s\n", tipo);
            }
            if (strlen(dieta) > 0) {
                printf("Dieta: %s\n", dieta);
            }
            if (strlen(habitat) > 0) {
                printf("Lugar que habitava: %s\n", habitat);
            }
            if (tamanho > 0) {
                printf("Tamanho: %.1f m\n", tamanho);
            }

            // Concatenar a unidade de medida com "m/h"
            if (unidadeMedida == 'k') {
                printf("Velocidade: %d km/h\n", velocidade);
            } else if (unidadeMedida == 'h') {
                printf("Velocidade: %d hm/h\n", velocidade);
            } else if (unidadeMedida == 'c') {
                printf("Velocidade: %d cm/h\n", velocidade);
            }
            printf("\n");
            registrosLidos++;
        }

        // Pular para o próximo registro (160 bytes ao todo)
        fseek(binFile, 160 - (sizeof(char) + sizeof(int) * 2 + sizeof(float) + sizeof(char) + sizeof(int) +
                             strlen(nome) + 1 + strlen(especie) + 1 + strlen(habitat) + 1 +
                             strlen(tipo) + 1 + strlen(dieta) + 1), SEEK_CUR);
    }

    if (registrosLidos == 0) {
        printf("Registro inexistente.\n");
    }

    fclose(binFile);
}
// Função para comparar strings de forma segura considerando o tamanho máximo
int stringIgual(char *str1, char *str2, size_t maxLen) {
    return strncmp(str1, str2, maxLen) == 0;
}

// Função para buscar e exibir registros que atendem ao critério de busca
void buscarRegistros(char *arquivoEntrada, char *nomeCampo, char *valorCampo) {
    FILE *binFile = fopen(arquivoEntrada, "rb");

    if (binFile == NULL) {
        printf("Falha ao abrir o arquivo.\n");
        return;
    }

    // Pular o cabeçalho
    fseek(binFile, 1600, SEEK_SET);

    char removido;
    int encadeamento;
    int populacao;
    float tamanho;
    char unidadeMedida;
    int velocidade;
    char nome[30], especie[30], habitat[30], tipo[30], dieta[30];

    int registrosEncontrados = 0;

    // Ler cada registro de 160 bytes
    while (fread(&removido, sizeof(char), 1, binFile)) {
        fread(&encadeamento, sizeof(int), 1, binFile);
        fread(&populacao, sizeof(int), 1, binFile);
        fread(&tamanho, sizeof(float), 1, binFile);
        fread(&unidadeMedida, sizeof(char), 1, binFile);
        fread(&velocidade, sizeof(int), 1, binFile);

        // Ler campos de tamanho variável até o delimitador '#'
        lerStringComDelimitador(binFile, nome, 30);
        lerStringComDelimitador(binFile, especie, 30);
        lerStringComDelimitador(binFile, habitat, 30);
        lerStringComDelimitador(binFile, tipo, 30);
        lerStringComDelimitador(binFile, dieta, 30);

        if (removido == '0') { // Considerar apenas registros não removidos
            int match = 0; // Flag para correspondência de campo

            // Comparar o campo de busca com o valor especificado
            if (strcmp(nomeCampo, "nome") == 0) {
                match = strcmp(nome, valorCampo) == 0;
            } else if (strcmp(nomeCampo, "especie") == 0) {
                match = strcmp(especie, valorCampo) == 0;
            } else if (strcmp(nomeCampo, "habitat") == 0) {
                match = strcmp(habitat, valorCampo) == 0;
            } else if (strcmp(nomeCampo, "tipo") == 0) {
                match = strcmp(tipo, valorCampo) == 0;
            } else if (strcmp(nomeCampo, "dieta") == 0) {
                match = strcmp(dieta, valorCampo) == 0;
            } else if (strcmp(nomeCampo, "populacao") == 0) {
                match = atoi(valorCampo) == populacao;
            } else if (strcmp(nomeCampo, "tamanho") == 0) {
                match = atof(valorCampo) == tamanho;
            } else if (strcmp(nomeCampo, "velocidade") == 0) {
                match = atoi(valorCampo) == velocidade;
            } else {
                printf("Campo de busca inválido.\n");
                fclose(binFile);
                return;
            }

            if (match) {
                if (strlen(nome) > 0) printf("Nome: %s\n", nome);
                if (strlen(especie) > 0) printf("Especie: %s\n", especie);
                if (strlen(tipo) > 0) printf("Tipo: %s\n", tipo);
                if (strlen(dieta) > 0) printf("Dieta: %s\n", dieta);
                if (strlen(habitat) > 0) printf("Lugar que habitava: %s\n", habitat);
                if (tamanho > 0) printf("Tamanho: %.1f m\n", tamanho);

                // Concatenar a unidade de medida com "m/h"
                if (unidadeMedida == 'k') {
                    printf("Velocidade: %d km/h\n", velocidade);
                } else if (unidadeMedida == 'h') {
                    printf("Velocidade: %d hm/h\n", velocidade);
                } else if (unidadeMedida == 'c') {
                    printf("Velocidade: %d cm/h\n", velocidade);
                }
                printf("\n");
                registrosEncontrados++;
            }
        }

        // Pular para o próximo registro (160 bytes ao todo)
        fseek(binFile, 160 - (sizeof(char) + sizeof(int) * 2 + sizeof(float) + sizeof(char) + sizeof(int) +
                             strlen(nome) + 1 + strlen(especie) + 1 + strlen(habitat) + 1 +
                             strlen(tipo) + 1 + strlen(dieta) + 1), SEEK_CUR);
    }

    if (registrosEncontrados == 0) {
        printf("Registro inexistente.\n");
    }

    fclose(binFile);
}


// Função para remover logicamente registros com base em um critério de busca
void removerRegistros(char *arquivoEntrada, char *nomeCampo, char *valorCampo) {
    FILE *binFile = fopen(arquivoEntrada, "rb+"); // Abrir para leitura e escrita

    if (binFile == NULL) {
        printf("Falha ao abrir o arquivo.\n");
        return;
    }

    // Ler o cabeçalho
    char status;
    int topo;
    int proxRRN;
    int nroRegRem;
    int nroPagDisco;
    int qttCompacta;

    fread(&status, sizeof(char), 1, binFile);
    fread(&topo, sizeof(int), 1, binFile);
    fread(&proxRRN, sizeof(int), 1, binFile);
    fread(&nroRegRem, sizeof(int), 1, binFile);
    fread(&nroPagDisco, sizeof(int), 1, binFile);
    fread(&qttCompacta, sizeof(int), 1, binFile);

    // Atualizar o status para inconsistente durante a modificação
    fseek(binFile, 0, SEEK_SET);
    status = '0';
    fwrite(&status, sizeof(char), 1, binFile);
    fseek(binFile, 1600, SEEK_SET); // Voltar para o início dos registros

    char removido;
    int encadeamento;
    int populacao;
    float tamanho;
    char unidadeMedida;
    int velocidade;
    char nome[30], especie[30], habitat[30], tipo[30], dieta[30], alimento[30];

    int registrosRemovidos = 0;

    // Ler cada registro do arquivo
    while (fread(&removido, sizeof(char), 1, binFile)) {
        int posicaoRegistro = ftell(binFile) - sizeof(char); // Marca o início do registro
        fread(&encadeamento, sizeof(int), 1, binFile);
        fread(&populacao, sizeof(int), 1, binFile);
        fread(&tamanho, sizeof(float), 1, binFile);
        fread(&unidadeMedida, sizeof(char), 1, binFile);
        fread(&velocidade, sizeof(int), 1, binFile);
        fread(nome, sizeof(char), 30, binFile);
        fread(especie, sizeof(char), 30, binFile);
        fread(habitat, sizeof(char), 30, binFile);
        fread(tipo, sizeof(char), 30, binFile);
        fread(dieta, sizeof(char), 30, binFile);
        fread(alimento, sizeof(char), 30, binFile);

        if (removido == '0') { // Considerar apenas registros não removidos
            int match = 0;

            // Comparar o campo de busca com o valor especificado
            if (strcmp(nomeCampo, "nome") == 0) {
                match = stringIgual(nome, valorCampo, 30);
            } else if (strcmp(nomeCampo, "especie") == 0) {
                match = stringIgual(especie, valorCampo, 30);
            } else if (strcmp(nomeCampo, "habitat") == 0) {
                match = stringIgual(habitat, valorCampo, 30);
            } else if (strcmp(nomeCampo, "tipo") == 0) {
                match = stringIgual(tipo, valorCampo, 30);
            } else if (strcmp(nomeCampo, "dieta") == 0) {
                match = stringIgual(dieta, valorCampo, 30);
            } else if (strcmp(nomeCampo, "alimento") == 0) {
                match = stringIgual(alimento, valorCampo, 30);
            } else if (strcmp(nomeCampo, "populacao") == 0) {
                match = atoi(valorCampo) == populacao;
            } else if (strcmp(nomeCampo, "tamanho") == 0) {
                match = atof(valorCampo) == tamanho;
            } else if (strcmp(nomeCampo, "velocidade") == 0) {
                match = atoi(valorCampo) == velocidade;
            } else {
                printf("Campo de busca inválido.\n");
                fclose(binFile);
                return;
            }

            if (match) {
                // Marcar o registro como removido e atualizar o encadeamento
                fseek(binFile, posicaoRegistro, SEEK_SET);
                removido = '1';
                fwrite(&removido, sizeof(char), 1, binFile);
                fwrite(&topo, sizeof(int), 1, binFile); // Novo topo aponta para o antigo
                topo = (posicaoRegistro - 1600) / sizeof(char) / 160; // Calcular o RRN do registro
                nroRegRem++; // Incrementar o número de registros removidos
                registrosRemovidos++;
            }
        }
        fseek(binFile, posicaoRegistro + 160, SEEK_SET); // Ir para o próximo registro
    }

    if (registrosRemovidos == 0) {
        printf("Registro inexistente.\n");
    } else {
        // Atualizar o cabeçalho com o novo topo e número de registros removidos
        fseek(binFile, sizeof(char), SEEK_SET);
        fwrite(&topo, sizeof(int), 1, binFile);
        fwrite(&proxRRN, sizeof(int), 1, binFile);
        fwrite(&nroRegRem, sizeof(int), 1, binFile);
        fwrite(&nroPagDisco, sizeof(int), 1, binFile);
        fwrite(&qttCompacta, sizeof(int), 1, binFile);
    }

    // Atualizar o status para consistente
    fseek(binFile, 0, SEEK_SET);
    status = '1';
    fwrite(&status, sizeof(char), 1, binFile);

    fclose(binFile);
}


void inserirRegistro(char *arquivoEntrada, Registro novoRegistro) {
    FILE *fp = fopen(arquivoEntrada, "rb+");
    if (!fp) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Lê o cabeçalho do arquivo
    Cabecalho cabecalho;
    fseek(fp, 0, SEEK_SET);
    fread(&cabecalho, sizeof(Cabecalho), 1, fp);

    // Verifica se existe um registro logicamente removido para reutilizar
    int rrnReutilizado = -1;
    if (cabecalho.topo != -1) {
        rrnReutilizado = cabecalho.topo;
        int offset = sizeof(Cabecalho) + (rrnReutilizado * sizeof(Registro));

        // Lê o registro logicamente removido
        Registro registroRemovido;
        fseek(fp, offset, SEEK_SET);
        fread(&registroRemovido, sizeof(Registro), 1, fp);

        // Atualiza o topo do cabeçalho para o próximo registro removido
        cabecalho.topo = registroRemovido.encadeamento;
        cabecalho.nroRegRem--;  // Decrementa o número de registros removidos

        // Volta o cabeçalho atualizado para o arquivo
        fseek(fp, 0, SEEK_SET);
        fwrite(&cabecalho, sizeof(Cabecalho), 1, fp);

        // Posiciona o cursor no local de inserção
        fseek(fp, offset, SEEK_SET);
    } else {
        // Se não houver registros logicamente removidos, insere no final do arquivo
        rrnReutilizado = cabecalho.proxRRN;
        int offset = sizeof(Cabecalho) + (rrnReutilizado * sizeof(Registro));
        fseek(fp, offset, SEEK_SET);

        // Atualiza o próximo RRN disponível
        cabecalho.proxRRN++;
        cabecalho.nroPagDisco = (cabecalho.proxRRN * sizeof(Registro)) / 1600 + 1; // Calcula o número de páginas de disco ocupadas

        // Volta o cabeçalho atualizado para o arquivo
        fseek(fp, 0, SEEK_SET);
        fwrite(&cabecalho, sizeof(Cabecalho), 1, fp);
    }

    // Prepara o novo registro
    novoRegistro.removido = '0';      // Marca como não removido
    novoRegistro.encadeamento = -1;   // Não há próximo registro removido

    // Escreve o novo registro no arquivo
    fwrite(&novoRegistro, sizeof(Registro), 1, fp);

    // Fecha o arquivo
    fclose(fp);
    printf("Registro inserido com sucesso.\n");
}


// Função para compactar o arquivo de dados binário
void compactarArquivo(char *arquivoEntrada) {
    FILE *arquivo = fopen(arquivoEntrada, "rb+");
    if (arquivo == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Leitura do cabeçalho
    Cabecalho cabecalho;
    fread(&cabecalho, sizeof(Cabecalho), 1, arquivo);

    // Verificação de consistência
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivo);
        return;
    }

    // Marcando o arquivo como inconsistente durante a operação
    cabecalho.status = '0';
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivo);

    // Leitura e compactação dos registros
    Registro registro;
    int rrnAtual = 0;
    int rrnProximo = 0;
    
    // Lê cada registro e compacta apenas os que não estão removidos
    while (fread(&registro, sizeof(Registro), 1, arquivo) == 1) {
        if (registro.removido == '0') {
            if (rrnAtual != rrnProximo) {
                // Mover registro válido para o local apropriado
                fseek(arquivo, sizeof(Cabecalho) + rrnProximo * sizeof(Registro), SEEK_SET);
                fwrite(&registro, sizeof(Registro), 1, arquivo);
            }
            rrnProximo++;  // Incrementa o RRN para o próximo registro válido
        }
        rrnAtual++;  // Incrementa o RRN para o próximo registro lido
    }

    // Atualizando o número de páginas de disco no cabeçalho
    cabecalho.nroPagDisco = (rrnProximo * sizeof(Registro) + sizeof(Cabecalho) - 1) / 1600 + 1;

    // Atualizando o cabeçalho após a compactação
    cabecalho.status = '1';  // Marcando o arquivo como consistente
    cabecalho.proxRRN = rrnProximo;  // Atualiza o próximo RRN disponível
    cabecalho.nroRegRem = 0;  // Reseta o número de registros removidos
    cabecalho.qttCompacta++;  // Incrementa a contagem de compactações

    // Escrevendo o cabeçalho atualizado de volta no arquivo
    fseek(arquivo, 0, SEEK_SET);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivo);

    // Truncando o arquivo para o novo tamanho (compactado)
    ftruncate(fileno(arquivo), sizeof(Cabecalho) + rrnProximo * sizeof(Registro));

    // Fechando o arquivo
    fclose(arquivo);

    // Exibindo o conteúdo do arquivo compactado usando a função binarioNaTela
    binarioNaTela(arquivoEntrada);
}
int main() {
    int funcionalidade;
    char arquivoEntrada[100], arquivoSaida[100];
    char nomeCampo[30], valorCampo[30]; // Declaração movida para fora do switch

    // Leitura da funcionalidade
    scanf("%d", &funcionalidade);

    switch (funcionalidade) {
        case 1:
            // Leitura dos arquivos de entrada e saída para a funcionalidade 1
            scanf("%s %s", arquivoEntrada, arquivoSaida);
            lerArquivoCSV(arquivoEntrada, arquivoSaida);
            break;
        case 2:
            // Leitura do arquivo de entrada para a funcionalidade 2
            scanf("%s", arquivoEntrada);
            exibirRegistros(arquivoEntrada);
            break;
        case 3:
            // Leitura do arquivo de entrada e dos parâmetros de busca para a funcionalidade 3
            scanf("%s %s %s", arquivoEntrada, nomeCampo, valorCampo);
            buscarRegistros(arquivoEntrada, nomeCampo, valorCampo);
            break;
        case 4:
            // Leitura do arquivo de entrada e dos parâmetros de remoção para a funcionalidade 4
            scanf("%s %s %s", arquivoEntrada, nomeCampo, valorCampo);
            removerRegistros(arquivoEntrada, nomeCampo, valorCampo);
            break;
        case 5:
            // Leitura do arquivo de entrada para inserção de registros na funcionalidade 5
            scanf("%s", arquivoEntrada);
            // Aqui, o código de leitura dos dados do registro deve ser adicionado
            break;
        case 6:
            // Leitura do arquivo de entrada para a funcionalidade 6
            scanf("%s", arquivoEntrada);
            compactarArquivo(arquivoEntrada);
            break;
        default:
            printf("Funcionalidade inválida.\n");
            break;
    }

    return 0;
}
