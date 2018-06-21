// demo: CAN-BUS Shield, receive data with interrupt mode
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-6-13

#include <SPI.h>
#include "Timer.h"
#include "mcp_can.h"

#define UPDATE_MS     100

#define DATA_PRINT

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

const int D0_CONTRL_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin


unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
byte tx_buf_[8]= {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08};
//unsig/ned int sensor_data_[] = {1, 12, 123, 2, 23, 234, 3, 34, 345, 1018};

int sensorValue[6]; 
Timer t;
int readEvent;

void updateSensors();

void setup()
{
#ifdef DATA_PRINT
    Serial.begin(115200);
    readEvent = t.every(UPDATE_MS, updateSensors);
#endif
    while (CAN_OK != CAN.begin(CAN_40KBPS))              // init can bus : baudrate = 500k
    {
#ifdef DATA_PRINT
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
#endif
        delay(100);
    }
#ifdef DATA_PRINT
    Serial.println("CAN BUS Shield init ok!");
#endif
    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
}

void MCP2515_ISR()
{
    flagRecv = 1;
}

void loop()
{
    t.update();
    if(flagRecv) 
    {                                   // check if get data

        flagRecv = 0;                   // clear flag
        encodeData(sensorValue, 6, tx_buf_);
        // iterate over all pending messages
        // If either the bus is saturated or the MCU is busy,
        // both RX buffers may be in use and reading a single
        // message does not clear the IRQ conditon.
        while (CAN_MSGAVAIL == CAN.checkReceive()) 
        {
            // read data,  len: data length, buf: data buf
            CAN.readMsgBuf(&len, buf);
            CAN.sendMsgBuf(0x00, 0, 8, tx_buf_);
            // print the data
#ifdef DATA_PRINT
            for(int i = 0; i<len; i++)
            {
                Serial.print(buf[i], HEX);Serial.print("\t");
            }
            Serial.println();
#endif
        }
    }
}

void encodeData(unsigned int *input_data, int len, byte *output_data)
{
  boolean data_in_bits[64];
  int index = 64;
  for (int i=3; i>=0; i--)
  {
    data_in_bits[--index] = (len >> i) & 0x01;
  }
  for (int i=0; i<6; i++)
  {
    for (int j=9; j>=0; j--)
    {
      data_in_bits[--index] = (input_data[i] >> j) & 0x01;
    }
  }
  index = 64;
  for (int i=0; i<8; i++)
  {
    byte a = 0x00;
    output_data[i] = 0x00;
    for (int j=7; j>=0; j--)
    {
      a = data_in_bits[--index];
      output_data[i]  += a << j;
    }
  }
}


void updateSensors()
{
  sensorValue[0] = analogRead(A0);
  sensorValue[1] = analogRead(A1);
  sensorValue[2] = analogRead(A2);
  sensorValue[3] = analogRead(A3);
  sensorValue[4] = analogRead(A4);
  sensorValue[5] = analogRead(A5);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
