#include <HX711.h>
#include <Wire.h>

static int numMatrix[10][4] = {
                                { 0,0,0,0 }, //0
                                { 0,0,0,1 }, //1
                                { 0,0,1,0 }, //2
                                { 0,0,1,1 }, //3
                                { 0,1,0,0 }, //4
                                { 0,1,0,1 }, //5
                                { 0,1,1,0 }, //6
                                { 0,1,1,1 }, //7
                                { 1,0,0,0 }, //8
                                { 1,0,0,1 }  //9
                               };

static const int DISPLAYS = 3;

static const int PINS_DISPLAY_DRIVER1[4] = { 2, 3, 4, 5 };
static const int PINS_DISPLAY_DRIVER2[4] = { 6, 9, 8, 7 };// B & D flipped

static const int PINS_DISPLAY_SELECT1[DISPLAYS] = { 10, 11, 12 };
static const int PINS_DISPLAY_SELECT2[DISPLAYS] = { 13, 14, 15 };

static const int PIN_HX711_CLK = 16;
static const int PIN_HX711_DAT = 17;

long previousMillis = 0;

int activeDisplay = 1;
int displayBufferDriver1[DISPLAYS] = { 0, 0, 0 };
int displayBufferDriver2[DISPLAYS] = { 0, 0, 0 };

int count = 0;
int weight = 0;

boolean flip = false;
boolean clearDisplays = false;

void setup()
{
  Serial.begin(9600);
  
  pinMode(PINS_DISPLAY_DRIVER1[0], OUTPUT);
  pinMode(PINS_DISPLAY_DRIVER1[1], OUTPUT);
  pinMode(PINS_DISPLAY_DRIVER1[2], OUTPUT);
  pinMode(PINS_DISPLAY_DRIVER1[3], OUTPUT);
  
  pinMode(PINS_DISPLAY_DRIVER2[0], OUTPUT);
  pinMode(PINS_DISPLAY_DRIVER2[1], OUTPUT);
  pinMode(PINS_DISPLAY_DRIVER2[2], OUTPUT);
  pinMode(PINS_DISPLAY_DRIVER2[3], OUTPUT);
  
  pinMode(PINS_DISPLAY_SELECT1[0], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT1[1], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT1[2], OUTPUT);
  
  pinMode(PINS_DISPLAY_SELECT2[0], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT2[1], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT2[2], OUTPUT);

  setUnitLbs();
}

void loop()
{
  long currentMillis = millis();
  
  if(currentMillis - previousMillis > 100)
  {
    previousMillis = currentMillis;
    count++;
  
    if (count > 9999)
    {
      count = 0;
    }

    weight = count;
  }

  updateWeight();
  
  delay(4);

  processDriver(PINS_DISPLAY_SELECT1, displayBufferDriver1, PINS_DISPLAY_DRIVER1);
  processDriver(PINS_DISPLAY_SELECT2, displayBufferDriver2, PINS_DISPLAY_DRIVER2);
  
  activeDisplay++;
  
  if (activeDisplay > DISPLAYS)
  {
    activeDisplay = 1;
  }
}

void updateWeight()
{
  int d1 = (weight / 1000) % 10;
  int d2 = (weight / 100) % 10;
  int d3 = (weight / 10) % 10;
  int d4 = weight % 10;
  setWeightBuffer(d1, d2, d3, d4);
}

void setWeightBuffer(int d1, int d2, int d3, int d4)
{
  displayBufferDriver1[0] = d1;
  displayBufferDriver1[1] = d2;
  displayBufferDriver1[2] = d3;
  displayBufferDriver2[0] = d4;
}

void setUnitLbs()
{
  displayBufferDriver2[1] = 1;
  displayBufferDriver2[2] = 6;
}

void clearDriver(int selectPins[])
{
  for (int displayId = 0; displayId < DISPLAYS; displayId++)
  {
    digitalWrite(selectPins[displayId], LOW);
  }
}

void processDriver(int selectPins[], int displayBuffer[], int driverPins)
{
  for (int displayId = 0; displayId < DISPLAYS; displayId++)
  {
    if (activeDisplay % DISPLAYS == displayId)
    {
      digitalWrite(selectPins[displayId], HIGH);

      int number = displayBuffer[displayId];
  
      if (number != -1)
      {
        writeIntToDisplayDriver(number, driverPins);
      }
      else
      {
        digitalWrite(selectPins[displayId], LOW);
      }
    }
    else
    {
      digitalWrite(selectPins[displayId], LOW);
    }
  }
}

void writeIntToDisplayDriver(int num, int driverPins[])
{
  int p = 3;
  
  for (int i = 0; i < 4; i++)
  {
    int n = numMatrix[num][i];
    digitalWrite(driverPins[p], n);
    p--;
  }
}
