/*
 * Project iot-co2-traffic-lamp
 * Description: A device to measure the indoor climate, i.e. the CO² concentration, and to display the concentration via an LED traffic light.
 * Author: Juri Lozowoj
 * Date: 20-12-2020
 */
#include <neopixel.h>

#define SENSOR_PIN D6

// Measuring ranges according to manufacturer (0-1000, 0-2000 & 0-5000 ppm CO²)
#define RANGE 5000 // default measuring ranges
 
#define SERIAL_TRANSMISSION_RATE 115200 // according to christoph "reicht hier jeder Wert" :-)

#define PIXEL_COUNT 1						// 1 pixels on our strip
#define PIXEL_PIN D3
#define PIXEL_TYPE WS2812B			// strip uses WS2812 Pixels

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

unsigned long duration_1, duration_2;

char eventData[64];


// setup() runs once, when the device is first turned on.
void setup() {

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

    // transmission rate to serial monitor 
  Serial.begin(SERIAL_TRANSMISSION_RATE);
  // Put initialization like pinMode and begin functions here.
  pinMode(SENSOR_PIN, INPUT);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  rainbow(20);

  // The core of your code will likely live here.
  int pulse_high, pulse_high_2, pulse_low, co2_ppm, co2_corrected;
  unsigned long time_start, time_end;

  // measures the duration of a signal in microseconds
  duration_1 = pulseIn(SENSOR_PIN, HIGH);
  
  // start of the LOW-signal
  time_start = millis();
  
  pulse_high = duration_1 / 1000;

  duration_2 = pulseIn(SENSOR_PIN, HIGH);

  if (duration_1 != 0) {
    pulse_high_2 = (duration_2 / 1000);

    time_end = millis();

    pulse_low = (time_end - (time_start + pulse_high_2));
    
    co2_ppm = (((pulse_high - 2) * RANGE) / 1000);
    // corrected for measurement inaccuracy
    co2_corrected = (((pulse_high - 2) * RANGE) / (pulse_high + pulse_low - 4));

    // print to serial
    Serial.print("CO2 ppm: " + String(co2_ppm) + "; ");

    Serial.println("corrected CO2 ppm: " + String(co2_corrected) + "; ");

    publishMesaruement(co2_corrected);

    Serial.print("DURATION_1: " + String(duration_1) + "; ");

    Serial.print("DURATION_2: " + String(duration_2) + "; ");

    Serial.print("HIGH: " + String(pulse_high) + "; ");

    Serial.print("HIGH_2: " + String(pulse_high_2) + "; ");

    Serial.println("LOW: " + String(pulse_low) + "; ");

  }
  delay(5000);
}

void publishMesaruement(const int co2_ppm) {

  int timestamp = Time.now();

  JSONBufferWriter writer(eventData, sizeof(eventData));
  writer.beginObject();
      writer.name("value").value(co2_ppm);
      writer.name("timestamp").value(timestamp);
  writer.endObject();

  Particle.publish("co2_concentration", eventData, PRIVATE);
}


void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}