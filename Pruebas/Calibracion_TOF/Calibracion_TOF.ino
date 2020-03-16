#include "Wire.h"

// i2cMUX
#define TCAADDR 0x70
extern "C" {
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

// TOF
#include "Adafruit_VL6180X.h"
Adafruit_VL6180X vl = Adafruit_VL6180X();


// Selecciona el puerto del MUX a usar
void tcaselect(uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  while (!Serial);
  delay(1000);

  Wire.begin();

  Serial.begin(115200);
  Serial.println("\nTCAScanner ready!");

  for (uint8_t t = 0; t < 8; t++) {
    tcaselect(t);
    Serial.print("TCA Port #"); Serial.println(t);

    for (uint8_t addr = 0; addr <= 127; addr++) {
      if (addr == TCAADDR) continue;

      uint8_t data;
      if (! twi_writeTo(addr, &data, 0, 1, 1)) {
        Serial.print("Found I2C 0x");  Serial.println(addr, HEX);
      }
    }
  }
  Serial.println("\ndone");

  // Inicializacion TOFs
  for (int i = 0; i < 5; i++) {
    tcaselect(i);
    Serial.println("Adafruit VL6180x test!");
    if (! vl.begin()) {
      Serial.print("Failed to find sensor on port: "); Serial.println(i);
    }
    Serial.print("TOF Sensor found on port: "); Serial.println(i);
  }

}

void getTOFdata(int tof_num) {
  int mux_num;

  switch (tof_num) {
    case 1:
      mux_num = 4;
      break;
    case 2:
      mux_num = 3;
      break;
    case 3:
      mux_num = 2;
      break;
    case 4:
      mux_num = 0;
      break;
    case 5:
      mux_num = 1;
      break;
    default:
      break;
  }

  tcaselect(mux_num);

  float lux = vl.readLux(VL6180X_ALS_GAIN_5);
  uint8_t range = vl.readRange();
  uint8_t status = vl.readRangeStatus();

  if (status == VL6180X_ERROR_NONE) {
    Serial.print(lux); Serial.print("\t"); Serial.print(range); Serial.print("\t");
  }

}


void loop() {
  for (int j = 0; j < 100; j++) {
    for (int i = 1; i <= 5; i++) {
      getTOFdata(i);
    }
    Serial.println();
  }
  while(1);
}
