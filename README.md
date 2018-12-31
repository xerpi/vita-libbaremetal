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

**Memory card authentication**:

For the Memory card authentication to work, you will have to place the key used by [`rmauth_sm`'s command 0x2](https://wiki.henkaku.xyz/vita/F00D_Commands#rmauth_sm.self) in the `msif.c` file.

Check [this](https://wiki.henkaku.xyz/vita/Keys) for more information.

## Credits

Thanks to everybody who contributes to [wiki.henkaku.xyz](https://wiki.henkaku.xyz/) and helps reverse engineering the PSVita OS.

Specially the [Team Molecule](https://twitter.com/teammolecule) (formed by [Davee](https://twitter.com/DaveeFTW), Proxima, [xyz](https://twitter.com/pomfpomfpomf3), and [YifanLu](https://twitter.com/yifanlu)), [TheFloW](https://twitter.com/theflow0), [motoharu](https://github.com/motoharu-gosuto), and everybody at the [HENkaku](https://discord.gg/m7MwpKA) Discord channel.
