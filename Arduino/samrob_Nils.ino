#include "EspMQTTClient.h" 

typedef enum States {
  Init,
  Drive,
  Stop
  }; //States

States St; //Defines States as St

const byte pwm_b=4;
const byte dir=2;
const byte interruptPin=14;
int AIValue=0;
int oldAIValue=0;
unsigned long lastmillis = 0;
int samplePeriod=3000;
bool drive=false;
volatile float count = 0;
float lastlenght = 0;
float lenght = 0;
float fast = 0;
float deltalenght = 0;
float borvarde=0;
int Speed=300;
long deltamillis=0;
float totalFault=0; 
byte sent=1;



void onConnectionEstablished(); //When mqtt connects

EspMQTTClient client(
 "ABB_Indgym_Guest",           // Wifi ssid
  "Welcome2abb",           // Wifi password
  "maqiatto.com",  // MQTT broker ip
  1883,             // MQTT broker port
  "nils.borg@abbindustrigymnasium.se",            // MQTT username
  "dodoärbra",       // MQTT password
  "microdator",          // Client name
  onConnectionEstablished, // Connection established callback
  true,             // Enable web updater
  true              // Enable debug messages
);

void onConnectionEstablished() 
{
  client.subscribe("nils.borg@abbindustrigymnasium.se/samrob_onoff", [] (const String &payload) 
  {
    Serial.println(payload);
    if (payload=="change"){
      drive=!drive; //subscribe to onoff topic, whenever we receive a payload containing "change" invert drive  
    }
  });
  client.subscribe("nils.borg@abbindustrigymnasium.se/samrob_speed", [] (const String &payload)
  {
    Serial.println(payload);
    borvarde=payload.toFloat(); //Sets borvarde to the float representative of our payload
 
  });
  
  client.publish("nils.borg@abbindustrigymnasium.se/samrob", "Älg");  //See if the car has connected
}


ICACHE_RAM_ATTR void Add(){
  count+=0.25;
}

void changeSpeed() {
  float pProp=0.75;
  float iProp=0.004;
  if(deltamillis>100) { //if 100 seconds have passed since the last time the funtion ran successfully
    float fault=borvarde-fast; //error
    Serial.println("Error: "+ String(fault));
    totalFault+=fault*deltamillis/1000;//integral of error/second
    Speed+=(fault*pProp)+(totalFault*iProp);//PI-regulation
    if (Speed>1024) {
      Speed=1024;
    }
    else if (Speed<300){
      Speed=300;
      }//resets speed if its too high or low
  }
}
 
void getSpeed() {
  long currentmillis = millis();
  deltamillis = currentmillis-lastmillis; //diffrence in time since the last time the function ran correctly
  if(deltamillis>100) //if its over 100
  {
    lenght = (count/50)*11.6; //
    deltalenght = lenght-lastlenght;
    Serial.println("Deltalength:" + String(deltalenght));
    fast = (deltalenght/deltamillis)*1000;
    Serial.print(fast*1000);
    Serial.println("cm/s");
    if (sent=3){
      client.publish("nils.borg@abbindustrigymnasium.se/samrob_Nils",String(fast));
      sent=0;
    }
    sent+=1;
    lastlenght = lenght;
    lastmillis = currentmillis;
  }
}



void setup() {
  Serial.begin(9600);
  pinMode(pwm_b, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(interruptPin, INPUT);
  Serial.println("Starting");
  attachInterrupt(digitalPinToInterrupt(interruptPin), Add, CHANGE); //Whenever a change in current is detected on the interrupt pin, run the Add function
  St=Init;
}

void loop() {
  switch(St){
    case Init:
      Serial.println("Starting in 5 sec");
      client.loop();
      delay(5000);
      St=Drive;
    break;
    case Drive:
      analogWrite(pwm_b, Speed);
      digitalWrite(dir, HIGH);
      client.loop();
      Serial.println("Fast:"+ String(fast));
      Serial.println("drive");
      Serial.println(count);
      getSpeed();
      changeSpeed();  
      if (drive==false){
        Serial.println("stop");
        St=Stop;
      }
    break;
    case Stop:
      Speed=0;
      digitalWrite(pwm_b, LOW);
      client.loop();
      Serial.println("Stop");
      if (drive==true){
        totalFault=0;
        Serial.println("drive");
        St=Drive;
        lastmillis=millis();
        lastlenght=(count/50)*11.6;  
      }
    break;
  }
}
