#include <Arduino.h>


EncoderButton encoder(31, 32, 30);

void incrementSelectedPort(int increment)
{
  selectedPort +=increment;

  if(selectedPort >= getSwitchPortCount())
    selectedPort = -2;

  if(selectedPort < -2)
    selectedPort = (getSwitchPortCount() - 1);
}

void incrementVLAN(int increment)
{
  int vlan = SwitchPorts[selectedPort].VLANId;

  vlan += increment;

  if(vlan > TPLINK_MAX_VLAN)
    vlan = TPLINK_MAX_VLAN;

  if(vlan  < 0)
    vlan  = 0;

  SwitchPorts[selectedPort].VLANId = vlan;
}


void encoderMoved(EncoderButton& eb)
{
  Serial.println("Encoder");

  if(editMode)
    incrementVLAN(eb.increment());
  else
    incrementSelectedPort(eb.increment());

  drawDisplay();
}

void encoderPressed(EncoderButton& eb)
{
  if(editMode)
  {
    //Save the new VLAN
    setPortVLAN(selectedPort, SwitchPorts[selectedPort].VLANId);
  }

  if(selectedPort >= 0)
  {
    editMode = !editMode;
    drawDisplay();
  }
  else if(selectedPort == -1 && editModified)
  {
    cancelTPLink();
    drawDisplay();
  }
  else if(selectedPort == -2 && editModified)
  {
    saveTPLink();
    drawDisplay();
  }
}

void setupEncoder()
{
  encoder.setEncoderHandler(encoderMoved);
  encoder.setPressedHandler(encoderPressed);
}

void readEncoder()
{
  encoder.update();
}