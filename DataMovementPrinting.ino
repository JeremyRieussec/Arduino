// made by BinaryWorlds
// Not for commercial use, in other case by free to use it.
// Just copy this text and link to oryginal repository:
// https://github.com/BinaryWorlds/ThermalPrinter

// I am not responsible for errors in the library. I deliver it "as it is".
// I will be grateful for all suggestions.

// Tested on firmware 2.69 and JP-QR701
// Some features may not work on the older firmware.
#include <iostream>
#include <fstream>
#include <string>

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

#include "all_headers.h"


const byte rxPin = 16;
const byte txPin = 17;
const byte dtrPin = 27;  // optional
const byte rsePin = 4;   // direction of transmission, max3485

HardwareSerial mySerial(1);
Tprinter myPrinter(&mySerial, 19200);


boolean newData = false;
int dataNumber = 0;             // new for this version

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
int end_sequence = 0;

const int numBytes = 2066;
byte receivedBytes[numBytes];
byte numReceived = 0;

String dataMessage;

// dimensions images 
int width_pixel_form = 384;
int height_pixel_form = 100;

int start_line_height = 110;

int width_line_8_bit = 48;
int width_form_8_bit = 3 ;

// for line printing
int add_offset = 4;
int numberFormsLine = 11;

const int numOctets = 4800;
uint8_t line_to_modify[numOctets];

// correspondance 
uint8_t* au = gauche_haut_data;
uint8_t* am = gauche_milieu_data;
uint8_t* al = gauche_bas_data;
uint8_t* bl = diagonale_arriere_bas_gauche_data;
uint8_t* bm = diagonale_arriere_milieu_gauche_data;
uint8_t* bu = diagonale_arriere_haut_gauche_data;
uint8_t* cxl = arriere_bas_droite_data;
uint8_t* cxm = arriere_milieu_droit_data;
uint8_t* cxu = arriere_haut_droit_data;
uint8_t* cyl = arriere_bas_gauche_data;
uint8_t* cym = arriere_milieu_gauche_data;
uint8_t* cyu = arriere_haut_gauche_data;
uint8_t* dl = diagonale_arriere_bas_droit_data;
uint8_t* dm = diagonale_arriere_milieu_droit_data;
uint8_t* du = diagonale_arriere_haut_droit_data;
uint8_t* el = droit_bas_data;
uint8_t* em = droit_milieu_data;
uint8_t* eu = droit_haut_data;
uint8_t* fl = diagonale_avant_bas_droit_data;
uint8_t* fm = diagonale_avant_milieu_droit_data;
uint8_t* fu = diagonale_avant_haut_droit_data;
uint8_t* gxl = devant_bas_droit_data;
uint8_t* gxm = devant_milieu_droit_data;
uint8_t* gxu = devant_haut_droit_data;
uint8_t* gyl = devant_bas_gauche_data;
uint8_t* gym = devant_milieu_gauche_data;
uint8_t* gyu = devant_haut_gauche_data;
uint8_t* hl = diagonale_avant_bas_gauche_data;
uint8_t* hm = diagonale_avant_milieu_gauche_data;
uint8_t* hu = diagonale_avant_haut_gauche_data;
uint8_t* il = centre_bas_data;
uint8_t* im = centre_milieu_data;
uint8_t* iu = centre_haut_data;


void setup() {
  micros();
  mySerial.begin(19200, SERIAL_8N1, rxPin, txPin);

  Serial.begin(19200);
  while (!Serial) {
         ; // wait for serial port to connect. Needed for native USB port only
    }
  Serial.println("<Arduino is ready>");
  
  pinMode(rsePin, OUTPUT);     // optional
  digitalWrite(rsePin, HIGH);  // optional
  digitalWrite(0, LOW);  // optional

  // myPrinter.enableDtr(dtrPin, LOW);

  myPrinter.begin();
  myPrinter.setHeat(1, 224, 40);  // in begin setHeat was called with val: 0,255,0
  
  // Initialize SD card
  //SD.begin(SD_CS);  
//  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
//  if(!SD.begin(SD_CS, sdSPI)) {
//    Serial.println("Card Mount Failed");
//    return;
//  }
//  Serial.println("1");
  
//  uint8_t cardType = SD.cardType();
//  
//  if(cardType == CARD_NONE) {
//    Serial.println("No SD card attached");
//    myPrinter.println(F("No SD card attached"));
//    myPrinter.feed(2);
//    return;
//  }
//  Serial.println("Initializing SD card...");
//  if (!SD.begin(SD_CS)) {
//    Serial.println("ERROR - SD card initialization failed!");
//    return;    // init failed
//  }
//  Serial.println("2");

  //File file = SD.open("/data2.txt");
  //if(!file) {
    //Serial.println("File doens't exist");
    //myPrinter.println(F("File doens't exist"));
    //myPrinter.feed(2);
//    Serial.println("Creating file...");
//    
//    myPrinter.println(F("Creating file..."));
//    myPrinter.feed(2);
//    
//    writeFile(SD, "/data2.txt", "Reading ID, \r\n Date, \r\nHour, \r\nTemperature \r\n");
//  }
//  else {
//    Serial.println("File already exists");  
//    myPrinter.println(F("File already exists"));
//    myPrinter.feed(2);
//  }
//  file.close();


  //readFileImage(SD, "/arriere_haut_gauche.bmp");

  mySerial.println("<Arduino is ready>");

  resetLine();
  
  //examplePrint();
}


void loop() {
//    myPrinter.printFromSerial();  // open monitor and print something
    recvWithStartEndMarkers();
    showNewData();
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {   
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                end_sequence = ndx;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
    if (newData == true) {        
        dataNumber = 0; 
        int offset = 1;
        char current_char;

        if (receivedChars[0] == 's') {
          myPrinter.printBitmap(start_line_data, width_pixel_form, start_line_height);
        }
        else {
            
          for (int i=0; i<end_sequence ; i++){
            current_char = receivedChars[i];
            if (current_char == 'z'){
              resetForm(offset);
            }
            else if (current_char == '_'){
              offset = offset + add_offset;
            }
            else if (current_char == 'a'){ // gauche
              if (receivedChars[i+1] == 'u'){ // #### au
                placeForm(offset, au);
              }
              else if (receivedChars[i+1] == 'l'){ // #### al
                placeForm(offset, al);
              }
              else if (receivedChars[i+1] == 'm'){ // #### am
                placeForm(offset, am);
              }
              else {
                // problem here
              }
              i++;
            }
            else if (current_char == 'b'){ // 
              if (receivedChars[i+1] == 'u'){ // ### bu
                  placeForm(offset, bu);
              }
              else if (receivedChars[i+1] == 'l'){ // bl
                  placeForm(offset, bl);
              }
              else if (receivedChars[i+1] == 'm'){ // bm
                  placeForm(offset, bm);
              }
              else {
                // problem here
              }
              i++;
            }
  
            else if (current_char == 'c'){ // c
              
              if (receivedChars[i+1] == 'x'){ // cx
                
                  if (receivedChars[i+2] == 'u'){ // cxu
                    placeForm(offset, cxu);
                  }
                  else if (receivedChars[i+2] == 'l'){ // cxl
                      placeForm(offset, cxl);
                  }
                  else if (receivedChars[i+2] == 'm'){ // cxm
                      placeForm(offset, cxm);
                  }
                  else {
                    // problem here
                  }
                  i++;
              }
              else if (receivedChars[i+1] == 'y'){ // cy
                
                  if (receivedChars[i+2] == 'u'){ // cyu
                    placeForm(offset, cyu);
                  }
                  else if (receivedChars[i+2] == 'l'){ // cyl
                      placeForm(offset, cyl);
                  }
                  else if (receivedChars[i+2] == 'm'){ // cym
                      placeForm(offset, cym);
                  }
                  else {
                    // problem here
                  }
                  i++;
              }
              else{
                // problem here 
              }
              
              i++;
            }
  
            else if (current_char == 'd'){ // d
              if (receivedChars[i+1] == 'u'){ // du
                  placeForm(offset, du);
              }
              else if (receivedChars[i+1] == 'l'){ // dl
                  placeForm(offset, dl);
              }
              else if (receivedChars[i+1] == 'm'){ // dm
                  placeForm(offset, dm);
              }
              else {
                // problem here
              }
              i++;
            }
  
            else if (current_char == 'e'){ // e
              if (receivedChars[i+1] == 'u'){ // eu
                  placeForm(offset, eu);
              }
              else if (receivedChars[i+1] == 'l'){ // el
                  placeForm(offset, el);
              }
              else if (receivedChars[i+1] == 'm'){ // em
                  placeForm(offset, em);
              }
              else {
                // problem here
              }
              i++;
            }
  
            else if (current_char == 'f'){ // f
              if (receivedChars[i+1] == 'u'){ // fu
                  placeForm(offset, fu);
              }
              else if (receivedChars[i+1] == 'l'){ // fl
                  placeForm(offset, fl);
              }
              else if (receivedChars[i+1] == 'm'){ // fm
                  placeForm(offset, fm);
              }
              else {
                // problem here
              }
              i++;
            }
  
            else if (current_char == 'g'){ // ### g ###
              if (receivedChars[i+1] == 'x'){ // gx
                
                  if (receivedChars[i+2] == 'u'){ // gxu
                    placeForm(offset, gxu);
                  }
                  else if (receivedChars[i+2] == 'l'){ // gxl
                      placeForm(offset, gxl);
                  }
                  else if (receivedChars[i+2] == 'm'){ // gxm
                      placeForm(offset, gxm);
                  }
                  else {
                    // problem here
                  }
                  i++;
              }
              else if (receivedChars[i+1] == 'y'){ // gy
                
                  if (receivedChars[i+2] == 'u'){ // gyu
                    placeForm(offset, gyu);
                  }
                  else if (receivedChars[i+2] == 'l'){ // gyl
                      placeForm(offset, gyl);
                  }
                  else if (receivedChars[i+2] == 'm'){ // gym
                      placeForm(offset, gym);
                  }
                  else {
                    // problem here
                  }
                  i++;
              }
              else {
                // problem here
              }
              i++;
            }
  
            else if (current_char == 'h'){ // ### h ###
              if (receivedChars[i+1] == 'u'){ // hu
                  placeForm(offset, hu);
              }
              else if (receivedChars[i+1] == 'l'){ // hl
                  placeForm(offset, hl);
              }
              else if (receivedChars[i+1] == 'm'){ // hm
                  placeForm(offset, hm);
              }
              else {
                // problem here
              }
              i++;
            }

            else if (current_char == 'i'){ // ### i ###
              if (receivedChars[i+1] == 'u'){ // iu
                  placeForm(offset, iu);
              }
              else if (receivedChars[i+1] == 'l'){ // il
                  placeForm(offset, il);
              }
              else if (receivedChars[i+1] == 'm'){ // im
                  placeForm(offset, im);
              }
              else {
                // problem here
              }
              i++;
            }


            else {
              // problem here 
              // case not supposed to happen 
            }
          
          }
          myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
        }
        newData = false;
    }
}

void helloUser() {
    // Hello 
    myPrinter.justify('L'); // text on left
    myPrinter.println(F("Hello..."));
    myPrinter.feed(1);
    
    myPrinter.justify('C'); // text in center
    myPrinter.println(F("You !!! "));
  
    myPrinter.justify('R'); // text on right
    myPrinter.println(F(":)"));
    myPrinter.feed(1);
  
    
    myPrinter.justify('C');
    myPrinter.feed(2);
}


void goodbyeUser() {
    // Hello 
    myPrinter.justify('L'); // text on left
    myPrinter.println(F("Goodbye..."));
    myPrinter.feed(1);
    
    myPrinter.justify('C'); // text in center
    myPrinter.println(F("See you soon! "));
    myPrinter.feed(1);
    
    myPrinter.justify('R'); // text on right
    myPrinter.println(F("<3"));
  
    
    myPrinter.justify('C');
    myPrinter.feed(2);
}


void startRecording() {
    // Hello 
    myPrinter.justify('C'); // text on center
    myPrinter.println(F("Recording..."));
  
    
    myPrinter.feed(2);
}

void stopRecording() {
    
    // Hello 
    myPrinter.justify('C'); // text on left
    myPrinter.println(F("Cut..."));
      
    myPrinter.feed(2);
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

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFileTxt(fs::FS &fs, const char * path){
  char rc;
  static int ndx = 0;
  
  Serial.printf("Reading file: %s\n", path);
  myPrinter.printf("Reading file: %s\n", path);
  myPrinter.feed(2);
  
  File file = SD.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  myPrinter.printf("Read from file: ");
  myPrinter.feed(2);
  
  ndx = 0;
  while(file.available()){
    rc = file.read();
    receivedChars[ndx] = rc;
    ndx++;
    if (ndx >= numChars) {
        ndx = numChars - 1;
    }
  }
  myPrinter.println(F("index : "));
  myPrinter.println(ndx);
  myPrinter.feed(2);
  
  myPrinter.println(receivedChars);
  myPrinter.feed(2);
  
  file.close();
}

void readFileImage(fs::FS &fs, const char * path){
  byte rc;
  static int ndx = 0;
  
  Serial.printf("Reading file: %s\n", path);
  myPrinter.printf("Reading file: %s\n", path);
  myPrinter.feed(2);
  
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  myPrinter.printf("Read from file: ");
  myPrinter.feed(2);
  
  ndx = 0;
  while(file.available()){
    rc = file.read();
    receivedBytes[ndx] = rc;
    ndx++;
    if (ndx >= numChars) {
        ndx = numChars - 1;
    }
  }
  
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
    
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void resetLine(){
  for (int i = 0; i < numOctets; i++) {
    line_to_modify[i] = blank_line_data[i];
  }
}

void resetForm(int offset){
  int pos_line = 0;
  int pos_form = 0;
    
  for (int i = 1; i < height_pixel_form + 1; i++) {

    for (int j = offset; j < offset + width_form_8_bit; j++) {
       pos_line = (i-1)*width_line_8_bit + j;

       if (offset <= j) {
          if (j <= offset + width_form_8_bit - 1) {
              pos_form = (i-1)*width_form_8_bit + (j - offset );
              line_to_modify[pos_line] = 0x00;
          }
       }
    }
  
  }
}

void placeFormTest(int offset){
  int pos_line = 0;
  int pos_form = 0;
    
  for (int i = 1; i < height_pixel_form + 1; i++) {

    for (int j = offset; j < offset + width_form_8_bit; j++) {
       pos_line = (i-1)*width_line_8_bit + j;

       if (offset <= j) {
          if (j <= offset + width_form_8_bit - 1) {
              pos_form = (i-1)*width_form_8_bit + (j - offset );
              line_to_modify[pos_line] = center_data[pos_form];
          }
       }
    }
  
  }

}

void placeForm(int offset, uint8_t* mvt_data){
  int pos_line = 0;
  int pos_form = 0;
    
  for (int i = 1; i < height_pixel_form + 1; i++) {

    for (int j = offset; j < offset + width_form_8_bit; j++) {
       pos_line = (i-1)*width_line_8_bit + j;

       if (offset <= j) {
          if (j <= offset + width_form_8_bit - 1) {
              pos_form = (i-1)*width_form_8_bit + (j - offset );
              line_to_modify[pos_line] = mvt_data[pos_form];
          }
       }
    }
  
  }

}


void examplePrint() {
  myPrinter.feed(1);
  myPrinter.printBitmap(start_line_data, width_pixel_form, start_line_height);

  resetLine();
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
  
  placeForm(column_8, diagonale_avant_haut_gauche_data);
  placeForm(column_9, diagonale_avant_haut_droit_data);
  placeForm(column_2, arriere_haut_droit_data);
  placeForm(column_3, arriere_haut_gauche_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
  
  placeForm(column_8, arriere_milieu_droit_data);
  placeForm(column_9, arriere_milieu_gauche_data);
  placeForm(column_2, centre_bas_data);
  placeForm(column_3, centre_haut_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
  
  placeForm(column_8, devant_bas_droit_data);
  placeForm(column_9, devant_bas_gauche_data);
  placeForm(column_2, devant_haut_droit_data);
  placeForm(column_3, devant_haut_gauche_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
  
   placeForm(column_8, devant_bas_droit_data);
  placeForm(column_9, devant_bas_gauche_data);
  placeForm(column_2, devant_haut_droit_data);
  placeForm(column_3, devant_haut_gauche_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
  
  placeForm(column_8, devant_milieu_droit_data);
  placeForm(column_9, diagonale_arriere_bas_droit_data);
  placeForm(column_2, diagonale_arriere_bas_gauche_data);
  placeForm(column_3, diagonale_arriere_haut_droit_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
    
  placeForm(column_8, diagonale_arriere_haut_gauche_data);
  placeForm(column_9, diagonale_arriere_milieu_droit_data);
  placeForm(column_2, diagonale_arriere_milieu_gauche_data);
  placeForm(column_3, diagonale_avant_bas_droit_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);

   placeForm(column_8, diagonale_avant_bas_gauche_data);
  placeForm(column_9, diagonale_avant_milieu_droit_data);
  placeForm(column_2, diagonale_arriere_milieu_gauche_data);
  placeForm(column_3, diagonale_avant_milieu_gauche_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);

   placeForm(column_8, droit_bas_data);
  placeForm(column_9, droit_haut_data);
  placeForm(column_2, droit_milieu_data);
  placeForm(column_3, gauche_bas_data);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);

   placeForm(column_8, gauche_haut_data);
  placeForm(column_9, gauche_milieu_data);
  resetForm(column_2);
  resetForm(column_3);
  myPrinter.printBitmap(line_to_modify, width_pixel_form, height_pixel_form);
    myPrinter.feed(4);
}
