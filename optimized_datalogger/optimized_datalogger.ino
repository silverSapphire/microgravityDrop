#include <SD.h>

const byte in = 2;
const int chipSelect = 4;   //designates CS for datalogger

const int xInput = A2;
const int yInput = A1;
const int zInput = A0;

// File naming vars.
String fileName = "";

void setup() 
{
  
  analogReference(EXTERNAL);
  Serial.begin(9600);

  pinMode(in, INPUT);
  if (!SD.begin(chipSelect)) {
        return;
  }

  // Get the name of the next file, e.g., data1,txt, data2.txt -> data3.txt
  int fileNumber = 1;
  byte foundFileNumber = 0; //Bool
  
  fileName = "data" + (String)fileNumber + ".txt";
  
  while(!foundFileNumber)
  {
    if(SD.exists(fileName)){
      fileNumber += 1;
      fileName = "data" + (String)fileNumber + ".txt";
    }
    else {
      foundFileNumber = 1;  //Found our next filename, exit loop
    }
  }
  
}

void loop() 
{
  
  File dataFile = SD.open(fileName, FILE_WRITE);
  
  // if the file is available, write to it:
  if(dataFile) {
    dataFile.print(millis());
    dataFile.print(", ");
    dataFile.print(analogRead(xInput));
    dataFile.print(", ");
    dataFile.print(analogRead(yInput));
    dataFile.print(", ");
    dataFile.println(analogRead(zInput));
            
    dataFile.close();
  }
}
