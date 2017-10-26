

/*
    Arduino SD Card Tutorial Example

    by Dejan Nedelkovski, www.HowToMechatronics.com
*/
#include <SD.h>
#include <SPI.h>
#include "MultichannelGasSensor2.h"
#include "RTClib.h"
#include <dht.h>
#include <SoftwareSerial.h>

// the following functions are written at the end of the code and defined below 

void printTime(DateTime now);
void serialPrintTime(DateTime now);
void serialPrintHT(); 
void getMCGS(); 
void getN0X(); 
void getO3(); 
   
int iter = 5; // iter used to declare the number of times a reading is averaged over before printing to SD card - used to slow down number of readings printed to SD 
File myFile;
int chipSelect = 4;
dht DHT;
#define DHT11_PIN 7
int i = 0;
SoftwareSerial mySerialO(2, 3); // RX, TX for O3 SPEC SENSOR 
SoftwareSerial mySerial(5, 4); // RX, TX for NO2 SPEC SENSOR 

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  pinMode(chipSelect, OUTPUT);
  gas.begin(0x04);
  gas.powerOn();
  Serial.println(gas.getVersion());
  RTC_DS1307 RTC;

  // SD Card Initialization

  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }

  if (!RTC.begin()) {
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }

}

void loop() {
  delay(500);
  // digitalWrite(3, HIGH);
  float c;
  DateTime now;
  RTC_DS1307 RTC;

  myFile = SD.open("TEST2.txt", FILE_WRITE);

  if (myFile) {

    // printing time and date to serial and SD card

   now = RTC.now();

// print headings to SD card 
   //myFile.print("Date, Time, Temperature, Humidity, Rs0, Rs1, Rs2, SN NO2, PPB NO2, T (°C) NO2, RH (%) NO2, ADC Raw NO2, T Raw NO2, RH Raw NO2, Day NO2, Hour NO2, Minute NO2, Second NO2, SN O3, PPB O3, T (°C) O3, RH (%) O3, ADC Raw O3, T Raw O3, RH Raw O3, Day O3, Hour O3, Minute O3, Second O3"); 
   //myFile.print("\n");

   printTime(now); // function that prints time to SD card
   serialPrintTime(now); // function that prints time to serial   
    
   getHT();   // taking and humitidy reading from DHT.11 sensor and printing to serial and SD card
   delay(500); 
   getMCGS(); //taking reads from multichannel gas sensor and printing to serial and SD card 

   
   getN0X(); // taking NO2 readings from SPEC sensor and prints to serial  
   getO3(); // taking O2 readings from SPEC sensor and prints to serial 
   
   
   delay(200);
   Serial.println("...");

    myFile.close();

  }

} 

void printTime(DateTime now){
    myFile.print(now.year(), DEC);
    myFile.print("-");
    myFile.print(now.month(), DEC);
    myFile.print("-");
    myFile.print(now.day(), DEC);
    myFile.print(" ");
    myFile.print(now.hour(), DEC);
    myFile.print(":");
    myFile.print(now.minute(), DEC);
    myFile.print(":");
    myFile.print(now.second(), DEC);
    myFile.print(",");
}



void serialPrintTime(DateTime now){
  
    Serial.print(now.year(), DEC);
    Serial.print("-");
    Serial.print(now.month(), DEC);
    Serial.print("-");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print("\n");
}

void getMCGS(){
  
    float sumR0 = 0;
    float sumR1 = 0;
    float sumR2 = 0;
    float ratio0 = gas.getRs(0);
    float ratio1 = gas.getRs(1);
    float ratio2 = gas.getRs(2);

    for (int j = 0; j < iter; j++) {
      gas.getRs(0);
      gas.getRs(1);
      gas.getRs(2);
      sumR0 += ratio0;
      sumR1 += ratio1;
      sumR2 += ratio2;
      delay(1000);
    }

    sumR0 = sumR0 / (float)iter;
    sumR1 = sumR1 / (float)iter;
    sumR2 = sumR2 / (float)iter;

    Serial.print(sumR0);
    Serial.print("\n");
    Serial.print(sumR1);
    Serial.print("\n");
    Serial.print(sumR2);
    Serial.print("\n");

    myFile.print(sumR0); 
    myFile.print(","); 
    myFile.print(sumR1); 
    myFile.print(",");
    myFile.print(sumR2); 
    myFile.print(",");
}

void getHT(){
    
    float sumT = 0;
    float sumH = 0;
    
for (int j = 0; j < iter; j++) {
      DHT.read11(DHT11_PIN);
      sumT += DHT.temperature;
      sumH += DHT.humidity;
      delay(1000);
    }
    
    sumT = sumT / (float)iter;
    sumH = sumH / (float)iter;
    
    Serial.print(sumT);
    Serial.print("\n");
    Serial.print(sumH);
    Serial.print("\n");

    myFile.print(sumT); 
    myFile.print(",");
    myFile.print(sumH); 
    myFile.print(","); 
}
     
void getN0X() {
  
  mySerial.begin(9600);
  delay(700);
  
  unsigned int avgs[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (int j = 0; j < iter; ++j) {
      mySerial.print("c");
      delay(1000);
      int index = 0;

      while (mySerial.available() && index < 12) { 
        int c = mySerial.parseInt();
        avgs[index] += c;
        ++index;
        delay(500);
      }
     
    }
    for (int j = 1; j < 11; ++j) {
      avgs[j] = avgs[j] / iter;
      Serial.println(avgs[j]);
      myFile.print(avgs[j]);
       
    }
   mySerial.end();
}

void getO3() {

  mySerialO.begin(9600);
  delay(700); 
  unsigned int avgs[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (int l = 0; l < iter; ++l) {
      mySerialO.print("c");
      delay(1000);
      int index = 0;
      while (mySerialO.available() && index < 12) {
        int c = mySerialO.parseInt();
        avgs[index] += c;
        ++index;
        delay(500);
      }
    }
    
    for (int l = 1; l < 11; ++l) {
      avgs[l] = avgs[l] / iter;
      Serial.println(avgs[l]);  
      myFile.print(avgs[l]); 
       
    }
    myFile.print("\n");
    mySerialO.end();
}
