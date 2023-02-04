void pack_payload()
{

  payload[0] = device[0]; // id
  payload[1] = voltage_pzem >> 8;
  payload[2] = voltage_pzem & 0xFF;
  payload[3] = current_pzem >> 8;
  payload[4] = current_pzem & 0xFF;
  payload[5] = power_pzem >> 8;
  payload[6] = power_pzem & 0xFF;
  payload[7] = energy_pzem >> 24;
  payload[8] = energy_pzem >> 16;
  payload[9] = energy_pzem >> 8;
  payload[10] = energy_pzem & 0xFF;
  payload[11] = device[1];
  payload[12] = (uint8_t)(current_acs[0]);
  payload[13] = state[0];
  payload[14] = device[2];
  payload[15] = (uint8_t)(current_acs[1]);
  payload[16] = state[1];
  payload[17] = device[3];
  payload[18] = (uint8_t)(current_acs[2]);
  payload[19] = state[2];
  payload[20] = device[4];
  payload[21] = (uint8_t)(current_acs[3]);
  payload[22] = state[3];
  payload[23] = device[5];
  payload[24] = (uint8_t)(current_acs[4]);
  payload[25] = state[4];
  payload[26] = device[6];
  payload[27] = (uint8_t)(current_acs[5]);
  payload[28] = state[5];
}

void requestEvent()
{

  Wire.write(payload, 29); // respond with message of 29 bytes
                           // as expected by master
  for (int i = 0; i < sizeof(payload); i++)
  {
    Serial.print(payload[i], HEX);
  }
  Serial.println(" ");
}

void receiveEvent(int howMany)
{

  uint8_t buff[howMany];
  Serial.print("received ");
  Serial.print(howMany);
  Serial.print(" byte :  ");

  for (int i = 0; i < howMany; i++)
  {
    buff[i] = Wire.read();
    Serial.print(buff[i], HEX);
  }
  Serial.println();

  for (int i = 0; i < sizeof(buff); i++)
  {
    if (i < 6)
    {
      int chan = buff[i] >> 4;
      int state_buf = buff[i] & 0xF;
      state[i] = state_buf;
      relay(chan, state[i]);
      Serial.print(String("ch :") + chan);
      Serial.println(String(" state :") + state[i]);
    }
    if(i > 5 && buff[i] == 0xFF)
    {
      pzem.resetEnergy();
      Serial.print("reset energy success");
    }
  }
}
