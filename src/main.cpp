#include <BleMouse.h>
#include <PacketSerial.h>
#include <Arduino.h>
#include <M5Core2.h>

#include <Encoding/COBS.h>
# define BUF_SIZE 10
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

typedef union{
  uint8_t bytes[sizeof(char)*6];
  struct 
  {
    signed char buttons;
    signed char pressing;
    signed char x;
    signed char y;
    signed char wheel; // tate
    signed char hWheel;// yoko
  }B;
}MousePos;

// task
void displayTask(void* args);
void mouseTask(void* args);
void serialTask(void* args);

// mouse
void moveMouse();
BleMouse bleMouse;
MousePos pos;
int count = 0;        // moved count

// serial
void serialArrivedEvent();
volatile bool arrived = false;
volatile uint8_t buffer[BUF_SIZE];
volatile size_t bufferSize;

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  bleMouse.begin();
  // Serial.begin(115200);
  Serial.begin(9600);
  //xTaskCreatePinnedToCore(displayTask, "displayTask", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(serialTask, "serialTask", 4096, NULL, 8, NULL, 0);
  xTaskCreatePinnedToCore(mouseTask, "mouseTask", 4096, NULL, 7, NULL, 0);
}

void loop() {
  portENTER_CRITICAL(&mutex);
  int c = count;
  MousePos p = pos;
  portEXIT_CRITICAL(&mutex);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("%d:%d,%d,%d,%d        ",c, p.B.x, p.B.y, p.B.hWheel, p.B.hWheel);
  delay(1000);
}

void serialTask(void *args){
  while (true)
  {
    if(Serial.available()>0){
      uint8_t tmpBuf[BUF_SIZE];
      size_t tmpSize;
      int i = 0;
      for(i=0; i<BUF_SIZE;i++){
        uint8_t tmp;
        tmp = Serial.read();
        if(tmp == -1)break;
        else if(tmp == 0)break;
        else tmpBuf[i] = tmp;
      }
      tmpSize = i;
      //tmpSize = Serial.readBytesUntil(0x00, tmpBuf, BUF_SIZE);
      //tmpSize = Serial.readBytes(tmpBuf,BUF_SIZE);

      portENTER_CRITICAL(&mutex);
      arrived = true;
      for(int i =0;i<BUF_SIZE;i++ )buffer[i] = tmpBuf[i];
      bufferSize = tmpSize;
      count++;
      portEXIT_CRITICAL(&mutex);
    }
    delay(1);
  }
}

void mouseTask(void *args){
  while (true)
  {
      serialArrivedEvent();
      delay(1);
  }
}

void serialEvent(){
  // uint8_t tmpBuf[BUF_SIZE];
  // size_t tmpSize;
  
  // tmpSize = Serial.readBytes(tmpBuf,BUF_SIZE);

  // portENTER_CRITICAL(&mutex);
  // arrived = true;
  // for(int i =0;i<BUF_SIZE;i++ )buffer[i] = tmpBuf[i];
  // bufferSize = tmpSize;
  // count++;
  // portEXIT_CRITICAL(&mutex);
}

void moveMouse(){
  if(bleMouse.isConnected()){
      if(pos.B.pressing == 1){
        bleMouse.press(pos.B.buttons);
      }else if(pos.B.pressing == 0){
        bleMouse.release(pos.B.buttons);
      }
      bleMouse.move(pos.B.x, pos.B.y, pos.B.wheel, pos.B.hWheel);
  }
}

void serialArrivedEvent(){
  uint8_t decoded[BUF_SIZE];
  uint8_t encoded[BUF_SIZE];
  uint8_t tmp[BUF_SIZE];
  size_t decodedSize = 0;
  if(arrived){
    portENTER_CRITICAL(&mutex);
    for(int i =0; i<BUF_SIZE;i++)tmp[i] = buffer[i];
    decodedSize = COBS::decode(tmp, bufferSize, decoded);
    arrived = false;
    portEXIT_CRITICAL(&mutex);
  }
  if(decodedSize!=0){
    // size_t encodedSize = COBS::encode(decoded, decodedSize, encoded);
    // if(decodedSize == 6){
    //   for(int i=0;i<6;i++){
    //     pos.bytes[i] = decoded[i];
    //   }
    // }
    moveMouse();
    // Serial.write(encoded,encodedSize);
    Serial.write(tmp, bufferSize);
    //M5.Lcd.setCursor(0, 50);
    //M5.Lcd.printf("%d:",encodedSize);
    //for(int i =0; i< encodedSize; i++)M5.Lcd.printf("%d ",encoded[i]);
    //M5.Lcd.printf("       ");
    //M5.Lcd.printf("%d:%d,%d,%d,%d,%d,%d        ",decodedSize,decoded[0], decoded[1], decoded[2], decoded[3], decoded[4], decoded[5]);
  }
}

void displayTask(void* args){
  while (true)
  {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%d:%d,%d,%d,%d        ",count, pos.B.x, pos.B.y, pos.B.hWheel, pos.B.hWheel);
    delay(100);
  }
}