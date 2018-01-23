/*
*******************************************************************************
midi messages come in as 3 bytes each byte can be of values 0 - 127 | 0x00 - 0x7F

byte[0]
type of message, 
- 0xB0 seems to be change in a ctrl knob - note we seem to get this changing when not touching the pad (possibly due to noise on the arduino board)
- 0x99 down on a pad
- 0x89 up on a pad
- 0xD9 aftertouch - is there only 1 aftertouch vailbe for all pads?

byte[1]
data 1 
- when a control this is the control number 0x03 is the first control, 0x09 is the second and all are in series after thiss, 0x0a, 0x0b ...
- when an up or down event this is the pad, based on note (settings will change per device this one has settings for different scales)
- when after touch this is the value of aftertouch (change in pressure on a pad). 

byte[3]
data 2
- when a control this is the value.
- when down this is the velocity of the hit 
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

USB Usb;
USBH_MIDI  Midi(&Usb);

const char * notes [] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };

const uint8_t control_threshold = 1;

uint8_t controlValues[20];

void MIDI_poll();

void doDelay(uint32_t t1, uint32_t t2, uint32_t delayTime);

void setup()
{
  Serial.begin(9600); //for debugging to arduino serial monitor
  Serial1.begin(31250); 
  
  Serial.println("begin");

  if (Usb.Init() == -1) {
    while (1);
  }
  delay( 200 );
}

void loop()
{

  //USB stuff
  Usb.Task();
  uint32_t t1 = (uint32_t)micros();

  if ( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {
    MIDI_poll();
  }

  //this is make it fixed framerate right?
  doDelay(t1, (uint32_t)micros(), 1000);
}

// Poll USB MIDI Controler and handle
void MIDI_poll()
{
  
  uint8_t outBuf[ 3 ];
  uint8_t size;

  do {
    if ( (size = Midi.RecvData(outBuf)) > 0 ) {
      //MIDI Output
      handleMidiData(outBuf);
    }
  } while (size > 0);

  
}

// this is a way to reduce noise on development boards.
bool updateControls(uint8_t control, uint8_t newVale){

if(abs(controlValues[control] - newVale) > control_threshold){
  controlValues[control] = newVale;
  return true;
}

 return false;
}



void handleMidiData(uint8_t bytes[]){
  

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
