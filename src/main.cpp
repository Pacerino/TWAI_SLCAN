#include "pins.h"
#include "driver/twai.h"
#include "Arduino.h"

// CURRENTLY ESP32 Dev Module Board Definition
// PIN 4  CANTX to WAVESHARE CAN transceiver
// PIN 5  CANRX to WAVESHARE CAN transceiver
// PIN 12 BLUETOOTH SWITCH outer pin
// PIN 14 USED FOR SWITCH outer pin TO SHOW MSG COUNTER
// PIN 15 10k to ground to remove boot messages
// PIN 21 SDA (4.7k to 5v) for SSD1306
// PIN 22 SCL (4.7k to 5v) for SSD1306
// 3.3v to SSD1306 & WAVESHARE CAN transceiver
// GND to SWITCH CENTER, SSD1306 & WAVESHARE CAN transceiver

bool working = false;
bool timestamp = false;
static uint8_t hexval[17] = "0123456789ABCDEF";

#define BAUDRATE 500000
//#define DEBUG

QueueHandle_t tx_queue;         // Sender Queue
twai_timing_config_t t_config;  // CAN Speed Config
twai_general_config_t g_config; // TWAI General Config
twai_filter_config_t f_config;  // TWAI Filter COnfig

//----------------------------------------------------------------

void slcan_ack()
{
  Serial.write('\r'); // CR
} // slcan_ack()

//----------------------------------------------------------------

void slcan_nack()
{
  Serial.write('\a');
} // slcan_nack()

//----------------------------------------------------------------

void pars_slcancmd(char *buf)
{
  switch (buf[0])
  {
  case 'O': // Open CAN channel
    if (!working)
    {
      if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) // Check if already working and install TWAI driver
      {
        if (twai_start() != ESP_OK) // Start TWAI driver
        {
          slcan_nack();
          digitalWrite(YELLOW_LED, HIGH);
#ifdef DEBUG
          Serial.println("twai_start() failed");
#endif
        }
        else
        {
          // Install and start successfull
          working = true;
          slcan_ack();
          digitalWrite(BLUE_LED, HIGH);
          delay(100);
          digitalWrite(BLUE_LED, LOW);
        }
      }
      else
      {
        slcan_nack();
        digitalWrite(YELLOW_LED, HIGH);
#ifdef DEBUG
        Serial.println("twai_driver_install() failed");
#endif
      }
    }
    break;
  case 'C':      // Close CAN channel
    if (working) // check if working
    {
      if (twai_stop() != ESP_OK) // stop TWAI driver
      {
        slcan_nack();
        digitalWrite(YELLOW_LED, HIGH);
#ifdef DEBUG
        Serial.println("twai_stop() failed");
#endif
      }
      else
      {
        if (twai_driver_uninstall() != ESP_OK) // Uninstall TWAI driver
        {
          slcan_nack();
          digitalWrite(YELLOW_LED, HIGH);
#ifdef DEBUG
          Serial.println("twai_driver_uninstall() failed");
#endif
        }
        else
        {
          // Stop and uninstall successfull
          working = false;
          slcan_ack();
          digitalWrite(BLUE_LED, HIGH);
          delay(100);
          digitalWrite(BLUE_LED, LOW);
          delay(100);
          digitalWrite(BLUE_LED, HIGH);
          delay(100);
          digitalWrite(BLUE_LED, LOW);
        }
      }
    }

    break;
  case 't': // SEND STD FRAME
    // send_canmsg(buf, false, false);
    slcan_ack();
    break;
  case 'T': // SEND EXT FRAME
    // send_canmsg(buf, false, true);
    slcan_ack();
    break;
  case 'r': // SEND STD RTR FRAME
    // send_canmsg(buf, true, false);
    slcan_ack();
    break;
  case 'R': // SEND EXT RTR FRAME
    // send_canmsg(buf, true, true);
    slcan_ack();
    break;
  case 'Z': // ENABLE TIMESTAMPS
    switch (buf[1])
    {
    case '0': // TIMESTAMP OFF
      timestamp = false;
      slcan_ack();
      break;
    case '1': // TIMESTAMP ON
      timestamp = true;
      slcan_ack();
      break;
    default:
      break;
    }
    break;
  case 'M': /// set ACCEPTANCE CODE ACn REG
    slcan_ack();
    break;
  case 'm': // set ACCEPTANCE CODE AMn REG
    slcan_ack();
    break;
  case 's': // CUSTOM CAN bit-rate
    slcan_nack();
#ifdef DEBUG
    Serial.println("Custom CAN bit-rate not supported");
#endif
    break;
  case 'S': // CAN bit-rate
    if (working)
    {
      break;
    }
    else
    {
      switch (buf[1])
      {
      case '0': // 10k
        t_config = TWAI_TIMING_CONFIG_10KBITS();
        slcan_ack();
        break;
      case '1': // 20k
        t_config = TWAI_TIMING_CONFIG_20KBITS();
        slcan_ack();
        break;
      case '2': // 50k
        t_config = TWAI_TIMING_CONFIG_50KBITS();
        slcan_ack();
        break;
      case '3': // 100k
        t_config = TWAI_TIMING_CONFIG_100KBITS();
        slcan_ack();
        break;
      case '4': // 125k
        t_config = TWAI_TIMING_CONFIG_125KBITS();
        slcan_ack();
        break;
      case '5': // 250k
        t_config = TWAI_TIMING_CONFIG_250KBITS();
        slcan_ack();
        break;
      case '6': // 500k
        t_config = TWAI_TIMING_CONFIG_500KBITS();
        slcan_ack();
        break;
      case '7': // 800k
        t_config = TWAI_TIMING_CONFIG_800KBITS();
        slcan_ack();
        break;
      case '8': // 1Mbit
        t_config = TWAI_TIMING_CONFIG_1MBITS();
        slcan_ack();
        break;
      default:
        slcan_nack();
        digitalWrite(YELLOW_LED, HIGH);
#ifdef DEBUG
        Serial.println("Invalid CAN bit-rate");
#endif
        break;
      }
    }
    break;
  case 'F': // STATUS FLAGS
    Serial.print("F00");
    slcan_ack();
    break;
  case 'V': // VERSION NUMBER
    Serial.print("V1234");
    slcan_ack();
    break;
  case 'N': // SERIAL NUMBER
    Serial.print("N2208");
    slcan_ack();
    break;
  default:
    slcan_nack();
    digitalWrite(YELLOW_LED, HIGH);
#ifdef DEBUG
    Serial.println("Invalid command");
#endif
    break;
  }
} // pars_slcancmd()

//----------------------------------------------------------------

void transfer_tty2can()
{
  int ser_length;
  static char cmdbuf[32];
  static int cmdidx = 0;
  if ((ser_length = Serial.available()) > 0)
  {
    for (int i = 0; i < ser_length; i++)
    {
      char val = Serial.read();
      cmdbuf[cmdidx++] = val;
      if (cmdidx == 32)
      {
        slcan_nack();
        cmdidx = 0;
      }
      else if (val == '\r')
      {
        cmdbuf[cmdidx] = '\0';
        pars_slcancmd(cmdbuf);
        cmdidx = 0;
      }
    }
  }
} // transfer_tty2can()

//----------------------------------------------------------------

void transfer_can2tty()
{
  twai_message_t rx_frame;
  String command = "";
  long time_now = 0;
  // receive next CAN frame from queue
  if (twai_receive(&rx_frame, 0) == ESP_OK)
  {
    // do stuff!
    if (working)
    {
      digitalWrite(BLUE_LED, HIGH);
      if (rx_frame.extd)
      {
        // Message is extended
        if (rx_frame.rtr)
        {
          // Message is Remote Transmission Request
          command = command + "R";
        }
        else
        {
          // Message is Data Frame
          command = command + "T";
        }
        command = command + char(hexval[(rx_frame.identifier >> 28) & 1]);
        command = command + char(hexval[(rx_frame.identifier >> 24) & 15]);
        command = command + char(hexval[(rx_frame.identifier >> 20) & 15]);
        command = command + char(hexval[(rx_frame.identifier >> 16) & 15]);
        command = command + char(hexval[(rx_frame.identifier >> 12) & 15]);
        command = command + char(hexval[(rx_frame.identifier >> 8) & 15]);
        command = command + char(hexval[(rx_frame.identifier >> 4) & 15]);
        command = command + char(hexval[rx_frame.identifier & 15]);
        command = command + char(hexval[rx_frame.data_length_code]);
      }
      else
      {
        // Message is standard
        if (rx_frame.rtr)
        {
          // Message is Remote Transmission Request
          command = command + "r";
        }
        else
        {
          // Message is Data Frame
          command = command + "t";
        }
        command = command + char(hexval[(rx_frame.identifier >> 8) & 15]);
        command = command + char(hexval[(rx_frame.identifier >> 4) & 15]);
        command = command + char(hexval[rx_frame.identifier & 15]);
        command = command + char(hexval[rx_frame.data_length_code]);
      }
      for (int i = 0; i < rx_frame.data_length_code; i++)
      {
        command = command + char(hexval[rx_frame.data[i] >> 4]);
        command = command + char(hexval[rx_frame.data[i] & 15]);
        // printf("%c\t", (char)rx_frame.data.u8[i]);
      }
      if (timestamp)
      {
        time_now = millis() % 60000;
        command = command + char(hexval[(time_now >> 12) & 15]);
        command = command + char(hexval[(time_now >> 8) & 15]);
        command = command + char(hexval[(time_now >> 4) & 15]);
        command = command + char(hexval[time_now & 15]);
      }
      command = command + '\r';
      Serial.print(command);
      digitalWrite(BLUE_LED, LOW);
    }
  }
} // transfer_can2tty()

void setup()
{
  Serial.begin(BAUDRATE);
  Serial.setTxTimeoutMs(0);
  delay(100);

  pinMode(CAN_RS, OUTPUT);
  digitalWrite(CAN_RS, LOW);
  // setup LED
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX, (gpio_num_t)CAN_RX, TWAI_MODE_LISTEN_ONLY);
  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  delay(2000);
} // setup()

//----------------------------------------------------------------

void loop()
{
  transfer_can2tty();
  transfer_tty2can();
}