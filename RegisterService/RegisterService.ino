
/*
   ArrowheadESP Basic example by Szvetlin Tanyi <szvetlin@aitia.ai>
   Connects to local network.
   Loads SSL certificates, then registers a temperature service in Service Registry.

*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArrowheadESP.h>
#include <DHTesp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

#define SENSOR_PIN D7
#define SERVICE_URI "/temperature"

// Temperature variables
float temperature = 0.0;
String tempval = (String)temperature;

// DHT11 sensor
DHTesp dht;

// NTP
const long utcOffsetInSeconds = 3600;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time1.google.com", utcOffsetInSeconds, 60000);
unsigned long epochTime = 0;

ArrowheadESP Arrowhead;

const char* systemName = "myesp2"; // name of the system, must match the common name of the certificate
int port = 8888; // doesn't really matter what number we type in, since it won't listen on it anyway
const char* publicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAszAI5Ly2ubjWVfMhUw/u" \
                        "FMd3dCW2upvjA8vp7wHBEVoqLUbW51sC5RmKwXuyO7HFtJiQOkOPWE5/QeEtaZPd" \
                        "i5bEiC2EEwjdff9be4Ys5z5KXtdRwTwGA48YhFzhzXNJSzB9Kk96SS7oZEk/UO0K" \
                        "hceE74HG0QzHbp/rV50MVHIvXCN8tOMXhJH4RATVkmkVNjffw3GJdlI4owhlDmpF" \
                        "kSb4zfeXw3Z3xnpgt2B9ZDmqEROXxb9TT3QzK+U8DSVunIBoEL5kWvo94zjuut+h" \
                        "aPLGlRPRPcUn/8npx828nob+GxJQbZ3QARQAg0IdbZetwJvp5TIYcpR+7cs4mGce" \
                        "owIDAQAB";


// This function will handle the incoming service request (return with the current temperature in SenML format).
void handleServiceRequest() {
  
    //build the SenML format
    StaticJsonDocument<500> root;
    root["bn"]  = Arrowhead.getArrowheadESPFS().getProviderInfo().systemName;
    root["t"]  = epochTime;
    root["bu"]  = "celsius";
    root["ver"] = 1;
    JsonArray e = root.createNestedArray("e");
    JsonObject meas = e.createNestedObject();
    meas["n"] = Arrowhead.getArrowheadESPFS().getProviderInfo().systemName;
    meas["sv"] = tempval;

    String response;
    serializeJson(root, response);
    Arrowhead.getWebServer().send(200, "application/json", response); // or use getSecureWebServer()

    // Generate the prettified JSON and send it to the Serial port.
    //
    serializeJsonPretty(root, Serial);
    // Start a new line
    Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Arrowhead.getArrowheadESPFS().loadConfigFile("netConfig.json"); // loads network config from file system
  Arrowhead.getArrowheadESPFS().loadSSLConfigFile("sslConfig.json"); // loads ssl config from file system
  Arrowhead.getArrowheadESPFS().loadProviderConfigFile("providerConfig.json"); // loads provider config from file system
  

  // Set the Address and port of the Service Registry.
  Arrowhead.setServiceRegistryAddress(
    Arrowhead.getArrowheadESPFS().getProviderInfo().serviceRegistryAddress,
    Arrowhead.getArrowheadESPFS().getProviderInfo().serviceRegistryPort
  );

  //Arrowhead.useSecureWebServer(); // call secure configuration if you plan to use secure web server

  bool startupSuccess = Arrowhead.begin(); // true of connection to WiFi and loading Certificates is successful
  if (startupSuccess) {
    String response = "";
    int statusCode = Arrowhead.serviceRegistryEcho(&response);
    Serial.print("Status code from server: ");
    Serial.println(statusCode);
    Serial.print("Response body from server: ");
    Serial.println(response);
    Serial.println("############################");
    Serial.println();

    String serviceRegistryEntry = "{\"endOfValidity\":\"2023-12-05T11:59:59Z\",\"interfaces\":[\"HTTP-SECURE-SENML\"],\"providerSystem\":{\"address\":\" " + Arrowhead.getIP() + "\",\"authenticationInfo\":\"" + publicKey + "\",\"port\":" + port + ",\"systemName\":\"" + systemName + "\"},\"secure\":\"CERTIFICATE\",\"serviceDefinition\":\"temperature\",\"serviceUri\":\"/\",\"version\":1}";

    response = "";
    statusCode = Arrowhead.serviceRegistryRegister(serviceRegistryEntry.c_str(), &response);
    Serial.print("Status code from server: ");
    Serial.println(statusCode);
    Serial.print("Response body from server: ");
    Serial.println(response);
    Serial.println("############################");
    Serial.println();
  }
  
  Arrowhead.getWebServer().on(SERVICE_URI, handleServiceRequest); // or use getSecureWebServer()
  Arrowhead.getWebServer().begin(port); // or use getSecureWebServer()

  timeClient.begin();
  dht.setup(SENSOR_PIN, DHTesp::DHT11); // Connect DHT sensor
  
}


void loop() {
  Arrowhead.loop(); // keep network connection up
  // put your main code here, to run repeatedly:

  delay(dht.getMinimumSamplingPeriod());
  temperature = dht.getTemperature();
  if(!isnan(temperature)) {
    tempval = (String)temperature;
  }

  timeClient.update();
  epochTime = timeClient.getEpochTime();

  delay(1000);

  yield();
}
