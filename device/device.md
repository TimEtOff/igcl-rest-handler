# Devices

## List devices

Returns a list of all available devices

> [!WARNING]
> Not yet implemented

### Path

`GET /device/`

### IGCL equivalent

```cpp
ctl_result_t ctlEnumerateDevices(ctl_api_handle_t hAPIHandle, uint32_t *pCount, ctl_device_adapter_handle_t *phDevices)
```

### Fields

| Name | Description | Type |
| ---- | ----------- | ---- |
| count | Number of available devices | int |
| **devices** | **List of all available devices** | **list** |
| &nbsp;&nbsp;&nbsp;&nbsp;<properties\> | See [get device infos](#get-device-infos) | |

## Get device infos

Returns a list of all available devices

> [!WARNING]
> Not yet implemented

### Path

`GET /device/`

### IGCL equivalent

```cpp
ctl_result_t ctlGetDeviceProperties(ctl_device_adapter_handle_t hDAhandle, ctl_device_adapter_properties_t *pProperties)
```

### Fields

| Name | Description | Type |
| ---- | ----------- | ---- |
| device_type | Device Type () | [ctl_device_type_t](../enums.md#ctl_device_type_t) (string) |
| supported_functions_flags | Supported functions | [ctl_supported_functions_flags_t](../enums.md#ctl_supported_functions_flags_t) (int) |
| driver_version | Driver version | int64 |
| driver_version_str | Readable driver version (w.x.y.z) | string |
| pci_vendor_id | PCI Vendor ID | int |
| pci_device_id | PCI Device ID | int |
| rev_id | PCI Revision ID | int |
| num_eus_per_sub_slice | Number of EUs per sub-slice | int |
| num_sub_slices_per_slice | Number of sub-slices per slice | int |
| num_slices | Number of slices | int |
| name | Device name | string |
| graphics_adapter_properties | Graphics Adapter Properties | [ctl_adapter_properties_flags_t](../enums.md#ctl_adapter_properties_flags_t) (int) |
| frequency | This represents the average frequency an end user may see in the typical gaming workload. Also referred as Graphics Clock. Supported only for Version > 0 | int |
| pci_subsys_id | PCI SubSys ID, Supported only for Version > 1 | int |
| pci_subsys_vendor_id | PCI SubSys Vendor ID, Supported only for Version > 1 | int |
| **adapter_bdf** | **Pci Bus, Device, Function. Supported only for Version > 1** | **object** |
| &nbsp;&nbsp;&nbsp;&nbsp;bus | PCI Bus Number | int |
| &nbsp;&nbsp;&nbsp;&nbsp;device | PCI device number | int |
| &nbsp;&nbsp;&nbsp;&nbsp;function | PCI function | int |
| num_xe_cores | Number of Xe Cores. Supported only for Version > 2 | int |
