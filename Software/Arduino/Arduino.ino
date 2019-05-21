
// Enable debug prints
#define MY_DEBUG

#define MY_RADIO_NRF5_ESB

#define MY_RF24_PA_LEVEL RF24_PA_HIGH

#define MY_NODE_ID 15

#include <MySensors.h>
#include <Wire.h> 
#include "Adafruit_MCP23017.h"

int RELAY_PIN[] = {PIN_T1,PIN_T2,PIN_T3,PIN_T4,PIN_T5,PIN_T6,PIN_T7,PIN_T8};
#define NUMBER_OF_RELAYS 8 // Total number of attached relays
#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

#define SavedPos 100
#define SLEEPDELAY 10000

Adafruit_MCP23017 mcp1;
Adafruit_MCP23017 mcp2;

int MCP1_IN_PULLUP_Tab[]  = {A0,A1,A2,A3,A4,A5,A6,A7};
int MCP1_IN_PULLDOWN_Tab[]  = {};
int MCP1_OUT_Tab[] = {B0,B1,B2,B3,B4,B5,B6,B7};

int MCP2_IN_PULLUP_Tab[]  = {A0,A1,A2,A3,A4,A5,A6,A7};
int MCP2_IN_PULLDOWN_Tab[]  = {};
int MCP2_OUT_Tab[] = {B0,B1,B2,B3,B4,B5,B6,B7};


// Initialize general message
void before()
{
  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos3dBm;
  
  for (int sensor=1, pin=0; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    // Then set relay pins in output mode
    hwPinMode(RELAY_PIN[pin], OUTPUT_H0H1);
    // Set relay to last known state (using eeprom storage)
    digitalWrite(RELAY_PIN[pin], loadState(sensor)?RELAY_ON:RELAY_OFF);
  }

}

void setup()
{

  initMCP();
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RPI_INTA,INPUT);
  pinMode(PIN_RPI_INTB,INPUT);
  
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Utilipy", "0.4");

  for (int sensor=1, pin=0; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_BINARY);
  }

}

void handleInterrupt(){  
  // Get more information from the MCP from the INT
  uint8_t pin=mcp1.getLastInterruptPin();
  uint8_t val=mcp1.getLastInterruptPinValue();
  
  uint8_t flashes=4; 
  if(pin==A0) flashes=1;
  if(pin==A1) flashes=2;
  if(val!=LOW) flashes=3;

  for(int i=0;i<flashes;i++){  
    delay(400);
    digitalWrite(LED_BUILTIN,0);
    delay(400);
    digitalWrite(LED_BUILTIN,1);
  }
}

void loop()
{
   
  //Enter deep Sleep
  int sleeptrig = smartSleep(digitalPinToInterrupt(PIN_RPI_INTA),CHANGE,SLEEPDELAY);

  //Gestion de l'interuption
  if (sleeptrig == 7){
    handleInterrupt();
    mcp1.digitalRead(A0);
  }

   
}

void receive(const MyMessage &message)
{
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_STATUS) {
    // Change relay state
    digitalWrite(RELAY_PIN[message.sensor-1], message.getBool()?RELAY_ON:RELAY_OFF);
    // Store state in eeprom
    saveState(message.sensor, message.getBool());
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}

void initMCP(){
  
  mcp1.begin();      // use default address 0

  for (int i=0;i<sizeof(MCP1_IN_PULLUP_Tab);i++){
    mcp1.pinMode(MCP1_IN_PULLUP_Tab[i], INPUT);
    mcp1.pullUp(MCP1_IN_PULLUP_Tab[i], HIGH);           // turn on a 100K pullup internally
    mcp1.digitalRead(MCP1_IN_PULLUP_Tab[i]);             // Reads the MCP to reset leftover interupt
  }
  
  for (int i=0;i<sizeof(MCP1_IN_PULLDOWN_Tab);i++){
    mcp1.pinMode(MCP1_IN_PULLDOWN_Tab[i], INPUT);
    mcp1.pullUp(MCP1_IN_PULLDOWN_Tab[i], HIGH);           // turn on a 100K pullup internally
    mcp1.digitalRead(MCP1_IN_PULLDOWN_Tab[i]);             // Reads the MCP to reset leftover interupt
  }
  
  for (int i=0;i<sizeof(MCP1_OUT_Tab);i++){
    mcp1.pinMode(MCP1_OUT_Tab[i], OUTPUT);
  }

//UNCOMMENT and modify to create interupt on MCP1  
  mcp1.setupInterrupts(true,false,LOW);
  mcp1.setupInterruptPin(A0,FALLING);


  mcp2.begin(4);   // use default address 4
  
  for (int i=0;i<sizeof(MCP2_IN_PULLUP_Tab);i++){
    mcp2.pinMode(MCP1_IN_PULLUP_Tab[i], INPUT);
    mcp2.pullUp(MCP1_IN_PULLUP_Tab[i], HIGH);           // turn on a 100K pullup internally
    mcp2.digitalRead(MCP1_IN_PULLUP_Tab[i]);             // Reads the MCP to reset leftover interupt
  }
  
  for (int i=0;i<sizeof(MCP2_IN_PULLDOWN_Tab);i++){
    mcp2.pinMode(MCP1_IN_PULLDOWN_Tab[i], INPUT);
    mcp2.pullUp(MCP1_IN_PULLDOWN_Tab[i], HIGH);           // turn on a 100K pullup internally
    mcp2.digitalRead(MCP1_IN_PULLDOWN_Tab[i]);             // Reads the MCP to reset leftover interupt
  }
  
  for (int i=0;i<sizeof(MCP2_OUT_Tab);i++){
    mcp2.pinMode(MCP1_OUT_Tab[i], OUTPUT);
  }
}
