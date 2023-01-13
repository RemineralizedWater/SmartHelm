# Smart Helm

![Smart Helm](/320391047_729731128126008_1694693137878049535_n.jpg)

Coordinator:
Ground control for the emergency team. Can access the database and change
values according to team communication.

HelmetUsers:
Emergency team members wearing the fan/buzzer.

Flow of Data:
From Firebase on browser, can change the value of fan spin/IR distance.
The LoRa sender (Coordinator in Github files) will check this database
continuously for changes. If it recognizes a change, it will send a LoRa
packet to the correct receiver to apply the changes. Sender will keep
sending the same packet until the receiver sends back a 'thank you'
packet.

Temp sensor libraries in Arduino:
- DHT sensor library by Adafruit
- Adafruit unified sensor by Adafruit
