#include <stdio.h>

// Definisi struct Mahasiswa dengan member nama dan ipk
typedef struct {
    char nama[50];
    float ipk;
} Mahasiswa;

// Fungsi Bubble Sort untuk mengurutkan IPK dari tertinggi ke terendah
void urutkanIpk(Mahasiswa daftar[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int ada_tukar = 0; // Penanda apakah terjadi pertukaran pada putaran ini

        for (int j = 0; j < n - 1 - i; j++) {
            // Bandingkan IPK saat ini dengan IPK berikutnya (Descending)
            if (daftar[j].ipk < daftar[j + 1].ipk) {
                // Salin seluruh struct menggunakan variabel sementara
                Mahasiswa temp = daftar[j];
                daftar[j] = daftar[j + 1];
                daftar[j + 1] = temp;
                ada_tukar = 1;
            }
        }

        // Jika tidak ada pertukaran sama sekali, array sudah terurut
        if (ada_tukar == 0) {
            break;
        }
    }
}

int main() {
    Mahasiswa daftar[5] = {
        {"Budi", 3.45},
        {"Siti", 3.90},
        {"Andi", 3.10},
        {"Dewi", 3.75},
        {"Rina", 3.60}
    };
    int jumlah = 5;

    // 1. Tampilkan data awal sebelum diurutkan (untuk perbandingan di demo)
    printf("Data Awal Mahasiswa:\n");
    for (int i = 0; i < jumlah; i++) {
        printf("- %-6s %.2f\n", daftar[i].nama, daftar[i].ipk);
    }
    printf("\n");

    // 2. Panggil fungsi pengurutan IPK
    urutkanIpk(daftar, jumlah);

    // 3. Tampilkan hasil akhir sesuai format soal
    printf("Peringkat IPK:\n");
    for (int i = 0; i < jumlah; i++) {
        printf("%d. %-6s %.2f\n", i + 1, daftar[i].nama, daftar[i].ipk);
    }

    return 0;
}