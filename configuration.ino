// address map is having a 16 step
// positions: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240,256,272,288
//304,320,336,352,368,384,400,416,432,448,464,480,496

const int configFlag = 0;
//const int passwordFlag = 16;
//const int wifiFlag = 32;
//const int wifiPassFlag = 80;

const int passwordFlag = 32;
const int wifiFlag = 64;
const int wifiPassFlag = 96;
// using address 80 since my board has problem with 48 to 80...
// ----------------------------------------------------------------------  EEPROM
void EEPROM_write_to_address(int address, String stringValue) {

  char value[32];
  stringValue.toCharArray(value, 32);

  for (int i = 0; i < 32; i++) {
    if (i < strlen(value)) {
      // write to memory
      EEPROM.write(address + i, (int)value[i]);
    } else {
      // clear rest of the memory
      EEPROM.write(address + i, 0);
    }
    delay(20);
  }
  EEPROM.commit();
}

void EEPROM_clear() {
  EEPROM.begin(512);
  for (int i = 0 ; i < 512 ; i++) {
    EEPROM.write(i, 0);
    delay(10);
  }
  EEPROM.commit();
}

String EEPROM_read_from_address(int address) {
  char temp;
  int counter = 0;
  String result = "";
  EEPROM.begin(512);
  for (int i = address; i < address + 32; i++) {
    temp = EEPROM.read(i);
    delay(50);
    if (temp != NULL && int(temp) != 0) {
      counter++;
      result.concat(temp);
    }
  }
  Serial.print("Data read: ");
  Serial.println(result);
  return result;
}
// ----------------------------------------------------------------------  configuration flag
bool isConfigured() {
  String isConfigured = EEPROM_read_from_address(configFlag);
  isConfigured.trim();
  Serial.println("Checking if device is configured...");
  delay(10);
  return isConfigured.equals("true");
}

void setToConfigured() {
  EEPROM_write_to_address(configFlag, "true");
  Serial.println("Configuration saved");
  Serial.println("Restart..");
  delay(1500);
  ESP.restart();
}

void destroyConfiguration() {
  EEPROM_clear();
  EEPROM_write_to_address(configFlag, "false");
  Serial.println("Configuration destroyed!!");
  Serial.println("Restart..");
  delay(1500);
  ESP.restart();
}
// ----------------------------------------------------------------------  pass
void set_password(String passowrd) {
  Serial.println("Writing password");
  EEPROM_write_to_address(passwordFlag, password);
}
String get_password() {
  Serial.println("Reading password");
  return EEPROM_read_from_address(passwordFlag);
}
// ----------------------------------------------------------------------  wifi
void set_wifi(String wifi) {
  Serial.println("Writing wifi");
  EEPROM_write_to_address(wifiFlag, wifi);
}
String get_wifi() {
  Serial.println("Reading wifi");
  return EEPROM_read_from_address(wifiFlag);
}

// ----------------------------------------------------------------------  wifi pass
void set_wifi_pass(String wifi_pass) {
  Serial.println("Writing wifi pass");
  EEPROM_write_to_address(wifiPassFlag, wifi_pass);
}
String get_wifi_pass() {
  Serial.println("Reading wifi password");
  return EEPROM_read_from_address(wifiPassFlag);
}
