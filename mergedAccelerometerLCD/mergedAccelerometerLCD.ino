#include <SD.h>
#include <LiquidCrystal.h>

// LCD vars
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
long timeLastRefreshed = 0;

// SD vars
const byte in = 2;
const int chipSelect = 4;

// Accelerometer vars
const int xInput = A2;
const int yInput = A1;
const int zInput = A0;

// File naming vars.
String fileName = "";


void setup() 
{
  
  // Set up the LCD's number of columns and rows: ------------
  lcd.begin(16, 2);
   
  analogReference(EXTERNAL);
  Serial.begin(9600);

  pinMode(in, INPUT);
  if (!SD.begin(chipSelect)) {
        return;
  }

  // Get the name of the next file. ------------------
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
  int xRaw = analogRead(xInput);
  int yRaw = analogRead(yInput);
  int zRaw = analogRead(zInput);

  // Convert raw values to 'milli-Gs"
  // Use the calibration script to get the raw min and max values for x, y, and z.
  // WARNING: Positive z is inaccurate, 0.75 G measured = 1 G real.
  // We only accurately measure +/- 1 G. [see below for scaling]
  float xScaled = map(xRaw, 409, 620, -1000.0, 1000.0);
  float yScaled = map(yRaw, 402, 612, -1000.0, 1000.0);
  float zScaled = map(zRaw, 406, 651, -1000.0, 1000.0);

  // re-scale to fractional Gs
  float xAccel = xScaled / 1000.0;
  float yAccel = yScaled / 1000.0;
  float zAccel = zScaled / 1000.0;
  
  // Scale Z by a factor to make it accurate.
  zAccel *= 1.25;

  Serial.print("X, Y, Z  :: ");
  Serial.print(xRaw);
  Serial.print(", ");
  Serial.print(yRaw);
  Serial.print(", ");
  Serial.print(zRaw);
  Serial.print(" :: ");
  Serial.print(xAccel,4);
  Serial.print("G, ");
  Serial.print(yAccel,4);
  Serial.print("G, ");
  Serial.print(zAccel,4);
  Serial.println("G");

  datalog(xRaw, yRaw, zRaw);

  // Refresh the display every 150 ms.
  if(millis() - timeLastRefreshed >= 150)
  {
    displayData(xAccel, yAccel, zAccel);
    timeLastRefreshed = millis();
  }
  
}

void datalog(int xRaw, int yRaw, int zRaw)
{
  
     File dataFile = SD.open(fileName, FILE_WRITE);
  
    // if the file is available, write to it:
    if(dataFile) {
      dataFile.println(millis());
      dataFile.print(xRaw);
      dataFile.print(", ");
      dataFile.print(yRaw);
      dataFile.print(", ");
      dataFile.print(zRaw);
      dataFile.print(", ");
            
      dataFile.close();
    }
    
}

void displayData(float xAccel, float yAccel, float zAccel)
{

  // Displaying G Values on LCD
  // set the cursor for x value. column 0, line 0
  lcd.setCursor(0, 0);
  lcd.print("X ");
  lcd.print(xAccel, 3);
  // set the cursor for y value. column 0, line 1
  lcd.setCursor(0, 1);
  lcd.print("Y ");
  lcd.print(yAccel, 3); 
  // set the cursor for z value. column 9, line 0
  lcd.setCursor(9, 0);
  lcd.print("Z ");
  lcd.print(zAccel, 3);
  
}
