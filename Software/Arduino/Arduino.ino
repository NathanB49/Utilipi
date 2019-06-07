//#define MY_DEBUG

#define MY_RADIO_NRF5_ESB
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_NODE_ID 14
#include <MySensors.h>

#include <Wire.h> 
#include "Adafruit_MCP23017.h"

#define FRONT_PAR_TOUR  500
#define ANGLE_PAR_FRONT 360/FRONT_PAR_TOUR

//Constantes
#define BI_MANUEL         A0            // Actionneur utilisateur
#define SHUNT_SECU        A1            // Shunt défaut pour réglage
#define CAPTEUR_BANDE     A2            // Capteur présence bande
#define BP_ACQUIT         A3            // à définir
#define LED_YELLOW        A4            // à définir
#define LED_RED           A5            // à définir
#define LED_GREEN         A6            // SORTIE_SECU_OUTIL (la presse fonctionne si le moteur tourne et embrayé)
#define SORTIE_SECU_OUTIL A7            // Acquittement d'une erreur
#define ENCODEURA         PIN_RPI4      // Signal A du codeur
#define ENCODEURB         PIN_RPI2      // Signal B du codeur

#define INTERVALLE      10
#define ANGLE_BAS       100
#define ANGLE_HAUT      300

#define CHILD_ID_WORKING    0  
#define CHILD_ID_CADENCE    1 
#define CHILD_ID_NBR_PIECE  2 

MyMessage msg_working(CHILD_ID_WORKING, V_STATUS);
MyMessage msg_cadence(CHILD_ID_CADENCE, V_VAR1);
MyMessage msg_nbr_piece(CHILD_ID_NBR_PIECE, V_VAR2);

Adafruit_MCP23017 mcp1;
Adafruit_MCP23017 mcp2;

int MCP1_IN_Tab[]  = {};
int MCP1_IN_PULLUP_Tab[]  = {BI_MANUEL,SHUNT_SECU,CAPTEUR_BANDE,BP_ACQUIT};
int MCP1_OUT_Tab[] = {LED_GREEN, LED_YELLOW, LED_RED, SORTIE_SECU_OUTIL};
int MCP1_INTERRUPT_Tab[]  = {};

int MCP2_IN_Tab[]  = {};
int MCP2_IN_PULLUP_Tab[]  = {};
int MCP2_OUT_Tab[] = {};
int defaut = 0;
int Transistors_Tab[] = {PIN_T1,PIN_T2,PIN_T3,PIN_T4,PIN_T5,PIN_T6,PIN_T7,PIN_T8};
bool TransistorsState_Tab[]={0,0,0,0,0,0,0,0};

// Message d'erreur
char msg1[]="Pas de bande ou pas trop court";
char msg2[]="Capteur HS reste a 1 ou copeau bloque sur le capteur";

// Variable globale
volatile int angle = 0;
volatile uint32_t nb_pieces = 0;
volatile unsigned long cadence = 0;

void setup(){

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  //Serial.begin(115200);     // not needed if mysensor.h is called
  Serial.println("Starting...");

  initMCP();
  mcp1.digitalWrite(SORTIE_SECU_OUTIL,LOW);
  mcp1.digitalWrite(LED_GREEN,LOW);
  mcp1.digitalWrite(LED_YELLOW,LOW);
  mcp1.digitalWrite(LED_RED,LOW);

  pinMode(ENCODEURA,INPUT);
  pinMode(ENCODEURB,INPUT);
  //attachInterrupt(digitalPinToInterrupt(PIN_RPI_INTx), handleInterrupt, polarity)
  attachInterrupt(digitalPinToInterrupt(ENCODEURA),Codeur,RISING);
  
  for (int i=0; i<=sizeof(Transistors_Tab); i++){
    pinMode(Transistors_Tab[i], OUTPUT);
    digitalWrite(Transistors_Tab[i], HIGH);
  }

  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Ready!");

}

void loop(){

  static int defaut1 = 0;
  static int defaut2 = 0;
  static uint32_t last_nb_pieces = 0;
  static unsigned long last_cadence = 0;
  static bool working = false;
  static bool last_working = false;

  if (defaut!=0 && mcp1.digitalRead(BP_ACQUIT)==0){
    mcp1.digitalWrite(LED_RED,LOW);
    switch(defaut){
      case 1:
        defaut1++;
        break;
      case 2:
        defaut2++;
        break;
    }
    defaut=0;     
  }

  if ((mcp1.digitalRead(BI_MANUEL)==0)&&(defaut==0)){
    // la presse fonctionne
    working = true;
    if (mcp1.digitalRead(SHUNT_SECU)==0){
      
      if((angle>=ANGLE_BAS-INTERVALLE)&&(angle<=ANGLE_BAS+INTERVALLE)&&(mcp1.digitalRead(CAPTEUR_BANDE)==1)){
        mcp1.digitalWrite(LED_RED,HIGH);
        working = false;
        defaut=1;
      } else if((angle>=ANGLE_HAUT-INTERVALLE)&&(angle<=ANGLE_HAUT+INTERVALLE)&&(mcp1.digitalRead(CAPTEUR_BANDE)==0)){
        mcp1.digitalWrite(LED_RED,HIGH);
        working = false;
        defaut=2;
      }else{
        mcp1.digitalWrite(SORTIE_SECU_OUTIL,HIGH);
      }
      
    }else{
      // Mode shunt, on laisse l'outil touner
      mcp1.digitalWrite(SORTIE_SECU_OUTIL,HIGH);
    }
        
  }else{
    mcp1.digitalWrite(SORTIE_SECU_OUTIL,LOW);
  }

  // en train de descendre
  if(angle<180){
    mcp1.digitalWrite(LED_GREEN,HIGH);
  }else{
    mcp1.digitalWrite(LED_GREEN,LOW);
  }

  // en train de monter
  if(angle>180){
    mcp1.digitalWrite(LED_YELLOW,HIGH);
  }else{
    mcp1.digitalWrite(LED_YELLOW,LOW);
  }

  if (Serial.available()>0){          // Si des données sont disponibles
    int received = Serial.read();     // On les récupère
    serialmsg(received, defaut);              // Et on les traite
  }

  if (working != last_working) {
    send(msg_working.set(working));
    last_working = working;
  }
  
  if (nb_pieces != last_nb_pieces) {
    send(msg_nbr_piece.set(nb_pieces));
    last_nb_pieces = nb_pieces;
  }

  if (cadence != last_cadence) {
    send(msg_cadence.set(cadence));
    last_cadence = cadence;
  }
    
}

// Initialize general message
void before(){
  NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Pos3dBm;
}

void presentation(){
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("TeTou", "1.0");
  present(CHILD_ID_WORKING,S_BINARY);
  present(CHILD_ID_CADENCE,S_CUSTOM);
  present(CHILD_ID_NBR_PIECE,S_CUSTOM);
}

void receive(const MyMessage &message){
  if (message.type==V_VAR2) {
    // Change nombre piece
    nb_pieces = uint32_t(atoi(message.data));
  }
}

void Codeur(){
  /*
   * To achieve max speed for interrupt, you have to optimize read operation on I2C bus.
   * Since we know which pin are configured as interrupt and which port triggered the callback,
   * read ONLY the pin(s) on the port who actually triggered the interrupt
   * 
   * I do not advice to use the getLastInterruptPin() and getLastInterruptPinValue() function 
   * from the adafruit library for high speed interrupt (>1KHz)
   */

  static int front = 0;
  static unsigned long temps = 0;

  if(digitalRead(ENCODEURB)){
    front--;
  }else{
    front++;
  }

  // il y a 500 front par tour, on veut une valeur comprise entre 0 et 499
  if(front<0 or front > FRONT_PAR_TOUR-1){
    front=0;
    cadence = 60000/(millis()-temps);     // nombre de pièce minute
    temps=millis();
    nb_pieces++;
  }

  // angle est compris entre 0° et 359°
  angle = front*ANGLE_PAR_FRONT;

}

void serialmsg(int received, int defaut){
  
  switch (received){
  
    case 'a': 
      Serial.println("CONNECTE");
      break;
   
    case 'b':        
      Serial.println(angle);        // On envoie la valeur de l'angle
      break;

    case 'c':
      Serial.println(nb_pieces);
      break;
  
    case 'd':
      if (mcp1.digitalRead(BI_MANUEL)==0){
        Serial.println("Marche");
      }else{ 
        Serial.println("Arret"); 
      }
      break;

    case 'e':
      if (mcp1.digitalRead(CAPTEUR_BANDE)==0){ 
        Serial.println("Marche"); 
      }else{ 
        Serial.println("Arret"); 
      }
      break;

    case 'f':
      if (digitalRead(SORTIE_SECU_OUTIL)==1){ 
        Serial.println("Marche"); 
      }else{
        switch(defaut){
          case 0:
            Serial.println("Arret");
            break;
          case 1: 
            Serial.println(msg1);
            break;
          case 2:
            Serial.println(msg2);
            break;
        }
      }
      break; 

    case 'g':
      Serial.println(cadence);
      break;
  }
}  

void initMCP(){
  mcp1.begin();       // use default address 0
  mcp2.begin(4);      // use default address 4
  
  Wire.setClock(400000);  // change from 100KHz to 400KHz (max speed)
  pinMode(PIN_WIRE_SDA, INPUT);   //disable internal 13k pullup, better have 5k pullup external
  pinMode(PIN_WIRE_SCL, INPUT);   //idem as SDA

  for (int i=0;i<sizeof(MCP1_IN_Tab)/sizeof(int);i++){
    mcp1.pinMode(MCP1_IN_Tab[i], INPUT);
    mcp1.pullUp(MCP1_IN_Tab[i], LOW);                   // Normal input
  }

  for (int i=0;i<sizeof(MCP1_IN_PULLUP_Tab)/sizeof(int);i++){
    mcp1.pinMode(MCP1_IN_PULLUP_Tab[i], INPUT);
    mcp1.pullUp(MCP1_IN_PULLUP_Tab[i], HIGH);           // turn on a 100K pullup internally
  }
  
  for (int i=0;i<sizeof(MCP1_OUT_Tab)/sizeof(int);i++){
    mcp1.pinMode(MCP1_OUT_Tab[i], OUTPUT);
  }

  if(sizeof(MCP1_INTERRUPT_Tab)){
    //mcpx.setupInterrupts(mirroring, openDrain, polarity) --> default is false/false/LOW
    mcp1.setupInterrupts(false, false, HIGH);
    for (int i=0;i<sizeof(MCP1_INTERRUPT_Tab)/sizeof(int);i++){
      //mcpx.setupInterruptPin(pin, polarity)
      mcp1.setupInterruptPin(MCP1_INTERRUPT_Tab[i],CHANGE);
    }
  }

  for (int i=0;i<sizeof(MCP2_IN_Tab)/sizeof(int);i++){
    mcp2.pinMode(MCP2_IN_Tab[i], INPUT);
    mcp2.pullUp(MCP2_IN_Tab[i], LOW);                   // Normal input
  }
  
  for (int i=0;i<sizeof(MCP2_IN_PULLUP_Tab)/sizeof(int);i++){
    mcp2.pinMode(MCP2_IN_PULLUP_Tab[i], INPUT);
    mcp2.pullUp(MCP2_IN_PULLUP_Tab[i], HIGH);           // turn on a 100K pullup internally
  }
  
  for (int i=0;i<sizeof(MCP2_OUT_Tab)/sizeof(int);i++){
    mcp2.pinMode(MCP2_OUT_Tab[i], OUTPUT);
  }
}
