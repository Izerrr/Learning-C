#include <stdio.h>

typedef struct {
    char nama[50];
    float ipk;
} Mahasiswa;

// Fungsi Bubble Sort Descending (IPK Tertinggi -> Terendah)
void urutkanIpk(Mahasiswa daftar[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int ada_tukar = 0; // Optimasi: berhenti lebih awal jika sudah urut

        for (int j = 0; j < n - 1 - i; j++) {
            if (daftar[j].ipk < daftar[j + 1].ipk) {
                // Tukar satu paket struct utuh (nama & ipk ikut berpindah)
                Mahasiswa temp = daftar[j];
                daftar[j] = daftar[j + 1];
                daftar[j + 1] = temp;
                ada_tukar = 1;
            }
        }

        if (!ada_tukar) break;
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

    // Tampilkan data sebelum diurutkan (untuk pembuktian di demo)
    printf("Data Awal Mahasiswa:\n");
    for (int i = 0; i < jumlah; i++) {
        printf("- %-6s (%.2f)\n", daftar[i].nama, daftar[i].ipk);
    }
    printf("\n");

    // Proses pengurutan
    urutkanIpk(daftar, jumlah);

    // Output akhir sesuai ketentuan soal
    printf("Peringkat IPK:\n");
    for (int i = 0; i < jumlah; i++) {
        printf("%d. %-6s %.2f\n", i + 1, daftar[i].nama, daftar[i].ipk);
    }

    return 0;
}