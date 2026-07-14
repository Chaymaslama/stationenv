#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// --- Capteurs ---
#define DHT_PIN PB9
#define DHT_TYPE DHT22
#define LDR_PIN PA0
#define MQ2_PIN PA1

// --- Actionneurs ---
#define ALERT_LED_PIN PB8
#define BUZZER_PIN PA2
#define SERVO_PIN PA3
#define RELAY_PIN PA4

// --- Seuils ---
#define TEMP_SEUIL_HAUT 28.0
#define GAZ_SEUIL 800   // valeur analogique (0-4095 sur STM32, 12 bits)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo aeration;

bool aerationOuverte = false;

void setup() {
  Serial.begin(115200);
  Serial.println("StationEnv - Demarrage");

  dht.begin();

  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(ALERT_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  // Le servo n'est pas attache au demarrage : il ne le sera
  // que ponctuellement, pour ne pas interferer avec le DHT22.

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur : ecran OLED non detecte !");
    while (true);
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

  // --- Lecture DHT22 avec tentatives multiples ---
  float temperature = NAN;
  float humidity = NAN;
  for (int tentative = 0; tentative < 3 && isnan(temperature); tentative++) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity)) {
      delay(100);
    }
  }

  int luminosite = analogRead(LDR_PIN);
  int gaz = analogRead(MQ2_PIN);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erreur de lecture DHT22 (apres 3 tentatives) !");
    return;
  }

  bool alerteTemp = temperature > TEMP_SEUIL_HAUT;
  bool alerteGaz = gaz > GAZ_SEUIL;
  bool alerte = alerteTemp || alerteGaz;

  // --- LED + buzzer + relais (reaction immediate, pas de timing sensible) ---
  digitalWrite(ALERT_LED_PIN, alerte ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, alerte ? HIGH : LOW);
  digitalWrite(RELAY_PIN, alerteTemp ? HIGH : LOW);

  // --- Servo : attache -> bouge -> detache, uniquement au changement d'etat ---
  if (alerteTemp && !aerationOuverte) {
    aeration.attach(SERVO_PIN);
    aeration.write(90);
    delay(300);
    aeration.detach();
    aerationOuverte = true;
  } else if (!alerteTemp && aerationOuverte) {
    aeration.attach(SERVO_PIN);
    aeration.write(0);
    delay(300);
    aeration.detach();
    aerationOuverte = false;
  }

  // --- Log serie ---
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" C | Humidite: "); Serial.print(humidity);
  Serial.print(" % | Luminosite: "); Serial.print(luminosite);
  Serial.print(" | Gaz: "); Serial.print(gaz);
  if (alerteTemp) Serial.print(" -> ALERTE TEMPERATURE");
  if (alerteGaz) Serial.print(" -> ALERTE GAZ");
  Serial.println();

  // --- Affichage OLED ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("T:"); display.print(temperature, 1); display.print("C ");
  display.print("H:"); display.print(humidity, 0); display.println("%");
  display.print("Lum:"); display.println(luminosite);
  display.print("Gaz:"); display.println(gaz);
  if (alerte) {
    display.println("");
    display.println("!! ALERTE !!");
  }
  display.display();
}