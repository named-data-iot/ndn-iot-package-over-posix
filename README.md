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
CMake >= 3.11

### Build ###
This package uses CMake. A way to build is:
 ```
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
 