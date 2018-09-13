# PSVita bare-metal payload sample

## What's this?

This is a bare-metal payload sample that performs some hardware initialization, such as turning on the OLED display or HDMI controller, setting up a framebuffer, and reading the control buttons, among other things.

All the code in this project has been implemented by reverse engineering the PSVita OS and mimicking it to perform the required steps to carry out hardware initialization.

Simultaneously, I have also been updating and documenting [wiki.henkaku.xyz](https://wiki.henkaku.xyz/) with the findings obtained by reverse engineering the PSVita OS (such as register bits and initialization sequences).

## Instructions

**Compilation**:

* [vitasdk](https://vitasdk.org/) is needed.

**Installation**:

1. Copy `baremetal-sample.bin` to your PSVita
2. Run [vita-baremetal-loader](https://github.com/xerpi/vita-baremetal-loader)

## Credits

Thanks to everybody who contributes to [wiki.henkaku.xyz](https://wiki.henkaku.xyz/) and helps reverse engineering the PSVita OS.
