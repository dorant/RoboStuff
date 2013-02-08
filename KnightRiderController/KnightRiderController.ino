#include <Wire.h>

#define I2C_SLAVE_ADDR  0x26            // i2c slave address

int ledPin = 13; 

void setup()
{
  Wire.begin();
  pinMode(ledPin, OUTPUT);
}


void loop ()
{
  digitalWrite(ledPin, HIGH); 
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(0x01);
  Wire.endTransmission();
  delay(100);
  digitalWrite(ledPin, LOW);
  
  delay(2000);

  digitalWrite(ledPin, HIGH); 
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(0x64);
  Wire.endTransmission();
  delay(100);
  digitalWrite(ledPin, LOW);

  delay(2000);

  digitalWrite(ledPin, HIGH); 
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(100);
  digitalWrite(ledPin, LOW);

  delay(2000);
}
