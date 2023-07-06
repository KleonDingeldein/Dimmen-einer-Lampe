#include <Arduino.h>
// MRT-Praktikum: Dimmen einer Lampe

// Inputs:
unsigned int taster1 = 0; // high wenn Taster gedrückt
unsigned int taster2 = 1; // high wenn Taster gedrückt
unsigned int i_ndg = 9; // Interrupt: wird high bei Spannungsnulldurchgang, interrupt-fähiger Pin

// Outputs:
unsigned int gatestrom = 3; // prägt Gatestrom in Triac ein, wenn high

// Variablen:
float alpha = 90; // Initialzündwinkel
float alpha_min = 10; // Minimaler Zündwinkel
float alpha_max = 160; // Maximaler Zündwinkel
float delta_alpha = 0.001 // einzustellender Parameter welcher Geschwindigkeit der 
                          // Änderung des Zündwinkels bei Tasterbetätigung bestimmt

volatile bool puls_state = false // Zustandspeicher ob Puls abgegeben wurde
volatile unsigned long cpu_time = 0; // CPU-Zeit seit Programmbeginn
volatile unsigned int z = 0; // Zähler für Nulldurchgänge
// (Variablen welche in der ISR verwendet werden sollen als 'volatile' deklariert werden'

void ISR_nulldurchgang() {
  // für Blink-feature: Zähler der Nulldurchgänge
  z++; 
  
  // für Dimm-feature: 
  // Speicher setzen: seit diesem Nulldurchgang wurde noch kein Puls abgegeben:
  puls_state = false; 
  
  // für Dimm-feature: Zeitpunkt des Nulldurchganges wird abgespeichert:
  cpu_time = micros();
}

// Dimm-Feature: Die Helligkeit der Lampe variiert mit dem Zündwinkel
void dimmen() {
  // Bedingung zum Zünden: 
  // - Zündzeitverzögerung ist abgewartet 
  // - es wurde noch kein Puls seit dem letzten NDG abgegeben
  if(micros()>=cpu_time + (20000*alpha)/360 && !puls_state) {
    
    // Zündstrom wird in TRIAC eingeprägt:
    digitalWrite(gatestrom, HIGH);
    
    // 10 micro Sekunden werden abgewartet (Verwendung von delayMicroseconds() hier
    // unkritisch da kein NDG zu erwarten ist):
    delayMicroseconds(10);
    
    // Zündstrom ausschalten:
    digitalWrite(gatestrom, LOW);
    
    // Speicher setzen: Zündstrom wurde in dieser Halbwelle eingeprägt
    puls_state=true;
  }
}

// Blink-feature: liefert Blinksignal als bool'sche Variable
bool blink() {
  // Blink-Dauer: 1 Sekunde = 100 * 10 ms
  // Zähler z der NDG wird in ISR bearbeitet
  
  if(z<=100) { 
    return(1);
  }
    
  else if(z>100) {
    
    // eine Periode ist vergangen: setze Zähler der NDG auf =0
    if(z>200) {
      z=0;
    }
    
    return(0);
  }
}

// Feature: Helligkeit der Lampe kann mit Taster 1 und Taster 2 verändert werden
// (durch Veränderung des Zündwinkels)
bool taster() {
  // Bedingung zum Verringern/Erhöhen des Zündwinkels:
  // - Digitale Verriegelung der Taster
  // - minimaler/maximaler Zündwinkel wurde nicht unter-/überschritten

  if(digitalRead(taster1) && !digitalRead(taster2) && alpha<=alpha_max) {

    // Erhöhung des Zündwinkels um delta_alpha pro Schleifendurchgang
    alpha += delta_alpha;
  }
  else if(!digitalRead(taster1) && digitalRead(taster2) && alpha >=alpha_min) {

    // Verringerung des Zündwinkels um delta_alpha pro Schleifendurchgang
    alpha -= delta_alpha;
  }
}

void setup() {
  pinMode(gatestrom, OUTPUT);
  pinMode(p_ndg, INPUT);
  pinMode(taster1, INPUT);
  pinMode(taster2, INPUT);
  
  // Interrupt einstellen:
  // - digitalPinToInterrupt(i_ndg) legt Pin fest
  // - 'ISR_nulldurchgang' wird aufgerufen wenn Interrupt eintritt
  // - RISING: Interrupt triggert auf steigende Flanke
  attachInterrupt(digitalPinToInterrupt(i_ndg), ISR_nulldurchgang, RISING);
  
}

void loop() {
  // Blinken: TRIAC wird nur gezündet wenn Blink-Signal =true
  if(blink()){
    dimmen();
  }

  // Abfragen der Taster zur Veränderung des Zündwinkels
  taster();
}
