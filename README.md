MAVspeed
========
Control your drone from the keyboard and visualize navigation and camera feedback. It works on Linux with drones supported by MAVwork. Yaw and altitude are controlled in position. Horizontal displacements are controlled in speed.

Watch it in action in the video at http://www.uavster.com/projects/MAVwork-support-for-the-LinkQuad-quadcopter

SAFETY WARNING AND DISCLAIMER
-----------------------------
You are using this software at your onw risk. The authors decline any responsibility for personal injuries and/or property damage.

Some drones supported by this framework ARE NOT TOYS. Even operation by expert users might cause SERIOUS INJURIES to people around. So, please consider flying in a properly screened or isolated flight area.

Prerequisites
-------------
- Atlante: https://github.com/uavster/atlante
- MAVwork: https://github.com/uavster/mavwork
- OpenCV
- ncourses

Compilation
-----------
Open bin/makefile and change ATLANTE\_ROOTPATH and DRONECLIENT\_ROOTPATH to point to your Atlante and MAVwork API directories, respectively. Then, run 'make'.

Configuration
-------------
The PID gains are configured for a LinkQuad. Although, they might work for other MAVs as well, it is recommended that you check them before flying. They are defined at the beginning of MyDrone.cpp with explanatory comments.

The UI configuration parameters, like speeds or yaw and altitude increments, are defined at the beginning of ConsoleUI.cpp with proper explanatory comments.

UI keys
-------
ESC - Quit application

t - Take off  
l - Land  
h - Hover (managed by MAVwork)  
m - Start moving (moving commands only accepted when MOVE mode set)  
Space bar - Emergency stop (ONLY AVAILABLE AT SOME DRONES!! Use only if strictly necessary, as the drone will literally fall down like a stone)  
  
These keys only work after pressing 'm' once:  
Up arrow or 8 - Go forward at constant speed  
Down arrow or 2 - Go backwards at constant speed  
Left arrow or 4 - Go left at constant speed  
Right arrow or 6 - Go right at constant speed  
5 - Stay where you are  
z or 7 - Yaw left one increment  
x or 9 - Yaw right one increment  
q - Go one increment up  
a - Go one increment down  

Typical key sequence
--------------------
**IMPORTANT SAFETY NOTE**: Before the flight, make sure that nobody is in the flight area or surroundings or that they are behind a wall or fence. Do not rely on the space bar, as it is disabled by default for some (expensive) drones. If something goes wrong press 'h' to enter hover mode and wait for the drone to become steady in air.

1. Take off with 't' and wait for the drone to enter hovering mode (shown in UI)
2. Start accepting movement commands by pressing 'm'
3. Move as you like with arrow keys, q/a and z/x
4. When finished, enter hover mode with 'h'
5. Wait for the drone to stabilize while hovering
6. Land with 'l'

Contact
-------
For suggestions, please go to http://uavster.com/contact

Contributors
------------
[Ignacio M. Bataller](https://github.com/uavster): Original project creator and maintainer  
[Jesús Pestana](https://github.com/jespestana): Backup pilot during test flights  
