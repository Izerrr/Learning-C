#include <stdio.h>
#include <stdlib.h>

struct Transaksi {
    char namaBarang[50];
    long harga;
    int jumlah;
    long subtotal;
};

int main() {
    int n;
    long totalPemasukan = 0;

    printf("===========================================\n");
    printf("   GENERATOR LAPORAN PENJUALAN (.CSV)      \n");
    printf("===========================================\n");
    
    printf("Masukkan jumlah data transaksi (N): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Jumlah transaksi tidak valid.\n");
        return 1;
    }

    // 1. Buka/buat file dengan mode "w" (write)
    FILE *file = fopen("penjualan.csv", "w");
    if (file == NULL) {
        printf("Error: Gagal membuat atau membuka file 'penjualan.csv'.\n");
        return 1;
    }

    // 2. Tulis Header kolom CSV
    fprintf(file, "No,Nama Barang,Harga Satuan,Jumlah,Subtotal\n");

    // 3. Input data per transaksi dan langsung tulis ke file
    for (int i = 0; i < n; i++) {
        char nama[50];
        long harga;
        int jumlah;

        printf("\nTransaksi ke-%d:\n", i + 1);
        printf("Nama Barang  : ");
        // Membaca string berspasi dan membersihkan sisa newline buffer
        scanf(" %[^\n]", nama); 
        printf("Harga Satuan : Rp ");
        scanf("%ld", &harga);
        printf("Jumlah Beli  : ");
        scanf("%d", &jumlah);

        long subtotal = harga * (long)jumlah;
        totalPemasukan += subtotal;

        // Tulis baris data ke file CSV.
        // Tanda kutip ganda (\"%s\") menjaga nama barang tetap aman jika ada tanda koma.
        fprintf(file, "%d,\"%s\",%ld,%d,%ld\n", i + 1, nama, harga, jumlah, subtotal);
    }

    // 4. Tambahkan baris total pemasukan di paling bawah
    // Kolom kosong diisi koma berurutan (,,,,) agar nilai total tepat berada di bawah kolom Subtotal
    fprintf(file, "Total Pemasukan,,,,%ld\n", totalPemasukan);

    // 5. Selalu tutup file agar data tersimpan tuntas ke disk
    fclose(file);

    printf("\n-------------------------------------------\n");
    printf("[SUKSES] Data berhasil disimpan ke 'penjualan.csv'!\n");
    printf("Total Pemasukan Tercatat: Rp %ld\n", totalPemasukan);
    printf("-------------------------------------------\n");

    return 0;
}