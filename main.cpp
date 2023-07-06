#include <Arduino.h>
// MRT-Praktikum: Dimmen einer Lampe

// Inputs:
unsigned int taster1 = 0; // high wenn Taster gedrückt
unsigned int taster2 = 1; // high wenn Taster gedrückt
unsigned int p_ndg = 6; // Polling: wird high bei Spannungsnulldurchgang
unsigned int i_ndg = 9; // Interrupt: wird high bei Spannungsnulldurchgang, interrupt-fähiger Pin
unsigned int led1 = 14; // LED leuchtet wenn Pin high

// Outputs:
unsigned int gatestrom = 3; // prägt Gatestrom in Triac ein, wenn high
unsigned int led2 = 15; // LED leuchtet wenn Pin high

// Variablen:
unsigned long cpu_time = 0;
volatile unsigned int z = 0; // Zähler für Nulldurchgänge
float alpha = 36; // Zündwinkel in Grad
float alpha_min = 10;
float alpha_max = 160;
bool last_state_ndg = true; // Letzter Stand des Nulldurchganges
bool puls_eingepraegt = false;


void ISR_nulldurchgang() {
  z++;
  cpu_time = micros();
}
void dimmen() {
  if(!digitalRead(p_ndg)) {
    last_state_ndg = false;
  }
  
  if(digitalRead(p_ndg) && !last_state_ndg) {
    cpu_time = micros();
    last_state_ndg = true;
    puls_eingepraegt = false;
  }
 
  if(micros()>=cpu_time+(20000*alpha)/360 && !puls_eingepraegt) { // Zeit für Phasenanschnitt ist abgewartet und noch kein Puls eingeprägt
    digitalWrite(gatestrom, HIGH);
    cpu_time = micros();
    delayMicroseconds(10); // ähnlich delay
    digitalWrite(gatestrom, LOW);
    puls_eingepraegt = true;
  }
}

bool blink() {
  // Blink-Dauer: 1 Sekunde
  if(z<=100) { 
    // 100 * 10ms = 1000ms
    return(1);
  }
  else if(z>100) {
    
    if(z>200) {
      z=0;
    }
    return(0);
  }
}

void taster() {
  if(digitalRead(taster1) && !digitalRead(taster2) && alpha<=alpha_max) {
    alpha += 0.001;
  }
  else if(!digitalRead(taster1) && digitalRead(taster2) && alpha >=alpha_min) {
    alpha -= 0.001;
  }
}

void setup() {
  pinMode(gatestrom, OUTPUT);
  pinMode(p_ndg, INPUT);
  // Interrupt einstellen:
  attachInterrupt(digitalPinToInterrupt(i_ndg), ISR_nulldurchgang, RISING);
}

void loop() {
  if(blink()){
    dimmen();
  }

  taster();
}
