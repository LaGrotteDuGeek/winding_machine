/* La Bobineuse from La Grotte du Geek
 * This code controls a motor with included encoder used as a winding machine
 * A description of the system is made here (in french): https://www.youtube.com/watch?v=rHf5dyYqVNg
 * 
 * WARNING: As the precision required is low, this is not a true PID feedback control loop
 * Note that motor control and rotation measure are independants, so it is possible to turn the rotor by hand to adjust the value
 */

#define MOTOR_PIN 5
#define ENCODER_PIN_0 2
#define ENCODER_PIN_1 4
#define BUTTON_PIN 7

#define TICK_PER_ROT 32.0 //number of tick per rotation of the encoder
#define GEAR_TRAIN_RATIO 0.2 //this ratio is made after the encoder so the precision si affected by the gear

#define ROTATION_GOAL 80.0 //hard coded end of winding, should be changed

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h> //You have to download this library to use OLED Display

#define OLED_RESET 12
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

volatile long tick = 0;//volatile because used in interrupt

void setup()
{ 
  pinMode(ENCODER_PIN_0, INPUT);
  pinMode(ENCODER_PIN_1, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  digitalWrite(MOTOR_PIN, LOW);
  pinMode(MOTOR_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_0), wheelUpdate, RISING);//Resolution can be doubled using "CHANGE" and changing the TICK_PER_ROT value

  
  //Initialize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}


void loop()
{
  static float rotation = 0.0;
  static uint8_t motor_pwm = 0;
  static unsigned long pressed_time = 0;
  static unsigned long current_time = 0;

  //Update time and compute loop time
  unsigned long loop_time = millis() - current_time;
  current_time += loop_time;

  //translate tick to number of rotations
  rotation = tick * GEAR_TRAIN_RATIO / TICK_PER_ROT;


  if(!digitalRead(BUTTON_PIN))//When button is pressed, speed increases from 0 to 4s then slows 2 turns before goal and stops at goal
  {
    pressed_time += loop_time;
    if(pressed_time < 4000)
    {
      motor_pwm = (uint8_t) (pressed_time * 104 / 4000) + 150;
    }
    else if(pressed_time >= 4000)
    {
      motor_pwm = 255;
    }
    else
    {
      motor_pwm = 0;
    }

    if(rotation >= ROTATION_GOAL)
    {
      motor_pwm = 0;
    }
    else if(ROTATION_GOAL - rotation < 2)
    {
      motor_pwm *= 0.8;
    }
  }
  else
  {
    pressed_time = 0;
    motor_pwm = 0;
  }

  //set pwm order to control motor
  analogWrite(MOTOR_PIN, motor_pwm);

  //display number of rotation with 1 decimal
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.print(rotation, 1);
  
  display.display();
    
}

void wheelUpdate()
{
  //When the interrupt is reached, it means ENCODER_PIN_0 is passing from LOW to HIGH, checking ENCODER_PIN_1 state gives the rotation direction
  if(digitalRead(ENCODER_PIN_1))
  {
    ++tick;
  }
  else
  {
    --tick;
  }
}
