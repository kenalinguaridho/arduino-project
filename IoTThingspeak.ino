#include "Arduino.h"
#include <Ethernet.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

byte mac[] = SECRET_MAC;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 43, 47);
IPAddress myDns(192, 168, 0, 1);

EthernetClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
#define pinSensor0 A0 // MQ-2
#define pinSensor1 A1 // MQ-9
#define pinSensor2 A2 // MQ-135
#define pinSensor3 A3 // MQ-136

int RL=10;

float MQ2_m     = -0.36388, MQ2_b     = 1.29271, MQ2_Ro    = 5.88;
float MQ9_m     = -0.45796, MQ9_b     = 1.26117, MQ9_Ro    = 1.31;
float MQ135_m   = -0.40756, MQ135_b   = 0.81659, MQ135_Ro  = 3.46;
float MQ136_m   = -0.75220, MQ136_b   = 1.01838, MQ136_Ro  = 0.52;

void setup() {
  Ethernet.init(10);  // Most Arduino Ethernet hardware
  Serial.begin(9600);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
      
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  
  // MQ2 calculation to get ppm
  float MQ2_VRL = analogRead(pinSensor0);
  MQ2_VRL = MQ2_VRL * 5/1023; 
  float MQ2_Rs = ((5*RL)/MQ2_VRL)-RL; 
  float MQ2_ratio = MQ2_Rs/MQ2_Ro;  
  float MQ2_ppm = pow(10, ((log10(MQ2_ratio)-MQ2_b)/MQ2_m));
 

  // MQ9 calculation to get ppm
  
  float MQ9_VRL = analogRead(pinSensor1);
  MQ9_VRL = MQ9_VRL * 5/1023; 
  float MQ9_Rs = ((5*RL)/MQ9_VRL)-RL; 
  float MQ9_ratio = MQ9_Rs/MQ9_Ro;  
  float MQ9_ppm = pow(10, ((log10(MQ9_ratio)-MQ9_b)/MQ9_m));
 

  // MQ135 calculation to get ppm
  
  float MQ135_VRL = analogRead(pinSensor2);
  MQ135_VRL = MQ9_VRL * 5/1023; 
  float MQ135_Rs = ((5*RL)/MQ9_VRL)-RL; 
  float MQ135_ratio = MQ135_Rs/MQ135_Ro;  
  float MQ135_ppm = pow(10, ((log10(MQ135_ratio)-MQ135_b)/MQ135_m));
  

  // MQ136 calculation to get ppm
  
  float MQ136_VRL = analogRead(pinSensor3);
  MQ136_VRL = MQ136_VRL * 5/1023; 
  float MQ136_Rs = ((5*RL)/MQ136_VRL)-RL; 
  float MQ136_ratio = MQ136_Rs/MQ136_Ro;  
  float MQ136_ppm = pow(10, ((log10(MQ136_ratio)-MQ136_b)/MQ136_m));

  
  // set the fields with the values
  ThingSpeak.setField(1, MQ2_ppm);
  ThingSpeak.setField(2, MQ9_ppm);
  ThingSpeak.setField(3, MQ135_ppm);
  ThingSpeak.setField(4, MQ136_ppm);

  
  // write to the ThingSpeak channel 
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  Serial.print("LPG = ");
  Serial.println(MQ2_ppm);
  Serial.print("CO = ");
  Serial.println(MQ9_ppm);
  Serial.print("NH3 = ");
  Serial.println(MQ135_ppm);
  Serial.print("H2S = ");
  Serial.println(MQ136_ppm);

  
  delay(15000); // Wait 15 seconds to update the channel again
}
