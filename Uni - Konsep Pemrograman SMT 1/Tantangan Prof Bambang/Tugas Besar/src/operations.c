#include "disastercare.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================================
 * HELPER POINTER STOK & VALIDASI KEDALUWARSA
 * ========================================================================== */

int tambahStokBantuan(int *stokCurrent, int jumlahMasuk) {
    if (stokCurrent == NULL) {
        return 0;
    }
    if (jumlahMasuk <= 0) {
        printf("%s[ERROR] Jumlah penerimaan harus lebih besar dari nol!%s\n",
               CLR_BRED, CLR_RESET);
        return 0;
    }
    *stokCurrent += jumlahMasuk;
    return 1;
}

int kurangiStokBantuan(int *stokCurrent, int jumlahKeluar) {
    if (stokCurrent == NULL) {
        return 0; /* Pointer Guard */
    }
    if (jumlahKeluar <= 0) {
        printf("%s[ERROR] Jumlah pengeluaran harus lebih besar dari nol!%s\n",
               CLR_BRED, CLR_RESET);
        return 0;
    }
    if (*stokCurrent < jumlahKeluar) {
        printf("%s[ERROR] Stok tidak mencukupi! Available: %d, Requested: %d%s\n",
               CLR_BRED, *stokCurrent, jumlahKeluar, CLR_RESET);
        return 0;
    }
    *stokCurrent -= jumlahKeluar; /* Kurangi stok via dereference */
    return 1;
}

int isBarangKedaluwarsa(const Bantuan *b, const char *tanggalDistribusi) {
    if (b == NULL || b->jenisDetail != DETAIL_KEDALUWARSA) {
        return 0;
    }
    int expInt = parseDateToInt(b->detail.tanggalKedaluwarsa);
    int distInt = parseDateToInt(tanggalDistribusi);
    if (expInt < 0 || distInt < 0) {
        return 1; /* Fail-safe jika tanggal rusak */
    }
    return (distInt > expInt);
}

/* ==========================================================================
 * ALGORITMA KALKULASI PRIORITAS POSKO (S = P + M + R)
 * ========================================================================== */

int hitungSkorPengungsi(int jumlahJiwa) {
    if (jumlahJiwa > 250) {
        return 3;
    }
    if (jumlahJiwa >= 151) {
        return 2;
    }
    return 1;
}

int hitungSkorMakanan(float hariBertahan) {
    if (hariBertahan < 1.0f) {
        return 3;
    }
    if (hariBertahan < 3.0f) {
        return 2;
    }
    return 1;
}

int hitungSkorRentan(int kelompokRentan) {
    if (kelompokRentan > 50) {
        return 3;
    }
    if (kelompokRentan >= 21) {
        return 2;
    }
    return 1;
}

int hitungSkorPrioritasPosko(Posko *posko) {
    if (posko == NULL) {
        return 0;
    }
    int p = hitungSkorPengungsi(posko->jumlahJiwa);
    int m = hitungSkorMakanan(posko->hariBertahanMakanan);
    int r = hitungSkorRentan(posko->kelompokRentan);
    posko->skorPrioritas = p + m + r;
    return posko->skorPrioritas;
}

const char *getKategoriPrioritas(int skor) {
    if (skor >= 7) {
        return "TINGGI";
    }
    if (skor >= 5) {
        return "SEDANG";
    }
    return "RENDAH";
}

const char *getWarnaPrioritas(int skor) {
    if (skor >= 7) {
        return CLR_BRED;
    }
    if (skor >= 5) {
        return CLR_BYELLOW;
    }
    return CLR_BGREEN;
}

void formatDetailBantuan(const Bantuan *b, char *buf, size_t size) {
    if (b == NULL || buf == NULL || size == 0) {
        return;
    }
    if (b->jenisDetail == DETAIL_PAKAIAN) {
        snprintf(buf, size, "Ukuran: %s", b->detail.ukuranPakaian);
    } else if (b->jenisDetail == DETAIL_KEDALUWARSA) {
        snprintf(buf, size, "Exp: %s", b->detail.tanggalKedaluwarsa);
    } else {
        snprintf(buf, size, "Kit: %s", b->detail.kategoriKit);
    }
}

/* ==========================================================================
 * HELPER PENCARIAN LINEAR, GENERATOR KODE & REALISASI KEBUTUHAN
 * ========================================================================== */

int cariIndexPoskoByKode(const SistemDisasterCare *sys, const char *kode) {
    if (sys == NULL || kode == NULL) {
        return -1;
    }
    for (int i = 0; i < sys->jumlahPosko; i++) {
        if (equalsIgnoreCase(sys->posko[i].kodePosko, kode)) {
            return i;
        }
    }
    return -1;
}

int cariIndexBantuanByKode(const SistemDisasterCare *sys, const char *kode) {
    if (sys == NULL || kode == NULL) {
        return -1;
    }
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        if (equalsIgnoreCase(sys->bantuan[i].kodeBantuan, kode)) {
            return i;
        }
    }
    return -1;
}

void generateNextKode(const char *prefix, int nextNum, int digits, char *out, size_t size) {
    snprintf(out, size, "%s%0*d", prefix, digits, nextNum);
}

static int findNextKorbanNum(const SistemDisasterCare *sys) {
    int maxNum = 0;
    for (int i = 0; i < sys->jumlahKorban; i++) {
        int num = atoi(sys->korban[i].kodeKorban + 1);
        if (num > maxNum) {
            maxNum = num;
        }
    }
    return maxNum + 1;
}

static int findNextKebutuhanNum(const SistemDisasterCare *sys) {
    int maxNum = 0;
    for (int i = 0; i < sys->jumlahKebutuhan; i++) {
        int num = atoi(sys->kebutuhan[i].kodeKebutuhan + 3);
        if (num > maxNum) {
            maxNum = num;
        }
    }
    return maxNum + 1;
}

static int findNextTransaksiNum(const SistemDisasterCare *sys) {
    int maxNum = 0;
    const NodeTransaksi *curr = sys->riwayatTransaksi.head;
    while (curr != NULL) {
        int num = atoi(curr->kodeTransaksi + 3);
        if (num > maxNum) {
            maxNum = num;
        }
        curr = curr->next;
    }
    return maxNum + 1;
}

void updateRealisasiKebutuhan(SistemDisasterCare *sys, const char *kodePosko,
                              const char *kodeBantuan, int jumlah) {
    if (sys == NULL || kodePosko == NULL || kodeBantuan == NULL || jumlah <= 0) {
        return;
    }
    int sisaDistribusi = jumlah;
    int lastMatchedIdx = -1;

    for (int i = 0; i < sys->jumlahKebutuhan && sisaDistribusi > 0; i++) {
        Kebutuhan *kb = &sys->kebutuhan[i];
        if (equalsIgnoreCase(kb->kodePosko, kodePosko) &&
            equalsIgnoreCase(kb->kodeBantuan, kodeBantuan)) {
            lastMatchedIdx = i;
            int kurang = kb->jumlahPermintaan - kb->jumlahRealisasi;
            if (kurang > 0) {
                int alokasi = (sisaDistribusi < kurang) ? sisaDistribusi : kurang;
                kb->jumlahRealisasi += alokasi;
                sisaDistribusi -= alokasi;
            }
        }
    }
    if (sisaDistribusi > 0 && lastMatchedIdx >= 0) {
        sys->kebutuhan[lastMatchedIdx].jumlahRealisasi += sisaDistribusi;
    }
}

/* ==========================================================================
 * ALGORITMA SORTING (SELECTION SORT & INSERTION SORT — DESCENDING)
 * ========================================================================== */

void sortPoskoByPengungsi(Posko arr[], int n) {
    /* Selection Sort Descending berdasarkan jumlahJiwa (tie-breaker: skorPrioritas) */
    for (int i = 0; i < n - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j].jumlahJiwa > arr[maxIdx].jumlahJiwa ||
                (arr[j].jumlahJiwa == arr[maxIdx].jumlahJiwa &&
                 arr[j].skorPrioritas > arr[maxIdx].skorPrioritas)) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            Posko temp = arr[i];
            arr[i] = arr[maxIdx];
            arr[maxIdx] = temp;
        }
    }
}

void sortPoskoByPrioritas(Posko arr[], int n) {
    /* Insertion Sort Descending berdasarkan skorPrioritas (tie-breaker: jumlahJiwa) */
    for (int i = 1; i < n; i++) {
        Posko key = arr[i];
        int j = i - 1;
        while (j >= 0 &&
               (arr[j].skorPrioritas < key.skorPrioritas ||
                (arr[j].skorPrioritas == key.skorPrioritas &&
                 arr[j].jumlahJiwa < key.jumlahJiwa))) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/* ==========================================================================
 * KOMPONEN TAMPILAN TABEL & DETAIL
 * ========================================================================== */

static void printHeaderSection(const char *judul) {
    printf("\n%s=========================================================================================%s\n",
           CLR_BCYAN, CLR_RESET);
    printf("  %s%s%s\n", CLR_BWHITE, judul, CLR_RESET);
    printf("%s=========================================================================================%s\n",
           CLR_BCYAN, CLR_RESET);
}

void tampilkanTabelPosko(const Posko daftar[], int count) {
    if (count <= 0) {
        printf("%s[INFO] Belum ada data posko terdaftar.%s\n", CLR_YELLOW, CLR_RESET);
        return;
    }
    printf("%-6s | %-18s | %5s | %6s | %6s | %9s | %-13s | %-8s\n",
           "Kode", "Nama Desa / Posko", "KK", "Jiwa", "Rentan", "Mkn(Hari)", "Prioritas", "Status");
    printf("-------+--------------------+-------+--------+--------+-----------+---------------+---------\n");

    int totalKK = 0, totalJiwa = 0, totalRentan = 0;
    for (int i = 0; i < count; i++) {
        const Posko *p = &daftar[i];
        totalKK += p->jumlahKK;
        totalJiwa += p->jumlahJiwa;
        totalRentan += p->kelompokRentan;

        const char *warnaPrio = getWarnaPrioritas(p->skorPrioritas);
        const char *katPrio = getKategoriPrioritas(p->skorPrioritas);
        const char *statusStr = p->statusAktif ? "AKTIF" : "NONAKTIF";
        const char *warnaSt = p->statusAktif ? CLR_BGREEN : CLR_DIM;

        printf("%-6s | %-18.18s | %5d | %6d | %6d | %9.1f | %s%d (%-6s)%s    | %s%-8s%s\n",
               p->kodePosko, p->namaDesa, p->jumlahKK, p->jumlahJiwa,
               p->kelompokRentan, p->hariBertahanMakanan,
               warnaPrio, p->skorPrioritas, katPrio, CLR_RESET,
               warnaSt, statusStr, CLR_RESET);
    }
    printf("-------+--------------------+-------+--------+--------+-----------+---------------+---------\n");
    printf("%-6s | %-18s | %5d | %6d | %6d | %9s | %-13s | %-8s\n",
           "TOTAL", "Seluruh Posko", totalKK, totalJiwa, totalRentan, "-", "-", "-");
}

void tampilkanDetailPosko(const SistemDisasterCare *sys, const Posko *p) {
    if (p == NULL) {
        return;
    }
    int scP = hitungSkorPengungsi(p->jumlahJiwa);
    int scM = hitungSkorMakanan(p->hariBertahanMakanan);
    int scR = hitungSkorRentan(p->kelompokRentan);
    int totalS = scP + scM + scR;

    printf("\n%s--- DETAIL POSKO %s ---%s\n", CLR_BCYAN, p->kodePosko, CLR_RESET);
    printf("  Kode Posko          : %s\n", p->kodePosko);
    printf("  Nama Desa / Posko   : %s\n", p->namaDesa);
    printf("  Status Operasional  : %s%s%s\n",
           p->statusAktif ? CLR_BGREEN : CLR_BRED,
           p->statusAktif ? "AKTIF" : "NONAKTIF", CLR_RESET);
    printf("  Jumlah KK           : %d KK\n", p->jumlahKK);
    printf("  Jumlah Pengungsi    : %d Jiwa (Skor P = %d)\n", p->jumlahJiwa, scP);
    printf("  Ketahanan Makanan   : %.1f Hari (Skor M = %d)\n", p->hariBertahanMakanan, scM);
    printf("  Kelompok Rentan     : %d Orang (Skor R = %d)\n", p->kelompokRentan, scR);
    printf("  Total Skor Prioritas: %s%d (%s)%s [Rumus S = P(%d) + M(%d) + R(%d)]\n",
           getWarnaPrioritas(totalS), totalS, getKategoriPrioritas(totalS), CLR_RESET,
           scP, scM, scR);

    if (sys != NULL) {
        int jmlTrx = 0, totalUnit = 0;
        const NodeTransaksi *curr = sys->riwayatTransaksi.head;
        while (curr != NULL) {
            if (curr->jenisTransaksi == TRX_KELUAR &&
                equalsIgnoreCase(curr->kodePosko, p->kodePosko)) {
                jmlTrx++;
                totalUnit += curr->jumlah;
            }
            curr = curr->next;
        }
        printf("  Distribusi Diterima : %d transaksi (%d total unit logistik)\n",
               jmlTrx, totalUnit);
    }
}

void tampilkanTabelBantuan(const Bantuan daftar[], int count) {
    if (count <= 0) {
        printf("%s[INFO] Belum ada data master bantuan logistik.%s\n", CLR_YELLOW, CLR_RESET);
        return;
    }
    printf("%-5s | %-26s | %-6s | %6s | %5s | %-18s | %-8s\n",
           "Kode", "Nama Bantuan Logistik", "Satuan", "Stok", "Min", "Detail Spesifik", "Status");
    printf("------+----------------------------+--------+--------+-------+--------------------+---------\n");

    for (int i = 0; i < count; i++) {
        const Bantuan *b = &daftar[i];
        char detailStr[48];
        formatDetailBantuan(b, detailStr, sizeof(detailStr));

        const char *stLabel = "AMAN";
        const char *stWarna = CLR_BGREEN;
        if (b->stokTersedia < b->stokMinimum) {
            stLabel = "KRITIS!";
            stWarna = CLR_BRED;
        } else if (b->stokTersedia == b->stokMinimum) {
            stLabel = "WASPADA";
            stWarna = CLR_BYELLOW;
        }

        printf("%-5s | %-26.26s | %-6s | %6d | %5d | %-18.18s | %s%-8s%s\n",
               b->kodeBantuan, b->namaBantuan, b->satuan,
               b->stokTersedia, b->stokMinimum, detailStr,
               stWarna, stLabel, CLR_RESET);
    }
}

/* ==========================================================================
 * F01 — MENU 1: KELOLA DATA POSKO BENCANA
 * ========================================================================== */

static void tambahPoskoBaru(SistemDisasterCare *sys) {
    if (sys->jumlahPosko >= MAX_POSKO) {
        printf("%s[ERROR] Kapasitas maksimum posko (%d) telah tercapai!%s\n",
               CLR_BRED, MAX_POSKO, CLR_RESET);
        return;
    }
    Posko baru;
    memset(&baru, 0, sizeof(baru));

    printf("\n%s--- TAMBAH POSKO BARU ---%s\n", CLR_BCYAN, CLR_RESET);
    if (!inputStringSafe("Kode Posko (contoh P006)         : ",
                         baru.kodePosko, sizeof(baru.kodePosko), 0)) {
        return;
    }
    if (cariIndexPoskoByKode(sys, baru.kodePosko) >= 0) {
        printf("%s[ERROR] Registrasi ditolak! Kode Posko '%s' sudah terdaftar (duplikat).%s\n",
               CLR_BRED, baru.kodePosko, CLR_RESET);
        return;
    }
    if (!inputStringSafe("Nama Desa / Posko                : ",
                         baru.namaDesa, sizeof(baru.namaDesa), 0)) {
        return;
    }
    if (!inputIntRange("Jumlah KK Awal (0 - 10000)       : ", 0, 10000, &baru.jumlahKK)) {
        return;
    }
    if (!inputIntRange("Jumlah Jiwa Pengungsi (0 - 50000): ", 0, 50000, &baru.jumlahJiwa)) {
        return;
    }
    if (!inputIntRange("Jumlah Kelompok Rentan (0-50000) : ", 0, 50000, &baru.kelompokRentan)) {
        return;
    }
    if (!inputFloatRange("Ketahanan Makanan (hari, >=0)    : ", 0.0f, 365.0f,
                         &baru.hariBertahanMakanan)) {
        return;
    }
    baru.statusAktif = 1;
    (void)hitungSkorPrioritasPosko(&baru);

    sys->posko[sys->jumlahPosko++] = baru;
    (void)saveAllData(sys);

    printf("%s[SUKSES] Posko %s (%s) berhasil ditambahkan & disimpan ke '%s'! (Skor Prioritas: %d - %s)%s\n",
           CLR_BGREEN, baru.kodePosko, baru.namaDesa, FILE_POSKO,
           baru.skorPrioritas, getKategoriPrioritas(baru.skorPrioritas), CLR_RESET);
}

static void ubahDataPosko(SistemDisasterCare *sys) {
    tampilkanTabelPosko(sys->posko, sys->jumlahPosko);
    char kode[12];
    if (!inputStringSafe("\nMasukkan Kode Posko yang akan diubah: ", kode, sizeof(kode), 0)) {
        return;
    }
    int idx = cariIndexPoskoByKode(sys, kode);
    if (idx < 0) {
        printf("%s[ERROR] Posko dengan kode '%s' tidak ditemukan!%s\n", CLR_BRED, kode, CLR_RESET);
        return;
    }
    Posko *p = &sys->posko[idx];
    printf("\nMengubah Posko %s (%s). Tekan Enter pada nama untuk mempertahankan nilai lama.\n",
           p->kodePosko, p->namaDesa);

    char namaBaru[60];
    if (inputStringSafe("Nama Desa baru (Enter=tetap)     : ", namaBaru, sizeof(namaBaru), 1) &&
        namaBaru[0] != '\0') {
        snprintf(p->namaDesa, sizeof(p->namaDesa), "%s", namaBaru);
    }
    (void)inputIntRange("Jumlah KK baru                   : ", 0, 10000, &p->jumlahKK);
    (void)inputIntRange("Jumlah Jiwa baru                 : ", 0, 50000, &p->jumlahJiwa);
    (void)inputIntRange("Jumlah Kelompok Rentan baru      : ", 0, 50000, &p->kelompokRentan);
    (void)inputFloatRange("Ketahanan Makanan baru (hari)    : ", 0.0f, 365.0f,
                          &p->hariBertahanMakanan);

    (void)hitungSkorPrioritasPosko(p);
    (void)saveAllData(sys);
    printf("%s[SUKSES] Data Posko %s diperbarui. Skor Prioritas baru: %d (%s).%s\n",
           CLR_BGREEN, p->kodePosko, p->skorPrioritas,
           getKategoriPrioritas(p->skorPrioritas), CLR_RESET);
}

static void toggleStatusPosko(SistemDisasterCare *sys) {
    tampilkanTabelPosko(sys->posko, sys->jumlahPosko);
    char kode[12];
    if (!inputStringSafe("\nMasukkan Kode Posko (aktifkan/nonaktifkan): ", kode, sizeof(kode), 0)) {
        return;
    }
    int idx = cariIndexPoskoByKode(sys, kode);
    if (idx < 0) {
        printf("%s[ERROR] Posko dengan kode '%s' tidak ditemukan!%s\n", CLR_BRED, kode, CLR_RESET);
        return;
    }
    sys->posko[idx].statusAktif = !sys->posko[idx].statusAktif;
    (void)saveAllData(sys);
    printf("%s[SUKSES] Status Posko %s (%s) sekarang: %s.%s\n",
           CLR_BGREEN, sys->posko[idx].kodePosko, sys->posko[idx].namaDesa,
           sys->posko[idx].statusAktif ? "AKTIF" : "NONAKTIF", CLR_RESET);
}

void menuKelolaPosko(SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F01 — KELOLA DATA MASTER POSKO BENCANA");
        printf(" 1. Tampilkan Daftar Semua Posko\n");
        printf(" 2. Tambah Posko Baru\n");
        printf(" 3. Ubah Data Posko\n");
        printf(" 4. Aktifkan / Nonaktifkan Posko\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-4]: ", 0, 4, &sub)) {
            return;
        }
        switch (sub) {
            case 1:
                printf("\n");
                tampilkanTabelPosko(sys->posko, sys->jumlahPosko);
                pauseScreen();
                break;
            case 2:
                tambahPoskoBaru(sys);
                pauseScreen();
                break;
            case 3:
                ubahDataPosko(sys);
                pauseScreen();
                break;
            case 4:
                toggleStatusPosko(sys);
                pauseScreen();
                break;
            default:
                break;
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F02 — MENU 2: PENCATATAN DATA KORBAN & KELOMPOK RENTAN
 * ========================================================================== */

static void catatKorbanBaru(SistemDisasterCare *sys) {
    if (sys->jumlahKorban >= MAX_KORBAN) {
        printf("%s[ERROR] Kapasitas pencatatan korban (%d) penuh!%s\n",
               CLR_BRED, MAX_KORBAN, CLR_RESET);
        return;
    }
    printf("\n");
    tampilkanTabelPosko(sys->posko, sys->jumlahPosko);

    Korban k;
    memset(&k, 0, sizeof(k));
    if (!inputStringSafe("\nKode Posko Tujuan                : ", k.kodePosko, sizeof(k.kodePosko), 0)) {
        return;
    }
    int idx = cariIndexPoskoByKode(sys, k.kodePosko);
    if (idx < 0) {
        printf("%s[ERROR] Posko '%s' tidak ditemukan!%s\n", CLR_BRED, k.kodePosko, CLR_RESET);
        return;
    }
    if (!sys->posko[idx].statusAktif) {
        printf("%s[ERROR] Posko '%s' sedang NONAKTIF! Aktifkan posko terlebih dahulu.%s\n",
               CLR_BRED, sys->posko[idx].kodePosko, CLR_RESET);
        return;
    }
    snprintf(k.kodePosko, sizeof(k.kodePosko), "%s", sys->posko[idx].kodePosko);

    if (!inputStringSafe("Nama Kepala Keluarga / Kelompok  : ",
                         k.namaKepalaKeluarga, sizeof(k.namaKepalaKeluarga), 0)) {
        return;
    }
    if (!inputPositiveInt("Jumlah KK dicatat (> 0)          : ", &k.jumlahKK)) {
        return;
    }
    if (!inputPositiveInt("Jumlah Anggota / Jiwa (> 0)      : ", &k.jumlahAnggota)) {
        return;
    }
    if (!inputIntRange("Jumlah Lansia (>= 0)             : ", 0, k.jumlahAnggota, &k.jumlahLansia)) {
        return;
    }
    if (!inputIntRange("Jumlah Balita (>= 0)             : ", 0, k.jumlahAnggota - k.jumlahLansia,
                       &k.jumlahBalita)) {
        return;
    }
    int sisaJiwa = k.jumlahAnggota - k.jumlahLansia - k.jumlahBalita;
    if (!inputIntRange("Jumlah Ibu Hamil (>= 0)          : ", 0, sisaJiwa, &k.jumlahIbuHamil)) {
        return;
    }
    k.totalRentan = k.jumlahLansia + k.jumlahBalita + k.jumlahIbuHamil;

    generateNextKode("K", findNextKorbanNum(sys), 3, k.kodeKorban, sizeof(k.kodeKorban));
    sys->korban[sys->jumlahKorban++] = k;

    /* Akumulasi otomatis ke Master Posko terkait & re-hitung prioritas */
    Posko *p = &sys->posko[idx];
    p->jumlahKK += k.jumlahKK;
    p->jumlahJiwa += k.jumlahAnggota;
    p->kelompokRentan += k.totalRentan;
    (void)hitungSkorPrioritasPosko(p);

    (void)saveAllData(sys);

    printf("\n%s[SUKSES] Data korban %s berhasil dicatat & terakumulasi ke Posko %s (%s)!%s\n",
           CLR_BGREEN, k.kodeKorban, p->kodePosko, p->namaDesa, CLR_RESET);
    printf("  Akumulasi Posko %s kini -> KK: %d | Jiwa: %d | Rentan: %d | Skor Prioritas: %d (%s)\n",
           p->kodePosko, p->jumlahKK, p->jumlahJiwa, p->kelompokRentan,
           p->skorPrioritas, getKategoriPrioritas(p->skorPrioritas));
}

static void tampilkanDaftarKorban(const SistemDisasterCare *sys) {
    if (sys->jumlahKorban == 0) {
        printf("%s[INFO] Belum ada catatan detail korban yang ditambahkan.%s\n",
               CLR_YELLOW, CLR_RESET);
        return;
    }
    printf("\n%-6s | %-6s | %-24s | %4s | %5s | %6s | %6s | %5s | %6s\n",
           "Kode", "Posko", "Nama KK / Kelompok", "KK", "Jiwa", "Lansia", "Balita", "Bumil", "Rentan");
    printf("-------+--------+--------------------------+------+-------+--------+--------+-------+-------\n");
    for (int i = 0; i < sys->jumlahKorban; i++) {
        const Korban *k = &sys->korban[i];
        printf("%-6s | %-6s | %-24.24s | %4d | %5d | %6d | %6d | %5d | %6d\n",
               k->kodeKorban, k->kodePosko, k->namaKepalaKeluarga,
               k->jumlahKK, k->jumlahAnggota,
               k->jumlahLansia, k->jumlahBalita, k->jumlahIbuHamil, k->totalRentan);
    }
}

void menuCatatKorban(SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F02 — PENCATATAN DATA KORBAN & KELOMPOK RENTAN");
        printf(" 1. Catat Data Korban & Kelompok Rentan Baru\n");
        printf(" 2. Lihat Daftar Catatan Korban Terdaftar\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-2]: ", 0, 2, &sub)) {
            return;
        }
        switch (sub) {
            case 1:
                catatKorbanBaru(sys);
                pauseScreen();
                break;
            case 2:
                tampilkanDaftarKorban(sys);
                pauseScreen();
                break;
            default:
                break;
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F03 — MENU 3: KELOLA MASTER JENIS BANTUAN LOGISTIK (TAGGED UNION)
 * ========================================================================== */

static int inputDetailTaggedUnion(Bantuan *b) {
    int jenis = 0;
    printf("Pilih Tipe Detail Spesifik (Tagged Union):\n");
    printf("  1. Pakaian / Sandang (Ukuran, misal Dewasa-XL)\n");
    printf("  2. Pangan / Obat (Tanggal Kedaluwarsa DD-MM-YYYY)\n");
    printf("  3. Hygiene / Kebersihan (Kategori Kit)\n");
    if (!inputIntRange("Pilihan Tipe Detail [1-3]        : ", 1, 3, &jenis)) {
        return 0;
    }
    b->jenisDetail = (JenisDetailBantuan)jenis;
    if (jenis == DETAIL_PAKAIAN) {
        return inputStringSafe("Ukuran Pakaian (misal Dewasa-XL) : ",
                               b->detail.ukuranPakaian, sizeof(b->detail.ukuranPakaian), 0);
    }
    if (jenis == DETAIL_KEDALUWARSA) {
        return inputTanggalSafe("Tanggal Kedaluwarsa (DD-MM-YYYY) : ",
                                b->detail.tanggalKedaluwarsa,
                                sizeof(b->detail.tanggalKedaluwarsa), 0);
    }
    return inputStringSafe("Kategori Kit Hygiene             : ",
                           b->detail.kategoriKit, sizeof(b->detail.kategoriKit), 0);
}

static void tambahBantuanBaru(SistemDisasterCare *sys) {
    if (sys->jumlahBantuan >= MAX_BANTUAN) {
        printf("%s[ERROR] Kapasitas master jenis bantuan (%d) penuh!%s\n",
               CLR_BRED, MAX_BANTUAN, CLR_RESET);
        return;
    }
    Bantuan b;
    memset(&b, 0, sizeof(b));

    printf("\n%s--- TAMBAH JENIS BANTUAN BARU ---%s\n", CLR_BCYAN, CLR_RESET);
    if (!inputStringSafe("Kode Bantuan (contoh B009)       : ",
                         b.kodeBantuan, sizeof(b.kodeBantuan), 0)) {
        return;
    }
    if (cariIndexBantuanByKode(sys, b.kodeBantuan) >= 0) {
        printf("%s[ERROR] Kode Bantuan '%s' sudah terdaftar (duplikat)!%s\n",
               CLR_BRED, b.kodeBantuan, CLR_RESET);
        return;
    }
    if (!inputStringSafe("Nama Bantuan Logistik            : ",
                         b.namaBantuan, sizeof(b.namaBantuan), 0)) {
        return;
    }
    if (!inputStringSafe("Satuan (kg/dus/liter/paket/buah) : ",
                         b.satuan, sizeof(b.satuan), 0)) {
        return;
    }
    if (!inputStringSafe("Kategori Logistik                : ",
                         b.kategoriLogistik, sizeof(b.kategoriLogistik), 0)) {
        return;
    }
    if (!inputIntRange("Stok Awal (>= 0)                 : ", 0, 1000000, &b.stokTersedia)) {
        return;
    }
    if (!inputIntRange("Batas Stok Minimum (>= 0)        : ", 0, 1000000, &b.stokMinimum)) {
        return;
    }
    if (!inputDetailTaggedUnion(&b)) {
        return;
    }

    sys->bantuan[sys->jumlahBantuan++] = b;
    (void)saveAllData(sys);
    printf("%s[SUKSES] Jenis bantuan %s (%s) berhasil ditambahkan & disimpan!%s\n",
           CLR_BGREEN, b.kodeBantuan, b.namaBantuan, CLR_RESET);
}

static void ubahBantuanMaster(SistemDisasterCare *sys) {
    printf("\n");
    tampilkanTabelBantuan(sys->bantuan, sys->jumlahBantuan);
    char kode[12];
    if (!inputStringSafe("\nKode Bantuan yang akan diubah    : ", kode, sizeof(kode), 0)) {
        return;
    }
    int idx = cariIndexBantuanByKode(sys, kode);
    if (idx < 0) {
        printf("%s[ERROR] Bantuan '%s' tidak ditemukan!%s\n", CLR_BRED, kode, CLR_RESET);
        return;
    }
    Bantuan *b = &sys->bantuan[idx];
    char namaBuf[60];
    char satuanBuf[20];
    if (inputStringSafe("Nama Bantuan baru (Enter=tetap)  : ", namaBuf, sizeof(namaBuf), 1) &&
        namaBuf[0] != '\0') {
        snprintf(b->namaBantuan, sizeof(b->namaBantuan), "%s", namaBuf);
    }
    if (inputStringSafe("Satuan baru (Enter=tetap)        : ", satuanBuf, sizeof(satuanBuf), 1) &&
        satuanBuf[0] != '\0') {
        snprintf(b->satuan, sizeof(b->satuan), "%s", satuanBuf);
    }
    (void)inputIntRange("Batas Stok Minimum baru          : ", 0, 1000000, &b->stokMinimum);

    int gantiDetail = 0;
    if (inputIntRange("Ubah detail spesifik (1=Ya, 0=Tidak): ", 0, 1, &gantiDetail) && gantiDetail == 1) {
        (void)inputDetailTaggedUnion(b);
    }
    (void)saveAllData(sys);
    printf("%s[SUKSES] Data bantuan %s berhasil diperbarui!%s\n",
           CLR_BGREEN, b->kodeBantuan, CLR_RESET);
}

void menuKelolaBantuan(SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F03 — KELOLA MASTER JENIS BANTUAN LOGISTIK");
        printf(" 1. Tampilkan Daftar Master Jenis Bantuan\n");
        printf(" 2. Tambah Jenis Bantuan Baru\n");
        printf(" 3. Ubah Data Master Bantuan\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-3]: ", 0, 3, &sub)) {
            return;
        }
        switch (sub) {
            case 1:
                printf("\n");
                tampilkanTabelBantuan(sys->bantuan, sys->jumlahBantuan);
                pauseScreen();
                break;
            case 2:
                tambahBantuanBaru(sys);
                pauseScreen();
                break;
            case 3:
                ubahBantuanMaster(sys);
                pauseScreen();
                break;
            default:
                break;
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F04 — MENU 4: CATAT BANTUAN MASUK (PENERIMAAN LOGISTIK)
 * ========================================================================== */

void menuPenerimaanBantuan(SistemDisasterCare *sys) {
    clearScreen();
    printHeaderSection("F04 — PENERIMAAN BANTUAN MASUK (POSKO INDUK)");
    tampilkanTabelBantuan(sys->bantuan, sys->jumlahBantuan);

    char kodeBantuan[12];
    if (!inputStringSafe("\nKode Bantuan Masuk               : ",
                         kodeBantuan, sizeof(kodeBantuan), 0)) {
        return;
    }
    int idx = cariIndexBantuanByKode(sys, kodeBantuan);
    if (idx < 0) {
        printf("%s[ERROR] Bantuan dengan kode '%s' tidak ditemukan!%s\n",
               CLR_BRED, kodeBantuan, CLR_RESET);
        pauseScreen();
        return;
    }
    Bantuan *b = &sys->bantuan[idx];
    printf("  Terpilih: %s%s (%s)%s | Stok saat ini: %d %s\n",
           CLR_BCYAN, b->kodeBantuan, b->namaBantuan, CLR_RESET, b->stokTersedia, b->satuan);

    char donor[64];
    if (!inputStringSafe("Sumber / Nama Donor              : ", donor, sizeof(donor), 0)) {
        return;
    }
    int jumlah = 0;
    if (!inputPositiveInt("Jumlah Penerimaan (> 0, 'batal') : ", &jumlah)) {
        printf("%s[INFO] Penerimaan bantuan dibatalkan.%s\n", CLR_YELLOW, CLR_RESET);
        pauseScreen();
        return;
    }
    char tanggal[12];
    if (!inputTanggalSafe("Tanggal Terima (DD-MM-YYYY, Enter=hari ini): ",
                          tanggal, sizeof(tanggal), 1)) {
        return;
    }

    int stokSebelum = b->stokTersedia;
    if (!tambahStokBantuan(&b->stokTersedia, jumlah)) {
        pauseScreen();
        return;
    }

    NodeTransaksi trx;
    memset(&trx, 0, sizeof(trx));
    generateNextKode("TRX", findNextTransaksiNum(sys), 4,
                     trx.kodeTransaksi, sizeof(trx.kodeTransaksi));
    snprintf(trx.tanggal, sizeof(trx.tanggal), "%s", tanggal);
    snprintf(trx.kodePosko, sizeof(trx.kodePosko), "INDUK");
    snprintf(trx.kodeBantuan, sizeof(trx.kodeBantuan), "%s", b->kodeBantuan);
    snprintf(trx.keterangan, sizeof(trx.keterangan), "Donor: %.55s", donor);
    trx.jenisTransaksi = TRX_MASUK;
    trx.jumlah = jumlah;

    if (appendTransaksi(&sys->riwayatTransaksi, &trx)) {
        (void)saveAllData(sys);
        printf("\n%s[SUKSES] Penerimaan %s tercatat di Linked List & disimpan!%s\n",
               CLR_BGREEN, trx.kodeTransaksi, CLR_RESET);
        printf("  Bantuan : %s (%s)\n", b->kodeBantuan, b->namaBantuan);
        printf("  Stok    : %d %s -> %s%d %s%s (+%d %s)\n",
               stokSebelum, b->satuan, CLR_BGREEN, b->stokTersedia, b->satuan, CLR_RESET,
               jumlah, b->satuan);
    }
    pauseScreen();
}

/* ==========================================================================
 * F05 — MENU 5: MONITORING STOK LOGISTIK REAL-TIME
 * ========================================================================== */

void menuMonitoringStok(const SistemDisasterCare *sys) {
    clearScreen();
    printHeaderSection("F05 — MONITORING STOK LOGISTIK REAL-TIME (POSKO INDUK)");
    tampilkanTabelBantuan(sys->bantuan, sys->jumlahBantuan);

    int kritis = 0;
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        if (sys->bantuan[i].stokTersedia < sys->bantuan[i].stokMinimum) {
            kritis++;
        }
    }
    if (kritis > 0) {
        printf("\n%s[PERINGATAN] Terdapat %d jenis bantuan di bawah batas stok minimum (KRITIS)!%s\n",
               CLR_BRED, kritis, CLR_RESET);
        printf("  Buka Menu 12 untuk rincian peringatan stok minimum.\n");
    } else {
        printf("\n%s[AMAN] Seluruh stok logistik berada pada atau di atas batas minimum.%s\n",
               CLR_BGREEN, CLR_RESET);
    }
    pauseScreen();
}

/* ==========================================================================
 * F06 — MENU 6: CATAT KEBUTUHAN POSKO DESA
 * ========================================================================== */

static void tampilkanTabelKebutuhan(const SistemDisasterCare *sys) {
    if (sys->jumlahKebutuhan == 0) {
        printf("%s[INFO] Belum ada data permintaan kebutuhan posko.%s\n", CLR_YELLOW, CLR_RESET);
        return;
    }
    printf("\n%-7s | %-6s | %-15s | %-5s | %-20s | %6s | %6s | %6s | %-10s\n",
           "KodeReq", "Posko", "Nama Desa", "KodeB", "Nama Bantuan", "Minta", "Real.", "Sisa", "Status");
    printf("--------+--------+-----------------+-------+----------------------+--------+--------+--------+-----------\n");

    for (int i = 0; i < sys->jumlahKebutuhan; i++) {
        const Kebutuhan *kb = &sys->kebutuhan[i];
        int pIdx = cariIndexPoskoByKode(sys, kb->kodePosko);
        int bIdx = cariIndexBantuanByKode(sys, kb->kodeBantuan);
        const char *namaDesa = (pIdx >= 0) ? sys->posko[pIdx].namaDesa : "-";
        const char *namaBant = (bIdx >= 0) ? sys->bantuan[bIdx].namaBantuan : "-";

        int sisa = kb->jumlahPermintaan - kb->jumlahRealisasi;
        if (sisa < 0) {
            sisa = 0;
        }
        const char *st = (sisa == 0) ? "TERPENUHI" : "KURANG";
        const char *clr = (sisa == 0) ? CLR_BGREEN : CLR_BYELLOW;

        printf("%-7s | %-6s | %-15.15s | %-5s | %-20.20s | %6d | %6d | %6d | %s%-10s%s\n",
               kb->kodeKebutuhan, kb->kodePosko, namaDesa, kb->kodeBantuan, namaBant,
               kb->jumlahPermintaan, kb->jumlahRealisasi, sisa, clr, st, CLR_RESET);
    }
}

static void tambahKebutuhanPosko(SistemDisasterCare *sys) {
    if (sys->jumlahKebutuhan >= MAX_KEBUTUHAN) {
        printf("%s[ERROR] Kapasitas pencatatan kebutuhan penuh!%s\n", CLR_BRED, CLR_RESET);
        return;
    }
    printf("\n");
    tampilkanTabelPosko(sys->posko, sys->jumlahPosko);

    Kebutuhan kb;
    memset(&kb, 0, sizeof(kb));
    if (!inputStringSafe("\nKode Posko Pemohon               : ", kb.kodePosko, sizeof(kb.kodePosko), 0)) {
        return;
    }
    int pIdx = cariIndexPoskoByKode(sys, kb.kodePosko);
    if (pIdx < 0 || !sys->posko[pIdx].statusAktif) {
        printf("%s[ERROR] Posko tidak ditemukan atau sedang nonaktif!%s\n", CLR_BRED, CLR_RESET);
        return;
    }
    snprintf(kb.kodePosko, sizeof(kb.kodePosko), "%s", sys->posko[pIdx].kodePosko);

    printf("\n");
    tampilkanTabelBantuan(sys->bantuan, sys->jumlahBantuan);
    if (!inputStringSafe("\nKode Bantuan Dibutuhkan          : ", kb.kodeBantuan, sizeof(kb.kodeBantuan), 0)) {
        return;
    }
    int bIdx = cariIndexBantuanByKode(sys, kb.kodeBantuan);
    if (bIdx < 0) {
        printf("%s[ERROR] Kode bantuan '%s' tidak ditemukan!%s\n", CLR_BRED, kb.kodeBantuan, CLR_RESET);
        return;
    }
    snprintf(kb.kodeBantuan, sizeof(kb.kodeBantuan), "%s", sys->bantuan[bIdx].kodeBantuan);

    if (!inputPositiveInt("Jumlah Permintaan (> 0)          : ", &kb.jumlahPermintaan)) {
        return;
    }
    if (!inputTanggalSafe("Tanggal Permintaan (DD-MM-YYYY, Enter=hari ini): ",
                          kb.tanggalPermintaan, sizeof(kb.tanggalPermintaan), 1)) {
        return;
    }
    kb.jumlahRealisasi = 0;
    generateNextKode("REQ", findNextKebutuhanNum(sys), 3,
                     kb.kodeKebutuhan, sizeof(kb.kodeKebutuhan));

    sys->kebutuhan[sys->jumlahKebutuhan++] = kb;
    (void)saveAllData(sys);
    printf("%s[SUKSES] Permintaan %s (%s butuh %d %s %s) berhasil dicatat!%s\n",
           CLR_BGREEN, kb.kodeKebutuhan, kb.kodePosko, kb.jumlahPermintaan,
           sys->bantuan[bIdx].satuan, sys->bantuan[bIdx].namaBantuan, CLR_RESET);
}

void menuKelolaKebutuhan(SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F06 — PENCATATAN KEBUTUHAN SPESIFIK POSKO DESA");
        printf(" 1. Lihat Daftar Kebutuhan & Realisasi Posko\n");
        printf(" 2. Catat Permintaan Kebutuhan Posko Baru\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-2]: ", 0, 2, &sub)) {
            return;
        }
        switch (sub) {
            case 1:
                tampilkanTabelKebutuhan(sys);
                pauseScreen();
                break;
            case 2:
                tambahKebutuhanPosko(sys);
                pauseScreen();
                break;
            default:
                break;
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F07 — MENU 7: HITUNG & TAMPILKAN PRIORITAS POSKO (S = P + M + R)
 * ========================================================================== */

static void tampilkanTabelPrioritasLengkap(SistemDisasterCare *sys) {
    printf("\n%-6s | %-16s | %9s | %11s | %10s | %7s | %-10s\n",
           "Kode", "Nama Desa", "Jiwa (P)", "Mkn/Hr (M)", "Rentan (R)", "Total S", "Kategori");
    printf("-------+------------------+-----------+-------------+------------+---------+-----------\n");

    for (int i = 0; i < sys->jumlahPosko; i++) {
        Posko *p = &sys->posko[i];
        int scP = hitungSkorPengungsi(p->jumlahJiwa);
        int scM = hitungSkorMakanan(p->hariBertahanMakanan);
        int scR = hitungSkorRentan(p->kelompokRentan);
        int s = hitungSkorPrioritasPosko(p);

        printf("%-6s | %-16.16s | %4d (P=%d) | %5.1f (M=%d) | %5d (R=%d) |   %s%2d%s    | %s%-10s%s\n",
               p->kodePosko, p->namaDesa,
               p->jumlahJiwa, scP,
               p->hariBertahanMakanan, scM,
               p->kelompokRentan, scR,
               getWarnaPrioritas(s), s, CLR_RESET,
               getWarnaPrioritas(s), getKategoriPrioritas(s), CLR_RESET);
    }
    printf("\n%sKeterangan Formula S = P + M + R (Rentang 3..9):%s\n", CLR_DIM, CLR_RESET);
    printf("  P (Pengungsi) : >250=3 | 151-250=2 | <=150=1\n");
    printf("  M (Makanan)   : <1 hari=3 | 1-<3 hari=2 | >=3 hari=1\n");
    printf("  R (Rentan)    : >50=3 | 21-50=2 | <=20=1\n");
    printf("  Kategori      : %s7-9 TINGGI%s | %s5-6 SEDANG%s | %s3-4 RENDAH%s\n",
           CLR_BRED, CLR_RESET, CLR_BYELLOW, CLR_RESET, CLR_BGREEN, CLR_RESET);
}

void menuPrioritasPosko(SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F07 — KALKULASI & KLASIFIKASI PRIORITAS POSKO (S = P + M + R)");
        tampilkanTabelPrioritasLengkap(sys);

        printf("\nSubmenu Prioritas:\n");
        printf(" 1. Cek Rincian Skor Prioritas Satu Posko\n");
        printf(" 2. Update Ketahanan Makanan (Hari) & Hitung Ulang Posko\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-2]: ", 0, 2, &sub)) {
            return;
        }
        if (sub == 1) {
            char kode[12];
            if (inputStringSafe("Masukkan Kode Posko (contoh P003): ", kode, sizeof(kode), 0)) {
                int idx = cariIndexPoskoByKode(sys, kode);
                if (idx >= 0) {
                    (void)hitungSkorPrioritasPosko(&sys->posko[idx]);
                    tampilkanDetailPosko(sys, &sys->posko[idx]);
                } else {
                    printf("%s[ERROR] Posko '%s' tidak ditemukan!%s\n", CLR_BRED, kode, CLR_RESET);
                }
            }
            pauseScreen();
        } else if (sub == 2) {
            char kode[12];
            if (inputStringSafe("Masukkan Kode Posko              : ", kode, sizeof(kode), 0)) {
                int idx = cariIndexPoskoByKode(sys, kode);
                if (idx >= 0) {
                    float hari = 0.0f;
                    if (inputFloatRange("Ketahanan Makanan baru (hari, misal 0.5): ",
                                        0.0f, 365.0f, &hari)) {
                        sys->posko[idx].hariBertahanMakanan = hari;
                        (void)hitungSkorPrioritasPosko(&sys->posko[idx]);
                        (void)saveAllData(sys);
                        tampilkanDetailPosko(sys, &sys->posko[idx]);
                    }
                } else {
                    printf("%s[ERROR] Posko '%s' tidak ditemukan!%s\n", CLR_BRED, kode, CLR_RESET);
                }
            }
            pauseScreen();
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F08 & F09 — MENU 8: PROSES DISTRIBUSI BANTUAN KE POSKO (VALIDASI KETAT)
 * ========================================================================== */

void menuDistribusiBantuan(SistemDisasterCare *sys) {
    clearScreen();
    printHeaderSection("F08 & F09 — PROSES DISTRIBUSI BANTUAN KE POSKO (VALIDASI KETAT)");
    tampilkanTabelPosko(sys->posko, sys->jumlahPosko);

    char kodePosko[12];
    if (!inputStringSafe("\nKode Posko Penerima              : ", kodePosko, sizeof(kodePosko), 0)) {
        return;
    }
    int pIdx = cariIndexPoskoByKode(sys, kodePosko);
    if (pIdx < 0) {
        printf("%s[DITOLAK] Posko dengan kode '%s' tidak ditemukan!%s\n",
               CLR_BRED, kodePosko, CLR_RESET);
        pauseScreen();
        return;
    }
    if (!sys->posko[pIdx].statusAktif) {
        printf("%s[DITOLAK] Posko %s (%s) berstatus NONAKTIF!%s\n",
               CLR_BRED, sys->posko[pIdx].kodePosko, sys->posko[pIdx].namaDesa, CLR_RESET);
        pauseScreen();
        return;
    }

    printf("\n");
    tampilkanTabelBantuan(sys->bantuan, sys->jumlahBantuan);

    char kodeBantuan[12];
    if (!inputStringSafe("\nKode Bantuan yang Didistribusikan: ",
                         kodeBantuan, sizeof(kodeBantuan), 0)) {
        return;
    }
    int bIdx = cariIndexBantuanByKode(sys, kodeBantuan);
    if (bIdx < 0) {
        printf("%s[DITOLAK] Bantuan dengan kode '%s' tidak ditemukan!%s\n",
               CLR_BRED, kodeBantuan, CLR_RESET);
        pauseScreen();
        return;
    }

    Bantuan *b = &sys->bantuan[bIdx];
    Posko *p = &sys->posko[pIdx];

    char tanggal[12];
    if (!inputTanggalSafe("Tanggal Distribusi (DD-MM-YYYY, Enter=hari ini): ",
                          tanggal, sizeof(tanggal), 1)) {
        return;
    }

    /* Validasi Kedaluwarsa Akurat (YYYYMMDD) — U14 */
    if (isBarangKedaluwarsa(b, tanggal)) {
        printf("\n%s[DITOLAK — BARANG KEDALUWARSA]%s\n", CLR_BRED, CLR_RESET);
        printf("  Bantuan %s (%s) telah melewati tanggal kedaluwarsa!\n",
               b->kodeBantuan, b->namaBantuan);
        printf("  Tanggal Kedaluwarsa: %s%s%s | Tanggal Distribusi: %s\n",
               CLR_BRED, b->detail.tanggalKedaluwarsa, CLR_RESET, tanggal);
        printf("  Distribusi diblokir demi keselamatan pengungsi.\n");
        pauseScreen();
        return;
    }

    printf("  Stok tersedia %s (%s): %d %s\n",
           b->kodeBantuan, b->namaBantuan, b->stokTersedia, b->satuan);

    int jumlah = 0;
    if (!inputPositiveInt("Jumlah Distribusi (> 0, 'batal') : ", &jumlah)) {
        printf("%s[INFO] Distribusi dibatalkan.%s\n", CLR_YELLOW, CLR_RESET);
        pauseScreen();
        return;
    }

    /* Validasi & Pengurangan Stok Aman via Pointer — U05 & U06 */
    int stokSebelum = b->stokTersedia;
    if (!kurangiStokBantuan(&b->stokTersedia, jumlah)) {
        printf("%s[DITOLAK] Transaksi dibatalkan. Stok %s tetap %d %s.%s\n",
               CLR_BYELLOW, b->kodeBantuan, b->stokTersedia, b->satuan, CLR_RESET);
        pauseScreen();
        return;
    }

    char catatan[64];
    if (!inputStringSafe("Catatan / Petugas (Enter=Reguler): ", catatan, sizeof(catatan), 1)) {
        snprintf(catatan, sizeof(catatan), "Distribusi Reguler");
    } else if (catatan[0] == '\0') {
        snprintf(catatan, sizeof(catatan), "Distribusi Reguler");
    }

    NodeTransaksi trx;
    memset(&trx, 0, sizeof(trx));
    generateNextKode("TRX", findNextTransaksiNum(sys), 4,
                     trx.kodeTransaksi, sizeof(trx.kodeTransaksi));
    snprintf(trx.tanggal, sizeof(trx.tanggal), "%s", tanggal);
    snprintf(trx.kodePosko, sizeof(trx.kodePosko), "%s", p->kodePosko);
    snprintf(trx.kodeBantuan, sizeof(trx.kodeBantuan), "%s", b->kodeBantuan);
    snprintf(trx.keterangan, sizeof(trx.keterangan), "%s", catatan);
    trx.jenisTransaksi = TRX_KELUAR;
    trx.jumlah = jumlah;

    if (appendTransaksi(&sys->riwayatTransaksi, &trx)) {
        updateRealisasiKebutuhan(sys, p->kodePosko, b->kodeBantuan, jumlah);
        (void)saveAllData(sys);

        printf("\n%s[SUKSES] Distribusi %s berhasil diproses & disimpan!%s\n",
               CLR_BGREEN, trx.kodeTransaksi, CLR_RESET);
        printf("  Tujuan  : %s (%s)\n", p->kodePosko, p->namaDesa);
        printf("  Bantuan : %s (%s) sebanyak %d %s\n",
               b->kodeBantuan, b->namaBantuan, jumlah, b->satuan);
        printf("  Stok    : %d %s -> %s%d %s%s\n",
               stokSebelum, b->satuan, CLR_BCYAN, b->stokTersedia, b->satuan, CLR_RESET);

        if (b->stokTersedia < b->stokMinimum) {
            printf("%s[ALERT STOK KRITIS] Stok %s (%d %s) kini berada di bawah batas minimum (%d %s)!%s\n",
                   CLR_BRED, b->kodeBantuan, b->stokTersedia, b->satuan,
                   b->stokMinimum, b->satuan, CLR_RESET);
        }
    }
    pauseScreen();
}

/* ==========================================================================
 * F10 — MENU 9: PENCARIAN DATA (LINEAR SEARCH CASE-INSENSITIVE)
 * ========================================================================== */

static void cariPoskoLinear(const SistemDisasterCare *sys) {
    char keyword[64];
    if (!inputStringSafe("\nKata kunci Kode / Nama Posko     : ", keyword, sizeof(keyword), 0)) {
        return;
    }
    int found = 0;
    for (int i = 0; i < sys->jumlahPosko; i++) {
        const Posko *p = &sys->posko[i];
        if (containsIgnoreCase(p->kodePosko, keyword) ||
            containsIgnoreCase(p->namaDesa, keyword)) {
            tampilkanDetailPosko(sys, p);
            found++;
        }
    }
    if (found == 0) {
        printf("%s[INFO] Tidak ditemukan posko yang cocok dengan '%s'.%s\n",
               CLR_YELLOW, keyword, CLR_RESET);
    } else {
        printf("\n%sTotal ditemukan: %d posko.%s\n", CLR_BGREEN, found, CLR_RESET);
    }
}

static void cariBantuanLinear(const SistemDisasterCare *sys) {
    char keyword[64];
    if (!inputStringSafe("\nKata kunci Kode / Nama / Kategori: ", keyword, sizeof(keyword), 0)) {
        return;
    }
    Bantuan hasil[MAX_BANTUAN];
    int found = 0;
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        const Bantuan *b = &sys->bantuan[i];
        if (containsIgnoreCase(b->kodeBantuan, keyword) ||
            containsIgnoreCase(b->namaBantuan, keyword) ||
            containsIgnoreCase(b->kategoriLogistik, keyword)) {
            hasil[found++] = *b;
        }
    }
    if (found == 0) {
        printf("%s[INFO] Tidak ditemukan bantuan yang cocok dengan '%s'.%s\n",
               CLR_YELLOW, keyword, CLR_RESET);
    } else {
        printf("\n");
        tampilkanTabelBantuan(hasil, found);
        printf("%sTotal ditemukan: %d jenis bantuan.%s\n", CLR_BGREEN, found, CLR_RESET);
    }
}

static void cariTransaksiLinear(const SistemDisasterCare *sys) {
    char keyword[64];
    if (!inputStringSafe("\nKata kunci Trx / Posko / Bantuan / Tgl: ", keyword, sizeof(keyword), 0)) {
        return;
    }
    int found = 0;
    const NodeTransaksi *curr = sys->riwayatTransaksi.head;
    while (curr != NULL) {
        if (containsIgnoreCase(curr->kodeTransaksi, keyword) ||
            containsIgnoreCase(curr->kodePosko, keyword) ||
            containsIgnoreCase(curr->kodeBantuan, keyword) ||
            containsIgnoreCase(curr->tanggal, keyword) ||
            containsIgnoreCase(curr->keterangan, keyword)) {
            if (found == 0) {
                printf("\n%-8s | %-10s | %-6s | %-6s | %-6s | %6s | %-24s\n",
                       "KodeTrx", "Tanggal", "Jenis", "Posko", "KodeB", "Jumlah", "Keterangan");
                printf("---------+------------+--------+--------+--------+--------+-------------------------\n");
            }
            const char *jenisStr = (curr->jenisTransaksi == TRX_MASUK) ? "MASUK" : "KELUAR";
            const char *clr = (curr->jenisTransaksi == TRX_MASUK) ? CLR_BGREEN : CLR_BCYAN;
            printf("%-8s | %-10s | %s%-6s%s | %-6s | %-6s | %6d | %-24.24s\n",
                   curr->kodeTransaksi, curr->tanggal, clr, jenisStr, CLR_RESET,
                   curr->kodePosko, curr->kodeBantuan, curr->jumlah, curr->keterangan);
            found++;
        }
        curr = curr->next;
    }
    if (found == 0) {
        printf("%s[INFO] Tidak ditemukan transaksi yang cocok dengan '%s'.%s\n",
               CLR_YELLOW, keyword, CLR_RESET);
    } else {
        printf("%sTotal ditemukan: %d transaksi.%s\n", CLR_BGREEN, found, CLR_RESET);
    }
}

static void cariKorbanLinear(const SistemDisasterCare *sys) {
    char keyword[64];
    if (!inputStringSafe("\nKata kunci Kode / Posko / Nama KK: ", keyword, sizeof(keyword), 0)) {
        return;
    }
    int found = 0;
    for (int i = 0; i < sys->jumlahKorban; i++) {
        const Korban *k = &sys->korban[i];
        if (containsIgnoreCase(k->kodeKorban, keyword) ||
            containsIgnoreCase(k->kodePosko, keyword) ||
            containsIgnoreCase(k->namaKepalaKeluarga, keyword)) {
            if (found == 0) {
                printf("\n%-6s | %-6s | %-24s | %4s | %5s | %6s\n",
                       "Kode", "Posko", "Nama KK / Kelompok", "KK", "Jiwa", "Rentan");
                printf("-------+--------+--------------------------+------+-------+-------\n");
            }
            printf("%-6s | %-6s | %-24.24s | %4d | %5d | %6d\n",
                   k->kodeKorban, k->kodePosko, k->namaKepalaKeluarga,
                   k->jumlahKK, k->jumlahAnggota, k->totalRentan);
            found++;
        }
    }
    if (found == 0) {
        printf("%s[INFO] Tidak ditemukan data korban yang cocok dengan '%s'.%s\n",
               CLR_YELLOW, keyword, CLR_RESET);
    } else {
        printf("%sTotal ditemukan: %d catatan korban.%s\n", CLR_BGREEN, found, CLR_RESET);
    }
}

void menuPencarianData(const SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F10 — PENCARIAN DATA (LINEAR SEARCH CASE-INSENSITIVE)");
        printf(" 1. Cari Posko (Kode / Nama Desa)\n");
        printf(" 2. Cari Bantuan Logistik (Kode / Nama / Kategori)\n");
        printf(" 3. Cari Riwayat Transaksi (Kode / Posko / Bantuan / Tanggal)\n");
        printf(" 4. Cari Data Korban (Kode / Posko / Nama KK)\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-4]: ", 0, 4, &sub)) {
            return;
        }
        switch (sub) {
            case 1:
                cariPoskoLinear(sys);
                pauseScreen();
                break;
            case 2:
                cariBantuanLinear(sys);
                pauseScreen();
                break;
            case 3:
                cariTransaksiLinear(sys);
                pauseScreen();
                break;
            case 4:
                cariKorbanLinear(sys);
                pauseScreen();
                break;
            default:
                break;
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F11 — MENU 10: PENGURUTAN DATA POSKO (SELECTION & INSERTION SORT)
 * ========================================================================== */

void menuPengurutanPosko(const SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F11 — PENGURUTAN DATA POSKO (DESCENDING)");
        printf(" 1. Urutkan berdasarkan Jumlah Pengungsi Terbanyak (Selection Sort)\n");
        printf(" 2. Urutkan berdasarkan Skor Prioritas Tertinggi   (Insertion Sort)\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-2]: ", 0, 2, &sub)) {
            return;
        }
        if (sub == 1 || sub == 2) {
            Posko salinan[MAX_POSKO];
            memset(salinan, 0, sizeof(salinan));
            int count = sys->jumlahPosko;
            if (count > MAX_POSKO) {
                count = MAX_POSKO;
            }
            for (int i = 0; i < count; i++) {
                salinan[i] = sys->posko[i];
            }
            if (sub == 1) {
                sortPoskoByPengungsi(salinan, count);
                printf("\n%s[HASIL PENGURUTAN — PENGUNGSI TERBANYAK (Selection Sort)]%s\n",
                       CLR_BCYAN, CLR_RESET);
            } else {
                sortPoskoByPrioritas(salinan, count);
                printf("\n%s[HASIL PENGURUTAN — SKOR PRIORITAS TERTINGGI (Insertion Sort)]%s\n",
                       CLR_BCYAN, CLR_RESET);
            }
            printf("%-4s | %-6s | %-18s | %5s | %6s | %6s | %9s | %-13s\n",
                   "Rank", "Kode", "Nama Desa / Posko", "KK", "Jiwa", "Rentan", "Mkn(Hari)", "Prioritas");
            printf("-----+--------+--------------------+-------+--------+--------+-----------+--------------\n");
            for (int i = 0; i < count; i++) {
                const Posko *p = &salinan[i];
                printf(" #%-2d | %-6s | %-18.18s | %5d | %6d | %6d | %9.1f | %s%d (%-6s)%s\n",
                       i + 1, p->kodePosko, p->namaDesa, p->jumlahKK, p->jumlahJiwa,
                       p->kelompokRentan, p->hariBertahanMakanan,
                       getWarnaPrioritas(p->skorPrioritas), p->skorPrioritas,
                       getKategoriPrioritas(p->skorPrioritas), CLR_RESET);
            }
            pauseScreen();
        }
    } while (sub != 0);
}

/* ==========================================================================
 * F12 — MENU 11: RIWAYAT TRANSAKSI LOGISTIK (DYNAMIC SINGLY LINKED LIST)
 * ========================================================================== */

void menuRiwayatTransaksi(const SistemDisasterCare *sys) {
    clearScreen();
    printHeaderSection("F12 — RIWAYAT TRANSAKSI LOGISTIK (KRONOLOGIS)");

    if (sys->riwayatTransaksi.head == NULL) {
        printf("%s[INFO] Belum ada riwayat transaksi masuk ataupun distribusi.%s\n",
               CLR_YELLOW, CLR_RESET);
        pauseScreen();
        return;
    }

    printf("%-8s | %-10s | %-6s | %-6s | %-5s | %-18s | %6s | %-20s\n",
           "KodeTrx", "Tanggal", "Jenis", "Posko", "KodeB", "Nama Bantuan", "Jumlah", "Keterangan");
    printf("---------+------------+--------+--------+-------+--------------------+--------+---------------------\n");

    int countMasuk = 0, countKeluar = 0;
    const NodeTransaksi *curr = sys->riwayatTransaksi.head;
    while (curr != NULL) {
        int bIdx = cariIndexBantuanByKode(sys, curr->kodeBantuan);
        const char *namaB = (bIdx >= 0) ? sys->bantuan[bIdx].namaBantuan : "-";
        const char *jenisStr = (curr->jenisTransaksi == TRX_MASUK) ? "MASUK" : "KELUAR";
        const char *clr = (curr->jenisTransaksi == TRX_MASUK) ? CLR_BGREEN : CLR_BCYAN;
        if (curr->jenisTransaksi == TRX_MASUK) {
            countMasuk++;
        } else {
            countKeluar++;
        }

        printf("%-8s | %-10s | %s%-6s%s | %-6s | %-5s | %-18.18s | %6d | %-20.20s\n",
               curr->kodeTransaksi, curr->tanggal, clr, jenisStr, CLR_RESET,
               curr->kodePosko, curr->kodeBantuan, namaB, curr->jumlah, curr->keterangan);
        curr = curr->next;
    }
    printf("---------+------------+--------+--------+-------+--------------------+--------+---------------------\n");
    printf("Total Riwayat: %d transaksi (%s%d Masuk%s | %s%d Keluar%s)\n",
           sys->riwayatTransaksi.count, CLR_BGREEN, countMasuk, CLR_RESET,
           CLR_BCYAN, countKeluar, CLR_RESET);
    pauseScreen();
}

/* ==========================================================================
 * F13 — MENU 12: SISTEM PERINGATAN STOK MINIMUM & KEDALUWARSA (ALERT SYSTEM)
 * ========================================================================== */

void menuPeringatanStok(const SistemDisasterCare *sys) {
    clearScreen();
    printHeaderSection("F13 — PERINGATAN STOK MINIMUM & KEDALUWARSA (ALERT SYSTEM)");

    printf("%s[1] ALERT STOK KRITIS (Stok Tersedia < Batas Minimum)%s\n", CLR_BRED, CLR_RESET);
    int jumlahKritis = 0;
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        const Bantuan *b = &sys->bantuan[i];
        if (b->stokTersedia < b->stokMinimum) {
            if (jumlahKritis == 0) {
                printf("%-5s | %-26s | %8s | %8s | %10s | %-7s | %-12s\n",
                       "Kode", "Nama Bantuan", "Stok", "Minimum", "Kekurangan", "Satuan", "Status");
                printf("------+----------------------------+----------+----------+------------+---------+-------------\n");
            }
            int kurang = b->stokMinimum - b->stokTersedia;
            printf("%s%-5s | %-26.26s | %8d | %8d | %10d | %-7s | STOK KRITIS!%s\n",
                   CLR_BRED, b->kodeBantuan, b->namaBantuan,
                   b->stokTersedia, b->stokMinimum, kurang, b->satuan, CLR_RESET);
            jumlahKritis++;
        }
    }
    if (jumlahKritis == 0) {
        printf("%s  Tidak ada bantuan dengan stok di bawah batas minimum.%s\n",
               CLR_BGREEN, CLR_RESET);
    } else {
        printf("\n%s  >> SEGERA LAKUKAN PENGADAAN untuk %d jenis bantuan kritis di atas!%s\n",
               CLR_BRED, jumlahKritis, CLR_RESET);
    }

    char today[12];
    getTodayDate(today, sizeof(today));
    printf("\n%s[2] ALERT BARANG KEDALUWARSA (Acuan Tanggal: %s)%s\n",
           CLR_BYELLOW, today, CLR_RESET);
    int jumlahExp = 0;
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        const Bantuan *b = &sys->bantuan[i];
        if (isBarangKedaluwarsa(b, today)) {
            printf("%s  [KEDALUWARSA] %s — %s (Exp: %s | Stok: %d %s — DILARANG DISTRIBUSI)%s\n",
                   CLR_BRED, b->kodeBantuan, b->namaBantuan,
                   b->detail.tanggalKedaluwarsa, b->stokTersedia, b->satuan, CLR_RESET);
            jumlahExp++;
        }
    }
    if (jumlahExp == 0) {
        printf("%s  Tidak ada bantuan pangan yang melewati masa kedaluwarsa.%s\n",
               CLR_BGREEN, CLR_RESET);
    }
    pauseScreen();
}

/* ==========================================================================
 * F14 — MENU 13: LAPORAN REKAPITULASI & STATISTIK BENCANA
 * ========================================================================== */

void menuLaporanRekapitulasi(const SistemDisasterCare *sys) {
    clearScreen();
    printHeaderSection("F14 — LAPORAN REKAPITULASI & STATISTIK BENCANA DISASTERCARE");

    /* Bagian 1: Ringkasan Posko & Pengungsi */
    int poskoAktif = 0, totalKK = 0, totalJiwa = 0, totalRentan = 0;
    int cntTinggi = 0, cntSedang = 0, cntRendah = 0;
    for (int i = 0; i < sys->jumlahPosko; i++) {
        const Posko *p = &sys->posko[i];
        if (p->statusAktif) {
            poskoAktif++;
        }
        totalKK += p->jumlahKK;
        totalJiwa += p->jumlahJiwa;
        totalRentan += p->kelompokRentan;
        if (p->skorPrioritas >= 7) {
            cntTinggi++;
        } else if (p->skorPrioritas >= 5) {
            cntSedang++;
        } else {
            cntRendah++;
        }
    }

    printf("%s[1] RINGKASAN KONDISI PENGUNGSI & POSKO%s\n", CLR_BCYAN, CLR_RESET);
    printf("  Jumlah Posko Terdaftar : %d Posko (%d Aktif, %d Nonaktif)\n",
           sys->jumlahPosko, poskoAktif, sys->jumlahPosko - poskoAktif);
    printf("  Total Terdampak        : %d KK | %d Jiwa | %d Kelompok Rentan\n",
           totalKK, totalJiwa, totalRentan);
    printf("  Sebaran Prioritas      : %s%d TINGGI%s | %s%d SEDANG%s | %s%d RENDAH%s\n",
           CLR_BRED, cntTinggi, CLR_RESET,
           CLR_BYELLOW, cntSedang, CLR_RESET,
           CLR_BGREEN, cntRendah, CLR_RESET);

    /* Bagian 2: Rekapitulasi Logistik (Total Masuk, Total Keluar, Sisa Stok) */
    printf("\n%s[2] REKAPITULASI ARUS LOGISTIK PER JENIS BANTUAN%s\n", CLR_BCYAN, CLR_RESET);
    printf("%-5s | %-24s | %-6s | %9s | %10s | %9s | %-8s\n",
           "Kode", "Nama Bantuan", "Satuan", "Tot.Masuk", "Tot.Keluar", "Sisa Stok", "Status");
    printf("------+--------------------------+--------+-----------+------------+-----------+---------\n");

    int grandMasuk = 0, grandKeluar = 0, grandSisa = 0;
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        const Bantuan *b = &sys->bantuan[i];
        int masukItem = 0, keluarItem = 0;
        const NodeTransaksi *curr = sys->riwayatTransaksi.head;
        while (curr != NULL) {
            if (equalsIgnoreCase(curr->kodeBantuan, b->kodeBantuan)) {
                if (curr->jenisTransaksi == TRX_MASUK) {
                    masukItem += curr->jumlah;
                } else if (curr->jenisTransaksi == TRX_KELUAR) {
                    keluarItem += curr->jumlah;
                }
            }
            curr = curr->next;
        }
        grandMasuk += masukItem;
        grandKeluar += keluarItem;
        grandSisa += b->stokTersedia;

        const char *st = (b->stokTersedia < b->stokMinimum) ? "KRITIS!" : "AMAN";
        const char *clr = (b->stokTersedia < b->stokMinimum) ? CLR_BRED : CLR_BGREEN;
        printf("%-5s | %-24.24s | %-6s | %9d | %10d | %9d | %s%-8s%s\n",
               b->kodeBantuan, b->namaBantuan, b->satuan,
               masukItem, keluarItem, b->stokTersedia, clr, st, CLR_RESET);
    }
    printf("------+--------------------------+--------+-----------+------------+-----------+---------\n");
    printf("%-5s | %-24s | %-6s | %9d | %10d | %9d | %-8s\n",
           "TOTAL", "Akumulasi Semua Bantuan", "unit",
           grandMasuk, grandKeluar, grandSisa, "-");

    /* Bagian 3: Rekapitulasi Distribusi per Posko */
    printf("\n%s[3] REKAPITULASI DISTRIBUSI BANTUAN PER POSKO%s\n", CLR_BCYAN, CLR_RESET);
    printf("%-6s | %-18s | %8s | %12s | %-11s\n",
           "Kode", "Nama Desa / Posko", "Jml Trx", "Total Unit", "Prioritas");
    printf("-------+--------------------+----------+--------------+------------\n");

    int grandTrxPosko = 0, grandUnitPosko = 0;
    for (int i = 0; i < sys->jumlahPosko; i++) {
        const Posko *p = &sys->posko[i];
        int trxPosko = 0, unitPosko = 0;
        const NodeTransaksi *curr = sys->riwayatTransaksi.head;
        while (curr != NULL) {
            if (curr->jenisTransaksi == TRX_KELUAR &&
                equalsIgnoreCase(curr->kodePosko, p->kodePosko)) {
                trxPosko++;
                unitPosko += curr->jumlah;
            }
            curr = curr->next;
        }
        grandTrxPosko += trxPosko;
        grandUnitPosko += unitPosko;
        printf("%-6s | %-18.18s | %8d | %12d | %s%d (%-6s)%s\n",
               p->kodePosko, p->namaDesa, trxPosko, unitPosko,
               getWarnaPrioritas(p->skorPrioritas), p->skorPrioritas,
               getKategoriPrioritas(p->skorPrioritas), CLR_RESET);
    }
    printf("-------+--------------------+----------+--------------+------------\n");
    printf("%-6s | %-18s | %8d | %12d | %-11s\n",
           "TOTAL", "Total Distribusi", grandTrxPosko, grandUnitPosko, "-");

    /* Bagian 4: Ringkasan Pemenuhan Kebutuhan */
    int totMinta = 0, totReal = 0, totSisa = 0;
    for (int i = 0; i < sys->jumlahKebutuhan; i++) {
        totMinta += sys->kebutuhan[i].jumlahPermintaan;
        totReal += sys->kebutuhan[i].jumlahRealisasi;
        int sisa = sys->kebutuhan[i].jumlahPermintaan - sys->kebutuhan[i].jumlahRealisasi;
        if (sisa > 0) {
            totSisa += sisa;
        }
    }
    printf("\n%s[4] RINGKASAN PEMENUHAN KEBUTUHAN POSKO%s\n", CLR_BCYAN, CLR_RESET);
    printf("  Total Permintaan: %d unit | Total Realisasi: %d unit | Sisa Kebutuhan: %d unit\n",
           totMinta, totReal, totSisa);

    pauseScreen();
}

/* ==========================================================================
 * F15 — MENU 14: SIMPAN & MUAT ULANG DATA (FILE I/O)
 * ========================================================================== */

void menuFileIO(SistemDisasterCare *sys) {
    int sub = -1;
    do {
        clearScreen();
        printHeaderSection("F15 — MANAJEMEN PERSISTENSI DATA (FILE I/O DELIMITER '|')");
        printf(" Lokasi penyimpanan: %s/ (*.dat)\n\n", DATA_DIR);
        printf(" 1. Simpan Seluruh Data ke File Sekarang\n");
        printf(" 2. Muat Ulang Seluruh Data dari File\n");
        printf(" 3. Reset Semua Data ke Data Seed Awal\n");
        printf(" 0. Kembali ke Menu Utama\n");
        if (!inputIntRange("\nPilih Submenu [0-3]: ", 0, 3, &sub)) {
            return;
        }
        switch (sub) {
            case 1:
                if (saveAllData(sys)) {
                    printf("%s[SUKSES] Seluruh data (Posko, Bantuan, Korban, Kebutuhan, Transaksi) tersimpan ke '%s/'!%s\n",
                           CLR_BGREEN, DATA_DIR, CLR_RESET);
                } else {
                    printf("%s[ERROR] Terjadi kegagalan saat menyimpan file data!%s\n",
                           CLR_BRED, CLR_RESET);
                }
                pauseScreen();
                break;
            case 2:
                (void)loadAllData(sys);
                pauseScreen();
                break;
            case 3: {
                int konfirm = 0;
                if (inputIntRange("Yakin reset ke data seed awal? (1=Ya, 0=Batal): ",
                                  0, 1, &konfirm) && konfirm == 1) {
                    seedInitialData(sys);
                    (void)saveAllData(sys);
                    printf("%s[SUKSES] Data berhasil direset ke kondisi seed awal & disimpan!%s\n",
                           CLR_BGREEN, CLR_RESET);
                }
                pauseScreen();
                break;
            }
            default:
                break;
        }
    } while (sub != 0);
}

/* ==========================================================================
 * TAMPILAN MENU UTAMA (14 MENU SESUAI SPESIFIKASI)
 * ========================================================================== */

void tampilkanMenuUtama(const SistemDisasterCare *sys) {
    int kritis = 0;
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        if (sys->bantuan[i].stokTersedia < sys->bantuan[i].stokMinimum) {
            kritis++;
        }
    }

    printf("\n%s===========================================================%s\n",
           CLR_BCYAN, CLR_RESET);
    printf("              %sDISASTERCARE LOGISTICS SYSTEM%s\n", CLR_BWHITE, CLR_RESET);
    printf("         Sistem Manajemen Bantuan Korban Bencana\n");
    printf("%s===========================================================%s\n",
           CLR_BCYAN, CLR_RESET);
    printf(" %sStatus:%s %d Posko | %d Bantuan | %d Transaksi",
           CLR_DIM, CLR_RESET, sys->jumlahPosko, sys->jumlahBantuan, sys->riwayatTransaksi.count);
    if (kritis > 0) {
        printf(" | %s[ALERT: %d Stok Kritis]%s", CLR_BRED, kritis, CLR_RESET);
    }
    printf("\n%s-----------------------------------------------------------%s\n",
           CLR_DIM, CLR_RESET);
    printf("  1. Kelola Data Posko Bencana\n");
    printf("  2. Catat Data Korban & Kelompok Rentan\n");
    printf("  3. Kelola Master Jenis Bantuan Logistik\n");
    printf("  4. Catat Bantuan Masuk (Penerimaan)\n");
    printf("  5. Monitoring Stok Logistik Real-Time\n");
    printf("  6. Catat Kebutuhan Posko Desa\n");
    printf("  7. Hitung & Tampilkan Prioritas Posko\n");
    printf("  8. Proses Distribusi Bantuan ke Posko\n");
    printf("  9. Pencarian Data (Posko / Bantuan / Transaksi)\n");
    printf(" 10. Pengurutan Data Posko (Pengungsi / Prioritas)\n");
    printf(" 11. Lihat Riwayat Transaksi Logistik\n");
    printf(" 12. Peringatan Stok Minimum (Alert System)\n");
    printf(" 13. Laporan Rekapitulasi & Statistik Bencana\n");
    printf(" 14. Simpan & Muat Ulang Data (File I/O)\n");
    printf("  0. Keluar dari Aplikasi\n");
    printf("%s===========================================================%s\n",
           CLR_BCYAN, CLR_RESET);
}
