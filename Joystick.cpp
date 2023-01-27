#include "Arduino.h"
#include "Joystick.h"

void Joystick::ReadYaxis()
{
  yValue = analogRead(A1); 
  if(yValue < 100)
  {
    keyPressed = 'U';
  }
  else if(yValue > 900)
  {
    keyPressed = 'D';
  }else
  {
    keyPressed = ' ';
  }
}