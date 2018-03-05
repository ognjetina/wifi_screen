#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

StaticJsonBuffer<200> jsonBuffer;

const char* username = "admin";
char password[32] = "";
char wifi[32];
char wifi_pass[32];

bool shouldPing = false;
int refreshRate = 3000;
String pingAddress = "";

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

void writeToDisplay(String line1, String line2) {
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void handleDisplay() {
  if (!server.authenticate(username, password))
    return server.requestAuthentication();

  if (server.arg("line1") == "" && server.arg("line2") == "") {
    server.send(400, "text/plain", "Error");
  } else {
    server.send(200, "text/plain", "Lines recived");
    writeToDisplay(server.arg("line1"), server.arg("line2"));
  }
}

void handlePingStop() {
  if (!server.authenticate(username, password))
    return server.requestAuthentication();

  lcd.clear();
  shouldPing = false;
  server.send(200, "text/plain", "Ping stop.");
}

void handlePing() {
  if (!server.authenticate(username, password))
    return server.requestAuthentication();

  if (server.arg("pingAddress") == "" && server.arg("refreshRate") == "") {
    server.send(400, "text/plain", "Error");
  } else {
    pingAddress = server.arg("pingAddress");
    refreshRate = server.arg("refreshRate").toInt();
    shouldPing = true;
    server.send(200, "text/plain", "Ping start.");
  }
}

void handleDestroy() {
  if (!server.authenticate(username, password))
    return server.requestAuthentication();

  destroyConfiguration();
  server.send(200, "text/plain", "Configuration destroyed");
}
void handleNotFound() {
  server.send(404, "text/plain", "nothing to do here...");
}

void setup(void) {
  Serial.begin(9600);

  //destroyConfiguration();

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  if (isConfigured()) {
    Serial.println("Reading config");
    get_wifi().toCharArray(wifi, 32);
    get_wifi_pass().toCharArray(wifi_pass, 32);
    get_password().toCharArray(password, 32);

    lcd.clear();
    lcd.print("Connecting to:");
    lcd.setCursor(0, 1);
    lcd.print(wifi);
  } else {
    writeToDisplay("Device is not", "configured!");
    Serial.println("Waiting for configuration");

    bool configured = false;
    String configurationTemp = "";
    String questions[4] = {"Input device password:", "Input wifi:", "Input wifi password:"};

    while (!configured) {
      Serial.println("Expecting configuration: device password,wifi,wifi_pass");
      for (int i = 0; i < 3; i++) {
        Serial.println(questions[i]);
        while (Serial.available() == 0) {
        }
        configurationTemp = Serial.readString();
        configurationTemp.trim();
        if (configurationTemp.length() < 1) {
          Serial.println("Invalid input");
          i--;
        } else {
          Serial.print("You have inputed: ");
          Serial.println(configurationTemp);
          switch (i) {
            case 0:
              configurationTemp.toCharArray(password, 32);
              break;
            case 1:
              configurationTemp.toCharArray(wifi, 32);
              break;
            case 2:
              configurationTemp.toCharArray(wifi_pass, 32);
              configured = true;
              break;
            default:
              Serial.println("This is imposiburu!");
          }

        }
      }
      Serial.println("------------------------");
      Serial.println("Configuration done.");
      Serial.print("Device Password: ");
      Serial.println(password);
      Serial.print("Wifi: ");
      Serial.println(wifi);
      Serial.print("Wifi password: ");
      Serial.println(wifi_pass);
      Serial.println("------------------------");

    }
    Serial.println("Saving data to eeprom");
    set_password(password);
    set_wifi(String(wifi));
    set_wifi_pass(String(wifi_pass));
    setToConfigured();
    Serial.println("------------------------");
  }

  WiFi.begin(wifi, wifi_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected");

  lcd.clear();
  lcd.print(wifi);
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());


  server.on("/display", handleDisplay);
  server.on("/destroy", handleDestroy);
  server.on("/ping", handlePing);
  server.on("/pingStop", handlePingStop);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop(void) {
  ArduinoOTA.handle();
  server.handleClient();

  while (shouldPing) {
    Serial.print("Pinging: ");
    Serial.println(pingAddress);
    Serial.print("At rate: ");
    Serial.println(refreshRate);
    HTTPClient http;
    http.begin(pingAddress);
    int httpCode = http.GET();                                                                  //Send the request

    if (httpCode = 200) {
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      if (!root.success()) {
        Serial.println("Serial parse failed");
        return;
      } else {
        writeToDisplay(root["line1"], root["line2"]);
        jsonBuffer.clear();
      }
    } else {
      writeToDisplay("Something went", "wrong!!!");
    }
    http.end();
    delay(refreshRate);
    ArduinoOTA.handle();
    server.handleClient();
  }

}
