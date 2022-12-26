#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>
#include <Wire.h>

#define rxPin 10
#define txPin 11
#define s0_pin 8
#define s1_pin 9
#define s2_pin 12
#define s3_pin 13
#define analog_pin A0
#define I2C_UNO_ADDR 8
#define addr 0x55
#define interval 5000

byte payload[29];
const uint8_t device[7] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
int state[6];
unsigned long time;
float BaseVolt[6];
float current_acs[6];
uint16_t voltage_pzem;
uint16_t current_pzem;
uint16_t power_pzem;
uint32_t energy_pzem;


SoftwareSerial mySerial(rxPin, txPin);
PZEM004Tv30 pzem(mySerial);



void setup() {
  Serial.begin(9600);

  Wire.begin(8);                 // join I2C bus with address #8
  Wire.onReceive(receiveEvent);  // register event
  Wire.onRequest(requestEvent);  // register event
  pinMode(analog_pin, INPUT);
  pinMode(s0_pin, OUTPUT);
  pinMode(s1_pin, OUTPUT);
  pinMode(s2_pin, OUTPUT);
  pinMode(s3_pin, OUTPUT);
  for (int i = 2; i < 8; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, 1);
  }
  for (int i = 0; i < 6; i++) {
    state[i] = 1;
  }


  //set address for pzem
  if (!pzem.setAddress(addr)) {
    // Setting custom address failed. Probably no PZEM connected
    Serial.println("Error setting address.");
  } else {
    // Print out the new custom address
    Serial.print("Current address:    0x");
    Serial.println(pzem.readAddress(), HEX);
    Serial.println();
  }
  Serial.print("Pzem Custom Address:");
  Serial.println(pzem.readAddress(), HEX);

  setMidPoint();
}

void relay(int channel, int state) {
  const int pin_ch[] = { 2, 3, 4, 5, 6, 7 };
  int index = channel - 1;
  //--------- logic LOW mean relay pin com->on = circuit cut off----------//
  digitalWrite(pin_ch[index], state);
  Serial.print(String("pin :") + pin_ch[index]);
  Serial.println(String("  state : ") + state);
}

void loop() {
  if (millis() - time >= interval) {
    time = millis();
    pzemRead();
    readSensorACS();
    pack_payload();
  }
}
