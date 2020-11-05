#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>

#define RELAY D6
#define LED_MERAH D2
#define LED_HIJAU D1
#define BUZZ D3
#define SENSOR_PIN A0
int NILAI_SENSOR;
String STATUS_SOIL;

const char *ssid = "Achlis Djafar";
const char *password = "kartika123";
const char *mqtt_server = "192.168.43.38";
const char *mqtt_user = "achlisdjafar";
const char *mqtt_password = "sembarang";

char pesanPUB[256];
String pesanSUB;

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup()
{
  SPI.begin();
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(BUZZ, LOW);
  
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Sedang koneksi ke ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Berhasil terhubung ke WiFi");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address ");
  Serial.println(WiFi.macAddress());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  pesanSUB ="";
  StaticJsonDocument<200> subjson;     
  Serial.print("Pesan masuk [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    pesanSUB += (char)payload[i];
  }
  Serial.println();
  deserializeJson(subjson, pesanSUB);
  String status_data = subjson["status"];
  String mac_addr = subjson["mac_address"];
  Serial.println();
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Koneksi ke server MQTT ...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password))
    {
      Serial.println(" koneksi berhasil");
      //client.publish("rfid.out", "Halo SMK Kartika");
      client.subscribe("soilm");
    }
    else
    {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" konek ulang dalam 5 detik");
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

   NILAI_SENSOR = map(analogRead(SENSOR_PIN), 0, 1023, 100, 0);
   Serial.print(NILAI_SENSOR);
   if(NILAI_SENSOR < 50)
   {
      STATUS_SOIL = "KERING";
      digitalWrite(BUZZ, HIGH);
      delay(200);
      digitalWrite(BUZZ, LOW);
      delay(200);
      digitalWrite(BUZZ, HIGH);
      delay(200);
      digitalWrite(BUZZ, LOW);
      delay(200);
      digitalWrite(LED_HIJAU, LOW);
      digitalWrite(LED_MERAH, HIGH);
      digitalWrite(RELAY, LOW);
   }else{
      STATUS_SOIL = "BASAH ";
      digitalWrite(LED_HIJAU, HIGH);
      digitalWrite(LED_MERAH, LOW);
      digitalWrite(BUZZ, LOW);
      digitalWrite(RELAY, HIGH);
   }
   
   sprintf(msg, "%d", NILAI_SENSOR);
   Serial.print("Publish pesan: ");
   Serial.println(msg);
   client.publish("soilm", msg);
}
