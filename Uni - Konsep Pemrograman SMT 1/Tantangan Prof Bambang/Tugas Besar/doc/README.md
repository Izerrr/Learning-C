# DisasterCare: Sistem Manajemen Bantuan Korban Bencana

Aplikasi terminal terintegrasi berbasis Bahasa C (`C99`/`C11`) untuk **Tugas Besar Konsep Pemrograman (S1 Informatika, FATISDA UNS — Kelas B, Ganjil 2026/2027)**.

Sistem ini membantu petugas Posko Induk Kabupaten Sejahtera dalam mengelola posko pengungsian, pencatatan korban & kelompok rentan, master logistik, penerimaan & distribusi barang dengan validasi ketat, kalkulasi skor prioritas posko ($S = P + M + R$), pencarian, pengurutan, peringatan stok minimum, laporan rekapitulasi, serta persistensi data menggunakan file teks berpemisah pipe (`|`).

---

## 1. Struktur Folder Proyek (Siap Kumpul Google Drive)

```text
Tugas Besar/
├── .vscode/
│   └── settings.json       # Konfigurasi Code Runner VS Code untuk kompilasi multi-file
├── src/
│   ├── disastercare.h      # Include guards, konstanta, warna ANSI, enum, tagged union, struct & prototipe
│   ├── storage.c           # Utilitas CLI cross-platform, input aman, parser tanggal, Linked List, File I/O & auto-seed
│   ├── operations.c        # Pointer stok, kalkulasi prioritas, sorting, searching, tabel & fitur F01–F15
│   └── main.c              # Inisialisasi terminal, auto-load/seed, loop menu 0–14, auto-save & free() memori
├── test/
│   ├── uat_test.py         # Skrip otomatisasi pengujian 15 skenario UAT (U01–U15)
│   ├── posko.dat           # (Otomatis dibuat saat runtime) Data master posko
│   ├── bantuan.dat         # (Otomatis dibuat saat runtime) Data master bantuan logistik
│   ├── korban.dat          # (Otomatis dibuat saat runtime) Data catatan korban & kelompok rentan
│   ├── kebutuhan.dat       # (Otomatis dibuat saat runtime) Data permintaan & realisasi kebutuhan posko
│   └── transaksi.dat       # (Otomatis dibuat saat runtime) Riwayat transaksi logistik (Linked List)
└── doc/
    └── README.md           # Dokumentasi teknis, cara kompilasi, fitur F01–F15 & panduan demo U01–U15
```

---

## 2. Arsitektur & Penerapan Konsep Bahasa C

| Konsep Wajib | Implementasi Nyata pada Source Code |
| --- | --- |
| **Modularitas (50+ fungsi)** | Terpisah rapi ke `src/disastercare.h`, `src/storage.c`, `src/operations.c`, dan `src/main.c` |
| **Pointer (Pass-by-Reference)** | `tambahStokBantuan(int *stokCurrent, int)` & `kurangiStokBantuan(int *stokCurrent, int)` dengan `NULL` guard dan dereference `*stokCurrent -= jumlahKeluar`, serta passing `SistemDisasterCare *sys` & `Posko *posko` |
| **Struct Lengkap** | `Posko` (`hariBertahanMakanan` bertipe `float`), `Korban`, `Bantuan`, `Kebutuhan`, `NodeTransaksi`, `ListTransaksi`, `SistemDisasterCare` |
| **Tagged Union** | `enum JenisDetailBantuan` (`DETAIL_PAKAIAN`, `DETAIL_KEDALUWARSA`, `DETAIL_HYGIENE`) + `union DetailBantuan` (`ukuranPakaian`, `tanggalKedaluwarsa`, `kategoriKit`) |
| **Dynamic Singly Linked List** | `NodeTransaksi` + `ListTransaksi` (`head`, `tail`, `count`) menggunakan `malloc()` pada `appendTransaksi()` dan dibersihkan via `free()` pada `freeListTransaksi()` saat reload/reset/exit |
| **Linear Search** | Case-insensitive substring search (`containsIgnoreCase`) untuk Posko, Bantuan, Riwayat Transaksi, dan Korban (Menu 9) |
| **Sorting (Descending)** | **Selection Sort** (`sortPoskoByPengungsi`) untuk peringkat jumlah pengungsi & **Insertion Sort** (`sortPoskoByPrioritas`) untuk peringkat skor prioritas (Menu 10) |
| **Validasi Tanggal Akurat** | `parseDateToInt("DD-MM-YYYY")` mengonversi ke integer `YYYYMMDD` (termasuk validasi tahun kabisat) agar perbandingan kedaluwarsa akurat (tidak memakai `strcmp`) |
| **Safe CLI Input & Sanitasi** | Seluruh input memakai `fgets` + `sscanf` (anti infinite-loop saat input huruf pada menu angka), serta `sanitizePipe()` mengubah `|` menjadi `/` agar file delimiter tidak rusak |
| **File Processing & Atomic Save** | Format teks delimiter `|` pada folder `test/*.dat`, penulisan via temporary file `.tmp` + `rename`, serta auto-seed otomatis bila file `.dat` belum ada |
| **Cross-Platform CLI** | `#ifdef _WIN32` (`cls`, `ENABLE_VIRTUAL_TERMINAL_PROCESSING`, `_mkdir`) dan POSIX (`clear`, `mkdir`) beserta warna ANSI |

---

## 3. Cara Kompilasi & Menjalankan Program

### A. Melalui Terminal (GCC — Rekomendasi)

Jalankan dari direktori root proyek (`Tugas Besar/`):

**Windows (Git Bash / CMD / PowerShell 7):**
```bash
gcc -Wall -Wextra -std=c11 src/*.c -o disastercare.exe && ./disastercare.exe
```

**Linux / macOS:**
```bash
gcc -Wall -Wextra -std=c11 src/*.c -o disastercare && ./disastercare
```

**Windows PowerShell 5.1:**
```powershell
gcc -Wall -Wextra -std=c11 src/*.c -o disastercare.exe
if ($?) { .\disastercare.exe }
```

### B. Melalui VS Code Code Runner

File `.vscode/settings.json` sudah dikonfigurasi dengan:
```json
"code-runner.runInTerminal": true,
"code-runner.fileDirectoryAsCwd": false,
"code-runner.executorMap": {
  "c": "gcc -Wall -Wextra src/*.c -o disastercare.exe && ./disastercare.exe"
}
```
Cukup buka `src/main.c` lalu tekan tombol **Run Code** (`Ctrl + Alt + N`).
*(Catatan Windows: jika default terminal VS Code masih Windows PowerShell 5.1 yang belum mendukung operator `&&`, ubah Default Terminal Profile VS Code ke **Git Bash**, **Command Prompt**, atau **PowerShell 7**).*

### C. Menjalankan Uji Otomatis UAT (U01–U15)

Setelah `disastercare` / `disastercare.exe` selesai dikompilasi:
```bash
python test/uat_test.py
```
Skrip ini akan menjalankan seluruh 15 skenario UAT (termasuk penghapusan `.dat` untuk U13 dan restart proses untuk U12), menyimpan log detail ke `test/uat_results.log`, lalu mereset file `.dat` kembali ke data seed awal yang bersih agar siap didemokan.

---

## 4. Daftar Fitur Fungsional (F01–F15) & Navigasi Menu

| Kode | Nama Fitur | Menu Utama | Ringkasan Kemampuan |
| --- | --- | --- | --- |
| **F01** | Kelola Master Posko | Menu 1 | Lihat tabel posko, tambah posko unik, ubah data, aktifkan/nonaktifkan posko |
| **F02** | Pencatatan Data Korban | Menu 2 | Catat KK, jumlah jiwa, lansia/balita/ibu hamil; otomatis terakumulasi ke master `Posko` + re-hitung prioritas |
| **F03** | Kelola Jenis Bantuan | Menu 3 | Kelola master logistik, satuan, kategori, stok minimum, dan detail Tagged Union |
| **F04** | Penerimaan Bantuan Masuk | Menu 4 | Catat bantuan dari donor, perbarui stok via pointer, tambah node ke Linked List |
| **F05** | Monitoring Stok Real-Time | Menu 5 | Tampilkan saldo stok real-time dengan status warna (`AMAN`, `WASPADA`, `KRITIS!`) |
| **F06** | Pencatatan Kebutuhan Posko | Menu 6 | Catat permintaan per posko; kolom realisasi bertambah otomatis saat distribusi F08 cocok |
| **F07** | Kalkulasi Prioritas Posko | Menu 7 | Hitung otomatis $S = P + M + R$ (rentang 3–9) dan klasifikasi `TINGGI` / `SEDANG` / `RENDAH` |
| **F08** | Distribusi Bantuan ke Posko | Menu 8 | Salurkan logistik ke posko aktif, kurangi stok via pointer, catat ke Linked List |
| **F09** | Validasi Transaksi Ketat | Menu 4 & 8 | Tolak ID tidak ditemukan, posko nonaktif, jumlah $\le 0$, stok kurang, atau barang kedaluwarsa |
| **F10** | Fitur Pencarian (Searching) | Menu 9 | Linear Search case-insensitive substring untuk Posko, Bantuan, Transaksi, dan Korban |
| **F11** | Pengurutan Data (Sorting) | Menu 10 | Selection Sort (pengungsi terbanyak) & Insertion Sort (skor prioritas tertinggi) |
| **F12** | Riwayat Transaksi Logistik | Menu 11 | Log kronologis seluruh transaksi masuk & keluar dari traversal Linked List |
| **F13** | Sistem Peringatan Stok Minimal | Menu 12 | Alert warna merah otomatis untuk stok `< stokMinimum` beserta deteksi barang kedaluwarsa |
| **F14** | Laporan & Rekapitulasi | Menu 13 | Ringkasan pengungsi, total masuk/keluar/sisa stok, rekap distribusi per posko & pemenuhan kebutuhan |
| **F15** | Persistensi File I/O | Menu 14 & Exit | Simpan/muat/reset file `.dat` delimiter `|`, auto-seed bila file belum ada, dan `free()` memori saat keluar |

---

## 5. Data Seed Awal Terkalibrasi

Saat pertama kali dijalankan tanpa file `.dat`, aplikasi otomatis membuat:
- **5 Posko (Total 350 KK, 1.200 Jiwa):**
  - `P001` Desa Makmur — 80 KK, 280 Jiwa, 45 Rentan, 2.0 hari makanan ($S = 3+2+2 = 7$, **TINGGI**)
  - `P002` Desa Sentosa — 65 KK, 220 Jiwa, 30 Rentan, 2.5 hari makanan ($S = 2+2+2 = 6$, **SEDANG**)
  - `P003` Desa Harapan — 90 KK, 310 Jiwa, 65 Rentan, **0.5 hari** makanan ($S = 3+3+3 = 9$, **TINGGI**) — *siap untuk U08*
  - `P004` Desa Sukamaju — 55 KK, 180 Jiwa, 18 Rentan, 4.0 hari makanan ($S = 2+1+1 = 4$, **RENDAH**)
  - `P005` Desa Mulyasari — 60 KK, 210 Jiwa, 25 Rentan, 3.0 hari makanan ($S = 2+1+2 = 5$, **SEDANG**)
- **8 Jenis Bantuan Logistik:**
  - `B001` Beras Pangan Utama (`kg`, stok **300**, min 100, Exp `31-12-2027`) — *siap untuk U04, U05, U06*
  - `B002` Mi Instan Paket (`dus`, stok 150, min 50, Exp `30-06-2027`)
  - `B003` Air Mineral Kemasan (`liter`, stok 500, min 150, Exp `31-12-2027`)
  - `B004` Makanan Siap Saji / Kaleng (`paket`, stok 80, min 30, Exp **`01-01-2025`**) — *siap diblokir untuk U14*
  - `B005` Selimut & Hangat (`buah`, stok 120, min 40, Ukuran `All-Size`)
  - `B006` Pakaian Layak Pakai (`paket`, stok 90, min 30, Ukuran `Dewasa-L`)
  - `B007` Popok Bayi & Balita (`paket`, stok **15**, min **40**, Kit `Perawatan Bayi`) — *siap memicu alert merah U11*
  - `B008` Kit Kebersihan / Hygiene (`paket`, stok 75, min 25, Kit `Sanitasi Mandi`)

---

## 6. Panduan Demo Manual UAT (U01–U15)

1. **U13 (Auto-Seed):** Hapus seluruh file `test/*.dat`, jalankan program -> program tidak crash dan otomatis membuat 5 file `.dat` dengan data seed.
2. **U01 (Tambah Posko):** Pilih Menu `1` -> `2` -> masukkan `P006`, `Desa Damai`, `0`, `0`, `0`, `3.5` -> cek Menu `1` -> `1`.
3. **U02 (Kode Duplikat):** Pilih Menu `1` -> `2` -> masukkan `P001` -> sistem menolak dengan pesan duplikasi.
4. **U03 (Data Korban & Akumulasi):** Pilih Menu `2` -> `1` -> pilih `P006`, masukkan `80` KK, `160` Jiwa, Lansia `10`, Balita `10`, Ibu Hamil `5` (total 25 Rentan) -> cek `P006` terakumulasi otomatis.
5. **U04 (Bantuan Masuk):** Pilih Menu `4` -> `B001`, Donor `BNPB Pusat`, Jumlah `500`, Tanggal `24-11-2026` -> stok `B001` naik `300 -> 800 kg`.
6. **U05 (Distribusi Valid):** Pilih Menu `8` -> Posko `P003`, Bantuan `B001`, Tanggal `24-11-2026`, Jumlah `200` -> stok turun `800 -> 600 kg` & realisasi `REQ001` naik `0 -> 200`.
7. **U06 (Stok Kurang):** Pilih Menu `8` -> Posko `P003`, Bantuan `B001`, Tanggal `24-11-2026`, Jumlah `700` -> ditolak (`Available: 600, Requested: 700`), stok tetap `600 kg`.
8. **U07 (Input Negatif / Nol):** Pilih Menu `4` -> `B001` -> masukkan `-50` lalu `0` -> keduanya ditolak & diminta angka positif (ketik `batal` untuk kembali).
9. **U08 (Prioritas P003):** Pilih Menu `7` -> `1` -> masukkan `P003` -> tampil `P=3 (310 Jiwa), M=3 (0.5 Hari), R=3 (65 Rentan), Total S=9 (TINGGI)`.
10. **U09 (Pencarian P003):** Pilih Menu `9` -> `1` -> masukkan `P003` -> tampil detail lengkap Posko `P003 Desa Harapan`.
11. **U10 (Sorting Pengungsi):** Pilih Menu `10` -> `1` -> tampil urutan descending `P003 (310) -> P001 (280) -> P002 (220) -> P005 (210) -> P004 (180) -> P006 (160)`.
12. **U11 (Alert Stok Kritis):** Pilih Menu `5` atau Menu `12` -> `B007 Popok Bayi & Balita` (stok `15 < min 40`) disorot warna merah `STOK KRITIS!`.
13. **U14 (Blokir Kedaluwarsa):** Pilih Menu `8` -> Posko `P003`, Bantuan `B004`, Tanggal `24-11-2026` -> distribusi diblokir karena `Exp: 01-01-2025`.
14. **U12 (Simpan, Tutup & Restart):** Pilih Menu `14` -> `1`, keluar lewat Menu `0`, lalu jalankan ulang aplikasi -> seluruh 6 posko, 8 bantuan, 1 korban, 3 kebutuhan, dan 2 transaksi termuat utuh.
15. **U15 (Laporan Rekapitulasi):** Pilih Menu `13` -> verifikasi Total Masuk `500`, Total Keluar `200`, Sisa Stok `B001 = 600` (Grand Total `1630`), dan Distribusi `P003 = 200`.
