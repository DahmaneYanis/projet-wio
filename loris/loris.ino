#include "TFT_eSPI.h"
#include"LIS3DHTR.h"
LIS3DHTR<TwoWire> lis;
TFT_eSPI tft;
float f=10.0/3;
String chaine="et oui !";
unsigned long currentTime=0;
float x_angle, y_angle;
unsigned int x_pos = 100, y_pos = 100;
int cercleRadius = 7;

void setup() { 
    tft.begin();
    tft.setRotation(1); // Définis le coin de l'écran de coordonnées 0,0
    tft.fillScreen(TFT_RED); // fond rouge
    tft.fillCircle(50, 70, 9, TFT_CYAN);
    lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); //Data output rate
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G);
}
 
void loop() {
  currentTime = millis();
  tft.fillCircle(x_pos, y_pos, cercleRadius, TFT_CYAN);
  x_angle = lis.getAccelerationX();
  y_angle = lis.getAccelerationY();
  x_pos -= x_angle;
  Serial.println(x_pos);
  delay(100);
}
