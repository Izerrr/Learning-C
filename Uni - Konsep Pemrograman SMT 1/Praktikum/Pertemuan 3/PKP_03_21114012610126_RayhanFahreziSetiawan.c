#include <stdio.h>

int main() {
    // 1. Siapkan data pesanan
    int harga_kopi = 15000;
    int jumlah_beli = 2;
    int is_member = 1;

    // 2. Hitung total harga pesanan
    int total = harga_kopi * jumlah_beli;

    // 3. Tambahkan biaya kemasan ke total
    total += 2000;

    // 4. Tentukan apakah pesanan "hemat"
    int hemat = total > 30000;

    // 5. Tentukan apakah pelanggan dapat diskon
    int dapat_diskon = is_member && hemat;

    // 6. Naikkan hitungan transaksi (pre-increment)
    int jumlah_transaksi = 5;
    ++jumlah_transaksi;

    // 7. Tentukan kebalikan status member
    int bukan_member = !is_member;

    // 8. Catat dua kebiasaan iseng kasir
    int dobel_gelas = jumlah_beli << 1;
    int cek_ganjil = jumlah_transaksi & 1;

    // 9. Cetak semua hasil
    printf("Total harga =  %d\n", total);
    printf("Hemat? =  %d\n", hemat);
    printf("Dapat diskon? =  %d\n", dapat_diskon);
    printf("Jumlah transaksi =  %d\n", jumlah_transaksi);
    printf("Bukan member? =  %d\n", bukan_member);
    printf("Dobel gelas =  %d\n", dobel_gelas);
    printf("Cek ganjil =  %d\n", cek_ganjil);

    return 0;
}

    // Apabila is_member diubah menjadi 0, variabel yang akan terpengaruh adalah dapat_diskon (Karena apabila ingin diskon terpenuhi, hemat dan is_member harus sama sama terpenuhi)
    // Apabila is_member diubah menjadi 0, variabel berikutnya yang akan terpengaruh adalah bukan_member karena bukan_member akan berkebalikan dengan nilai is_member awal (operasi logika !)