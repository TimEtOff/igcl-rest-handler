# IGCL REST Handler

## Introduction

This application is meant to be an easier way for third-parties to access [Intel Graphics Control Library (IGCL)](https://github.com/intel/drivers.gpu.control-library) for Intel's graphics hardware control, translating from REST requests to native API methods.

## Notes

* The IGCL library is **not** included in this application, it is shipped and installed with Intel Graphics Driver.
* This project is not affiliated with Intel in any way, this is a personnal project that might help other users.
* It was created mainly for Performance & Telemetry control and monitoring, i.e., Engine/Fan/Telemetry/Frequency/Memory/Overclock/PCI/Power/Temperature. If you need any other features that are not already implemented (i.e. Display, 3D & Media), I probably do not plan on adding it.

## Usage

This app will run as a service in the background and wait to receive requests from any other programs.
It will also appear in your system tray for informations and settings.

_//TODO System tray explanations_

## Integration

Default port is `9738` (TCP).

The API works by layers, for exemple:

* to get the list of all available devices: `GET /device/`
* to get information about the second sensor of the first device: `GET /device/0/temp/1/`
* to set the speed of the first fan on the first device: `POST /device/0/fan/0/set/`

> [!NOTE]
> For every GET request, all fields are returned by default, but you can retrieve only specific ones if you define them as `true` in the request body.
> In this case, every other will default to `false`.

For the details of every methods, see related handle page:

* [Devices 🢱](./device/device.md)
  * [Temperature sensors 🢱](./device/temp/temperature.md)
  * ...
