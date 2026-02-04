#include <Arduino.h>
#include <pins.h>

#include <SPI.h>
#include <Servo.h>

// BMP390 (SPI) 
#include <Adafruit_BMP3XX.h>

// Flash (W25Q128) 
#include <Adafruit_SPIFlash.h>

// LoRa (RFM96W / SX127x) 
#include <LoRa.h>

// IMU (ICM-42670-P) 
#include <ICM42670P.h>

// put function declarations here:
int xPos = 0; // variable to store the x-axis servo position
int yPos = 0; // variable to store the y-axis servo position

Servo xServo;
Servo yServo;

Adafruit_BMP3XX bmp;

ICM42670P imu(SPI, IMU_CS);

Adafruit_FlashTransport_SPI flashTransport(MEM_CS, SPI);
Adafruit_SPIFlash flash(&flashTransport);

static const long LORA_FREQ = 433E6;

static void idleHigh() {
  // CS lines
  pinMode(MEM_CS, OUTPUT);   
  digitalWrite(MEM_CS, HIGH);
  
  pinMode(BARO_CS, OUTPUT);  
  digitalWrite(BARO_CS, HIGH);
  
  pinMode(IMU_CS, OUTPUT);   
  digitalWrite(IMU_CS, HIGH);
  
  pinMode(LORA_CS, OUTPUT);  
  digitalWrite(LORA_CS, HIGH);

  // Flash control pins
  pinMode(MEM_HOLD, OUTPUT);          
  digitalWrite(MEM_HOLD, HIGH);
  
  pinMode(MEM_WRITE_PROTECT, OUTPUT); 
  digitalWrite(MEM_WRITE_PROTECT, HIGH);

  // LEDs
  pinMode(RED, OUTPUT); 
  digitalWrite(RED, LOW);
  
  pinMode(GRN, OUTPUT); 
  digitalWrite(GRN, LOW);
  
  pinMode(YLW, OUTPUT); 
  digitalWrite(YLW, LOW);

  // Buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200); // baud rate = 115200 bits/s
  while (!Serial && millis() < 2000) {}

  // Teensy 4.0 default SPI pins: MOSI=11, MISO=12, SCK=13
  SPI.begin();

  idleHigh(); // set all CS lines high

  // Servo Setup
  xServo.attach(TVCX);
  yServo.attach(TVCY);

  // BMP390 Setup
  // begin_SPI(cs, spi, freq)
  if (!bmp.begin_SPI(BARO_CS, &SPI, 2000000)) {
    Serial.println("BMP390 init FAILED (SPI). Check BARO_CS wiring and SPI lines.");
  } else {
    Serial.println("BMP390 OK");
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  }

  // Flash Setup
  if (!flash.begin()) {
    Serial.println("Flash init FAILED. Check MEM_CS, HOLD, WP, and SPI lines.");
  } else {
    uint32_t jedec = flash.getJEDECID();
    Serial.print("Flash OK. JEDEC ID = 0x");
    Serial.println(jedec, HEX);
  }

  // LoRa Setup
  LoRa.setPins(LORA_CS, LORA_RESET, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa init FAILED. Check LORA_CS/LORA_RESET/LORA_DIO0 and freq.");
  } else {
    Serial.println("LoRa OK");
    LoRa.setTxPower(17);            // adjust if needed
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.enableCrc();
  }
  
  // IMU Setup
  int imuStatus = imu.begin();
  if (imuStatus != 0) {
    Serial.print("ICM42670P init FAILED. status=");
    Serial.println(imuStatus);
  } else {
    Serial.println("ICM42670P OK");
    // startAccel(ODR_Hz, FSR_g), startGyro(ODR_Hz, FSR_dps)
    imu.startAccel(200, 16);
    imu.startGyro(200, 2000);
  }

  Serial.println("--- Bring-up complete ---\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  bool bmp_ok = bmp.performReading();

  // ---- IMU read ----
  inv_imu_sensor_event_t evt;
  int imu_rc = imu.getDataFromRegisters(&evt);

  // ---- Print ----
  if (bmp_ok) {
    Serial.print("BMP T="); Serial.print(bmp.temperature, 2);
    Serial.print("C P=");   Serial.print(bmp.pressure, 1);
    Serial.print("Pa");
  } else {
    Serial.print("BMP read FAIL");
  }

  Serial.print(" | ");

  if (imu_rc == 0) {
    // Data validity helpers exist in your header — use them
    if (imu.isAccelDataValid(&evt)) {
      Serial.print("a[");
      Serial.print(evt.accel[0]); Serial.print(",");
      Serial.print(evt.accel[1]); Serial.print(",");
      Serial.print(evt.accel[2]); Serial.print("] ");
    } else {
      Serial.print("a[invalid] ");
    }

    if (imu.isGyroDataValid(&evt)) {
      Serial.print("g[");
      Serial.print(evt.gyro[0]); Serial.print(",");
      Serial.print(evt.gyro[1]); Serial.print(",");
      Serial.print(evt.gyro[2]); Serial.print("] ");
    } else {
      Serial.print("g[invalid] ");
    }
  } else {
    Serial.print("IMU read FAIL rc=");
    Serial.print(imu_rc);
  }

  Serial.println();

  // ---- Periodic LoRa TX (1 Hz) ----
  static uint32_t lastTx = 0;
  if (LoRa.beginPacket(), true) { /* no-op to keep formatting simple */ }
  if (millis() - lastTx > 1000) {
    lastTx = millis();
    LoRa.beginPacket();
    LoRa.print("t=");
    LoRa.print(lastTx);
    if (bmp_ok) {
      LoRa.print(" P=");
      LoRa.print((int)bmp.pressure);
    }
    LoRa.endPacket();
  }

  // ---- Non-blocking servo wiggle (for bench test only) ----
  static uint32_t lastServo = 0;
  static int x = 90, y = 90;
  static int dx = 1, dy = 1;

  if (millis() - lastServo > 20) {
    lastServo = millis();
    x += dx; 
    y += dy;
    if (x >= 180 || x <= 0) dx = -dx;
    if (y >= 180 || y <= 0) dy = -dy;
    xServo.write(x);
    yServo.write(y);
  }

  delay(20);
}
