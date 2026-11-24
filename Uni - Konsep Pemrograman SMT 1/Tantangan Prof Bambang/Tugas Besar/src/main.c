#include "disastercare.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    SistemDisasterCare sys;
    memset(&sys, 0, sizeof(sys));
    initListTransaksi(&sys.riwayatTransaksi);

    initTerminalColors();
    clearScreen();

    printf("%s===========================================================%s\n",
           CLR_BCYAN, CLR_RESET);
    printf("  %sMEMULAI DISASTERCARE — POSKO INDUK KABUPATEN SEJAHTERA%s\n",
           CLR_BWHITE, CLR_RESET);
    printf("%s===========================================================%s\n",
           CLR_BCYAN, CLR_RESET);

    (void)loadAllData(&sys);

    int pilihan = -1;
    do {
        tampilkanMenuUtama(&sys);
        if (!inputIntRange("Pilih Menu [0-14]: ", 0, 14, &pilihan)) {
            /* Keluar aman jika stdin mencapai EOF */
            pilihan = 0;
        }

        switch (pilihan) {
            case 1:
                menuKelolaPosko(&sys);
                break;
            case 2:
                menuCatatKorban(&sys);
                break;
            case 3:
                menuKelolaBantuan(&sys);
                break;
            case 4:
                menuPenerimaanBantuan(&sys);
                break;
            case 5:
                menuMonitoringStok(&sys);
                break;
            case 6:
                menuKelolaKebutuhan(&sys);
                break;
            case 7:
                menuPrioritasPosko(&sys);
                break;
            case 8:
                menuDistribusiBantuan(&sys);
                break;
            case 9:
                menuPencarianData(&sys);
                break;
            case 10:
                menuPengurutanPosko(&sys);
                break;
            case 11:
                menuRiwayatTransaksi(&sys);
                break;
            case 12:
                menuPeringatanStok(&sys);
                break;
            case 13:
                menuLaporanRekapitulasi(&sys);
                break;
            case 14:
                menuFileIO(&sys);
                break;
            case 0:
                printf("\n%s[INFO] Menyimpan seluruh data sebelum keluar...%s\n",
                       CLR_BCYAN, CLR_RESET);
                if (saveAllData(&sys)) {
                    printf("%s[OK] Data berhasil disimpan secara persisten di folder '%s/'.%s\n",
                           CLR_BGREEN, DATA_DIR, CLR_RESET);
                } else {
                    printf("%s[WARNING] Gagal menyimpan sebagian file data.%s\n",
                           CLR_BRED, CLR_RESET);
                }
                freeListTransaksi(&sys.riwayatTransaksi);
                printf("%s[OK] Memori dinamis Linked List berhasil dibebaskan (0 leak).%s\n",
                       CLR_BGREEN, CLR_RESET);
                printf("%sTerima kasih telah menggunakan DisasterCare. Sampai jumpa!%s\n",
                       CLR_BWHITE, CLR_RESET);
                break;
            default:
                break;
        }
    } while (pilihan != 0);

    return 0;
}
