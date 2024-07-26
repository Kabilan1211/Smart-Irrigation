#define BLYNK_TEMPLATE_ID "TMPLvoRsrLST"
#define BLYNK_TEMPLATE_NAME "Smart_Irrigation"
#define BLYNK_AUTH_TOKEN "gqxmMCa7tPvjm7EUHynYSl_YJWI8ttDk"

#define moisture_sensor A0
#define servo_motor D1
#define common_delay 60000
#define all_delay 1000
#define CLOUDCHECKER V4

unsigned long previous_millis = 0;
int moisture = 0;
int temperature = 0;
int humidity = 0;

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <Servo.h>
#define DHTPIN D2   // Define the digital pin where your DHT sensor is connected
#define DHTTYPE DHT11
Servo servo;
WiFiClient client;
BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);


char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "kabilan s";
char pass[] = "12112003@v";

void setup() {
 Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  servo.attach(servo_motor);
  dht.begin();
}

BLYNK_CONNECTED()
{
  Blynk.setProperty(CLOUDCHECKER, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(CLOUDCHECKER, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(CLOUDCHECKER, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

 void moisture_Sensor(){
  delay(2000);
   float moisture_percentage;

  moisture_percentage = ( 100.00 - ( (analogRead(moisture_sensor)/1023.00) * 100.00 ) );

  Serial.print("Soil Moisture(in Percentage) = ");
  Serial.print(moisture_percentage);
  Serial.println("%");
  moisture = moisture_percentage;
  Blynk.virtualWrite(V1, moisture);
  }

 void temp_hum_Sensor(){
  float temperature1 = dht.readTemperature();
  float humidity1 = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.print(temperature1);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity1);
  Serial.println(" %");

  temperature = temperature1;
  humidity = humidity1;
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V0, humidity);

  delay(2000);

}

int servom(){
unsigned long currentMillis1 = millis();
  if (currentMillis1 - previous_millis > common_delay){
servo.write(90);
delay(3000);
servo.write(0);
}
return 0;
}

void myTimerEvent()
{
  Blynk.virtualWrite(V5, millis() / 1000);
}
   
int prediction() {
  char path[100]; // Declare the path variable as a character array
  const char* host = "http://localhost:5000";
  int predict = 0; // Initialize predict variable

  sprintf(path, "/predict?temp=%d&hum=%d&moisture=%d", temperature, humidity, moisture);

  HTTPClient http;
  
  // Make an HTTP GET request
  Serial.println(String(host)+String(path));
  http.begin(client,String(host) + String(path));

  int httpCode = http.GET();
  Serial.println(httpCode);
  if (httpCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpCode);
    String payload = http.getString();
    Serial.println("Response: " + payload);
    // Parse the response or set predict based on the response if needed
    // For now, I'm assuming you set predict based on some logic here
    predict = some_logic_based_on_response(payload);
  } else {
    Serial.println("HTTP GET failed");
  }

  http.end();

  // Wait for some time before making another request
  delay(5000); // 5 seconds
  return predict;
}

#include <ArduinoJson.h>

int some_logic_based_on_response(const String& response) {
  StaticJsonDocument<200> doc; // Adjust the buffer size accordingly
  
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.println("Failed to parse JSON");
    return -1; // or an appropriate error code
  }
  
  int result = doc["prediction Result"]; // Assuming there's a "result" field in the JSON
  return result;
}
void loop() {
  Blynk.run();
  timer.run();
moisture_Sensor();
temp_hum_Sensor();
prediction();
if(predict == 1){
  servom();
  delay(4000);
  predict = 0;
}
}
