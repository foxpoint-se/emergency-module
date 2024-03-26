#include <SPI.h>
#include <LoRa.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "emergencyModule.h"

#define SERVO_PIN 3
#define LED_PIN 2

Servo magnetServo;
Adafruit_INA219 ina219;

unsigned long TIME_NOW = 0;

bool EMERGENCY = 0;
const int GPS_PERIOD_MS = 1000;
unsigned long LAST_GPS_TS = 0;
unsigned long LAST_VALID_GPS_TS = 0;
const unsigned long EMERGENCY_TIME_MS = 120000;

const float MAX_BATTERY_VOLTAGE = 4.2;
const float MIN_BATTERY_VOLTAGE = 3.4;
const float MIN_BATTERY_MAH = 240.0;
const float MAX_BATTERY_MAH = 1200;

int BROADCAST_PERIOD_MS = 10000;
unsigned long LAST_BROADCAST_TS = 0;

struct GPSData gps_data = {" ", 0.0, 'F', 0.0, 'F', 0, 0, 0.0, 0.0, false};

float getBatteryPercentage(float voltage_reading);
float getBatteryTimeLeft(float voltage_reading, float current_reading);

const int DIM_FREQ_MS = 30;
const int LED_MAX_BRIGHT_LVL = 255;
const int DIM_LEVEL = 5;
const int LED_BLINK_FREQ = 60000;
unsigned long LAST_LED_BLINK_TS = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);

  magnetServo.attach(SERVO_PIN);
  magnetServo.writeMicroseconds(1000);
  
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  delay(1000);
  Serial.println("Initiation complete, starting program");
}

void loop() {
  int servo_value = 0;
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
  float battery_percentage = 0.0;
  float battery_time_left = 0.0;
  
  TIME_NOW = millis();

  // Parse GPS data
  if ((TIME_NOW - LAST_GPS_TS) > GPS_PERIOD_MS){
    gps_data = getPos();

    LAST_GPS_TS = TIME_NOW;

    // If we recieved a valid gps position, update the last gps time stamp
    if (gps_data.valid) {
      LAST_VALID_GPS_TS = TIME_NOW;
    }
  }

  // Check if the time since last gps coordinate exceeds the set emergency value (10 mins)
  if (((TIME_NOW - LAST_VALID_GPS_TS) > EMERGENCY_TIME_MS) && !EMERGENCY) {
    EMERGENCY = 1;

    // Move servo back and forth to ensure a detachment from Ålen
    TIME_NOW = millis();
    magnetServo.writeMicroseconds(2000);
    while((millis() - TIME_NOW) < 2000);
    
    TIME_NOW = millis();
    magnetServo.writeMicroseconds(1000);
    while((millis() - TIME_NOW) < 2000);
    
    TIME_NOW = millis();
    magnetServo.writeMicroseconds(2000);
    while((millis() - TIME_NOW) < 2000);

    magnetServo.writeMicroseconds(1000);
  }

  // Check if there we are in emergency mode and it is time to flash LEDS
  if (EMERGENCY && ((TIME_NOW - LAST_LED_BLINK_TS) > LED_BLINK_FREQ)){
    for(int i = 0; i < 5; i++){
      fadeLEDOn();
      fadeLEDOff();
    }
    
    LAST_LED_BLINK_TS = TIME_NOW;
  }

  // Send Lora message
  if (((TIME_NOW - LAST_BROADCAST_TS) > BROADCAST_PERIOD_MS) && gps_data.valid){
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    loadvoltage = busvoltage + (shuntvoltage / 1000);
    battery_percentage = getBatteryPercentage(busvoltage);
    battery_time_left = getBatteryTimeLeft(busvoltage, current_mA);
    
    LoRa.beginPacket();
    LoRa.print(gps_data.UTCtime);
    LoRa.print("\n");
    LoRa.print(EMERGENCY);
    LoRa.print("\n");
    LoRa.print(gps_data.longitude, 6);
    LoRa.print("\n");
    LoRa.print(gps_data.latitude, 6);
    LoRa.print("\n");
    LoRa.print(battery_percentage);
    LoRa.print("\n");
    LoRa.endPacket();
    
    LAST_BROADCAST_TS = TIME_NOW;
  }
}

float getBatteryPercentage(float voltage_reading) {
  const float incremental_level = (MAX_BATTERY_VOLTAGE- MIN_BATTERY_VOLTAGE) / 20.0;
   
  float battery_percentage = 0.0;
  float check_level = 3.4;
  float counter = 0.0;

  if (voltage_reading >= MAX_BATTERY_VOLTAGE) {
    battery_percentage = 100.0;

  }else if (voltage_reading <= MIN_BATTERY_VOLTAGE){
    battery_percentage = 0.0;

  }else {
    
    while(battery_percentage == 0.0){
      float next_level = check_level + incremental_level;
      
      if (next_level > voltage_reading && voltage_reading >= check_level) {
        battery_percentage = counter * 5.0;
        break;
      }

      check_level = next_level;
      counter = counter + 1.0;
    }
        
  }

  return battery_percentage;
}

float getBatteryTimeLeft(float voltage_reading, float current_reading) {
  float mAh_left = 0.0;
  float battery_time_left = 0.0;

  mAh_left = map(voltage_reading, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE, MIN_BATTERY_MAH, MAX_BATTERY_MAH);

  battery_time_left = mAh_left / current_reading;

  return battery_time_left;
}

void fadeLEDOn(){
  int dim_value = 0;
  unsigned long last_led_cmd = millis();
  unsigned long now = millis();

  while(dim_value < LED_MAX_BRIGHT_LVL){
    now = millis();
    
    if((now - last_led_cmd) > DIM_FREQ_MS){
      dim_value += DIM_LEVEL;
      analogWrite(LED_PIN, dim_value);

      last_led_cmd = now;
    }
  }
}

void fadeLEDOff(){
  int dim_value = LED_MAX_BRIGHT_LVL;
  unsigned long last_led_cmd = millis();
  unsigned long now = millis();

  while(dim_value > 0){
    now = millis();
    
    if((now - last_led_cmd) > DIM_FREQ_MS){
      dim_value -= DIM_LEVEL;
      analogWrite(LED_PIN, dim_value);

      last_led_cmd = now;
    }
  }  
}
