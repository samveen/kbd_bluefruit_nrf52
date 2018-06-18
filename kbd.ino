/*********************************************************************
 This code is for the Bluefruit nRF52 Feather and started off as the
 Adafruit nRF52 Bluefruit LE HID example. Much mutated since then.
*********************************************************************/
#include <bluefruit.h>

BLEDis bledis;
BLEHidAdafruit blehid;

bool hasKeyPressed = false;

char keyIndex=(char)32; /* 32 to 126 */

#define GPIO(x) (x)

/* The keyboard matrix is designed as a pair of matrices with the same
write pins, but different read pins. For implementation purposes, the
read pins can be considered as either:
 - 1 set of 10 read pins (simpler implementation)
 - 2 sets of 5 read pins each (easier to envision for me)
*/

/* Pins to read from:
 * We need write 10 pins, but we have 11
 * sideways layout: Top row is Right side of the board */
uint32_t pins_read [] = {
                               GPIO(16), GPIO(15), GPIO( 7), GPIO(11),
    GPIO(2), GPIO(3), GPIO(4), GPIO( 5), GPIO(28), GPIO(29), GPIO(12),
};

/* Pins to write to:
 * We need write 7 pins, but we have 9 (Overkill?)
 * Sideways layout: Top row is Right side of the board */
uint32_t pins_write [] = { /* P31(VBAT ADC) is separator */
              GPIO(30), GPIO(27), GPIO(26), GPIO(25),
    GPIO(13), GPIO(14), GPIO( 8), GPIO( 6), GPIO(20),
};

void init_keymatrix()
{
  /* Source: https://gist.github.com/mitchtech/2865219 */
  int32_t i=0;

  // Read pins in Input mode, and High state
  for(;i<(sizeof(pins_read)/sizeof(pins_read[0]));++i){
    pinMode(pins_read[i], INPUT);
    // Enable internal pull-ups
    digitalWrite(pins_read[i], 1);
  }

  for(i=0;i<(sizeof(pins_write)/sizeof(pins_write[0]));++i){
    pinMode(pins_write[i], OUTPUT);
  }
}

void setup()
{
  init_keymatrix();
  Serial.begin(115200);

  Serial.println("Bluefruit52 HID Keyboard Example");
  Serial.println("--------------------------------\n");

  Serial.println();
  Serial.println("Go to your phone's Bluetooth settings to pair your device");
  Serial.println("then open an application that accepts keyboard input");

  Serial.println();
  Serial.println("Enter the character(s) to send:");
  Serial.println();

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(-20);
  Bluefruit.setName("MyBtKbd");

  // Configure and Start Device Information Service
  bledis.setManufacturer("samveen.in");
  bledis.setModel("MBK101");
  bledis.begin();

  /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
   * connection interval to 11.25  ms and 15 ms respectively for best performance.
   */
  blehid.begin();

  /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
   */
  /* Bluefruit.setConnInterval(9, 12); */

  // Set up and start advertising
  startAdv();
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   *
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(30);               // 0 = Don't stop advertising after n seconds
}

char keys[6]="";
uint8_t keycount=0;

/* Function to scan key matrix and return the keys pressed */
uint8_t scankeys()
{
  /* Source: https://github.com/FriesFlorian/keyawesome */
}

void loop()
{
  keycount=scankeys();
  // Only send KeyRelease if previously pressed to avoid sending
  // multiple keyRelease reports (that consume memory and bandwidth)
  if ( hasKeyPressed )
  {
    hasKeyPressed = false;
    blehid.keyRelease();

    // Delay a bit after a report
    delay(1000);
  }

  char ch = (char) keyIndex;
  blehid.keyPress(ch);
  hasKeyPressed = true;
  ++keyIndex; if (keyIndex>=126){ keyIndex=(char)32; }

  // Delay a bit after a report: Don't dely too much: we are in keypress state.
  //delay(1000);

  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}

/**
 * RTOS Idle callback is automatically invoked by FreeRTOS
 * when there are no active threads. E.g when loop() calls delay() and
 * there is no bluetooth or hw event. This is the ideal place to handle
 * background data.
 *
 * NOTE: FreeRTOS is configured as tickless idle mode. After this callback
 * is executed, if there is time, freeRTOS kernel will go into low power mode.
 * Therefore waitForEvent() should not be called in this callback.
 * http://www.freertos.org/low-power-tickless-rtos.html
 *
 * WARNING: This function MUST NOT call any blocking FreeRTOS API
 * such as delay(), xSemaphoreTake() etc ... for more information
 * http://www.freertos.org/a00016.html
 */
void rtos_idle_callback(void)
{
  // Don't call any other FreeRTOS blocking API()
  // Perform background task(s) here
}
