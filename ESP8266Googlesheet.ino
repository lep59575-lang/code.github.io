

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define MAXRECORD 10
byte CurrentRFID[4];
byte RFIDData[MAXRECORD][4] = {0};

#define pinRST      D1
#define pinSDA      D2

#define SW_PIN 0
#define LED 2       //On board LED


#define CMDWIFI 0xB5
#define CMDRFIDLOGIN1 0xB6
#define CMDRFIDLOGIN2 0xB7
#define CMDRFIDLOGIN3 0xB8


#define CMDRFIDEXIT 0xB9

byte LastPumpCount=0;
byte LastChargeCount=0;
MFRC522 MyRFID(pinSDA, pinRST);

unsigned long TimeCheckRFID=0;
unsigned long TimeResetRFID=0;
// Google Apps Script
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
String SCRIPT_ID = "AKfycbyCha5bqvvbb5uu0MlQSI37e4OUlGUQyC8UJ0V6nsQS086cYNc2_rfm4Ist_G6KAWdHCA";



#define DataSerial Serial


byte UARTByteCount;
byte UARTBuffer[50];



int Voltage=0;
float Current=0;
float Power=0;
int Energy=0;

byte PumpCount=0;
byte ChargeCount=0;

int MinuteCharge=0;

int ChargeMonney=0;



byte CurrentLogInID = 0;
byte CheckLogInID = 0;
byte ExitLogInID = 0;
byte RFIDActive = 0;


// ---------------- EEPROM ----------------
void WriteDataEEPROM()
{
  for (int i = 0; i < MAXRECORD; i++)
  {
    EEPROM.write(i * 4 + 0, RFIDData[i][0]);
    EEPROM.write(i * 4 + 1, RFIDData[i][1]);
    EEPROM.write(i * 4 + 2, RFIDData[i][2]);
    EEPROM.write(i * 4 + 3, RFIDData[i][3]);
  }
  EEPROM.commit();
}

void ReadDataEEPROM()
{
  byte EEPROMData = EEPROM.read(0);
  if (EEPROMData == 0xFF)
  {
    WriteDataEEPROM();
  } 
  else 
  {
    for (int i = 0; i < MAXRECORD; i++)
    {
      RFIDData[i][0] = EEPROM.read(i * 4 + 0);
      RFIDData[i][1] = EEPROM.read(i * 4 + 1);
      RFIDData[i][2] = EEPROM.read(i * 4 + 2);
      RFIDData[i][3] = EEPROM.read(i * 4 + 3);
    }
  }
}

void SaveCardCode()
{

  RFIDData[0][0]=0x61;
  RFIDData[0][1]=0x12;
  RFIDData[0][2]=0xA7;
  RFIDData[0][3]=0x17;

  
  RFIDData[1][0]=0x51;
  RFIDData[1][1]=0xEB;
  RFIDData[1][2]=0x61;
  RFIDData[1][3]=0x17;

  RFIDData[2][0]=0x60;
  RFIDData[2][1]=0xCB;
  RFIDData[2][2]=0x60;
  RFIDData[2][3]=0x61;
  
  WriteDataEEPROM();
}

void CheckRFID()
{

  if (MyRFID.PICC_IsNewCardPresent()) 
  {
    Serial.print("RFID detected!");
    if (MyRFID.PICC_ReadCardSerial() && RFIDActive==0)
    {
      Serial.print("RFID TAG ID:");
      for (byte i=0; i<MyRFID.uid.size; ++i)
      {
        Serial.print(MyRFID.uid.uidByte[i], HEX);
        Serial.print(" ");
        CurrentRFID[i]=MyRFID.uid.uidByte[i];
      }
      Serial.println();
      RFIDActive=1;

    
      CheckLogInID=VerifyRFID();
      
      if(CurrentLogInID==0&&CheckLogInID!=0)
      {
        CurrentLogInID=CheckLogInID;
        ExitLogInID=0;
        if(CurrentLogInID==1) Serial.write(CMDRFIDLOGIN1);
        else if(CurrentLogInID==2) Serial.write(CMDRFIDLOGIN2);
        else if(CurrentLogInID==3) Serial.write(CMDRFIDLOGIN3);
      }
      else if(CurrentLogInID!=0&&CurrentLogInID==CheckLogInID)
      {
        ExitLogInID=CurrentLogInID;
        CurrentLogInID=0;
        Serial.write(CMDRFIDEXIT);
        sendData("EXIT", 0, 0, 0,  0, 0);
      }
      
    }

      // *** Important cleanup ***
  MyRFID.PICC_HaltA();        // halt card
  MyRFID.PCD_StopCrypto1();   // stop encryption
  
  }

 
}

/*

void sendData(String Type, int Among, int Voltage, float Current, float Power, int ChargeTime) {
  Serial.println("==========");

  String url = "/macros/s/" + SCRIPT_ID + "/exec?" +
               "Type=" + Type +
               "&Among=" + String(Among) +
               "&Voltage=" + String(Voltage) +
               "&Current=" + String(Current) +
               "&Power=" + String(Power) +
               "&ChargeTime=" + String(ChargeTime);

  bool success = false;
  client.setInsecure(); // allow HTTPS without certificate

  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.printf("Attempt %d connecting...\n", attempt);

    if (client.connect(host, httpsPort))
    {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      Serial.println(host+url);
      
      unsigned long timeout = millis();
      while (client.connected() && millis() - timeout < 3000)
      {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;
      }

      Serial.println("✅ Data sent to Google Sheet");
      client.stop();
      success = true;
      break;
    }
    client.stop();
    delay(500);
  }

  if (!success) Serial.println("⚠️ FAILED to send!");
}

*/

void sendData(String Type, int Among, int Voltage, float Current, float Power, int ChargeTime)
{
  Serial.println("\n=== Sending to Google Sheets ===");

  String url = "/macros/s/" + SCRIPT_ID + "/exec?" +
               "Type=" + Type +
               "&Among=" + String(Among) +
               "&Voltage=" + String(Voltage) +
               "&Current=" + String(Current) +
               "&Power=" + String(Power) +
               "&ChargeTime=" + String(ChargeTime) +
               "&IDE" + String(ExitLogInID) + 
               "&ID=" + String(CurrentLogInID);

  bool saved = false;
  client.setInsecure();

  for (int attempt = 1; attempt <= 5; attempt++)
  {
    Serial.printf("🔄 Attempt %d...\n", attempt);

    if (!client.connect(host, httpsPort)) {
      Serial.println("❌ Connect failed");
    }
    else {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: script.google.com\r\n" +
                   "User-Agent: ESP8266\r\n" +
                   "Connection: close\r\n\r\n");

      unsigned long timeout = millis();
      while (millis() - timeout < 5000) {
        if (client.available()) {
          String line = client.readStringUntil('\n');
          Serial.println(line);

          // ✅ Accept 302 and 200 as success from Google Script
          if (line.startsWith("HTTP/1.1 302") || line.startsWith("HTTP/1.1 200")) {
            saved = true;
          }
        }
        if (!client.connected()) break;
      }
      client.stop();
    }

    if (saved) {
      Serial.println("✅ Send success!");
      break;
    }

    Serial.println("⏳ Retry in 1s...");
    delay(1000);
  }

  if (!saved)
    Serial.println("⚠️ FAILED after 5 attempts!");
}



void UARTRead()
{
   byte UARTDataCome;
  
   if (DataSerial.available()>0)
   {
      UARTDataCome=DataSerial.read();
      Serial.write(UARTDataCome);
      if(UARTDataCome==0xFE)
      {
        UARTByteCount=0;
      }
      else if(UARTDataCome==0xFD)
      {
        UARTByteCount=0;
      }
      else if(UARTDataCome==0xFC)
      {
        Voltage=UARTBuffer[0]*1000+UARTBuffer[1]*100+UARTBuffer[2]*10+UARTBuffer[3];
        Current=UARTBuffer[4]*1000+UARTBuffer[5]*100+UARTBuffer[6]*10+UARTBuffer[7];
        Current=Current/10;
        Power=UARTBuffer[8]*1000+UARTBuffer[9]*100+UARTBuffer[10]*10+UARTBuffer[11];
        Power=Power/10;
        Energy=UARTBuffer[12]*1000+UARTBuffer[13]*100+UARTBuffer[14]*10+UARTBuffer[15];
        PumpCount=UARTBuffer[16];
        ChargeCount=UARTBuffer[17];
        MinuteCharge = UARTBuffer[18]*1000+UARTBuffer[19]*100+UARTBuffer[20]*10+UARTBuffer[21];

        ChargeMonney = UARTBuffer[22]*1000+UARTBuffer[23]*100+UARTBuffer[24]*10+UARTBuffer[25];
        if(LastPumpCount!=PumpCount)
        {
          sendData("BOM", 2000, Voltage, Current,  Power, 0);
        }
        if(LastChargeCount!=ChargeCount)
        {
          sendData("SAC", ChargeMonney, Voltage, Current,  Power, MinuteCharge);
          LastChargeCount=ChargeCount;
        }
        LastPumpCount=PumpCount;
        LastChargeCount=ChargeCount;
      
      }
      else
      {
          UARTBuffer[UARTByteCount]=UARTDataCome;
          UARTByteCount++;
          if(UARTByteCount==50) UARTByteCount=0; //prevent accidentcount
      }
   }     
}


byte VerifyRFID() {
  for (int i=0; i<MAXRECORD; i++)
  {
    if (RFIDData[i][0]==CurrentRFID[0] &&
        RFIDData[i][1]==CurrentRFID[1] &&
        RFIDData[i][2]==CurrentRFID[2] &&
        RFIDData[i][3]==CurrentRFID[3])
    {
      return i+1;
    }
  }
  return 0;
}


void setup() {
  Serial.begin(9600);                    // Thiết lập kết nối Serial để truyền dữ liệu đến máy tính
 
  pinMode(SW_PIN,INPUT_PULLUP); // Nut tren board bam de xoa wifi da luu
  pinMode(LED,OUTPUT);
  UARTByteCount=0;

  WiFiManager wifiManager;                 // Khởi tạo đối tượng cho WiFiManager
  Serial.println("Delete old wifi? Press Flash button within 3 second");
  for(int i=3;i>0;i--)
  {
    Serial.print(String(i)+" ");
    delay(1000);
      }
  if(digitalRead(SW_PIN)==LOW)// press button
  {
   //Serial.println();
   //Serial.println("Reset wifi config!");
   
   digitalWrite(LED,LOW); //nhap nhay LED
   delay(200);
   digitalWrite(LED,HIGH);
   delay(200);
   digitalWrite(LED,LOW);
   delay(200);
   digitalWrite(LED,HIGH);
   delay(200);
   digitalWrite(LED,LOW);
   delay(200);
   digitalWrite(LED,HIGH);
   delay(200);
   digitalWrite(LED,LOW);
   
   wifiManager.resetSettings();   
   
  }

   wifiManager.autoConnect("SMART SHEET","12345678");
   //wifiManager.autoConnect();// use this to display host as ESP name + CHIPID
  // if can go next mean already connected wifi
  Serial.println("YOU ARE CONNECTED TO WIFI");

  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println("WIFI CONNECTED SUCCESSFULLY!"); 
   
    digitalWrite(LED,LOW); //nhap nhay LED
    delay(200);
    digitalWrite(LED,HIGH);
    delay(200);
    digitalWrite(LED,LOW);
    delay(200);
    digitalWrite(LED,HIGH);
    delay(200);
    digitalWrite(LED,LOW);
    delay(200);
    digitalWrite(LED,HIGH);
    delay(1000); 

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    client.setInsecure();

    delay(10000);
    Serial.write(CMDWIFI);
  
  }
  else 
  {
    //Serial.println("WIFI Not connect. warning!");
    digitalWrite(LED,LOW);
   
  }
  //MAIN CODE FOR SETUP

   
  EEPROM.begin(512);
  SPI.begin(); 
  MyRFID.PCD_Init(); 
  MyRFID.PCD_SetAntennaGain(MyRFID.RxGain_max);
  // Library sometimes allows set SPI clock manually:
  MyRFID.PCD_SetRegisterBitMask(MyRFID.RFCfgReg, (0x07<<4)); // max gain
  Serial.print("MFRC522 Version: ");
  byte version = MyRFID.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.println(version, HEX);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: MFRC522 not detected. Check wiring and power!");
  } else {
    Serial.println("MFRC522 detected OK!");
  }
  SaveCardCode();
  ReadDataEEPROM();
  
  TimeCheckRFID=millis();
  TimeResetRFID=millis();
}

void loop() {
 
  // put your main code here, to run repeatedly:
  UARTRead();

   if(millis()-TimeResetRFID>2000)
  {
    RFIDActive=0;
    TimeResetRFID=millis();
  }
  if(millis()-TimeCheckRFID>500)
  {
    CheckRFID();
    TimeCheckRFID=millis();
  }
  
  
}
