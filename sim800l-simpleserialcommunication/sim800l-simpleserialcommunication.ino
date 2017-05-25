/* Piece of code for testing AT commands on SIM800L from Arduino UNO  */

#include <SoftwareSerial.h>
 
//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 8
 
//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 7
 
//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);


/*

PINOUT Connection:

///////////////////////////////////////////////////////////////////////////////

External 12VDC/2A Power Supply                                    MP1584 (Turn knot until a volmeter shows 5VDC)

Positive--------------------------------------------------------->Positive

Negative--------------------------------------------------------->Negative

///////////////////////////////////////////////////////////////////////////////

MP1584                                                            SIM800L-EVB

Positive--------------------------------------------------------->5V/4V

Negative--------------------------------------------------------->GNB

///////////////////////////////////////////////////////////////////////////////

Arduino UNO                                                        SIM800L-EVB

Digital 7--------------------------------------------------------->SIM_RXD

Digital 8--------------------------------------------------------->SIM_TX

RESET------------------------------------------------------------->RST

GND (POWER SECTION)----------------------------------------------->GND

///////////////////////////////////////////////////////////////////////////////

*/


void setup()
 {
  //Begin serial comunication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  while(!Serial);
   
  //Being serial communication witj Arduino and SIM800
  serialSIM800.begin(9600);
  delay(1000);
   
  Serial.println("Setup Complete!");
}
 
void loop()
{
  //Read SIM800 output (if available) and print it in Arduino IDE Serial Monitor
  if(serialSIM800.available())
  {
    Serial.write(serialSIM800.read());
  }
  //Read Arduino IDE Serial Monitor inputs (if available) and send them to SIM800
  if(Serial.available())
  {    
    serialSIM800.write(Serial.read());
  }
}

