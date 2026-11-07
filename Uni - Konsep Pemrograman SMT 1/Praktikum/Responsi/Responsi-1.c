#include <stdio.h>

int main() {
    // 1. DEKLARASI KONSTANTA, VARIABEL, DAN ARRAY 1D
    // Inisialisasi batas jumlah data
    const int jumlah_mahasiswa = 5;
    // Array 1 Dimensi: Menyimpan 5 data nilai sekuensial di memori
    int nilai[5] = {45, 78, 85, 52, 90};

    // Variabel penampung akumulasi dan counter statistik
    int total_nilai = 0;
    int jumlah_lulus = 0;
    int jumlah_remidi = 0;

    // Template Header Tampilan Laporan
    printf("====================================================\n");
    printf("   SISTEM EVALUASI & REKAPITULASI NILAI MAHASISWA   \n");
    printf("====================================================\n\n");

    // 2. PERULANGAN FOR & PENGAKSESAN ARRAY
    printf("--- 1. DAFTAR EVALUASI INDIVIDUAL MAHASISWA ---\n");
    for (int i = 0; i < jumlah_mahasiswa; i++) {
        // Akumulasi nilai elemen saat ini ke total
        total_nilai += nilai[i];
        printf("Mahasiswa ke-%d [Indeks %d] | Nilai: %d -> Status: ", i + 1, i, nilai[i]);
        // 3. PERCABANGAN BERTINGKAT (if - else if - else)
        if (nilai[i] >= 85) {
            printf("LULUS (Grade A - Sangat Memuaskan)\n");
            jumlah_lulus++;
            // Evaluasi predikat memuaskan (60 - 84)
        } else if (nilai[i] >= 60) {
            printf("LULUS (Grade B/C - Memuaskan)\n");
            jumlah_lulus++;
            // Evaluasi remidi bersyarat (50 - 59)
        } else if (nilai[i] >= 50) {
            printf("REMIDI BERSYARAT (Grade D)\n");
            jumlah_remidi++;
            // Evaluasi remidi mutlak (di bawah 50)
        } else {
            printf("REMIDI MUTLAK (Grade E)\n");
            jumlah_remidi++;
        }
    }

    // Perhitungan rata-rata dengan typecasting float agar tidak terjadi integer division
    // Konversi eksplisit (float) mencegah truncation desimal
    float rata_rata = (float)total_nilai / jumlah_mahasiswa;

    // 4. PERULANGAN WHILE: Algoritma Pencarian Nilai Tertinggi
    int j = 0;
    int nilai_tertinggi = nilai[0];
    int indeks_tertinggi = 0;

    while (j < jumlah_mahasiswa) {
        if (nilai[j] > nilai_tertinggi) {
            nilai_tertinggi = nilai[j];
            indeks_tertinggi = j;
        }
        j++; // Modifikasi nilai iterator (mencegah infinite loop)
    }

    // 5. OUTPUT REKAPITULASI
    printf("\n====================================================\n");
    printf("                  RINGKASAN KELAS                   \n");
    printf("====================================================\n");
    printf("Total Nilai Keseluruhan : %d\n", total_nilai);
    printf("Rata-Rata Nilai Kelas   : %.2f\n", rata_rata);
    printf("Jumlah Mahasiswa Lulus  : %d orang\n", jumlah_lulus);
    printf("Jumlah Mahasiswa Remidi : %d orang\n", jumlah_remidi);
    printf("Nilai Tertinggi         : %d (Mahasiswa ke-%d / Indeks [%d])\n",
           nilai_tertinggi, indeks_tertinggi + 1, indeks_tertinggi);
    printf("====================================================\n");

    return 0;
}