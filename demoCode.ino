#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "BluetoothSerial.h"

// ====== BLUETOOTH ======
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth not enabled! Please enable in menuconfig.
#endif

BluetoothSerial SerialBT;

// ====== L9110S MOTOR PINS ======
#define AIN1 21
#define AIN2 22
#define BIN1 25
#define BIN2 33

// ====== IR SENSORS ======
#define LEFT_SENSOR   34
#define CENTER_SENSOR 35
#define RIGHT_SENSOR  14

// ====== OTHER SENSORS ======
#define DHTPIN 5
#define DHTTYPE DHT11
#define VIB_PIN 18

Adafruit_MPU6050 mpu;
DHT dht(DHTPIN, DHTTYPE);

// ====== PID VARIABLES ======
float Kp = 15, Ki = 0, Kd = 10;
float P = 0, I = 0, D = 0, previousError = 0;
float multiP = 1, multiI = 1, multiD = 1;
int baseSpeed = 180;
bool onoff = false;

// ====== ROAD QUALITY ======
volatile int vibCount = 0;
const int SAMPLE_COUNT = 10;
float azValues[SAMPLE_COUNT];
int indexA = 0;
unsigned long lastRoadCheck = 0;
const unsigned long ROAD_INTERVAL = 1000;

// ====== MOTOR CONTROL FOR L9110S ======
void motorLeft(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    analogWrite(AIN1, speed);
    digitalWrite(AIN2, LOW);
  } else if (speed < 0) {
    digitalWrite(AIN1, LOW);
    analogWrite(AIN2, -speed);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  }
}

void motorRight(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    analogWrite(BIN1, speed);
    digitalWrite(BIN2, LOW);
  } else if (speed < 0) {
    digitalWrite(BIN1, LOW);
    analogWrite(BIN2, -speed);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }
}

void stopMotors() {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
}

// ====== INTERRUPT & STATS ======
void IRAM_ATTR vibISR() {
  vibCount++;
}

float computeStdDev(float *arr, int n) {
  float sum = 0, mean, stddev = 0;
  for (int i = 0; i < n; i++) sum += arr[i];
  mean = sum / n;
  for (int i = 0; i < n; i++) stddev += pow(arr[i] - mean, 2);
  return sqrt(stddev / n);
}

// ====== PID LINE FOLLOWING ======
void lineFollowPID() {
  int leftVal = digitalRead(LEFT_SENSOR);
  int centerVal = digitalRead(CENTER_SENSOR);
  int rightVal = digitalRead(RIGHT_SENSOR);

  int error = 0;
  if (leftVal == HIGH && centerVal == LOW && rightVal == LOW) error = -2;
  else if (leftVal == HIGH && centerVal == HIGH && rightVal == LOW) error = -1;
  else if (leftVal == LOW && centerVal == HIGH && rightVal == LOW) error = 0;
  else if (leftVal == LOW && centerVal == HIGH && rightVal == HIGH) error = 1;
  else if (leftVal == LOW && centerVal == LOW && rightVal == HIGH) error = 2;
  else if (leftVal == LOW && centerVal == LOW && rightVal == LOW) error = previousError;

  P = error;
  I += error;
  D = error - previousError;

  float Pvalue = (Kp / pow(10, multiP)) * P;
  float Ivalue = (Ki / pow(10, multiI)) * I;
  float Dvalue = (Kd / pow(10, multiD)) * D;

  float PIDvalue = Pvalue + Ivalue + Dvalue;
  previousError = error;

  int leftSpeed = baseSpeed - PIDvalue;
  int rightSpeed = baseSpeed + PIDvalue;

  motorLeft(leftSpeed);
  motorRight(rightSpeed);
}

// ====== BLUETOOTH COMMAND HANDLER ======
int val, cnt = 0, v[3];
void valuesread() {
  val = SerialBT.read();
  cnt++;
  v[cnt] = val;
  if (cnt == 2) cnt = 0;
}

void processing() {
  int a = v[1];
  if (a == 1) Kp = v[2];
  if (a == 2) multiP = v[2];
  if (a == 3) Ki = v[2];
  if (a == 4) multiI = v[2];
  if (a == 5) Kd = v[2];
  if (a == 6) multiD = v[2];
  if (a == 7) onoff = v[2];
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-L9110S-LineBot");
  Serial.println("Bluetooth Started — Ready to pair!");

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(CENTER_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);

  pinMode(VIB_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(VIB_PIN), vibISR, RISING);

  Wire.begin();
  dht.begin();

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setFilterBandwidth(MPU6050_BAND_10_HZ);

  Serial.println("PID Line Follower (L9110S) + Road Monitor Ready!");
}

// ====== MAIN LOOP ======
void loop() {
  if (SerialBT.available()) {
    while (SerialBT.available() == 0);
    valuesread();
    processing();
  }

  if (onoff) {
    lineFollowPID();
  } else {
    stopMotors();
  }

  // --- Road Quality Monitoring ---
  sensors_event_t a, g, tempEvent;
  mpu.getEvent(&a, &g, &tempEvent);
  azValues[indexA] = a.acceleration.z;
  indexA = (indexA + 1) % SAMPLE_COUNT;
  float stdAz = computeStdDev(azValues, SAMPLE_COUNT);

  if (millis() - lastRoadCheck >= ROAD_INTERVAL) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    float vibNorm = min(vibCount / 10.0, 1.0);
    float roughness = 0.8 * stdAz + 0.4 * vibNorm;

    float traction = 1.0;
    if (temp < 5) traction -= 0.4;
    else if (temp > 30) traction -= 0.3;
    if (hum > 60) traction -= 0.3;
    traction = max(traction, 0.4f);

    float normR = constrain(roughness / 1.5, 0.0, 1.0);
    float roadQuality = (1.0 - normR) * traction;

    String roadState = (roadQuality > 0.85) ? "GOOD" :
                       (roadQuality > 0.45) ? "MEDIUM" : "BAD";

    Serial.println("===== ROAD QUALITY REPORT =====");
    Serial.print("Temp: "); Serial.print(temp); Serial.print(" °C  ");
    Serial.print("Humidity: "); Serial.print(hum); Serial.println(" %");
    Serial.print("Accel Z stddev: "); Serial.println(stdAz, 3);
    Serial.print("Vibration count/sec: "); Serial.println(vibCount);
    Serial.print("Road Quality Score: "); Serial.println(roadQuality, 2);
    Serial.print("Condition: "); Serial.println(roadState);
    Serial.println();

    vibCount = 0;
    lastRoadCheck = millis();
  }
}
