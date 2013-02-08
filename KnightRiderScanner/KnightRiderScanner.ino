#include "TinyWireS.h"                  // wrapper class for I2C slave routines

#define I2C_SLAVE_ADDR  0x26            // i2c slave address

// LED processor connections left to right (LED D0-D8)  
// Port pins: D2 D3 D4 D5 D6 B0 B1 B2 B3

// PortD
#define LOWER_LED_PORT      PORTD
#define NUM_LOWER_LEDS      5
#define LED0_BIT            0x04
#define LED1_BIT            0x08
#define LED2_BIT            0x10
#define LED3_BIT            0x20
#define LED4_BIT            0x40

// PortB
#define UPPER_LED_PORT      PORTB
#define NUM_UPPER_LEDS      4
#define LED5_BIT            0x01
#define LED6_BIT            0x02
#define LED7_BIT            0x04
#define LED8_BIT            0x08

// Limits for the position
#define POSITION_INITIAL        1
#define POSITION_LIMIT          9

// Direction that the LEDs are strobing
#define DIR_MOVING_RIGHT        1 // from front view
#define DIR_MOVING_LEFT         -1

// Brightness options
#define MAX_BRIGHTNESS          100
#define HIGH_BRIGHTNESS         85
#define MEDIUM_HIGH_BRIGHTNESS  70
#define MEDIUM_BRIGHTNESS       50
#define MEDIUM_LOW_BRIGHTNESS   30
#define LOW_BRIGHTNESS          15
#define NO_BRIGHTNESS           0

#define NUM_TOT_LEDS            9
#define MAX_LED_INDEX           (NUM_TOT_LEDS-1)

// A way to adjust the timing, higher equals slower
//#define DEFAULT_SLOWNESS        0
//#define DEFAULT_SLOWNESS        2000
#define DEFAULT_SLOWNESS        3800


// Storage for application settings
typedef struct _LarsonSettings
{
  int slowness;
} LarsonSettings;

// Storage for application data
typedef struct _LarsonRuntimeData
{
  int8_t direction;
  uint8_t position;
  uint8_t led_brightness[NUM_TOT_LEDS];
} LarsonRuntimeData;

// Static variables
static LarsonRuntimeData _data;
static LarsonSettings _settings;

// store bits in array for easy loop updates
static const uint8_t lower_led_bits[] = {LED0_BIT, LED1_BIT, LED2_BIT, LED3_BIT, LED4_BIT};
static const uint8_t upper_led_bits[] = {LED5_BIT, LED6_BIT, LED7_BIT, LED8_BIT};

// Update ports where the leds are connected
void updateLeds(void)
{
  static uint8_t cnts = 0;
  uint8_t *led_ptr = &_data.led_brightness[0];

  //uint8_t lowerPort = 0;
  for(uint8_t i=0; i < NUM_LOWER_LEDS; ++i)
  {
    if(*led_ptr++ > cnts)
    {
      LOWER_LED_PORT |= lower_led_bits[i];
      //lowerPort |= lower_led_bits[i];
    }
    else
    {
      LOWER_LED_PORT &= ~lower_led_bits[i];
      //lowerPort &= ~lower_led_bits[i];
    }
  }  
  //LOWER_LED_PORT = lowerPort;

  //uint8_t upperPort = 0;
  for(uint8_t i=0; i < NUM_UPPER_LEDS; ++i)
  {
    if(*led_ptr++ > cnts)
    {
      UPPER_LED_PORT |= upper_led_bits[i];
      //upperPort |= upper_led_bits[i];
    }
    else
    {
      UPPER_LED_PORT &= ~upper_led_bits[i];
      //upperPort &= ~upper_led_bits[i];
    }
  }
//  UPPER_LED_PORT = upperPort;

  cnts++;
  if(cnts > MAX_BRIGHTNESS)
  {
    cnts = 0;
  }
}




/**
 * Displays something similar to KITT from Knight Rider.
 */
#define KNIGHT_RIDER_BRIGHTNESS_DELTA   5
void update_led_brightness_knight_rider(void)
{
  int8_t center_led_index = _data.position-1;
  int8_t low_index = center_led_index - 1;
  int8_t high_index = center_led_index + 1;
  int8_t low_brightness = LOW_BRIGHTNESS;
  int8_t high_brightness = LOW_BRIGHTNESS;
  uint8_t new_brightness[NUM_TOT_LEDS];
  
  // Setup initial and center-led
  for(uint8_t i=0; i < NUM_TOT_LEDS; ++i)
  {
    new_brightness[i] = 0;
  }
  new_brightness[center_led_index] = MEDIUM_HIGH_BRIGHTNESS;

  // Setup leds left of center
  while(low_index >= 0)
  {
    if(low_brightness > 0)
    {
      new_brightness[low_index] = low_brightness;
      low_brightness -= KNIGHT_RIDER_BRIGHTNESS_DELTA;
    }
    else
    {
      new_brightness[low_index] = 0;
    }
    low_index--;
    if(_data.direction == DIR_MOVING_LEFT)
    {
      // only do one LED ahead
      break;
    }
  }

  // Setup leds right of center
  while(high_index <= MAX_LED_INDEX)
  {
    new_brightness[high_index] = high_brightness;
    if(high_brightness > 0)
    {
      new_brightness[high_index] = high_brightness;
      high_brightness -= KNIGHT_RIDER_BRIGHTNESS_DELTA;
    }
    else
    {
      new_brightness[high_index] = 0;
    }
    high_index++;
    if(_data.direction == DIR_MOVING_RIGHT)
    {
      // only do one LED ahead
      break;
    }
  }
  
  // Copy new calculated brightness
  for(uint8_t i=0; i < NUM_TOT_LEDS; ++i)
  {
    _data.led_brightness[i] = new_brightness[i];
  }
}


/**
 * Updates the LED position. For now scrolls between POSITION_INITIAL and 
 * POSITION_LIMIT.
 */
void update_position(void)
{
  _data.position += _data.direction;
  
  if (_data.position > POSITION_LIMIT)
  {
    // toggle direction
    _data.direction = -_data.direction;
    _data.position = POSITION_LIMIT - 1;
  }
  else if(_data.position < POSITION_INITIAL)
  {
    // toggle direction
    _data.direction = -_data.direction;
    _data.position = POSITION_INITIAL + 1;    
  }
}


/**
 * Initializes the board.
 */
void init_board(void)
{
  // Initialization routine: Clear watchdog timer
  // -- this can prevent several things from going wrong.               
  MCUSR  &= 0xF7; // Clear WDRF Flag
  WDTCSR = 0x18;  // Set stupid bits so we can clear timer...
  WDTCSR = 0x00;
  
  // Data direction register: DDR's
  // Port B: 0-3 are outputs, B4 is an input.   
  // Port D: 1-6 are outputs, D0 is an input.
  DDRA = 0x00;
  DDRB = 0x0F;  
  DDRD = 0x7E;
}

// Receive i2c data
void receiveEvent(uint8_t howMany)
{
  while (TinyWireS.available() > 0)
  {
    uint8_t byteRcvd = TinyWireS.receive();
    _settings.slowness = byteRcvd * 20;
  }
}

void setup()
{
  TinyWireS.begin(I2C_SLAVE_ADDR);
  TinyWireS.onReceive(receiveEvent);
    
  init_board();

  _settings.slowness = DEFAULT_SLOWNESS;
  _data.direction = DIR_MOVING_RIGHT;
  _data.position = POSITION_INITIAL;
}

void loop()
{
  // Check i2c
  TinyWireS_stop_check();

  updateLeds();

  static int loops = 0;
  if (loops++ >= _settings.slowness)
  {
    update_position();
    update_led_brightness_knight_rider();
    loops = 0;  
  }
}

