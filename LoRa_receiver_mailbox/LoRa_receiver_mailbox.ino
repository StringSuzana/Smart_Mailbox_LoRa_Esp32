#include <iostream>
#include <string>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SPI.h>
#include <LoRa.h>
#include "Properties.h"
using namespace std;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const int counter = 0;
const int NSS_PIN = 5;
const int RESET_PIN = 4;
const int IO0_PIN = 2;


void sendMessageToTelegram(String message) {
  Serial.println("sendMessageToTelegram");
  Serial.println(message);

  String from_name = bot.messages[1].from_name;

  if (from_name == "")
    from_name = "Guest";

  bot.sendMessage(CHAT_ID, "Mailbox door status from ESP32 over LoRa:");
  bot.sendMessage(CHAT_ID, message);
}
void loRaSetup() {
  Serial.println("============ LoRa Receiver ============");
  LoRa.setPins(NSS_PIN, RESET_PIN, IO0_PIN);
  while (!LoRa.begin(868E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void wifiSetup() {
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}
void timeSetup() {

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org");  // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void setup() {

  Serial.begin(115200);
  Serial.println();

  loRaSetup();
  wifiSetup();
  timeSetup();
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    String status = "";

    while (LoRa.available()) {  //Reading LoRa packet
      char letter = (char)LoRa.read();
      status.concat(letter);
    }
    status.concat(" with RSSI ");
    status.concat(String(LoRa.packetRssi()));

    sendMessageToTelegram(status);

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.print(LoRa.packetRssi());

    Serial.print(" with SNR ");
    Serial.println(LoRa.packetSnr());

    Serial.print(" with SNR ");
    Serial.println(LoRa.packetFrequencyError());
  }
}