# Transfer Library

## Library for sending and receiving files and folders
The library provides an instrumentary for convenient receptions and sending files. it is possible to send objects carefully and carelessly for different tasks. Provides for logging<br>
The library is based on Boost 1.74 and C++ 23<br>
For more information, see the doxygen documentation

## Library building
Necessary Boost packages: asio, filesystem, log_setup, log<br>
It is assumed that Boost on Windows was installed in the directory "C:\Program Files\boost\boost_version_"<br>

### For example in build directory:
1) `cmake ..`
2) build with `cmake --build . --target install`

### Testing example:
1) go to `/transfer/test`
2) `cmake ..`
3) build with `cmake --build .`
4) in the directory where the test application was built run<br>
`sen` - for sending file<br>
`rec` - for receiving file<br>