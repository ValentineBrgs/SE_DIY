// Capteur de temperature et d'humidite DHT11
// https://tutoduino.fr/
// Copyleft 2020
 
#include "DHT.h"

// Definition de la broche sur laquelle la 
// broche DATA du capteur est reliee 
#define DHTPIN 2

// Definition du type de capteur utilisé
#define DHTTYPE DHT11

// Declaration d'un objet de type DHT
// Il faut passer en parametre du constructeur 
// de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
   
  // Initialisation le capteur DHT11
  dht.begin();
}

void loop() {
  // Recupere la temperature et l'humidite du capteur 
  // et l'affiche sur le moniteur serie
  Serial.println("Temperature = " + String(...)+" °C");
  Serial.println("Humidite = " + String(...)+" %");

  // Attendre 10 secondes avant de reboucler
  delay(10000);
}
