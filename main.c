#include <stdio.h>
#include <stdlib.h>
#include "funcionalidades.h"


int main() {
    int opt;
    scanf("%d", &opt);

    switch (opt) {
        case 1:
            funcionalidade1();
            break;

        case 2:
            funcionalidade2();
            break;

        case 3:
            funcionalidade3();
            break;
        case 4:
            funcionalidade4();
            break;
        case 5:
                      funcionalidade5(); 
            break;
        case 6:
            funcionalidade6();
            break;    
        default:
            break;
    }
}