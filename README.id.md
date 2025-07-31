# Sistem Rumah Pintar Multi-Modul dengan ESP32

## 🎯 Visi Proyek

Proyek ini adalah sebuah inisiatif *open-source* untuk mengubah rumah konvensional menjadi rumah pintar (*smart home*) melalui pengembangan modul-modul otomatis yang praktis dan bermanfaat. Tujuannya adalah menciptakan sistem yang andal, mudah diperluas, dan dapat diadaptasi untuk berbagai kebutuhan otomatisasi rumah.

**Status Saat Ini:** `Prototipe Fungsional`

---

## ✨ Fitur Utama

- **Deteksi Bahaya Komprehensif:**
  - **Api:** Deteksi cepat menggunakan sensor inframerah.
  - **Gas LPG:** Deteksi kebocoran gas menggunakan sensor MQ-6.
  - **Asap:** Deteksi asap dari potensi kebakaran menggunakan sensor MQ-2.
- **Kontrol Pompa Air Cerdas:**
  - **Verifikasi Dua Langkah:** Mencegah pompa berjalan kering dengan memeriksa **keberadaan air** di pipa terlebih dahulu, lalu memverifikasi **aliran air** setelah pompa menyala.
  - **Monitoring Aliran:** Mengukur dan menampilkan laju aliran air (Liter per Menit) saat pompa beroperasi.
- **Sistem Peringatan Cerdas:**
  - **Alarm Bahaya:** Nada alarm yang berbeda untuk setiap jenis bahaya (api, gas, asap).
  - **Alarm Pompa:** Alarm putus-putus yang khas jika tandon meminta air tetapi pasokan dari sumber (PDAM) tidak tersedia.
- **Arsitektur Kode Profesional:**
  - **Non-Blocking:** Seluruh sistem berjalan tanpa `delay()` yang mengganggu, memastikan responsivitas tinggi.
  - **State Machine:** Mengelola status setiap modul (alarm, pompa) secara efisien dan terstruktur.

---

## 📖 Panduan Instalasi dan Penggunaan

Ikuti langkah-langkah berikut secara berurutan untuk merakit dan menjalankan sistem.

### Langkah 1: Kebutuhan Perangkat Keras

| Jumlah | Komponen | Fungsi |
|:------:|:---|:---|
| 1x | ESP32 DevKit V4 (atau kompatibel) | Otak dari keseluruhan sistem. |
| 1x | Sensor Api (Flame Sensor) | Mendeteksi nyala api. |
| 1x | Sensor Gas MQ-6 | Mendeteksi kebocoran gas LPG. |
| 1x | Sensor Gas MQ-2 | Mendeteksi asap. |
| 1x | Speaker Pasif | Memberikan alarm suara. |
| 1x | Modul Deteksi Tegangan AC | Mendeteksi permintaan dari saklar tandon. |
| 1x | Sensor Level Kapasitif (XKC-Y25-NPN) | Memeriksa keberadaan air di pipa. |
| 1x | Sensor Aliran Air (YF-S201) | Memverifikasi dan mengukur aliran air. |
| 1x | Modul Relay 1-Channel 5V (min. 10A) | Saklar elektronik untuk mengontrol pompa. |
| 1x | Breadboard & Kabel Jumper | Untuk merangkai sirkuit. |

### Langkah 2: Perakitan dan Penyambungan Kabel

#### A. Koneksi Tegangan Rendah (ESP32 ke Modul)

Hubungkan semua sensor dan modul ke ESP32 sesuai tabel berikut. Perhatikan pin `VIN` (untuk 5V) dan `3V3` (untuk 3.3V).

| Komponen | Pin Komponen | Terhubung ke Pin ESP32 |
|:---|:---|:---|
| **Sensor Api** | `VCC`, `GND`, `DO` | `3V3`, `GND`, `GPIO 27` |
| **Sensor Gas MQ-6 (LPG)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 34` |
| **Sensor Gas MQ-2 (Asap)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 35` |
| **Speaker Pasif** | `+`, `-` | `GPIO 25`, `GND` |
| **Sensor Tegangan AC** | `VCC`, `GND`, `Signal/OUT` | `3V3`, `GND`, `GPIO 32` |
| **Sensor Level Kapasitif** | `VCC (Coklat)`, `GND (Biru)`, `Signal (Hitam)` | `VIN`, `GND`, `GPIO 13` |
| **Sensor Aliran Air** | `VCC (Merah)`, `GND (Hitam)`, `Signal (Kuning)` | `VIN`, `GND`, `GPIO 12` |
| **Modul Relay** | `VCC`, `GND`, `IN` | `VIN`, `GND`, `GPIO 26` |

#### B. Koneksi Tegangan Tinggi (AC 220V)

> **⚠️ PERINGATAN KERAS:** Bagian ini melibatkan listrik tegangan tinggi yang **SANGAT BERBAHAYA**. Kesalahan dapat menyebabkan cedera serius atau kematian. Jika Anda tidak 100% yakin, mintalah bantuan teknisi listrik profesional. **Pastikan sumber listrik utama (MCB) ke pompa sudah dimatikan sepenuhnya sebelum memulai.**

Sistem ini akan "mencegat" sinyal dari saklar tandon Anda.

1.  **Identifikasi Kabel:**
    - **Kabel Sumber Fasa & Netral:** Dari sumber listrik PLN.
    - **Kabel Permintaan:** Kabel Fasa yang keluar dari saklar otomatis tandon Anda.

2.  **Sambungkan "Kabel Permintaan" ke Dua Tempat:**
    - **Ke Sensor Deteksi AC:** Sambungkan `Kabel Permintaan` ke satu input sensor, dan `Kabel Sumber Netral` ke input lainnya. Ini memberitahu ESP32 bahwa tandon meminta air.
    - **Ke Modul Relay:** Buat cabang dari `Kabel Permintaan` dan sambungkan ke terminal **`COM`** pada relay.

3.  **Sambungkan Relay ke Pompa:**
    - Sambungkan terminal **`NO` (Normally Open)** pada relay ke terminal **Fasa** Pompa Air.

4.  **Sambungkan Netral Pompa:**
    - Sambungkan `Kabel Sumber Netral` langsung ke terminal **Netral** Pompa Air.

### Langkah 3: Setup Notifikasi Telegram

Untuk menerima notifikasi real-time di ponsel Anda, Anda perlu membuat sebuah Bot Telegram.

1.  **Buat Bot Baru:**
    - Di aplikasi Telegram, cari akun bernama `BotFather` (ada tanda centang biru).
    - Mulai percakapan dan ketik `/newbot`.
    - Ikuti instruksinya: beri nama untuk bot Anda (misal: "Alarm Rumah Pintar") dan username (harus diakhiri `bot`, misal: `AlarmRumahPintarBot`).
    - **BotFather** akan memberikan Anda sebuah **Token API** rahasia. Salin dan simpan token ini baik-baik.

2.  **Dapatkan Chat ID:**
    - **Untuk Notifikasi Pribadi:** Cari bot `userinfobot`, mulai percakapan, dan ia akan langsung memberikan **Chat ID** numerik Anda.
    - **Untuk Notifikasi Grup:**
        1. Buat grup baru di Telegram.
        2. Tambahkan bot yang baru saja Anda buat sebagai anggota grup.
        3. Tambahkan `userinfobot` ke dalam grup.
        4. Ketik `/my_id` di dalam grup. `userinfobot` akan membalas dengan **Chat ID** grup tersebut. ID ini akan berupa angka negatif (contoh: `-1001234567890`).

3.  **Perbarui Kode Program:**
    - Buka file `src/main.cpp`.
    - Cari bagian `WiFi & Telegram Configuration`.
    - Ganti nilai placeholder untuk `WIFI_SSID`, `WIFI_PASSWORD`, `TELEGRAM_BOT_TOKEN`, dan `TELEGRAM_CHAT_ID` dengan kredensial Anda yang sebenarnya.

### Langkah 4: Setup Perangkat Lunak

1.  **Instalasi:**
    - Instal **Visual Studio Code**.
    - Instal ekstensi **PlatformIO IDE** dari dalam VS Code.
2.  **Buka Proyek:**
    - Clone repositori ini.
    - Buka folder proyek di VS Code (`File > Open Folder...`).
    - PlatformIO akan secara otomatis menginstal semua kebutuhan yang diperlukan.

### Langkah 5: Kalibrasi Sensor Gas (Wajib!)

Sensor gas tidak akan akurat tanpa kalibrasi.

1.  **Pemanasan (Burn-in):** Setelah merakit dan mengunggah kode, biarkan perangkat menyala selama **15-30 menit** agar sensor gas stabil.
2.  **Amati Nilai Normal:** Buka **Serial Monitor** di PlatformIO (Baud Rate: `115200`). Di udara bersih, catat nilai rata-rata `LPG (MQ-6)` dan `Asap (MQ-2)`. Nilainya mungkin sekitar 1500.
3.  **Atur Ambang Batas:** Buka file `src/main.cpp` dan ubah nilai `LPG_THRESHOLD` dan `SMOKE_THRESHOLD` menjadi angka yang lebih tinggi dari nilai normal (misalnya, nilai normal + 500).
4.  **Unggah Ulang:** Simpan dan unggah kembali kode Anda.

### Langkah 5: Penggunaan dan Monitoring

1.  **Unggah Kode:** Klik ikon **Upload (→)** pada status bar PlatformIO.
2.  **Monitor Sistem:** Klik ikon **Serial Monitor (🔌)** untuk melihat log sistem secara real-time.
3.  **Memahami Log Pompa:**
    - `IDLE`: Sistem normal, menunggu permintaan.
    - `MENUNGGU AIR`: Tandon minta air, tapi pipa PDAM kosong. Alarm putus-putus akan aktif.
    - `TESTING`: Pipa terdeteksi ada air, pompa menyala untuk tes verifikasi aliran.
    - `RUNNING`: Tes berhasil, pompa berjalan normal. Laju aliran akan ditampilkan.
    - `LOCKED OUT`: Tes gagal (pompa macet/aliran sangat lemah), pompa dimatikan selama 15 menit.

---

## 💡 Potensi Pengembangan

- **Konektivitas IoT:** Menambahkan notifikasi ke Telegram atau MQTT jika ada alarm atau pompa gagal.
- **Tampilan Lokal:** Mengintegrasikan layar OLED untuk menampilkan status tanpa perlu komputer.
- **Data Logging:** Menyimpan riwayat kejadian ke kartu SD atau memori internal.

## 🤝 Kontribusi

Proyek ini bersifat *open-source* dan kontribusi sangat diharapkan. Jangan ragu untuk membuat *pull request* atau membuka *issue*.

## ⚠️ Penafian (Disclaimer)

Perangkat lunak ini disediakan 'sebagaimana adanya' (*as is*). Penggunaan, modifikasi, atau penjualan sistem yang dibangun berdasarkan kode ini sepenuhnya menjadi tanggung jawab pengguna. Para kontributor tidak bertanggung jawab atas segala kerusakan, kerugian, atau penyalahgunaan yang mungkin timbul dari penggunaan sistem ini.

## 📄 Lisensi

Proyek ini dilisensikan di bawah **Lisensi MIT**.

---
*Dibuat dengan semangat eksplorasi dan keamanan.*