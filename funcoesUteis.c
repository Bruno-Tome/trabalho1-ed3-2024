
#include <ctype.h>
#include <string.h>
#include "funcoesFornecidas.h"
#include "cabecalho.h"

#define DELIMITER '#'
#define TRASH '$'

// Função auxiliar para ler strings delimitadas e tratar o lixo
void lerString(FILE *fp, char *dest) {
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