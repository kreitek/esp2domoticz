// *************************************************************
// esp2domoticz
// Lhrod: anphora@gmail.com
// ESP8266 send data sensor at domoticz amoung wifi conection
// 
// Used pin:
// 	D3 DHT22 Humidity and temperature
// **************************************************************
#include <ESP8266WiFi.h>

#include <DHT.h>

#define DEBUG

#define DHTPin D3                                           // what digital pin we're connected to
#define DHTTYPE DHT22                                       // DHT 22  (AM2302), AM2321

#ifdef DEBUG                                                // Debug mode?
#define sPrint(a) Serial.print(a)                           // Display all debug information
#define sPrintln(a) Serial.println(a)
#else
#define sPrint(a)                                           // No compile nothing debug
#define sPrintln(a) 
#endif

// Defines for diferent ranges of times in milliseconds
#define ONE_MIN_IN_MILLIS 60000
#define FIVE_MIN_IN_MILLIS 300000
#define TEN_MIN_IN_MILLIS 600000
#define THIRTY_MIN_IN_MILLIS 1800000
#define TIME_BETWEEN_UPDATE_SENSORS TEN_MIN_IN_MILLIS       // Selected time between updates sensors

// Put here your data network user/pass
const char* ssid = "";
const char* password = "";

// Domoticz communication layer
#define DOMOTICZ_CMD_WORD "cmd"
const char *domoticz_server = "192.168.0.205";              // Your domoticz ip address
int port = 8080;                                            // Your domoticz port
// If you use the basic authentification in domoticz you'll need code in base64 the user pass on next line how user:pass
#define AUTH_STRING "Authorization: Basic <put the pair user:password on base64 here>"
#define DHT_IDX 12                                          // Put here the identifier assigned by domoticz at your remote sensor

DHT dht(DHTPin, DHTTYPE);

WiFiClient client;

long time2UpdateSensors;                                    // This manage the time between updates

void setup(void){
#ifdef DEBUG
  Serial.begin(9600);
#endif
  dht.begin();
  time2UpdateSensors = 0;
    
  IPAddress staticIp(192, 168, 0, 210);                    // Static ip for your sensor
  
  WiFi.config(staticIp, IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  WiFi.begin(ssid, password);

  sPrintln("");
  while (WiFi.status() != WL_CONNECTED) {                 // Wait for connection
    delay(500);
    sPrint(".");
  }
  sPrintln("");
  sPrint("Connected to ");
  sPrint(ssid);
  sPrint("IP address: ");
  sPrintln(WiFi.localIP());
}

/*
 * Established conection with domoticz server and send Rest head petition
 */
boolean domoticzSendHead(int idx, String param)
{
  if (client.connect(domoticz_server, port)) {
    sPrintln("Connect domoticz");
    client.print("GET /json.htm?type=command&param="+param+"&idx="+String(idx));
    sPrint("GET /json.htm?type=command&param="+param+"&idx="+String(idx));
    return true;
  }
  return false;
}

/*
 * Send a simple Rest command with a IDX and one param
 */
void domoticzSend(int idx, String param)
{
  if (domoticzSendHead(idx, param)) {
    domoticzSendFoot();
  }
}

/*
 * Send a Rest command with a IDX, param and a key and value for update on domoticz
 */
void domoticzSend(int idx, String param, String key, String val)
{
  if (domoticzSendHead(idx, param)) {
    client.print("&" + key + "=");
    sPrint("&" + key + "=");
    client.print(val);
    sPrint(val);
    domoticzSendFoot();
  }
}

/*
 * Send a complex Rest command with a IDX, param and a twice of keys and values for update on domoticz
 */
void domoticzSend(int idx, String param, String key1, String val1, String key2, String val2)
{
  if (domoticzSendHead(idx, param)) {
    client.print("&" + key1 + "=");
    sPrint("&" + key1 + "=");
    client.print(val1);
    sPrint(val1);
    client.print("&" + key2 + "=");
    sPrint("&" + key2 + "=");
    client.print(val2);
    sPrint(val2);
    domoticzSendFoot();
  }
}

/*
 * Send end of Rest petition and close conection with DomoticZ server
 */
void domoticzSendFoot()
{
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.print(domoticz_server);
    client.print(":");
    client.println(port);
    client.println(AUTH_STRING);
    client.println("User-Agent: Arduino-ethernet");
    client.println("Connection: close");
    client.println();
    client.stop();

    sPrintln(" HTTP/1.1");
    sPrint("Host: ");
    sPrint(domoticz_server);
    sPrint(":");
    sPrintln(port);
    sPrintln(AUTH_STRING);
    sPrintln("User-Agent: Arduino-ethernet");
    sPrintln("Connection: close");
    sPrintln();
}

/*
 * Update DHT sensor info and send to DomoticZ server amoung of rest api DomoticZ
 */
void updateInfoDHT(DHT dht, int idx)
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (isnan(h) || isnan(t)) {
      sPrintln("Failed to read from DHT sensor!");
    }
    else
    {
      String hum_stat = "0";
      if (h > 60.0)
        hum_stat = "3";
      else if (h < 25.0)
        hum_stat = "2";
      else if ((h > 45.0) && (h < 55.0))
        hum_stat = "1";
      String sT = String(t);
      String sH = String(h);

      sPrintln("Temp: " + sT + ", Hum: " + sH);
      domoticzSend(idx, "udevice", "nvalue", "0", "svalue", sT + ";" + sH + ";" + hum_stat); // Temperature and humidity sensor
    }
}

/*
 * Main loop: test if it's time to update sensor and update if true
 */
void loop(void){
  if (millis() >= time2UpdateSensors)
  {
    updateInfoDHT(dht, DHT_IDX);

    time2UpdateSensors = millis() + TIME_BETWEEN_UPDATE_SENSORS;
  }
}
