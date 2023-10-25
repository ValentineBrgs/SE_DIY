// -------------------------------------------------------------------------- //
// Code pour l'activité Kinéis
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
// 3 - Lancement de l'acquisition, traitement et envoi
//     Récupération des données météo
//     Codage en hexadécimal
//     Envoi des données au shield Kinéis et transmission
//
//
// Les parties à remplir par les élèves seront indiquées par la notation suivante :
// // A remplir //
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
// --------------------- Initiaisation des Paramètres------------------------ //
// -------------------------------------------------------------------------- //


//------------- Définition des paramètres pour la station météo --------------//

//Librairies
#include <DHT.h>

//Constantes
#define DHTPIN 2        // définition du pin auxquel on connecte le détecteur
#define DHTTYPE DHT11   // DHT 11  (AM2302)

// Initialisation du détecteur
DHT dht(DHTPIN, DHTTYPE);

//Variables
int chk;
float hum;  //enregistre la valeur d'humidité
float temp; //enregistre la valeur de la température


//------------- Définition des paramètres pour le shield Kineis --------------//
//> Librairie du shield Kinéis
#include "KIM_Arduino_Library.h"

//> définition des intervalles de temporisation
#define TX_PERIOD       50000                      // période d'émission
#define TX_DURATION_MS  6000                       // temps d'émission
#define INTERVAL_MS     (TX_PERIOD-TX_DURATION_MS) // intervale de temps entre chaque émission

//Variables
char ID[2]  = "ID";
char FW[2]  = "FW";
char TX[2]  = "TX";
char PWR[3] = "PWR";

// Définition des variables pour stocker les mesures 
// issues du capteur et converties en hexadécimal
char stringhum[6];
char stringtemp[6];

// Définition de la variable contenant le message à envoyer au satellite
char DATA[] = "00000000";

/*
 * Le tableau ci-dessous décrit le formatage de la variable DATA qui contiendra
 * le message envoyé vers le satellite avec les valeurs mesurées de température
 * et d'humidité
	 ________________________________________
	|__________________DATA__________________|
	|  0 | 1  | 2  | 3  || 4  | 5  | 6  | 7  | < Hex ASCII
	|      Humidité     ||   Temperature     |
	|      2 octets     ||    2 octets       | < (0x0000 et 0x0000)
	|___________________||___________________|

	Exemple de construction de DATA:

		# Si Humidité = 49 % (decimal)
		=> 49 (decimal) = 0x31 (hexadecimal)

		# Si Temperature = 25.3 °C (decimal)
    // Opération qui nous permet de garder la précision au dixième de degré
    => 25.3 * 10 = 252                    
		=> 253 (decimal) = 0xFD (hexadecimal)

		Humidité    = 0x31 (hexadecimal)
		Temperature = 0xFD (hexadecimal)

		# Donc :
		DATA = "003100FD"
*/

// Définition de configurations

#if defined(__AVR_ATmega328P__) or defined(ESP8266) // Arduino UNO and Wemos D1

SoftwareSerial kserial(RX_KIM, TX_KIM);

#else // Arduino UNO Wifi Rev2, Arduino MKR Wifi 1010 and Arduino NANO 33 BLE

HardwareSerial &kserial = Serial1;

#endif

KIM kim(&kserial);
//------------------- Fin d'initialisation des paramètres----------------------//

// -------------------------------------------------------------------------- //
//-------------- Lancement de la boucle d'initialisation ---------------------//
// -------------------------------------------------------------------------- //
void setup() {
        dht.begin();
        Serial.begin(9600);   // Choix du taux d'échange d'informations
        Serial.println();
        Serial.println("KIM1 Arduino shield");

        kim.KIM_powerON(true);
        kim.KIM_userWakeupPinToggle();

        if (kim.KIM_check()) { //Vérification de la bonne connexion des jumpers
                Serial.println("KIM1 -- Check success");
        } else {
                Serial.println("KIM1 -- Check fail. Please check wiring and jumpers. Freezing.");
                while(1);
        }

        Serial.print("KIM1 -- Get ID : ");
        Serial.println(kim.KIM_sendATCommandGet(ID, sizeof(ID)));
        Serial.print("KIM1 -- Get FW : ");
        Serial.println(kim.KIM_sendATCommandGet(FW, sizeof(FW)));

        /* Set TX configuration */
        kim.KIM_sendATCommandSet(PWR, sizeof(PWR), "500", sizeof("500") - 1);
        Serial.print("KIM1 -- Get PWR : ");
        Serial.println(kim.KIM_sendATCommandGet(PWR, sizeof(PWR)));
}
//------------------ Fin de la boucle d'initialisation ------------------------//



// -------------------------------------------------------------------------- //
// -------------------- Lancement de la boucle principale --------------------//
// -------------------------------------------------------------------------- //
void loop() {
        // ----------- Partie 1 - Récupération des données météo -------------------//

        //> Lecture des données du capteur et stockage dans les vaiables hum et temp
        hum = dht.readHumidity();
        temp = dht.readTemperature();

        //> Affichage des valeurs dans le moniteur série
        Serial.print("Humidity: ");
        Serial.print(hum);
        Serial.print(" %, Temp: ");
        Serial.print(temp);
        Serial.println(" Celsius");

        // ------------ Partie 2 - Transformation en hexadécimal -------------------//
        /*
         * La fonction sprintf() permet d'écrire dans une variable au format chaine de caractère
         * des informations mises en forme.
         * Le premier paramètre est la variable cible
         * Le deuxième paramètre précise le format voulu, ici %04x signifie sur 4 octet et en hexadécimal
         * Le troisième paramètre est la valeur à formater
         * (int) permet de convertir ce qui suit en un nombre entier. Pour un nombre réel, cela revient
         * à le tronquer. Exemple : (int) 42.3 => 42
        */
        sprintf(stringtemp, "%04x", (int)(temp * 10));
        sprintf(stringhum , "%04x", (int)hum);

        // Copie des valeurs hexadecimales de temperature et d'humidité dans DATA
        for (int i = 0; i < 4; i++)
                DATA[i] = stringtemp[i];
        for (int i = 0; i < 4; i++)
                DATA[i + 4] = stringhum[i];

        // ------------ Partie 3 - Transfert vers le shield Kinéis et transmission --- //
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
// --------------------- Fin de la boucle principale ---------------------//
