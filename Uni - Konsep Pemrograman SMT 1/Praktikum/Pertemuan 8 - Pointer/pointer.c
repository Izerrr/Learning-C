#include <stdio.h>

int main() {
    int syam = 25;

    int *basmara = &syam;

    printf("nilai syam dari basmara %d\n", *basmara);

    int **reyhan = &basmara;
    int ***imam = &reyhan;
    printf("nilai imam dari reyhan %d\n", ***imam);

    **reyhan = 30;

    printf("nilai syam %d\n", syam);
    printf("alamat syam %p\n", &syam);
    printf("nilai basmara %p\n", basmara);
    printf("alamat basmara %p\n", &basmara);
    printf("nilai reyhan %p\n", reyhan);
    printf("alamat reyhan %p\n", &reyhan);
    printf("nilai imam %p\n", imam);
    printf("alamat imam %p\n", &imam);

    return 0;
}