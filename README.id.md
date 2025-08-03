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
  - **Alarm Bahaya Desibel Tinggi:** Menggunakan buzzer aktif 12V untuk peringatan yang sangat keras saat api, gas, atau asap terdeteksi.
  - **Alarm Status Pompa:** Menggunakan speaker pasif untuk notifikasi putus-putus yang informatif (misalnya, saat menunggu air).
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
| 1x | Speaker Pasif | Memberikan alarm suara untuk status pompa. |
| 1x | Buzzer Aktif 12V (min. 110dB) | Memberikan alarm bahaya yang sangat keras. |
| 1x | Modul Deteksi Tegangan AC | Mendeteksi permintaan dari saklar tandon. |
| 1x | Sensor Level Kapasitif (XKC-Y25-NPN) | Memeriksa keberadaan air di pipa. |
| 1x | Sensor Aliran Air (YF-S201) | Memverifikasi dan mengukur aliran air. |
| 1x | Modul Relay 1-Channel 5V (min. 10A) | Saklar elektronik untuk mengontrol Pompa Air 220V. |
| 1x | Modul MOSFET P-Channel (mis. IRF5305S) | Untuk mengontrol Buzzer Aktif 12V. Alternatif: Modul Relay 5V. |
| 1x | Breadboard & Kabel Jumper | Untuk merangkai sirkuit. |

### Langkah 2: Perakitan dan Penyambungan Kabel

#### A. Koneksi Tegangan Rendah (ESP32 ke Modul)

Hubungkan semua sensor dan modul ke ESP32 sesuai tabel berikut. Perhatikan pin `VIN` (untuk 5V) dan `3V3` (untuk 3.3V).

| Komponen | Pin Komponen | Terhubung ke Pin ESP32 | Keterangan |
|:---|:---|:---|:---|
| **Sensor Api** | `VCC`, `GND`, `DO` | `3V3`, `GND`, `GPIO 27` | |
| **Sensor Gas MQ-6 (LPG)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 34` | |
| **Sensor Gas MQ-2 (Asap)** | `VCC`, `GND`, `AO` | `VIN`, `GND`, `GPIO 35` | |
| **Speaker Pasif (Alarm Pompa)** | `+`, `-` | `GPIO 25`, `GND` | |
| **Sensor Tegangan AC** | `VCC`, `GND`, `Signal/OUT` | `3V3`, `GND`, `GPIO 32` | |
| **Sensor Level Kapasitif** | `VCC (Coklat)`, `GND (Biru)`, `Signal (Hitam)` | `VIN`, `GND`, `GPIO 13` | Kabel Kuning (Mode) tidak perlu disambungkan. |
| **Sensor Aliran Air** | `VCC (Merah)`, `GND (Hitam)`, `Signal (Kuning)` | `VIN`, `GND`, `GPIO 12` | |
| **Modul Relay (Pompa)** | `VCC`, `GND`, `IN` | `VIN`, `GND`, `GPIO 26` | |
| **Modul MOSFET (Buzzer 12V)** | `VCC`, `GND`, `SIG/IN` | `3V3`, `GND`, `GPIO 14` | `VCC` bisa ke `3V3` atau `VIN` tergantung spesifikasi modul. |

#### B. Koneksi Tegangan Tinggi (AC 220V)

> **⚠️ PERINGATAN KERAS:** Bagian ini melibatkan listrik tegangan tinggi yang **SANGAT BERBAHAYA**. Kesalahan dapat menyebabkan cedera serius atau kematian. Jika Anda tidak 100% yakin, mintalah bantuan teknisi listrik profesional. **Pastikan sumber listrik utama (MCB) ke pompa sudah dimatikan sepenuhnya sebelum memulai.**

Sistem ini akan "mencegat" kabel perintah tunggal yang berasal dari saklar otomatis di tandon Anda.

1.  **Identifikasi Kabel:**
    - **Kabel Perintah:** Kabel Fasa tunggal yang keluar dari saklar otomatis tandon Anda dan menuju ke pompa.
    - **Kabel Netral & Ground:** Kabel yang langsung terhubung dari sumber PLN ke pompa.

2.  **Potong Kabel Perintah:** Potong `Kabel Perintah` di lokasi yang mudah dijangkau. Anda kini memiliki dua ujung:
    - **Ujung A:** Yang berasal **DARI** saklar tandon.
    - **Ujung B:** Yang menuju **KE** pompa.

3.  **Sambungkan Ujung A (Sumber Perintah):**
    - Sambungkan `Ujung A` ke terminal **`COM` (Common)** pada Modul Relay.
    - Buat cabang/jumper dari `Ujung A` dan sambungkan ke **salah satu terminal input AC** pada Sensor Deteksi Tegangan AC.

4.  **Lengkapi Sirkuit Sensor Deteksi AC:**
    - Sambungkan **terminal input AC lainnya** pada sensor ke **Kabel Netral** utama. Ini diperlukan agar sensor dapat bekerja.

5.  **Sambungkan Ujung B (Menuju Pompa):**
    - Sambungkan `Ujung B` ke terminal **`NO` (Normally Open)** pada Modul Relay.

Dengan cara ini, saat tandon kosong, `Ujung A` menjadi aktif. Sensor AC mendeteksinya dan memberitahu ESP32. Jika logika sistem mengizinkan, ESP32 akan mengaktifkan relay, menyambungkan `COM` ke `NO`, dan mengalirkan listrik ke pompa melalui `Ujung B`.

#### C. Koneksi Buzzer Aktif 12V

> **⚠️ PERHATIAN:** Buzzer ini membutuhkan catu daya 12V eksternal dan tidak bisa dihubungkan langsung ke ESP32. Gunakan modul kontrol seperti MOSFET atau Relay.

1.  **Hubungkan Modul MOSFET ke ESP32:** Sesuai tabel di atas, hubungkan pin `VCC`, `GND`, dan `SIG` (atau `IN`) dari modul MOSFET ke `3V3`, `GND`, dan `GPIO 14` di ESP32.
2.  **Hubungkan Catu Daya 12V ke Modul:**
    -   Hubungkan kutub **positif (+)** dari catu daya 12V ke terminal input daya pada modul (sering ditandai `VIN+`, `V+`, atau `Power+`).
    -   Hubungkan kutub **negatif (-)** dari catu daya 12V ke terminal input ground pada modul (sering ditandai `VIN-`, `GND`, atau `Power-`).
3.  **Hubungkan Buzzer ke Modul:**
    -   Hubungkan kabel **merah (+)** dari Buzzer Aktif ke terminal output pada modul (sering ditandai `OUT+` atau `Load+`).
    -   Hubungkan kabel **hitam (-)** dari Buzzer Aktif ke terminal output ground pada modul (sering ditandai `OUT-` atau `Load-`).
4.  **Pastikan Common Ground:** Sangat penting untuk menghubungkan pin `GND` dari ESP32, `GND` dari modul MOSFET, dan kutub **negatif (-)** dari catu daya 12V bersama-sama. Ini memastikan semua komponen memiliki titik referensi tegangan yang sama.

### Langkah 3: Setup Perangkat Lunak

1.  **Instalasi:**
    - Instal **Visual Studio Code**.
    - Instal ekstensi **PlatformIO IDE** dari dalam VS Code.
2.  **Buka Proyek:**
    - Clone repositori ini.
    - Buka folder proyek di VS Code (`File > Open Folder...`).
    - PlatformIO akan secara otomatis menginstal semua kebutuhan yang diperlukan.

### Langkah 4: Kalibrasi Sensor Gas (Wajib!)

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
