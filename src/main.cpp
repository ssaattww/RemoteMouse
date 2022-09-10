#include <BleMouse.h>
#include <PacketSerial.h>
#include <Arduino.h>
#include <M5Core2.h>

#include <Encoding/COBS.h>

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

// mouse
void moveMouse();
BleMouse bleMouse;
MousePos pos;
int count = 0;        // moved count

// serial
void serialArrivedEvent();
bool arrived = false;
uint8_t buffer[64];
size_t bufferSize;

void setup() {
  M5.begin();
  xTaskCreatePinnedToCore(displayTask, "displayTask", 4096, NULL, 2, NULL, 0);
  Serial.begin(115200);
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  bleMouse.begin();
}

void loop() {
  if(arrived){
    serialArrivedEvent();
    moveMouse();
  }
}

void serialEvent(){
  arrived = true;
  bufferSize = Serial.readBytes(buffer,64);
}

void moveMouse(){
  if(bleMouse.isConnected()){
      if(pos.B.pressing == 1){
        bleMouse.press(pos.B.buttons);
      }else if(pos.B.pressing == 0){
        bleMouse.release(pos.B.buttons);
      }

      bleMouse.move(pos.B.x, pos.B.y, pos.B.wheel, pos.B.hWheel);
      count++;
  }
}

void serialArrivedEvent(){
  uint8_t decoded[64];
  uint8_t encoded[64];
  size_t decodedSize = COBS::decode(buffer, bufferSize, decoded);
  size_t encodedSize = COBS::encode(decoded, decodedSize, encoded);
  if(decodedSize == 6){
    for(int i=0;i<6;i++){
      pos.bytes[i] = decoded[i];
    }
  }
  Serial.write(encoded,encodedSize);
  arrived = false;
}

void displayTask(void* args){
  while (true)
  {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%d:%d,%d,%d,%d        ",count, pos.B.x, pos.B.y, pos.B.hWheel, pos.B.hWheel);
    delay(100);
  }
}