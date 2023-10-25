// -------------------------------------------------------------------------- //
// Document support activité Kinéis
// 
// Ce document est constitué de trois parties :
//
// 1 - Initialisation des paramètres 
//    Du capteur météo
//    du shield Kinéis
//
// 2 - Lancement des protocoles d'initialisation
//    (pas important de comprendre cette partie purement technique)
//
// 3 - Lancement des algorithmes
//     Récupération des données météo
//     Codage en hexadécimal
//     Envoi des données au shield Kinéis et transmission
//
// Les parties à remplir par les élèves seront indiquées par la notation suivante :
// // A remplir //
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
// --------------------- Initiaisation des Paramètres------------------------ //
// -------------------------------------------------------------------------- //


//------------- Définition des paramètres pour la station météo --------------//

//Libraries
#include <DHT.h>

//Constantes
#define DHTPIN 2     // définition du pin auxquel on connecte le détecteur
#define DHTTYPE DHT11   // DHT 11  (AM2302)
// Initialisation du détecteur
DHT dht(DHTPIN, DHTTYPE);

//Variables
int chk;
float hum;  //enregistre la valeur d'humidité
float temp; //enregistre la valeur de la température


//------------- Définition des paramètres pour le shield Kineis --------------//

//Librairie
#include "KIM_Arduino_Library.h"

// définition d'intervales de temps
#define TX_PERIOD 50000       // période d'émission
#define TX_DURATION_MS  6000  // durée d'émission
#define INTERVAL_MS     (TX_PERIOD-TX_DURATION_MS) // intervale de temps entre chaque émission

//Variables
char ID[2]  = "ID";
char FW[2]  = "FW";
char TX[2]  = "TX";
char PWR[3] = "PWR";
char DATA[] = "...";
char stringhum[] = "...";
char stringtemp[] = "...";

// Définition de configurations

#if defined(__AVR_ATmega328P__) or defined(ESP8266) // Arduino UNO and Wemos D1

SoftwareSerial kserial(RX_KIM, TX_KIM);

#else // Arduino UNO Wifi Rev2, Arduino MKR Wifi 1010 and Arduino NANO 33 BLE

HardwareSerial &kserial = Serial1;

#endif

KIM kim(&kserial);


//------------------- Fin d'initialisation des paramètres----------------------//

// -------------------------------------------------------------------------- //
//------------------------ Lancement des protocoles --------------------------//
void setup() {

  Serial.begin(9600);
  dht.begin();
  Serial.println();

  Serial.println("Space Elevator - Kinéis Arduino shield");
	
  kim.KIM_powerON(true);
  kim.KIM_userWakeupPinToggle();
  
  //Vérification de la bonne connexion des jumpers
  if (kim.KIM_check()) { 
    Serial.println("KIM1 -- Check success");
  } 
  else {
    Serial.println("KIM1 -- Check fail. Please check wiring and jumpers. Freezing.");
    while(1)
    ;
  }
  
  // Vérifier la présence du module KIM 1 puis du KIM 2 et ces identifiants
  if (kim.KIM_isKIM2() == true) 
  Serial.println("KIM2 -- KIM2 module detected");
  else
  Serial.println("KIM1 -- KIM1 module detected");

  Serial.print("KIM1 -- Get ID : ");
  Serial.println(kim.KIM_sendATCommandGet(ID, sizeof(ID)));
  Serial.print("KIM1 -- Get FW : ");
  Serial.println(kim.KIM_sendATCommandGet(FW, sizeof(FW)));

  /* Set TX configuration */
  kim.KIM_sendATCommandSet(PWR, sizeof(PWR), "500", sizeof("500") - 1);
  Serial.print("KIM1 -- Get PWR : ");
  Serial.println(kim.KIM_sendATCommandGet(PWR, sizeof(PWR)));
}
// -------------------------- Fin des protocoles d'initialisation -----------------//


// -------------------------------------------------------------------------- //
// ------------------------- Lancement des algorithmes -----------------------//
void loop() {  
  // ----------- Partie 1 - Récupération des données météo -------------------//
  //Lecture des données et stockage dans les vaiables hum et temp
  hum = dht.readHumidity();
  temp= dht.readTemperature();

  //Affichage des valeurs dans la commande
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
 
  // ------------ Partie 2 - transformation en hexadécimal -------------------//
  stringtemp =  char(temp, HEX);   
  stringhum =  char(hum, HEX);    
  DATA = stringtemp  ; 
  
  // ------------ Partie 3 - Transfert vers le shield Kinéis et transmission --- //
  // Boucle controlant l'envoi du message donc si tous les paramètres //
  // sont vérifiés et remplis alors le message est envoye //
  Serial.print("KIM1 -- Send data ... ");
  
  kim.KIM_sendATCommandSet(TX, sizeof(TX), DATA, sizeof(DATA) - 1);
  if (kim.KIM_getState() == KIM_OK)
    Serial.println("Message sent");
  else
    Serial.println("Error");

  Serial.println("KIM1 -- Turn OFF");

  kim.KIM_powerON(false);
  delay(INTERVAL_MS + random(-0.1 * TX_PERIOD, 0.1 * TX_PERIOD));
  kim.KIM_powerON(true);
}

// ----------------------------- Fin du programe --------------------------//
