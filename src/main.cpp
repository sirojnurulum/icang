#include <Arduino.h>
#include "config.h"
#include "kitchen_safety.h"
#include "pump_control.h"

void setup() {
    Serial.begin(115200);
    Serial.println("\n======================================");
    Serial.println("Sistem Cerdas iCang - Inisialisasi");
    Serial.println("======================================");

    // Inisialisasi modul keamanan dapur
    setupKitchenSafety();

    // Inisialisasi modul kontrol pompa
    // Untuk menonaktifkan, komentari baris di bawah ini
    // setupPumpControl();

    Serial.println("--------------------------------------");
    Serial.println("Inisialisasi Selesai. Sistem berjalan.");
    Serial.println("--------------------------------------");
}

void loop() {
    // Jalankan loop untuk modul keamanan dapur
    loopKitchenSafety();

    // Jalankan loop untuk modul kontrol pompa
    // Untuk menonaktifkan, komentari baris di bawah ini
    // loopPumpControl();
}

