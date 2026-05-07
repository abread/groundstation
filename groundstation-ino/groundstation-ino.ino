// !!!CORRIGIR FREQUÊNCIA!!!
#define RADIO_FREQUENCY 433000000 /* Hz */
#define CJKIT_VERSION 2

#define RADIO_SS_PIN 10
#define RADIO_IRQ_PIN 3
#define RADIO_NET_ID 100 // 0-255, must be the same on all nodes
#define RADIO_NODE_ID 1 // 0-254, must be unique in network, 255=broadcast
#define RADIO_ATC_RSSI -80

#include <RFM69.h>
#include <RFM69_ATC.h>

#if CJKIT_VERSION <= 1
#define RADIO_LIB RFM69_ATC
#elif CJKIT_VERSION == 2

#define RADIO_LIB RFM69_ATC_SlowSpi
class RFM69_ATC_SlowSpi : public RFM69_ATC {
  public:

    RFM69_ATC_SlowSpi(uint8_t slaveSelectPin, uint8_t interruptPin, bool isRFM69HW, uint8_t interruptNum __attribute__((unused))) //interruptNum is now deprecated
                : RFM69_ATC(slaveSelectPin, interruptPin, isRFM69HW) {}

    RFM69_ATC_SlowSpi(uint8_t slaveSelectPin=RF69_SPI_CS, uint8_t interruptPin=RF69_IRQ_PIN, bool isRFM69HW_HCW=false, SPIClass *spi=nullptr) : RFM69_ATC(slaveSelectPin, interruptPin, isRFM69HW_HCW, spi) {}

    bool initialize(uint8_t freqBand, uint16_t nodeID, uint8_t networkID=1) {
      bool result = RFM69_ATC::initialize(freqBand, nodeID, networkID);

      // initialization goes okay with the 8MHz default, but sending data does not
      // drop it down to 1MHz
      _settings = SPISettings(1000000, MSBFIRST, SPI_MODE0);

      return result;
    }
};
#else
#error "Unknown kit version"
#endif

RADIO_LIB radio = RADIO_LIB(RADIO_SS_PIN, RADIO_IRQ_PIN);

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
