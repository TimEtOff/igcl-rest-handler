# Devices

* _Devices_
  * [Temperature sensors 🢱](./temp/temperature.md)
  * [Frequency domains 🢱](./freq/frequency.md)
  * [LEDs 🢱](./led/led.md)
  * [Power domains 🢱](./power/power.md)
  * [Fans 🢱](./fan/fan.md)
  * [Memory modules 🢱](./memory/memory.md)
  * [Engine groups 🢱](./engine/engine.md)
  * [Display output 🢱](./display/display.md)

## List devices

Returns a list of all available devices

**Path**

`GET /device`

**IGCL equivalent**

```cpp
ctl_result_t ctlEnumerateDevices(ctl_api_handle_t hAPIHandle, uint32_t *pCount, ctl_device_adapter_handle_t *phDevices)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of available devices | int |
| **devices** | List of all available devices | array[object] |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get device properties](#get-device-properties) | |

## Get device properties

Returns a specific device properties. Indexes start at 0.

**Path**

`GET /device/{index}`

**IGCL equivalent**

```cpp
ctl_result_t ctlGetDeviceProperties(ctl_device_adapter_handle_t hDAhandle, ctl_device_adapter_properties_t *pProperties);
ctl_result_t ctlDevPropGetProperties(ctl_device_adapter_handle_t hDAhandle, ctl_dev_prop_properties_t *pProperties)
```

**Fields**

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_type | Device Type (Graphics, System) | string ([ctl_device_type_t](../enums.md#ctl_device_type_t)) |
| supported_subfunction_flags | Supported functions as a byte | int ([ctl_supported_functions_flag_t](../enums.md#ctl_supported_functions_flag_t)) |
| driver_version | Driver version | int |
| driver_version_str | Readable driver version (w.x.y.z) | string |
| pci_vendor_id | PCI Vendor ID | int |
| pci_device_id | PCI Device ID | int |
| rev_id | PCI Revision ID | int |
| num_eus_per_sub_slice | Number of EUs per sub-slice | int |
| num_sub_slices_per_slice | Number of sub-slices per slice | int |
| num_slices | Number of slices | int |
| name | Device name | string |
| graphics_adapter_properties | Graphics Adapter Properties | int ([ctl_adapter_properties_flag_t](../enums.md#ctl_adapter_properties_flag_t)) |
| frequency | This represents the average frequency an end user may see in the typical gaming workload. Also referred as Graphics Clock. Supported only for Version > 0 | int |
| pci_subsys_id | PCI SubSys ID, Supported only for Version > 1 | int |
| pci_subsys_vendor_id | PCI SubSys Vendor ID, Supported only for Version > 1 | int |
| **adapter_bdf** | Pci Bus, Device, Function. Supported only for Version > 1 | object |
| &nbsp;&nbsp;&nbsp;&nbsp;bus | PCI Bus Number | int |
| &nbsp;&nbsp;&nbsp;&nbsp;device | PCI device number | int |
| &nbsp;&nbsp;&nbsp;&nbsp;function | PCI function | int |
| num_xe_cores | Number of Xe Cores. Supported only for Version > 2 | int |
| is_workstation | [dev_prop] Indicates if the graphics device is a workstation SKU (Arc PRO) | bool |

## ctlGetSupportedRetroScalingCapability

## ctlGetSetCombinedDisplay

## ctlGetSetDisplayGenlock

## ctlLinkDisplayAdapters

## ctlUnlinkDisplayAdapters

## ctlGetLinkedDisplayAdapters

## ctlEccGetProperties

## ctlEccGetState ctlEccSetState

## ctlGetFirmwareProperties

## ctlGetSupportedVideoProcessingCapabilities

## ctlOverclockGetProperties

## ctlOverclockGpuFrequencyOffsetGet

## ctlOverclockWaiverSet ctlOverclockGpuFrequencyOffsetSet

## ctlOverclockGpuVoltageOffsetGet

## ctlOverclockWaiverSet ctlOverclockGpuVoltageOffsetSet

## ctlOverclockGpuLockGet

## ctlOverclockWaiverSet ctlOverclockGpuLockSet

## ctlOverclockVramFrequencyOffsetGet

## ctlOverclockWaiverSet ctlOverclockVramFrequencyOffsetSet

## ctlOverclockVramVoltageOffsetGet

## ctlOverclockWaiverSet ctlOverclockVramVoltageOffsetSet

## ctlOverclockPowerLimitGet

## ctlOverclockWaiverSet ctlOverclockPowerLimitSet

## ctlOverclockTemperatureLimitGet

## ctlOverclockWaiverSet ctlOverclockTemperatureLimitSet

## ctlPowerTelemetryGet

## ctlOverclockResetToDefault

## ctlOverclockGpuFrequencyOffsetGetV2

## ctlOverclockWaiverSet ctlOverclockGpuFrequencyOffsetSetV2

## ctlOverclockGpuMaxVoltageOffsetGetV2

## ctlOverclockWaiverSet ctlOverclockGpuMaxVoltageOffsetSetV2

## ctlOverclockVramMemSpeedLimitGetV2

## ctlOverclockWaiverSet ctlOverclockVramMemSpeedLimitSetV2

## ctlOverclockPowerLimitGetV2

## ctlOverclockWaiverSet ctlOverclockPowerLimitSetV2

## ctlOverclockTemperatureLimitGetV2

## ctlOverclockWaiverSet ctlOverclockTemperatureLimitSetV2

## ctlOverclockReadVFCurve

## ctlOverclockWaiverSet ctlOverclockWriteCustomVFCurve

## ctlPciGetProperties

## ctlPciGetState
