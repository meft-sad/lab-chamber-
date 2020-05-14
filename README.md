# Lab-chamber
## Final project of Data Acquisition Systems IST 

### Anna Toschi

## Introduction
This project consists in the realization of a set of controls that can be implemented in a laboratory chamber before a scientific experiment and it is realized using Arduino UNO and some sensors.

During an experiment it can be very important that the support on which the sample and the machines are located, like a table of the lab, is very stable, or for example that nobody is too close to the table because of safety reasons, both of the people and the sample. For this reason a sonar sensor is used in order to measure the distance from sample and if this distance is too small a red LED turns on. 

Sometimes it can also be very important that the temperature inside the laboratory is within a specific range because of the sample that is under analysis. For this reason a temperature sensor is used in order to control that the temperature is compatible with the requests. In case it is not, an LED will turn on.

Many times in the scientific laboratories there are some inflammable materials which can be very dangerous. It is very important that the laboratory is safe, for this reason a flame sensor will be used to detect the possible presence of a flames. 

## Sensors:

1) HC-SR04 ultrasonic sensor: https://www.electroschematics.com/wp-content/uploads/2013/07/HCSR04-datasheet-version-1.pdf
2) KY-013 temperature sensor: https://datasheetspdf.com/pdf-file/1402026/Joy-IT/KY-013/1
3) KY-026 flame sensor: https://datasheet4u.com/datasheet-parts/KY-026-datasheet.php?id=1402037

## Distance

The first part of the project consists in measuring the distance of an object/person from the table on which the sample and all the machines are located. In order to do this an ultrasonic sensor HC-SR04 has been used. This sensor emits an ultrasound which travels through the air and if there is an object or obstacle on its path it will bounce back to the source. Considering the travel time and the speed of the sound it is possible to calculate the distance. 
The HC-SR04 Ultrasonic sensor has 4 pins: Ground, VCC, Trig and Echo. The Ground and the VCC pins of the sensor are connected to the Ground and the 5 volts pins on the Arduino Board respectively. When the trigger pin is high the sensor creates an 8 cycle burst of ultrasound at 40 kHz and when this ultrasonic signal finds an obstacle it is refelected towards the sensor. The echo pin becomes high and stays high for a duration given by the time in which the signal is sent and returns to the sensor and it is proportional to the distance that needs to be measured.
Therefore, in order to measure the distance between the obstacle and the sensor it is necessary to calculate the time of travel of the signal, that means the time in which the echo stays high. To to do the Input Caputure mode of the Timer 1 of Arduino UNO has been used. It is a 16-bits timer which incorporate an input capture unit that can capture external events and give them a time-stamp indicating the time of occurrence. When a change of the logic level occurs on the input capture pin, which corresponds to Pin 8 or Arduino (PB0), the 16-bits value of the counter TCNT1 is written inot the ICR1 register. The echo pin is connected to the input capture pin, while the trigger pin can be connected to any I/O pin of Arduino.
A signal of 10 µs is sent by setting the trigger pin high for this time interval. It is possible to decide which edge on the input capture pin is used to trigger a capture event: initially this set to the rising edge so that when the echo pin becomes high the value of the counter is saved in a variable and afterward it is set to the falling edge so that when the echo pin becomes low the second value of the counter is saved. 



By making the difference between those two values and divide by the clock frequency, which depends on the crystal inside Arduino and in this case is 16 MHz, it is possible to get the time in which the signal has travel.
By multiplying for the speed of sound which at 20°C is roughly 343 m/s and dividing by two, since we need to take into account that the signal crosses twice the distance between the obstacle and the sensor, it is possible to get the distance:

distance = (time_interval* speed_sound) / 2


In end end when the distance is below a critical value an LED is turned on. 
Le't notice from the code that an initial signal is sent in the setup and then it is sent again only when the prevoius one is recevied and the corresponding distance has been calculated. In this way it is possible to avoid the superposition of two signals. 

## Temperature
### Sensor 
In order to measure the temperature of the room an NTC thermistor has been used.
Thermistors are variable resistors that change their resistance with temperature. In particular the sensor used is called KY-013 which consists of a NTC thermistor and a 10 kΩ resistor.

The operating voltage is 5V and it allows temperature measurements within the range of -55°C and 125°C with an accuracy of ±0.5°C.

The voltage across the thermistor is the analog input that can be measured. From the picture it is possible to notice that the circuit is a voltage divider: 

Vout = Vref R2/(R1+R2)

Where R1 is the known resistance, that in this case is 10kΩ, while R2 is the variable resistance of the thermistor and Vref in this case is 5 V supply voltage. In this way it is possible to find the resistance R2 of the thermistori, knowing the value of the voltage across it.

R2 = R1(Vref/Vout-1)

This quantity is related to the temperature through the Steinhart–Hart equation:

1/T = A + B lnR + C (lnR)^3

Where A, B and C are the Steinhart-Hart coefficients which vary depending on the type and model of thermistor and the temperature range of interest. In this case:

A = 0.001129148
B = 0.000234125
C = 0.000000087674

At room temperature the value of the voltage is between 2 and 3 V.
### Look-up Table
Since Arduino, for complicated operations like logarithms, is not very efficient, instead of calculating the temperature using the Steinhart–Hart equation, the temperature is calculated using look-up tables. In particular a finite number of values of the voltage are stored in an array and the corresponding values of temperature are calculated outside Arduino (in Matlab) and an array for those values of temperature is created. In order to find the other values of temperature a linear interpolation is performed. 
In particular this introduces an error in the calculation of temperature. 
This has as advantage to reduce the computational cost of the calculation of the temperature but it introduces an error. The maximu value of the error introduced is 0.8587°C.

## Flame
The sensor KY-026 is used. It is principally composed by an Infra-red sensor: when fire burns it emits a small amount of infra-red light, this light will be received by the Photodiode on the sensor module. 
