# USB Programmable Power Supply

![Photo](Docs/UCDC-Photo.jpg)

This USB-C to DC adapter utilizes the USB Power Delivery protocol that turns your USB-PD charger into a programmable power supply. Voltage regulation is done in the USB-PD capable charger, while this adapter only controls the on/off and negotiates the desired voltage with the charger.

If the charger has PD 3.0 PPS feature, the output voltage can be adjusted in the unit of 20mV within the APDO range. Otherwise, only fixed voltages announced by the charger (PDOs) can be selected.

## Hardware

The STM32G071 MCU controls all the communications and user interactions. Complete hardware design files are in the `Schematics` folder.

## Build the Firmware

Set an environment variable that points to [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads), for example:

```
export GCC_PATH=/opt/gcc-arm-none-eabi-8-2019-q3-update/bin/
```

Then build the firmware with make:

```
make
```

After a successful compilation, you may download `build/UCDC.bin` to the STM32 with ST-Link or any other Link you have.


## Acknowledgment

This project is based on the demo code of [STM32G071B-DISCO](https://www.st.com/en/evaluation-tools/stm32g071b-disco.html).

