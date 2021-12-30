#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MQUnifiedsensor.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//  Pinout Definitions
#define SENSOR_LDR            A0   // Analog Pin 2
#define SENSOR_MOISTURE       A1   // Analog Pin 2
#define SENSOR_MQ135          A2   // Analog Pin 2
#define SENSOR_DHT11          2   // Digital Pin 2
#define SIM900_TX             7
#define SIM900_RX             8

//  MQ135 Library Definitions
#define VOLTAGE_RESOLUTION    5
#define ADC_BIT_RESOLUTION    10
#define RATIO_MQ135           3.6
#define TYPE                  "MQ-135"
#define BOARD                 "Arduino Pro Mini"

//  DHT11 Library Definitions
#define DHTTYPE               DHT11     

//  Device Specific 
#define RECEIVER_NUMBER       "+639085401095"
#define DEVICE_NUMBER         "+639192860545"
#define SEND_SMS_INTERVAL     5000
#define NUM_READINGS          20

MQUnifiedsensor MQ135(BOARD, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, SENSOR_MQ135, TYPE);
SoftwareSerial SIM900(SIM900_TX, SIM900_RX);
DHT dht(SENSOR_DHT11, DHTTYPE);

int temp, hum, light, moist, co2, moist_ave;
int n_reading = 1;
long previousTime = 0;

void printData(){
  Serial.println();
  Serial.println("-----------------");
  Serial.print("Reading  #");
  Serial.println(n_reading);
  Serial.println("-----------------");
  
  Serial.print("Soil Moisture: ");
  Serial.print(moist);
  Serial.println('%');

  Serial.print("Light: ");
  Serial.print(light);
  Serial.println('%');

  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" PPM");

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println('%');
}

void sendSMS(){
  SIM900.println("AT+CMGF=1\r");
  delay(100);
  SIM900.print("AT+CMGS=\"");
  SIM900.print(RECEIVER_NUMBER);
  SIM900.println("\"");
  delay(100);

  SIM900.print("The device has collected new set of readings.\n");
  SIM900.print("Reading #");
  SIM900.print(n_reading);
  SIM900.print("\nSoil Moisture: ");
  SIM900.print(moist);
  SIM900.print("%\nLight: ");
  SIM900.print(light);
  SIM900.print("%\nCO2: ");
  SIM900.print(co2);
  SIM900.print(" PPM\nTemperature: ");
  SIM900.print(temp);
  SIM900.print("°C\nHumidity: ");
  SIM900.print(hum);
  SIM900.print("%\nHumidity: ");

  SIM900.print("\n\n - PlantMo");
  delay(100);
  SIM900.write((char)26);
  SIM900.println();
  delay(1500);
}

boolean isTime(long interval){
   unsigned long now = millis();
   if(now - (unsigned)previousTime > (unsigned)interval){
      previousTime = now;
      return 1; 
   }

   else{
      return 0;
   }
}

void setup() {
  Serial.begin(9600);
  SIM900.begin(9600);
  dht.begin();

  MQ135.setRegressionMethod(1); 
  MQ135.setA(110.47); MQ135.setB(-2.862);
  MQ135.init(); 

  Serial.print("Calibrating sensors please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); 
    calcR0 += MQ135.calibrate(RATIO_MQ135);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!");
  SIM900.println("AT\r");
}

void loop() {
  MQ135.update(); 

  for(uint8_t j = 0; j < NUM_READINGS; j++){
    moist_ave += analogRead(SENSOR_MOISTURE);
  }

  moist_ave /= NUM_READINGS;

  co2 = (int)MQ135.readSensor();
  temp = (int)dht.readTemperature();
  hum = (int)dht.readHumidity();
  light = map(analogRead(SENSOR_LDR), 0, 1023, 0, 100);
  moist = map(moist_ave, 0, 1023, 0, 100);

  printData();

  if(isTime(SEND_SMS_INTERVAL)){
    sendSMS();
    Serial.println();
    Serial.println("-----------------");
    Serial.println("SMS SENT");
    Serial.println("-----------------");
    Serial.println();
  }

  n_reading += 1;
  delay(1000);
}