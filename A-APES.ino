#include <Time.h> // to keep track of the time
#include <TimeLib.h> // to keep track of the time
#include <Wire.h> // for the ADC
#include <Adafruit_ADS1015.h> // for the ADC
#include <millisDelay.h> // for measuring and venting control
#include <SoftwareSerial.h> // for XBEE communication

// pins that control the venting
const int rigpin1 = 3;
const int rigpin2 = 2;
const int rigpin3 = 4;
const int rigpin4 = 5;

// pins that control the xbee communication
const int xbdout = 11;
const int xbdin = 10;

// SETTINGS
const int ventfrequency1 = 5; // UNITS IN MINUTES: vent frequency
const int ventfrequency2 = 5; // UNITS IN MINUTES: vent frequency
const int ventfrequency3 = 10000; // UNITS IN MINUTES: vent frequency
const int ventfrequency4 = 10000; // UNITS IN MINUTES: vent frequency

const int measurementfrequency = 1; // UNITS IN MINUTES: measurement frequency
const unsigned long venttime = 7000; // UNITS IN MILLISECONDS: time each reactor vents to atmosphere
const unsigned long readinterval = 50; // UNITS IN MILLISECONDS
const int numintervals = 20; // pressure measurements will be averaged NUMINTERVALS times every READINTERVAL. It is important that numintervals < 100.

int counter;
unsigned long int pressures[4] = {0, 0, 0, 0};

SoftwareSerial xbee(xbdout, xbdin); // xbee(RX, TX) with Arduino RX -> XBee Dout, Arduino TX -> XBee Din
Adafruit_ADS1115 ads1115(0x48); // the default address of Adafruit ADC on the I2C interface

millisDelay ventingDelay1; // non-blocking delay for venting
millisDelay ventingDelay2; // non-blocking delay for venting
millisDelay ventingDelay3; // non-blocking delay for venting
millisDelay ventingDelay4; // non-blocking delay for venting

millisDelay measurementDelay; // non-blocking delay for measurements
millisDelay openDelay1; // non-blocking delay for venting the system to atmosphere
millisDelay openDelay2; // non-blocking delay for venting the system to atmosphere
millisDelay openDelay3; // non-blocking delay for venting the system to atmosphere
millisDelay openDelay4; // non-blocking delay for venting the system to atmosphere

millisDelay readDelay; // non-blocking delay for taking multiple readings

void setup() {
  ads1115.begin();  // initialize ads1115
  xbee.begin(9600); // initialize the xbee
  Serial.begin(9600); //  initialize a serial communication stream for debugging

  pinMode(rigpin1, OUTPUT); // vent when high
  pinMode(rigpin2, OUTPUT); // vent when high
  pinMode(rigpin3, OUTPUT); // vent when high
  pinMode(rigpin4, OUTPUT); // vent when high
  zeroRigs(); // vent the system at time = 0
  delay(1000);

  setTime(0, 0, 0, 1, 1, 2020); // set system time
  ventingDelay1.start((unsigned long) 1000 * 60 * ventfrequency1); //  input unit to function is milliseconds
  ventingDelay2.start((unsigned long) 1000 * 60 * ventfrequency2); //  input unit to function is milliseconds
  ventingDelay3.start((unsigned long) 1000 * 60 * ventfrequency3); //  input unit to function is milliseconds
  ventingDelay4.start((unsigned long) 1000 * 60 * ventfrequency4); //  input unit to function is milliseconds
  
  measurementDelay.start((unsigned long) 1000 * 60 * measurementfrequency); //  input unit to function is milliseconds
  Serial.println("System done initializing.");
  xbee.println("System done initializing.");
}


void loop() {
  // measure first
  if (measurementDelay.justFinished()) 
    {
      measurementDelay.repeat();
      readDelay.start(readinterval);
      for (counter=0; counter<numintervals; counter++)
      {
        while (true) // waste time
        {
          if (readDelay.justFinished()) 
          {
            break;
          }
        }
        pressures[0] += (unsigned long) ads1115.readADC_SingleEnded(0);
        pressures[1] += (unsigned long) ads1115.readADC_SingleEnded(1);
        pressures[2] += (unsigned long) ads1115.readADC_SingleEnded(2);
        pressures[3] += (unsigned long) ads1115.readADC_SingleEnded(3);
        readDelay.restart();
      }
      transmitMeasurements();
      // reset pressure measurements
      pressures[0] = 0;
      pressures[1] = 0;
      pressures[2] = 0;
      pressures[3] = 0;
    }
    
  // Vent sequentially
  if (ventingDelay1.justFinished()) 
    {
      ventingDelay1.repeat(); // restart timer
      openRigValve(rigpin1); // open valve
      openDelay1.start(venttime); //  restart timer
      while (true) 
      {
        if (openDelay1.justFinished()) 
        {
          break;
        }
      }
      closeRigValve(rigpin1);
      xbee.println("VENTED-R1");
      Serial.println("VENTED-R1");
    }

  if (ventingDelay2.justFinished()) 
  {
    ventingDelay2.repeat(); // restart timer
    openRigValve(rigpin2); // open valve
    openDelay2.start(venttime); //  restart timer
    while (true) 
    {
      if (openDelay2.justFinished()) 
      {
        break;
      }
    }
    closeRigValve(rigpin2);
    xbee.println("VENTED-R2");
    Serial.println("VENTED-R2");
  }

  if (ventingDelay3.justFinished()) 
  {
    ventingDelay3.repeat(); // restart timer
    openRigValve(rigpin3); // open valve
    openDelay3.start(venttime); //  restart timer
    while (true) 
    {
      if (openDelay3.justFinished()) 
      {
        break;
      }
    }
    closeRigValve(rigpin3);
    xbee.println("VENTED-R3");
    Serial.println("VENTED-R3");
  }

  if (ventingDelay4.justFinished()) 
  {
    ventingDelay4.repeat(); // restart timer
    openRigValve(rigpin4); // open valve
    openDelay4.start(venttime); //  restart timer
    while (true) 
    {
      if (openDelay4.justFinished()) 
      {
        break;
      }
    }
    closeRigValve(rigpin4);
    xbee.println("VENTED-R4");
    Serial.println("VENTED-R4");
  }
}


void transmitMeasurements() {
  time_t t = now();
  int rig1, rig2, rig3, rig4;
  rig1 = (float) pressures[0] / (float) numintervals;
  rig2 = (float) pressures[1] / (float) numintervals;
  rig3 = (float) pressures[2] / (float) numintervals;
  rig4 = (float) pressures[3] / (float) numintervals;

  // Debug print the readings to the serial
  Serial.print("T=");
  Serial.print(day(t));
  Serial.print(",");
  Serial.print(hour(t));
  Serial.print(",");
  Serial.print(minute(t));
  Serial.print("#");
  Serial.print("R1=");
  Serial.print(rig1);
  Serial.print("#");
  Serial.print("R2=");
  Serial.print(rig2);
  Serial.print("#");
  Serial.print("R3=");
  Serial.print(rig3);
  Serial.print("#");
  Serial.print("R4=");
  Serial.println(rig4);

  // Send measurements
  xbee.print(day(t)); // assume will run for less than a week
  xbee.print(",");
  xbee.print(hour(t));
  xbee.print(",");
  xbee.print(minute(t));
  xbee.print("#");
  xbee.print(rig1);
  xbee.print("#");
  xbee.print(rig2);
  xbee.print("#");
  xbee.print(rig3);
  xbee.print("#");
  xbee.println(rig4);
}

void openRigValve(int rigpin) {
  digitalWrite(rigpin, HIGH);
}

void closeRigValve(int rigpin) {
  digitalWrite(rigpin, LOW);
}

void zeroRigs() {
  openRigValve(rigpin1);
  delay(5000); //  the use of delay is okay here because this block of code is used before the timing library is initialized.
  closeRigValve(rigpin1);
  openRigValve(rigpin2);
  delay(5000);
  closeRigValve(rigpin2);
  openRigValve(rigpin3);
  delay(5000); 
  closeRigValve(rigpin3);
  openRigValve(rigpin4);
  delay(5000);
  closeRigValve(rigpin4);
}
