/*  Bette Mathys
 *  CIEL 2
 *  Alarme1_v2
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
 *  23-24-26/09/24 : Création d'une alarme pour activer ou la desactiver si activer porte= 15 sec pour desactiver ou alarme declencher 
 *  si fenetre ouverte déclenche tout de suite l'alarme et possibilité de modifier le code dans le menu
 *  27/09/24 : Rajout d'un code pour activer ou desactiver l'alarme
 */
 
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
void CodeVerifs() {
  
  Serial.println("Entrez le code (4 chiffres) : ");
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

  while (Serial.available() > 0) {
    Serial.read();
  }

  if (!valide) {
    Serial.println("Erreur: Le code doit être composé de 4 chiffres.");
    CodeBon = false;
    return;
  }

  for (byte i = 0; i < 4; i++) {
    Serial.print(Code[i]);
  }
  
  Serial.println("");
  SaisieCode();

  if (CodeBon) {
    Serial.println("Code Bon");
  } else {
    Serial.println("Code incorrect");
  }
}

//****************************************************************


//****************************************************************
// Fonction qui affiche le menu à l'utilisateur
void AfficherMenu() {
  Serial.println("Menu :");
  if (etatAlarme == 0) {
      Serial.println("1 : Activer l'alarme");
      Serial.println("3 : Changer le code");
  } else if (etatAlarme == 1) {
      Serial.println("2 : Désactiver l'alarme");
  }
  Serial.println("4 : Quitter");
}


//****************************************************************
// Fonction pour activer l'alarme
void ActiverAlarme() {
  
  etatAlarme = 1;
  etatLed = 1;
  Serial.println("Alarme activée");
  digitalWrite(ledPin, etatLed);
  tempsPrecedent = millis();  // Enregistre l'instant d'activation
  AfficherMenu();
}

//****************************************************************
// Fonction pour désactiver l'alarme
void DesactiverAlarme() {
  etatAlarme = 0;
  etatLed = 0;
  alarmeDeclenchee = false; // Réinitialise l'état d'alarme déclenchée
  enAttenteDelai = false;   // Arrête la temporisation si elle est en cours
  digitalWrite(Buzzer, LOW); // Stoppe le buzzer
  Serial.println("Alarme désactivée");
  digitalWrite(ledPin, etatLed);
  AfficherMenu();
}

//****************************************************************
// Fonction pour changer le code de l'alarme
void ModifierCode() {
  Serial.println("Entrez le nouveau code (4 chiffres) : ");
  while (Serial.available() < 4) {
    // Attend la saisie de 4 chiffres
  }
  delay(100);

  for (byte i = 0; i < 4; i++) {
    CodeTableau[i] = Serial.read(); // Sauvegarde le nouveau code
  }

  Serial.print("Nouveau code: ");
  for (byte i = 0; i < 4; i++) {
    Serial.print(CodeTableau[i]); // Affiche le nouveau code
  }
  Serial.println();
  AfficherMenu();
  
}

//****************************************************************
// Fonction pour déclencher l'alarme
void DeclencherAlarme() {
  if (!alarmeDeclenchee) {  // Évite de redéclencher l'alarme si elle l'est déjà
    Serial.println("Alarme déclenchée !");
    digitalWrite(Buzzer, HIGH); // Active le buzzer
    alarmeDeclenchee = true;
  }
}

//****************************************************************
// Fonction pour arrêter l'alarme
void ArreterAlarme() {
  Serial.println("Alarme arrêtée.");
  digitalWrite(Buzzer, LOW); // Désactive le buzzer
  alarmeDeclenchee = false;
}

//****************************************************************
// Fonction pour vérifier si les capteurs de porte ou de fenêtre sont activés
void VerifierOuvertures() {
  etatFenetre = digitalRead(Fenetre); // Lit l'état de la fenêtre
  etatPorte = digitalRead(Porte);     // Lit l'état de la porte

  if (etatAlarme == 1) { // Si l'alarme est activée
    if (etatFenetre == HIGH) { // Si la fenêtre est ouverte
      DeclencherAlarme();     // Déclenche l'alarme
    }
    if (etatPorte == HIGH) {  // Si la porte est ouverte
      if (!enAttenteDelai) { // Si le délai n'est pas déjà en cours
        Serial.println("Porte ouverte, délai de 15 secondes pour désactiver l'alarme.");
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
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Porte, INPUT);
  pinMode(Fenetre, INPUT);
  Serial.println("Alarme Désactivée");
  AfficherMenu(); // Affiche le menu au démarrage
}


//****************************************************************
void loop() {

  VerifierOuvertures(); // Vérifie l'état des capteurs

  if (Serial.available() > 0) {
    char commande = Serial.read();
    
    if (Serial.available() == 1){
      commande = 1;
    }else{
      if (Serial.available() > 0) {
        Serial.println("ERREUR");
        Serial.read(); // Vider tout le buffer
    }
   }

    switch (commande) {
      case '1':  
      // Activer l'alarme
        if (etatAlarme == 0) {
          CodeVerifs();
          if (CodeBon) {
            ActiverAlarme();
          } else {
            Serial.println("Code incorrect");
          }
        }
        break;

      case '2': // Désactiver l'alarme
        if (etatAlarme == 1) {
          CodeVerifs();
          if (CodeBon) {
            DesactiverAlarme();
          } else {
            Serial.println("Code incorrect");
          }
        }
        break;

      case '3': // Modifier le code
        if (etatAlarme == 0) {
          ModifierCode();
        } else {
          Serial.println("Impossible de modifier le code lorsque l'alarme est activée.");
        }
        break;

      case '4': // Retourner au menu principal
        Serial.println("Retour au menu principal...");
        AfficherMenu();
        break;

      default: // Commande non reconnue
        Serial.println("Commande non reconnue.");
        break;
    }
  }
}
