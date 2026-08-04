# Memory modules

* [Devices 🢱](../device.md)
  * [Temperature sensors 🢱](../temp/temperature.md)
  * [Frequency domains 🢱](../freq/frequency.md)
  * [LEDs 🢱](../led/led.md)
  * [Power domains 🢱](../power/power.md)
  * [Fans 🢱](../fan/fan.md)
  * _Memory modules_
  * [Engine groups 🢱](../engine/engine.md)
  * [Display output 🢱](../display/display.md)

## List memory modules

Returns a list of all memory modules

**Path**

`GET /device/{i}/memory`

**IGCL equivalent**

```cpp
ctl_result_t ctlEnumMemoryModules(ctl_device_adapter_handle_t hDAhandle, uint32_t *pCount, ctl_mem_handle_t *phMemory)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of memory modules | int |
| **modules** | List of all memory modules | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get module properties](#get-memory-module-properties) | |

## Get memory module properties

Returns a specific memory module properties.

**Path**

`GET /device/{i}/memory/{index}`

**IGCL equivalent**

```cpp
ctl_result_t ctlMemoryGetProperties(ctl_mem_handle_t hMemory, ctl_mem_properties_t *pProperties)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| module_index | Requested memory module index. Should be the same as in request target | int |
| type | The memory type | string ([ctl_mem_type_t](../../enums.md#ctl_mem_type_t)) |
| location | Location of this memory (system, device) | string ([ctl_mem_loc_t](../../enums.md#ctl_mem_loc_t)) |
| physical_size | Physical memory size in bytes. A value of 0 indicates that this property is not known. However, a call to ::ctlMemoryGetState() will correctly return the total size of usable memory | int |
| bus_width | Width of the memory bus. A value of -1 means that this property is unknown | int |
| num_channels | The number of memory channels. A value of -1 means that this property is unknown | int |

## Get memory module state

Get memory state - health, allocated.

**Path**

`GET /device/{i}/memory/{index}/state`

**IGCL equivalent**

```cpp
ctl_result_t ctlMemoryGetState(ctl_mem_handle_t hMemory, ctl_mem_state_t *pState)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| module_index | Requested memory module index. Should be the same as in request target | int |
| free | The free memory in bytes | int |
| size | The total allocatable memory in bytes (can be less than [properties](#get-memory-module-properties).physical_size) | int |

## Get memory module bandwidth

Get memory bandwidth

**Path**

`GET /device/{i}/memory/{index}/bandwidth`

**IGCL equivalent**

```cpp
ctl_result_t ctlMemoryGetBandwidth(ctl_mem_handle_t hMemory, ctl_mem_bandwidth_t *pBandwidth)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_index | Requested device index. Should be the same as in request target | int |
| module_index | Requested memory module index. Should be the same as in request target | int |
| max_bandwidth | Current maximum bandwidth in units of bytes/sec | int |
| timestamp | The timestamp (in microseconds) when these measurements were sampled. This timestamp should only be used to calculate delta time between snapshots of this structure. Never take the delta of this timestamp with the timestamp from a different structure since they are not guaranteed to have the same base. The absolute value of the timestamp is only valid during within the application and may be different on the next execution | int |
| read_counter | Total bytes read from memory. Supported only for Version > 0 | int |
| write_counter | Total bytes written to memory. Supported only for Version > 0 | int |
