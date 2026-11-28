#include <stdio.h>

int n = 5;
int fatorial = 1;

while (n > 1) {
    fatorial = fatorial * n;
    n = n - 1;
}

printf("5! = %d\n", fatorial);
