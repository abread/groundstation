// !!!CORRIGIR FREQUÊNCIA!!!
#define RADIO_FREQUENCY 433000000 /* Hz */

#define RADIO_SS_PIN 10
#define RADIO_IRQ_PIN 3
#define RADIO_NET_ID 100 // 0-255, must be the same on all nodes
#define RADIO_NODE_ID 1 // 0-254, must be unique in network, 255=broadcast
#define RADIO_ATC_RSSI -80

#include <RFM69.h>
#include <RFM69_ATC.h>

RFM69_ATC radio = RFM69_ATC(RADIO_SS_PIN, RADIO_IRQ_PIN);

void write_data(uint8_t *data, size_t len) {
  Serial.print("data: ");
  for (size_t i = 0; i < len; i++) {
    uint8_t b = data[i];

    // Encode newlines as \n\n (double-newline)
    if (b == '\n') {
      Serial.print("\n\n");
    } else {
      Serial.write(data[i]);
    }
  }
  Serial.println();
}

void write_rssi(int rssi) {
  Serial.print("rssi: ");
  Serial.println(rssi);
}

unsigned long lastUpdate;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(19200);
  radio.initialize(RF69_433MHZ, RADIO_NODE_ID, RADIO_NET_ID);
  radio.setHighPower();
  radio.encrypt(null);
  radio.enableAutoPower(RADIO_ATC_RSSI);
  radio.setFrequency(RADIO_FREQUENCY);

  digitalWrite(LED_BUILTIN, LOW);
  write_rssi(-999);
  lastUpdate = millis();
}

void loop() {
  if (radio.receiveDone()) {
    digitalWrite(LED_BUILTIN, HIGH);
    write_rssi(radio.RSSI);
    write_data(radio.DATA, radio.DATALEN);

    if (radio.ACKRequested()) {
      radio.sendACK();
      radio.sendACK();
    }
    digitalWrite(LED_BUILTIN, LOW);
    lastUpdate = millis();
  } else if (millis() - lastUpdate > 1500) {
      write_rssi(-999);
      lastUpdate = millis();
  }
}
