#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHT_PIN PB9
#define DHT_TYPE DHT22
#define ALERT_LED_PIN PB8
#define TEMP_SEUIL_HAUT 28.0

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Serial.println("StationEnv - Demarrage");
  dht.begin();
  pinMode(ALERT_LED_PIN, OUTPUT);
  digitalWrite(ALERT_LED_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur : ecran OLED non detecte !");
    while (true); // bloque ici si l'écran ne répond pas
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("StationEnv");
  display.display();
  delay(1000);
}

void loop() {
  delay(2000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Erreur de lecture du capteur DHT22 !");
    return;
  }

  bool alerte = temperature > TEMP_SEUIL_HAUT;
  digitalWrite(ALERT_LED_PIN, alerte ? HIGH : LOW);

  // Affichage série
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C  |  Humidite: ");
  Serial.print(humidity);
  Serial.println(alerte ? " %  -> ALERTE TEMPERATURE !" : " %");

  // Affichage OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("StationEnv");
  display.println("");
  display.setTextSize(2);
  display.print(temperature, 1);
  display.println(" C");
  display.setTextSize(1);
  display.print("Humidite: ");
  display.print(humidity, 0);
  display.println(" %");

  if (alerte) {
    display.println("");
    display.println("!! ALERTE !!");
  }

  display.display();
}