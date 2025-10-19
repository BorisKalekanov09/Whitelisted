#define GPS_TX 17
#define GPS_RX 16

HardwareSerial gpsSerial(2);
void setup(){
  gpsSerial.begin(9600,SERIAL_8N1,GPS_RX,GPS_TX);
  Serial.begin(115200);
  delay(500);
}

void loop(){
  while(gpsSerial.available()>0){
    Serial.print(gpsSerial.read());
  }
  delay(500);
  Serial.println("__________________________________________");
}