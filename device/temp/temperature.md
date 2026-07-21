# Temperature sensors

* [Devices 🢱](../device.md)
  * _Temperature sensors_
  * [Frequence domains 🢱](../freq/frequence.md)
  * [LEDs 🢱](../led/led.md)
  * [Power domains 🢱](../power/power.md)
  * [Fans 🢱](../fan/fan.md)
  * [Memory modules 🢱](../memory/memory.md)
  * [Engine groups 🢱](../engine/engine.md)
  * [Display output 🢱](../display/display.md)

## List sensors

Returns a list of all available temperature sensors

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/temp`

**IGCL equivalent**

```cpp
ctl_result_t ctlEnumTemperatureSensors(ctl_device_adapter_handle_t hDAhandle, uint32_t *pCount, ctl_temp_handle_t *phTemperature)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of available sensors | int |
| **sensors** | List of all available sensors | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get sensor properties](#get-sensor-properties) | |

## Get sensor properties

Returns a specific sensor properties. Indexes start at 0.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/temp/{index}`

**IGCL equivalent**

```cpp
ctl_result_t ctlTemperatureGetProperties(ctl_temp_handle_t hTemperature, ctl_temp_properties_t *pProperties)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| type | Which part of the device the temperature sensor measures | string ([ctl_temp_sensors_t](../../enums.md#ctl_temp_sensors_t)) |
| maxTemperature | Will contain the maximum temperature for the specific device in degrees Celsius | float |

## Get sensor temperature

Returns a specific sensor temperature in degrees Celcius. Indexes start at 0.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/temp/{index}/state`

**IGCL equivalent**

```cpp
ctl_result_t ctlTemperatureGetState(ctl_temp_handle_t hTemperature, double *pTemperature)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| temperature | Sensor temperature in degrees Celcius | float |
