#include "EspMQTTClient.h"

//Defines all states used in the code
typedef enum DriveState {
  Init,
  Drive
};

DriveState State;

//Defines all global variables
const byte Pwm_b=4;
const byte dir=2;

int bor = 30;

float Speed = 350;
bool go = false;
byte sent = 0;

const float KProp = 0.75;

volatile float count = 0;
float lastlenght = 0;
float fast = 0;
float totalfault = 0;

long lastmillis = 0;
long deltamillis = 0;
long currentmillis = 0;
long lastmillis2 = 0;
long currentmillis2 = 0;

//A interrupt that occurs everytime the hallsensor sends a pulse
ICACHE_RAM_ATTR void Add(){
  count +=0.25;//Number of revolutions
};

void onConnectionEstablished();

//Login credentials for MQTT
EspMQTTClient client(
 "ABB_Indgym_Guest",           // Wifi ssid
  "Welcome2abb",           // Wifi password
  "maqiatto.com",  // MQTT broker ip
  1883,             // MQTT broker port
  "nils.borg@abbindustrigymnasium.se",            // MQTT username
  "dodoärbra",       // MQTT password
  "Adam_microdator",          // Client name
  onConnectionEstablished, // Connection established callback
  true,             // Enable web updater
  true              // Enable debug messages
);

//Function for measuring speed
void getSpeed() {
  float lenght = 0;
  float deltalenght = 0;
  
  lenght = (count/50)*11.6;//Converts number of revolutions to distance in cm
  deltalenght = lenght-lastlenght;

  //Converts cm to cm/s
  fast = deltalenght/deltamillis;
  fast *= 1000;
  
  Serial.println(fast);
  //Saves the last values for the next time speed is measured
  lastlenght = lenght;
  lastmillis = currentmillis;
  sent += 1;
}

//Funktion that changes the speed given to the motor
void changeSpeed() {
  const float KProp = 0.75;
  const float KInte = 0.001;
  float fault = 0;
  long currentmillis2 = 0;
  float deltamillis2 = 0;

  //Proportional change
  fault = bor - fast;
  Speed += fault * KProp;

  //Integral change
  currentmillis2 = millis();
  deltamillis2 = currentmillis2 - lastmillis2;
  totalfault = fault*(deltamillis2/1000);
  Speed += totalfault * KInte;
  lastmillis2 = currentmillis2;

  //Makes sure it only sends speeds that the engine can handle
  if (Speed > 1024) {
    Speed = 1024;
  }
  else if (Speed < 350) {
    Speed = 350;
  }
}


void onConnectionEstablished()
{
  //Sends a message that shows the car is connected
  client.publish("nils.borg@abbindustrigymnasium.se/samrob", "Adam car connected");

  //Either turns on or turns off the car when the car recieves a change message i the onoff topic
  client.subscribe("nils.borg@abbindustrigymnasium.se/samrob_onoff", [] (const String &payload){
    Serial.println(payload);
    if (payload == "change"){
      go = !go;
    }
    });
  //Changes the set Speed when it recieves a value from speed
  client.subscribe("nils.borg@abbindustrigymnasium.se/samrob_speed", [] (const String &payload){
    bor = payload.toInt();
  });
};


void sendSpeed(){
  //Sends data when the car has gone trough getSpeed 5 times
  if(sent = 5)
  {
    client.loop();
    //Sends the current speed to the website
    client.publish("nils.borg@abbindustrigymnasium.se/samrob_Adam", String(fast));
    sent = 0;
  }
};

void setup() {
  Serial.begin(9600);
  pinMode(Pwm_b, OUTPUT);
  pinMode(dir, OUTPUT);
  digitalWrite(Pwm_b, LOW);

  Serial.println("beging");
  pinMode(14, INPUT);
  attachInterrupt(digitalPinToInterrupt(14), Add, CHANGE);

}

void loop() {
  client.loop();
  switch (State) {
    case Init:
    //Turns off the motor
      analogWrite(Pwm_b, 0);
      Speed = 0;
      //If go has changed to True it will change state to Drive and start the car
      if (go){
        State = Drive;
        lastmillis2 = millis();
        totalfault = 0;
      }
      break;
     case Drive:
      //Changes the speed of the motor to Speed
      analogWrite(Pwm_b, Speed);
      digitalWrite(dir, HIGH);
      currentmillis = millis();
      //Calculates deltamillis and only runs the rest of the sates if more than 100 ms has passed.
      deltamillis = currentmillis-lastmillis;
      if(deltamillis>100) {
      getSpeed();
      changeSpeed();
      sendSpeed();
      }
      //Stops the car if go is False
      if (!go){
        State = Init;
      }
      break;
  }
  
}
