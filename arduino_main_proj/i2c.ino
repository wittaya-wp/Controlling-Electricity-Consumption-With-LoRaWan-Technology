void pack_payload() {


  uint8_t current[6];
  for (int i = 0; i < 6; i++) {
    current[i] = (uint8_t)(current_acs[i]);
  }
  byte energy_high, energy_low;
  energy_high = energy_pzem >> 8 >> 8;
  energy_low = energy_pzem & 0xFFFF;

  payload[0] = device[0];  //id
  payload[1] = voltage_pzem >> 8;
  payload[2] = voltage_pzem & 0xFF;
  payload[3] = current_pzem >> 8;
  payload[4] = current_pzem & 0xFF;
  payload[5] = power_pzem >> 8;
  payload[6] = power_pzem & 0xFF;
  payload[7] = energy_high >> 8;
  payload[8] = energy_high & 0xFF;
  payload[9] = energy_low >> 8;
  payload[10] = energy_low & 0xFF;
  payload[11] = device[1];
  payload[12] = current[0];
  payload[13] = 0x01;
  payload[14] = device[2];
  payload[15] = current[1];
  payload[16] = 0x01;
  payload[17] = device[3];
  payload[18] = current[2];
  payload[19] = 0x01;
  payload[20] = device[4];
  payload[21] = current[3];
  payload[22] = 0x01;
  payload[23] = device[5];
  payload[24] = current[4];
  payload[25] = 0x01;
  payload[26] = device[6];
  payload[27] = current[5];
  payload[28] = 0x01;




  for (int i = 0; i < sizeof(payload); i++) {
    //     if (payload[i] == 0) {
    //   Serial.print("00");
    // } else {
    Serial.print(payload[i], HEX);
  }
  // }
  Serial.println(" ");
}

void requestEvent() {
  pack_payload();
  Wire.write(payload, 29);  // respond with message of 29 bytes
  // as expected by master
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {

  uint8_t buff[howMany];
  Serial.print("received ");
  Serial.print(howMany);
  Serial.print(" byte :  ");

  for (int i = 0; i < howMany; i++) {
    buff[i] = Wire.read();
    Serial.print(buff[i], HEX);
  }
  Serial.println();
  for (int i = 0; i < sizeof(buff); i++) {
    int ch  = (int)buff[i] >> 4;
    state[i] = (int)buff[i] & 0xF ;
    relay(ch, state[i]);
    Serial.print("state : ");
    Serial.println(state[i]);
  }
}
