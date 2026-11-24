#include <stdio.h>

// Fungsi untuk menghitung panjang string sekaligus menghapus karakter Enter ('\n')
int bersihkanDanHitung(char teks[]) {
    int i = 0;
    while (teks[i] != '\0') {
        if (teks[i] == '\n') {
            teks[i] = '\0';
            break;
        }
        i++;
    }
    return i;
}

// Fungsi pengecekan palindrom dengan metode Two-Pointer
int cekPalindrom(const char teks[], int panjang) {
    int kiri = 0;
    int kanan = panjang - 1;

    while (kiri < kanan) {
        if (teks[kiri] != teks[kanan]) {
            return 0; // Langsung kembalikan 0 (bukan palindrom) jika ada yang beda
        }
        kiri++;
        kanan--;
    }
    return 1; // Semua pasangan ujung cocok -> palindrom
}

int main() {
    char kata[100];

    printf("=== DETEKTOR KATA PALINDROM ===\n");
    printf("Masukkan kata: ");
    fgets(kata, sizeof(kata), stdin);

    int panjang = bersihkanDanHitung(kata);

    if (cekPalindrom(kata, panjang)) {
        printf("Hasil: \"%s\" adalah PALINDROM (panjang: %d huruf).\n", kata, panjang);
    } else {
        printf("Hasil: \"%s\" BUKAN palindrom.\n", kata);
    }

    return 0;
}