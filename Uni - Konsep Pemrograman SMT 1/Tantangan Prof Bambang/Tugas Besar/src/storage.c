#include "disastercare.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/* ==========================================================================
 * UTILITAS TERMINAL CROSS-PLATFORM
 * ========================================================================== */

void initTerminalColors(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    SetConsoleOutputCP(CP_UTF8);
#endif
}

void clearScreen(void) {
#ifdef _WIN32
    if (_isatty(_fileno(stdout))) {
        (void)system("cls");
    }
#else
    if (isatty(STDOUT_FILENO)) {
        (void)system("clear");
    }
#endif
}

void pauseScreen(void) {
    char dummy[16];
    printf("\n%sTekan [ENTER] untuk melanjutkan...%s", CLR_DIM, CLR_RESET);
    fflush(stdout);
    (void)readLineSafe(dummy, sizeof(dummy));
}

/* ==========================================================================
 * UTILITAS STRING & SANITASI INPUT
 * ========================================================================== */

void trimNewline(char *str) {
    if (str == NULL) {
        return;
    }
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

void trimWhitespace(char *str) {
    if (str == NULL) {
        return;
    }
    trimNewline(str);
    char *start = str;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
}

void sanitizePipe(char *str) {
    if (str == NULL) {
        return;
    }
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|') {
            str[i] = '/';
        }
    }
}

int equalsIgnoreCase(const char *a, const char *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

int containsIgnoreCase(const char *haystack, const char *needle) {
    if (haystack == NULL || needle == NULL) {
        return 0;
    }
    size_t needleLen = strlen(needle);
    if (needleLen == 0) {
        return 1;
    }
    for (size_t i = 0; haystack[i] != '\0'; i++) {
        size_t j = 0;
        while (j < needleLen && haystack[i + j] != '\0' &&
               tolower((unsigned char)haystack[i + j]) ==
                   tolower((unsigned char)needle[j])) {
            j++;
        }
        if (j == needleLen) {
            return 1;
        }
    }
    return 0;
}

/* ==========================================================================
 * HELPER INPUT AMAN (FGETS + SSCANF, ANTI INFINITE LOOP)
 * ========================================================================== */

int readLineSafe(char *buf, size_t size) {
    if (buf == NULL || size == 0) {
        return 0;
    }
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return 0;
    }
    if (strchr(buf, '\n') == NULL) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Buang sisa karakter yang melebihi kapasitas buffer */
        }
    }
    trimNewline(buf);
    return 1;
}

int inputIntRange(const char *prompt, int minVal, int maxVal, int *outVal) {
    char buf[128];
    int val = 0;
    char extra = '\0';

    if (outVal == NULL) {
        return 0;
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLineSafe(buf, sizeof(buf))) {
            return 0; /* EOF */
        }
        trimWhitespace(buf);
        if (sscanf(buf, "%d %c", &val, &extra) != 1) {
            printf("%s[ERROR] Input harus berupa bilangan bulat yang valid!%s\n",
                   CLR_BRED, CLR_RESET);
            continue;
        }
        if (val < minVal || val > maxVal) {
            printf("%s[ERROR] Nilai harus berada dalam rentang %d s.d. %d!%s\n",
                   CLR_BRED, minVal, maxVal, CLR_RESET);
            continue;
        }
        *outVal = val;
        return 1;
    }
}

int inputPositiveInt(const char *prompt, int *outVal) {
    char buf[128];
    int val = 0;
    char extra = '\0';

    if (outVal == NULL) {
        return 0;
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLineSafe(buf, sizeof(buf))) {
            return 0;
        }
        trimWhitespace(buf);
        if (equalsIgnoreCase(buf, "batal") || equalsIgnoreCase(buf, "q")) {
            return 0;
        }
        if (sscanf(buf, "%d %c", &val, &extra) != 1) {
            printf("%s[ERROR] Input tidak valid! Masukkan angka positif (> 0) atau 'batal'.%s\n",
                   CLR_BRED, CLR_RESET);
            continue;
        }
        if (val <= 0) {
            printf("%s[ERROR] Jumlah tidak boleh nol atau negatif (%d)! Masukkan angka positif (> 0).%s\n",
                   CLR_BRED, val, CLR_RESET);
            continue;
        }
        *outVal = val;
        return 1;
    }
}

int inputFloatRange(const char *prompt, float minVal, float maxVal, float *outVal) {
    char buf[128];
    float val = 0.0f;
    char extra = '\0';

    if (outVal == NULL) {
        return 0;
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLineSafe(buf, sizeof(buf))) {
            return 0;
        }
        trimWhitespace(buf);
        if (sscanf(buf, "%f %c", &val, &extra) != 1) {
            printf("%s[ERROR] Input harus berupa angka (boleh desimal, misal 0.5)!%s\n",
                   CLR_BRED, CLR_RESET);
            continue;
        }
        if (val < minVal || val > maxVal) {
            printf("%s[ERROR] Nilai harus dalam rentang %.1f s.d. %.1f!%s\n",
                   CLR_BRED, minVal, maxVal, CLR_RESET);
            continue;
        }
        *outVal = val;
        return 1;
    }
}

int inputStringSafe(const char *prompt, char *dest, size_t size, int allowEmpty) {
    char buf[LINE_BUF_SIZE];
    if (dest == NULL || size == 0) {
        return 0;
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLineSafe(buf, sizeof(buf))) {
            return 0;
        }
        trimWhitespace(buf);
        sanitizePipe(buf);
        if (!allowEmpty && buf[0] == '\0') {
            printf("%s[ERROR] Input tidak boleh kosong!%s\n", CLR_BRED, CLR_RESET);
            continue;
        }
        snprintf(dest, size, "%s", buf);
        return 1;
    }
}

/* ==========================================================================
 * PARSER & VALIDATOR TANGGAL (DD-MM-YYYY -> YYYYMMDD)
 * ========================================================================== */

static int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int parseDateToInt(const char *tanggal) {
    if (tanggal == NULL || strlen(tanggal) != 10) {
        return -1;
    }
    if (tanggal[2] != '-' || tanggal[5] != '-') {
        return -1;
    }
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            continue;
        }
        if (!isdigit((unsigned char)tanggal[i])) {
            return -1;
        }
    }
    int day = (tanggal[0] - '0') * 10 + (tanggal[1] - '0');
    int month = (tanggal[3] - '0') * 10 + (tanggal[4] - '0');
    int year = (tanggal[6] - '0') * 1000 + (tanggal[7] - '0') * 100 +
               (tanggal[8] - '0') * 10 + (tanggal[9] - '0');

    if (year < 1900 || year > 2100 || month < 1 || month > 12) {
        return -1;
    }
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && isLeapYear(year)) {
        daysInMonth[1] = 29;
    }
    if (day < 1 || day > daysInMonth[month - 1]) {
        return -1;
    }
    return year * 10000 + month * 100 + day;
}

void getTodayDate(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *tmNow = localtime(&now);
    if (tmNow != NULL) {
        snprintf(buf, size, "%02d-%02d-%04d",
                 tmNow->tm_mday, tmNow->tm_mon + 1, tmNow->tm_year + 1900);
    } else {
        snprintf(buf, size, "01-01-2026");
    }
}

int inputTanggalSafe(const char *prompt, char *dest, size_t size, int allowTodayDefault) {
    char buf[64];
    if (dest == NULL || size < 11) {
        return 0;
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLineSafe(buf, sizeof(buf))) {
            return 0;
        }
        trimWhitespace(buf);
        if (allowTodayDefault && buf[0] == '\0') {
            getTodayDate(dest, size);
            return 1;
        }
        if (parseDateToInt(buf) < 0) {
            printf("%s[ERROR] Format tanggal tidak valid! Gunakan DD-MM-YYYY (contoh: 24-11-2026).%s\n",
                   CLR_BRED, CLR_RESET);
            continue;
        }
        snprintf(dest, size, "%s", buf);
        return 1;
    }
}

/* ==========================================================================
 * DYNAMIC SINGLY LINKED LIST RIWAYAT TRANSAKSI (MALLOC & FREE)
 * ========================================================================== */

void initListTransaksi(ListTransaksi *list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

int appendTransaksi(ListTransaksi *list, const NodeTransaksi *data) {
    if (list == NULL || data == NULL) {
        return 0;
    }
    NodeTransaksi *node = (NodeTransaksi *)malloc(sizeof(NodeTransaksi));
    if (node == NULL) {
        printf("%s[ERROR] Gagal mengalokasikan memori untuk transaksi baru!%s\n",
               CLR_BRED, CLR_RESET);
        return 0;
    }
    *node = *data;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->count++;
    return 1;
}

void freeListTransaksi(ListTransaksi *list) {
    if (list == NULL) {
        return;
    }
    NodeTransaksi *curr = list->head;
    while (curr != NULL) {
        NodeTransaksi *nextNode = curr->next;
        free(curr);
        curr = nextNode;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

/* ==========================================================================
 * PERSISTENSI FILE I/O DELIMITER PIPE '|' & AUTO-SEED DATA
 * ========================================================================== */

int ensureDataDirectory(void) {
#ifdef _WIN32
    if (_mkdir(DATA_DIR) == 0 || errno == EEXIST) {
        return 1;
    }
#else
    if (mkdir(DATA_DIR, 0755) == 0 || errno == EEXIST) {
        return 1;
    }
#endif
    return 0;
}

int splitPipeLine(char *line, char *fields[], int maxFields) {
    if (line == NULL || fields == NULL || maxFields <= 0) {
        return 0;
    }
    int count = 0;
    char *p = line;
    fields[count++] = p;
    while (*p != '\0' && count < maxFields) {
        if (*p == '|') {
            *p = '\0';
            fields[count++] = p + 1;
        }
        p++;
    }
    return count;
}

static int fileExists(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp != NULL) {
        fclose(fp);
        return 1;
    }
    return 0;
}

static int commitTempFile(const char *tmpPath, const char *targetPath) {
    (void)remove(targetPath);
    if (rename(tmpPath, targetPath) != 0) {
        return 0;
    }
    return 1;
}

void seedInitialData(SistemDisasterCare *sys) {
    if (sys == NULL) {
        return;
    }
    freeListTransaksi(&sys->riwayatTransaksi);
    memset(sys, 0, sizeof(*sys));
    initListTransaksi(&sys->riwayatTransaksi);

    /* 5 Posko Wilayah Terdampak: Total 350 KK, 1.200 Jiwa */
    const Posko seedPosko[5] = {
        { "P001", "Desa Makmur",    80, 280, 45, 2.0f, 0, 1 },
        { "P002", "Desa Sentosa",   65, 220, 30, 2.5f, 0, 1 },
        { "P003", "Desa Harapan",   90, 310, 65, 0.5f, 0, 1 }, /* U08: P=3, M=3, R=3 -> S=9 TINGGI */
        { "P004", "Desa Sukamaju",  55, 180, 18, 4.0f, 0, 1 },
        { "P005", "Desa Mulyasari", 60, 210, 25, 3.0f, 0, 1 }
    };
    sys->jumlahPosko = 5;
    for (int i = 0; i < 5; i++) {
        sys->posko[i] = seedPosko[i];
        (void)hitungSkorPrioritasPosko(&sys->posko[i]);
    }

    /* 8 Master Jenis Bantuan Logistik (dengan Tagged Union terkalibrasi) */
    sys->jumlahBantuan = 8;

    snprintf(sys->bantuan[0].kodeBantuan, sizeof(sys->bantuan[0].kodeBantuan), "B001");
    snprintf(sys->bantuan[0].namaBantuan, sizeof(sys->bantuan[0].namaBantuan), "Beras Pangan Utama");
    snprintf(sys->bantuan[0].satuan, sizeof(sys->bantuan[0].satuan), "kg");
    snprintf(sys->bantuan[0].kategoriLogistik, sizeof(sys->bantuan[0].kategoriLogistik), "Pangan Pokok");
    sys->bantuan[0].stokTersedia = 300; /* U04/U05/U06 */
    sys->bantuan[0].stokMinimum = 100;
    sys->bantuan[0].jenisDetail = DETAIL_KEDALUWARSA;
    snprintf(sys->bantuan[0].detail.tanggalKedaluwarsa,
             sizeof(sys->bantuan[0].detail.tanggalKedaluwarsa), "31-12-2027");

    snprintf(sys->bantuan[1].kodeBantuan, sizeof(sys->bantuan[1].kodeBantuan), "B002");
    snprintf(sys->bantuan[1].namaBantuan, sizeof(sys->bantuan[1].namaBantuan), "Mi Instan Paket");
    snprintf(sys->bantuan[1].satuan, sizeof(sys->bantuan[1].satuan), "dus");
    snprintf(sys->bantuan[1].kategoriLogistik, sizeof(sys->bantuan[1].kategoriLogistik), "Makanan Siap Saji");
    sys->bantuan[1].stokTersedia = 150;
    sys->bantuan[1].stokMinimum = 50;
    sys->bantuan[1].jenisDetail = DETAIL_KEDALUWARSA;
    snprintf(sys->bantuan[1].detail.tanggalKedaluwarsa,
             sizeof(sys->bantuan[1].detail.tanggalKedaluwarsa), "30-06-2027");

    snprintf(sys->bantuan[2].kodeBantuan, sizeof(sys->bantuan[2].kodeBantuan), "B003");
    snprintf(sys->bantuan[2].namaBantuan, sizeof(sys->bantuan[2].namaBantuan), "Air Mineral Kemasan");
    snprintf(sys->bantuan[2].satuan, sizeof(sys->bantuan[2].satuan), "liter");
    snprintf(sys->bantuan[2].kategoriLogistik, sizeof(sys->bantuan[2].kategoriLogistik), "Air & Minuman");
    sys->bantuan[2].stokTersedia = 500;
    sys->bantuan[2].stokMinimum = 150;
    sys->bantuan[2].jenisDetail = DETAIL_KEDALUWARSA;
    snprintf(sys->bantuan[2].detail.tanggalKedaluwarsa,
             sizeof(sys->bantuan[2].detail.tanggalKedaluwarsa), "31-12-2027");

    snprintf(sys->bantuan[3].kodeBantuan, sizeof(sys->bantuan[3].kodeBantuan), "B004");
    snprintf(sys->bantuan[3].namaBantuan, sizeof(sys->bantuan[3].namaBantuan), "Makanan Siap Saji / Kaleng");
    snprintf(sys->bantuan[3].satuan, sizeof(sys->bantuan[3].satuan), "paket");
    snprintf(sys->bantuan[3].kategoriLogistik, sizeof(sys->bantuan[3].kategoriLogistik), "Pangan Olahan");
    sys->bantuan[3].stokTersedia = 80;
    sys->bantuan[3].stokMinimum = 30;
    sys->bantuan[3].jenisDetail = DETAIL_KEDALUWARSA;
    snprintf(sys->bantuan[3].detail.tanggalKedaluwarsa,
             sizeof(sys->bantuan[3].detail.tanggalKedaluwarsa), "01-01-2025"); /* U14: Kedaluwarsa */

    snprintf(sys->bantuan[4].kodeBantuan, sizeof(sys->bantuan[4].kodeBantuan), "B005");
    snprintf(sys->bantuan[4].namaBantuan, sizeof(sys->bantuan[4].namaBantuan), "Selimut & Hangat");
    snprintf(sys->bantuan[4].satuan, sizeof(sys->bantuan[4].satuan), "buah");
    snprintf(sys->bantuan[4].kategoriLogistik, sizeof(sys->bantuan[4].kategoriLogistik), "Perlengkapan Tidur");
    sys->bantuan[4].stokTersedia = 120;
    sys->bantuan[4].stokMinimum = 40;
    sys->bantuan[4].jenisDetail = DETAIL_PAKAIAN;
    snprintf(sys->bantuan[4].detail.ukuranPakaian,
             sizeof(sys->bantuan[4].detail.ukuranPakaian), "All-Size");

    snprintf(sys->bantuan[5].kodeBantuan, sizeof(sys->bantuan[5].kodeBantuan), "B006");
    snprintf(sys->bantuan[5].namaBantuan, sizeof(sys->bantuan[5].namaBantuan), "Pakaian Layak Pakai");
    snprintf(sys->bantuan[5].satuan, sizeof(sys->bantuan[5].satuan), "paket");
    snprintf(sys->bantuan[5].kategoriLogistik, sizeof(sys->bantuan[5].kategoriLogistik), "Sandang Pakaian");
    sys->bantuan[5].stokTersedia = 90;
    sys->bantuan[5].stokMinimum = 30;
    sys->bantuan[5].jenisDetail = DETAIL_PAKAIAN;
    snprintf(sys->bantuan[5].detail.ukuranPakaian,
             sizeof(sys->bantuan[5].detail.ukuranPakaian), "Dewasa-L");

    snprintf(sys->bantuan[6].kodeBantuan, sizeof(sys->bantuan[6].kodeBantuan), "B007");
    snprintf(sys->bantuan[6].namaBantuan, sizeof(sys->bantuan[6].namaBantuan), "Popok Bayi & Balita");
    snprintf(sys->bantuan[6].satuan, sizeof(sys->bantuan[6].satuan), "paket");
    snprintf(sys->bantuan[6].kategoriLogistik, sizeof(sys->bantuan[6].kategoriLogistik), "Perlengkapan Bayi");
    sys->bantuan[6].stokTersedia = 15; /* U11: Stok Kritis (15 < 40) */
    sys->bantuan[6].stokMinimum = 40;
    sys->bantuan[6].jenisDetail = DETAIL_HYGIENE;
    snprintf(sys->bantuan[6].detail.kategoriKit,
             sizeof(sys->bantuan[6].detail.kategoriKit), "Perawatan Bayi");

    snprintf(sys->bantuan[7].kodeBantuan, sizeof(sys->bantuan[7].kodeBantuan), "B008");
    snprintf(sys->bantuan[7].namaBantuan, sizeof(sys->bantuan[7].namaBantuan), "Kit Kebersihan / Hygiene");
    snprintf(sys->bantuan[7].satuan, sizeof(sys->bantuan[7].satuan), "paket");
    snprintf(sys->bantuan[7].kategoriLogistik, sizeof(sys->bantuan[7].kategoriLogistik), "Sanitasi & Mandi");
    sys->bantuan[7].stokTersedia = 75;
    sys->bantuan[7].stokMinimum = 25;
    sys->bantuan[7].jenisDetail = DETAIL_HYGIENE;
    snprintf(sys->bantuan[7].detail.kategoriKit,
             sizeof(sys->bantuan[7].detail.kategoriKit), "Sanitasi Mandi");

    /* Kebutuhan Awal Simulasi (agar F06 & integrasi realisasi F08 langsung terlihat) */
    const Kebutuhan seedKebutuhan[3] = {
        { "REQ001", "P003", "B001", 500, 0, "20-11-2026" },
        { "REQ002", "P003", "B007",  50, 0, "20-11-2026" },
        { "REQ003", "P001", "B003", 200, 0, "20-11-2026" }
    };
    sys->jumlahKebutuhan = 3;
    for (int i = 0; i < 3; i++) {
        sys->kebutuhan[i] = seedKebutuhan[i];
    }

    sys->jumlahKorban = 0;
}

static int savePoskoFile(const SistemDisasterCare *sys) {
    const char *tmpPath = "test/posko.dat.tmp";
    FILE *fp = fopen(tmpPath, "w");
    if (fp == NULL) {
        return 0;
    }
    for (int i = 0; i < sys->jumlahPosko; i++) {
        const Posko *p = &sys->posko[i];
        fprintf(fp, "%s|%s|%d|%d|%d|%.2f|%d|%d\n",
                p->kodePosko, p->namaDesa, p->jumlahKK, p->jumlahJiwa,
                p->kelompokRentan, p->hariBertahanMakanan,
                p->skorPrioritas, p->statusAktif);
    }
    int ok = !ferror(fp);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok ? commitTempFile(tmpPath, FILE_POSKO) : 0;
}

static int saveBantuanFile(const SistemDisasterCare *sys) {
    const char *tmpPath = "test/bantuan.dat.tmp";
    FILE *fp = fopen(tmpPath, "w");
    if (fp == NULL) {
        return 0;
    }
    for (int i = 0; i < sys->jumlahBantuan; i++) {
        const Bantuan *b = &sys->bantuan[i];
        const char *detailVal = "";
        if (b->jenisDetail == DETAIL_PAKAIAN) {
            detailVal = b->detail.ukuranPakaian;
        } else if (b->jenisDetail == DETAIL_KEDALUWARSA) {
            detailVal = b->detail.tanggalKedaluwarsa;
        } else {
            detailVal = b->detail.kategoriKit;
        }
        fprintf(fp, "%s|%s|%s|%s|%d|%d|%d|%s\n",
                b->kodeBantuan, b->namaBantuan, b->satuan, b->kategoriLogistik,
                b->stokTersedia, b->stokMinimum, (int)b->jenisDetail, detailVal);
    }
    int ok = !ferror(fp);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok ? commitTempFile(tmpPath, FILE_BANTUAN) : 0;
}

static int saveKorbanFile(const SistemDisasterCare *sys) {
    const char *tmpPath = "test/korban.dat.tmp";
    FILE *fp = fopen(tmpPath, "w");
    if (fp == NULL) {
        return 0;
    }
    for (int i = 0; i < sys->jumlahKorban; i++) {
        const Korban *k = &sys->korban[i];
        fprintf(fp, "%s|%s|%s|%d|%d|%d|%d|%d|%d\n",
                k->kodeKorban, k->kodePosko, k->namaKepalaKeluarga,
                k->jumlahKK, k->jumlahAnggota,
                k->jumlahLansia, k->jumlahBalita, k->jumlahIbuHamil,
                k->totalRentan);
    }
    int ok = !ferror(fp);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok ? commitTempFile(tmpPath, FILE_KORBAN) : 0;
}

static int saveKebutuhanFile(const SistemDisasterCare *sys) {
    const char *tmpPath = "test/kebutuhan.dat.tmp";
    FILE *fp = fopen(tmpPath, "w");
    if (fp == NULL) {
        return 0;
    }
    for (int i = 0; i < sys->jumlahKebutuhan; i++) {
        const Kebutuhan *kb = &sys->kebutuhan[i];
        fprintf(fp, "%s|%s|%s|%d|%d|%s\n",
                kb->kodeKebutuhan, kb->kodePosko, kb->kodeBantuan,
                kb->jumlahPermintaan, kb->jumlahRealisasi, kb->tanggalPermintaan);
    }
    int ok = !ferror(fp);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok ? commitTempFile(tmpPath, FILE_KEBUTUHAN) : 0;
}

static int saveTransaksiFile(const SistemDisasterCare *sys) {
    const char *tmpPath = "test/transaksi.dat.tmp";
    FILE *fp = fopen(tmpPath, "w");
    if (fp == NULL) {
        return 0;
    }
    const NodeTransaksi *curr = sys->riwayatTransaksi.head;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%d|%d|%s\n",
                curr->kodeTransaksi, curr->tanggal, curr->kodePosko,
                curr->kodeBantuan, curr->jenisTransaksi, curr->jumlah,
                curr->keterangan);
        curr = curr->next;
    }
    int ok = !ferror(fp);
    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok ? commitTempFile(tmpPath, FILE_TRANSAKSI) : 0;
}

int saveAllData(const SistemDisasterCare *sys) {
    if (sys == NULL) {
        return 0;
    }
    if (!ensureDataDirectory()) {
        return 0;
    }
    int ok = 1;
    ok &= savePoskoFile(sys);
    ok &= saveBantuanFile(sys);
    ok &= saveKorbanFile(sys);
    ok &= saveKebutuhanFile(sys);
    ok &= saveTransaksiFile(sys);
    return ok;
}

static int loadPoskoFile(SistemDisasterCare *sys) {
    FILE *fp = fopen(FILE_POSKO, "r");
    if (fp == NULL) {
        return 0;
    }
    char line[LINE_BUF_SIZE];
    sys->jumlahPosko = 0;
    while (fgets(line, sizeof(line), fp) != NULL && sys->jumlahPosko < MAX_POSKO) {
        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }
        char *fields[8];
        if (splitPipeLine(line, fields, 8) == 8) {
            Posko *p = &sys->posko[sys->jumlahPosko];
            snprintf(p->kodePosko, sizeof(p->kodePosko), "%s", fields[0]);
            snprintf(p->namaDesa, sizeof(p->namaDesa), "%s", fields[1]);
            p->jumlahKK = atoi(fields[2]);
            p->jumlahJiwa = atoi(fields[3]);
            p->kelompokRentan = atoi(fields[4]);
            p->hariBertahanMakanan = (float)atof(fields[5]);
            p->statusAktif = atoi(fields[7]);
            (void)hitungSkorPrioritasPosko(p);
            sys->jumlahPosko++;
        }
    }
    fclose(fp);
    return 1;
}

static int loadBantuanFile(SistemDisasterCare *sys) {
    FILE *fp = fopen(FILE_BANTUAN, "r");
    if (fp == NULL) {
        return 0;
    }
    char line[LINE_BUF_SIZE];
    sys->jumlahBantuan = 0;
    while (fgets(line, sizeof(line), fp) != NULL && sys->jumlahBantuan < MAX_BANTUAN) {
        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }
        char *fields[8];
        if (splitPipeLine(line, fields, 8) == 8) {
            Bantuan *b = &sys->bantuan[sys->jumlahBantuan];
            memset(b, 0, sizeof(*b));
            snprintf(b->kodeBantuan, sizeof(b->kodeBantuan), "%s", fields[0]);
            snprintf(b->namaBantuan, sizeof(b->namaBantuan), "%s", fields[1]);
            snprintf(b->satuan, sizeof(b->satuan), "%s", fields[2]);
            snprintf(b->kategoriLogistik, sizeof(b->kategoriLogistik), "%s", fields[3]);
            b->stokTersedia = atoi(fields[4]);
            b->stokMinimum = atoi(fields[5]);
            int jenis = atoi(fields[6]);
            if (jenis == DETAIL_PAKAIAN) {
                b->jenisDetail = DETAIL_PAKAIAN;
                snprintf(b->detail.ukuranPakaian, sizeof(b->detail.ukuranPakaian), "%s", fields[7]);
            } else if (jenis == DETAIL_KEDALUWARSA) {
                b->jenisDetail = DETAIL_KEDALUWARSA;
                snprintf(b->detail.tanggalKedaluwarsa, sizeof(b->detail.tanggalKedaluwarsa), "%s", fields[7]);
            } else {
                b->jenisDetail = DETAIL_HYGIENE;
                snprintf(b->detail.kategoriKit, sizeof(b->detail.kategoriKit), "%s", fields[7]);
            }
            sys->jumlahBantuan++;
        }
    }
    fclose(fp);
    return 1;
}

static void loadKorbanFile(SistemDisasterCare *sys) {
    FILE *fp = fopen(FILE_KORBAN, "r");
    sys->jumlahKorban = 0;
    if (fp == NULL) {
        return;
    }
    char line[LINE_BUF_SIZE];
    while (fgets(line, sizeof(line), fp) != NULL && sys->jumlahKorban < MAX_KORBAN) {
        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }
        char *fields[9];
        if (splitPipeLine(line, fields, 9) == 9) {
            Korban *k = &sys->korban[sys->jumlahKorban];
            snprintf(k->kodeKorban, sizeof(k->kodeKorban), "%s", fields[0]);
            snprintf(k->kodePosko, sizeof(k->kodePosko), "%s", fields[1]);
            snprintf(k->namaKepalaKeluarga, sizeof(k->namaKepalaKeluarga), "%s", fields[2]);
            k->jumlahKK = atoi(fields[3]);
            k->jumlahAnggota = atoi(fields[4]);
            k->jumlahLansia = atoi(fields[5]);
            k->jumlahBalita = atoi(fields[6]);
            k->jumlahIbuHamil = atoi(fields[7]);
            k->totalRentan = atoi(fields[8]);
            sys->jumlahKorban++;
        }
    }
    fclose(fp);
}

static void loadKebutuhanFile(SistemDisasterCare *sys) {
    FILE *fp = fopen(FILE_KEBUTUHAN, "r");
    sys->jumlahKebutuhan = 0;
    if (fp == NULL) {
        return;
    }
    char line[LINE_BUF_SIZE];
    while (fgets(line, sizeof(line), fp) != NULL && sys->jumlahKebutuhan < MAX_KEBUTUHAN) {
        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }
        char *fields[6];
        if (splitPipeLine(line, fields, 6) == 6) {
            Kebutuhan *kb = &sys->kebutuhan[sys->jumlahKebutuhan];
            snprintf(kb->kodeKebutuhan, sizeof(kb->kodeKebutuhan), "%s", fields[0]);
            snprintf(kb->kodePosko, sizeof(kb->kodePosko), "%s", fields[1]);
            snprintf(kb->kodeBantuan, sizeof(kb->kodeBantuan), "%s", fields[2]);
            kb->jumlahPermintaan = atoi(fields[3]);
            kb->jumlahRealisasi = atoi(fields[4]);
            snprintf(kb->tanggalPermintaan, sizeof(kb->tanggalPermintaan), "%s", fields[5]);
            sys->jumlahKebutuhan++;
        }
    }
    fclose(fp);
}

static void loadTransaksiFile(SistemDisasterCare *sys) {
    freeListTransaksi(&sys->riwayatTransaksi);
    FILE *fp = fopen(FILE_TRANSAKSI, "r");
    if (fp == NULL) {
        return;
    }
    char line[LINE_BUF_SIZE];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }
        char *fields[7];
        if (splitPipeLine(line, fields, 7) == 7) {
            NodeTransaksi trx;
            memset(&trx, 0, sizeof(trx));
            snprintf(trx.kodeTransaksi, sizeof(trx.kodeTransaksi), "%s", fields[0]);
            snprintf(trx.tanggal, sizeof(trx.tanggal), "%s", fields[1]);
            snprintf(trx.kodePosko, sizeof(trx.kodePosko), "%s", fields[2]);
            snprintf(trx.kodeBantuan, sizeof(trx.kodeBantuan), "%s", fields[3]);
            trx.jenisTransaksi = atoi(fields[4]);
            trx.jumlah = atoi(fields[5]);
            snprintf(trx.keterangan, sizeof(trx.keterangan), "%s", fields[6]);
            (void)appendTransaksi(&sys->riwayatTransaksi, &trx);
        }
    }
    fclose(fp);
}

int loadAllData(SistemDisasterCare *sys) {
    if (sys == NULL) {
        return 0;
    }
    (void)ensureDataDirectory();

    if (!fileExists(FILE_POSKO) || !fileExists(FILE_BANTUAN)) {
        printf("%s[INFO] File data (.dat) belum ditemukan. Melakukan auto-seed data awal ke folder '%s/'...%s\n",
               CLR_BYELLOW, DATA_DIR, CLR_RESET);
        seedInitialData(sys);
        if (saveAllData(sys)) {
            printf("%s[OK] Data awal (5 Posko, 8 Bantuan, 3 Kebutuhan) berhasil dibuat dan disimpan.%s\n",
                   CLR_BGREEN, CLR_RESET);
        } else {
            printf("%s[WARNING] Data awal dimuat di memori, namun gagal menulis file ke '%s/'.%s\n",
                   CLR_BRED, DATA_DIR, CLR_RESET);
        }
        return 1;
    }

    int okPosko = loadPoskoFile(sys);
    int okBantuan = loadBantuanFile(sys);
    if (!okPosko || !okBantuan) {
        seedInitialData(sys);
        (void)saveAllData(sys);
        return 0;
    }
    loadKorbanFile(sys);
    loadKebutuhanFile(sys);
    loadTransaksiFile(sys);

    printf("%s[OK] Data dimuat dari '%s/': %d Posko, %d Bantuan, %d Korban, %d Kebutuhan, %d Transaksi.%s\n",
           CLR_BGREEN, DATA_DIR,
           sys->jumlahPosko, sys->jumlahBantuan, sys->jumlahKorban,
           sys->jumlahKebutuhan, sys->riwayatTransaksi.count, CLR_RESET);
    return 1;
}
