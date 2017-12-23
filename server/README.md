# Compiling

For convenient, use  [Dockcross](https://github.com/dockcross/dockcross)'s linux-armv7 image to cross compile for Raspberry Pi.

## Basic Setup
```
cd ~/
docker run --rm dockcross/linux-armv7 > ./cc-linux-armv7
chmod +x ./cc-linux-armv7
```
This will generate a script cc-linux-armv7 under the ~ directory.

## Launching the image
```
cd /path/to/v4l2-server/
sudo ~/cc-linux-armv7 bash
```
And this will map the current directory as root: /work inside the docker image.

## Scripts

* **cmake.sh** Invoke cmake to generate build scripts
* **compile.sh** Invoke the generated Makefile to compile the program and upload to Raspberry Pi if succeeded.
* **rpi_config.sh** Contains the Raspberry Pi's username and IP address

# Submodules
* asio - ASIO config to compile without Boost
* dukglue - Binding library for Duktap JS
* loguru - Logging library
* pigpio - GPIO library for Raspberry Pi


# Messages
All TCP/IP message use the following generic format
```
sizeof(data)    [4 bytes]
data            [sizeof(data) bytes]
```
As as result a message will be in total: size + 4 bytes.

## Keep Alive message
A 4-bytes long Keep Alive message will be sent every 5 seconds:
```
0x00 [1 byte]
0x00 [1 byte]
0x00 [1 byte]
0x00 [1 byte]
```

## JPEG message
A JPEG message will be sent once a video frame is captured:
```
sizeof(jpeg data + 4) [4 bytes]
'J' [1 byte]
'P' [1 byte]
'E' [1 byte]
'G' [1 byte]
jpeg data
```


## GPIO message
A GPIO message will be sent once gpio_get_0_31() or gpio_get_32_53 is invoked:
```
0x10 [4 bytes]
'G' [1 byte]
'P' [1 byte]
'I' [1 byte]
'O' [1 byte]
bank [4 bytes]
values [4 bytes]
```