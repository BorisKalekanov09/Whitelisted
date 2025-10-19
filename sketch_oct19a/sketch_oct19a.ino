
const int sensorPin = 25;

void setup() {
  Serial.begin(9600);         
  pinMode(sensorPin, INPUT);  
}

void loop() {
  if (digitalRead(sensorPin)) {              
    Serial.println("Detected vibration...");  
  } 
  else {
    Serial.println("..."); 
  }

  delay(100);
}