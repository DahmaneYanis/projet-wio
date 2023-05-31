/*
-----------------------------------------------------------------
                          PROTOTYPES
-----------------------------------------------------------------
*/

void victoire();
void defaite();
void attente(unsigned int sec);
void launch();
void cleanTab(unsigned int tab[], unsigned int nbVal, unsigned int val);


/*
Joystick en bas à droite :

- y : -1 -> Droite | 1 -> Gauche
- x : -1 -> Haut | 1 -> Bas

Ordre bouton 
(C | B | A )
*/

#include"LIS3DHTR.h"
#include"TFT_eSPI.h"
#include <string.h>

TFT_eSPI tft;
LIS3DHTR<TwoWire> lis;

/*
-----------------------------------------------------------------
                        CONSTANTES GLOBALES
-----------------------------------------------------------------
*/
const unsigned int x_start = 160;
const unsigned int y_start = 120;
const unsigned int rayonArgent = 5;
const unsigned int rayonBoule = 7;
const unsigned int pas = 35;
const unsigned int nbArgent = 5;
const unsigned int valeurPiece = 10;

const String level[] = {"Novice", "Confirme", "Expert"};
const unsigned int fond = TFT_DARKGREY;
const unsigned int argent[3][5][2] = {{{50, 110},{30, 150},{120, 200},{210, 170},{290, 90}},{{10, 200},{150, 100},{100, 150},{40, 90},{70, 90}},{{150, 200},{150, 100},{100, 150},{40, 90},{70, 90}}};
const unsigned int piege[5][2] = {{50, 110},{30, 150},{120, 200},{210, 170},{290, 90}};

/*
-----------------------------------------------------------------
                        VARIABLES GLOBALES
-----------------------------------------------------------------
*/

unsigned int tabArgentTouche[5] = {5, 5, 5, 5, 5};
unsigned int tabPiegeTouche[5] = {5, 5, 5, 5, 5};
unsigned int cursorArgent = 0;
unsigned int cursorPiege = 0;
unsigned int x_pos = x_start; // Position abscisse
unsigned int y_pos = y_start; // Position ordonnée
float x_values, y_values; // Lecture des valeurs sur l'accéléromètre
bool playing = false; // En jeu ?
unsigned int currentLevel = 0;
unsigned int time = 30; 
unsigned long checkTime = 0;
unsigned int score = 0;
bool help = false; // Quand on appuie sur le bouton A
float x_angle, y_angle;
unsigned int save_x, save_y;

/*
=================================================================
                            AFFICHAGE
=================================================================
*/

/**
 * @brief Actualise l'affichage du temps actuel
 * 
 */
void afficheTemps(){
  tft.fillRect(280, 0, 30, 40, TFT_BLUE);
  tft.drawString((String) time, 280, 18);
}

/**
 * @brief Actualise l'affichage du score
 * 
 */
void afficheScore(){
  tft.fillRect(150, 0, 60, 40, TFT_BLUE);
  tft.drawString((String) score, 150, 18);
}

void bonus(){
  if (playing)
  {
    time += 10;
    help = true;
  }
}

/**
 * @brief Affiche graphiquement le point rouge à ses coordonnées actuelles
 * 
 */
void affichePointRouge(){
  tft.fillCircle(x_pos, y_pos, 7, TFT_RED);
}

/**
 * @brief Créer graphiquement la bannière
 * 
 */
void newBanniere(){
  tft.fillRect(0, 0, 320, 40, TFT_BLUE);
  tft.drawString((String) time, 280, 18);
  tft.drawString(level[currentLevel], 15, 18);
  afficheScore();
}

/**
 * @brief Créé une nouvelle page remplis d'un fond de la couleur de fond
 * 
 */
void newPage(){
  tft.fillScreen(fond);
}

/**
 * @brief Permet de charger graphiquement le level selectionné
 * 
 */
void chargeLevel(){
  newPage();
  newBanniere();

  // chargement des pieces
  for (unsigned int i = 0 ; i < 5 ; i++){
    tft.fillCircle(argent[currentLevel][i][0], argent[currentLevel][i][1], 5, TFT_YELLOW);
  }

  // chargement des pieges si il y en a
  if (currentLevel == 2){
      for (unsigned int i = 0 ; i < 5 ; i++){
        tft.fillCircle(piege[i][0], piege[i][1], 5, TFT_BLACK);
      }
  }
  x_pos = x_start;
  y_pos = y_start;
  affichePointRouge(); 

}


/*
=================================================================
                            COLLISION
=================================================================
*/

/**
 * @brief Vérifie si une pièce (dollar ou piege) n'a pas déjà été touchée (si l'indice est dans le tableau correspondant)
 * 
 * @param indice indice de la piece touchée
 * @param type 1 -> argent | 2 -> piege
 * @return true 
 * @return false 
 */
bool touche(unsigned int indice, const unsigned int tab[], unsigned int cursor){
  for (unsigned int i = 0 ; i < cursor+1 ; i++){
    if (indice == tab[i]){

      return true;
    }
  }
  return false;
}

/**
 * @brief Ajoute l'indice de la piece touchée dans le tableau correspondant
 * 
 * @param indice 
 * @param tab 
 * @param cursor 
 */
void ajouteTouche(unsigned int indice, unsigned int tab[], unsigned int * cursor){
  tab[*cursor] = indice;
  (*cursor)++;
}

/**
 * @brief Suppression de la pièce touchée
 * 
 * @param indice 
 * @param tab 
 * @param cursor 
 * @param x 
 * @param y 
 */
void gereTouche(unsigned int indice, int unsigned tab[], int unsigned * cursor, int x, int y){
  ajouteTouche(indice, tab, cursor); // Ajout de la piece touchée en mémoire
  tft.fillCircle(x, y, rayonArgent, fond); 
}

/**
 * @brief Gestion des collisions avec les pièces
 * 
 */
void argentEstTouche(unsigned int i){
  gereTouche(i, tabArgentTouche, &cursorArgent, argent[currentLevel][i][0], argent[currentLevel][i][1]);
  Serial.println("aRGENT touché");

  // AJout du score
  if (help){
    score+= valeurPiece/2;
  }
  else{
    score+= valeurPiece;
  }

  afficheScore();
}

/**
 * @brief Gestion des collisions avec les pièges
 * 
 * @param i 
 */
void piegeEstTouche(unsigned int i){
    gereTouche(i, tabPiegeTouche, &cursorPiege, piege[i][0], piege[i][1]);
    Serial.println("Piege touché");

    //ajouter la gestion du temps
    if ((int)time-10 > 0){
      Serial.println(time-10);
      time = time-10;
    }
    else{
      time = 0;
    }
    afficheTemps();
}

/**
 * @brief Gestion des collisions avec les pièces
 * 
 */
void collision(){
  // Test argent
  for (unsigned int i = 0 ; i < 5 ; i++){
    if (x_pos < argent[currentLevel][i][0]+rayonArgent+rayonBoule && x_pos > argent[currentLevel][i][0]-rayonArgent-rayonBoule && y_pos > argent[currentLevel][i][1]-rayonArgent-rayonBoule && y_pos < argent[currentLevel][i][1]+rayonArgent+rayonBoule){
      Serial.println(cursorArgent);
      if (!touche(i, tabArgentTouche, cursorArgent)){
        Serial.println("Erreror dslkfjs");
        argentEstTouche(i);
      }
    }
  }

  // Test Piege
  if (currentLevel == 2){
    for (unsigned int i = 0 ; i < 5 ; i++){
      if (x_pos < piege[i][0]+rayonArgent+rayonBoule && x_pos > piege[i][0]-rayonArgent-rayonBoule && y_pos > piege[i][1]-rayonArgent-rayonBoule && y_pos < piege[i][1]+rayonArgent+rayonBoule){
        Serial.println("WTF");
        Serial.println(cursorPiege);
        if (!touche(i, tabPiegeTouche, cursorPiege)){
          piegeEstTouche(i);
        }
      }
    }
  }
}

/**
 * @brief Gestion de la limite de l'écran
 * 
 */
void limite(){
  if (y_pos < 47 || y_pos > 233 || x_pos < 7 || x_pos > 313){
    defaite();
  }
}

/*
=================================================================
                      CONTRÔLE ET GAMEPLAY
=================================================================
*/

/**
 * @brief Victoire lorsque l'on a obtenu toutes les pièces
 * 
 */
void victoire(){
  tone(WIO_BUZZER, 80,  10000);
  launch();
}

/**
 * @brief Défaite causée soit par le temps soit par les limites de l'écran
 * 
 */
void defaite(){
  tone(WIO_BUZZER, 29,  10000);
  launch();
}

/**
 * @brief Test de fin de jeu (victoire ou défaite)
 * 
 * @return true 
 * @return false 
 */
bool fin(){
  if (time == 0){
    defaite();
    return true;
  }
  else if (score == 50){
    victoire();
    return true;
  }
  return false;  
}

/**
 * @brief Utilisation temporaire du joystick pour le développement du projet
 * 
 */
void joystick(){
  //Test echec
  x_angle = lis.getAccelerationX();
  y_angle = lis.getAccelerationY();
  save_x = x_pos;
  save_y = y_pos;
  x_pos -= y_angle * pas;
  y_pos += x_angle * pas;
  tft.fillCircle(save_x, save_y, rayonBoule, fond);
  
  affichePointRouge();


  // CLic sur le joystick : 
  // else if (digitalRead(WIO_5S_PRESS) == LOW) {
  //   Serial.println("5 Way Press");
  // }

}

/**
 * @brief Arrete le mode Jeu
 * 
 */
void stopLevel(){
  playing = false;
}

/**
 * @brief Démarre le mode Jeu
 * 
 */
void startLevel(){
  Serial.println("Test Interrupt");
  playing = true;
  checkTime = millis();
}

/**
 * @brief Change le level actuel et l'affiche
 * 
 */
void changeLevel(){
  Serial.println("ChangeLevel");
  if (currentLevel == 2){
    currentLevel = 0;
  } 
  else{
    currentLevel++;
  }
  chargeLevel();
}

/**
 * @brief Gestion des boutons C et B - Permet de choisir le level de son choix
 * 
 */
void selectLevel(){
  if (digitalRead(WIO_KEY_B) == LOW) {
    startLevel();
  }
  else if (digitalRead(WIO_KEY_C) == LOW) {
    changeLevel();
  }
}

/**
 * @brief Prépare l'environnement pour jouer/rejouer une nouvelle partie
 * 
 */
void launch(){
  playing = false;
  cleanTab(tabArgentTouche, nbArgent, nbArgent);
  cleanTab(tabPiegeTouche, nbArgent, nbArgent);
  cursorArgent = 0;
  cursorPiege = 0;
  x_pos = x_start; // Position abscisse
  y_pos = y_start; // Position ordonnée
  help = false; // Quand on appuie sur le bouton A
  score = 0;
  time = 30;
  chargeLevel();
}


void temps(){
  
  if (millis() - checkTime >= 1000){
    if (time-1 < 0){
      time = 0;
    }
    else{
      time--;
    }
    checkTime = millis();
    afficheTemps();
  }

}

/*
=================================================================
                            OUTILS
=================================================================
*/

/**
 * @brief Fonction attente avec utilisation de millis()
 * 
 * @param sec Temps d'attente en millisecondes
 */
void attente(unsigned int sec){
  unsigned long start = millis();
  unsigned long now = millis();
  while (now-start < sec){
    now = millis();
  }
}

/**
 * @brief Met à la valeur par défaut un tableau
 * 
 * @param tab 
 * @param nbVal 
 * @param val
 */
void cleanTab(unsigned int tab[], unsigned int nbVal, unsigned int val){
  for (unsigned int i = 0; i < nbVal; i++){
    tab[i] = val;
  }
}

/*
=================================================================
                        LOOP ET SETUP
=================================================================
*/

void setup() {
  pinMode(WIO_BUZZER, OUTPUT);

    // Initialisation des boutons 
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(WIO_KEY_C), changeLevel, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_KEY_B), startLevel, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_KEY_A), bonus, FALLING);
  
  Serial.begin(115200);
  // Initialisation du joystick
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  // Initialisation de l'écran (320x240)
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  launch();

  lis.begin(Wire1);
  if (!lis) {
    Serial.println("ERROR");
    while(1);
  }

  lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); //Data output rate
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G); //Scale range set to 2g
}

void loop() {
  if (playing){
    if (fin()){
      launch();
    }
    else {
      temps();
      limite();
      joystick();
      collision();
    }
  }
  delay(20);
}