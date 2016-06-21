#include <Arduino.h>
#include <DMXSerial.h>
#include <EEPROM.h>
#include <Servo.h>

#define SHUTTER_HIGH 90
#define SHUTTER_LOW 0
#define SERVO_PIN 5
#define MANUAL_MODE_BUTTON A4
#define SIGNAL_LED 7
#define PWM_SIGNAL_LED 6
#define LEARN_DMX_BUTTON_PIN 9
#define DMX_EEPROM_ADDRESS 0

Servo servo;
bool lastDMXState = false;
bool lastManualState = false;
int lastServo = SHUTTER_LOW;
long lastMove = -1001;



void openShutter()
{
  if (lastServo != SHUTTER_HIGH)
  {
    servo.attach(SERVO_PIN);
    servo.write(SHUTTER_HIGH);
    lastServo = SHUTTER_HIGH;
  }
}

void closeShutter()
{
  if (lastServo != SHUTTER_LOW)
  {
    servo.attach(SERVO_PIN);
    servo.write(SHUTTER_LOW);
    lastServo = SHUTTER_LOW;
  }
}

void blink(int occurence)
{
  for (int i = 0 ; i < occurence ; i++)
  {
    digitalWrite(SIGNAL_LED, HIGH);
    digitalWrite(PWM_SIGNAL_LED, HIGH);
    delay(70);
    digitalWrite(SIGNAL_LED, LOW);
    digitalWrite(PWM_SIGNAL_LED, LOW);
    delay(300);
  }
}

void fastBlink()
{
  digitalWrite(SIGNAL_LED, HIGH);
  delay(70);
  digitalWrite(SIGNAL_LED, LOW);
  delay(50);
  digitalWrite(SIGNAL_LED, HIGH);
  delay(70);
  digitalWrite(SIGNAL_LED, LOW);
  delay(300);
}


void updateLearnDMX()
{
  if ( ! digitalRead(LEARN_DMX_BUTTON_PIN) )
  {
    for (int i = 0 ; i < 512 ; i++)
    {
      if (DMXSerial.read(i) != 0)
      {
        EEPROM.write(DMX_EEPROM_ADDRESS, i & B11111111 );
        EEPROM.write(DMX_EEPROM_ADDRESS + 1, i  >> 8 & B11111111 );
        blink(3);
      }
    }
  }
}

bool getDMXState()
{
  unsigned long lastPacket = DMXSerial.noDataSince();
  if (lastPacket < 1000 )
  {
    int address = EEPROM.read(DMX_EEPROM_ADDRESS);
    address += EEPROM.read(DMX_EEPROM_ADDRESS + 1) << 8;
    int dmxValue = DMXSerial.read(address);
    analogWrite(PWM_SIGNAL_LED, dmxValue);
    if (dmxValue > 127)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    fastBlink();
  }
}

bool getManualState()
{
  return digitalRead( MANUAL_MODE_BUTTON );
}

void updateShutter(bool state)
{
  if ( state == true )
  {
    openShutter();
  }
  else
  {
    closeShutter();
  }
  lastMove = millis();
}

void updateSleepMode()
{
  if ( millis() - lastMove > 1000 )
  {
    servo.detach();
  }
}


void setup()
{
  pinMode(SIGNAL_LED, OUTPUT);
  pinMode(PWM_SIGNAL_LED, OUTPUT);
  pinMode(MANUAL_MODE_BUTTON, INPUT_PULLUP);
  pinMode(LEARN_DMX_BUTTON_PIN, INPUT_PULLUP);

  DMXSerial.init(DMXReceiver);
  servo.attach(SERVO_PIN);
  fastBlink();
  delay(1000);
}

void loop()
{
  updateLearnDMX();
  bool currentDMXState = getDMXState();
  bool currentManualState = getManualState();
  if ( currentDMXState != lastDMXState )
  {
    lastDMXState = currentDMXState;
    updateShutter( lastDMXState );
  }
  else if ( currentManualState != lastManualState )
  {
    lastManualState = currentManualState;
    updateShutter( lastManualState );
  }
  else
  {
    updateSleepMode();
  }
}
