const int ButtonUP = 6;
const int ButtonDOWN = 7;
const int ButtonGreen = 49;
const int ButtonRed = 9;

const int Relay1 = 10;
const int Relay2 = 11;
const int Relay3 = 41;
const int Relay4 = 48;

int RPWM_Output = 12; 
int LPWM_Output = 13; 

bool pushed_up = true;
bool pushed_down = true;
bool pushed_green = true;
bool pushed_red = true;

#include <SD.h>
#include "HX711.h"
#define DOUT  4
#define CLK  5

HX711 scale;

float calibration_factor = -9970; 

#define pinA 2  // Interrupt pin 0 (INT0)
#define pinB 3 // Interrupt pin 1 (INT1)
#define pinR 8  // Digital pin for reference pulse

#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 4); // Default address of most PCF8574 modules, change according

volatile long position = 0;  // Track the position
volatile bool direction = true; // True for forward, false for backward
volatile bool referenceHit = false; // To detect the reference pulse

// Store the previous state of A and B
volatile bool lastA = LOW;
volatile bool lastB = LOW;

// Conversion factor based on the given range
const float conversionFactor = 250.0 / 27500.0;

float max_position = 0.0;
int max_weight = 0;

const int chipSelect = 53;
File myFile;

//double logArray[2][200];
char filename[] = "LOG-001.csv";
int logStartTime = 0;
bool isLogging = false;
int motorSpeed = 10;

void setup (void) {
  //Serial.begin(9600);  // Used when debugging

  pinMode (ButtonUP, INPUT_PULLUP);
  pinMode (ButtonDOWN, INPUT_PULLUP);
  pinMode (ButtonGreen, INPUT_PULLUP);
  pinMode (ButtonRed, INPUT_PULLUP);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);

  pinMode(RPWM_Output, OUTPUT);
  pinMode(LPWM_Output, OUTPUT);

  digitalWrite(Relay1, LOW);
  digitalWrite(Relay2, LOW);
  digitalWrite(Relay3, LOW);
  digitalWrite(Relay4, LOW);

  lcd.begin(); // If you are using more I2C devices using the Wire library use lcd.begin(false)
  lcd.backlight();

  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinR, INPUT_PULLUP); // Assuming active low reference signal

  lastA = digitalRead(pinA);
  lastB = digitalRead(pinB);

  attachInterrupt(digitalPinToInterrupt(pinA), readQuadrature, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), readQuadrature, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinR), referencePulse, FALLING);

  scale.begin(DOUT, CLK);
  scale.tare(); //Reset the scale to 0
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
}

void loop (void) {
  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SD Card Not Detected");
    delay(3000);
    lcd.clear();
  } else {
    // Convert the position to millimeters
    float positionMM = position * conversionFactor;

    bool prev_up = pushed_up;
    bool prev_down = pushed_down;
    bool prev_green = pushed_green;
    bool prev_red = pushed_red;

    pushed_up = digitalRead (ButtonUP) == LOW;
    pushed_down = digitalRead (ButtonDOWN) == LOW;
    pushed_green = digitalRead (ButtonGreen) == LOW;
    pushed_red = digitalRead (ButtonRed) == LOW;

    // Print Distance on LCD
    lcd.setCursor(0, 0);
    lcd.print("Distance:");
    lcd.setCursor(10, 0);
    lcd.print(positionMM);
    lcd.print(" mm");

    // Print Weight on LCD
    lcd.setCursor(0, 1);
    lcd.print("Weight:");
    lcd.setCursor(8, 1);
    int currentWeight = (scale.get_units() / 0.00220462);

    // Disable negative weights (Makes output hard to read)
    if(currentWeight < 0) {
      lcd.print("000000");
    } else {
      if (currentWeight<10000) lcd.print('0');
      if (currentWeight<10000) lcd.print('0');
      if (currentWeight<1000) lcd.print('0');
      if (currentWeight<100) lcd.print('0');
      if (currentWeight<10) lcd.print('0');
      lcd.print(currentWeight);
    }
    lcd.setCursor(15, 1);
    lcd.print("grams");

    // Up button pushed
    if (prev_up != pushed_up) {
      if (pushed_up && pushed_down) {
        lcd.clear();
        double startTime = millis();
        lcd.setCursor(0,0);
        lcd.print("Motor Speed: "+String(motorSpeed)+"%");
        analogWrite(LPWM_Output, 0);
        analogWrite(RPWM_Output, 0);

        digitalWrite(Relay1, HIGH);
        digitalWrite(Relay2, HIGH);
        delay(750);
        while (startTime+3000 > millis()) {
          if (digitalRead (ButtonUP) == LOW) {
            if(motorSpeed<100) {
              motorSpeed += 5;
              delay(300);
              startTime = millis();
              lcd.clear();
            }
          }
          if (digitalRead (ButtonDOWN) == LOW) {
            if(motorSpeed>10) {
              motorSpeed -= 5;
              delay(300);
              startTime = millis();
              lcd.clear();
            }
          }
          lcd.setCursor(0,0);
          lcd.print("Motor Speed: "+String(motorSpeed)+"%");
        }
        lcd.clear();
        digitalWrite(Relay1, LOW);
        digitalWrite(Relay2, LOW);
      }
    
      if (pushed_up && !pushed_down) { // Prevents multiple button pushed
        digitalWrite(Relay1, HIGH);
        analogWrite(RPWM_Output, 0);
        analogWrite(LPWM_Output, (motorSpeed*2.55));
      }
      else {
        digitalWrite(Relay1, LOW);
        analogWrite(LPWM_Output, 0);
      }
    }

    // Down button pushed
    if (prev_down != pushed_down) {
      if (pushed_down && !pushed_up) {  // Prevents multiple button pushed 
          digitalWrite(Relay2, HIGH);
          analogWrite(LPWM_Output, 0);
          analogWrite(RPWM_Output, (motorSpeed*2.55));
      }
      else {
          digitalWrite(Relay2, LOW);
          analogWrite(RPWM_Output, 0);
      }
    }

    // Green button pushed
    if (prev_green != pushed_green) {
      if (pushed_green) {
        digitalWrite(Relay4, LOW); // turn off Red light
        digitalWrite(Relay3, HIGH);// turn on Green light
        
        newFile();
        isLogging = true;
        scale.tare();
        max_position = 0.0;
        max_weight = 0;
        position = 0;
        positionMM = 0;

        digitalWrite(Relay2, HIGH);// Turn on Motor Down light
        analogWrite(LPWM_Output, 0);
        analogWrite(RPWM_Output, (motorSpeed*2.55));
      }
    }

    // Red button pushed
    if (prev_red != pushed_red) {
      if (pushed_red) {
        if(isLogging) {
          digitalWrite(Relay2, LOW); // turn motor light off
          analogWrite(LPWM_Output, 0);
          analogWrite(RPWM_Output, 0);
          endLog();
        }
        position = 0;
        scale.tare();
        digitalWrite(Relay4, HIGH); // Turn on Red Light
        delay(1000);
        digitalWrite(Relay4, LOW); // Turn off Red light
      }
    }

    if (currentWeight >= max_weight) {
      max_weight = currentWeight;
    }

    if (positionMM >= max_position) {
      max_position = positionMM;
    }

    int logTime = millis()-logStartTime;
    double logPosition = positionMM;
    double logWeight = currentWeight;  

    // Display if currently logging data
    if(isLogging) {
      logData(String(logTime),String(logPosition),String(logWeight));
      
      lcd.setCursor(0,2);
      lcd.print("Logging... "+String(logTime/1000)+" Sec");

      if( (currentWeight*1.25+500) < max_weight) {
        digitalWrite(Relay2, LOW);
        analogWrite(RPWM_Output, 0);

        isLogging = false;
        endLog();
        delay(5);
        digitalWrite(Relay4, LOW);
      }
    }
  }
}

void readQuadrature() {
  bool currentA = digitalRead(pinA);
  bool currentB = digitalRead(pinB);

  if (currentA != lastA || currentB != lastB) {
    // Determine direction based on the change in A and B states
    if (lastA == LOW && currentA == HIGH) {
      if (currentB == LOW) {
        position++;
      } else {
        position--;
      }
    } else if (lastA == HIGH && currentA == LOW) {
      if (currentB == HIGH) {
        position++;
      } else {
        position--;
      }
    }

    lastA = currentA;
    lastB = currentB;
  }
}

void endLog() {
  digitalWrite(Relay3, LOW);
  digitalWrite(Relay4, HIGH);
  lcd.clear();

  int logTime = millis()-logStartTime;
  double logPosition = position * conversionFactor;
  double logWeight = scale.get_units();
  
  lcd.setCursor(0, 0);
  lcd.print("Log Number: "+String(filename[4])+String(filename[5])+String(filename[6]));

  // Print max distance to LCD
  lcd.setCursor(0, 1);
  lcd.print("Distance:");
  lcd.setCursor(10, 1);
  lcd.print(max_position);
  lcd.print(" mm");

  // Print max weight to LCD
  lcd.setCursor(0, 2);
  lcd.print("Weight:");
  lcd.setCursor(10, 2);
  lcd.print(max_weight);
  lcd.print(" grams");

  // Last log entry
  logData(String(logTime),String(logPosition),String(logWeight));
  endLogging();

  delay(15000);
  lcd.clear();

  isLogging=false;
}

// Declare lastFileIndex globally
uint16_t lastFileIndex = 1; // Store the last used file index globally

// Buffer to hold data before writing to SD
#define BUFFER_SIZE 512  // Adjust buffer size as needed
char dataBuffer[BUFFER_SIZE];
uint16_t bufferIndex = 0;

void newFile() {
  for (uint16_t i = lastFileIndex; i < 1000; i++) {
    filename[4] = i / 100 + '0';       // Hundreds place
    filename[5] = (i / 10) % 10 + '0'; // Tens place
    filename[6] = i % 10 + '0';        // Units place

    if (!SD.exists(filename)) {
      myFile = SD.open(filename, FILE_WRITE); 
      if (myFile) {
        logStartTime = millis();
        lastFileIndex = i + 1;  // Update last used index
        
        // Initialize file header
        addToBuffer("Log Number:," + String(filename[4]) + String(filename[5]) + String(filename[6]) + "\n");
        addToBuffer("Motor Power:,"+String(motorSpeed)+"%\n");
        addToBuffer("Time(ms),Position(mm),Weight(grams)\n");
        break;  // Leave the loop
      }
    }
  }
}

// Function to add data to the buffer
void addToBuffer(String data) {
  for (size_t i = 0; i < data.length(); i++) {
    if (bufferIndex < BUFFER_SIZE - 1) {
      dataBuffer[bufferIndex++] = data[i];
    } else {
      flushBuffer();  // Flush buffer when full
      dataBuffer[bufferIndex++] = data[i];
    }
  }
}

// Function to write the buffer to the SD card
void flushBuffer() {
  if (bufferIndex > 0) {
    dataBuffer[bufferIndex] = '\0';  // Null-terminate the buffer
    myFile.write(dataBuffer, bufferIndex);
    myFile.flush();  // Ensure data is written to the card
    bufferIndex = 0;  // Reset the buffer index
  }
}

// Call this function to add data to the buffer during logging
void logData(String time, String position, String weight) {
  addToBuffer(time + "," + position + "," + weight + "\n");
}

// Ensure the buffer is flushed when done
void endLogging() {
  flushBuffer();
  if (myFile) {
    myFile.close();  // Close the file
  }
}

void referencePulse() {
  referenceHit = true;
}