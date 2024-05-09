const byte lldp_mac[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e};

enum LLDP_TLVType
{
  TLV_CHASSIS_ID = 1,
  TLV_PORT_ID = 2,
  TLV_TIME_TO_LIVE = 3,
  TLV_SYSTEM_NAME = 5,
  TLV_SYSTEM_DESCRIPTION = 6,
  TLV_MANAGEMENT_ADDRESS = 8
};



void setupLLDP()
{
  if(!Ethernet.setMACAddressAllowed(lldp_mac,true))
    Serial.println("Unable to allow LLDP!");
}

void readLLDP(const uint8_t* data, int size)
{
  int position = 14;
  while (position < size) { // read all remaining TLV fields
    unsigned int tlvType = data[position] >> 1;
    position += 1;
    unsigned int tlvLength = data[position];
    /*Serial.print(" type:");
      Serial.print(tlvType, HEX);
      Serial.print(" Length:");
      Serial.print(tlvLength);*/
    position += 1;

    switch (tlvType) {

      case TLV_SYSTEM_NAME: //System Name
        readTLVString(data + position,tlvLength,EthernetLinkInformation.SystemnName);
        //Serial.println(EthernetLinkInformation.SystemnName);
        break;

      case TLV_SYSTEM_DESCRIPTION: //System Description
        readTLVString(data + position,tlvLength,EthernetLinkInformation.SystemDescription);
        //Serial.println(EthernetLinkInformation.SystemnName);
        break;

      case TLV_MANAGEMENT_ADDRESS: //Port Description
        readTLVAddress(data + position + 2,tlvLength,&EthernetLinkInformation.ManagementIP);
        //Serial.println(EthernetLinkInformation.ManagementIP);
        setTPLinkManagementIP(EthernetLinkInformation.ManagementIP);
        break;

      case 0x000a: //Device Name
        //Device = 1;
        //handleCdpAsciiField( Ethernet::buffer, DataIndex , (TLVFieldLength));
        //Device = 0;
        break;

      case 0x000e://Capabilities
        // handleCdpCapabilities( Ethernet::buffer, DataIndex + 2, TLVFieldLength - 2);
        break;

      case 0x0010: //Management IP Address
        //handleLLDPIPField(Ethernet::buffer, DataIndex + 1, TLVFieldLength);
        break;

      case 0x00fe: //LLDP Organisational TLV #
        //handleLLDPOrgTLV(Ethernet::buffer, DataIndex, TLVFieldLength);
        break;
    }
    position += tlvLength;
  }
}

void readTLVString(const uint8_t* buffer, int length, char* text)
{
    memcpy(text, buffer, length);

    //Ensure the string is null terminated.
    if(text[length-1] != 0)
      text[length-1] = 0;
}

void readTLVAddress(const uint8_t* buffer,int length, IPAddress* address)
{
  (*address)[0] = buffer[0];
  (*address)[1] = buffer[1];
  (*address)[2] = buffer[2];
  (*address)[3] = buffer[3];
  (*address)[4] = buffer[4];
  (*address)[5] = buffer[5];
}

