/*
  Drew Sinha
  Pincus Lab
  
  Provides a simple template for basic event acquisition and communication for an Arduino MICRO device over a serial connection
*/

const int ledPin = 2;  // Pin on Arduino board to which LED is connected (an additional tool that may be useful for debugging)
const int pedalPin =3; // Pin on Arduino board to which pedal is connected
const boolean VERBOSE = true;  // Boolean flag to turn verbose mode on
const boolean DEBUG = true;  //Boolean flag to turn debug mode on

const int MAX_BUFFER_SIZE = 64;  // Maximum buffer size (in number of chars); 64 is the minimum amount of characters neccessary at present
const int MSG_DELAY =  500;  // Delay in (us) between sending a serial message and pausing to allow appropriate communicat
enum TimerScale  //Enumeration for different timer modes (so the timer can run in milli or microseconds)
{
  MICROS,
  MILLIS
};
const TimerScale currentTimerMode = MILLIS;  //Mode flag for the mode used in this program
const long interruptDiscard = 2;  //Time to ignore further interrupt calls after initial interrupt (units of "currentTimerMode")
//Note: Don't need to mess around with timer overflow in any timer mode as all timer functions work with unsigned values, and subtraction will implicitly deal with turnover

int eventInterrupt;  //Place holder for the interrupt that we want to initialize for capturing events (corresponding to the approprite pin we connected the pedal/sensor to)
int intFlag;  //Used to reset the External Interrupt Flag Register (EIFR) so that pending interrupts are discarded in between times we attach and deattach the interrupt

unsigned long lastTriggerTime;  //The last time that the interrupt was triggered (units of "currentTimerMode)
char buffer[MAX_BUFFER_SIZE];  //Character array that gives our buffer
int bufferPos;  //Index that keeps track of the current position in the buffer
boolean msgToSend;  //Flag to check whether a message is waiting in the buffer to be sent
boolean isAcquiring; //State flag to signify whether the machine is acquiring an event


//Setup function for initializing the board
void setup()
{
  pinMode(ledPin,OUTPUT);    //Set pins to respective modes
  pinMode(pedalPin,INPUT);
  digitalWrite(pedalPin,HIGH);  //Turn on internal pull-up resistor on pedal pin
  lastTriggerTime = 0;    //Initialize other variables
  msgToSend =false;
  isAcquiring = false;
  bufferPos = 0;
  
  Serial.begin(9600);  //Start the serial connections
  
  switch(pedalPin)  //Pin to interrupt assignments; configured for Micro/Leonardo
  {
    case 2:
      eventInterrupt = 1;
      intFlag = INTF1;
      break;
    case 3:
      eventInterrupt = 0;
      intFlag = INTF0;
      break;  
  }
  detachInterrupt(eventInterrupt);  //Safeguard to prevent any other service from acting at this pin
  EIFR = intFlag;  //Reset the flag register
}

//Looping function that iterates on the board
void loop()
{
  if(!isAcquiring)  //If the device is not armed for acquiring an event
  {
    if(Serial.available())  //Check whether there is an character waiting
    {
         if(bufferPos == MAX_BUFFER_SIZE-1) {Serial.print("ERR-Buffer is full. Restart the board\n");} //If the buffer is about to be overloaded, run away from the problem
         else  //Otherwise, read the next character in the buffer
         {
            buffer[bufferPos] = Serial.read();
            if(buffer[bufferPos]=='\n'||buffer[bufferPos]=='\r'||buffer[bufferPos]=='\0')   //If we reached an EOL character,
            {
              if(DEBUG) {Serial.println("Buffer filled - reading...");}                  
              readBuffer();  //Read the buffer
            }
            //if(DEBUG && (buffer[bufferPos]=='\n'||buffer[bufferPos]=='\r'||buffer[bufferPos]=='\0')) {Serial.print("Read 1 character");}
            else{bufferPos++;} //Otherwise, increment the index and keep on going
         }
    }
  }
  else if (isAcquiring)  //If we are acquiring an event,
  {
    if(msgToSend)  // Check for a message to send; if there is one,
    {
      if(DEBUG) {Serial.println("Message found. Sending...");}
      msgToSend=false;  //Reset message flag
      printBufferToSerial(true);  //Print the buffer to the serial connection
      isAcquiring=false; //Reset state flag
    }
  }
  //Add more conditions as necessary
}

//Reads a filled buffer and performs the appropriate action after reading
void readBuffer()
{    
  //char tempStr[MAX_BUFFER_SIZE];
  //strToLower(buffer,tempStr);
  if(DEBUG) 
  {
    Serial.print("Buffer contents:");
    Serial.println(buffer);
  }
  if(strncmp(buffer,"aq",bufferPos-1)==0)  //If we wish to acquire some data,
   {
     isAcquiring = true;  //Set the state flag
     bufferPos=0;  //Reset the buffer position
     Serial.println("ACK-Command 'aq' received");  //Acknowledge receipt of command
     EIFR = intFlag;  //Reset the EIFR before attaching the interrupt to start waiting for events
     attachInterrupt(eventInterrupt,catchEvent,RISING);
     if(VERBOSE) {Serial.println("Currently waiting for events...");}
   }
   //Add more commands here as necessary
}

//Helper function for writing a string to the buffer
void writeBuffer(const char strToWrite[])   //Assume that character string is null-terminated
{ 
  volatile int len = strlen(strToWrite);  //Get the length of the string to make sure it isn't too long and is terminated properly
  if(len > MAX_BUFFER_SIZE-1){Serial.println("ERR-Buffer Write Failed: String too long.\n");}
  else if(strToWrite[len] != '\0'){Serial.println("ERR-Buffer Write Failed: String not null-terminated.\n");}  
  else {memcpy(buffer,strToWrite,len);}  //Copy the string to the buffer
}

//Copies the buffer to the serial port and (optionally) resets the buffer
//    resetBuffer - boolean that indicates whether to reset the buffer (position); true is the affirmative value
void printBufferToSerial(boolean resetBuffer)
{
  int tempPos = 0;  //Loop through the buffer and push the string through
  while(buffer[tempPos]!='\0')
  {
    Serial.print(buffer[tempPos]);
    tempPos++;
  }
  Serial.print('\n');  //Print the appropriate terminating character
  delay(MSG_DELAY);
  if(VERBOSE) {Serial.println("MSG-Message Sent.");}
  if(resetBuffer) {bufferPos=0;}  //If we wish to reset the buffer, reset the buffer position to the start
}

//Function for catching events; this one is bound to the interrupt in setup()
void catchEvent()
{
  int timeNow;  //Initialize a variable for grabbing the current time (relative to start of program)
  switch (currentTimerMode)
  {
    case MICROS:
      timeNow = micros();
      break;
    case MILLIS:
      timeNow = millis();
      break;
  }
  if(timeNow-lastTriggerTime > interruptDiscard)  //If this interrupt has been called long enough after the last trigger time, catch the event.
  {
    if(DEBUG) {Serial.println("MSG-Event triggered.");}
    writeBuffer("MSG-Event captured.\0");
    
    //Process the call as an actual event
    msgToSend = true;
    lastTriggerTime=timeNow; 
    
    digitalWrite(ledPin,HIGH);  //Light up the LED when we get an event
    delay(500); 
    digitalWrite(ledPin,LOW);
    detachInterrupt(eventInterrupt);  //Detach itself so that we only catch the first event (debounces the event)
  }
}
