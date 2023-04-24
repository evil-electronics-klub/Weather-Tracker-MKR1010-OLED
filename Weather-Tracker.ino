#include <ArduinoJson.h>
#include <WiFiNINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char ssid[] = "Your_SSID";      // your network SSID (name)
char pass[] = "Your_Password"; // your network password
int status = WL_IDLE_STATUS;   // the WiFi radio's status

void setup() {
  Serial.begin(9600);

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
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
  // Make a request to OpenWeatherMap API
  WiFiClient client;
  const char* host = "api.openweathermap.org";
  const char* apiKey = "Your_API_Key";
  String url = "/data/2.5/weather?q=Patchogue,us&units=imperial&appid=" + String(apiKey); //note your city and units will be different

  if (!client.connect(host, 80)) {
    Serial.println("Connection failed");
    return;
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

  // Parse the response
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);

  float temperature = doc["main"]["temp"];
  float humidity = doc["main"]["humidity"];
  float pressure = doc["main"]["pressure"];

  // Display the values on the OLED screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Temp: ");
  display.print(temperature);
  display.print("F");

  display.setCursor(0,10);
  display.print("Humidity: ");
  display.print(humidity);
  display.print("%");

  display.setCursor(0,20);
  display.print("Pressure: ");
  display.print(pressure);
  display.print("M");

  display.display();

  // Wait for 10 seconds before making another request
  delay(10000);
}
