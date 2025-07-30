# Proyek Sistem Keamanan Rumah Multi-Sensor dengan ESP32

Sistem keamanan rumah pintar berbasis ESP32 yang dirancang untuk mendeteksi berbagai potensi bahaya secara real-time. Proyek ini mengintegrasikan beberapa sensor untuk memonitor kondisi lingkungan dan memberikan peringatan dini melalui alarm suara yang dinamis.

## 📜 Status Proyek

**Status:** `Prototipe Fungsional`

Sistem saat ini berhasil mendeteksi api, gas LPG, dan asap, serta mampu mengaktifkan alarm suara sesuai dengan jenis ancaman. Pengembangan selanjutnya akan berfokus pada penambahan fitur konektivitas dan sensor lainnya.

## Fitur

- **Deteksi Nyala Api:** Menggunakan sensor inframerah (IR) untuk mendeteksi keberadaan api secara cepat.
- **Deteksi Kebocoran Gas LPG:** Menggunakan sensor gas MQ-6 yang dioptimalkan untuk mendeteksi Propana dan Butana.
- **Deteksi Asap:** Menggunakan sensor gas MQ-2 untuk mendeteksi asap dari potensi kebakaran.
- **Alarm Suara Cerdas:** Speaker mengeluarkan nada yang berbeda untuk setiap jenis bahaya, memungkinkan identifikasi ancaman dengan cepat.
- **Monitoring Real-time:** Status semua sensor dapat dipantau secara langsung melalui Serial Monitor untuk debugging dan kalibrasi.
- **Logika State Machine:** Mencegah alarm berbunyi terus-menerus secara tidak efisien dengan hanya mengaktifkan atau menonaktifkan alarm saat status berubah.

## ⚙️ Kebutuhan Hardware

- 1x ESP32 DevKit V4 (atau yang kompatibel)
- 1x Sensor Api (Flame Sensor Module, Digital Output)
- 1x Sensor Gas MQ-6 (LPG)
- 1x Sensor Gas MQ-2 (Asap)
- 1x Speaker Pasif (Passive Speaker)
- 1x Breadboard
- Kabel Jumper

## 🔌 Diagram Koneksi

Pastikan semua komponen terhubung ke ESP32 sesuai dengan tabel berikut:

| Komponen | Pin Komponen | Terhubung ke Pin ESP32 | Keterangan |
| :--- | :--- | :--- | :--- |
| **Sensor Api** | `VCC` | `3V3` | Catu daya 3.3V. |
| | `GND` | `GND` | Terhubung ke ground. |
| | `DO` | `GPIO 27` | Sinyal deteksi digital. |
| **Sensor MQ-6 (LPG)** | `VCC` | `VIN` | **Penting:** Membutuhkan 5V untuk pemanas. |
| | `GND` | `GND` | Terhubung ke ground. |
| | `AO` | `GPIO 34` | Sinyal konsentrasi gas analog. |
| **Sensor MQ-2 (Asap)** | `VCC` | `VIN` | Berbagi daya 5V dengan MQ-6. |
| | `GND` | `GND` | Terhubung ke ground. |
| | `AO` | `GPIO 35` | Sinyal konsentrasi asap analog. |
| **Speaker Pasif** | `Positif (+)` | `GPIO 25` | Sinyal nada dari ESP32. |
| | `Negatif (-)` | `GND` | Terhubung ke ground. |

## 🛠️ Kebutuhan Software

- Visual Studio Code
- Ekstensi PlatformIO IDE untuk VSCode

## 🚀 Setup & Instalasi

1.  Clone repositori ini ke komputer Anda.
2.  Buka folder proyek menggunakan Visual Studio Code.
3.  PlatformIO akan secara otomatis mendeteksi file `platformio.ini` dan menginstal framework Arduino untuk ESP32.

## 🔬 Konfigurasi & Kalibrasi (Langkah Kritis!)

Sensor gas MQ memerlukan kalibrasi untuk berfungsi dengan benar di lingkungan Anda dan menghindari alarm palsu.

1.  **Pemanasan Sensor (Burn-in):** Setelah merangkai dan mengunggah kode untuk pertama kali, biarkan perangkat menyala tanpa gangguan selama **minimal 15-30 menit**. Ini sangat penting agar elemen pemanas di dalam sensor gas mencapai suhu kerja yang stabil.
2.  **Amati Nilai Udara Bersih:** Buka **Serial Monitor** di PlatformIO (pastikan baud rate diatur ke `115200`). Amati nilai `LPG (MQ-6)` dan `Asap (MQ-2)` dalam kondisi udara yang bersih dan berventilasi baik. Catat nilai rata-ratanya setelah stabil (misalnya, sekitar 1500).
3.  **Atur Ambang Batas:**
    - Buka file `src/main.cpp`.
    - Cari konstanta `LPG_THRESHOLD` dan `SMOKE_THRESHOLD`.
    - Ubah nilainya menjadi angka yang **secara signifikan lebih tinggi** dari nilai udara bersih yang Anda catat. Sebagai titik awal yang aman, tambahkan 500-700 dari nilai normal. Contoh: jika nilai normal adalah 1500, atur threshold menjadi `2200`.
    ```cpp
    const int LPG_THRESHOLD   = 2200; // Sesuaikan nilai ini!
    const int SMOKE_THRESHOLD = 2200; // Sesuaikan nilai ini!
    ```
4.  **Unggah Ulang:** Simpan perubahan dan unggah kembali kode ke ESP32.

## 🕹️ Cara Menggunakan

1.  Pastikan semua komponen terhubung dengan benar sesuai diagram.
2.  Hubungkan ESP32 ke komputer melalui kabel USB.
3.  Di VSCode, gunakan PlatformIO untuk **Build** dan **Upload** proyek.
4.  Setelah unggah selesai, buka **Serial Monitor** untuk memantau status sensor.
5.  Sistem sekarang aktif. Coba dekatkan sumber api, gas (dari korek yang tidak dinyalakan), atau asap untuk menguji setiap alarm.

## 💡 Potensi Pengembangan

- **Konektivitas IoT:** Menambahkan koneksi Wi-Fi untuk mengirim notifikasi alarm ke ponsel melalui Telegram, Pushover, atau MQTT.
- **Alarm Berkedip:** Membuat pola alarm yang lebih dinamis (misalnya, suara putus-putus) dengan menggunakan `millis()` untuk menghindari penggunaan `delay()` yang memblokir.
- **Tampilan Lokal:** Mengintegrasikan layar OLED atau LCD untuk menampilkan status sensor secara langsung pada perangkat.
- **Sensor Aliran Air:** Menambahkan sensor aliran air (water flow sensor) untuk mendeteksi penggunaan air PDAM, sesuai rencana awal proyek.
- **Data Logging:** Menyimpan data sensor ke kartu SD atau SPIFFS untuk analisis historis.

## 📄 Lisensi

Proyek ini dilisensikan di bawah Lisensi MIT.

---
*Dibuat dengan semangat eksplorasi dan keamanan.*