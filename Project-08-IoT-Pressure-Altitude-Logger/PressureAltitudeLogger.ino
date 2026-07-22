#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_BMP085 bmp;

// ---------------- Pins ----------------

#define POT_PIN 34

#define GREEN_LED 25
#define RED_LED 26

// ---------------- Variables ----------------

float pressure;
float temperature;
float altitude;
float seaLevelPressure;

float pressureLog[24];

int logIndex = 0;

unsigned long lastLog = 0;
unsigned long lastPage = 0;

bool page = 0;

String trend = "STABLE";

// ---------------- Function Prototypes ----------------

void readSensors();
void logPressure();
void calculateTrend();
void displayPage1();
void displayPage2();
void printSerial();

// ---------------- Setup ----------------

void setup()
{

Serial.begin(115200);

pinMode(GREEN_LED,OUTPUT);
pinMode(RED_LED,OUTPUT);

digitalWrite(GREEN_LED,LOW);
digitalWrite(RED_LED,LOW);

Wire.begin();

if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C))
{
Serial.println("OLED Failed");
while(1);
}

if(!bmp.begin())
{
Serial.println("BMP180 Failed");
while(1);
}

display.clearDisplay();

display.setTextColor(SSD1306_WHITE);

display.setTextSize(1);

display.setCursor(10,25);

display.println("BMP180 LOGGER");

display.display();

delay(2000);

// Initialize pressure array

for(int i=0;i<24;i++)
{
pressureLog[i]=0;
}

}
// ---------------- Loop ----------------

void loop()
{

readSensors();

if(millis()-lastLog>30000)
{
lastLog=millis();

logPressure();

calculateTrend();

printSerial();
}

if(millis()-lastPage>5000)
{
lastPage=millis();

page=!page;
}

if(page)
displayPage1();
else
displayPage2();

delay(100);

}

// ---------------- Read Sensors ----------------

void readSensors()
{

temperature = bmp.readTemperature();

pressure = bmp.readPressure();

int potValue = analogRead(POT_PIN);

float localAltitude = map(potValue,0,4095,0,2000);

altitude = localAltitude;
seaLevelPressure =
pressure /
pow((1.0-(localAltitude/44330.0)),5.255);

}

// ---------------- Pressure Log ----------------

void logPressure()
{

pressureLog[logIndex]=pressure;

logIndex++;

if(logIndex>=24)
logIndex=0;

}
// ---------------- Trend Calculation ----------------

void calculateTrend()
{

float oldest = pressureLog[logIndex];

float newest;

if(logIndex==0)
newest=pressureLog[23];
else
newest=pressureLog[logIndex-1];

float diff = newest-oldest;

if(diff>50)
{

trend="RISING";

digitalWrite(GREEN_LED,HIGH);
digitalWrite(RED_LED,LOW);

}

else if(diff<-50)
{

trend="FALLING";

digitalWrite(GREEN_LED,LOW);
digitalWrite(RED_LED,HIGH);

}

else
{

trend="STABLE";

digitalWrite(GREEN_LED,HIGH);
digitalWrite(RED_LED,LOW);

}

}

// ---------------- Serial Monitor ----------------

void printSerial()
{

Serial.println("==========================================");

Serial.print("Temperature : ");
Serial.print(temperature);
Serial.println(" C");

Serial.print("Pressure    : ");
Serial.print(pressure);
Serial.println(" Pa");

Serial.print("Altitude    : ");
Serial.print(altitude);
Serial.println(" m");

Serial.print("Sea Level P : ");
Serial.print(seaLevelPressure);
Serial.println(" Pa");

Serial.print("Trend       : ");
Serial.println(trend);
weatherPrediction();

Serial.println("==========================================");

}
// ---------------- OLED Page 1 ----------------

void displayPage1()
{

display.clearDisplay();

display.setCursor(0,0);

display.setTextSize(1);

display.println("BMP180 LOGGER");

display.println("----------------");

display.print("Temp : ");
display.print(temperature);
display.println(" C");

display.print("Press: ");
display.print(pressure);
display.println(" Pa");

display.print("Alt  : ");
display.print(altitude);
display.println(" m");

display.display();

}

// ---------------- OLED Page 2 ----------------

void displayPage2()
{

display.clearDisplay();

display.setCursor(0,0);

display.setTextSize(1);

display.println("PRESSURE TREND");

display.println("----------------");

display.print("Sea Level:");

display.println(seaLevelPressure);

display.print("Trend : ");

display.println(trend);

display.print("Arrow : ");

if(trend=="RISING")
{
display.println("^");
}
else if(trend=="FALLING")
{
display.println("v");
}
else
{
display.println("->");
}

display.display();

}
// ================= FINAL PART =================

// Optional: Print last 24 pressure readings
void printLog()
{
  Serial.println("------ Pressure Log ------");

  for (int i = 0; i < 24; i++)
  {
    Serial.print(i + 1);
    Serial.print(" : ");
    Serial.print(pressureLog[i]);
    Serial.println(" Pa");
  }

  Serial.println("--------------------------");
}

// Reset pressure log
void resetLog()
{
  for (int i = 0; i < 24; i++)
  {
    pressureLog[i] = pressure;
  }

  logIndex = 0;
}

// Future expansion function
void weatherPrediction()
{
  if (trend == "RISING")
  {
    Serial.println("Prediction : Good Weather");
  }
  else if (trend == "FALLING")
  {
    Serial.println("Prediction : Rain Likely");
  }
  else
  {
    Serial.println("Prediction : Stable Weather");
  }
}
