#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 5
#define RST_PIN 4
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials

// #define WIFI_SSID "Galaxy A21s52EC"
// #define WIFI_PASSWORD "enyh9882"
// #define WIFI_SSID "realme narzo 20 Pro"
// #define WIFI_PASSWORD "akshaisunil"
#define WIFI_SSID "Galaxy A14 0C7B"
#define WIFI_PASSWORD "cnt4zfz2j3rnndn"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDWa7rC1WO4MHopdiqy9xn4lp1Jj7mDpqY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://smartcartstthomas-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Define prices for items
#define LAYS_PRICE 10
#define BISCUITS_PRICE 10

// Variable to keep track of total amount
int totalAmount = 0;

// Define LCD connection details
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address might be different (check LCD datasheet)


// Variable to store the last scanned item
String lastItem = "";
bool signupOK = false;
String sValue, sValue2;

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  
   // Initialize LCD (number of columns, rows)
  lcd.init();
  // Turn on backlight and clear display
  lcd.backlight();
  lcd.clear();
  lcd.print("Cart is Ready");
  lcd.setCursor(0, 1);
  lcd.print("Place tag near");

  Serial.println("RFID Ready");
  Serial.println("Place your tag near the reader...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
Firebase.RTDB.setString(&fbdo, "/biscut","0");
Firebase.RTDB.setString(&fbdo, "/lays","0");
Firebase.RTDB.setString(&fbdo, "/total","0");

}

void loop() {
   if (Firebase.ready() && signupOK ) {
     
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    
    // Check if scanned tag corresponds to Lays
    if (content.substring(1) == "43 A8 2A C5") {
      if (Firebase.RTDB.setString(&fbdo, "/lays","1")){
          Serial.println("lays PASSED"); 
               
          }

      Serial.println("Lays");
      totalAmount += LAYS_PRICE;
      lastItem = "Lays:10rps";
    }
    // Check if scanned tag corresponds to Biscuits
    else if (content.substring(1) == "03 EF 43 F6") {
      if (Firebase.RTDB.setString(&fbdo, "/biscut","1")){
          Serial.println("water level PASSED"); 
               
          }

      Serial.println("Biscuits:10rps");
      totalAmount += BISCUITS_PRICE;
      lastItem = "Biscuit";
    }
    else {
      Serial.println("Unknown tag scanned");
      lastItem = "Unknown";
    }




    Serial.print("Total Amount: ");
    Serial.println(totalAmount);

    // Update LCD display
    lcd.clear();
    lcd.print("LastItem:");
    //lcd.setCursor(0, 1);
    lcd.print(lastItem);
    lcd.setCursor(0, 1);
    if (Firebase.RTDB.getString(&fbdo, "/total")) {
      if (fbdo.dataType() == "string") {
        sValue = fbdo.stringData();
        int a = sValue.toInt();
        lcd.print("Total Amount: ");
    lcd.print(a);
      }
      }
    
    delay(1000);
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
   }
}
