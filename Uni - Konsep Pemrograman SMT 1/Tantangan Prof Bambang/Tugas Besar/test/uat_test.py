#!/usr/bin/env python3
"""
Automated UAT Verification Suite (U01 - U15) for DisasterCare C Application.
Menjalankan binary terkompilasi secara end-to-end via stdin/stdout serta
memverifikasi persistensi file .dat berpemisah pipe '|' pada folder test/.
"""

import os
import re
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
TEST_DIR = ROOT_DIR / "test"
EXE_NAME = "disastercare.exe" if os.name == "nt" else "disastercare"
EXE_PATH = ROOT_DIR / EXE_NAME
LOG_PATH = TEST_DIR / "uat_results.log"

DAT_FILES = [
    TEST_DIR / "posko.dat",
    TEST_DIR / "bantuan.dat",
    TEST_DIR / "korban.dat",
    TEST_DIR / "kebutuhan.dat",
    TEST_DIR / "transaksi.dat",
]

ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


def strip_ansi(text: str) -> str:
    return ANSI_RE.sub("", text)


def run_app(inputs: list[str]) -> tuple[int, str, str]:
    payload = "\n".join(inputs) + "\n"
    proc = subprocess.run(
        [str(EXE_PATH)],
        input=payload,
        text=True,
        capture_output=True,
        cwd=str(ROOT_DIR),
        encoding="utf-8",
        errors="replace",
        timeout=20,
    )
    return proc.returncode, proc.stdout, proc.stderr


def clean_dat_files() -> None:
    for f in DAT_FILES:
        if f.exists():
            f.unlink()


def main() -> int:
    if not EXE_PATH.exists():
        print(f"[FATAL] Binary tidak ditemukan: {EXE_PATH}")
        return 1

    results: list[tuple[str, str, bool, str]] = []
    full_log: list[str] = []

    def record(code: str, name: str, ok: bool, detail: str) -> None:
        status = "PASS" if ok else "FAIL"
        results.append((code, name, ok, detail))
        full_log.append(f"[{status}] {code} — {name}: {detail}")

    # =========================================================================
    # FASE 1: U13 — Jalankan Aplikasi saat File .dat Belum Ada (Auto-Seed)
    # =========================================================================
    clean_dat_files()
    rc, raw_out, err_out = run_app(["0"])
    out = strip_ansi(raw_out)
    full_log.append("=== FASE 1 (U13: AUTO-SEED TANPA FILE .DAT) ===\n" + out)

    all_created = all(f.exists() for f in DAT_FILES)
    posko_lines = (TEST_DIR / "posko.dat").read_text(encoding="utf-8").strip().splitlines() if (TEST_DIR / "posko.dat").exists() else []
    bantuan_lines = (TEST_DIR / "bantuan.dat").read_text(encoding="utf-8").strip().splitlines() if (TEST_DIR / "bantuan.dat").exists() else []

    u13_ok = (
        rc == 0
        and all_created
        and "auto-seed" in out.lower()
        and len(posko_lines) == 5
        and len(bantuan_lines) == 8
    )
    record(
        "U13",
        "Jalankan Aplikasi saat File .dat Belum Ada",
        u13_ok,
        f"Exit={rc}, 5 file .dat terbuat={all_created}, seed={len(posko_lines)} Posko & {len(bantuan_lines)} Bantuan",
    )

    # =========================================================================
    # FASE 2: Sesi Operasional Terpadu (U01 - U11, U14, Simpan & Exit untuk U12)
    # =========================================================================
    session1_inputs = [
        # Uji input huruf pada menu utama (anti infinite-loop)
        "abc",
        # U01: Menu 1 -> Submenu 2 (Tambah Posko P006 Desa Damai) -> Pause -> Submenu 1 (Lihat) -> Pause
        "1", "2", "P006", "Desa Damai", "0", "0", "0", "3.5", "",
        "1", "",
        # U02: Masih di Menu 1 -> Submenu 2 (Tambah Duplikat P001) -> Pause -> Submenu 0
        "2", "P001", "",
        "0",
        # U03: Menu 2 -> Submenu 1 (Catat Korban ke P006: 80 KK, 160 Jiwa, 10+10+5=25 Rentan) -> Pause -> Submenu 0
        "2", "1", "P006", "Warga Blok A | Sektor 1", "80", "160", "10", "10", "5", "",
        "0",
        # U04: Menu 4 (Bantuan Masuk B001 500 kg, Stok Awal 300 -> 800) -> Pause
        "4", "B001", "BNPB Pusat", "500", "24-11-2026", "",
        # U05: Menu 8 (Distribusi B001 200 kg ke P003, Stok 800 -> 600) -> Pause
        "8", "P003", "B001", "24-11-2026", "200", "Tahap 1 Desa Harapan", "",
        # U06: Menu 8 (Distribusi B001 700 kg ke P003, Stok 600 -> Ditolak) -> Pause
        "8", "P003", "B001", "24-11-2026", "700", "",
        # U07: Menu 4 (Input Jumlah Negatif -50 & Nol 0 -> Ditolak, lalu batal) -> Pause
        "4", "B001", "Donor Uji Validasi", "-50", "0", "batal", "",
        # U08: Menu 7 (Prioritas Posko) -> Submenu 1 (Cek Detail P003) -> Pause -> Submenu 0
        "7", "1", "P003", "",
        "0",
        # U09: Menu 9 (Pencarian) -> Submenu 1 (Cari Posko P003) -> Pause -> Submenu 0
        "9", "1", "P003", "",
        "0",
        # U10: Menu 10 (Pengurutan) -> Submenu 1 (Pengungsi Desc) -> Pause -> Submenu 2 (Prioritas Desc) -> Pause -> Submenu 0
        "10", "1", "", "2", "", "0",
        # U11: Menu 5 (Monitoring Stok) -> Pause & Menu 12 (Peringatan Stok Minimum) -> Pause
        "5", "",
        "12", "",
        # U14: Menu 8 (Distribusi B004 Kedaluwarsa 01-01-2025 pada 24-11-2026 -> Diblokir) -> Pause
        "8", "P003", "B004", "24-11-2026", "",
        # U12 (bagian 1): Menu 14 -> Submenu 1 (Simpan Data) -> Pause -> Submenu 0 -> Menu 0 (Keluar)
        "14", "1", "", "0",
        "0",
    ]

    rc1, raw_out1, err_out1 = run_app(session1_inputs)
    out1 = strip_ansi(raw_out1)
    full_log.append("\n=== FASE 2 (SESI 1: U01-U11, U14, SIMPAN & EXIT) ===\n" + out1)

    posko_dat_after1 = (TEST_DIR / "posko.dat").read_text(encoding="utf-8")
    korban_dat_after1 = (TEST_DIR / "korban.dat").read_text(encoding="utf-8")
    bantuan_dat_after1 = (TEST_DIR / "bantuan.dat").read_text(encoding="utf-8")
    kebutuhan_dat_after1 = (TEST_DIR / "kebutuhan.dat").read_text(encoding="utf-8")
    transaksi_dat_after1 = (TEST_DIR / "transaksi.dat").read_text(encoding="utf-8")

    # U01 check
    u01_ok = (
        rc1 == 0
        and "Posko P006 (Desa Damai) berhasil ditambahkan" in out1
        and "P006|Desa Damai|" in posko_dat_after1
    )
    record("U01", "Tambah Posko Baru (P006, Desa Damai)", u01_ok,
           "P006 tersimpan di memori, tampil di daftar, dan tercatat di test/posko.dat")

    # U02 check
    u02_ok = "Kode Posko 'P001' sudah terdaftar (duplikat)" in out1
    record("U02", "Tambah Posko Kode Duplikat (P001)", u02_ok,
           "Registrasi ditolak dengan pesan kesalahan duplikasi")

    # U03 check (termasuk verifikasi sanitasi '|' -> '/')
    u03_ok = (
        "Akumulasi Posko P006 kini -> KK: 80 | Jiwa: 160 | Rentan: 25" in out1
        and "P006|Desa Damai|80|160|25|" in posko_dat_after1
        and "K001|P006|Warga Blok A / Sektor 1|80|160|10|10|5|25" in korban_dat_after1
    )
    record("U03", "Pencatatan Data Korban & Rentan (80 KK, 25 Rentan)", u03_ok,
           "P006 terakumulasi (80 KK, 160 Jiwa, 25 Rentan) + karakter '|' disanitasi menjadi '/'")

    # U04 check
    u04_ok = (
        "Penerimaan TRX0001 tercatat di Linked List" in out1
        and "300 kg -> 800 kg (+500 kg)" in out1
        and "TRX0001|24-11-2026|INDUK|B001|1|500|" in transaksi_dat_after1
    )
    record("U04", "Bantuan Masuk B001 500 kg (Stok Awal 300 kg)", u04_ok,
           "Stok B001 bertambah 300 -> 800 kg, TRX0001 tercatat di Linked List")

    # U05 check
    u05_ok = (
        "Distribusi TRX0002 berhasil diproses" in out1
        and "800 kg -> 600 kg" in out1
        and "TRX0002|24-11-2026|P003|B001|2|200|" in transaksi_dat_after1
        and "REQ001|P003|B001|500|200|" in kebutuhan_dat_after1
    )
    record("U05", "Distribusi B001 200 kg ke P003 (Stok 800 kg)", u05_ok,
           "Stok B001 berkurang 800 -> 600 kg, TRX0002 tercatat, realisasi REQ001 naik 0 -> 200")

    # U06 check
    u06_ok = (
        "Stok tidak mencukupi! Available: 600, Requested: 700" in out1
        and "Stok B001 tetap 600 kg" in out1
        and "B001|Beras Pangan Utama|kg|Pangan Pokok|600|100|" in bantuan_dat_after1
    )
    record("U06", "Distribusi B001 700 kg (Stok hanya 600 kg)", u06_ok,
           "Transaksi ditolak (Available 600 < Requested 700), stok tetap 600 kg")

    # U07 check
    u07_ok = (
        "Jumlah tidak boleh nol atau negatif (-50)" in out1
        and "Jumlah tidak boleh nol atau negatif (0)" in out1
        and "Input harus berupa bilangan bulat yang valid" in out1
    )
    record("U07", "Input Jumlah Bantuan Negatif / Nol (-50 kg & 0)", u07_ok,
           "Input -50, 0, dan huruf 'abc' ditolak bersih tanpa crash/infinite loop")

    # U08 check
    u08_ok = (
        "Jumlah Pengungsi    : 310 Jiwa (Skor P = 3)" in out1
        and "Ketahanan Makanan   : 0.5 Hari (Skor M = 3)" in out1
        and "Kelompok Rentan     : 65 Orang (Skor R = 3)" in out1
        and "Total Skor Prioritas: 9 (TINGGI)" in out1
    )
    record("U08", "Hitung Prioritas Posko P003 (310 Jiwa, 0.5 Hari, 65 Rentan)", u08_ok,
           "P=3, M=3, R=3 -> Total Skor S=9 (Kategori Prioritas TINGGI)")

    # U09 check
    u09_ok = (
        "--- DETAIL POSKO P003 ---" in out1
        and "Nama Desa / Posko   : Desa Harapan" in out1
        and "Total ditemukan: 1 posko." in out1
    )
    record("U09", "Pencarian Posko berdasarkan Kode P003", u09_ok,
           "Linear Search menampilkan detail lengkap Posko P003 Desa Harapan")

    # U10 check — pastikan urutan #1 P003 -> #2 P001 -> #3 P002 -> #4 P005 -> #5 P004 -> #6 P006
    sort_block_match = re.search(
        r"\[HASIL PENGURUTAN — PENGUNGSI TERBANYAK.*?\n(.*?)\nTekan \[ENTER\]",
        out1,
        re.DOTALL,
    )
    sort_codes = re.findall(r"#\d+\s*\|\s*(P\d+)", sort_block_match.group(1)) if sort_block_match else []
    u10_ok = sort_codes == ["P003", "P001", "P002", "P005", "P004", "P006"]
    record("U10", "Pengurutan Posko berdasarkan Pengungsi", u10_ok,
           f"Urutan Selection Sort Descending: {' -> '.join(sort_codes)}")

    # U11 check — alert merah ANSI untuk stok kritis B007
    u11_ok = (
        "B007" in out1
        and "Popok Bayi & Balita" in out1
        and "STOK KRITIS!" in out1
        and "\x1b[1;31mB007" in raw_out1
    )
    record("U11", "Monitoring Stok di bawah Batas Minimum", u11_ok,
           "B007 Popok Bayi (Stok 15 < Min 40, kurang 25) memicu alert warna merah ANSI")

    # U14 check — blokir barang kedaluwarsa B004
    u14_ok = (
        "[DITOLAK — BARANG KEDALUWARSA]" in out1
        and "B004 (Makanan Siap Saji / Kaleng)" in out1
        and "Tanggal Kedaluwarsa: 01-01-2025 | Tanggal Distribusi: 24-11-2026" in out1
        and "B004|Makanan Siap Saji / Kaleng|paket|Pangan Olahan|80|30|" in bantuan_dat_after1
    )
    record("U14", "Distribusi Barang Kedaluwarsa (B004 Exp 01-01-2025)", u14_ok,
           "Distribusi B004 diblokir via parseDateToInt (20261124 > 20250101), stok tetap 80")

    # =========================================================================
    # FASE 3: U12 & U15 — Restart Aplikasi, Cek Persistensi & Laporan Rekapitulasi
    # =========================================================================
    session2_inputs = [
        # Menu 1 -> Submenu 1 (Cek Posko termuat) -> Pause -> Submenu 0
        "1", "1", "", "0",
        # Menu 11 (Cek Riwayat Transaksi di Linked List termuat) -> Pause
        "11", "",
        # Menu 6 -> Submenu 1 (Cek Realisasi Kebutuhan termuat) -> Pause -> Submenu 0
        "6", "1", "", "0",
        # U15: Menu 13 (Laporan Rekapitulasi Akhir) -> Pause
        "13", "",
        # Keluar
        "0",
    ]
    rc2, raw_out2, err_out2 = run_app(session2_inputs)
    out2 = strip_ansi(raw_out2)
    full_log.append("\n=== FASE 3 (SESI 2: RESTART U12 & LAPORAN REKAPITULASI U15) ===\n" + out2)

    # U12 check
    u12_ok = (
        rc2 == 0
        and "Data dimuat dari 'test/': 6 Posko, 8 Bantuan, 1 Korban, 3 Kebutuhan, 2 Transaksi." in out2
        and "P006" in out2 and "Desa Damai" in out2
        and "TRX0001" in out2 and "TRX0002" in out2
        and "Memori dinamis Linked List berhasil dibebaskan (0 leak)" in out2
    )
    record("U12", "Simpan Data, Tutup Aplikasi, dan Jalankan Ulang", u12_ok,
           "Restart memuat utuh 6 Posko, 8 Bantuan, 1 Korban, 3 Kebutuhan, 2 Transaksi dari file")

    # U15 check
    u15_ok = (
        "Total Terdampak        : 430 KK | 1360 Jiwa | 208 Kelompok Rentan" in out2
        and re.search(r"B001\s*\|\s*Beras Pangan Utama\s*\|\s*kg\s*\|\s*500\s*\|\s*200\s*\|\s*600", out2) is not None
        and re.search(r"TOTAL\s*\|\s*Akumulasi Semua Bantuan\s*\|\s*unit\s*\|\s*500\s*\|\s*200\s*\|\s*1630", out2) is not None
        and re.search(r"P003\s*\|\s*Desa Harapan\s*\|\s*1\s*\|\s*200", out2) is not None
        and "Total Permintaan: 750 unit | Total Realisasi: 200 unit | Sisa Kebutuhan: 550 unit" in out2
    )
    record("U15", "Laporan Rekapitulasi Akhir", u15_ok,
           "Total Masuk=500, Keluar=200, Sisa Stok B001=600 (Total=1630), Rekap P003=200, Akurat 100%")

    # =========================================================================
    # RESET FILE .DAT KE KONDISI SEED AWAL TERKALIBRASI UNTUK DEMO MANUAL
    # =========================================================================
    clean_dat_files()
    rc_reset, _, _ = run_app(["0"])

    # Urutkan hasil sesuai kode U01..U15
    results.sort(key=lambda item: int(item[0][1:]))

    summary_lines = [
        "=====================================================================================",
        "               HASIL PENGUJIAN OTOMATIS UAT DISASTERCARE (U01 - U15)                 ",
        "=====================================================================================",
        f"{'Kode':<5} | {'Status':<6} | {'Skenario Pengujian':<48} | Detail Aktual",
        "------+--------+--------------------------------------------------+------------------",
    ]
    passed = 0
    for code, name, ok, detail in results:
        st = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        summary_lines.append(f"{code:<5} | {st:<6} | {name:<48} | {detail}")
    summary_lines.append("=====================================================================================")
    summary_lines.append(f"TOTAL LULUS: {passed}/{len(results)} | Reset seed demo exit={rc_reset}")

    summary_text = "\n".join(summary_lines)
    print(summary_text)

    LOG_PATH.write_text(summary_text + "\n\n" + "\n".join(full_log), encoding="utf-8")
    print(f"\nLog lengkap tersimpan di: {LOG_PATH}")

    return 0 if (passed == len(results) and rc_reset == 0) else 1


if __name__ == "__main__":
    sys.exit(main())
