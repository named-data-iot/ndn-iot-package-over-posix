Quickstart Examples
========

Prepare a NDN environment
--------------

#. `Get Started with ndn-cxx`_
#. `Get Started with NFD`_
    Notice: Please build from source

#. Start your NFD 

    .. code-block:: bash
        $ nfd-start


.. _`Get Started with ndn-cxx`: https://named-data.net/doc/ndn-cxx/current/INSTALL.html
.. _`Get Started with NFD`: https://named-data.net/doc/NFD/current/INSTALL.html

Set up the IoT system controller
--------------

Download and _ndn-iot-controller: http://github.com/named-data-iot/ndn-iot-controller

.. code-block:: bash
    $ cd /path/to/controller
    $ git clone http://github.com/named-data-iot/ndn-iot-controller

Install dependencies (if you're using macOS or brew) :

.. code-block:: bash
    $ brew install zbar leveldb
    
Set up development environment:

.. code-block:: bash
    $ python3 -m venv ./venv
    $ ./venv/bin/python -m pip install -r requirements.txt

Run the controller server:

.. code-block:: bash
    $ ./venv/bin/python app.py

Share QR Code and bootstrap Device
-------------

There're some pre-generated QR codes in folder ``/<project-root>/devices``. Hard-coded bindings between device program and QR code are:

    +----------------------+----------------------------+
    | Device Program       | Pre-generated QR Code      | 
    +======================+============================+
    | tutorial-app         | device-398.png             |
    +----------------------+----------------------------+
    | tutorial-app-sub     | device-24777.png           |
    +----------------------+----------------------------+

Open controller's UI in browser at ``127.0.0.1:6060``, then click ``Device Bootstrapping`` button, a blank for uploading QR code should show up.

We upload ``device-398.png`` to the blank, and click ``bootstrap`` button, which enable controller waiting for bootstrapping request in the following 5 seconds.

Now run the corresponding device program inside this 5-second bootstrapping window:

.. code-block:: bash
    $ cd /<project-root>/build
    $ ./examples/tutorial-app

In this process, controller may ask for ``sudo``, please give our access.

This ``device-398`` has two functions:
#. Subscribe to LED command and adjust illuminance value based on command content
#. Publish a string ``hello`` to a pre-defined topic every 400000ms

Play with Example Command
--------------

Click ``Service Invocation`` button, you shall see a form asking for interested service and command parameters.
Given now only one device has been bootstrapped, only one ``service`` can be selected. 
Input any integer between 0 and 100 and send command, in the terminal which runs tutoriala-app, device side result should show.


Fetch a Published Content
--------------

Following similar steps with bootstrapping ``device-398`` (please do not kill it), we can bootstrap ``device-24777`` to the controller in another terminal by running ``tutorial-app-sub``.
This device subscribes to the pre-defined topic where ``device-398`` publishes its string.
After a while, the ``hello`` string should appeer in the terminal.