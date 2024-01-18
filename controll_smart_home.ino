#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"

// wifi stuff
char ssid[] = SECRET_SSID;   
char pass[] = SECRET_PASS;    
int status = WL_IDLE_STATUS;

// wizz stuff
WiFiUDP udp;
unsigned int localUdpPort = 4210;
unsigned int wizPort = 38899;

IPAddress dinerLampe(192, 168, 1, 138);
IPAddress schreibtischLicht(192, 168, 1, 101);
IPAddress fernsehLicht(192, 168, 1, 146);
IPAddress ufo(192, 168, 1, 100);

// initialize default values
int switchStateSet = 0;

int temperatureSet = 0;

int brightnessDinerLampeSet = 0;
int brightnessSchreibtischLichtSet = 0;
int brightnessFernsehLichtSet = 0;
int brightnessUfoSet = 0;


void setup() {

  // Initialize the digital pin as an input
  pinMode(2, INPUT_PULLUP);
  
  Serial.begin(9600);
  while (!Serial) {}
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  // Starting UDP
  udp.begin(localUdpPort);
}

void loop() {
  // Read the state of the switch
  int switchState = digitalRead(2);
  if (switchState != switchStateSet) {
    switchStateSet = switchState;
    Serial.println(switchState);
    if (switchState == 0) {
      turnOff(dinerLampe);
      turnOff(fernsehLicht);
      turnOff(schreibtischLicht);
      turnOff(ufo);
    }
    else {
      turnOn(dinerLampe);
      turnOn(fernsehLicht);
      turnOn(schreibtischLicht);
      turnOn(ufo);
    }
  } 

  // Temperatur
  int temperature = analogRead(A1) * 100 / 1023;
  if (temperature != temperatureSet && switchState == 1) {
    temperatureSet = temperature;
    setTemperature(temperature * 40 + 2000);
    Serial.println("Neue Temperatur:");
    Serial.println(temperature * 40 + 2000);
  }
    
  // DinerLampe
  int brightnessDinerLampe = analogRead(A2) * 100 / 1023;
  if (brightnessDinerLampeSet != brightnessDinerLampe && switchState == 1) {    
    brightnessDinerLampeSet = brightnessDinerLampe;
    setBrightness(brightnessDinerLampe, dinerLampe);
    Serial.println("Neue Helligkeit der Dinerlampe:");
    Serial.println(brightnessDinerLampe);
  }
  
  // DinerLampe
  int brightnessSchreibtischLicht = analogRead(A3) * 100 / 1023;
  if (brightnessSchreibtischLichtSet != brightnessSchreibtischLicht && switchState == 1) {    
    brightnessSchreibtischLichtSet = brightnessSchreibtischLicht;
    setBrightness(brightnessSchreibtischLicht, schreibtischLicht);
    Serial.println("Neue Helligkeit der Schreibtischlichts:");
    Serial.println(brightnessSchreibtischLicht);
  }

  // FernsehLampe
  int brightnessFernsehLicht = analogRead(A4) * 100 / 1023;
  if (brightnessFernsehLichtSet != brightnessFernsehLicht && switchState == 1) {    
    brightnessFernsehLichtSet = brightnessFernsehLicht;
    setBrightness(brightnessFernsehLicht, fernsehLicht);
    Serial.println("Neue Helligkeit der Fernsehlichts:");
    Serial.println(brightnessFernsehLicht);
  }

  // add delay
  delay(200);
}

void turnOff(int ip) {
  String command = String("{\"method\":\"setPilot\",\"params\":{\"state\":false}}");
  // Send command
  udp.beginPacket(ip, wizPort);
  udp.write(command.c_str());
  udp.endPacket();
}

void turnOn(int ip) {
  String command = String("{\"method\":\"setPilot\",\"params\":{\"state\":true}}");
  // Send command
  udp.beginPacket(ip, wizPort);
  udp.write(command.c_str());
  udp.endPacket();
}

void setBrightness(int brightness, int ip ) { 
  if (brightness == 0 && ip == dinerLampe) {
    turnOff(ip);
  }
  else {
    turnOn(ip);  
    // Create JSON command to set brightness
    String command = String("{\"method\":\"setPilot\",\"params\":{\"dimming\":" + String(brightness) + "}}");

    // Send command
    udp.beginPacket(ip, wizPort);
    udp.write(command.c_str());
    udp.endPacket();
  }
}

void setTemperature(int temperature) {
  temperature = constrain(temperature, 2000, 6000);

  String command = String("{\"method\":\"setPilot\",\"params\":{\"temp\":" + String(temperature) + "}}");

  udp.beginPacket(dinerLampe, wizPort);
  udp.write(command.c_str());
  udp.endPacket();
  udp.beginPacket(schreibtischLicht, wizPort);
  udp.write(command.c_str());
  udp.endPacket();
  udp.beginPacket(fernsehLicht, wizPort);
  udp.write(command.c_str());
  udp.endPacket();
}

