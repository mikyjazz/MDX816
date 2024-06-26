# MDX816 ![Github Build Status](https://github.com/mikyjazz/MDX816/actions/workflows/build.yml/badge.svg)

![MDX816](https://github.com/mikyjazz/MDX816/assets/38558161/0ff04726-2644-4256-ab79-6f2ff34ff0a4)

MDX816 is derived from MiniDexed that is a FM synthesizer closely modeled on the famous DX7 by a well-known Japanese manufacturer running on a bare metal Raspberry Pi (without a Linux kernel or operating system). On Raspberry Pi 2 and larger, it can run 8 tone generators, not unlike the TX816/TX802 (8 DX7 instances without the keyboard in one box). [Featured by HACKADAY](https://hackaday.com/2022/04/19/bare-metal-gives-this-pi-some-classic-synths/), [Adafruit](https://blog.adafruit.com/2022/04/25/free-yamaha-dx7-synth-emulator-on-a-raspberry-pi/), and [Synth Geekery](https://www.youtube.com/watch?v=TDSy5nnm0jA).

IMPORTANT NOTES:
- A separate repository has been created instead of a fork as many changes have been made (and many more will be made) that match my personal taste of what the application should be; I'm not interested in discussing the appropriateness and validity of my pull requests... :)
- I recommend you use the main repository (https://github.com/probonopd/MiniDexed) if you want to follow the main line of development; I cannot guarantee if and what changes in the original main project will be mirrored here.
- All references to external libraries have been changed to local references to forks on my repository to not depend on "external events".
- Any information or reference to the original MiniDexed project that you can find here can generally be considered valid if there is no different information available for MDX816.

## Demo songs

Listen to some examples made with MiniDexed by Banana71 [here](https://soundcloud.com/soundplantage/sets/minidexed2).

## Features

- [x] Uses [Synth_Dexed](https://codeberg.org/dcoredump/Synth_Dexed) with [circle-stdlib](https://github.com/smuehlst/circle-stdlib)
- [x] SD card contents can be downloaded from [GitHub Releases](../../releases)
- [x] Runs on all Raspberry Pi models (except Pico); see below for details
- [x] Produces sound on the headphone jack, HDMI display or [audio extractor](https://github.com/mikyjazz/MDX816/wiki/Hardware#hdmi-to-audio) (better), or a [dedicated DAC](https://github.com/mikyjazz/MDX816/wiki/Hardware#i2s-dac) (best)
- [x] Supports multiple voices through Program Change and Bank Change LSB/MSB MIDI messages
- [x] Loads voices from `.syx` files from SD card (e.g., using `getsysex.sh` or from [Dexed_cart_1.0.zip](http://hsjp.eu/downloads/Dexed/Dexed_cart_1.0.zip))
- [x] Menu structure on optional [HD44780 display](https://www.berrybase.de/sensoren-module/displays/alphanumerische-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb) and rotary encoder
- [x] Runs up to 8 Dexed instances simultaneously (like in a TX816) and mixes their output together
- [x] Allows for each Dexed instance to be detuned and stereo shifted
- [x] Allows to configure multiple Dexed instances through `performance.ini` files (e.g., [converted](https://github.com/BobanSpasic/MDX_Vault) from DX1, DX5, TX816, DX7II, TX802)
- [x] Compressor effect
- [x] Reverb effect
- [x] Voices can be edited over MIDI, e.g., using the [synthmata](https://synthmata.github.io/volca-fm/) online editor (requires [additional hardware](https://github.com/mikyjazz/MDX816/wiki/Hardware#usb-midi-devices))

## Introduction

Video about this project by [Floyd Steinberg](https://www.youtube.com/watch?v=Z3t94ceMHJo):

[![YouTube Video about MiniDexed (Floyd Steinberg)](https://i.ytimg.com/vi/Z3t94ceMHJo/sddefault.jpg)](https://www.youtube.com/watch?v=Z3t94ceMHJo)

## System Requirements

- Raspberry Pi 1, 2, 3, 4, or 400 (Zero and Zero 2 can be used but need HDMI or a supported i2s DAC for audio out). On Raspberry Pi 1 and on Raspberry Pi Zero there will be severely limited functionality (only one tone generator instead of 8)
- A [PCM5102A or PCM5122 based DAC](https://github.com/mikyjazz/MDX816/wiki/Hardware#i2s-dac), HDMI display or [audio extractor](https://github.com/mikyjazz/MDX816/wiki/Hardware#hdmi-to-audio) for good sound quality. If you don't have this, you can use the headphone jack on the Raspberry Pi but on anything but the Raspberry 4 the sound quality will be seriously limited
- Optionally (but highly recommended), an [LCDC1602 Display](https://www.berrybase.de/en/sensors-modules/displays/alphanumeric-displays/alphanumerisches-lcd-16x2-gr-252-n/gelb) (with or without i2c "backpack" board) and a [KY-040 rotary encoder](https://www.berrybase.de/en/components/passive-components/potentiometer/rotary-encoder/drehregler/rotary-encoder-mit-breakoutboard-ohne-gewinde-und-mutter)

## Usage

- In the case of Raspberry Pi 4, Update the firmware and bootloader to the latest version (not doing this may cause USB reliability issues)
- Download from [GitHub Releases](../../releases)
- Unzip
- Put the files into the root directory of a FAT32 formatted partition on SD/microSD card (Note for small SD cards which are no longer sold: If less than 65525 clusters, you may need to format as FAT16.)
- Put SD/microSD card into Raspberry Pi 1, 2, 3 or 4, or 400 (Zero and Zero 2 can be used but need HDMI or a supported i2c DAC for audio out)
- Attach headphones to the headphone jack using `SoundDevice=pwm` in `mdx816.ini` (default) (poor audio quality)
- Alternatively, attach a  PCM5102A or PCM5122 based DAC and select i2c sound output using `SoundDevice=i2s` in `mdx816.ini` (best audio quality)
- Alternatively, attach a HDMI display with sound and select HDMI sound output using `SoundDevice=hdmi` in `mdx816.ini` (this may introduce slight latency)
- Attach a MIDI keyboard via USB (alternatively you can build a circuit that allows you to attach a "traditional" MIDI keyboard using a DIN connector, or use a DIN-MIDI-to-USB adapter)
- If you are using a LCDC1602 with an i2c "backpack" board, then you need to set `LCDI2CAddress=0x27` (or another address your i2c "backpack" board is set to) in `mdx816.ini`
- Boot
- Start playing
- If the system seems to become unresponsive after a few seconds, remove `usbspeed=full` from `cmdline.txt` and repeat ([details](https://github.com/probonopd/MiniDexed/issues/39))
- Optionally, put voices in `.syx` files onto the SD card (e.g., using `getsysex.sh`)
- See the Wiki for [Menu](https://github.com/mikyjazz/MDX816/wiki/Menu) operation
- For voice programming, use any DX series editor (using MIDI sysex), including Dexed
- For library management, use the dedicated [MiniDexedLibrarian](https://github.com/BobanSpasic/MiniDexedLibrarian) software
- If something is unclear or does not work, don't hesitate to [ask](https://github.com/mikyjazz/MDX816/discussions/)!

## Pinout

All devices on Raspberry Pi GPIOs are **optional**.

![Raspberry Pi Pinout/GPIO Diagram](https://private-user-images.githubusercontent.com/38558161/318956800-ec790a10-b0de-4a9a-9255-1f27626160f8.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3MTM5ODQ5NTUsIm5iZiI6MTcxMzk4NDY1NSwicGF0aCI6Ii8zODU1ODE2MS8zMTg5NTY4MDAtZWM3OTBhMTAtYjBkZS00YTlhLTkyNTUtMWYyNzYyNjE2MGY4LnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNDA0MjQlMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjQwNDI0VDE4NTA1NVomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTQ4MDNhYTdlMDA5OTQ1MTMzN2MwNGQ5NGE2OTA4Nzc2YmJkNWI1M2NlZTkzNWYyMGY1ZGZkYWQzMDRhMjc3YmUmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0JmFjdG9yX2lkPTAma2V5X2lkPTAmcmVwb19pZD0wIn0.mg_WMzQbxTu4Wbk1CsotlJaEOwbk4N2SbPKAl6FOiTU)

Please see the [wiki](https://github.com/mikyjazz/MDX816/wiki) for more information.

## Downloading

Compiled versions are available on [GitHub Releases](../../releases). Just download and put on a FAT32 formatted SD card.

## Building

Please see the [wiki](https://github.com/mikyjazz/MDX816/wiki/Development#building-locally) on how to compile the code yourself.

## Contributing

This project lives from the contributions of skilled C++ developers, testers, writers, etc. Please see <https://github.com/mikyjazz/MDX816/issues>.

## Discussions

We are happy to hear from you. Please join the discussions on <https://github.com/mikyjazz/MDX816/discussions>.

## Documentation

Project documentation is at <https://github.com/mikyjazz/MDX816/wiki>.

## Acknowledgements

This project stands on the shoulders of giants. Special thanks to:

- [probonopd](https://github.com/probonopd) for the original release of [MiniDexed](https://github.com/probonopd/MiniDexed)
- [raphlinus](https://github.com/raphlinus) for the [MSFA](https://github.com/google/music-synthesizer-for-android) sound engine
- [asb2m10](https://github.com/asb2m10/dexed) for the [Dexed](https://github.com/asb2m10/dexed) software
- [dcoredump](https://github.com/dcoredump) for [Synth Dexed](https://codeberg.org/dcoredump/Synth_Dexed), a port of Dexed for embedded systems
- [rsta2](https://github.com/rsta2) for [Circle](https://github.com/rsta2/circle), the library to run code on bare metal Raspberry Pi (without a Linux kernel or operating system) and for the bulk of the MiniDexed code
- [smuehlst](https://github.com/smuehlst) for [circle-stdlib](https://github.com/smuehlst/circle-stdlib), a version with Standard C and C++ library support
- [Banana71](https://github.com/Banana71) for the sound design of the [Soundplantage](https://github.com/Banana71/Soundplantage) performances shipped with MiniDexed
- [BobanSpasic](https://github.com/BobanSpasic) for the [MiniDexedLibrarian](https://github.com/BobanSpasic/MiniDexedLibrarian) software, [MiniDexed performance converter](https://github.com/BobanSpasic/MDX_PerfConv) and [collection of performances for MiniDexed](https://github.com/BobanSpasic/MDX_Vault)
- [diyelectromusic](https://github.com/diyelectromusic/) for many [contributions](https://github.com/mikyjazz/MDX816/commits?author=diyelectromusic)
