#include <stdio.h>
#include <stdlib.h>

int main() {
    int *arr = malloc(5 * sizeof(int));

    // Periksa apakah alokasi berhasil
    if (arr == NULL) {
        printf("Alokasi memori gagal.\n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        arr[i] = (i + 1) * 10;
    }

    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Bebaskan memori setelah selesai
    free(arr);

    return 0;
}