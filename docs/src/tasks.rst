Tasks
============

After getting familar with the package by going through tutorial, you can try simple development tasks with this package.

Task 1: if-smoke-then-alarm (if smoke,  then alarm)
--------------
Try to develop a ``if-smoke-then-alarm automation`` logic.
This application should subscribe content from a ``smoke`` topic, and publish command to ``alarm`` topic.
Data published by smoke detector can be binary: ``0 -- no smoke, 1 -- smoke``.
Unfortunately, you have to implement the smoke detector and alarm by yourself.
We have some `helper functions`_ that you may interest in.

.. _helper functions: https://github.com/shsssc/ndn-lite-mock-utils 

Task 2: its-too-hot (if temp > 80, then air conditioning on)
--------------
Try to develop an home automation that control your air conditioner.
This application should subscribe content from some temperature sensors, and publish command to ``air conditioner`` service.
Whenever current temperature is above 80 degree, this application should try to turn the air conditioner.
You may need to implement temperature sensor and air conditioner's logic in addition to the application logic described above.
But with our previously mentioned helpers, this should be easy.

Task 3: turn-off-when-i-leave (if no motion > 5 minute, turn off light)
-------------
Imagine users don't want waste electricty for living room lighting if no one is active working/moving.
This application should subscribe content from motion sensors in living room, and publish command to lights in living room.
Motion sensor periodically publish binary states: ``active, inactive``, on ``motion`` topic. 
Lights subscribe to command that carry the desired illuminance value (as ``tutorial-app`` we presented).
Whenever the inactive states last for over 5 minute, this application should issue command to set living room lights' illuminance value to 0.
Our ``tutorial-app-sub`` is a good template to reuse for light logic, and you may need to implement motion sensor logic on your own.