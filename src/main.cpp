#include <BleMouse.h>
#include <PacketSerial.h>
#include <Arduino.h>
#include <M5Core2.h>

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
void onPacketReceived(const void* sender, const uint8_t* buffer, size_t size);

BleMouse bleMouse;
PacketSerial myPacketSerial;
MousePos pos;
bool handled = false;
void setup() {
  M5.begin();
  myPacketSerial.setPacketHandler(&onPacketReceived);
  myPacketSerial.begin(115200);
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  bleMouse.begin();
}

void loop() {
  int count = 0;
  if(bleMouse.isConnected() && handled == false){
      if(pos.B.pressing == 1){
        bleMouse.press(pos.B.buttons);
      }
      else if(pos.B.pressing == 0){
        bleMouse.release(pos.B.buttons);
      }
      bleMouse.move(pos.B.x, pos.B.y, pos.B.wheel, pos.B.hWheel);
      handled = true;
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("%d:%d,%d,%d,%d        ",count, pos.B.x, pos.B.y, pos.B.hWheel, pos.B.hWheel);
      count++;
  }
}

void onPacketReceived(const void* sender, const uint8_t* buffer, size_t size){
  if(sender == &myPacketSerial && size == 6){
    for(int i=0;i<6;i++){
        pos.bytes[i] = buffer[i];
    }
    handled = false;
    M5.Lcd.setCursor(20, 0);
    M5.Lcd.printf("recieved");
  }
}