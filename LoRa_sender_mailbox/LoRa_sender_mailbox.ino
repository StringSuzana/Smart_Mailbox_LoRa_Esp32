#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include "stdio.h"

const int NSS_PIN = 5;
const int RESET_PIN = 4;
const int IO0_PIN = 2;

const int REED_PIN = 15; //Pin for reed switch

RTC_DATA_ATTR bool is_opened_state = false; //RTC_DATA_ATTR => Store data in RTC memory

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason; 

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void loRaSetup() {
  Serial.println("============ LoRa Sender ============");

  LoRa.setPins(NSS_PIN, RESET_PIN, IO0_PIN);
  while (!LoRa.begin(868E6)) { //868 MHz is frequency for LoRa communication in Europe
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0x30); //Sync word (Must be same as in receivers Sync word)

  Serial.println("LoRa Initializing OK!");
}

void sendMessageToLoRa(String message) {
  Serial.print("Sending packet: ");
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  delay(1000);
}

void notifyLoRa() {
  if (digitalRead(REED_PIN) == HIGH && is_opened_state == false) {
    sendMessageToLoRa("Opened");
    Serial.println("Opened");
    is_opened_state = true;
  } else if (digitalRead(REED_PIN) == LOW && is_opened_state == true) {
    Serial.println("Closed");
    sendMessageToLoRa("Closed");
    is_opened_state = false;
  }
}

void setup() {
  pinMode(REED_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println();

  loRaSetup();
  
  print_wakeup_reason();

  notifyLoRa();

  /*
  Enable sleep with WAKE UP trigger on REED_PIN toggled to be in different state than it is currently.
  */
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, !is_opened_state);

  Serial.println("Going to sleep now");
  /*
  Back to deep sleep
  */
  esp_deep_sleep_start();
}

void loop() { // Nothing to do in loop, because it is put to sleep in setup method
}