
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LCD_I2C.h>
LCD_I2C lcd(0x27);
// ======= Deklarasi Pin =======
#define water_flow_topic "sensor/water_flow_unit1"
#define water_consume_topic "sensor/water_consume_unit1"
#define SENSOR D4
// =============================
// Update these with values suitable for your network.

const char* ssid = "Abdul";
const char* password = "1sampai8";
const char* mqtt_server = "192.168.43.52";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

void IRAM_ATTR pulseCounter(){
  pulseCount++;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // Setting sensor to 0 after reset
  if (strcmp(topic,"reset/water_consume")==0){
    totalMilliLitres = 0;
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Berhasil Reconnect");
      // ... and resubscribe
      client.subscribe("reset/water_consume");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(SENSOR, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  lcd.begin();
  lcd.backlight();
  lcd.print("Abdul"); 
  lcd.setCursor(0, 1); 
  lcd.print("18570004"); 
  delay(3000); 
  lcd.clear(); 
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);


}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) /
    calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));
    Serial.print("L/min");
    Serial.print("\t");
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
    // ------- Tampilan LCD ---------
    lcd.clear();
    delay(10);
    lcd.setCursor(0, 0); 
    lcd.print("D:"); 
    lcd.print(int(flowRate)); 
    lcd.setCursor(6, 0); 
    lcd.print("L/Min"); 
    lcd.setCursor(0, 1); 
    lcd.print("V: "); 
    lcd.print(int(totalMilliLitres));
    lcd.print("mL/ "); 
    lcd.print(int(totalMilliLitres/ 1000));
    lcd.print("L"); 
//    if  (totalMilliLitres >= 5000){
//      totalMilliLitres = 0;
//    }
    // ------- End Tampilan LCD ---------

    client.publish(water_flow_topic, String(flowRate).c_str(), true);
    client.publish(water_consume_topic, String(totalMilliLitres).c_str(), true);

    
  }




  
  
}
