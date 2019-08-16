ndn-lite
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

### Generate XCode Project ###
 ```
 mkdir xcode
 cd xcode
 cmake -G "Xcode" ..
 open ndn-lite.xcodeproj
 ```

### Generate Doxygen Documentation ###
 ```
 mkdir build
 cd build
 cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCS=ON ..
 make docs
 open docs/html/index.html
 ```

## A Demonstration of NDN-Lite's Security Bootstrapping and Service Discovery

This requires two devices (e.g., two laptops, or one laptop one RaspberryPi) connected through WiFi ad-hoc or WiFi AP mode whose UDP multicast has been enabled.

### 1 Generate a QR code for the controller to scan
We first generate the shared secret and a new pair of ECC keys on the device side:
```
cd build
./bin/examples/tutorial-gen-new-shared-info
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

### 3 Play with the examples
Controller:
a. click `bootstrapping` on the left sidebar and upload the QR code image.
b. click `bootstrap` button which will trigger a blocking packet listen.
Device:
```
cd build
./bin/examples/tutorial-app
```

