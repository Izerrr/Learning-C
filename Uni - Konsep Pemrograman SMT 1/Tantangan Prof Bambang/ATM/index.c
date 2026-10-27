#include <stdio.h>
#include <stdbool.h>

// Fungsi pembantu untuk membersihkan buffer input
void bersihkanBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(void) {
    const int PIN_RAHASIA = 1234;
    int saldo = 1000000;

    // 1. Looping Utama (Mesin selalu aktif melayani nasabah baru)
    while (true) {
        printf("==========================================\n");
        printf("          SELAMAT DATANG DI ATM           \n");
        printf("==========================================\n");
        printf("[Kartu Terdeteksi]\n\n");

        int kesempatan = 3;
        bool loginBerhasil = false;

        // 2. Looping PIN (Maksimal 3 kali percobaan)
        while (kesempatan > 0) {
            int pinInput;
            printf("Masukkan 4 digit PIN (Sisa kesempatan: %d): ", kesempatan);

            if (scanf("%d", &pinInput) != 1) {
                printf("Input tidak valid! Harap masukkan angka.\n\n");
                bersihkanBuffer();
                kesempatan--;
                continue;
            }

            if (pinInput == PIN_RAHASIA) {
                loginBerhasil = true;
                printf("\nAutentikasi Berhasil.\n");
                break; // Keluar dari Looping PIN
            } else {
                kesempatan--;
                if (kesempatan > 0) {
                    printf("PIN Salah!\n\n");
                } else {
                    printf("\n[PERINGATAN] PIN salah 3 kali. Kartu Anda diblokir demi keamanan!\n");
                }
            }
        }

        // 3. Looping Menu Transaksi (Hanya jika lolos verifikasi PIN)
        if (loginBerhasil) {
            bool lanjutTransaksi = true;

            while (lanjutTransaksi) {
                printf("\n--------------- MENU UTAMA ---------------\n");
                printf("1. Cek Saldo\n");
                printf("2. Tarik Tunai\n");
                printf("3. Keluar\n");
                printf("Pilih menu (1-3): ");

                int menu;
                if (scanf("%d", &menu) != 1) {
                    printf("Pilihan menu harus berupa angka!\n");
                    bersihkanBuffer();
                    continue;
                }

                switch (menu) {
                    case 1:
                        printf("\n>> Saldo Anda saat ini: Rp%d\n", saldo);
                        break;

                    case 2: {
                        int nominal;
                        printf("\nMasukkan nominal tarik tunai: Rp");
                        if (scanf("%d", &nominal) != 1 || nominal <= 0) {
                            printf("Nominal tidak valid!\n");
                            bersihkanBuffer();
                            break;
                        }

                        if (saldo >= nominal) {
                            saldo -= nominal;
                            printf(">> Penarikan berhasil! Silakan ambil uang Anda.\n");
                            printf(">> Sisa saldo: Rp%d\n", saldo);
                        } else {
                            printf(">> Transaksi Gagal: Saldo Tidak Cukup!\n");
                        }
                        break;
                    }

                    case 3:
                        lanjutTransaksi = false;
                        break;

                    default:
                        printf("Menu tidak tersedia. Silakan pilih opsi 1-3.\n");
                        break;
                }

                // Konfirmasi transaksi lanjutan (jika belum memilih keluar)
                if (lanjutTransaksi) {
                    char respon;
                    printf("\nApakah ingin transaksi lagi? (y/n): ");
                    scanf(" %c", &respon);

                    if (respon == 'n' || respon == 'N') {
                        lanjutTransaksi = false;
                    }
                }
            }
        }

        // Selesai satu siklus nasabah, keluarkan kartu
        printf("\n[Kartu Dikeluarkan]\n");
        printf("Terima kasih telah menggunakan layanan kami.\n");
        printf("==========================================\n\n\n");
    }

    return 0;
}