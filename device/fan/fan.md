# Fans

* [Devices 🢱](../device.md)
  * [Temperature sensors 🢱](../temp/temperature.md)
  * [Frequence domains 🢱](../freq/frequence.md)
  * [LEDs 🢱](../led/led.md)
  * [Power domains 🢱](../power/power.md)
  * _Fans_
  * [Memory modules 🢱](../memory/memory.md)
  * [Engine groups 🢱](../engine/engine.md)
  * [Display output 🢱](../display/display.md)

## List fans

Returns a list of all available fans

> [!WARNING]
> Not yet implemented

### Path

`GET /device/{i}/fan`

### IGCL equivalent

```cpp
ctl_result_t ctlEnumFans(ctl_device_adapter_handle_t hDAhandle, uint32_t *pCount, ctl_fan_handle_t *phFan)
```

### Fields

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of available fans | int |
| **fans** | List of all available fans | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get fan properties](#get-fan-properties) | |

## Get fan properties

Returns a specific fan properties.

> [!WARNING]
> Not yet implemented

### Path

`GET /device/{i}/fan/{index}`

### IGCL equivalent

```cpp
ctl_result_t ctlFanGetProperties(ctl_fan_handle_t hFan, ctl_fan_properties_t *pProperties)
```

### Fields

| Name | Description | Type |
| ---- | ----------- | ---- |
| can_control | Indicates if software can control the fan speed assuming the user has permissions | bool |
| supported_modes | Bitfield of supported fan configuration modes (1<<[ctl_fan_speed_mode_t](../../enums.md#ctl_fan_speed_mode_t)) | int |
| supportedUnits | Bitfield of supported fan speed units (1<<[ctl_fan_speed_units_t](../../enums.md#ctl_fan_speed_units_t)) | int |
| max_rpm | The maximum RPM of the fan. A value of -1 means that this property is unknown | int |
| max_points | The maximum number of points in the fan temp/speed table. A value of -1 means that this fan doesn't support providing a temp/speed table | int |

## Get fan config

Returns a fan configurations and the current fan speed mode (default, fixed, temp-speed table).

> [!WARNING]
> Not yet implemented

### Path

`GET /device/{i}/fan/{index}/config`

### IGCL equivalent

```cpp
ctl_result_t ctlFanGetConfig(ctl_fan_handle_t hFan, ctl_fan_config_t *pConfig)
```

### Fields

| Name | Description | Type |
| ---- | ----------- | ---- |
| mode | The fan speed mode (fixed, temp-speed table) | string ([ctl_fan_speed_mode_t](../../enums.md#ctl_fan_speed_mode_t)) |
| **speed_fixed** | The current fixed fan setting | object |
| &nbsp;&nbsp;&nbsp;&nbsp;speed | The speed of the fan. On output, a value of -1 indicates that there is no fixed fan speed setting | int |
| &nbsp;&nbsp;&nbsp;&nbsp;units | The units that the fan speed is expressed in. On output, if fan speed is -1 then units should be ignored | string ([ctl_fan_speed_units_t](../../enums.md#ctl_fan_speed_units_t)) |
| **speed_table** | A table containing temperature/speed pairs | object |
| &nbsp;&nbsp;&nbsp;&nbsp;num_points | The number of valid points in the fan speed table. 0 means that there is no fan speed table configured. -1 means that a fan speed table is not supported by the hardware | int |
| &nbsp;&nbsp;&nbsp;&nbsp;table | Array of temperature/fan speed pairs. The table is ordered based on temperature from lowest to highest | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;temperature | Temperature in degrees Celsius | int |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**speed** | The speed of the fan | object |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;speed | The speed of the fan. On output, a value of -1 indicates that there is no fixed fan speed setting | int |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;units | The units that the fan speed is expressed in. On output, if fan speed is -1 then units should be ignored | string ([ctl_fan_speed_units_t](../../enums.md#ctl_fan_speed_units_t)) |

## Change fan mode

With no parameters or `mode` parameter set to `"CTL_FAN_SPEED_MODE_DEFAULT"`, configure the fan to run with hardware factory settings (set mode to [CTL_FAN_SPEED_MODE_DEFAULT](../../enums.md#ctl-fan-speed-mode-default)).

With `mode` parameter set to `"CTL_FAN_SPEED_MODE_FIXED"`, configure the fan to rotate at a fixed `speed` parameter (set mode to [CTL_FAN_SPEED_MODE_FIXED](../../enums.md#ctl-fan-speed-mode-fixed)).

With `mode` parameter set to `"CTL_FAN_SPEED_MODE_TABLE"`, configure the fan to adjust speed based on a temperature/speed `table` parameter (set mode to [CTL_FAN_SPEED_MODE_TABLE]((../../enums.md#ctl-fan-speed-mode-table)))

> [!WARNING]
> Not yet implemented

### Path

`PUT /device/{i}/fan/{index}/config`

### IGCL equivalent

```cpp
ctl_result_t ctlFanSetDefaultMode(ctl_fan_handle_t hFan)
ctl_result_t ctlFanSetFixedSpeedMode(ctl_fan_handle_t hFan, const ctl_fan_speed_t *speed)
ctl_result_t ctlFanSetSpeedTableMode(ctl_fan_handle_t hFan, const ctl_fan_speed_table_t *speedTable)
```

### Parameters

| Name | Description | Type |
| ---- | ----------- | ---- |
| mode | Which mode to set on the fan | string ([ctl_fan_speed_mode_t](../../enums.md#ctl_fan_speed_mode_t)) |
| **speed** | The current fixed fan setting | object |
| &nbsp;&nbsp;&nbsp;&nbsp;speed | The speed of the fan. On output, a value of -1 indicates that there is no fixed fan speed setting | int |
| &nbsp;&nbsp;&nbsp;&nbsp;units | The units that the fan speed is expressed in. On output, if fan speed is -1 then units should be ignored | string ([ctl_fan_speed_units_t](../../enums.md#ctl_fan_speed_units_t)) |
| **table** | A table containing temperature/speed pairs | object |
| &nbsp;&nbsp;&nbsp;&nbsp;num_points | The number of valid points in the fan speed table. 0 means that there is no fan speed table configured. -1 means that a fan speed table is not supported by the hardware | int |
| &nbsp;&nbsp;&nbsp;&nbsp;table | Array of temperature/fan speed pairs. The table is ordered based on temperature from lowest to highest | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;temperature | Temperature in degrees Celsius | int |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**speed** | The speed of the fan | object |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;speed | The speed of the fan. On output, a value of -1 indicates that there is no fixed fan speed setting | int |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;units | The units that the fan speed is expressed in. On output, if fan speed is -1 then units should be ignored | string ([ctl_fan_speed_units_t](../../enums.md#ctl_fan_speed_units_t)) |

### Fields

Returns the same output as [`GET /device/{i}/fan/{index}/config`](#get-fan-config).

## ctlFanGetState
