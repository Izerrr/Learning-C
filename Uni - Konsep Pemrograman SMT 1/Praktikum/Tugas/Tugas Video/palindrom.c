#include <stdio.h>

// Fungsi menghitung panjang string sekaligus menghapus karakter Enter ('\n')
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

// Fungsi mengecek palindrom dengan metode Two-Pointer
int cekPalindrom(const char teks[], int panjang) {
    int kiri = 0;
    int kanan = panjang - 1;

    while (kiri < kanan) {
        if (teks[kiri] != teks[kanan]) {
            return 0; // Langsung kembalikan 0 jika ada huruf yang tidak cocok
        }
        kiri++;
        kanan--;
    }
    return 1; // Kembalikan 1 jika seluruh pasangan ujung cocok
}

int main() {
    char kata[100];

    printf("=== DETEKTOR KATA PALINDROM ===\n");
    printf("Masukkan kata: ");
    fgets(kata, sizeof(kata), stdin);

    int panjang = bersihkanDanHitung(kata);
    // Cek apakah kata merupakan palindrom berdasarkan hasil fungsi cekPalindrom
    if (cekPalindrom(kata, panjang) == 1) {
        printf("Hasil: \"%s\" adalah palindrom (panjang: %d karakter).\n", kata, panjang);
    } else {
        printf("Hasil: \"%s\" bukan palindrom (panjang: %d karakter).\n", kata, panjang);
    }

    return 0;
}