#include <LiquidCrystal.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <BlynkSimpleStream.h>
#define BLYNK_PRINT DebugSerial

#define BLYNK_TEMPLATE_ID "TMPLrMz1KHYw"
#define BLYNK_DEVICE_NAME "MediCheck"
#define BLYNK_AUTH_TOKEN "sSYA8HGBVeoyKH7lZYtPXAeR-4qxXiH5"

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
TinyGPS gps;
SoftwareSerial mySerial(17, 16);
SoftwareSerial receivingSerial(19, 18);
SoftwareSerial gpsserial(15, 14);

int pressurePin = 0;
int temperaturePin = 1;

float lat = 27.2046,lon = 77.4977;

int HBSensor = 4;
int HBCount = 0;
int HBCheck = 0;
int TimeinSec = 0;
int HBperMin = 0;
int HBStartCheck = 0;
int number_of_iter = 0;
int Time = 0;

char PRESSURESHOW[10];
char TEMPERATURESHOW[4];
int TEMPERATURE = 27;
float PRESSURE = 101;
int HEARTRATE = 0;
const char location_statement[50];

char auth[] = BLYNK_AUTH_TOKEN;

void calculate_pressure(){
  PRESSURE = analogRead(pressurePin);
  PRESSURE = PRESSURE/8.38;
  int intPRESSUREVALUE = 0;
  intPRESSUREVALUE = PRESSURE*1.22;
  int intlowerPRESSUREVALUE = 0;
  intlowerPRESSUREVALUE = intPRESSUREVALUE*0.66;
  String strintPRESSUREVALUE = String(intPRESSUREVALUE) + "/" + String(intlowerPRESSUREVALUE);
  strintPRESSUREVALUE.toCharArray(PRESSURESHOW, 10);
}
void calculate_temperature(){
  TEMPERATURE = analogRead(temperaturePin);
  TEMPERATURE = TEMPERATURE/2;
  String strTEMPERATUREVALUE = String(TEMPERATURE);
  strTEMPERATUREVALUE.toCharArray(TEMPERATURESHOW, 10);
}

void conditionforsms(){
  if (TEMPERATURE < 30 || TEMPERATURE > 45 || PRESSURE > 164 || PRESSURE < 65 || HEARTRATE < 50 || HEARTRATE > 120){
    if (number_of_iter == 0){
      mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
      delay(1000);  // Delay of 1 second
      mySerial.println("AT+CMGS=\"+919625268535\"\r"); //mobile number to send a text to
      delay(1000);
      receivingSerial.println();
      receivingSerial.println("       SOS! EMERGENCY!");
      receivingSerial.println("......................");
      receivingSerial.println("Patient Name: Vaibhav Bansal");
      receivingSerial.println("......................");
      receivingSerial.println("Location....");
      receivingSerial.println(gps_connect());
      receivingSerial.println("......................");
      receivingSerial.println("Needed Immediate Medical Help");
      receivingSerial.println("......................");
      receivingSerial.println("Patient's Vitals...");
      receivingSerial.println("Body Temperature - " + String(TEMPERATURESHOW)+ "°C");
      receivingSerial.println("Blood Pressure - " + String(PRESSURESHOW));
      receivingSerial.println("Heart Beat - " + String(HEARTRATE));
      receivingSerial.println();
      delay(100);
      Blynk.virtualWrite(V4, "SOS! EMERGENCY!");
      Blynk.virtualWrite(V5, "Vaibhav Bansal");
      Blynk.virtualWrite(V6, location_statement);
      mySerial.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module 
      delay(1000);
      if (mySerial.available()>0){
        receivingSerial.write(mySerial.read());
      }
      number_of_iter = 1;
    }
  }
}

BLYNK_WRITE(V0){
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V1, 0);
  Blynk.virtualWrite(V4, 0);
  Blynk.virtualWrite(V5, 0);
  Blynk.virtualWrite(V6, 0);
  calculate_pressure();
  calculate_temperature();
  PulseRate();
  HEARTRATE = HBCount*14;
  Blynk.virtualWrite(V2, TEMPERATURESHOW);
  Blynk.virtualWrite(V3, PRESSURESHOW);
  Blynk.virtualWrite(V1, HEARTRATE);
  lcd.setCursor(0,0);
  lcd.print("Your Vitals!!      ");
  lcd.setCursor(0,1);
  lcd.print("HB per Min : ");
  lcd.print(HEARTRATE);
  lcd.print("      ");
  lcd.setCursor(0,2);
  lcd.print("                  ");
  lcd.setCursor(0,2);
  lcd.print("BP - ");
  for(int i=0; i<8; i++){
    lcd.write(PRESSURESHOW[i]);
  }
  lcd.setCursor(0, 3);
  lcd.print("Temperature- ");// print name
  lcd.print(TEMPERATURESHOW);
  lcd.print((char)223);
  lcd.print("C ");
  delay(500);    
  conditionforsms();
}
void PulseRate(){
  while(Time<=900){
    gpsserial.println(Time);
    Time = Time + 1;
    if (Time%85 == 0){
      TimeinSec = Time/85;
      lcd.setCursor(0,0);
      lcd.print("Wait for "+ String(10-TimeinSec)+ "     ");
    }
    if((digitalRead(HBSensor) == HIGH) && (HBCheck == 0))
    {
        HBCount = HBCount + 1;
        HBCheck = 1;
        gpsserial.println("HBCount" + String(HBCount));
    }
    if((digitalRead(HBSensor) == LOW) && (HBCheck == 1))
    {
      HBCheck = 0;   
    }
  }
}

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  gpsserial.begin(9600);
  receivingSerial.begin(9600);
  
  Blynk.begin(Serial, auth);
  
  pinMode(HBSensor, INPUT);
//  pinMode(HBStart, INPUT_PULLUP);
  
  lcd.begin(20, 4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Check Vitals - ");
  lcd.setCursor(0,1);
  lcd.print("Heart Beats per Min");
  lcd.setCursor(0,2);
  lcd.print("Blood Pressure ");
  lcd.setCursor(0,3);
  lcd.print("Body Temperature");
}

void loop()
{
  Blynk.run();
}

float gps_connect() {
  while (receivingSerial.available()) { // check for gps data
    if (gps.encode(gpsserial.read())) // encode gps data
    {
      gps.f_get_position(&lat, &lon); // get latitude and longitude
    }
  }

  String latitude = String(lat, 6);
  String longitude = String(lon, 6);
  String location = latitude + "°N , " + longitude + "°S";
  location.toCharArray(location_statement, 50);
  receivingSerial.println("Latitude: " + latitude + "," "Longitude: " + longitude);
  delay(1000);
}
