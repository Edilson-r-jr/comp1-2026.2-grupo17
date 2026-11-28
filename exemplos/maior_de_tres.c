#include <stdio.h>

int a = 12;
int b = 7;
int c = 19;
int maior = a;

if (b > maior) {
    maior = b;
}

if (c > maior) {
    maior = c;
}

printf("Maior valor = %d\n", maior);
