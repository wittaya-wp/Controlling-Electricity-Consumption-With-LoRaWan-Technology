//MISO ---> GPIO19
//MOSI ---> GPIO23
//SCK   ---> GPIO18
//NSS   ---> GPIO5
//DIO0 ---> GPIO27
//DIO1 ---> GPIO26
//DIO2 ---> GPIO25
//RST   ---> GPIO14
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "Wire.h"

#define WATCHDOG_TIMEOUT_S 30
#define I2C_UNO_ADDR 8
#define I2C_SDA 33
#define I2C_SCL 32
#define led 2

const unsigned TX_INTERVAL = 60;
hw_timer_t* watchDogTimer = NULL;
static osjob_t sendjob;
long unsigned last_time;

uint8_t payload[29];
static const PROGMEM u1_t NWKSKEY[16] = { 0x96, 0x69, 0x80, 0x29, 0x57, 0xE7, 0xA8, 0x85, 0x29, 0x1C, 0x0F, 0xE0, 0xDD, 0x37, 0x8E, 0x8A };
static const u1_t PROGMEM APPSKEY[16] = { 0xAF, 0xCD, 0x5E, 0x88, 0x9A, 0x5E, 0xFE, 0x0F, 0xDD, 0xF7, 0xF6, 0x9D, 0x22, 0x62, 0xEB, 0x0D };
static const u4_t DEVADDR = 0x260D8525;
const lmic_pinmap lmic_pins = {
  .nss = 5,  // chip select on feather (rf95module) CS
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = { 27, 26, 25 },
};

void os_getArtEui(u1_t* buf) {}
void os_getDevEui(u1_t* buf) {}
void os_getDevKey(u1_t* buf) {}

void IRAM_ATTR watchDogInterrupt() {
  Serial.println("reboot");
  ESP.restart();
}
void watchDogRefresh() {
  timerWrite(watchDogTimer, 0);  //reset timer (feed watchdog)
}

void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;

    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
        //------ Added ----------------
        resive_payload();
        //-----------------------------
      }
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;

    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      /* do not print anything -- it wrecks timing */
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      break;
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned)ev);
      break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, payload, sizeof(payload), 0);
    Serial.println(F("Packet queued"));
    Serial.println("HEX PAYLOAD ");
    for (int i = 0; i < sizeof(payload); i++) {
      // if (payload[i] == 0) {
      //   Serial.print("00");
      // } else {
      Serial.print(payload[i], HEX);
      // }
    }
    // Next TX is scheduled after TX_COMPLETE event.
  }
}


void setup() {

  Serial.begin(115200);



  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(led, OUTPUT);
  delay(100);  // per sample code on RF_95 test
  Serial.println(F("Starting"));
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  watchDogTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  timerAlarmEnable(watchDogTimer);
  delay(10000);
  get_payload_uno();

#ifdef PROGMEM
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession(0x13, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession(0x13, DEVADDR, NWKSKEY, APPSKEY);
#endif


  LMIC_setupChannel(0, 923200000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(1, 923400000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);

  //LMIC_setAdrMode(0);
  // Disable link check validation
  LMIC_setLinkCheckMode(1);


  LMIC.dn2Dr = DR_SF9;
  LMIC_setDrTxpow(DR_SF7, 14);
  // Start job
  do_send(&sendjob);
}


void loop() {
  if (millis() - last_time >= 30000) {
    last_time = millis();
    get_payload_uno();
  }
  watchDogRefresh();
  os_runloop_once();
}