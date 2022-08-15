// made by BinaryWorlds
// Not for commercial use, in other case by free to use it.
// Just copy this text and link to oryginal repository:
// https://github.com/BinaryWorlds/ThermalPrinter

// I am not responsible for errors in the library. I deliver it "as it is".
// I will be grateful for all suggestions.

// Tested on firmware 2.69 and JP-QR701
// Some features may not work on the older firmware.


// Libraries for SD card
#include "SD_MMC.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Define CS pin for the SD card module
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
SPIClass sdSPI(VSPI);

#include <Arduino.h>
#include <HardwareSerial.h>

#include "TPrinter.h"

#include "correspondance.h"
#include "formes.h"

const uint8_t bitmapWidth = 40;
const uint8_t bitmapHeight = 37;

const byte rxPin = 16;
const byte txPin = 17;
const byte dtrPin = 27;  // optional
const byte rsePin = 4;   // direction of transmission, max3485

HardwareSerial mySerial(1);
Tprinter myPrinter(&mySerial, 19200);


boolean newData = false;

int dataNumber = 0;             // new for this version

const byte numBytes = 128;
byte receivedBytes[numBytes];

byte numReceived = 0;

String dataMessage;

void setup() {
  micros();
  mySerial.begin(19200, SERIAL_8N1, rxPin, txPin);

  pinMode(rsePin, OUTPUT);     // optional
  digitalWrite(rsePin, HIGH);  // optional
  // myPrinter.enableDtr(dtrPin, LOW);

  myPrinter.begin();
  myPrinter.setHeat(1, 224, 40);  // in begin setHeat was called with val: 0,255,0

  myPrinter.printBitmap(qrcode, bitmapWidth, bitmapHeight);
  myPrinter.feed(1);

  myPrinter.setMode(FONT_B, DOUBLE_WIDTH, DOUBLE_HEIGHT);
  myPrinter.println("FONT_B, bigger");
  myPrinter.unsetMode(FONT_B);
  myPrinter.println("FONT_A, bigger");
  myPrinter.feed(2);

  // Initialize SD card
  //SD.begin(SD_CS);  
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS, sdSPI)) {
    Serial.println("Card Mount Failed");
    myPrinter.println(F("Card Mount Failed"));
    myPrinter.feed(2);
    return;
  }
  Serial.println("1");
  myPrinter.println(F("1"));
  myPrinter.feed(2);
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    myPrinter.println(F("No SD card attached"));
    myPrinter.feed(2);
    return;
  }
  Serial.println("Initializing SD card...");
  myPrinter.println(F("Initializing SD card..."));
  myPrinter.feed(2);
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    myPrinter.println(F("ERROR - SD card initialization failed!"));
    myPrinter.feed(2);
    return;    // init failed
  }
  Serial.println("2");
  myPrinter.println(F("2"));
  myPrinter.feed(2);

  File file = SD.open("/data1.txt");
  if(!file) {
    Serial.println("File doens't exist");
    myPrinter.println(F("File doens't exist"));
    myPrinter.feed(2);
    Serial.println("Creating file...");
    myPrinter.println(F("Creating file..."));
    myPrinter.feed(2);
    writeFile(SD, "/data1.txt", "Reading ID, Date, Hour, Temperature \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();

  
//  //SD_MMC.begin();
//  if(!SD.begin(5)){
//    Serial.println("Card Mount Failed");
//    myPrinter.println(F("Card Mount Failed"));
//    myPrinter.feed(2);
//    return;
//  }
//  uint8_t cardType = SD.cardType();
//
//  if(cardType == CARD_NONE){
//    Serial.println("No SD card attached");
//    myPrinter.println(F("No SD card attached"));
//    myPrinter.feed(2);
//    return;
//  }
//
//  Serial.print("SD Card Type: ");
//  myPrinter.println(F("SD Card Type: "));
//  myPrinter.feed(2);
//  if(cardType == CARD_MMC){
//    Serial.println("MMC");
//    myPrinter.println(F("MMC"));
//    myPrinter.feed(2);
//  } else if(cardType == CARD_SD){
//    Serial.println("SDSC");
//    myPrinter.println(F("SDSC"));
//    myPrinter.feed(2);
//  } else if(cardType == CARD_SDHC){
//    Serial.println("SDHC");
//    myPrinter.println(F("SDHC"));
//    myPrinter.feed(2);
//  } else {
//    Serial.println("UNKNOWN");
//    myPrinter.println(F("UNKNOWN"));
//    myPrinter.feed(2);
//  }
//
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  myPrinter.println(F("Card done"));
  myPrinter.feed(2);
}


void loop() {
    myPrinter.printFromSerial();  // open monitor and print something
  
}


// Write the sensor readings on the SD card
void logSDCard() {
  //dataMessage = String(readingID) + "," + String(dayStamp) + "," + String(timeStamp) + "," + 
  //              String(temperature) + "\r\n";
  dataMessage = "Hello World \n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data1.txt", dataMessage.c_str());
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
