// Fig. 7.20: fig07_20.c
// Menggunakan notasi indeks dan pointer dengan array.
#include <stdio.h>
#define ARRAY_SIZE 4

int main(void) {
    int b[] = {10, 20, 30, 40}; // Inisialisasi array b
    int *bPtr = b;               // bPtr menunjuk ke elemen pertama array b (&b[0])

    // 1. Menampilkan array b menggunakan notasi indeks array
    puts("Array b printed with:\nArray index notation");
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        printf("b[%u] = %d\n", (unsigned int)i, b[i]);
    }

    // 2. Menampilkan array b menggunakan nama array dan notasi pointer/offset
    puts("\nPointer/offset notation where\nthe pointer is the array name");
    for (size_t offset = 0; offset < ARRAY_SIZE; ++offset) {
        printf("*(b + %u) = %d\n", (unsigned int)offset, *(b + offset));
    }

    // 3. Menampilkan array b menggunakan bPtr dan notasi indeks pointer
    puts("\nPointer index notation");
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        printf("bPtr[%u] = %d\n", (unsigned int)i, bPtr[i]);
    }

    // 4. Menampilkan array b menggunakan bPtr dan notasi pointer/offset
    puts("\nPointer/offset notation");
    for (size_t offset = 0; offset < ARRAY_SIZE; ++offset) {
        printf("*(bPtr + %u) = %d\n", (unsigned int)offset, *(bPtr + offset));
    }

    return 0;
}