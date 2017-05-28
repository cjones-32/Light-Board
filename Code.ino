//96 Well Plate Light Board
//Designed, created and coded by Cody Jones


#include <Wire.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();
SoftwareSerial mySerial(12, 11); // pin 12 = RX (unused), pin 11 = TX

const byte ROWS = 4; //four rows
const byte COLS = 5; //five columns
String inData;//current input
char lastRowString[1];
char lastColumnString[2];
int stringLength = 0; //how many characters are in input including # command
int row = 0; //designates which row is being addressed
int lastType = 1; //for step forward - backwards - type 1 == individual lights; type 2 == full row; type 3 == full column
int lastRow = 0; //row of last user input
int lastColumn = 0; //column of last user input

//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'A', 'E', '1', '2', '3'},
  {'B', 'F', '4', '5', '6'},
  {'C', 'G', '7', '8', '9'},
  {'D', 'H', '*', '0', '#'}
};
byte rowPins[ROWS] = {4, 5, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9, 10}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  Serial.println("Program Initialized");
  matrix.begin(0x70);  // pass in the address
  matrix.clear();
  matrix.writeDisplay();
  mySerial.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(1); //clear display
  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.write("96 Well Plate");
  mySerial.write(254); // move cursor to beginning of second line
  mySerial.write(192);
  mySerial.write("Light Board");
}

void loop() {

  //record all button presses to array inData threw serial
  char recievedKey = customKeypad.getKey();


  //record all button presses to array inData threw serial
  //while (Serial.available() > 0) {  //for serial input
  if (recievedKey) {  //for keypad input
    char recieved = recievedKey;
    inData += recieved;
    stringLength++;
    Serial.println(inData);
    if (stringLength == 1) { //Start input display
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);
      mySerial.write("Input:          ");
      mySerial.write(254); // move cursor to beginning of second line
      mySerial.write(192);
      mySerial.write("                ");
      mySerial.write(254); // move cursor to end of Input:
      mySerial.write(135);
    }
    mySerial.write(recieved);

    //
    //Clear input if * recieved after input is already supplied
    //
    if ((stringLength > 1) && (recieved == '*')) { //clear if * pressed not first
      inData = ""; // Clear recieved buffer
      stringLength = 0;
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);
      mySerial.write("Input:          ");
      mySerial.write(254); // move cursor to beginning of second line
      mySerial.write(192);
      mySerial.write("                ");
      mySerial.write(254); // move cursor to end of Input:
      mySerial.write(135);
      Serial.println("Cleared input");
    }

    //
    //Step left if * is pressed first
    //

    else if ((stringLength < 2) && (recieved == '*')) {  //step left
      Serial.println(stringLength);
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(1);
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);
      if ( lastType == 1) {  //Last light toggle
        Serial.println("Shift Light Left");
        matrix.displaybuffer[lastRow] &= 0xFFFF - int(pow(2, lastColumn - 1) + .5);
        lastColumn--;
        if ( lastColumn < 1 ) {
          lastColumn = 12;
          lastRow --;
          if ( lastRow < 0 ) {
            lastRow = 7;
          }
        }
        matrix.displaybuffer[lastRow] |= int(pow(2, lastColumn - 1) + .5);
        mySerial.write("Previous light");
        mySerial.write(254); //move to secong line
        mySerial.write(192);
        sprintf(lastRowString, "%d", lastRow);
        mySerial.write(lastRowString[0] + 17);
        sprintf(lastColumnString, "%d", lastColumn);
        mySerial.write(lastColumnString[0]);
        if (lastColumn > 9) {
          mySerial.write(lastColumnString[1]);
        }
      }
      else if ( lastType == 2 ) {  //last row toggle
        Serial.println("Shift Row Left");
        matrix.displaybuffer[lastRow] &= 0x0000;
        lastRow --;
        if ( lastRow < 0 ) {
          lastRow = 7;
        }
        matrix.displaybuffer[lastRow] |= 0xFFFF;
        mySerial.write("Previous Row");
        mySerial.write(254); //move to secong line
        mySerial.write(192);
        sprintf(lastRowString, "%d", lastRow);
        mySerial.write(lastRowString[0] + 17);
      }
      else if ( lastType == 3 ) { //Last column toggle
        Serial.println("Shift Column Left");
        for (row = 0; row < 8; row ++) {
          matrix.displaybuffer[row] &= 0xFFFF - int(pow(2, lastColumn - 1) + .5);
        }
        lastColumn--;
        if ( lastColumn < 1 ) {
          lastColumn = 12;
        }
        for (row = 0; row < 8; row ++) {
          matrix.displaybuffer[row] |= int(pow(2, lastColumn - 1) + .5);
        }
        mySerial.write("Last column");
        mySerial.write(254); //move to secong line
        mySerial.write(192);
        sprintf(lastColumnString, "%d", lastColumn);
        mySerial.write(lastColumnString[0]);
        if (lastColumn > 9) {
          mySerial.write(lastColumnString[1]);
        }
      }
      Serial.println("Clear Data");
      matrix.writeDisplay();
      inData = ""; // Clear recieved buffer
      stringLength = 0;
    }
    //
    // Process message when # is recieved
    //
    else if (recieved == '#') {
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(1);   // Clear Screen
      mySerial.write(254); // move cursor to beginning of first line
      mySerial.write(128);

      if ( stringLength == 1 ) {
        Serial.println("Step Next");
        if ( lastType == 1) {  //next light toggle
          Serial.println("Shift Light Right");
          matrix.displaybuffer[lastRow] &= 0xFFFF - int(pow(2, lastColumn - 1) + .5);
          lastColumn++;
          if ( lastColumn > 12 ) {
            lastColumn = 1;
            lastRow ++;
            if ( lastRow > 7 ) {
              lastRow = 0;
            }
          }
          matrix.displaybuffer[lastRow] |= int(pow(2, lastColumn - 1) + .5);
          mySerial.write("Next light");
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          sprintf(lastRowString, "%d", lastRow);
          mySerial.write(lastRowString[0] + 17);
          sprintf(lastColumnString, "%d", lastColumn);
          mySerial.write(lastColumnString[0]);
          if (lastColumn > 9) {
            mySerial.write(lastColumnString[1]);
          }

        }
        else if ( lastType == 2 ) {  //next row toggle
          Serial.println("Shift Row Right");
          matrix.displaybuffer[lastRow] &= 0x0000;
          lastRow ++;
          if ( lastRow > 7 ) {
            lastRow = 0;
          }
          matrix.displaybuffer[lastRow] |= 0xFFFF;
          mySerial.write("Next Row");
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          sprintf(lastRowString, "%d", lastRow);
          mySerial.write(lastRowString[0] + 17);
        }
        else if ( lastType == 3 ) { //Next column toggle
          Serial.println("Shift Column Right");
          for (row = 0; row < 8; row ++) {
            matrix.displaybuffer[row] &= 0xFFFF - int(pow(2, lastColumn - 1) + .5);
          }
          lastColumn++;
          if ( lastColumn > 12 ) {
            lastColumn = 1;
          }
          for (row = 0; row < 8; row ++) {
            matrix.displaybuffer[row] |= int(pow(2, lastColumn - 1) + .5);
          }
          mySerial.write("Next column");
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          sprintf(lastColumnString, "%d", lastColumn);
          mySerial.write(lastColumnString[0]);
          if (lastColumn > 9) {
            mySerial.write(lastColumnString[1]);
          }
        }
      }
      if ( stringLength == 2 ) {
        if ( inData[0] == '0' ) { //clear all lights
          matrix.clear();
          Serial.println("Clear all");
          mySerial.write("Cleared all");
        }
        else if ( (inData[0] < 73) && (inData[0] > 64) ) { //whole row on - if first char is letter
          Serial.print("whole row on ");
          Serial.println(inData[0]);
          matrix.displaybuffer[(inData[0] - 65)] = 0xFFFF; //for ASCII lower case to DEC
          lastRow = inData[0] - 65;
          lastType = 2;
          mySerial.write("Whole row on"); //first line
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          mySerial.write(inData[0]); //display input
        }
        else if ( (inData[0] < 58) && (inData[0] > 48) ) {  //whole column on less then 10 - if first char is number
          Serial.print("full column on for less then 10 ");
          Serial.println(inData[0] - 48);
          for (row = 0; row < 8; row ++) {
            matrix.displaybuffer[row] |= int(pow(2, inData[0] - 49) + .5); // for ASCII
          }
          lastColumn = inData[0] - 48;
          lastType = 3;
          mySerial.write("Whole column on"); //first line
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          mySerial.write(inData[0]); //display input
        }
        else { //ERROR
          Serial.println("ERROR length == 2");
        }
      }
      else if ( stringLength == 3 ) {
        if ( ((inData[0] < 73) && (inData[0] > 64)) && ((inData[1] < 58) && (inData[1] > 48)) ) { //toggle light less then 10 - if first char is letter and second char is letter
          Serial.print("Light toggle ");
          Serial.print(inData[0]); //Row
          Serial.println(inData[1] - 48); //For ASCII column
          matrix.displaybuffer[(inData[0] - 65)] ^= int(pow(2, inData[1] - 49) + .5); // for ASCII
          lastRow = inData[0] - 65;
          lastColumn = inData[1] - 48;
          lastType = 1;
          mySerial.write("Toggled light"); //first line
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          mySerial.write(inData[0]); //display input
          mySerial.write(inData[1]); //display input
        }
        else if ( ((inData[0] < 50) && (inData[0] > 48)) && ((inData[1] < 58) && (inData[1]) > 47) ) { //column > 9 - if first and second chars are numbers
          Serial.print("whole column on for greater then 9 ");
          Serial.println((inData[0] - 48) * 10 + (inData[1] - 48)); //for ASCII
          for (row = 0; row < 8; row ++) {
            matrix.displaybuffer[row] |= int(pow(2, (inData[0] - 48) * 10 + (inData[1] - 49)) + .5); // for ASCII, 49 + .5 is for FLOPS
          }
          lastColumn = (inData[0] - 48) * 10 + (inData[1] - 48);
          lastType = 3;
          mySerial.write("Whole column on"); //first line
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          mySerial.write(inData[0]); //display input
          mySerial.write(inData[1]); //display input
        }
        else { //ERROR
          Serial.println("ERROR length == 3");
          mySerial.write("ERROR"); //first line
        }
      }
      else if (stringLength == 4) {
        if ( ((inData[0] < 73) && (inData[0] > 64)) && ((inData[1] < 50) && (inData[1] > 48)) && ((inData[2] < 58) && (inData[2]) > 47) ) { //light > 9 - if first and second chars are numbers
          Serial.print("light toggle ");
          Serial.print(inData[0]); //Row
          Serial.println((inData[1] - 48) * 10 + (inData[2] - 48)); //For ASCII column
          matrix.displaybuffer[(inData[0] - 65)] ^= int(pow(2, (inData[1] - 48) * 10 + (inData[2] - 49)) + .5); // for ASCII
          lastRow = inData[0] - 65;
          lastColumn = (inData[1] - 48) * 10 + (inData[2] - 48);
          lastType = 1;
          mySerial.write("Toggled light"); //first line
          mySerial.write(254); //move to secong line
          mySerial.write(192);
          mySerial.write(inData[0]); //display input
          mySerial.write(inData[1]); //display input
          mySerial.write(inData[2]); //display input
        }
        else { //ERROR
          Serial.println("ERROR length == 4");
          mySerial.write("ERROR"); //first line
        }
      }
      //Clear All
      else if ( inData[0] == '0' ) {
        matrix.clear();
        Serial.println("Clear all");
        mySerial.write("Cleared all");
      }
      Serial.println("Clear Data");
      matrix.writeDisplay();
      inData = ""; // Clear recieved buffer
      stringLength = 0;
    }
  }
}




