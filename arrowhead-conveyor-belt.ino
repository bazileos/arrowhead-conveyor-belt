/*
 * Arrowhead adapter for conveyor belt by and Balazs Riskutia <balazs.riskutia@outlook.hu>
 * Registers and sets up endpoints for services needed to utilize the functionalities of a basic conveyor belt.
 */

#include <NTPClient.h>
#include <ArduinoJson.h>
#include <ArrowheadESP.h>
#include <math.h>

// Server and service parameters
// TODO: change params, if needed!
#define STEP_PIN 16
#define DIR_PIN 14
#define SERVER_PORT 8080 

// speficy shaft radius here (in millimeters) to accurately calculate the movement distance of the belt
int shaftRadius = 6;
float transRatio = 0.0314*shaftRadius;

// NTP
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time1.google.com", utcOffsetInSeconds, 60000);
unsigned long epochTime = 0;

ArrowheadESP Arrowhead;

void handleTransportWithConveyor() {
  
  int signedDistance = 0;

  // check if the provided coordinates are valid in format
  if(Arrowhead.getWebServer().arg("signed-distance") == "") {
      sendErrorResponse("Missing signed-distance as a parameter.");
      return;
   }
   else {
    signedDistance = Arrowhead.getWebServer().arg("signed-distance").toInt();
    if(signedDistance == 0) {
      sendErrorResponse("Signed-distance parameter is 0 or it is invalid.");
      return;
    }
   }

  // MOVE BELT

  // set direction
  if(signedDistance > 0) digitalWrite(DIR_PIN,LOW);
  else digitalWrite(DIR_PIN,HIGH);

  // calculate number of pulses
  int pulseNum = round(abs(signedDistance)/transRatio);
  Serial.print("Number of pulses: ");
  Serial.println(pulseNum);

  // make pulses
  for(int i = 0; i < pulseNum; i++) {
    digitalWrite(STEP_PIN,HIGH); 
    delay(2); 
    digitalWrite(STEP_PIN,LOW); 
    delay(2);
  }

  // return with success
  StaticJsonDocument<150> root;
  String response;
  root["status"] = "success";
  root["timestamp"] = epochTime;
  serializeJson(root, response);
  Arrowhead.getWebServer().send(200, "application/json", response);
  
}

void handleTransportWithConveyorEndToEnd() {

  String beltDirection = Arrowhead.getWebServer().arg("direction");
  // check if the provided direction is valid in format
  if(beltDirection == "") {
    sendErrorResponse("Missing direction as a parameter.");
    return;
  }

  // MOVE BELT

  // set direction
  if(beltDirection == "forward") digitalWrite(DIR_PIN,LOW);
  if(beltDirection == "backward" || beltDirection == "backwards") digitalWrite(DIR_PIN,HIGH);

  // make pulses
  for(int i = 0; i < 3600; i++) {
    digitalWrite(STEP_PIN,HIGH); 
    delay(2); 
    digitalWrite(STEP_PIN,LOW); 
    delay(2);
  }

  // return with success
  StaticJsonDocument<150> root;
  String response;
  root["status"] = "success";
  root["timestamp"] = epochTime;
  serializeJson(root, response);
  Arrowhead.getWebServer().send(200, "application/json", response);
}

void sendErrorResponse(String message) {
  StaticJsonDocument<150> root;
  String response;
  root["status"] = "error";
  root["message"] = message;
  root["timestamp"] = epochTime;
  serializeJson(root, response);
  Arrowhead.getWebServer().send(400, "application/json", response); // return with error response
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // set step and direction pins as outputs
  pinMode(STEP_PIN,OUTPUT); 
  pinMode(DIR_PIN,OUTPUT);
  
  Arrowhead.getArrowheadESPFS().loadConfigFile("netConfig.json"); // loads network config from file system
  Arrowhead.getArrowheadESPFS().loadSSLConfigFile("sslConfig.json"); // loads ssl config from file system
  Arrowhead.getArrowheadESPFS().loadProviderConfigFile("providerConfig.json"); // loads provider config from file system

  // Set the Address and port of the Service Registry.
  Arrowhead.setServiceRegistryAddress(
    Arrowhead.getArrowheadESPFS().getProviderInfo().serviceRegistryAddress,
    Arrowhead.getArrowheadESPFS().getProviderInfo().serviceRegistryPort
  );

  bool startupSuccess = Arrowhead.begin(); // true of connection to WiFi and loading Certificates is successful
  if(startupSuccess){

    // Check if service registry
    String response = "";
    int statusCode = Arrowhead.serviceRegistryEcho(&response);
    Serial.print("Status code from server: ");
    Serial.println(statusCode);
    Serial.print("Response body from server: ");
    Serial.println(response);

    String serviceRegistryEntry1 = "{\"endOfValidity\":\"2021-12-05 12:00:00\",\"interfaces\":[\"HTTP-INSECURE-JSON\"],\"providerSystem\":{\"address\":\" "+ Arrowhead.getIP() +"\",\"authenticationInfo\":\""+ Arrowhead.getArrowheadESPFS().getSSLInfo().publicKey +"\",\"port\":"+ SERVER_PORT +",\"systemName\":\""+ Arrowhead.getArrowheadESPFS().getProviderInfo().systemName +"\"},\"serviceDefinition\":\"transport-with-conveyor\",\"serviceUri\":\"/transport-with-conveyor\",\"version\":1}";
    String serviceRegistryEntry2 = "{\"endOfValidity\":\"2021-12-05 12:00:00\",\"interfaces\":[\"HTTP-INSECURE-JSON\"],\"providerSystem\":{\"address\":\" "+ Arrowhead.getIP() +"\",\"authenticationInfo\":\""+ Arrowhead.getArrowheadESPFS().getSSLInfo().publicKey +"\",\"port\":"+ SERVER_PORT +",\"systemName\":\""+ Arrowhead.getArrowheadESPFS().getProviderInfo().systemName +"\"},\"serviceDefinition\":\"transport-with-conveyor-end-to-end\",\"serviceUri\":\"/transport-with-conveyor-end-to-end\",\"version\":1}";  

    response = "";

    // register transport-with-conveyor service
    statusCode = Arrowhead.serviceRegistryRegister(serviceRegistryEntry1.c_str(), &response);
    Serial.print("Status code from server: ");
    Serial.println(statusCode);
    Serial.print("Response body from server: ");
    Serial.println(response);

    // register transport-with-conveyor-end-to-end service
    statusCode = Arrowhead.serviceRegistryRegister(serviceRegistryEntry2.c_str(), &response);
    Serial.print("Status code from server: ");
    Serial.println(statusCode);
    Serial.print("Response body from server: ");
    Serial.println(response);
  }

  Arrowhead.getWebServer().on("/transport-with-conveyor", handleTransportWithConveyor);
  Arrowhead.getWebServer().on("/transport-with-conveyor-end-to-end", handleTransportWithConveyorEndToEnd);
  Arrowhead.getWebServer().begin(SERVER_PORT);

  timeClient.begin();
} 


void loop() {
  Arrowhead.loop(); // keep network connection up
  // put your main code here, to run repeatedly:

  timeClient.update();
  epochTime = timeClient.getEpochTime();
  
  yield();
}
