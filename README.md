| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Scanner
Scanner code to learn about ESP, WIFI, and Connecting Data to AWS

This goal of this project is to create a scanner that will detect and notify users via a Kiosk that they are wearing electronic devices. This will help prevent accidental security violations for lab environments that restrict outside electronics. 

## ESP
The ESP will scan devices in the local area. The ESP will send the device info to AWS for consumption. 

## AWS APP
The AWS application will have the following use cases (1) allow registration of a device. (2) notify the user they have electronics on them (3) report detections to the user in report format. 

