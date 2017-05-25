/*
  WebClientRepeating

  This sketch processes SMS and calls and after
  that it connect to a web server and makes an HTTP request
  using an ArduinoUNO, SIM800L-Coroboard and ESP8266 module.

*/
#include "WiFiEsp.h"

// Emulate ESP8266 on pins 2/3 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"


//The connection in ESP8266 should be like these:
//
//ESP8266_TX->RX(D3)
//ESP8266_RX->TX(D2)
//ESP8266_CH_PD->3.3V
//ESP8266_VCC->3.3V
//ESP8266_GND->GND

//ESP8266_TX is connected to Arduino UNO as RX in D3
#define ESP8266_RX_PIN 3

//ESP8266_RX is connected to Arduino UNO as TX in D2
#define ESP8266_TX_PIN 2

//Create software serial object to communicate with ESP8266
SoftwareSerial ESP8266(ESP8266_RX_PIN, ESP8266_TX_PIN);

//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 8

//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 7

//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN, SIM800_RX_PIN);

#define TIMEOUT 20000

#define onModulePin 13

////SoftwareSerial Serial1(6, 7); // RX, TX
#endif

int8_t answer;
char *ssid                              = "Casa";// your network SSID (name)
char *pass                              = "remioy2006202";// your network password
int status                              = WL_IDLE_STATUS;// the Wifi radio's status
unsigned long lastConnectionTime        = 0;// last time you connected to the server, in milliseconds
const unsigned long postingInterval     = 10000L;// delay between updates, in milliseconds
int sensorValue                         = 0;//read the input on analog pin 0:
float voltage                           = 0;// convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
String voltageAsString                  = "";// value read from readLDR()
char *server                            = "estredoyaqueclub.com.ve";
char *url                               = "/arduinoenviacorreo.php?telefono=";
String Password                         = ""; // where password will be saved

char currentLine[200];
int currentLineIndex = 0;

//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage                 = false;

//Boolean to be set to true if call notificaion was found and next
//line of serial output is the actual call message content
bool nextValidLineIsCall               = false;

// Integer indexes
int firstComma                         = -1;
int secondComma                        = -1;
int thirdComma                         = -1;
int forthComma                         = -1;
int fifthComma                         = -1;

bool ledStatus                         = true;

//last line from SMS or a call
String lastLine                        = "";

// name of contact registered
byte admin                             = -1;



// Initialize the Ethernet client object
WiFiEspClient client;


/*

PINOUT Connection:

///////////////////////////////////////////////////////////////////////////////

External 12VDC/2A Power Supply                                    MP1584 (Turn knot until a volmeter shows 5VDC for SIM800L-EVB)

Positive--------------------------------------------------------->Positive

Negative--------------------------------------------------------->Negative

///////////////////////////////////////////////////////////////////////////////

MP1584                                                            SIM800L-EVB

Positive--------------------------------------------------------->VCC

Negative--------------------------------------------------------->GND


///////////////////////////////////////////////////////////////////////////////

External 12VDC/2A Power Supply                                    MP1584 (Turn knot until a volmeter shows 3.3VDC for ESP8266)

Positive--------------------------------------------------------->Positive

Negative--------------------------------------------------------->Negative

///////////////////////////////////////////////////////////////////////////////

MP1584                                                            ESP8266

Positive--------------------------------------------------------->VCC

Positive--------------------------------------------------------->CH_PD

Negative--------------------------------------------------------->GND

///////////////////////////////////////////////////////////////////////////////

Arduino UNO                                                        SIM800L-EVB

Digital 7--------------------------------------------------------->SIM_RXD

Digital 8--------------------------------------------------------->SIM_TX

RESET------------------------------------------------------------->RST

GND (POWER SECTION)----------------------------------------------->GND


///////////////////////////////////////////////////////////////////////////////

Arduino UNO                                                        ESP8266

Digital 2                                                          ESP8266_RX->TX

Digital 3                                                          ESP8266_TX->RX

GND                                                                GND

*/

void setup()
{
  
  // initialize digital pin 13 as an output.
  pinMode(onModulePin, OUTPUT);

  // initialize serial for debugging
  Serial.begin(115200);
  //initialize serial for SIM800L module
  serialSIM800.begin(9600);
  // initialize serial for ESP8266 module
  ESP8266.begin(9600);

  //initialize ESP module
  WiFi.init(&ESP8266);

  //check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) 
     {
		Serial.println("WiFi shield not present");
		// don't continue
		while (true);
	 }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) 
		{
			Serial.print("Attempting to connect to WPA SSID: ");
			Serial.println(ssid);
			// Connect to WPA/WPA2 network
			status = WiFi.begin(ssid, pass);
		}

  Serial.println("You're connected to the network");

  printWifiStatus();
}

void loop()
{

  Sim800Module();
  delay(5000);
  ESP8266Module();

}

//////////////////////////////////////////////////////////////////////////////////////////////////////

// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();

  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) 
	 {
		Serial.println("Connecting...");

		// cleans voltageAsString
		voltageAsString = "";
		// converts float voltage to String
		voltageAsString = String(readLDR());

		// send the HTTP PUT request
		client.print(String("GET ") + url + voltageAsString + " HTTP/1.1\r\n" +
					 "Host: " + server + "\r\n" +
					 "Connection: close" + "\r\n");

		//    client.println(F("GET /WhiteList.txt HTTP/1.1"));
		//    client.println(F("Host: castillolk.com.ve"));
		//    client.println("Connection: close");
		client.println();

		// note the time that the connection was made
		lastConnectionTime = millis();
     }
	 else 
	 {
		// if you couldn't make a connection
		Serial.println("Connection failed");
     }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
float readLDR()
{
  //read the input on analog pin 0:
  sensorValue = analogRead(A0);
  //Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  Serial.println(voltage);
  return voltage;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void power_on() 
{

  uint8_t answer = 0;

  Serial.println("On Power_on...");

  // checks if the module is started
  answer = sendATcommand("AT\r\n", "OK\r\n", TIMEOUT, 0);
  if (answer == 0)
	 {
		// power on pulse
		digitalWrite(onModulePin, HIGH);
		delay(3000);
		digitalWrite(onModulePin, LOW);

    // waits for an answer from the module
		while (answer == 0) 
			  {
				  // Send AT every two seconds and wait for the answer
				  answer = sendATcommand("AT\r\n", "OK\r\n", TIMEOUT, 0);
				  Serial.println("Trying connection with module...");
			  }
     }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int8_t sendATcommand(const char* ATcommand, const char* expected_answer1,unsigned int timeout, int xpassword) 
{

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  //while ( Serial.available() > 0) Serial.read();   // Clean the input buffer

  //  while(Serial.available()) { //Cleans the input buffer
  //        Serial.read();
  //    }

  while (serialSIM800.available())
		{ 	//Cleans the input buffer
			serialSIM800.read();
		}

  Serial.println(ATcommand);    // Prints the AT command
  serialSIM800.write(ATcommand); // Sends the AT command
  x = 0;
  previous = millis();

  // this loop waits for the answer
  do 
  {
    ////if (Serial.available() != 0) {
    if (serialSIM800.available() != 0)
	   {
		  ////response[x] = Serial.read();
		  response[x] = serialSIM800.read();
		  x++;
		  // check if the desired answer is in the response of the module
		  if (strstr(response, expected_answer1) != NULL)
			 {
				answer = 1;
				String numbFromSim = String(response);
				numbFromSim = numbFromSim.substring(numbFromSim.indexOf(":"),
                                            numbFromSim.indexOf(",129,"));
				numbFromSim = numbFromSim.substring((numbFromSim.indexOf(34) + 1),
                                            numbFromSim.indexOf(34, numbFromSim.indexOf(34) + 1));
				if ( xpassword == 1)
				   {
						numbFromSim = numbFromSim.substring( 0, 4);
						Password = numbFromSim ;
						return 0;
				   }
				else
				   {
						numbFromSim = numbFromSim.substring( 0, 11 );
				   }
             }
        }

  }
        
  // Waits for the asnwer with time out
  while ((answer == 0) && ((millis() - previous) < timeout));

  return answer;
}
///////////////////////////////////////////////////////////////////////////////
void Sim800Module()
{
  serialSIM800.listen();
  power_on();
  initialSettings();
  while(true)
       {
			//Write current status to LED pin
			digitalWrite(onModulePin, ledStatus);
			//If there is serial output from SIM800
           if(serialSIM800.available())
		     {
				char lastCharRead = serialSIM800.read();
               //Read each character from serial output until \r or \n is reached (which denotes end of line)
               if(lastCharRead == '\r' || lastCharRead == '\n')
			     {
					String lastLine = String(currentLine);
					
				////////////////////////////////////SMS/////////////////////////////////
				
                   //If last line read +CMT, New SMS Message Indications was received.
                   //Hence, next line is the message content.
                   if(lastLine.startsWith("+CMT:"))
			         {
						Serial.println(lastLine);
						nextLineIsMessage = true;
						//get second comma to know if on whitelist
						firstComma   = lastLine.indexOf(',');
						secondComma  = lastLine.indexOf(',', firstComma + 1);
						String indexChecker = lastLine.substring((firstComma + 2), (secondComma - 1));
						Serial.println(indexChecker);
						admin = indexChecker.toInt();
                     } 
				   else if (lastLine.length() > 0 && nextLineIsMessage) 
			              {
							   if(secondComma - firstComma > 3) //On whitelist
								 {
									 if(nextLineIsMessage) 
									   {
											Serial.println(lastLine);
											Serial.println("In phonebook");
											
											///////////////////CHANGE STATES ON LED/////////////
											
											//Read message content and set status according to SMS content
											if((lastLine.indexOf("LED ON") >= 0) && (lastLine.indexOf(',' + Password + ',') >= 0))
											  {
												ledStatus = 1;
												digitalWrite(onModulePin, ledStatus);
												CleanCurrentLine();
												nextLineIsMessage = false;
												break;
											  }
											else if((lastLine.indexOf("LED OFF") >= 0) && (lastLine.indexOf(',' + Password + ',') >= 0))
												   {
														ledStatus = 0;
														digitalWrite(onModulePin, ledStatus);
														CleanCurrentLine();
														nextLineIsMessage = false;
														break;
												   }
				                            ///////////////////ADDING USERS/////////////
											else if ((lastLine.indexOf("ADD") >= 0))
											        {
														if ((admin >= 1) && (admin <= 5))
														    {
																firstComma              = lastLine.indexOf(',');
																secondComma             = lastLine.indexOf(',', firstComma + 1);
																thirdComma              = lastLine.indexOf(',', secondComma + 1);
																String indexAndName     = lastLine.substring((firstComma + 1), (secondComma)); //Position and name to be saved on SIM
																String newContact       = lastLine.substring((secondComma + 1), thirdComma);  // Number to be saved on SIM
																String tmp              = "AT+CPBW=" + indexAndName + ",\"" + newContact + "\"" + ",129," + "\"" + indexAndName + "\"" + "\r\n\"";
																answer = sendATcommand(tmp.c_str(),"OK\r\n",5000,0);
																if (answer == 1)
																   {
																	   CleanCurrentLine();
																	   nextLineIsMessage = false;
																	   break;
																   }
																   else
																	  {
																		  Serial.println("Contact no added");
																	  }
														    }
														else
														   {
															   Serial.println("Not allowed to add contacts");
														   }	
													}
													///////////////////Deleting USERS/////////////
											else if ((lastLine.indexOf("DEL") >= 0))
											        {
														if ((admin >= 1) && (admin <= 5))
														    {
																firstComma = lastLine.indexOf(',');
																secondComma = lastLine.indexOf(',', firstComma + 1);
																thirdComma = lastLine.indexOf(',', secondComma + 1);
																String indexAndName = lastLine.substring((firstComma + 1), (secondComma)); //Position and name to be saved on SIM
																String newContact = lastLine.substring((secondComma + 1), thirdComma);  // Number to be saved on SIM
																String tmp = "AT+CPBW=" + indexAndName + "\r\n\"";
																answer = sendATcommand(tmp.c_str(),"OK\r\n",5000,0);
																if (answer == 1)
																   {
																	   ///Serial.println("Contact deleted from SIM card");
																	   CleanCurrentLine();
																	   nextLineIsMessage = false;
																	   break;
																   }
																   else
																	  {
																		  Serial.println("Contact no deleted");
																	  }
															}
															else
															   {
																   Serial.println(F("Not allowed to delete contacts"));
																   //Serial.println("Not allowed to delete contacts");
															   }
													} 
											nextLineIsMessage = false;
									   }
								 }
								 else
									{
										Serial.println(lastLine);
										Serial.println("Not in phonebook");
										nextLineIsMessage = false;
									}
					   }
					   
					   ///////////////////////CALL//////////////////
					   
					   else if (lastLine.startsWith("RING"))
					         {
								 Serial.println(lastLine);
                                 nextValidLineIsCall = true;
							 }
						else if ((lastLine.length() > 0) && (nextValidLineIsCall))        // Rejects any empty line
                                {
									//LastLineIsCLIP();
									if (nextValidLineIsCall)
									   {
											Serial.println(lastLine);
											// Parsing lastLine to determine registration on SIM card
											firstComma = lastLine.indexOf(',');
											//Serial.println(firstComma);  //For debugging
											secondComma = lastLine.indexOf(',', firstComma + 1);
											//Serial.println(secondComma); //For debugging
											thirdComma = lastLine.indexOf(',', secondComma + 1);
											//Serial.println(thirdComma);  //For debugging
											forthComma = lastLine.indexOf(',', thirdComma + 1);
											//Serial.println(forthComma); //For debugging
											fifthComma = lastLine.indexOf(',', forthComma + 1);
											//Serial.println(fifthComma); //For debugging
											
											if (fifthComma - forthComma > 3) //On whitelist
									           {
											    Serial.println("In contact"); //For debugging
											    ledStatus = 0;
											    //Write current status to LED pin
											   digitalWrite(onModulePin, ledStatus);
											   CleanCurrentLine();
											   nextValidLineIsCall = false;
											   break;
									           }
									        else
									           {
													Serial.println("Not in contact"); //For debugging
													CleanCurrentLine();
													nextValidLineIsCall = false;
                                               }
										}	

                                }	 
		        CleanCurrentLine();
                }
				else 
				   {
						currentLine[currentLineIndex++] = lastCharRead;
                   }
             }
	   }
  serialSIM800.end();
}
/////////////////////////////////////////////////////////////////////////////////
void ESP8266Module()
{
  ESP8266.listen();

  // Try three times HTTP Request
  
  /*The while loop might be also controlled by time, like
  if 10 seconds have passed since your last connection,
  then connect again and send data
  if (millis() - lastConnectionTime > postingInterval)
  {
    httpRequest();
  }*/

  int counter = 0;
  while (counter < 3) 
		{
			httpRequest();

			// if there's incoming data from the net connection send it out the serial port
			// this is for debugging purposes only
			while (client.available())
				  {
					char c = client.read();
					Serial.write(c);
				  }

			counter = counter + 1;
        }

  ESP8266.end();
}
///////////////////////////////////////////////////////
int sendSMS(char *phone_number, char *sms_text)
{
  char aux_string[30];
  uint8_t answer = 0;
  Serial.print("Setting SMS mode...");
  sendATcommand("AT+CMGF=1\r\n", "OK\r\n", TIMEOUT, 0);   // sets the SMS mode to text
  Serial.println("Sending SMS");
  sprintf(aux_string, "AT+CMGS=\"%s\"\r\n", phone_number);
  answer = sendATcommand(aux_string, ">", TIMEOUT, 0);   // send the SMS number
  if (answer == 1)
     {
		Serial.println(sms_text);
		serialSIM800.write(sms_text);
		delay(1000);
		serialSIM800.write(0x1A);
		delay(500);
		answer = sendATcommand("", "OK\r\n", TIMEOUT, 0);
     if (answer == 1)
        {
			Serial.println("Sent ");
        }
     else
	    {
			Serial.println("error ");
        }
     }
  else
     {
		Serial.println("error ");
		Serial.println(answer, DEC);
     }
  return answer;
}
///////////////////////////////////
void CleanCurrentLine()
{
  //Clear char array for next line of read
  for ( int i = 0; i < sizeof(currentLine);  ++i )
      {
        currentLine[i] = (char)0;
      }
  currentLineIndex = 0;
}
//////////////////////////////////////////
void initialSettings()
{
  Serial.println("Connecting to the network...");
  while ( (sendATcommand("AT+CREG?\r\n", "+CREG: 0,1\r\n", 5000, 0) ||
           sendATcommand("AT+CREG?\r\n", "+CREG: 0,5\r\n", 5000, 0)) == 0 );
  sendATcommand("AT+CMGF=1\r\n", "OK\r\n", 5000, 0);
  sendATcommand("AT+CNMI=1,2,0,0,0\r\n", "OK\r\n", 5000, 0);
  sendATcommand("AT+CPBR=1,1\r\n", "OK\r\n", 5000, 1);
  Serial.println("Password:");
  Serial.println(Password);
  ////sendSMS("04168262667", "Hello World!");
}
