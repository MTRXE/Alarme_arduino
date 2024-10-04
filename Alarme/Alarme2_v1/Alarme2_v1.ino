/*  Bette Mathys
 *  CIEL 2
 *  Alarme2_v1
 *  Code 1234
 *  
 *  Systéme d'alarme avec un menu sécurisé si on veut activer ou désactiver l'alarme et une options pour modifier le code
 *  si alarme est activé et que la porte s'ouvre on as 15 sec pour désactiver avant que l'alarme ne se déclenche, si fenetre 
 *  ouverte l'alarme se déclenche instantanément
 *  
 *  2 Boutons, 1 LED, 1 Buzzer
 *  Boutons: D2 D4, LED: D6, Buzzer: D8
 *  
 *  
 *  Alarme1_v1 23-24-26/09/24 : Création d'une alarme pour activer ou la desactiver si activer porte= 15 sec pour desactiver ou alarme declencher 
 *  si fenetre ouverte déclenche tout de suite l'alarme et possibilité de modifier le code dans le menu
 *  Alarme1_v2 27/09/24 : Rajout d'un code pour activer ou desactiver l'alarme
 *  Alarme1_v3 30/09/24 : Paramétrage pour interdire le choix du menu avec plsuieurs cararctéres et vider les buffers
 *  Alarme1_v3 01/09/24 : Paramétrage pour interdire le choix du menu avec plsuieurs cararctéres et vider les buffers
 *  Alarme1_v3 01/09/24 : Affichage
 */

#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;
 
// Déclaration des pins et des variables
const int Porte = 2;
const int Fenetre = 4;
const int ledPin = 6;
const int Buzzer = 8;

// Variables d'état pour les capteurs et l'alarme
byte etatBuzzer = 0;
byte etatPorte = 0;
byte etatFenetre = 0;
byte etatLed = 0;  
byte etatAlarme = 0;  // 0: alarme désactivée, 1: alarme activée

// Code de l'alarme (initial et temporaire)
char CodeTableau[4] = {0x31, 0x32, 0x33, 0x34}; // Code initial
char Code[4]; // Code entré par l'utilisateur
bool CodeBon; // Variable pour vérifier si le code est correct

// Variables pour gérer les délais avec millis()
unsigned long tempsPrecedent = 0; // Pour stocker le dernier temps (millis)
unsigned long debutDelaiAlarme = 0; // Temps du début de la temporisation de l'alarme
const unsigned long delai15sec = 15000; // Durée du délai de 15 secondes
bool enAttenteDelai = false; // Pour savoir si le délai est en cours
bool alarmeDeclenchee = false; // Indique si l'alarme est déjà déclenchée

//****************************************************************
// Fonction pour vérifier si le code entré est correct
void SaisieCode() {
  CodeBon = true;
  for (byte i = 0; i < 4; i++) {
    if (Code[i] != CodeTableau[i]) {
      CodeBon = false;
      break;
    }
  }
}

//****************************************************************
// Fonction pour vérifier et entrer le code
void CodeVerifs() {
  lcd.clear();
  Serial.println("Entrez code (4) : ");
  lcd.print("Entrez code:");

  // Vider le buffer série avant la saisie du code
  while (Serial.available() > 0) {
    Serial.read();
  }
  
  while (Serial.available() < 4) {
    // Attend que l'utilisateur entre 4 chiffres
  }

  bool valide = true;
  for (byte i = 0; i < 4; i++) {
    Code[i] = Serial.read();
    if (Code[i] < '0' || Code[i] > '9') {
      valide = false;
      break;
    }
  }

  // Vider le buffer série après la saisie du code
  while (Serial.available() > 0) {
    Serial.read();
  }

  if (!valide) {
    lcd.clear();
    Serial.println("Code invalide");
    lcd.print("Code invalide");
    delay(1000);
    CodeBon = false;
    return;
  }

  SaisieCode();

  if (CodeBon) {
    lcd.clear();
    Serial.println("Code Bon");
    lcd.print("Code OK");
    delay(1000);
  } else {
    lcd.clear();
    Serial.println("Code incorrect");
    lcd.print("Code Faux");
    delay(1000);
  }
}

//****************************************************************
// Fonction qui affiche le menu à l'utilisateur
void AfficherMenu() {
  lcd.clear();
  Serial.println("Menu :");
  if (etatAlarme == 0) {
    Serial.println("A : Activer");
    Serial.println("C : Changer code");
    lcd.print("A:Activer");
    lcd.setCursor(0 , 1);
    lcd.print("C:Modif Code");
    lcd.setCursor(0 , 2);
  } else if (etatAlarme == 1) {
    Serial.println("B : Desactiver");
    lcd.print("B:Desactiver");
  }
}

//****************************************************************
// Fonction pour activer l'alarme
void ActiverAlarme() {
  lcd.clear();
  etatAlarme = 1;
  etatLed = 1;
  Serial.println("Alarme ON");
  lcd.print("Alarme ON");
  delay(2000);
  digitalWrite(ledPin, etatLed);
  tempsPrecedent = millis();  // Enregistre l'instant d'activation
  lcd.clear();
  AfficherMenu();
}

//****************************************************************
// Fonction pour désactiver l'alarme
void DesactiverAlarme() {
  lcd.clear();
  etatAlarme = 0;
  etatLed = 0;
  alarmeDeclenchee = false; // Réinitialise l'état d'alarme déclenchée
  enAttenteDelai = false;   // Arrête la temporisation si elle est en cours
  digitalWrite(Buzzer, LOW); // Stoppe le buzzer
  Serial.println("Alarme OFF");
  lcd.print("Alarme OFF");
  delay(2000);
  digitalWrite(ledPin, etatLed);
  AfficherMenu();
}

//****************************************************************
// Fonction pour changer le code de l'alarme
void ModifierCode() {
  lcd.clear();
  Serial.println("Nouveau code (4):");
  lcd.print("Nouveau code:");
  
  while (Serial.available() < 4) {
    // Attend la saisie de 4 chiffres
  }
  delay(100);

  for (byte i = 0; i < 4; i++) {
    CodeTableau[i] = Serial.read(); // Sauvegarde le nouveau code
  }

  lcd.clear();
  lcd.print("Code change");
  Serial.println("Code change");
  delay(1000);
  AfficherMenu();
}

//****************************************************************
// Fonction pour déclencher l'alarme
void DeclencherAlarme() {
  lcd.clear();
  if (!alarmeDeclenchee) {  // Évite de redéclencher l'alarme si elle l'est déjà
    lcd.print("Alarme ON!");
    Serial.println("Alarme déclenchée !");
    digitalWrite(Buzzer, HIGH); // Active le buzzer
    alarmeDeclenchee = true;
  }
}

//****************************************************************
// Fonction pour vérifier si les capteurs de porte ou de fenêtre sont activés
void VerifierOuvertures() {
  etatFenetre = digitalRead(Fenetre); // Lit l'état de la fenêtre
  etatPorte = digitalRead(Porte);     // Lit l'état de la porte

  if (etatAlarme == 1) { // Si l'alarme est activée
    if (etatFenetre == HIGH) { // Si la fenêtre est ouverte
      lcd.clear();
      lcd.print("Alarme ON!");
      DeclencherAlarme();     // Déclenche l'alarme
    }
    if (etatPorte == HIGH) {  // Si la porte est ouverte
      if (!enAttenteDelai) { // Si le délai n'est pas déjà en cours
        lcd.clear();
        Serial.println("Porte ouverte, 15 sec");
        lcd.print("Porte: 15sec off");
        debutDelaiAlarme = millis();  // Enregistre l'instant de l'ouverture de la porte
        enAttenteDelai = true;        // Active l'attente du délai
      }
    }

    // Vérifie si le délai de 15 secondes est écoulé
    if (enAttenteDelai && (millis() - debutDelaiAlarme >= delai15sec)) {
      enAttenteDelai = false; // Fin du délai
      if (etatAlarme == 1) {  // Si l'alarme est encore active
        DeclencherAlarme();   // Déclenche l'alarme
      }
    }
  }
}

//****************************************************************
void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Porte, INPUT);
  pinMode(Fenetre, INPUT);
  Serial.println("Alarme OFF");
  lcd.print("Alarme OFF");
  delay(3000);
  AfficherMenu(); // Affiche le menu au démarrage
}

//****************************************************************
void loop() {
  VerifierOuvertures(); // Vérifie l'état des capteurs

  if (Serial.available() > 0) {
    // Lire le premier caractère de la commande
    char commande = Serial.read();

    // Si plusieurs caractères sont entrés, afficher une erreur et vider le buffer
    if (Serial.available() > 0) {
      lcd.clear();
      Serial.println("Erreur multi-car.");
      lcd.print("Erreur multi-car.");
      delay(1000);
      
      // Vider le buffer série
      while (Serial.available() > 0) {
        Serial.read();
      }
      
      AfficherMenu();  // Affiche à nouveau le menu
      return; // Quitte la boucle pour ne pas exécuter d'autres commandes
    }

    // Exécuter la commande en fonction de l'entrée
    switch (commande) {
      case 'A':  
        // Activer l'alarme
        if (etatAlarme == 0) {
          CodeVerifs();
          if (CodeBon) {
            ActiverAlarme();
          }
        }
        break;

      case 'B': // Désactiver l'alarme
        if (etatAlarme == 1) {
          CodeVerifs();
          if (CodeBon) {
            DesactiverAlarme();
          } else {
            lcd.clear();
            Serial.println("Code faux");
            lcd.print("Code faux");
            delay(1000);
          }
        }
        break;

      case 'C': // Modifier le code
        if (etatAlarme == 0) {
          ModifierCode();
        } else {
          lcd.clear();
          Serial.println("Changer interdit ON");
          lcd.print("Changer interdit");
          delay(1000);
        }
        break;

      default: // Commande non reconnue
        lcd.clear();
        Serial.println("Cmd inconnue");
        lcd.print("Cmd inconnue");
        delay(1000);
        break;
    }
  }
} 
