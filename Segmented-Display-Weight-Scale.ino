#include <HX711.h>
#include <Wire.h>

static const byte NUM_MATRIX[10][4] = { 
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

static const int PINS_DRIVER1[4] = { 2, 3, 4, 5 };
static const int PINS_DRIVER2[4] = { 6, 9, 8, 7 };// B & D flipped
static const int PINS_DISPLAY_SELECT1[DISPLAYS] = { 10, 11, 12 };
static const int PINS_DISPLAY_SELECT2[DISPLAYS] = { 13, 14, 15 };
static const int PIN_HX711_CLK = 16;
static const int PIN_HX711_DAT = 17;

long previousMillis = 0;

int activeDisplay = 1;
int displayBufDrv1[DISPLAYS] = { -1, -1, -1 };
int displayBufDrv2[DISPLAYS] = { -1, -1, -1 };

HX711 scale;
float calibration_factor = -5280;
float scaleValue;
long zero_factor;
boolean calibrationMode = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Multiplexed Display Scale Firmware Version 1.0");
  printCopyright();
  Serial.println("");
  Serial.println("Initializing...");
  Serial.println("");
  Serial.print("Calibration factor set to ");
  Serial.println(calibration_factor);
  
  pinMode(PINS_DRIVER1[0], OUTPUT);
  pinMode(PINS_DRIVER1[1], OUTPUT);
  pinMode(PINS_DRIVER1[2], OUTPUT);
  pinMode(PINS_DRIVER1[3], OUTPUT);
  
  pinMode(PINS_DRIVER2[0], OUTPUT);
  pinMode(PINS_DRIVER2[1], OUTPUT);
  pinMode(PINS_DRIVER2[2], OUTPUT);
  pinMode(PINS_DRIVER2[3], OUTPUT);
  
  pinMode(PINS_DISPLAY_SELECT1[0], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT1[1], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT1[2], OUTPUT);
  
  pinMode(PINS_DISPLAY_SELECT2[0], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT2[1], OUTPUT);
  pinMode(PINS_DISPLAY_SELECT2[2], OUTPUT);
  Serial.print("IO pin modes set.");
  
  setUnitLbs();
  startScale();
}

void printCopyright() {
  Serial.println("Copyright (C) 2019 ASX Electronics");
  Serial.println("Designed and built by Dustin Christensen");
}

void startScale() {
  scale.begin(PIN_HX711_DAT, PIN_HX711_CLK);
  scale.set_scale();
  scale.tare();
  zero_factor = scale.read_average();
  Serial.print("Zero factor set to ");
  Serial.println(zero_factor);
  delay(500);
  Serial.println("Scale initialized.");
}

void printCalibrationFactor() {
  Serial.print("Calibration set to: ");
  Serial.println(calibration_factor);
}

void loop() {
  long currentMillis = millis();
  
  if(currentMillis - previousMillis > 100) {
    previousMillis = currentMillis;
    
    scale.set_scale(calibration_factor);
    scaleValue = scale.get_units() + 0.07F;
  }
  
  if (calibrationMode) {
    if(Serial.available()) {
      char temp = Serial.read();
      
      if(temp == '+') {
        calibration_factor += 1;
        printCalibrationFactor();
      } else if(temp == '-') {
        calibration_factor -= 1;
        printCalibrationFactor();
      } else if(temp == '.') {
        calibration_factor += 10;
        printCalibrationFactor();
      } else if(temp == ',') {
        calibration_factor -= 10;
        printCalibrationFactor();
      } else if(temp == 'x') {
        calibrationMode = false;
        Serial.println("Calibration mode disabled.");
      }
    }
  }
  
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil(' ');
    Serial.read();
    
    if (command == "calibrate") {
      calibrationMode = true;
      Serial.println("Entered calibration mode.");
      Serial.println("Use '+' or '-' to change the calibration by increments of 1.");
      Serial.println("Use '.' or ',' to change the calibration by increments of 10.");
    }
    if (command == "copyright") {
      printCopyright();
    }
    if (command == "help") {
      Serial.println("calibrate, copyright");
    }
  }

  updateWeight();

  clearDriver(PINS_DRIVER1);
  clearDriver(PINS_DRIVER2);
  processDriver(PINS_DISPLAY_SELECT1, displayBufDrv1, PINS_DRIVER1);
  processDriver(PINS_DISPLAY_SELECT2, displayBufDrv2, PINS_DRIVER2);
  activeDisplay++;
    
  if (activeDisplay > DISPLAYS)
  {
    activeDisplay = 1;
  }
  
  delay(4);
}

void updateWeight() {
  int weight = round(scaleValue);
  int d1, d2, d3, d4 = 0;
  
  if (weight > 0) {
    d1 = (weight / 1000) % 10;
    d2 = (weight / 100) % 10;
    d3 = (weight / 10) % 10;
    d4 = weight % 10;
  }

  if (d1 == 0) d1 = -1;
  if (d2 == 0) d2 = -1;
  if (d3 == 0) d3 = -1;

  if (weight < 0) {
    d1 = d2 = d3 = d4 = 0;
  }
  
  setWeightBuffer(d1, d2, d3, d4);
}

void setWeightBuffer(int d1, int d2, int d3, int d4) {
  displayBufDrv1[0] = d1;
  displayBufDrv1[1] = d2;
  displayBufDrv1[2] = d3;
  displayBufDrv2[0] = d4;
}

void setUnitLbs() {
  displayBufDrv2[1] = 1;
  displayBufDrv2[2] = 6;
}

void processDriver(int selectPins[], int displayBuffer[], int driverPins) {
  for (int displayId = 0; displayId < DISPLAYS; displayId++) {
    if (activeDisplay % DISPLAYS == displayId) {
      digitalWrite(selectPins[displayId], HIGH);

      int number = displayBuffer[displayId];
  
      if (number != -1) {
        writeIntToDisplayDriver(number, driverPins);
      } else {
        digitalWrite(selectPins[displayId], LOW);
      }
    } else {
      digitalWrite(selectPins[displayId], LOW);
    }
  }
}

void writeIntToDisplayDriver(int num, int driverPins[]) {
  int p = 3;
  
  for (int i = 0; i < 4; i++) {
    int n = NUM_MATRIX[num][i];
    digitalWrite(driverPins[p], n);
    p--;
  }
}

void clearDriver(int driverPins[]) {
  int p = 3;
  
  for (int i = 0; i < 4; i++) {
    digitalWrite(driverPins[p], HIGH);
    p--;
  }
}
