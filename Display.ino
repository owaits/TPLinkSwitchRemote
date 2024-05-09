
#define cs 10
#define dc 9
#define rst 8
#define blk 7     //Backlight on/off

#define BLACK ST77XX_BLACK
#define WHITE ST77XX_WHITE
#define RED ST77XX_BLUE
#define GREEN ST77XX_GREEN
#define BLUE ST77XX_RED
#define CYAN ST77XX_YELLOW
#define MAGENTA ST77XX_MAGENTA
#define YELLOW ST77XX_CYAN
#define ORANGE ST77XX_ORANGE

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

void setupDisplay()
{
  pinMode(blk, OUTPUT);     //Setup backlight control pin, leave low during init.

  delay(1000);
  tft.initR(INITR_GREENTAB);   // initialize a ST7735S chip, black tab
  delay(1000);

  tft.setTextWrap(false);
  tft.setRotation(1);
  tft.setCursor(0, 0);

  tft.fillScreen(BLACK);

  digitalWrite(blk, HIGH);    //Now we have initilised the display turn the backlight on.
}

void showPrompt(const char *text)
{
  tft.setTextWrap(true);
  tft.fillScreen(BLACK);
  drawCentreString(text,80,64);
  tft.setTextWrap(false);
}

void drawCentreString(const char *buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, 0,0, &x1, &y1, &w, &h); //calc width of new string
    tft.setCursor(x - w / 2, y - (h / 2));
    tft.print(buf);
}

void drawButton(int x, int y, int width, const char* label, bool selected)
{
    uint16_t background = BLUE;
    uint16_t foreground = WHITE;
    if(selected)
    {
      background = CYAN;
      foreground = BLACK;
    }      

    tft.fillRoundRect(x, y, width, 15, 2, background);
    tft.setTextColor(foreground);
    drawCentreString(label,x + (width / 2),y + 7);
}

void drawDisplay(bool refresh = false)
{
  if(refresh)
    tft.fillScreen(ST7735_BLACK);


  //Draw the Switch Information in the header.
  tft.setCursor(2, 4);
  tft.setTextColor(ST7735_WHITE);
  tft.println(EthernetLinkInformation.SystemnName);
  tft.println(EthernetLinkInformation.ManagementIP);

  int portsX = 0;
  int portsY = 30;
  for(int portIndex; portIndex < getSwitchPortCount(); portIndex ++)
  {
      uint16_t colour = BLUE;

      if(editMode && selectedPort == portIndex)
      {
          colour = CYAN;
          tft.setTextColor(BLACK);
      }
      else
      {
          tft.setTextColor(WHITE);
      }

      tft.fillRoundRect(portsX, portsY + 2, 15, 15, 2, colour);
      
      //If this is the selected port outline it to indicate the selection.
      if(selectedPort == portIndex)
        tft.drawRoundRect(portsX, portsY + 2, 15, 15, 2, CYAN);

      tft.setCursor(portsX + 5, portsY + 5);
      //tft.setTextSize(12);
      if(SwitchPorts[portIndex].tagged)
        tft.print("T");
      else
        tft.print(SwitchPorts[portIndex].VLANId);

      portsX += 16;
      if(portsX > 144)
      {
        portsX = 0;
        portsY += 16;
      }
  }

  tft.fillRect(0, portsY + 17, 160, 50, BLACK);
  if(selectedPort >= 0 && selectedPort < TPLINK_MAX_PORTS)
  {       
      //Display the selected port information.
      tft.setCursor(4, portsY + 21);
      tft.setTextColor(WHITE);

      tft.print("Port ");
      tft.println(selectedPort + 1);

      tft.println(SwitchPorts[selectedPort].VLANName);

      tft.print("VLAN ID:");
      tft.println(SwitchPorts[selectedPort].VLANId);

      if(SwitchPorts[selectedPort].tagged)
        tft.println("Tagged");
      else
        tft.println("Untagged");
  }
  else if (editModified)
  {
    drawButton(30,portsY + 25, 100, "SAVE", selectedPort == -2);
    drawButton(30,portsY + 41, 100, "CANCEL", selectedPort == -1);
  }
}