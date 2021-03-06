#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Set these to your desired credentials.
const char* ssid = "M&T2.4G";
const char* password = "seinaovei";

const char* host = "http://625a33d1.ngrok.io/iot";

const char* humTempType = "HumTemp";
const char* touchType = "Touch";
const char* ldrType = "Luminosity";

// DHT Sensor
int DHpin = D0;
byte dat [5];
byte readData () {
  byte data;
  for (int i = 0; i < 8; i ++) {
    if (digitalRead(DHpin) == LOW) {
      while (digitalRead(DHpin) == LOW);
      delayMicroseconds(30);
      if (digitalRead(DHpin) == HIGH)
        data |= (1 << (7-i));
      while (digitalRead(DHpin) == HIGH);
     }
  }
  return data;
}

int makeHttpRequest(char *sensorType, float arr[]) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected");
    
    if  (sensorType == humTempType) {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& body = jsonBuffer.createObject();
      body["sensorType"] = "HumTemp";
      body["humidity"] = arr[0];
      body["temperature"] = arr[1];

      sendRequest(body);
    } else if (sensorType == touchType) {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& body = jsonBuffer.createObject();
      body["sensorType"] = "Touch";
      body["touched"] = "true";

      sendRequest(body);
    } else if (sensorType == ldrType) {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& body = jsonBuffer.createObject();
      body["sensorType"] = "Luminosity";
      body["lumens"] = arr[0];

      sendRequest(body);
    }
  } else {
 
    Serial.println("Error in WiFi connection");
    return 0;
  }  
}

void sendRequest(JsonObject& body) {
  HTTPClient http;
  http.begin(host);
  http.addHeader("Content-Type", "application/json");
    
  char JSONmessageBuffer[300];
  body.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);
  
  int httpCode = http.POST(JSONmessageBuffer);
  String payload = http.getString();
  
  Serial.println(httpCode);
  Serial.println(payload);

  http.end();
}

void getLdrVoltage() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  
  // print out the value you read:
  Serial.println(voltage);

  float arr[1];
  arr[0] = voltage;
  
  makeHttpRequest("Luminosity", arr);
}

void getTempAndHumidity() {
  String hum;
  String temp;

  digitalWrite(DHpin, LOW);
  delay(30);
 
  digitalWrite(DHpin, HIGH);
  delayMicroseconds(40);
 
  pinMode(DHpin, INPUT);
  while (digitalRead(DHpin) == HIGH);
  delayMicroseconds(80);
  if (digitalRead(DHpin) == LOW);
  delayMicroseconds(80);
 
  for (int i = 0; i < 4; i ++)
    dat[i] = readData();
 
  pinMode(DHpin, OUTPUT);
  digitalWrite(DHpin, HIGH);

  hum = String(dat[0]) + String(".") + String(dat[1]);
  temp = String(dat[2]) + String(".") + String(dat[3]);

  float arr[2];
  arr[0] = hum.toFloat();
  arr[1] = temp.toFloat();
  
  makeHttpRequest("HumTemp", arr);
  
}

// -----------------------------------------------------------------------------------------------------------------------

// Metal Sensor
int touchButton = D7;
int val;

void getTouch() {
  val = digitalRead(touchButton);
  
  if (val == HIGH) {
    Serial.println("Touched");
    float arr[1];
    arr[0] = val;
    makeHttpRequest("Touch", arr);
  }
}

// -----------------------------------------------------------------------------------------------------------------------
 
void setup () {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  
  pinMode(DHpin, OUTPUT);
  pinMode(touchButton, INPUT);
}

void loop () {
  if (WiFi.status() == WL_CONNECTED) {
    getTempAndHumidity();
    getTouch();
    getLdrVoltage();
  } else {
    Serial.println("No internet...");
  }
  
  delay(10000);
}
