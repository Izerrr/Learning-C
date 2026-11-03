#include <stdio.h>
#include <stdlib.h>

int JUMLAH_DISK = 0;
int **tiang = NULL;
int *tinggi_tiang = NULL;
int langkah = 0;

// Konversi karakter tiang ('a', 'b', 'c') ke indeks array (0, 1, 2)
int get_index(char t) {
    return t - 'a';
}

void inisialisasi_tiang(int jumlah) {
    JUMLAH_DISK = jumlah;

    tiang = malloc(3 * sizeof(int *));
    if (tiang == NULL) {
        fprintf(stderr, "Gagal mengalokasi memori untuk tiang.\n");
        exit(1);
    }

    for (int i = 0; i < 3; i++) {
        tiang[i] = calloc((size_t)JUMLAH_DISK, sizeof(int));
        if (tiang[i] == NULL) {
            fprintf(stderr, "Gagal mengalokasi memori untuk stack tiang %d.\n", i);
            exit(1);
        }
    }

    tinggi_tiang = calloc(3, sizeof(int));
    if (tinggi_tiang == NULL) {
        fprintf(stderr, "Gagal mengalokasi memori untuk tinggi tiang.\n");
        exit(1);
    }

    for (int i = 0; i < JUMLAH_DISK; i++) {
        tiang[0][i] = JUMLAH_DISK - i;
    }
    tinggi_tiang[0] = JUMLAH_DISK;
}

void bebas_memori(void) {
    if (tiang != NULL) {
        for (int i = 0; i < 3; i++) {
            free(tiang[i]);
        }
        free(tiang);
    }

    free(tinggi_tiang);
}

// Fungsi untuk menggambar satu segmen disk / tiang
void print_disk(int ukuran) {
    if (ukuran == 0) {
        for (int i = 0; i < JUMLAH_DISK; i++) printf(" ");
        printf("|");
        for (int i = 0; i < JUMLAH_DISK; i++) printf(" ");
    } else {
        int padding = JUMLAH_DISK - ukuran;
        for (int i = 0; i < padding; i++) printf(" ");
        printf("[");
        for (int i = 0; i < ukuran - 1; i++) printf("=");
        printf("%d", ukuran);
        for (int i = 0; i < ukuran - 1; i++) printf("=");
        printf("]");
        for (int i = 0; i < padding; i++) printf(" ");
    }
    printf("   ");
}

// Menampilkan status visual ketiga tiang saat ini
void tampilkan_visual(char src, char dst, int disk) {
    printf("\n=======================================================\n");
    if (langkah == 0) {
        printf("KONDISI AWAL\n");
    } else {
        printf("Langkah %2d: Pindahkan Disk %d dari Tiang %c -> %c\n", langkah, disk, src, dst);
    }
    printf("=======================================================\n\n");

    for (int baris = JUMLAH_DISK - 1; baris >= 0; baris--) {
        for (int t = 0; t < 3; t++) {
            if (baris < tinggi_tiang[t]) {
                print_disk(tiang[t][baris]);
            } else {
                print_disk(0);
            }
        }
        printf("\n");
    }

    printf("  [Tiang A]      [Tiang B]      [Tiang C]\n");
}

// Memindahkan disk antar-array sekaligus memicu visualisasi
void move_disk(int n, char source, char dest) {
    int src_idx = get_index(source);
    int dst_idx = get_index(dest);

    int disk_dipindah = tiang[src_idx][--tinggi_tiang[src_idx]];
    tiang[dst_idx][tinggi_tiang[dst_idx]++] = disk_dipindah;

    langkah++;
    tampilkan_visual(source, dest, disk_dipindah);
}

// Logika rekursif inti Tower of Hanoi
void towerOfHanoi(int n, char source, char dest, char aux) {
    if (n == 1) {
        move_disk(1, source, dest);
        return;
    }
    towerOfHanoi(n - 1, source, aux, dest);
    move_disk(n, source, dest);
    towerOfHanoi(n - 1, aux, dest, source);
}

int main() {
    int jumlah_disk;

    printf("Masukkan jumlah cakram: ");
    if (scanf("%d", &jumlah_disk) != 1 || jumlah_disk <= 0) {
        printf("Input tidak valid. Masukkan angka positif.\n");
        return 1;
    }

    inisialisasi_tiang(jumlah_disk);
    tampilkan_visual(' ', ' ', 0);
    towerOfHanoi(JUMLAH_DISK, 'a', 'c', 'b');

    printf("\nSelesai! Seluruh disk berhasil dipindahkan ke Tiang C dalam %d langkah.\n\n", langkah);
    bebas_memori();
    return 0;
}