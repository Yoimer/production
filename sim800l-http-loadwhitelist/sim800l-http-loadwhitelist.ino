//---------Start-Libraries Section----------------------------------------------------//

#include <gprs.h>
#include <SoftwareSerial.h>

//--------------------------------End-Libraries Section----------------------------------------------------//


//--------------------------------Start-Definition Section--------------------------------------------------//

#define TIMEOUT    60000
#define LED_PIN    13
//#define TIMEOUTINTERNET 30000

//--------------------------------End-Definition Section--------------------------------------------------//


//--------------------------------Start-Object Creation--------------------------------------------------//

GPRS gprs;

//--------------------------------End-Object Creation--------------------------------------------------//


//--------------------------------Start-Variable Declaration-------------------------------------------//


// Integer indexes

int j = -1;
int i = -1;
int kk = -1;
int len = -1;


char startChar = '#'; // or '!', or whatever your start character is
char endChar = '#';
boolean storeString = false; //This will be our flag to put the data in our buffer

int const DATABUFFERSIZE = 180;  //10 phonenumbers
static char dataBuffer[DATABUFFERSIZE - 40]; //Includes NULL terminator
byte dataBufferIndex = 0;

char response [DATABUFFERSIZE + 1]; //Includes NULL terminator

char* wordlist;
//DATABUFFERPHONENUMBER = 10
char* phoneNumber[11]; //Add 1 for NULL terminator

byte oldNumber;
byte newNumber;

char jj[DATABUFFERSIZE + 1];

//--------------------------------End-Variable Declaration-------------------------------------------//



//------------------------------End-Libraries Section--------------------------------------------------------//


//--------------------------------Start-Setup Section-----------------------------------------------//

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  while (!Serial);

  // initialize digital pin 13 as an output.
  pinMode(LED_PIN, OUTPUT);

  //Write current status to LED pin
  //digitalWrite(LED_PIN, ledStatus);

  Serial.println("Starting SIM800 Http Development");
  gprs.preInit();
  delay(1000);

  while (0 != gprs.init())
  {
    delay(1000);
    Serial.print("init error\r\n");
  }

  Serial.println("Init success");
}
void loop() {
  // put your main code here, to run repeatedly:

  GetWhiteList();
  Serial.println("Added...");
  delay(180000);

}

///////////////////////////////////////////////////////////////////////////
void GetWhiteList()
{
  Serial.println("On GetWhiteList...");
  const int MAX = 6;
  char const * const commandlist[] =
  {
    //command 1  Sets GPRS Context
    "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n",
    //command 2  Sets APN
    "AT+SAPBR=3,1,\"APN\",\"internet.movistar.ve\"\r\n",
    //command 3  Connects to internet
    "AT+SAPBR=1,1\r\n",
    //command 4  Gets DHCP IP given by provider
    "AT+SAPBR=2,1\r\n",
    //command5   Establishes HTTP session
    "AT+HTTPINIT\r\n",
    //command 6 Defines Cloud
    "AT+HTTPPARA=\"URL\",\"www.castillolk.com.ve/WhiteList.txt\"\r\n",
  };

  for ( i = 0; i < MAX; i++)
  {
    Serial.println("Actual command:");
    Serial.println(commandlist[i]);
    if (0 != gprs.sendCmdAndWaitForResp(commandlist[i], "OK", TIMEOUT)) //Expects OK
    {
      ERROR("ERROR:");
      Serial.println("There was a network problem. System will restart, please wait...");
      RestartSystem();
      delay(TIMEOUT); // Waits for system to restart
    }
    Serial.println("Passed!");
  }

  //GET response command.
  Serial.println("Actual command:");
  Serial.println("AT+HTTPACTION=0\r\n");
  if (0 != gprs.sendCmdAndWaitForResp("AT+HTTPACTION=0\r\n", "200", TIMEOUT)) //Expects 200
  {
    ERROR("ERROR:");
    Serial.println("There was a network problem. System will restart, please wait...");
    RestartSystem();
    delay(TIMEOUT); // Waits for system to restart
  }
  Serial.println("Passed!");

  gprs.sendCmd("AT+HTTPREAD\r\n");
  Serial.print("Actual Command: ");
  Serial.println("AT+HTTPREAD\r\n");
  //getSerialString();
  gprs.cleanBuffer(response, sizeof(response));
  gprs.readBuffer(response, sizeof(response));
  Serial.print("Printing response:");
  Serial.println(response);
  Serial.print("Printing strlen:");
  Serial.println(strlen(response));  // Does not include a trailing null character
  len = strlen(response);
  len = len + 1;
  getSerialString();
  parseSerialString();
  LoadWhiteList();
  ////ClearWhiteList();


  //  wordlist = strtok(dataBuffer, ",");
  //  Serial.print("Printing wordlist:");
  //  Serial.println(wordlist);
  //  newNumber = atoi(wordlist);
  //  Serial.print("Printing newNumber :");
  //  Serial.println(newNumber);

  //  Serial.print("Printing for loop:");
  //  for (i = 0; i < len; ++i)
  //  {
  //    Serial.print(response[i]);
  //  }

  //gprs.cleanBuffer(response, sizeof(response));
  //readData(response, sizeof(response), 60000);
  //gprs.readBuffer(response, sizeof(response));
  //Serial.print("Printing response:");
  //Serial.println(response);

}

////////////////////////////////////////////////////////////////////////////////////////////////
void RestartSystem()
{
  if (0 != gprs.sendCmdAndWaitForResp("AT+CPOWD=1\r\n", "NORMAL POWER DOWN", TIMEOUT))
  {
    ERROR("ERROR:CPOWD");
    //return;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////

//Removes startChar and endChar
boolean getSerialString()
{
  Serial.println("On getSerialString...");
  Serial.print("Printing response:");
  Serial.println(response);
  Serial.print("Printing strlen:");
  Serial.print(len);
  static byte dataBufferIndex = 0;
  i = 0;
  ////while (Serial.available() > 0)
  while (i < len )
  {
    ////char incomingbyte = Serial.read();
    Serial.println("On while loop...");
    char incomingbyte = response[i];
    Serial.print(incomingbyte);
    if (storeString)
    {
      //Let's check our index here, and abort if we're outside our buffer size
      //We use our define here so our buffer size can be easily modified
      if (dataBufferIndex == DATABUFFERSIZE)
      {
        //Oops, our index is pointing to an array element outside our buffer.
        dataBufferIndex = 0;
        break;
      }
      if (incomingbyte == endChar)
      {
        dataBuffer[dataBufferIndex] = 0; //null terminate the C string
        //Our data string is complete.  return true
        Serial.println("Value of dataBuffer");
        Serial.println(dataBuffer);
        return true;
      }
      else
      {
        dataBuffer[dataBufferIndex++] = incomingbyte;
        dataBuffer[dataBufferIndex] = 0; //null terminate the C string   //<Hello, 10.5, 21>
      }
    }
    else
    {

    }
    if (incomingbyte == startChar)
    {
      dataBufferIndex = 0;  //Initialize our dataBufferIndex variable
      storeString = true;
    }
    i = i + 1;
  }

  //We've read in all the available Serial data, and don't have a valid string yet, so return false
  return false;
}
////////////////////////////////////////////////////////////////////////////
void parseSerialString()
{
  Serial.println("On parseSerialString...");
  wordlist = strtok(dataBuffer, ",");  //Extracts first comma
  Serial.print("Printing wordlist:");
  Serial.println(wordlist);
  oldNumber = atoi(wordlist);         //Converts first number into an integer
  Serial.print("Printing oldNumber :");
  Serial.println(oldNumber);
  wordlist = strtok(NULL, ",");      //Extracts second comma
  newNumber = atoi(wordlist);       //Converts second number into an integer
  Serial.print("Printing newNumber :");
  Serial.println(newNumber);
  static byte phoneNumberIndex = 0;
  while ((wordlist = strtok(NULL, ",")) != NULL)
  {
    phoneNumber[phoneNumberIndex++] = wordlist;
    phoneNumber[phoneNumberIndex] = 0; //null terminate the C string
  }

  j = 0;
  for ( j = 0; j < newNumber; ++j)
  {
    Serial.println(phoneNumber[j]);
  }
}

//////////////////////////////////////////////////////////////////////////////////
void ClearWhiteList()
{
  j = 1;         // lleva la cuenta de los nros a borrar
  while (j <= oldNumber)
  {
    Serial.println("Deleting Contacts.");
    itoa(j, jj, 10);
    Serial.println(jj);
    // tmp = "AT+CPBW=" + jj + "\r\n"; numChars
    char *tmp = join3Strings("AT+CPBW=", jj, "\r\n");
    if (0 != gprs.sendCmdAndWaitForResp(tmp, "OK", TIMEOUT))
    {
      ERROR("ERROR:CPBW");
      Serial.println("There was a network problem. System will restart, please wait...");
      RestartSystem();
      delay(TIMEOUT); // Waits for system to restart
    }
    Serial.println(tmp);       // comando AT a ejecutar ??
    j   = j + 1;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
char* join3Strings(char* string1, char* string2, char* string3)
{
  ////gprs.cleanBuffer(respuesta, indexRespuesta);
  gprs.cleanBuffer(response, sizeof(response));
  strcat(response, string1);
  strcat (response, string2);
  strcat (response, string3);
  return response;
}
///////////////////////////////////////////////////////////////////////////
char* join7Strings(char* string1, char* string2, char* string3, char* string4, char* string5, char* string6, char* string7)
{
  ////gprs.cleanBuffer(respuesta, indexRespuesta);
  gprs.cleanBuffer(response, sizeof(response));
  strcat(response, string1);
  strcat (response, string2);
  strcat (response, string3);
  strcat (response, string4);
  strcat (response, string5);
  strcat (response, string6);
  strcat (response, string7);
  return response;
}
//////////////////////////////////////////////////////////////////
void LoadWhiteList()
{
  ClearWhiteList();
  j = 0;
  while (j < newNumber)
  {
    //Serial.println("On LoadWhiteList() loop");
    itoa(j + 1, jj, 10);
    Serial.println(jj);
    char *tmp = join7Strings("AT+CPBW=", jj, ",\"", phoneNumber[j], "\",129,\"" , jj , "\"\r\n");
    Serial.println(tmp);

    if (0 != gprs.sendCmdAndWaitForResp(tmp, "OK", TIMEOUT))
    {
      ERROR("ERROR:CPBW");
      Serial.println("There was a network problem. System will restart, please wait...");
      RestartSystem();
      delay(TIMEOUT); // Waits for system to restart
    }

    j = j + 1;
  }
}


