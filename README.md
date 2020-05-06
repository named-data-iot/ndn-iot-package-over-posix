[![Build Status](https://travis-ci.org/named-data-iot/ndn-iot-package-over-posix.svg?branch=master)](https://travis-ci.org/named-data-iot/ndn-iot-package-over-posix)

NDN-Lite
========

<img src="https://zhiyi-zhang.com/images/ndn-lite-logo.jpg" alt="logo" width="500"/>

The NDN-Lite library implements the Named Data Networking Stack with the high-level application support functionalities and low-level OS/hardware adaptations for Internet of Things (IoT) scenarios.

The library is written in standard C.

Please go to our [wiki page](https://github.com/Zhiyi-Zhang/ndn_standalone/wiki) for the project details.

NDN-IoT Package over POSIX
--------------------------
NDN-IoT Package over POSIX is a integrated IoT package based on NDN-Lite for POSIX compatible OS, such as
Raspberry Pi, Mac OS and Ubuntu.

### Prerequisites ###
CMake >= 3.5

### Build ###
This package uses CMake. A way to build is:
 ```
 git clone --recursive https://github.com/named-data-iot/ndn-iot-package-over-posix.git
 cd ndn-iot-package-over-posix
 mkdir build
 cd build
 cmake -DCMAKE_BUILD_TYPE=Release ..
 make -j2
 ```

### Unit Test ###
Run `./unittest` in the `build` folder.

## A Demonstration of NDN-Lite's Security Bootstrapping and Service Discovery

This requires two devices (e.g., two laptops, or one laptop one RaspberryPi) connected through WiFi ad-hoc or WiFi AP mode whose UDP multicast has been enabled.

### 1 Generate a QR code for the controller to scan
**Note**: In `/device` folder there're few pre-generated shared secret and you can directly leverage them and skip this procedure
We first generate the shared secret and a new pair of ECC keys on the device side:
```
cd build
./examples/tutorial-gen-new-shared-info
```
Then we use the `QR_encoder.py` to encode the public information into a QR code image.
```
cd ..
python3 QR_encoder.py
```
Notice: to run `QR_encoder.py`, one needs to install the dependency:
```
pip3 install pyqrcode
pip3 install pypng
```

### 2 Set up the IoT system controller
Go to https://github.com/named-data-iot/ndn-iot-controller to download and run our python based controller.
The controller require NFD running on the same device as the controller.

### 3 Play with the examples: Bootstrapping
Controller:
a. click `bootstrapping` on the left sidebar and upload the QR code image.
b. click `bootstrap` button which will trigger a blocking packet listen.
Device:
```
cd build
./examples/tutorial-app
```
**Note**: In this example, the ``tutorial-app`` and controller should on the same host. If you want them played on different hosts, pleast modify the face setting in source code with UDP face using API [here](https://github.com/named-data-iot/ndn-iot-package-over-posix/blob/master/adaptation/udp/udp-face.h), and on the controller side, set up the face to the ``tutorial-app`` using [nfdc](http://named-data.net/doc/NFD/current/manpages/nfdc-face.html) command.

### 4 Play with the examples: Service Discovery
Device:
After the bootstrapping process, the device program will automatically broadcast its services to the system, where the controller will hear the information.
Controller:
By clicking the `service list` on the left sidebar, one can know all the heard services in the system.
To invoke a service, simply copy the service name, e.g., `/ndn-iot/%01/%0B/device-123` (the device ID must be the same as your bootstrapped device), into the `NDN ping` page, modify the name to be `/ndn-iot/%0B/device-123/%00`, set the signed interest to be true, input a integer 0 to 5 into the parameter box as the desired  and express the interest out.

Notice: `%01` means service in NDN-Lite service discovery protocol.
`%0B` is the service type, which is LED service.
`%00` at the end of the name is the command index 0 under the LED service, which is to control the LED light.

Notice: If the ping Interest is not signed, the service cannot be invoked successfully.

## Generate XCode Project and Documentation
If you prefer to use XCode for your application development, you can generate a XCode project by the following commands.
### Generate XCode Project ###
 ```
 mkdir xcode
 cd xcode
 cmake -G "Xcode" ..
 open ndn-lite.xcodeproj
 ```

One can also generate a Doxygen-empowered library documentation by the following commands.
### Generate Doxygen Documentation ###
 ```
 mkdir build
 cd build
 cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCS=ON ..
 make docs
 open docs/html/index.html
 ```
