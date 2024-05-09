

EthernetClient client;

IPAddress managementIP = INADDR_NONE;
int portCount = 0;

void setTPLinkManagementIP(IPAddress ip)
{
  managementIP = ip;
}

void connectTPLink(IPAddress ip)
{
  managementIP = ip;

  Serial.print("Connecting to Switch ");
  Serial.println(managementIP);
  showPrompt("Connecting to Switch");

  int connectionStatus = client.connect(managementIP, 23);
  if(connectionStatus == 1)
  {
    showPrompt("Logging in...");
    if(loginTPLink("admin","0f1lms") || loginTPLink("admin","admin"))
    {
      showPrompt("Updating ports...");
      updateAllPorts();
    }
    else
    {
      showPrompt("Failed to Login!\ncheck password");
    }
  }
  else
  {
    showPrompt("Failed to Connect!");
    Serial.print("Connection error ");
    Serial.println(connectionStatus);
  }
}

bool isConnected()
{
  return client.connected();
}

long lastConnectionAttempt = 0;

void readTPLink(bool print = true)
{
  while (client.available()) {
    char c = client.read();

    if(print)
      Serial.print(c);
  }
}

void skipWhitespace(bool print = false)
{
  while (client.peek() == ' ' || client.peek() == '\t' || client.peek() == '\r')
  {
      char c = client.read();      
      /*if(print)
        Serial.print(c);*/
  }    
}

void skipLines(int count,bool print = false)
{
  for(int n=0 ;n< count; n++)
  {
    while(client.read() != '\n')
    {   
        /*if(print)
          Serial.print(c);*/
    }
  }
}

void loopTPLink()
{
  if(client.connected())
  {
    readTPLink();
    return;
  }

  if(managementIP != INADDR_NONE && (millis() - lastConnectionAttempt) > 1000)
  {
    connectTPLink(managementIP);
    lastConnectionAttempt = millis();
  }
}

bool loginTPLink(const char* user, const char* password)
{
  delay(1000);
  readTPLink();
  client.println(user);    //User Name
  client.println(password);   //Password
  delay(500);

  skipLines(3,true);
  skipWhitespace(true);

  //Ensure the login was successful
  String loginStatus = client.readStringUntil('\r');
  if(loginStatus == "Login invalid.")
  {
    return false;    
  }

  readTPLink();

  client.println("enable");   //Enable Access

  delay(500);
  readTPLink();

  return true;
}

int getSwitchPortCount()
{
  return portCount;
}

void showVLANS()
{
  client.println("show vlan");

  delay(100);
  readTPLink();
}

void updateAllPorts()
{
  portCount=0;
  for(int port = 0; port < TPLINK_MAX_PORTS && portCount <= 0; port ++)
  {
      if(!readPortVlan(port))
        portCount = port;

      Serial.println("New PORT");
  }

  drawDisplay(true);  
}

bool readPortVlan(int portIndex)
{
  client.print("show interface switchport gigabitEthernet 1/0/");
  client.println(portIndex + 1);
  skipLines(1);
  skipWhitespace();


  String header = client.readStringUntil(' ');
  Serial.println(client.getReadError());
  if(header != "Port")
  {
    return false;
  }
  skipLines(9);
  
  skipWhitespace(); //Skip the space at the start of the line.

  //Read the VLAN ID column
  client.clearReadError();
  SwitchPorts[portIndex].VLANId = client.parseInt();
  skipWhitespace();

  //Read the VLAN Name Column
  client.readBytesUntil(' ',SwitchPorts[portIndex].VLANName, 25);
  skipWhitespace();

  //Read the tag column
  client.clearReadError();
  String tag = client.readStringUntil('\r');
  SwitchPorts[portIndex].tagged = (tag == "Tagged");
  skipWhitespace();

  if(client.getReadError() > 0)
    Serial.println("ERROR reading VLAN");

  readTPLink(false);   //Read any remaining data that was not read.

  return true;
}

void setPortVLAN(int portIndex, int vlan)
{
  Serial.println("Set Port VLAN");

  //Select the port we want to edit.
  client.println("configure");
  delay(200);
  readTPLink(true);

  client.print("interface gigabitEthernet 1/0/");
  client.println(portIndex + 1);
  delay(200);
  readTPLink(true);

  //Configure VLAN access
  client.print("switchport general allowed vlan ");
  client.print(vlan);
  client.println(" untagged");
  
  //Configure the PVID
  client.print("switchport pvid ");
  client.println(vlan);

  //Remove the port from all other VLANs.
  removePortFromOtherVLANS(vlan);

  //Exit Port Configuration
  client.println("end");

  delay(500);
  readTPLink(true);

  readPortVlan(portIndex);

  editModified = true;
}

void removePortFromOtherVLANS(int vlan)
{
    //Remove port from all but the specified vlan.
  client.print("no switchport general allowed vlan ");
  if(vlan > 1)
  {
    client.print("1-");
    client.print(vlan-1);
    client.print(",");
  }
  client.print(vlan+1);
  client.println("-1000");

  delay(200);
  readTPLink(true);
}

void saveTPLink()
{
  editModified = false; 
  
  //Save any config changes so they are restored after reboot.
  client.println("copy running-config startup-config");

  //Read the save status returned by the switch.
  skipLines(2);  
  String saveStatus = client.readStringUntil('\r');
  readTPLink(false);

  showPrompt(saveStatus.c_str());
  delay(3000);
  drawDisplay();
}

void cancelTPLink()
{
  rebootTPLink();
}

void rebootTPLink()
{
  editModified = false; 

  //Reset the switch and clear any unsaved changes.
  client.println("reboot");
  client.println("N");
  client.println("Y");

  //Read the reboot status returned by the switch.
  skipLines(2);  
  String rebootStatus = client.readStringUntil('\r');
  readTPLink(false);

  showPrompt(rebootStatus.c_str());
  delay(3000);
  drawDisplay();
}

void factoryResetTPLink()
{
  editModified = false; 

  //Reset the switch and clear any unsaved changes.
  client.println("reset");
  client.println("Y");
  delay(200);
  readTPLink(true);

  showPrompt("Factory reset...");
}

