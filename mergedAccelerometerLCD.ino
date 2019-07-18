#include <SD.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte in = 2;
const int chipSelect = 4;   //designates CS for datalogger
byte filerr = 0;

const int xInput = A2;
const int yInput = A1;
const int zInput = A0;

// initialize minimum and maximum Raw Ranges for each axis
float RawMin = 0.0;
float RawMax = 1023.0;

// Take multiple samples to reduce noise
const int sampleSize = 10;

// File naming vars.
String fileName = "";

void setup() 
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
   
  analogReference(EXTERNAL);
  Serial.begin(9600);

  pinMode(in, INPUT);
  if (!SD.begin(chipSelect)) {
    
    // don't do anything more:
    return;
  }

  // Get the name of the next file.
  int fileNumber = 1;
  int foundFileNumber = 0;
  
  fileName = "data" + (String)fileNumber + ".txt";
  
  while(!foundFileNumber)
  {
    if(SD.exists(fileName)){
      fileNumber += 1;
      fileName = "data" + (String)fileNumber + ".txt";
    }
    else {
      foundFileNumber = 1;
    }
  }
  
}

void loop() 
{
  //Read raw values
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);

  // Convert raw values to 'milli-Gs"
  // Use the calibration script to get the raw min and max values for x, y, and z.
  // WARNING: Positive z is inaccurate, 0.75 G measured = 1 G real.
  // We only accurately measure +/- 1 G. [see below for scaling]
  float xScaled = float_map((float)xRaw, 409, 620, -1000.0, 1000.0);
  float yScaled = float_map((float)yRaw, 402, 612, -1000.0, 1000.0);
  float zScaled = float_map((float)zRaw, 406, 651, -1000.0, 1000.0);

  // re-scale to fractional Gs
  float xAccel = xScaled / 1000.0;
  float yAccel = yScaled / 1000.0;
  float zAccel = zScaled / 1000.0;
  
  // Scale Z by a factor of 1.25 to make it accurate.
  zAccel *= 1.25;

  Serial.print("X, Y, Z  :: ");
  Serial.print(xRaw);
  Serial.print(", ");
  Serial.print(yRaw);
  Serial.print(", ");
  Serial.print(zRaw);
  Serial.print(" :: ");
  Serial.print(xAccel,3);
  Serial.print("G, ");
  Serial.print(yAccel,3);
  Serial.print("G, ");
  Serial.print(zAccel,3);
  Serial.println("G");

  datalog(xRaw, yRaw, zRaw);

  // Displaying Raw Values on LCD
  // set the cursor for x value. column 0, line 0
  lcd.setCursor(0, 0);
  lcd.print("X: ");
  lcd.print(xRaw);
  // set the cursor for y value. column 0, line 1
  lcd.setCursor(0, 1);
  lcd.print("Y: ");
  lcd.print(yRaw); 
  // set the cursor for z value. column 9, line 0
  lcd.setCursor(9, 0);
  lcd.print("Z: ");
  lcd.print(zRaw);
  
  delay(200);
}

// Take samples and return the average
int ReadAxis(int axisPin)
{
  long reading = 0;
//  analogRead(axisPin);
//  delay(1);
//  for (int i = 0; i < sampleSize; i++)
//  {
//  reading += analogRead(axisPin);
//  }
//  return reading/sampleSize;
   return analogRead(axisPin);
}

// Need to implement own map to get decimals.
float float_map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void datalog(int xRaw, int yRaw, int zRaw){
     File dataFile = SD.open(fileName, FILE_WRITE);  //open file
  
    // if the file is available, write to it:
    if(dataFile) {
      dataFile.print(xRaw);
      dataFile.print(", ");
      dataFile.print(yRaw);
      dataFile.print(", ");
      dataFile.print(zRaw);
      dataFile.print(", ");
      dataFile.println(millis());
            
      dataFile.close();
    }
    
  // if the file isn't open, pop up an error:
  else {
    filerr = 1;
  }
}
