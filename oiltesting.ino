#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ข้อมูล WiFi ของคุณ
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// ตั้งค่า LCD I2C (แต่ละจอต้องใช้ Address ที่แตกต่างกัน)
LiquidCrystal_I2C lcd1(0x27, 16, 2); // หน้าจอ 1 แสดงเวลา
LiquidCrystal_I2C lcd2(0x26, 16, 2); // หน้าจอ 2 แสดงวันที่
LiquidCrystal_I2C lcd3(0x25, 16, 2); // หน้าจอ 3 แสดงราคาน้ำมัน
LiquidCrystal_I2C lcd4(0x24, 16, 2); // หน้าจอ 4 แสดงจำนวนเงินที่จ่าย
LiquidCrystal_I2C lcd5(0x23, 16, 2); // หน้าจอ 5 แสดงจำนวนลิตรที่ได้

// NTP Client ตั้งค่าโซนเวลา (Bangkok)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600);  // Bangkok time (UTC +7)

// ข้อมูลราคาน้ำมัน, จำนวนเงิน, จำนวนลิตร
float oilPrice = 34.50;  // ราคาน้ำมัน (ตัวอย่าง)
float totalAmount = 500.00;  // จำนวนเงินที่จ่าย (ตัวอย่าง)
float totalLiters = totalAmount / oilPrice;  // จำนวนลิตร

// กำหนดพินสำหรับ relay และ switch
const int relayPin = 14; // ใช้ GPIO14 (D5) ควบคุม relay
const int switchPin = 12; // ใช้ GPIO12 (D6) เป็น input จาก switch

void setup() {
  // เริ่มต้น Serial Monitor
  Serial.begin(115200);

  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // เริ่มต้น NTP Client
  timeClient.begin();
  
  // เริ่มต้นจอ LCD แต่ละจอ
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();
  lcd3.init();
  lcd3.backlight();
  lcd4.init();
  lcd4.backlight();
  lcd5.init();
  lcd5.backlight();

  // ตั้งค่าพิน relay ให้เป็น OUTPUT และ switch เป็น INPUT
  pinMode(relayPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP); // ใช้ PULLUP เพื่อตรวจจับสัญญาณ switch

  // ปิด relay ก่อนเริ่มต้น
  digitalWrite(relayPin, HIGH); // relay ใช้ logic low ในการเปิด
}

void loop() {
  // อัปเดตเวลา NTP
  timeClient.update();

  // ดึงข้อมูลเวลาและวันที่
  String formattedTime = timeClient.getFormattedTime();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;

  // หน้าจอที่ 1: แสดงเวลา
  lcd1.clear();
  lcd1.setCursor(0, 0);
  lcd1.print("Time:");
  lcd1.setCursor(0, 1);
  lcd1.print(formattedTime);

  // หน้าจอที่ 2: แสดงวันที่
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("Date:");
  lcd2.setCursor(0, 1);
  lcd2.print(day);
  lcd2.print("/");
  lcd2.print(month);
  lcd2.print("/");
  lcd2.print(year);

  // หน้าจอที่ 3: แสดงราคาน้ำมัน
  lcd3.clear();
  lcd3.setCursor(0, 0);
  lcd3.print("Oil Price:");
  lcd3.setCursor(0, 1);
  lcd3.print(oilPrice);
  lcd3.print(" THB/L");

  // หน้าจอที่ 4: แสดงจำนวนเงินที่จ่าย
  lcd4.clear();
  lcd4.setCursor(0, 0);
  lcd4.print("Amount Paid:");
  lcd4.setCursor(0, 1);
  lcd4.print(totalAmount);
  lcd4.print(" THB");

  // หน้าจอที่ 5: แสดงจำนวนลิตรที่ได้
  lcd5.clear();
  lcd5.setCursor(0, 0);
  lcd5.print("Liters:");
  lcd5.setCursor(0, 1);
  lcd5.print(totalLiters);
  lcd5.print(" L");

  // อ่านค่าจาก switch
  int switchState = digitalRead(switchPin);

  // ตรวจสอบการสั่งงานจาก switch
  if (switchState == LOW) { // เมื่อ switch กด (LOW) จะเปิด relay
    Serial.println("Switch pressed - Pump ON");
    digitalWrite(relayPin, LOW); // เปิด relay (จ่ายน้ำมัน)
  } else {
    Serial.println("Switch not pressed - Pump OFF");
    digitalWrite(relayPin, HIGH); // ปิด relay (หยุดจ่ายน้ำมัน)
  }

  // หน่วงเวลาเล็กน้อยเพื่อลดการสลับสถานะเร็วเกินไป
  delay(1000);
}
