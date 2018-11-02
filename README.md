# rpi_guitar_effect_pedal

## Instruction
This project use Raspberry Pi to create a guitar effect pedal with a LCD monitor.        
The effects are including bypass, overdrive, delay, echo, reverb, octaver, tremolo.    
demo_video: https://ppt.cc/f6aJ2x

## Hardware
In order to solving noise issue due to raspberry_pi 5V unstable power, I added a 5V to 3.3V LDO to the circuit.   
On the other hand, I add two potentiometers for adjusting volume and effect values.    
Modified schematic is as followed.    
![Alt text](https://github.com/heyheychen/rpi_guitar_effect_pedal/blob/master/pic/rpi_pedal_schematic.png?raw=true)

## OS
At raspberry pi OS raspbian, we can use nano to edit code, gcc for complier, ./ for running application
![Alt text](https://github.com/heyheychen/rpi_guitar_effect_pedal/blob/master/pic/gcc_command_line.png?raw=true)    
note: BCM2835 library need to be installed first
  

## Software
Main code is pipedal.c  
lcd1602.c and lcd1602.h are the library files   
Software flow:    
![Alt text](https://github.com/heyheychen/rpi_guitar_effect_pedal/blob/master/pic/software%20flow.png?raw=true)
    
     
Reference:
1. Circuit and code: 
https://www.electrosmash.com/pedal-pi 
2. I2C lcd library: 
https://github.com/bitbank2/LCD1602
3. BCM2835 library:
https://www.airspayce.com/mikem/bcm2835/
