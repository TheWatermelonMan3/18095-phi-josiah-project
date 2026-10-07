Description of code here! It's good to describe what each file does and any dependencies.

### infrence.ino

This file is flashed onto the XIAO ESP32 which sits on top of the Grove AI module. This code is responsible for interfacing with the Grove AI module using sample code that was refactored from the Seeed Studio website. After opening a BLE server the ESP32 will invoke the AI module and periodically call to make an infrence. If there were no soda cans in sight, a 0 will be sent to who ever is connected to the BLE server. If a soda can is found, the X coordinated will be parsed and then published to the BLE server. 

### reciever.ino

This file is flashed onto the second ESP32 that has free pins. This code is a bit more complicated because it has to deal with disconnections and dynamically trying to reconnect to the BLE server hosted by the XIAO all while interpretting the XIAO results and sending commands to through wired connections to other Arduinos that control the different actuators. The ESP32 will start by scanning for a open BLE server on the specified port, then when it connects it looks for data sent by the XIAO. After it recieves data, it will add an offset because the X coord that is recieved is actually the corner of the soda can, not the middle. With the updated X coord, there is a "deicison tree" thats very simple which tells the Arduinos what directions the stepper motors should spin in based on whether Clank needs to turn left, right, or go straight. 

### forklift.ino

This file was on the first Arduino Uno which was not connected to any other boards except for a shared ground. This code controls the forklift using a finite state machine. First the forklift is in calibration state and moves down until a stripped wire on the forklift touches another stripped wire on the frame of the robot and completes a circuit. This indicates that the forklift is at a good starting position on the floor. Then the forklift moves into detection state where it uses a distance sensor to find the distance of the nearest object. If it's closer than 3 inches, the servo is moved to close the gate (making sure the can can't fall out) and the forklift is then raised up. Then the forklift is in holding state where it waits for a button press. When that happens, the forklift drops and the servo gate opens. Then the forklift is in waiting state as it waits until the distance reading is more than 3 and less than 10 inches (removing the can).

### *missing file: wheeldriver.ino*

This file was on the second Arduino Uno which was connected to the wheel servo motor drivers. I accidentally deleted this file and it cannot be recovered. It is a simple arduino script which listens for high voltages on three different pins: left wheel pin, right wheel pin, and reverse pin. Here is how the logic of it worked:
* If the reverse pin has a high voltage, reverse both wheels for two seconds.
* Otherwise:
    * If the left wheel and right wheel pins both have high voltage, step both wheels forward little by little to drive forward.
    * If the left wheel pin has high voltage and right wheel pin has low voltage, step the left wheel forward and the right wheel backward little by little to pivot toward the right side.
    * If the right wheel pin has high voltage and left wheel pin has low voltage, step the right wheel forward and the left wheel backward little by little to pivot toward the left side. 
The rotation is achieved by doing small steps to both the left and right stepper motor in turn. I think it was about 20 steps on each one alternating rapidly between them. The stepper speed was about 8.
