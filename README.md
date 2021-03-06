Overview
---

This board is design to fit on a raspberry and permit multiple features. It also have an embedded nrf52832 to allow real time process. The board can be in stand alone with only the nrf52.

**All the features has been tested**

Github : https://github.com/NathanB49/Utilipi

Openhardware.io : https://www.openhardware.io/view/618/UtiliPi

Features
---

#### IO EXTENDER
There is 32 channels I/O included 16 which can work with interrupt service thank to an I/O expender on I²C bus.

#### ADC
The Analog to Digital Converter have 4 channel 10V max and 4 channel with input selection. The input selection works either for 10V max or 4-20mA loop.

#### SPI, I²C and UART
For each protocol, the connectors are routed to their native port on the raspberry. The level input can be 16V max. For the SPI, 4 channel from the raspberry are used for CS for each connector.

#### Transistors NPN
There is 8 transistors NPN, the collector and emitter pin are available on the connector. For each transistors, a LED show it state (open/close).

#### GPIO
There is 4 channel directly connected to the raspberry. Unlike the others, they are not protected for over-voltage (max 3.3V).

#### POWER
The board can be powered by raspberry (remove the jumper JP1) or it can be powered by external supply (max 36V).
This module also works : https://fr.aliexpress.com/item/mini-DC-DC-Converter-Step-Down-buck-Power-Supply-Module-for-Flight-Control-Car/2038556858.html?spm=a2g0s.9042311.0.0.5bf36c37EOK96X

BLOG
---

- **22/08/2018** : Ordering the first board at a PCB manufacturer (JLCPCB)
- **28/08/2018** : The 5 board ordered are arrived, but there is 3 copper areas missing due to editor which don't make good gerber file. Issue post on the user forum. [https://easyeda.com/forum/topic/Copper-area-overlap-bug-deaf372a8630482aa5ac4a56d1aa59e7]
- Testing the board and the routing feature by feature
- **07/09/2018** : I²C adaptateur and IO extender has been soldered, the raspberry succed in discovering slave in the bus. Good point.
- **28/09/2018** : The hole size for connector is too thin, and routage will be update for allowing SPI without using IO extender.
- **30/10/2018** : The transistors, the IOExtender, the I2C and SPI voltage shift works well.
- **31/10/2018** : I've redesign the board according to the mistake i've found. Also, a library from ABElectronic use the IOextender and ease it use. [https://github.com/abelectronicsuk/ABElectronics_Python_Libraries/tree/master/IOPi]
- **25/02/2019** : Add the Github, multiple corrections in the PCB incoming with v2.2
- **28/02/2019** : New version 2.2, add the source code on github
- **29/04/2019** : New version 2.3, add the source code on github. Add the nrf52 to the board.

TO DO
---

- 3D model of a shell to protect the board

TEST
