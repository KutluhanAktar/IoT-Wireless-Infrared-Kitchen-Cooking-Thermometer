         /////////////////////////////////////////////  
        //     IoT Wireless Infrared Kitchen       //
       //          (Cooking) Thermometer          //
      //             ---------------             //
     //         (Arduino Nano & ESP8266)        //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// Monitor and track temperature remotely while cooking foods with pots, pans, saucepans, woks, etc., on Tuya Cloud compatible w/ Android and iOS.
//
// For more information:
// https://www.theamplituhedron.com/projects/IoT_Wireless_Infrared_Kitchen_(Cooking)_Thermometer/
//
//
// Connections
// Arduino Nano :  
//                                NodeMCU V3 LoLin ESP8266
// D8  --------------------------- TX 
// D9  --------------------------- RX
//                                DFRobot TS01 Non-contact IR Temperature Sensor
// A0  --------------------------- S     
//                                5mm Common Anode RGB LED
// D3  --------------------------- R
// D5  --------------------------- G
// D6  --------------------------- B
//                                Network Connection Button
// D7  --------------------------- S


// Include the required libraries.
#include <TuyaWifi.h>
#include <SoftwareSerial.h>

// Define the Tuya Device:
SoftwareSerial conn(8, 9); // RX, TX
TuyaWifi my_device(&conn);

// Define Tuya Device connection status settings:
unsigned char led_state = 0;
int wifi_key_pin = 7;

// Define data points (DPs) of the Tuya Device:
#define DPID_Cook_Temperature 101
#define DPID_Cook_Temperature_F 102

// Stores all DPs and their types. PS: array[][0]:dpid, dp type(TuyaDefs.h) : DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP
unsigned char dp_array[][2] =
{
  {DPID_Cook_Temperature, DP_TYPE_VALUE},
  {DPID_Cook_Temperature_F, DP_TYPE_VALUE},
};

// Define the Tuya Device Information:
unsigned char pid[] = {"Enter_PID"};
unsigned char mcu_ver[] = {"3.1.4"};

// Define the IR Temperature Sensor pin:
#define IR_Sensor A0

// Define RGB pins:
#define redPin 3
#define greenPin 5
#define bluePin 6

// Define the data holders:
unsigned long last_time = 0;
int tem_celsius, tem_fahrenheit;

void setup() {
  Serial.begin(9600);
  conn.begin(9600);
  
  // RGB:
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  adjustColor(0,0,0);
    
  // Initialize networking keys.
  pinMode(wifi_key_pin, INPUT_PULLUP);
  // Enter the PID and MCU software version:
  my_device.init(pid, mcu_ver);
  // Incoming all DPs and their types array, DP numbers:
  my_device.set_dp_cmd_total(dp_array, 2);
  // Register DP download processing callback function:
  my_device.dp_process_func_register(dp_process);
  // Register upload all DP callback function:
  my_device.dp_update_all_func_register(dp_update_all);
  // Define the last time:
  last_time = millis();

}

void loop() {
  // Start the serial communication with the Tuya Device:
  my_device.uart_service();
  // Activate the network connection mode when the network connection button (WiFi Key Pin) is pressed.
  if (digitalRead(wifi_key_pin) == LOW) {
    delay(80);
    if (digitalRead(wifi_key_pin) == LOW) {
      my_device.mcu_set_wifi_mode(SMART_CONFIG);
    }
  }
  // RGB LED blinks green when the Tuya Device is attempting to connect to the network:
  if((my_device.mcu_get_wifi_work_state() != WIFI_LOW_POWER) && (my_device.mcu_get_wifi_work_state() != WIFI_CONN_CLOUD) && (my_device.mcu_get_wifi_work_state() != WIFI_SATE_UNKNOW)) {
    if(millis()- last_time >= 500){
      last_time = millis();
      // Adjust:
      if(led_state == LOW){ led_state = HIGH; adjustColor(0,255,0); } else{ led_state = LOW; adjustColor(0,0,0); }
    }
  }
  // If the Tuya Device (ESP8266) is connected to the cloud server:
  if(my_device.mcu_get_wifi_work_state() == WIFI_CONN_CLOUD){
    adjustColor(0,0,255);
  }
  delay(10);

  // Get temperature measurements from the IR sensor and update the Tuya Cloud.
  get_data_from_IR_sensor();
  my_device.mcu_dp_update(DPID_Cook_Temperature, tem_celsius, 1);
  my_device.mcu_dp_update(DPID_Cook_Temperature_F, tem_fahrenheit, 1);

}

unsigned char dp_process(unsigned char dpid,const unsigned char value[], unsigned short length){
  /* all DP only report */
  return SUCCESS;
}

void dp_update_all(void){
  // Update all DPs with default values:
  my_device.mcu_dp_update(DPID_Cook_Temperature, 1, 1);
  my_device.mcu_dp_update(DPID_Cook_Temperature_F, 50, 1);
}

void get_data_from_IR_sensor(){
  unsigned int ADC_Value = analogRead(IR_Sensor);
  tem_celsius = ((double)ADC_Value*450/614.4-70);
  tem_fahrenheit = (tem_celsius * 1.8) + 32;
  Serial.print("Temperature: ");
  Serial.print(tem_celsius);
  Serial.println("\u2103");
  delay(100); 
}

void adjustColor(int r, int g, int b){
  analogWrite(redPin, (255-r));
  analogWrite(greenPin, (255-g));
  analogWrite(bluePin, (255-b));
}
