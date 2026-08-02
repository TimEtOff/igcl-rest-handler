# Power domains

* [Devices 🢱](../device.md)
  * [Temperature sensors 🢱](../temp/temperature.md)
  * [Frequency domains 🢱](../freq/frequency.md)
  * [LEDs 🢱](../led/led.md)
  * _Power domains_
  * [Fans 🢱](../fan/fan.md)
  * [Memory modules 🢱](../memory/memory.md)
  * [Engine groups 🢱](../engine/engine.md)
  * [Display output 🢱](../display/display.md)

## List power domains

Returns a list of all power domains

**Path**

`GET /device/{i}/power`

**IGCL equivalent**

```cpp
ctl_result_t ctlEnumPowerDomains(ctl_device_adapter_handle_t hDAhandle, uint32_t *pCount, ctl_pwr_handle_t *phPower)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of power domains | int |
| **domains** | List of all power domains | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get power properties](#get-power-properties) | |

## Get power properties

Returns a specific power domain properties.

**Path**

`GET /device/{i}/power/{index}`

**IGCL equivalent**

```cpp
ctl_result_t ctlPowerGetProperties(ctl_pwr_handle_t hPower, ctl_power_properties_t *pProperties)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| power_index | Requested power domain index. Should be the same as in request target | int |
| can_control | Indicates if software can change the power limits of this domain assuming the user has permissions | bool |
| default_limit | The factory default TDP power limit of the part in milliwatts. A value of -1 means that this is not known | int |
| min_limit | The minimum power limit in milliwatts that can be requested | int |
| max_limit | The maximum power limit in milliwatts that can be requested | int |

## Get power energy counter

Returns a specific power domain energy counter.

**Path**

`GET /device/{i}/power/{index}/counter`

**IGCL equivalent**

```cpp
ctl_result_t ctlPowerGetEnergyCounter(ctl_pwr_handle_t hPower, ctl_power_energy_counter_t *pEnergy)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| power_index | Requested power domain index. Should be the same as in request target | int |
| energy | The monotonic energy counter in microjoules | int |
| timestamp | Microsecond timestamp when energy was captured. This timestamp should only be used to calculate delta time between snapshots of this structure. Never take the delta of this timestamp with the timestamp from a different structure since they are not guaranteed to have the same base. The absolute value of the timestamp is only valid during within the application and may be different on the next execution | int |

## Get power limits

Returns a power domain limits.

**Path**

`GET /device/{i}/power/{index}/limits`

**IGCL equivalent**

```cpp
ctl_result_t ctlPowerGetLimits(ctl_pwr_handle_t hPower, ctl_power_limits_t *pPowerLimits)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| power_index | Requested power domain index. Should be the same as in request target | int |
| **sustained_power_limit** | Sustained power limit | object |
| &nbsp;&nbsp;&nbsp;&nbsp;enabled | Indicates if the limit is enabled (true) or ignored (false) | bool |
| &nbsp;&nbsp;&nbsp;&nbsp;power | Power limit in milliwatts | int |
| &nbsp;&nbsp;&nbsp;&nbsp;interval | Power averaging window (Tau) in milliseconds | int |
| **burst_power_limit** | Burst power limit | object |
| &nbsp;&nbsp;&nbsp;&nbsp;enabled | Indicates if the limit is enabled (true) or ignored (false) | bool |
| &nbsp;&nbsp;&nbsp;&nbsp;power | Power limit in milliwatts | int |
| **peak_power_limit** | Peak power limit | object |
| &nbsp;&nbsp;&nbsp;&nbsp;power_ac | Power limit in milliwatts for the AC power source | int |
| &nbsp;&nbsp;&nbsp;&nbsp;power_dc | power limit in milliwatts for the DC power source. On output, this will be -1 if the product does not have a battery | int |

## Set power limits

Changes power limits. Updates only specified fields.

**Path**

`PUT /device/{i}/power/{index}/limits`

**IGCL equivalent**

```cpp
ctl_result_t ctlPowerSetLimits(ctl_pwr_handle_t hPower, const ctl_power_limits_t *pPowerLimits)
```

**Body parameters**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| power_index | Requested power domain index. Should be the same as in request target | int |
| **sustained_power_limit** | Sustained power limit | object |
| &nbsp;&nbsp;&nbsp;&nbsp;enabled | Indicates if the limit is enabled (true) or ignored (false) | bool |
| &nbsp;&nbsp;&nbsp;&nbsp;power | Power limit in milliwatts | int |
| &nbsp;&nbsp;&nbsp;&nbsp;interval | Power averaging window (Tau) in milliseconds | int |
| **burst_power_limit** | Burst power limit | object |
| &nbsp;&nbsp;&nbsp;&nbsp;enabled | Indicates if the limit is enabled (true) or ignored (false) | bool |
| &nbsp;&nbsp;&nbsp;&nbsp;power | Power limit in milliwatts | int |
| **peak_power_limit** | Peak power limit | object |
| &nbsp;&nbsp;&nbsp;&nbsp;power_ac | Power limit in milliwatts for the AC power source | int |
| &nbsp;&nbsp;&nbsp;&nbsp;power_dc | power limit in milliwatts for the DC power source. On input, this is ignored if the product does not have a battery | int |

**Fields**

Returns the same output as [`GET /device/{i}/power/{index}/limits`](#get-power-limits).
