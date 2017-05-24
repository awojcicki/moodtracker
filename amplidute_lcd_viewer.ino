
#include <Wire.h>   
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

const unsigned long SERVER_SYNC_INTERVAL = 30 * 1000UL;
const int sampleWindow = 50; 
const int maxScale = 8;
unsigned int sample;
void setup()
{
  Serial.begin(9600);
  pinMode(A0, INPUT);
  lcd.begin(16, 2); 

  lcd.backlight(); 
  lcd.setCursor(0, 0);
}

static unsigned long lastSampleTime = 0;

unsigned int max = 0;
unsigned int min = 2000;
int previousLevel = 0;

unsigned int amplitudeSum = 0;
unsigned int amplitudeSamples = 0;
void loop()
{

  unsigned long startMillis = millis();
  unsigned int peakToPeak = 0;

  unsigned int signalMax = 0;
  unsigned int signalMin = 3024;

  // collect data for 50 mS
  amplitudeSamples++;
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(0);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  amplitudeSum += peakToPeak;

  lcd.setCursor(0, 0);
  
  unsigned int sound = peakToPeak;
  int level = peakToPeak * 16 / 400;
  lcd.setCursor(0, 0);

   if (amplitudeSamples % 10 == 0) {
    double average = amplitudeSum / (double)amplitudeSamples;
    lcd.print(average);
    lcd.print(" ");
    lcd.print(amplitudeSamples);
    lcd.print("      ");
  }
  lcd.setCursor(0, 1);
  printLevel(level);
  previousLevel = level;
  
  unsigned long now = millis();
  
  if (now - lastSampleTime >= SERVER_SYNC_INTERVAL)
  { 
    lastSampleTime = now;
    amplitudeSum = 0;
    amplitudeSamples = 0;
  }
  

}

void printLevel(int level) {
    for (int i = 0; i< 16; i++) {
    if (i<level)
      lcd.print((char)B11111111); 
   else 
    lcd.print(" "); 
  }
}


