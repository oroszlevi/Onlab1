
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
  
}


void loop() {
  Arrowhead.loop(); // keep network connection up
  // put your main code here, to run repeatedly:

  yield();
}
