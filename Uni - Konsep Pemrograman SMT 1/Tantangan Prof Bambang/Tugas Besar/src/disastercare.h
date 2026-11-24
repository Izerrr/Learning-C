#ifndef DISASTERCARE_H
#define DISASTERCARE_H

#include <stddef.h>
#include <stdio.h>

/* ==========================================================================
 * KONSTANTA KAPASITAS & LOKASI FILE DATA
 * ========================================================================== */
#define MAX_POSKO       100
#define MAX_BANTUAN     100
#define MAX_KORBAN      500
#define MAX_KEBUTUHAN   300
#define LINE_BUF_SIZE   512

#define DATA_DIR        "test"
#define FILE_POSKO      "test/posko.dat"
#define FILE_BANTUAN    "test/bantuan.dat"
#define FILE_KORBAN     "test/korban.dat"
#define FILE_KEBUTUHAN  "test/kebutuhan.dat"
#define FILE_TRANSAKSI  "test/transaksi.dat"

/* Jenis Transaksi Logistik */
#define TRX_MASUK       1
#define TRX_KELUAR      2

/* ==========================================================================
 * KODE WARNA ANSI (CROSS-PLATFORM TERMINAL CLI)
 * ========================================================================== */
#define CLR_RESET       "\033[0m"
#define CLR_BOLD        "\033[1m"
#define CLR_DIM         "\033[2m"
#define CLR_RED         "\033[31m"
#define CLR_GREEN       "\033[32m"
#define CLR_YELLOW      "\033[33m"
#define CLR_BLUE        "\033[34m"
#define CLR_MAGENTA     "\033[35m"
#define CLR_CYAN        "\033[36m"
#define CLR_WHITE       "\033[37m"
#define CLR_BRED        "\033[1;31m"
#define CLR_BGREEN      "\033[1;32m"
#define CLR_BYELLOW     "\033[1;33m"
#define CLR_BCYAN       "\033[1;36m"
#define CLR_BWHITE      "\033[1;37m"

/* ==========================================================================
 * ENUM & TAGGED UNION DETAIL BANTUAN LOGISTIK
 * ========================================================================== */
typedef enum {
    DETAIL_PAKAIAN     = 1,
    DETAIL_KEDALUWARSA = 2,
    DETAIL_HYGIENE     = 3
} JenisDetailBantuan;

typedef union {
    char ukuranPakaian[16];      /* Misal: "Dewasa-XL", "Anak-M", "All-Size" */
    char tanggalKedaluwarsa[12]; /* Format: "DD-MM-YYYY" */
    char kategoriKit[32];        /* Misal: "Sanitasi Mandi", "Perawatan Bayi" */
} DetailBantuan;

/* ==========================================================================
 * STRUCT ENTITAS UTAMA SISTEM DISASTERCARE
 * ========================================================================== */

/* Struct Master Bantuan Logistik (dengan Tagged Union) */
typedef struct {
    char kodeBantuan[12];
    char namaBantuan[60];
    char satuan[20];
    char kategoriLogistik[32];
    int stokTersedia;
    int stokMinimum;
    JenisDetailBantuan jenisDetail;
    DetailBantuan detail;
} Bantuan;

/* Struct Master Posko Desa (hariBertahanMakanan bertipe float sesuai U08) */
typedef struct {
    char kodePosko[12];
    char namaDesa[60];
    int jumlahKK;
    int jumlahJiwa;
    int kelompokRentan;          /* Lansia + Balita + Ibu Hamil */
    float hariBertahanMakanan;   /* Mendukung desimal seperti 0.5 hari */
    int skorPrioritas;           /* S = P + M + R (rentang 3..9) */
    int statusAktif;             /* 1 = Aktif, 0 = Nonaktif */
} Posko;

/* Struct Pencatatan Data Korban & Kelompok Rentan (F02) */
typedef struct {
    char kodeKorban[12];
    char kodePosko[12];
    char namaKepalaKeluarga[60];
    int jumlahKK;
    int jumlahAnggota;
    int jumlahLansia;
    int jumlahBalita;
    int jumlahIbuHamil;
    int totalRentan;
} Korban;

/* Struct Pencatatan Kebutuhan Spesifik Posko Desa (F06) */
typedef struct {
    char kodeKebutuhan[12];
    char kodePosko[12];
    char kodeBantuan[12];
    int jumlahPermintaan;
    int jumlahRealisasi;
    char tanggalPermintaan[12];  /* Format: "DD-MM-YYYY" */
} Kebutuhan;

/* Node Dynamic Singly Linked List Riwayat Transaksi */
typedef struct NodeTransaksi {
    char kodeTransaksi[16];
    char tanggal[12];            /* Format: "DD-MM-YYYY" */
    char kodePosko[12];          /* "INDUK" jika Penerimaan, Kode Posko jika Distribusi */
    char kodeBantuan[12];
    char keterangan[64];         /* Nama Donor / Catatan Distribusi */
    int jenisTransaksi;          /* 1 = Masuk (Penerimaan), 2 = Keluar (Distribusi) */
    int jumlah;
    struct NodeTransaksi *next;
} NodeTransaksi;

/* Wrapper Dynamic Singly Linked List dengan Head & Tail */
typedef struct {
    NodeTransaksi *head;
    NodeTransaksi *tail;
    int count;
} ListTransaksi;

/* State Terintegrasi Aplikasi DisasterCare */
typedef struct {
    Posko posko[MAX_POSKO];
    int jumlahPosko;

    Bantuan bantuan[MAX_BANTUAN];
    int jumlahBantuan;

    Korban korban[MAX_KORBAN];
    int jumlahKorban;

    Kebutuhan kebutuhan[MAX_KEBUTUHAN];
    int jumlahKebutuhan;

    ListTransaksi riwayatTransaksi;
} SistemDisasterCare;

/* ==========================================================================
 * PROTOTIPE FUNGSI — STORAGE, UTILITAS I/O AMAN & LINKED LIST (storage.c)
 * ========================================================================== */
void initTerminalColors(void);
void clearScreen(void);
void pauseScreen(void);

void trimNewline(char *str);
void trimWhitespace(char *str);
void sanitizePipe(char *str);
int equalsIgnoreCase(const char *a, const char *b);
int containsIgnoreCase(const char *haystack, const char *needle);

int readLineSafe(char *buf, size_t size);
int inputIntRange(const char *prompt, int minVal, int maxVal, int *outVal);
int inputPositiveInt(const char *prompt, int *outVal);
int inputFloatRange(const char *prompt, float minVal, float maxVal, float *outVal);
int inputStringSafe(const char *prompt, char *dest, size_t size, int allowEmpty);
int inputTanggalSafe(const char *prompt, char *dest, size_t size, int allowTodayDefault);

int parseDateToInt(const char *tanggal);
void getTodayDate(char *buf, size_t size);

void initListTransaksi(ListTransaksi *list);
int appendTransaksi(ListTransaksi *list, const NodeTransaksi *data);
void freeListTransaksi(ListTransaksi *list);

int ensureDataDirectory(void);
int splitPipeLine(char *line, char *fields[], int maxFields);
void seedInitialData(SistemDisasterCare *sys);
int saveAllData(const SistemDisasterCare *sys);
int loadAllData(SistemDisasterCare *sys);

/* ==========================================================================
 * PROTOTIPE FUNGSI — LOGIKA BISNIS & FITUR F01-F15 (operations.c)
 * ========================================================================== */
int tambahStokBantuan(int *stokCurrent, int jumlahMasuk);
int kurangiStokBantuan(int *stokCurrent, int jumlahKeluar);
int isBarangKedaluwarsa(const Bantuan *b, const char *tanggalDistribusi);

int hitungSkorPengungsi(int jumlahJiwa);
int hitungSkorMakanan(float hariBertahan);
int hitungSkorRentan(int kelompokRentan);
int hitungSkorPrioritasPosko(Posko *posko);
const char *getKategoriPrioritas(int skor);
const char *getWarnaPrioritas(int skor);
void formatDetailBantuan(const Bantuan *b, char *buf, size_t size);

int cariIndexPoskoByKode(const SistemDisasterCare *sys, const char *kode);
int cariIndexBantuanByKode(const SistemDisasterCare *sys, const char *kode);
void generateNextKode(const char *prefix, int nextNum, int digits, char *out, size_t size);
void updateRealisasiKebutuhan(SistemDisasterCare *sys, const char *kodePosko,
                              const char *kodeBantuan, int jumlah);

void sortPoskoByPengungsi(Posko arr[], int n);
void sortPoskoByPrioritas(Posko arr[], int n);

void tampilkanTabelPosko(const Posko daftar[], int count);
void tampilkanDetailPosko(const SistemDisasterCare *sys, const Posko *p);
void tampilkanTabelBantuan(const Bantuan daftar[], int count);

void tampilkanMenuUtama(const SistemDisasterCare *sys);
void menuKelolaPosko(SistemDisasterCare *sys);
void menuCatatKorban(SistemDisasterCare *sys);
void menuKelolaBantuan(SistemDisasterCare *sys);
void menuPenerimaanBantuan(SistemDisasterCare *sys);
void menuMonitoringStok(const SistemDisasterCare *sys);
void menuKelolaKebutuhan(SistemDisasterCare *sys);
void menuPrioritasPosko(SistemDisasterCare *sys);
void menuDistribusiBantuan(SistemDisasterCare *sys);
void menuPencarianData(const SistemDisasterCare *sys);
void menuPengurutanPosko(const SistemDisasterCare *sys);
void menuRiwayatTransaksi(const SistemDisasterCare *sys);
void menuPeringatanStok(const SistemDisasterCare *sys);
void menuLaporanRekapitulasi(const SistemDisasterCare *sys);
void menuFileIO(SistemDisasterCare *sys);

#endif /* DISASTERCARE_H */
