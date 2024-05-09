#include <Adafruit_GFX.h>    // Core graphics librar
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <EncoderButton.h>
#include <QNEthernet.h>

using namespace qindesign::network;

#define TPLINK_MAX_PORTS 30
#define TPLINK_MAX_VLAN 9

const IPAddress ip = {10,10,1,250};
const IPAddress subnet = {255,255,255,0};
const IPAddress gateway = {10,10,1,254};

bool editMode = false;
bool editModified = false;
int selectedPort = -1;

struct LLDP_PortInformtion
{
    char SystemnName[255];
    char SystemDescription[255];
    IPAddress ManagementIP;
    int PortVLANId;
    char PortVLANName;
};

struct TPLink_PortInformtion
{
    char Label[25];
    int VLANId;
    char VLANName[25];
    bool tagged;
};

LLDP_PortInformtion EthernetLinkInformation;
TPLink_PortInformtion SwitchPorts[TPLINK_MAX_PORTS];


void setup()
{
  Serial.begin(115200);

  if(!Ethernet.begin(ip, subnet, gateway))
    Serial.println("Failed to start Ethernet!");


  //Setup Modules
  setupEncoder();
  setupDisplay();
  setupLLDP();
  //setupTPLink(); 

  showPrompt("Discovering switch...");
}

void readRawFrame()
{
  int size = EthernetFrame.parseFrame();
  if (size < EthernetFrame.minFrameLen())
    return;

  const uint8_t *buffer = EthernetFrame.data();
  struct eth_hdr* header = (eth_hdr*) (buffer - 2);

  switch(header->type)
  {
    case PP_HTONS(ETHTYPE_LLDP):
      readLLDP(buffer,size);
      break;
  }

  /*for (int i = 0; i < size; i++) {
    printf(" %02x", buffer[i]);
  }
  printf("\r\n");*/
}

void loop()
{
    readRawFrame();
    loopTPLink();
    readEncoder();
}