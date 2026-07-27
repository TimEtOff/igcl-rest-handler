# Frequency domains

* [Devices 🢱](../device.md)
  * [Temperature sensors 🢱](../temp/temperature.md)
  * _Frequency domains_
  * [LEDs 🢱](../led/led.md)
  * [Power domains 🢱](../power/power.md)
  * [Fans 🢱](../fan/fan.md)
  * [Memory modules 🢱](../memory/memory.md)
  * [Engine groups 🢱](../engine/engine.md)
  * [Display output 🢱](../display/display.md)

## List frequency domains

Returns a list of all frequency domains

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/freq`

**IGCL equivalent**

```cpp
ctl_result_t ctlEnumFrequencyDomains(ctl_device_adapter_handle_t hDAhandle, uint32_t *pCount, ctl_freq_handle_t *phFrequency)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of frequency domains | int |
| **domains** | List of all frequency domains | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get domain properties](#get-frequency-domain-properties) | |

## Get frequency domain properties

Get frequency properties - available frequencies.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/freq/{index}`

**IGCL equivalent**

```cpp
ctl_result_t ctlFrequencyGetProperties(ctl_freq_handle_t hFrequency, ctl_freq_properties_t *pProperties)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| type | The hardware block that this frequency domain controls (GPU, memory, ...) | string ([ctl_freq_domain_t](../../enums.md#ctl_freq_domain_t)) |
| can_control | Indicates if software can control the frequency of this domain assuming the user has permissions | bool |
| min | The minimum hardware clock frequency in units of MHz | float |
| max | The maximum non-overclock hardware clock frequency in units of MHz | float |

## Get available clocks

Get available non-overclocked hardware clock frequencies for the frequency domain.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/freq/{index}/clocks`

**IGCL equivalent**

```cpp
ctl_result_t ctlFrequencyGetAvailableClocks(ctl_freq_handle_t hFrequency, uint32_t *pCount, double *phFrequency)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | The number of frequencies | int |
| frequencies | Array of frequencies in units of MHz and sorted from slowest to fastest | array[float] |

## Get frequency range

Get current frequency limits.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/freq/{index}/range`

**IGCL equivalent**

```cpp
ctl_result_t ctlFrequencyGetRange(ctl_freq_handle_t hFrequency, ctl_freq_range_t *pLimits)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| min | The min frequency in MHz below which hardware frequency management will not request frequencies. A negative value indicates that no external minimum frequency limit is in effect. | float |
| max | The max frequency in MHz above which hardware frequency management will not request frequencies. A negative number indicates that no external maximum frequency limit is in effect | float |

## Set frequency range

Get current frequency limits.

> [!WARNING]
> Not yet implemented

**Path**

`PUT /device/{i}/freq/{index}/range`

**IGCL equivalent**

```cpp
ctl_result_t ctlFrequencySetRange(ctl_freq_handle_t hFrequency, const ctl_freq_range_t *pLimits)
```

**Body parameters**

| Name | Description | Type |
| ---- | ----------- | ---- |
| min | The min frequency in MHz below which hardware frequency management will not request frequencies. Setting to 0 will permit the frequency to go down to the hardware minimum while setting to -1 will return the min frequency limit to the factory value (can be larger than the hardware min). | float |
| max | The max frequency in MHz above which hardware frequency management will not request frequencies. Setting to 0 or a very big number will permit the frequency to go all the way up to the hardware maximum while setting to -1 will return the max frequency to the factory value (which can be less than the hardware max) | float |

**Fields**

Returns the same output as [`GET /device/{i}/freq/{index}/range`](#get-frequency-range).

## Get frequency state

Get current frequency state - frequency request, actual frequency, TDP limits.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/freq/{index}/state`

**IGCL equivalent**

```cpp
ctl_result_t ctlFrequencyGetState(ctl_freq_handle_t hFrequency, ctl_freq_state_t *pState)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| current_voltage | Current voltage in Volts. A negative value indicates that this property is not known | float |
| request | The current frequency request in MHz. A negative value indicates that this property is not known | float |
| tdp | The maximum frequency in MHz supported under the current TDP conditions. This fluctuates dynamically based on the power and thermal limits of the part. A negative value indicates that this property is not known | float |
| efficient | The efficient minimum frequency in MHz. A negative value indicates that this property is not known | float |
| actual | The resolved frequency in MHz. A negative value indicates that this property is not known | float |
| throttle_reasons | The reasons that the frequency is being limited by the hardware. Returns 0 (frequency not throttled) or a combination of [ctl_freq_throttle_reason_flag_t](../../enums.md#ctl_freq_throttle_reason_flag_t) | int (byte) |

## Get throttle time

Get frequency throttle time.

> [!WARNING]
> Not yet implemented

**Path**

`GET /device/{i}/freq/{index}/throttle-time`

**IGCL equivalent**

```cpp
ctl_result_t ctlFrequencyGetState(ctl_freq_handle_t hFrequency, ctl_freq_state_t *pState)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| throttle_time | The monotonic counter of time in microseconds that the frequency has been limited by the hardware | int |
| timestamp | Microsecond timestamp when throttle_time was captured. This timestamp should only be used to calculate delta time between snapshots of this structure. Never take the delta of this timestamp with the timestamp from a different structure since they are not guaranteed to have the same base. The absolute value of the timestamp is only valid during within the application and may be different on the next execution | int |
