Demo Application
============

Applications are written in the same way with Device Programs: request key materials, subscribe to something anb publish something.

Application also use shared secrets like Device Programs.
It's weird that application identity is still named as "device".
We'll support different naming conventions in future.

Here, we present a template application: ``if-hot-then-light``.

This template appliction implements the logic: if temperature in livingroom is above 80 degree, turn on the light in the livingroom and set the brightness to 30 precent.
This application subscribe to "TEMP" service with locator "/livingroom" and publish command on "LED" service with locator "/livingroom". 

Two Device Programs and one Application are invovled:

    +----------------------------+----------------------------+--------------------+
    | Device Program/Application | Pre-generated QR Code      | Role               |
    +============================+============================+====================+
    | tutorial-app.c             | device-398.png             | LED Light          |
    +----------------------------+----------------------------+--------------------+
    | tutorial-app-sub.c         | device-24777.png           | Temperature Sensor |
    +----------------------------+----------------------------+--------------------+
    | app-template.c             | device-63884.png           | if-hot-then-light  |
    +----------------------------+----------------------------+--------------------+

When you play this demo application, bootstrap identities in three terminals in the order: device-398, device-24777, device-63884, so Device Program and Application can request keys on actually existing service.

LED light should print out the command, indicating command received.

This Application can be used as template to write your program.
When adding new Appliactions or Device Programs, you should also add them to CMakeInputs.
CMake configurations for examples are in ``/<project-root>/CMakeInputs/examples.cmake``.  

