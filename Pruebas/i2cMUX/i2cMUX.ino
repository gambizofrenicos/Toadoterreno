#include "Wire.h"

// i2cMUX
#define TCAADDR 0x70
extern "C" {
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

// TOF
#include "Adafruit_VL6180X.h"
Adafruit_VL6180X vl = Adafruit_VL6180X();

// Color
#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);

// IMU
#include "I2Cdev.h"
#include "MPU6050.h"
MPU6050 mpu(0x68);
int ax, ay, az;
int gx, gy, gz;

// Selecciona el puerto del MUX a usar
void tcaselect(uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  pinMode(13, OUTPUT);
  while (!Serial);
  delay(1000);

  Wire.begin();

  Serial.begin(115200);
  //Serial.println("\nTCAScanner ready!");

  for (uint8_t t = 0; t < 8; t++) {
    tcaselect(t);
    //   Serial.print("TCA Port #"); Serial.println(t);

    for (uint8_t addr = 0; addr <= 127; addr++) {
      if (addr == TCAADDR) continue;

      uint8_t data;
      if (! twi_writeTo(addr, &data, 0, 1, 1)) {
        //      Serial.print("Found I2C 0x");  Serial.println(addr, HEX);
      }
    }
  }
  // Serial.println("\ndone");

  // Inicializacion TOF
  tcaselect(0);
  //Serial.println("Adafruit VL6180x test!");
  if (! vl.begin()) {
    //  Serial.println("Failed to find sensor");
    while (1);
  }
  //Serial.println("TOF Sensor found!");
  tcaselect(3);
  //Serial.println("Adafruit VL6180x test!");
  if (! vl.begin()) {
    //  Serial.println("Failed to find sensor");
    while (1);
  }

  // Inicializacion Color
  tcaselect(2);
  if (tcs.begin()) {
    // Serial.println("Found COLOR sensor");
  } else {
    //  Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  // Inicializacion IMU
  tcaselect(1);
  mpu.initialize();
  // Serial.println(mpu.testConnection() ? F("IMU iniciado correctamente") : F("Error al iniciar IMU"));

  // Serial.println();
}

// getTOFdata devuelve la informacion del TOF; seleccionando el puerto del MUX correspondiente
void getTOFdata(int tof_num) {
  int mux_num;

  switch (tof_num) {
    case 1:
      mux_num = 0;
      break;
    case 2:
      mux_num = -1;
      break;
    case 3:
      mux_num = 3;
      break;
    case 4:
      mux_num = -1;
      break;
    case 5:
      mux_num = -1;
      break;
    default:
      break;
  }
  
  tcaselect(mux_num);

  float lux = vl.readLux(VL6180X_ALS_GAIN_5);
  Serial.println("=== TOF ===");
  Serial.print("Lux: "); Serial.println(lux);

  uint8_t range = vl.readRange();
  uint8_t status = vl.readRangeStatus();

  if (status == VL6180X_ERROR_NONE) {
    Serial.print("Range: "); Serial.println(range);
  } else {
  }
  delay(50);
}

void getColorData() {
  tcaselect(2);
  // uint16_t r, g, b, c, colorTemp, lux;
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  //colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  //lux = tcs.calculateLux(r, g, b);
  /*
    Serial.println("=== COLOR ===");
    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");
  */

  if (r > 1.3 * g && r > 1.3 * b) {
    Serial.print("ROJO\t");
  } else if (b > 1.3 * r && b > 1.3 * g) {
    Serial.print("AZUL\t");
  } else if (g > 1.3 * r && g > 1.3 * b) {
    Serial.print("VERDE\t");
  }
  Serial.println();

  /*
    Serial.print("colorTemp:");
    Serial.print(colorTemp, DEC);
    Serial.print(",");
    Serial.print("lux:");
    Serial.print(lux, DEC);
    Serial.print(",");
    Serial.print("r:");
    Serial.print(r, DEC);
    Serial.print(",");
    Serial.print("g:");
    Serial.print(g, DEC);
    Serial.print(",");
    Serial.print("b:");
    Serial.print(b, DEC);
    Serial.print(",");
    Serial.print("mu:");
    Serial.print(mu, DEC);
    Serial.print(",");
    Serial.print("c:");
    Serial.println(c, DEC);
  */
  delayMicroseconds(20000);
  // delay(50);
}


void printTab()
{
  Serial.print(F("\t"));
}

void getIMUData() {
  tcaselect(1);
  // Leer las aceleraciones y velocidades angulares
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);
  Serial.println("=== IMU ===");
  Serial.print(F("a[x y z] g[x y z]:\t"));
  Serial.print(ax); printTab();
  Serial.print(ay); printTab();
  Serial.print(az); printTab();
  Serial.print(gx); printTab();
  Serial.print(gy); printTab();
  Serial.println(gz);
  delay(50);
}

void loop() {
  getTOFdata(1);
  //getColorData();
  //getIMUData();
 // getTOFdata(3);
}
