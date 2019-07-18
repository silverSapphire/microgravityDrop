#include <SD.h>
#include <LiquidCrystal.h>

// Calibration vars
const byte calibrationButton = 2;

int xRawMin = 512;  // Initialize to mid levels.
int xRawMax = 512;
int yRawMin = 512;
int yRawMax = 512;
int zRawMin = 512;
int zRawMax = 512;

int expected_xRawMin = 409; // Taken from manual calibration run.
int expected_xRawMax = 620;
int expected_yRawMin = 402;
int expected_yRawMax = 612;
int expected_zRawMin = 406;
int expected_zRawMax = 651;

// LCD vars
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
long timeLastRefreshed = 0;

// SD vars
const int chipSelect = 4;
String fileName = "";

// Accelerometer vars
const int xInput = A2;
const int yInput = A1;
const int zInput = A0;

const int sampleSize = 3;


void setup() 
{
       
  analogReference(EXTERNAL);

  lcd.begin(16, 2);
  lcd.clear();
  
  setupSD();
    
}

void loop() 
{

  // Convert raw values to 'milli-Gs"
  // Use the calibration script to get the raw min and max values for x, y, and z.
  // WARNING: Positive z is inaccurate, 0.75 G measured = 1 G real.
  // We only accurately measure +/- 1 G.
  float xAccel = map(readAxis(xInput), expected_xRawMin, expected_xRawMax, -1000.0, 1000.0) / 1000.0;
  float yAccel = map(readAxis(yInput), expected_yRawMin, expected_yRawMax, -1000.0, 1000.0) / 1000.0;
  float zAccel = map(readAxis(zInput), expected_zRawMin, expected_zRawMax, -1000.0, 1000.0) / 1000.0 * 1.25;  //Scale by 1.25.

  datalog(xAccel, yAccel, zAccel);

  // Refresh the display every 150 ms.
  if(millis() - timeLastRefreshed >= 200)
  {
    displayData(xAccel, yAccel, zAccel);
    timeLastRefreshed = millis();
  }
  
}

// Take samples and return the average
int readAxis(int axisPin)
{
  
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
  reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

void datalog(float xAccel, float yAccel, float zAccel)
{
  
     File dataFile = SD.open(fileName, FILE_WRITE);
  
    // if the file is available, write to it:
    if(dataFile) {
      
      dataFile.print(millis()); 
      dataFile.print(", ");
      dataFile.print(xAccel, 3);
      dataFile.print(", ");
      dataFile.print(yAccel, 3);
      dataFile.print(", ");
      dataFile.println(zAccel, 3);
            
      dataFile.close();
    }

    else
    {
      lcd.setCursor(13, 1);
      lcd.print("ERR");
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

void setupSD()
{
    
  if (SD.begin(chipSelect)) 
  {
    
    int fileNumber = 1;
    int foundFileNumber = 0;
    
    fileName = "data" + (String)fileNumber + ".csv";

    // Find the next name in the file sequence.
    while(!foundFileNumber)
    {
      
      if(SD.exists(fileName)){
        fileNumber += 1;
        fileName = "data" + (String)fileNumber + ".csv";
      }
      else {
        foundFileNumber = 1;
      }
      
    }

    lcd.setCursor(11, 0);
    lcd.print("SD OK");
  }
  else
  {
    lcd.setCursor(13, 1);
    lcd.print("ERR");
  }
  
}
