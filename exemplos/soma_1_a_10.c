#include <stdio.h>

int i = 1;
int soma = 0;

while (i <= 10) {
    soma = soma + i;
    i = i + 1;
}

printf("Soma de 1 a 10 = %d\n", soma);
