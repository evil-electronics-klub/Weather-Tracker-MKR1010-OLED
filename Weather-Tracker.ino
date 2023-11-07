#include <ArduinoJson.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char ssid[] = "Your SSID";         // Replace with your WiFi network name
char pass[] = "Your Pasword";     // Replace with your WiFi network password
int status = WL_IDLE_STATUS;      // WiFi status
const char* host = "api.openweathermap.org";
const char* apiKey = "Your API Key";
String city = "Your Town,us";
String units = "imperial";

enum WeatherDisplayState {
  TEMPERATURE,
  HUMIDITY,
  PRESSURE
};

WeatherDisplayState currentState = TEMPERATURE;
unsigned long stateChangeTime = 0;

void setup() {
  Serial.begin(9600);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }

  Serial.println("Connected to WiFi");
  display.clearDisplay();
  display.display();
}

void loop() {
  switch (currentState) {
    case TEMPERATURE:
      displayTemperature();
      break;
    case HUMIDITY:
      displayHumidity();
      break;
    case PRESSURE:
      displayPressure();
      break;
  }

  if (millis() - stateChangeTime >= 10000) {
    // Switch to the next state after 10 seconds
    switch (currentState) {
      case TEMPERATURE:
        currentState = HUMIDITY;
        break;
      case HUMIDITY:
        currentState = PRESSURE;
        break;
      case PRESSURE:
        currentState = TEMPERATURE;
        break;
    }
    stateChangeTime = millis();
  }
}

void displayTemperature() {
  float temperature = getWeatherData("temp");
  displayValue("Temp", temperature, "°F");
}

void displayHumidity() {
  int humidity = getWeatherData("humidity");
  displayValue("Humidity", humidity, "%");
}

void displayPressure() {
  float pressure = getWeatherData("pressure");
  displayValue("Pressure", pressure, "hPa");
}

float getWeatherData(const char* key) {
  WiFiClient client;
  String url = "/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=" + units;
  
  if (!client.connect(host, 80)) {
    Serial.println("Connection failed");
    return 0.0;
  }
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  String response = client.readString();
  client.stop();
  
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, response);
  
  JsonObject main = doc["main"];
  return main[key];
}

void displayValue(const char* label, float value, const char* units) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(label);
  display.setCursor(0, 20);
  display.print(value);
  display.display();
}
