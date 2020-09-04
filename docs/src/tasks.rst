Tasks
============

After getting familar with the package by going through tutorial, you can try simple development tasks with this package.

Each of the task is a home automation senario where your application read result from one sensor and give commands to another device accrodingly.
To facilitize the testing of your application, we provide `ndn-lite-mock-utils`_ to generate mock devices needed.

.. _ndn-lite-mock-utils: https://github.com/shsssc/ndn-lite-mock-utils/blob/master/devices/

To generate a mocked smoke detector and a mocked alarm, you can use:

.. code-block:: bash

    $ make TASK=if-smoke-then-alarm

And then the binary of the devices together with their credentials are ready to use in the same directory.
The newly generated ``command_receiver`` can be used as the alarm, and ``sensor`` can be used as the smoke detector.

You can see the `code and documentation`_ of this library for more details.

.. _code and documentation: https://github.com/shsssc/ndn-lite-mock-utils/blob/master/devices/

**Note:** to prevent credential conflict, please **remove all devices from the iot-controller (especially tutorial-app and tutorial-app-sub)** as instructed in `Quickstart Examples`_ before you start these tasks.

Also, please use ``device-63884`` credentials in the ``/devices`` directory of the ndn-iot-package-over-posix for your automation applciation.

.. _Quickstart Examples: examples.html#share-qr-code-and-bootstrap-device

Task 1: if-smoke-then-alarm (if smoke,  then alarm)
--------------
Try to develop a ``if-smoke-then-alarm automation`` logic.
There should be a smoke detector that publishes content to ``smoke`` service, and a alarm that subscribes to ``alarm`` service. 
Data published by smoke detector can be binary: ``0 -- no smoke, 1 -- smoke``.
This application should subscribe content from a ``smoke`` topic, and publish command to ``alarm`` topic.
Payload carried in command can be binary, too: ``0 -- alarm off, 1 -- alarm on``.
Three identities are invovled in total.

Task 2: its-too-hot (if temp > 80, then air conditioning on)
--------------
Try to develop an home automation that control your air conditioner.
This application should subscribe content from some temperature sensors, and publish command to ``air conditioner`` service.
Whenever current temperature is above 80 degree, this application should try to turn the air conditioner.

Task 3: turn-off-when-i-leave (if no motion > 5 minute, turn off light)
-------------
Imagine users don't want waste electricty for living room lighting if no one is active working/moving.
This application should subscribe content from motion sensors in living room, and publish command to lights in living room.
Motion sensor periodically publish binary states: ``active, inactive``, on ``motion`` topic. 
Lights subscribe to command that carry the desired illuminance value (as ``tutorial-app`` we presented).
Whenever the inactive states last for over 5 minute, this application should issue command to set living room lights' illuminance value to 0.
Our ``tutorial-app-sub`` is a good template to reuse for light logic, and you may need to implement motion sensor logic on your own.