
#include "device.hpp"
#include "temp/temp.hpp"

json::object get_device_properties(ctl_device_adapter_handle_t &hDevice, ctl_result_t& ctlResult)
{
    json::object res = {};

    // Get Adapter Properties
    ctl_device_adapter_properties_t pDevice = { 0 };

    pDevice.Size           = sizeof(ctl_device_adapter_properties_t);
    pDevice.Version        = 0;
    pDevice.pDeviceID      = malloc(sizeof(LUID));
    pDevice.device_id_size = sizeof(LUID);
    ctlResult = ctlGetDeviceProperties(hDevice, &pDevice);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(pDevice.pDeviceID);
        return res;
    }

    res["device_type"]                 = magic_enum::enum_name(pDevice.device_type);
    res["supported_subfunction_flags"] = pDevice.supported_subfunction_flags;
    res["driver_version"]              = pDevice.driver_version;
    res["driver_version_str"]          = getReadableVersion(pDevice.driver_version);
    res["pci_vendor_id"]               = pDevice.pci_vendor_id;
    res["pci_device_id"]               = pDevice.pci_device_id;
    res["rev_id"]                      = pDevice.rev_id;
    res["num_eus_per_sub_slice"]       = pDevice.num_eus_per_sub_slice;
    res["num_sub_slices_per_slice"]    = pDevice.num_sub_slices_per_slice;
    res["num_slices"]                  = pDevice.num_slices;
    res["name"]                        = pDevice.name;
    res["graphics_adapter_properties"] = pDevice.graphics_adapter_properties;
    res["frequency"]                   = pDevice.Frequency;
    res["pci_subsys_id"]               = pDevice.pci_subsys_id;
    res["pci_subsys_vendor_id"]        = pDevice.pci_subsys_vendor_id;
    res["adapter_bdf"]                 = {
        {"bus", pDevice.adapter_bdf.bus},
        {"device", pDevice.adapter_bdf.device},
        {"function", pDevice.adapter_bdf.function}
    };
    res["num_xe_cores"]                = pDevice.num_xe_cores;

    ctl_dev_prop_properties_t dpDevice = { 0 };
    dpDevice.Size    = sizeof(ctl_dev_prop_properties_t);
    dpDevice.Version = 0;
    ctl_result_t devResult = ctlDevPropGetProperties(hDevice, &dpDevice);
    if (devResult == CTL_RESULT_SUCCESS)
        res["is_workstation"] = dpDevice.isWorkstation;

    free(pDevice.pDeviceID);

    return res;
}

http::message_generator
handle_device(
    http::request<http::string_body>&& req,
    std::string& target,
    ctl_api_handle_t& hAPIHandle)
{
    ctl_result_t ctlResult;

    ctl_device_adapter_handle_t *hDevices = nullptr;
    uint32_t devicesCount = 0;
    ctlEnumerateDevices(hAPIHandle, &devicesCount, hDevices);
    hDevices = (ctl_device_adapter_handle_t *)malloc(sizeof(ctl_device_adapter_handle_t) * devicesCount);
    ctlResult = ctlEnumerateDevices(hAPIHandle, &devicesCount, hDevices);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hDevices);
        return server_error("ctlEnumerateDevices: " + std::string(magic_enum::enum_name(ctlResult)), req);
    }

    json_body::value_type body;

    if (!target.empty()) {
        size_t pos = target.find_first_of('/');
        size_t deviceInd = static_cast<size_t>(std::stoul(target.substr(0, pos)));
        target = target.substr(pos + 1);

        if (deviceInd >= devicesCount) {
            free(hDevices);
            return bad_request("Device index out of bounds (" + std::to_string(deviceInd) + " out of " + std::to_string(devicesCount) + ")", req);
        }

        ctl_device_adapter_handle_t hDevice = hDevices[deviceInd];
        free(hDevices);

        if (!target.empty()) {                                      // /device/{index}/...
            if (target.rfind("temp/", 0) == 0)                      // /device/{index}/temp
            {
                target = target.substr(std::string("temp/").length());
                return handle_temp(std::move(req), target, hDevice);
            } else {
                return not_found(req.target(), req);
            }
        } else {                                                    // /device/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            body = get_device_properties(hDevice, ctlResult);
            if (ctlResult == CTL_RESULT_SUCCESS)
                return get_response(req, body);
            else
                return server_error(magic_enum::enum_name(ctlResult), req);
        }
    } else {                                                        // /device
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        json::object body = {};

        body["count"] = devicesCount;

        json::array devices;
        for(int i = 0; i < devicesCount; i++) {
            devices.push_back(get_device_properties(hDevices[i], ctlResult));
            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hDevices);
                return server_error(magic_enum::enum_name(ctlResult), req);
            }
        }

        body["devices"] = devices;

        free(hDevices);
        return get_response(req, body);
    }
}

