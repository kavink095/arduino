#include <Arduino_JSON.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include <SPI.h>
#include <MFRC522.h>
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4 ; //--> SDA / SS is connected to pinout D2
//--> RST is connected to pinout D1
MFRC522 rfid(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.
MFRC522::MIFARE_Key key;

const char* ssid = "UKT";
const char* password = "uMobi123";

String tag;
char jsonOutPut[128];
//-----------------------------------------------------------------------------------------------SETUP--------------------------------------------------------------------------------------//
void setup() {
  Serial.begin(115200); //--> Initialize serial communications with the PC
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  SPI.begin();      //--> Init SPI bus
  rfid.PCD_Init(); //--> Init MFRC522 card

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    //      Serial.println("....");
    digitalWrite(LED_BUILTIN, LOW);

    long rnd = random(1, 10);

    if (! rfid.PICC_IsNewCardPresent())
      return ;
    if (rfid.PICC_ReadCardSerial()) {
      for (byte i = 0; i < 4; i++) {
        tag += rfid.uid.uidByte[i];
        digitalWrite(LED_BUILTIN, HIGH);
      }
      //      Serial.println("read card :- " + tag);
      delay(500);
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      digitalWrite(LED_BUILTIN, LOW);
    }
    HTTPClient http;    //Declare object of class HTTPClient
    http.begin("http://192.168.8.100:8084/api/v1/enter/open");
    http.addHeader("Content-Type", "application/json");

//    const size_t CAPACITY = JSON_OBJECT_SIZE(1);
//    StaticJsonDocument<CAPACITY> doc;

//    JsonObject obj = doc.to<JsonObject>();
    Serial.println("read card :- " + tag);
//    serializeJson(doc, jsonOutPut);

    int httpCode = http.PUT(String(tag));
    Serial.println("Json :- " + String(jsonOutPut));

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("payload :- " + payload);
      if (payload == String(1)) {
        Serial.print("Access !");
      } else {
        Serial.print("Access Denied !");
      }
      http.end();
    }else{
      Serial.println("cannot connect with http !");
    }
     tag = "";
  }
}
