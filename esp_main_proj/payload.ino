

void get_payload_uno() {
  uint8_t bytesReceived = Wire.requestFrom(I2C_UNO_ADDR, 29);
  Serial.printf("requestFrom: %u\n", bytesReceived);
  if ((bool)bytesReceived) {  //If received more than zero bytes
    //uint8_t payload[bytesReceived];
    Wire.readBytes(payload, bytesReceived);
    log_print_buf(payload, bytesReceived);
  }
  Serial.println();
}


void resive_payload() {
  if (LMIC.dataLen) {
    uint8_t data[LMIC.dataLen];
    for (int i = 0; i < LMIC.dataLen; i++) {
      data[i] = LMIC.frame[LMIC.dataBeg + i];
      Serial.print(data[i], HEX);
    }
    Serial.println();
    //Write message to the slave
    Wire.beginTransmission(I2C_UNO_ADDR);
    Wire.write(data, LMIC.dataLen);
    uint8_t error = Wire.endTransmission(false);
    Serial.printf("endTransmission: %u\n", error);
  }
  Serial.println();
}
