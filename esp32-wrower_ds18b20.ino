#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// вписываем здесь SSID и пароль для вашей WiFi-сети:
const char* ssid = "netis";
const char* password = "vovik1991";
const char* host = "192.168.1.6";
const int port = 80;

//const int uartTxPin = 1; //For interrupts
//const int uartRxPin = 3; //For interrupts

const int sensorLed = 4;
const int uartTxLed = 14;
const int uartRxLed = 12;

// контакт для передачи данных подключен к D1 на ESP8266 12-E (GPIO5):
#define ONE_WIRE_BUS 5

// создаем экземпляр класса oneWire; с его помощью
// можно коммуницировать с любыми девайсами, работающими
// через интерфейс 1-Wire, а не только с температурными датчиками
// от компании Maxim/Dallas:
OneWire oneWire(ONE_WIRE_BUS);

// передаем объект oneWire объекту DS18B20:
DallasTemperature sensor(&oneWire);

DeviceAddress sensor1 = {0x28, 0xFE, 0xF1, 0x45, 0x92, 0x11, 0x2, 0x86};
DeviceAddress sensor2 = {0x28, 0xD, 0xA1, 0x45, 0x92, 0xC, 0x2, 0x5D};
DeviceAddress sensor3 = {0x28, 0xFF, 0x5D, 0x67, 0x23, 0x16, 0x4, 0x70};

byte counter = 0;
byte* counterPtr = &counter;

float prevTempC1 = 0.0;
float prevTempC2 = 0.0;
float prevTempC3 = 0.0;

float* prevTempC1Ptr = &prevTempC1;
float* prevTempC2Ptr = &prevTempC2;
float* prevTempC3Ptr = &prevTempC3;

char sensor1CString[6];
char sensor2CString[6];
char sensor3CString[6];

//char sensor1FString[6];
//char sensor2FString[6];
//char sensor3FString[6];

void ledBlink(int ledPin, int msPause){
  digitalWrite(ledPin, HIGH);
  delay(msPause);
  digitalWrite(ledPin, LOW);
  delay(msPause);
}

void getSensorTempC(DeviceAddress addr, float correct, float* prevTempCPtr, char* tempBufC){
  float tempC;
  sensor.requestTemperatures();
  tempC = sensor.getTempC(addr);
  if(tempC >= 85.0 || tempC <= (-100.0)){
      if(*counterPtr++ < 3){
        dtostrf(*prevTempCPtr, 2, 2, tempBufC); //Anti crash
      }
      else {
        *counterPtr = 0;
        dtostrf((*prevTempCPtr = 0.00), 2, 2, tempBufC);
      }
  }
  
  else if(tempC == 0.00){
    sensor.requestTemperatures();
      tempC = sensor.getTempC(addr);
   }
  else if(tempC == 0.00){
    //sensor.requestTemperatures();
      tempC = sensor.getTempC(addr);
   }
   else if(tempC == 0.00){
    sensor.requestTemperatures();
      tempC = sensor.getTempC(addr);
   }
  else{
    *prevTempCPtr = tempC;
    dtostrf(tempC + correct, 2, 2, tempBufC);
    ledBlink(sensorLed, 20);
  } 
}

//void getSensorTempF(DeviceAddress addr, float correct, char* tempBufF){
//  float tempF;
//  sensor.requestTemperatures();
//  tempF = sensor.getTempF(addr);
//  if(tempF == 85.0 || tempF == (-127.0)){
//    tempF = 0;
//    dtostrf(tempF, 3, 2, tempBufF);
//  }
//  else{
//    dtostrf(tempF + correct, 2, 2, tempBufF);
//  } 
//}

void getTemperature() {  

  getSensorTempC(sensor1, 0.0, prevTempC1Ptr, sensor1CString);
  getSensorTempC(sensor2, 0.0, prevTempC2Ptr, sensor2CString);
  getSensorTempC(sensor3, 0.0, prevTempC3Ptr, sensor3CString);

  delay(50);
}

// блок setup() запускается только один раз – при загрузке:
void setup() {
  
  pinMode(sensorLed, OUTPUT); 
  digitalWrite(sensorLed, LOW);

  // инициализируем последовательный порт (для отладочных целей):
  Serial.begin(115200);

  sensor.begin(); // по умолчанию разрешение датчика – 9-битное;
  // если у вас какие-то проблемы, его имеет смысл
  // поднять до 12 бит; если увеличить задержку,
  // это даст датчику больше времени на обработку
  // температурных данных

  // подключаемся к WiFi-сети:
  Serial.println();
  Serial.println();
  Serial.print("Connecting to Wi-Fi network: "); // "Подключаемся к "
  Serial.print("\"");
  Serial.print(ssid);
  Serial.println("\"");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected!"); // "Подключение к WiFi выполнено"
  Serial.println("Device IP address: ");
  Serial.println(WiFi.localIP());

} // End setup

// блок loop() будет запускаться снова и снова:
void loop() {

  getTemperature();

  WiFiClient wifi_client;
  Serial.println();
  Serial.print("Create WiFi client - ");
  
  if(!wifi_client.connect(host, port)){
    Serial.println("Connection failed");
    return;
  }
  Serial.println("OK!");
  Serial.print("Connecting to host: ");
  Serial.print(host);
  Serial.print(", port: ");
  Serial.print(port);
  Serial.println(" - OK!");
  Serial.println();

  if(sensor1CString==NULL) dtostrf(0.00, 2, 2, sensor1CString);
  if(sensor2CString==NULL) dtostrf(0.00, 2, 2, sensor2CString);
  if(sensor3CString==NULL) dtostrf(0.00, 2, 2, sensor3CString);

  String postData = postData + "device=TEST&" + "sensor1=" + sensor1CString + "&" + "sensor2=" + sensor2CString + "&" + "sensor3=" + sensor3CString;

  HTTPClient http_client;
  
  http_client.begin("http://192.168.1.6/tcontrol_war_exploded/saveData");              //Specify request destination
  http_client.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http_client.POST(postData);   //Send the request
  Serial.println(postData); //DEBUG
  String payload = http_client.getString();    //Get the response payload


  Serial.println(httpCode); //DEBUG
  Serial.println(payload); //DEBUG

  http_client.end();
  
  delay(500);
  if(wifi_client.connected()){
    wifi_client.stop();
  }
  Serial.println("Closing connection");
  Serial.println();
  Serial.print("CPU frequency in MHz: ");
  Serial.println(ESP.getCpuFreqMHz());
  Serial.print("Flash chip size (in bytes): ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("Free RAM memory (Heap): ");
  Serial.println(ESP.getFreeHeap());
  Serial.println();
  Serial.println("*************************************************"); 
  delay(1000);
}
