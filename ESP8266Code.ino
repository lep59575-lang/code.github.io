// phai khai bao truoc moi ket noi blynk duoc
#define BLYNK_TEMPLATE_ID "TMPL6zx1qEG0W"
#define BLYNK_TEMPLATE_NAME "SMARTSTATION"
#define BLYNK_AUTH_TOKEN "2bWLLl0gkWKDdTgyfyFuQNznKPkVxZcT"


#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include <BlynkSimpleEsp8266.h>


#define CMDONPUMP 0xB1
#define CMDOFFPUMP 0xB2

#define CMDONCHARGE 0xB3
#define CMDOFFCHARGE 0xB4


#define SRX D5 
#define STX D6

#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#define SW_PIN 0
#define LED 2       //On board LED


char blynk_token[] = BLYNK_AUTH_TOKEN;
//char blynk_token[] ="";
SoftwareSerial DataSerial(SRX, STX);
//#define DataSerial Serial


byte UARTByteCount;
byte UARTBuffer[50];


BlynkTimer UARTTimer;
BlynkTimer BlynkReconnectTimer;

int Voltage=0;
float Current=0;
float Power=0;
int Energy=0;

byte PumpCount=0;
byte ChargeCount=0;

int MinuteCharge=0;

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
      else if(UARTDataCome==0xFD||UARTDataCome==0xFC)
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
        
        Blynk.virtualWrite(V0,Voltage);
        Blynk.virtualWrite(V1,Current);
        Blynk.virtualWrite(V2,Power);
        Blynk.virtualWrite(V3,Energy);
        Blynk.virtualWrite(V4,PumpCount);
        Blynk.virtualWrite(V5,ChargeCount);
        Blynk.virtualWrite(V8,MinuteCharge);
      }
      else
      {
          UARTBuffer[UARTByteCount]=UARTDataCome;
          UARTByteCount++;
          if(UARTByteCount==50) UARTByteCount=0; //prevent accidentcount
      }
   }
   if(Serial.available())
   {
    UARTDataCome=Serial.read();
    DataSerial.write(UARTDataCome);
   }
}



BLYNK_WRITE(V6) 
{
  int Value = param.asInt();
  if(Value==1) 
  {
    DataSerial.write(CMDONPUMP);
  }
  else 
  {
    DataSerial.write(CMDOFFPUMP);
  }
 
}
BLYNK_WRITE(V7) 
{
  int Value = param.asInt();
  if(Value==1) 
  {
    DataSerial.write(CMDONCHARGE);
  }
  else 
  {
    DataSerial.write(CMDOFFCHARGE);
  }
 
}

void BlynkReconnect()
{
  if(Blynk.connected()==0)
  {
    Blynk.connect(3333);
  }
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
   Serial.println();
   Serial.println("Reset wifi config!");
   
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
   WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", blynk_token, 34);
   wifiManager.addParameter(&custom_blynk_token);
   wifiManager.autoConnect("SMART STATION","12345678");
   //wifiManager.autoConnect();// use this to display host as ESP name + CHIPID
  // if can go next mean already connected wifi
  Serial.println("YOU ARE CONNECTED TO WIFI");
  Serial.println(custom_blynk_token.getValue());
  Blynk.config(custom_blynk_token.getValue());
  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println("WIFI CONNECTED SUCCESSFULLY! NOW TRY TO CONNECT BLYNK..."); 
   
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
    Blynk.connect(3333); // try to connect to Blynk with time out 10 second
   if(Blynk.connected()) 
   {
    Serial.println("BLYNK CONNECTED! System ready"); 
    
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
   }
   else 
   {
    Serial.println(" BLYNK Not connect. warning!");
    digitalWrite(LED,LOW);
    
   }
  }
  else 
  {
    Serial.println("WIFI Not connect. warning!");
    digitalWrite(LED,LOW);
   
  }
  //MAIN CODE FOR SETUP
  DataSerial.begin(9600); // thiet lan ket noi voi arduino
  UARTTimer.setInterval(1L,UARTRead);
  BlynkReconnectTimer.setInterval(1000L,BlynkReconnect);

}

void loop() {
  Blynk.run();
  UARTTimer.run();
  BlynkReconnectTimer.run();

  // put your main code here, to run repeatedly:

}
