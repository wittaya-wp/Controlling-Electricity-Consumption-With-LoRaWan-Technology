
void mux(int index) {
  const int mux[16][4] = { { 0, 0, 0, 0 }, { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 1, 1, 0, 0 }, { 0, 0, 1, 0 }, { 1, 0, 1, 0 }, { 0, 1, 1, 0 }, { 1, 1, 1, 0 }, { 0, 0, 0, 1 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 1, 1, 0, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 0, 1, 1, 1 }, { 1, 1, 1, 1 } };
  const int device_ch[] = { 0, 2, 4, 6, 8, 10 };
  digitalWrite(s0_pin, mux[device_ch[index]][0]);
  digitalWrite(s1_pin, mux[device_ch[index]][1]);
  digitalWrite(s2_pin, mux[device_ch[index]][2]);
  digitalWrite(s3_pin, mux[device_ch[index]][3]);
  Serial.print("channel ");
  Serial.print(device_ch[index]);
  Serial.print("   ");
  delay(1);
}

void setMidPoint() {
  for (int i = 0; i < 6; i++) {
    float Analog = 0.0, samples = 0.0, avgAnalog = 0.0;
    mux(i);
    for (int x = 0; x < 100; x++) {  //This would take 500 Samples
      Analog = analogRead(analog_pin);
      samples = samples + Analog;
    }
    avgAnalog = samples / 100;
    BaseVolt[i] = avgAnalog * (5.0 / 1023.0);
    Serial.print(" voltage  :  ");
    Serial.println(BaseVolt[i]);
  }
}

void readSensorACS() {
  float volt_analog[6];
  float AvgACS = 0;
  for (int i = 0; i < 6; i++) {
    mux(i);
    AvgACS = analogRead(analog_pin);
    volt_analog[i] = AvgACS * (5.0 / 1023.0);
    current_acs[i] = (volt_analog[i] - BaseVolt[i]) / 0.066;  //0.066V = 66mvol.= 1 Amp This is sensitivity of your ACS module
    if (current_acs[i] < 0) {
      current_acs[i] = current_acs[i] * -1;
    } else if (current_acs[i] > 25) {
      current_acs[i] = 25.00;
    }
    Serial.print(" votage A0 :  ");
    Serial.print(volt_analog[i]);
    Serial.print("     ACS(A)   :  ");
    Serial.println(current_acs[i]);
  }
  Serial.println();
}


void pzemRead() {
  // Read the data from the sensor
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();

  voltage_pzem = (uint16_t)(voltage * 100);
  current_pzem = (uint16_t)(current * 100);
  power_pzem = (uint16_t)(power * 100);
  energy_pzem = (uint16_t)(energy * 100);

  // Check if the data is valid
  if (isnan(voltage)) {
    Serial.println("Error reading voltage");
  } else if (isnan(current)) {
    Serial.println("Error reading current");
  } else if (isnan(power)) {
    Serial.println("Error reading power");
  } else if (isnan(energy)) {
    Serial.println("Error reading energy");
  } else {
    //Print the values to the Serial console
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.println("V");
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println("A");
    Serial.print("Power: ");
    Serial.print(power);
    Serial.println("W");
    Serial.print("Energy: ");
    Serial.print(energy,2);
    Serial.println("kWh");

  }
  Serial.println();
}
