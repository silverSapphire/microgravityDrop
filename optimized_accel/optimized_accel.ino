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


void setup() 
{
       
  analogReference(EXTERNAL);

  lcd.begin(16, 2);
  
  setupSD();

  pinMode(calibrationButton, INPUT);
  runCalibrationSequence();

  lcd.clear();
  
}

void loop() 
{

  // Convert raw values to 'milli-Gs"
  // Use the calibration script to get the raw min and max values for x, y, and z.
  // WARNING: Positive z is inaccurate, 0.75 G measured = 1 G real.
  // We only accurately measure +/- 1 G.
  float xAccel = map(analogRead(xInput), xRawMin, xRawMax, -1000.0, 1000.0) / 1000.0;
  float yAccel = map(analogRead(yInput), yRawMin, yRawMax, -1000.0, 1000.0) / 1000.0;
  float zAccel = map(analogRead(zInput), zRawMin, zRawMax, -1000.0, 1000.0) / 1000.0 * 1.25;  //Scale by 1.25.

  datalog(xAccel, yAccel, zAccel);

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
      dataFile.print(millis()); 
      dataFile.print(", ");
      dataFile.print(xRaw);
      dataFile.print(", ");
      dataFile.print(yRaw);
      dataFile.print(", ");
      dataFile.println(zRaw);
            
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

void runCalibrationSequence()
{

  lcd.setCursor(0, 0);
  lcd.print("calibrate");

  lcd.setCursor(0, 1);
  lcd.print("X");

  lcd.setCursor(5, 1);
  lcd.print("Y");

  lcd.setCursor(10, 1);
  lcd.print("Z");
  
  int calibrated = 0;  // Bool for transitioning from calibration to recording.
  int pushed = 0;
  long timePushed = 0;
  
  while(!calibrated)
  {

    int buttonState = digitalRead(calibrationButton);
    
    // Take a calibration reading. ---------------------
    if(buttonState == HIGH)
    {
      autoCalibrate();
    }

    // Check for calibration end sequence. ----------------
    if(buttonState == HIGH && pushed == 0)  // Start timer.
    {
      pushed = 1;
      timePushed = millis(); 
    }
    
    if(buttonState == LOW)  // End timer.
    {
      pushed = 0;
    }

    if(pushed && (millis() - timePushed >= 5000)) // After 5 seconds pushed, exit calibration.
    {
      calibrated = 1;
    }

  }
  
}

void autoCalibrate()
{

  int xRaw = analogRead(xInput);
  int yRaw = analogRead(yInput);
  int zRaw = analogRead(zInput);
  
  // Readjust mins and maxes if necessary, according to input.
  if(xRaw < xRawMin)
  {
    xRawMin = xRaw;

    if(xRawMin <= expected_xRawMin)
    {
      lcd.setCursor(1, 1);
      lcd.print("-");
    }
  }
  if(xRaw > xRawMax)
  {
    xRawMax = xRaw;  

    if(xRawMax >= expected_xRawMax)
    {
      lcd.setCursor(2, 1);
      lcd.print("+");
    }
  }
  
  if(yRaw < yRawMin)
  {
    yRawMin = yRaw;

    if(yRawMin <= expected_yRawMin)
    {
      lcd.setCursor(6, 1);
      lcd.print("-");
    }
  }
  if(yRaw > yRawMax)
  {
    yRawMax = yRaw;

    if(yRawMax >= expected_yRawMax)
    {
      lcd.setCursor(7, 1);
      lcd.print("+");
    }
  }

  if(zRaw < zRawMin)
  {
    zRawMin = zRaw;

    if(zRawMin <= expected_zRawMin)
    {
      lcd.setCursor(11, 1);
      lcd.print("-");
    }
  }
  if(zRaw > zRawMax)
  {
    zRawMax = zRaw;

    if(zRawMax >= expected_zRawMax)
    {
      lcd.setCursor(12, 1);
      lcd.print("+");
    }
  }
  
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
    lcd.setCursor(11, 0);
    lcd.print("SDERR");
  }
  
}
