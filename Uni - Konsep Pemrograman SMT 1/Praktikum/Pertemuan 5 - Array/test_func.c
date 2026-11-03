
#include <stdio.h>

float rata_rata(int nilai[], int ukuran)
{
    int total = 0;

    for (int i = 0; i < ukuran; i++)
    {
        total += nilai[i];
    }

    return (float)total / ukuran;
}

void tampilkan_rata_rata(float average)
{
    printf("Average = %.2f\n", average);
}

// TODO Case 2
void tampilkan_nilai(int nilai[], int ukuran)
{
    // kerjain
    for (int i = 0; i < ukuran; i++) {
        printf("Hasil Nilai: %d\n", nilai[i]);
    }
}

// TODO Case 3
int nilai_tertinggi(int nilai[], int ukuran)
{
    // kerjain

    int highest = nilai[0];

    for (int i = 1; i < ukuran; i++) {
        if (nilai[i] > highest) {
            highest = nilai[i];
        }
    }
    return highest;
}

// TODO Case 4
void cek_kelulusan(float average)
{
    // kerjain
   if (average >= 70) {
        printf("Lulus!\n");
    } else {
        printf("Tidak Lulus!\n");
    }
}

int main()
{
    int nilai[6] = {75, 80, 55, 90, 65, 85};
    int ukuran = sizeof(nilai) / sizeof(nilai[0]);
    int pilihan;

    printf("1. Hitung rata-rata\n");
    printf("2. Tampilkan semua skor\n");
    printf("3. Cek skor tertinggi\n");
    printf("4. Cek lulus atau tidak\n");
    printf("Pilih: ");
    scanf("%d", &pilihan);

    switch (pilihan)
    {
    case 1:
    {
        float average = rata_rata(nilai, ukuran);
        tampilkan_rata_rata(average);
        break;
    }

    case 2:
        tampilkan_nilai(nilai, ukuran);
        break;

    case 3:
    {
        int highest = nilai_tertinggi(nilai, ukuran);
        printf("Highest score = %d\n", highest);
        break;
    }

    case 4:
    {
        float average = rata_rata(nilai, ukuran);
        cek_kelulusan(average);
        break;
    }

    default:
        printf("Invalid choice\n");
    }

    return 0;
}
