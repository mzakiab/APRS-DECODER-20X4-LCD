/*
Project:            Monitor APRS Signal
By:                 9W2KEY
Hardware:           I2C 20 x 4 LCD display
Koding asal:        https://github.com/chokelive/aprs_tnc 
31 Jan, 2023 hingga 3hb Feb,2023 (2am) mintak bantuan 9M2CIO

Terima Kasih abe Din atas bantuan ini.
*/

#include <LibAPRS.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
int LED_RX = 8;             // rx dan proses LED indicator 

int infoLen; 
int i;
int maxLen;

int columnLeft; 
int columnWidth; 
int columnNow;

int rowTop; 
int rowHeight; 
int rowNow;


LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

/*  I2C LCD     ARDUINO (NANO/UNO)
    VCC         5V
    GND         GND
    SDA         A4
    SCL         A5
*/

// APRS Configulation
#define CALL_SIGN "9W2KEY"           // sila bubuh callsign sendiri
#define CALL_SIGN_SSID 9

// #define ADC_REFERENCE REF_3V3
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
  char sentence; // 

    int rowTop = 1;                     // 2nd line
    int rowHeight = 4;                  // display height
    int rowNow;

    int columnLeft = 0;                 // 1st column
    int columnWidth = 20;               // display width
    int columnNow;

    int infoLen;
    int i;
    boolean done;
 
  if (gotPacket) {
    gotPacket = false;

    boolean done = false;
    infoLen = incomingPacket.len;
    columnLeft = 0;
    columnWidth = 20;
    rowTop = 1;
    rowHeight = 4;

    
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

    digitalWrite(LED_RX, HIGH);         // ON LED 
    delay(500);                         // tunggu kejap nak proses data yg masuk
    lcd.clear();                        // clear display dulu

lcd.setCursor(0, 0);
lcd.print(incomingPacket.src.call);
lcd.print(F("-"));
lcd.print(incomingPacket.src.ssid);
lcd.print(F(" "));
lcd.print(incomingPacket.dst.call);
lcd.print(F("-"));
lcd.print(incomingPacket.dst.ssid);

/*********************** SETUP UNTUK LCD 20 x 4 *********************/
     rowNow = rowTop;                    // start at top row
    columnNow = columnLeft;             // and left column

    infoLen = incomingPacket.len;       // max info length
    i = 0;                              // info string index
    done = false;                       // display full flag

    lcd.setCursor(columnNow,rowNow);    // start here

    while ( ! done && i < infoLen )     // while not done and still have info
    {
//     lcd.setCursor(columnNow,rowNow); // line utk bui jelas, keep comment
       lcd.write(incomingPacket.info[i]);       // put 1 char
       i++;                                     // next char

       columnNow++;                             // next column
       if ( columnNow >= columnWidth )          // if too wide
       {
          columnNow = columnLeft;               // to the left, <cr>
          rowNow++;                             // to the next line, <lf>
          done = ( rowNow >= rowHeight );       // if max height, done
          if ( ! done )                         // otherwise
          {  lcd.setCursor(columnNow,rowNow); } // move the cursor to new line
       }
   }
   
   while ( ( ! done ) && ( i < infoLen ) )
   {     // belum habis/done dan ada info belum print
       if ( columnNow >= columnWidth )  // if columNow dah terkanan
       {
          columnNow = columnLeft;       // gi ke kiri, <cr>
          if ( rowNow >= rowHeight )    // if rowNow dah di bawah
          { done = true; }              // dah habis dah
          else { rowNow++; }            // belum habis, gi next line, <lf>
       }
       else { columnNow++; }            // move a column to right

       if ( ! done )                    // if belum habis/done
       {
          lcd.setCursor(columnNow,rowNow);      // set cursor position
         lcd.write(incomingPacket.info[i]);     // muntahkannya
          i++;                                  // next char dlm info
       }
   }
    
    lcd.blink();                                // cursor kelip kelip
    digitalWrite(LED_RX, LOW);                  // OFF LED
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