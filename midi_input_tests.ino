/*
 *******************************************************************************
   USB-MIDI to Legacy Serial MIDI converter
   Copyright (C) 2012-2017 Yuuichi Akagawa

   Idea from LPK25 USB-MIDI to Serial MIDI converter
     by Collin Cunningham - makezine.com, narbotic.com

   This is sample program. Do not expect perfect behavior.
 *******************************************************************************
*/


#include <usbh_midi.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#ifdef USBCON
#define _MIDI_SERIAL_PORT Serial1
#else
#define _MIDI_SERIAL_PORT Serial
#endif
//////////////////////////
// MIDI Pin assign
// 2 : GND
// 4 : +5V(Vcc) with 220ohm
// 5 : TX
//////////////////////////

USB Usb;
USBH_MIDI  Midi(&Usb);

const char * notes [] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };

uint8_t controlValues[9];

void MIDI_poll();
void doDelay(uint32_t t1, uint32_t t2, uint32_t delayTime);

void setup()
{
  Serial.begin(9600);
  Serial.println("begin");
  Serial1.begin(31250);

  if (Usb.Init() == -1) {
    while (1); //halt
  }//if (Usb.Init() == -1...
  delay( 200 );
}

void loop()
{
  Usb.Task();
  uint32_t t1 = (uint32_t)micros();
  if ( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {
    MIDI_poll();
  }
  //delay(1ms)
  doDelay(t1, (uint32_t)micros(), 1000);
}

// Poll USB MIDI Controler and send to serial MIDI
void MIDI_poll()
{
  
  uint8_t outBuf[ 3 ];
  uint8_t size;

  do {
    if ( (size = Midi.RecvData(outBuf)) > 0 ) {
      //MIDI Output
      midiLog(outBuf);
      
//      _MIDI_SERIAL_PORT.write(outBuf, size);
    }
  } while (size > 0);

  
}

bool updateControls(uint8_t control, uint8_t newVale){

  if(control == 0x03){
    if(abs(controlValues[0] - newVale) > 1){
      controlValues[0] = newVale;
      return true;
    }
  } else {
    if(abs(controlValues[control - 8] - newVale) > 1){
      controlValues[control - 8] = newVale;
      return true;
    }
  }

  return false;
}



void midiLog(uint8_t bytes[]){
  //so here we ca


  /*
  WHAT WE KNOW SO FAR

  byte[0]
  type of message, 0xB0 seems to be change in a ctrl knob 




  */

  // if(bytes[0] == 0xB0 && bytes[1] == 0x1b)
  //   return;

  int channel = (bytes[0] & 0x0F) + 1;

  int note = (int) bytes[1];

  int vel = (int) bytes[2];
  // int octave = (int) ((bytes[1]/12) - 1);


  // if(
  //   (bytes[0] == 0xB0 && bytes[1] == 0x03) ||
  //   (bytes[0] == 0xB0 && bytes[1] == 0x0D)
  // )
  // {
  //   return;
  // }

  if(bytes[0] == 0x99){
    Serial.print("Down ");
  } else if (bytes[0] == 0x89){
    Serial.print("Up ");    
  } else if (bytes[0] == 0xD9){
    Serial.print("Aftertouch ");    
  } else if(bytes[0] == 0xB0){
      if(!updateControls(bytes[1], bytes[2])){
        return;
      } else {
        Serial.print("Control ");

      }
  }


  Serial.print((byte) bytes[0], HEX);
  Serial.print(":");
  Serial.print((byte) bytes[1], HEX);
  Serial.print(":");
  Serial.print((byte) bytes[2], HEX);  

  // Serial.print(" -- Chanel: ");
  // Serial.print(channel);
  // Serial.print(" -- Note: ");
  // Serial.print(note);
  // Serial.print(" -- Velocity: ");
  // Serial.print(vel);
  Serial.println(";");
}

// Delay time (max 16383 us)
void doDelay(uint32_t t1, uint32_t t2, uint32_t delayTime)
{
  uint32_t t3;

  if ( t1 > t2 ) {
    t3 = (0xFFFFFFFF - t1 + t2);
  } else {
    t3 = t2 - t1;
  }

  if ( t3 < delayTime ) {
    delayMicroseconds(delayTime - t3);
  }
}
