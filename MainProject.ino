#include "ESP8266WiFi.h"
#include "DHT.h"
DHT dht2(2,DHT11);

// WiFi parameters to be configured
const char* ssid = "MTNL";
const char* password = "AttOpmn63";

#define INTERVALO_ENVIO_THINGSPEAK  30000 // thingspeak environment

//global constants and variables
char ThingSpeakAPIAddress[] = "api.thingspeak.com";  //ThingSpeak API Address
String ThingSpeakWrittenKey = "9S48Z6FBHLI03KEV";   //ThingSpeak Written Key
long lastConnectionTime; 
WiFiClient client;

//prototypes
void SendInformationThingspeak(String StringData);
void MakeWiFiConnection(void);
float DoReadingHumidity(void);

// Implementation - Function: sends information to ThingSpeak, Parameters: String with the information to be sent, Return: none
void SendInformationThingspeak(String StringData)
{
  if (client.connect(ThingSpeakAPIAddress, 80))
  {
    //makes the HTTP request to ThingSpeak
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+ThingSpeakWrittenKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(StringData.length());
    client.print("\n\n");
    client.print(StringData);
    lastConnectionTime = millis();
    Serial.println("- Information sent to ThingSpeak!");
  }
}

// Function: makes WiFI connection, Parameters: none, Return: none
void MakeWiFiConnection(void)
{
  client.stop();
  Serial.println("Connecting to the WiFi network...");
  Serial.println();  
  delay(1000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully!");  
  Serial.println("IP obtained: ");
  // Serial.println(WiFi.localIP());
  delay(1000);
}

float DoReadingHumidity(void)
{
  int ValueADC;
  float HumidityPercent;
  ValueADC = analogRead(0);
  Serial.print("ADC Value: ");
  Serial.println(ValueADC);

  HumidityPercent = 100 * ((978-(float)ValueADC) / 978);
  Serial.print("Soil Humidity: ");
  Serial.print(HumidityPercent);
  Serial.println("%");
  return HumidityPercent;
}

void setup()
{  
  Serial.begin(9600);
  lastConnectionTime = 0;
  MakeWiFiConnection();
  Serial.println("IoT plant with ESP8266 NodeMCU");
  dht2.begin();
}

//loop principal
void loop()
{
  float humidity = dht2.readHumidity(); /* Get humidity value */
  float temperature = dht2.readTemperature(); /* Get temperature value */
  Serial.println("Status\tAir Humidity (%)\tTemperature(C)");
  Serial.print("\t");
  Serial.print((dht2.readHumidity()));
  Serial.print("\t\t\t");
  Serial.print((dht2.readTemperature()));
  Serial.println();
  
  float HumidityPercentRead;
  int TruncatedPercentHumidity;
  char FieldHumidity[11];
  // char FieldTemperature[1];
  if (client.connected())
  {
    client.stop();
    Serial.println("- Disconnected from ThingSpeak");
    Serial.println();
  }
  HumidityPercentRead = DoReadingHumidity();
  TruncatedPercentHumidity = (int)HumidityPercentRead;
  if(!client.connected() && (millis() - lastConnectionTime > INTERVALO_ENVIO_THINGSPEAK))
  {
    sprintf(FieldHumidity,"field1=%d",TruncatedPercentHumidity);
    // sprintf(FieldTemperature,"field2=%d",temperature);
    SendInformationThingspeak(FieldHumidity);
    // SendInformationThingspeak(FieldTemperature);
  }

  delay(15000);
}