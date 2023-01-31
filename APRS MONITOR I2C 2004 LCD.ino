/*
Project: Monitor APRS Signal
By: 9W2KEY
Hardware: I2C 20 x 4 LCD display
Koding asal https://github.com/chokelive/aprs_tnc 
*/
#include <LibAPRS.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
int LED_RX = 8; // rx dan proses LED indicator 

LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

/*  I2C LCD     ARDUINO (NANO/UNO)
    VCC         5V
    GND         GND
    SDA         A4
    SCL         A5
*/

// APRS Configulation
#define CALL_SIGN "9W2KEY" // sila bubuh callsign sendiri
#define CALL_SIGN_SSID 9

#define ADC_REFERENCE REF_5V
#define OPEN_SQUELCH false

// APRS Global Variable
boolean gotPacket = false;
AX25Msg incomingPacket;
uint8_t *packetData;


void setup() {
  // Set up serial port
  pinMode(LED_RX, OUTPUT);
  Serial.begin(115200);

  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Initialise APRS library - This starts the modem
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
  APRS_setCallsign(CALL_SIGN, CALL_SIGN_SSID);
  APRS_printSettings();
  Serial.print(F("Free RAM:     ")); Serial.println(freeMemory());

  // start print di LCD
  lcd.setCursor(2, 0); 
  lcd.print("APRS Decoder LCD");
  lcd.setCursor(5, 1);
  lcd.print("by 9W2KEY");
  lcd.setCursor(1, 2);
  lcd.print("mzakiab@gmail.com");
  lcd.setCursor(0, 3);
  lcd.print("Waiting APRS data...");
} 
void loop() {
  
  delay(500);
  processPacket();
}


void processPacket() {
  char sentence[150]; 
  if (gotPacket) {
    gotPacket = false;
    
    Serial.print(F("Received APRS packet. SRC: "));
    Serial.print(incomingPacket.src.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.src.ssid);
    Serial.print(F(". DST: "));
    Serial.print(incomingPacket.dst.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.dst.ssid);
    Serial.print(F(". Data: "));

    for (int i = 0; i < incomingPacket.len; i++) {
      Serial.write(incomingPacket.info[i]);
    }
    Serial.println("");

    digitalWrite(LED_RX, HIGH); // ON LED 
    delay(500); // tunggu kejap nak proses data yg masuk
    lcd.clear(); // clear display dulu
    //lcd.setCursor(0, 0); 
    lcd.print(incomingPacket.src.call);
    lcd.print(F("-"));
    lcd.print(incomingPacket.src.ssid);
    lcd.print(F(" "));
    lcd.print(incomingPacket.dst.call);
    lcd.print(F("-"));
    lcd.print(incomingPacket.dst.ssid);
    lcd.print(F("   "));
    //lcd.println();
    // lcd.set1X();
    //lcd.println();
    //lcd.setCursor(0, 1);
    for (int i = 0; i < incomingPacket.len; i++) {
    //if(i%20==0) lcd.println(); // tak tahu apa fungsi yg ini, try buang pun tak ada apa2 perubahan
    
    lcd.write(incomingPacket.info[i]); // asal gini 
    lcd.blink(); // cursor kelip kelip
    }
    //lcd.println("");
    digitalWrite(LED_RX, LOW); // OFF LED
    free(packetData);

    // Serial.print(F("Free RAM: ")); Serial.println(freeMemory());
  }
}

void aprs_msg_callback(struct AX25Msg *msg) {
  if (!gotPacket) {
    gotPacket = true;

    memcpy(&incomingPacket, msg, sizeof(AX25Msg));

    if (freeMemory() > msg->len) {
      packetData = (uint8_t*)malloc(msg->len);
      memcpy(packetData, msg->info, msg->len);
      incomingPacket.info = packetData;
    } else {
      // We did not have enough free RAM to receive
      // this packet, so we drop it.
      gotPacket = false;
    }
  }
}