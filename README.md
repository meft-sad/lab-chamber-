# lab-chamber-
## Final project of Data Acquisition Systems IST 

### Anna Toschi

## Introduction
This project consists in the realization of a set of controls that can be implemented in a laboratory chamber before a scientific experiment and it is realized using Arduino UNO and some sensors.

During an experiment it can be very important that the support on which the sample and the machines are located, like a table of the lab, is very stable, or for example that nobody is too close to the table because of safety reasons, both of the people and the sample. For this reason a sonar sensor is used in order to measure the distance from sample and if this distance is too small a red LED turns on. 

Sometimes it can also be very important that the temperature inside the laboratory is within a specific range because of the sample that is under analysis. For this reason a temperature sensor is used in order to control that the temperature is compatible with the requests. In case it is not, an LED will turn on.



## Distance

The first part of the project consists in measuring the distance of an object/person from the table on which the sample and all the machines are located. In order to do this an ultrasonic sensor HC-SR04 has been used. This sensor emits an ultrasound which travels through the air and if there is an object or obstacle on its path it will bounce back to the source. Considering the travel time and the speed of the sound it is possible to calculate the distance. 
The HC-SR04 Ultrasonic sensor has 4 pins: Ground, VCC, Trig and Echo. The Ground and the VCC pins of the sensor are connected to the Ground and the 5 volts pins on the Arduino Board respectively. When the trigger pin is high the sensor creates an 8 cycle burst of ultrasound at 40 kHz and when this ultrasonic signal finds an obstacle it is refelected towards the sensor. The echo pin becomes high and stays high for a duration given by the time in which the signal is sent and returns to the sensor and it is proportional to the distance that needs to be measured.
Therefore, in order to measure the distance between the obstacle and the sensor it is necessary to calculate the time of travel of the signal, that means the time in which the echo stays high. To to do the Input Caputure mode of the Timer 1 of Arduino UNO has been used. It is a 16-bits timer which incorporate an input capture unit that can capture external events and give them a time-stamp indicating the time of occurrence. When a change of the logic level occurs on the input capture pin, which corresponds to Pin 8 or Arduino (PB0), the 16-bits value of the counter TCNT1 is written inot the ICR1 register. The echo pin is connected to the input capture pin, while the trigger pin can be connected to any I/O pin of Arduino.
A signal of 10 µs is sent by setting the trigger pin high for this time interval. It is possible to decide which edge on the input capture pin is used to trigger a capture event: initially this set to the rising edge so that when the echo pin becomes high the value of the counter is saved in a variable and afterward it is set to the falling edge so that when the echo pin becomes low the second value of the counter is saved. 


<img src="trig-echo.png"
     alt="Trigger and Echo signals."
     style="float: left; margin-right: 10px;" />


By making the difference between those two values and divide by the clock frequency, which depends on the crystal inside Arduino and in this case is 16 MHz, it is possible to get the time in which the signal has travel.
By multiplying for the speed of sound which at 20°C is roughly 343 m/s and dividing by two, since we need to take into account that the signal crosses twice the distance between the obstacle and the sensor, it is possible to get the distance:

distance = (time_interval* speed_sound)/2


In end end when the distance is below a critical value an LED is turned on. 
Le't notice from the code that an initial signal is sent in the setup and then it is sent again only when the prevoius one is recevied and the corresponding distance has been calculated. In this way it is possible to avoid the superposition of two signals. 
