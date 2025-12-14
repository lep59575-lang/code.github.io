#include <PZEM004Tv30.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


#define CMDONPUMP 0xB1
#define CMDOFFPUMP 0xB2

#define CMDONCHARGE 0xB3
#define CMDOFFCHARGE 0xB4

#define CMDWIFI 0xB5
#define CMDRFIDLOGIN1 0xB6
#define CMDRFIDLOGIN2 0xB7
#define CMDRFIDLOGIN3 0xB8

#define CMDRFIDEXIT 0xB9


#define RELAYPUMP PC15
#define RELAYCHARGE PC14

#define BUTTONPUMP PB11
#define BUTTONCHARGE PB10

#define BUZZER PA11
#define CHARGEFULL PB12

#define PUMPRICE "2.000"

LiquidCrystal_I2C LCDDisplay(0x27,20,4);

unsigned long TimeCount;
PZEM004Tv30 *Pzem;   // pointer instead of global object
float Voltage, Current, Power, Energy;

byte ChargeState=0;
byte PumpState=0;

byte PumpCount=0;
int PumpMoney=0;

byte ChargeCount=0;

unsigned long ChargeTimeStart=0;
unsigned long ChargeTimeCount=0;
unsigned long ChargeMinute=0;
unsigned long ChargeMoney=0;
unsigned long TotalChargeMinute=0;

byte FullFlag=0;
unsigned long TimeFull=0;


int PumpCountLimit=20;
int ChargeCountLimit=10;

byte UARTData=0;

byte SystemReady=0;

void UpdateDisplay()
{
  Pzem->readAddress();
  //Serial.println("TEST"); 
  Voltage = Pzem->voltage();
  //Serial.println("V = " + (String)Voltage + " Volts");

  Current =  Pzem->current();
  //Serial.println("I = " + (String)Current + " Amps");
  
  Power = Pzem->power();
  //Serial.println("P = " + (String)Power + " W");
  
  Energy = Pzem->energy();
  //Serial.println("E = " + (String)Energy + " W*h");

  if(Power>200) OnBuzzer();
  else OffBuzzer();
}

void DisplayMain()
{
  LCDDisplay.clear();
  LCDDisplay.print("HT BOM HOI SAC DIEN");
  LCDDisplay.setCursor(0, 1); 
  LCDDisplay.print("DA:    V   DD:    A");
  LCDDisplay.setCursor(0, 2);
  LCDDisplay.print("CS:    W   DN:   KWh");
}

void DisplayParameter()
{
  
  if(Voltage==0)
  {
    LCDDisplay.setCursor(0, 3);
    LCDDisplay.print(" MAT NGUON DIEN ");
    LCDDisplay.setCursor(3, 1);
    LCDDisplay.print("    ");
    LCDDisplay.setCursor(3, 1);
    LCDDisplay.print((int)Voltage);
  }
  else
  {
      LCDDisplay.setCursor(3, 1);
      LCDDisplay.print("    ");
      LCDDisplay.setCursor(3, 1);
      LCDDisplay.print((int)Voltage);
      LCDDisplay.setCursor(14, 1);
      LCDDisplay.print("    ");
      LCDDisplay.setCursor(14, 1);
      LCDDisplay.print(Current,2);
      LCDDisplay.setCursor(3, 2);
      LCDDisplay.print("    ");
      LCDDisplay.setCursor(3, 2);
      LCDDisplay.print(Power,1);
      LCDDisplay.setCursor(14, 2);
      LCDDisplay.print("   ");
      LCDDisplay.setCursor(14, 2);
      LCDDisplay.print((int)Energy );
  }
}


void OnPump()
{
  digitalWrite(RELAYPUMP,LOW);
}
void OffPump()
{
  digitalWrite(RELAYPUMP,HIGH);
}
void OnCharge()
{
  digitalWrite(RELAYCHARGE,LOW);
}
void OffCharge()
{
  digitalWrite(RELAYCHARGE,HIGH);
}


void OnBuzzer()
{
  digitalWrite(BUZZER,HIGH);
}
void OffBuzzer()
{
  digitalWrite(BUZZER,LOW);
}

void DisplayRunPump()
{
  LCDDisplay.setCursor(0, 3);
  LCDDisplay.print("DANG BOM..");
}

void DisplayRunCharge()
{
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print("DANG SAC..");
}

void DisplayPumpDone()
{
  LCDDisplay.setCursor(0, 3);
  LCDDisplay.print("BOM XONG! ");
}

void DisplayPumpLimit()
{
  LCDDisplay.setCursor(0, 3);
  LCDDisplay.print("QUA GH BOM");
}
void DisplayChargeDone()
{
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print("SAC XONG! ");
}
void DisplayChargeLimit()
{
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print("QUA GH SAC");
}

void DisplayPumpMoney()
{
  LCDDisplay.setCursor(0, 3);
  LCDDisplay.print("          ");
  LCDDisplay.setCursor(0, 3);
  LCDDisplay.print(PUMPRICE);
}

void ClearPump()
{
  LCDDisplay.setCursor(0, 3);
  LCDDisplay.print("           ");
}

void DisplayChargeTime()
{
  ChargeMinute = millis()-ChargeTimeStart;
  ChargeMinute =ChargeMinute/60000;
  TotalChargeMinute=ChargeMinute;
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print("          ");
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print(ChargeMinute);
  LCDDisplay.print("h");
}

void DisplayChargeMoney()
{
  
  if(ChargeMinute<3)
  {
    ChargeMoney=ChargeMinute*10;
  }
  else if(ChargeMinute<5)
  {
     ChargeMoney=ChargeMinute*7;
  }
  else
  {
    ChargeMoney=ChargeMinute*5;
  }
  
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print("          ");
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print(ChargeMoney);
  LCDDisplay.print(".000");
}

void DisplayClearCharge()
{
  LCDDisplay.setCursor(10, 3);
  LCDDisplay.print("          ");
}

void SendESPData()
{
  int Value=0;
  Value=(int)Voltage;
  Serial.write(0xFE);
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

  Value=(int)(Current*10);
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

   Value=(int)(Power*10);
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

   Value=(int)Energy;
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

  Serial.write(PumpCount);
  Serial.write(ChargeCount);

  Value=(int)TotalChargeMinute;
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);
  Serial.write(0xFD);
}

void SendESPDataDone()
{
  int Value=0;
  Value=(int)Voltage;
  Serial.write(0xFE);
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

  Value=(int)(Current*10);
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

   Value=(int)(Power*10);
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

   Value=(int)Energy;
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

  Serial.write(PumpCount);
  Serial.write(ChargeCount);

  Value=(int)TotalChargeMinute;
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);

  Value=(int)ChargeMoney;
  Serial.write(Value/1000);
  Serial.write((Value%1000)/100);
  Serial.write((Value%100)/10);
  Serial.write(Value%10);
  
  Serial.write(0xFC);
}

void DisplayWaitWifi()
{
  LCDDisplay.clear();
  LCDDisplay.print("CHO KET NOI WIFI...");
  LCDDisplay.setCursor(0, 1); 
}
void DisplayWaitforRFID()
{
  LCDDisplay.clear();
  LCDDisplay.print("HAY QUET THE RFID ");
}


void setup() {
  Serial.begin(9600);                    // Thiết lập kết nối Serial để truyền dữ liệu đến máy tính
  Serial2.begin(9600);
  // Initialize PZEM only after Serial2 is started
  Pzem = new PZEM004Tv30(Serial2);

  pinMode(RELAYPUMP,OUTPUT);
  pinMode(RELAYCHARGE,OUTPUT);
  pinMode(BUTTONPUMP,INPUT);
  pinMode(BUTTONCHARGE,INPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(CHARGEFULL,INPUT);
  OffPump();
  OffCharge();
  OffBuzzer();
  Wire.begin();
  Wire.setClock(50000);   // 50 kHz instead of 100kHz/400kHz
  
  LCDDisplay.init();
  LCDDisplay.backlight();

  DisplayWaitWifi();
  
  Serial.println("START"); 
  
  OnBuzzer();
  delay(1000);
  OffBuzzer();
  TimeCount=millis();
  TimeFull=millis();
}

void loop() {


  if(SystemReady==1)
  {
      byte Check=digitalRead(CHARGEFULL);
      delay(1); // delay 1 de ham check update gia tri  
      //Serial.print("FULL:");
      //Serial.println(Check);
      if(Check==1) // neu khong delay 1 thi Check luon = 1
      {
        TimeFull=millis();
        FullFlag=0;
        //Serial.println("Check Full");
      }
      /*
      if(millis()-TimeFull>10000)
      {
         FullFlag=1;
         TimeFull=millis();
         Serial.println("Check Full");
      }
    
      */
      if(millis()-TimeCount>5000)
      {
         
         //Serial.print("FULL:");
         //Serial.println(FullFlag);
         UpdateDisplay();
         DisplayParameter();
         SendESPData();
         if(FullFlag==1)
         {
           OnBuzzer();
           delay(500);
           OffBuzzer();
         }
         TimeCount=millis();
         FullFlag=1;
      }
    
      if(digitalRead(BUTTONCHARGE)==LOW)
      {
          delay(350);
          while(digitalRead(BUTTONCHARGE)==LOW);
          if(ChargeCount<ChargeCountLimit)
          {
            if(ChargeState==0)
            {
              OnCharge();
              ChargeState=1;
              ChargeTimeStart=millis();
              DisplayRunCharge();
            }
            else
            {
              OffCharge();
              ChargeCount++;
              
              ChargeState=0;
              DisplayChargeDone();
              delay(1000);
              DisplayChargeTime();
              delay(1000);
              DisplayChargeMoney();
              SendESPDataDone();
              delay(2000);
              DisplayClearCharge();
            }
          }
          else
          {
            DisplayChargeLimit();
            delay(2000);
            DisplayClearCharge();
          }
          delay(350);
      }
       if(digitalRead(BUTTONPUMP)==LOW)
      {
          delay(350);
          while(digitalRead(BUTTONPUMP)==LOW);
    
          if(PumpCount<PumpCountLimit)
          {
            if(PumpState==0)
            {
              OnPump();
              DisplayRunPump();
              PumpState=1;
            }
            else
            {
              OffPump();
              PumpCount++;
             
              DisplayPumpDone();
              delay(1000);
              DisplayPumpMoney();
              SendESPDataDone();
              delay(2000);
              ClearPump();
              PumpState=0;
            }
          }
          else
          {
            DisplayPumpLimit();
            delay(2000);
            ClearPump();
          }
      }
    
      if(Serial.available())
      {
        UARTData=Serial.read();
        if(UARTData==CMDONPUMP)
        {
           if(PumpCount<PumpCountLimit)
          {
              OnPump();
              DisplayRunPump();
              PumpState=1;
          }
          else
          {
            DisplayPumpLimit();
            delay(2000);
            ClearPump();
          }
        }
        else  if(UARTData==CMDOFFPUMP)
        {
              OffPump();
              PumpCount++;
             
              DisplayPumpDone();
              delay(1000);
              DisplayPumpMoney();
               SendESPDataDone();
              delay(2000);
              ClearPump();
              PumpState=0;
        }
        else  if(UARTData==CMDONCHARGE)
        {
          if(ChargeCount<ChargeCountLimit)
          {
              OnCharge();
              ChargeState=1;
              ChargeTimeStart=millis();
              DisplayRunCharge();
          }
          else
          {
            DisplayChargeLimit();
            delay(2000);
            DisplayClearCharge();
          }
        }
         else  if(UARTData==CMDOFFCHARGE)
        {
              OffCharge();
              ChargeCount++;
              
              ChargeState=0;
              DisplayChargeDone();
              delay(1000);
              DisplayChargeTime();
              delay(1000);
              DisplayChargeMoney();
              SendESPDataDone();
              delay(2000);
              DisplayClearCharge();
        }
        else if(UARTData==CMDRFIDEXIT)
        {
          OnBuzzer();
          delay(200);
          OffBuzzer();
          DisplayWaitforRFID();
          SystemReady=0;
        }
      }
  }
  else
  {
    if(Serial.available())
      {
        UARTData=Serial.read();
        if(UARTData==CMDWIFI)
        {
          DisplayWaitforRFID();
        }
        else if(UARTData==CMDRFIDLOGIN1)
        {
          LCDDisplay.print("1");
          OnBuzzer();
          delay(200);
          OffBuzzer();
          delay(1000);
          DisplayMain();
          SystemReady=1;
          
        }
        else if(UARTData==CMDRFIDLOGIN2)
        {
          LCDDisplay.print("2");
          OnBuzzer();
          delay(200);
          OffBuzzer();
          delay(1000);
          DisplayMain();
          SystemReady=1;
          
        }
        else if(UARTData==CMDRFIDLOGIN3)
        {
          LCDDisplay.print("3");
          OnBuzzer();
          delay(200);
          OffBuzzer();
          delay(1000);
          DisplayMain();
          SystemReady=1;
        }
      }
  }
}
