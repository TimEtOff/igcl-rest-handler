
#include "boost/json/object.hpp"
#include "device.hpp"
#include "temp/temp.hpp"
#include "fan/fan.hpp"
#include "freq/freq.hpp"

json::object
get_device_properties(
    ctl_device_adapter_handle_t &hDevice,
    ctl_result_t ctlResult,
    const query_type &query = query_type())
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

    add_value(res, query, "device_type"                  , json::value_from(magic_enum::enum_name(pDevice.device_type)));
    add_value(res, query, "supported_subfunction_flags"  , pDevice.supported_subfunction_flags);
    add_value(res, query, "driver_version"               , pDevice.driver_version);
    add_value(res, query, "driver_version_str"           , json::value_from(getReadableVersion(pDevice.driver_version)));
    add_value(res, query, "pci_vendor_id"                , pDevice.pci_vendor_id);
    add_value(res, query, "pci_device_id"                , pDevice.pci_device_id);
    add_value(res, query, "rev_id"                       , pDevice.rev_id);
    add_value(res, query, "num_eus_per_sub_slice"        , pDevice.num_eus_per_sub_slice);
    add_value(res, query, "num_sub_slices_per_slice"     , pDevice.num_sub_slices_per_slice);
    add_value(res, query, "num_slices"                   , pDevice.num_slices);
    add_value(res, query, "name"                         , pDevice.name);
    add_value(res, query, "graphics_adapter_properties"  , pDevice.graphics_adapter_properties);
    add_value(res, query, "frequency"                    , pDevice.Frequency);
    add_value(res, query, "pci_subsys_id"                , pDevice.pci_subsys_id);
    add_value(res, query, "pci_subsys_vendor_id"         , pDevice.pci_subsys_vendor_id);
    add_value(res, query, "adapter_bdf"                  , json::object({
        {"bus", pDevice.adapter_bdf.bus},
        {"device", pDevice.adapter_bdf.device},
        {"function", pDevice.adapter_bdf.function}
    }));
    add_value(res, query, "num_xe_cores"                 , pDevice.num_xe_cores);

    ctl_dev_prop_properties_t dpDevice = { 0 };
    dpDevice.Size    = sizeof(ctl_dev_prop_properties_t);
    dpDevice.Version = 0;
    ctl_result_t devResult = ctlDevPropGetProperties(hDevice, &dpDevice);
    if (devResult == CTL_RESULT_SUCCESS)
        add_value(res, query, "is_workstation", dpDevice.isWorkstation);

    free(pDevice.pDeviceID);

    return res;
}

http::message_generator
handle_device(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_api_handle_t &hAPIHandle)
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

    if (!reqElements.target.empty()) {
        size_t deviceInd;
        try {
            deviceInd = static_cast<size_t>(std::stoul(str_extract(&reqElements.target, '/')));
        }
        catch (std::invalid_argument) {
            free(hDevices);
            return bad_request("Invalid device index", req);
        }

        if (deviceInd >= devicesCount) {
            free(hDevices);
            return bad_request("Device index out of bounds (" + std::to_string(deviceInd) + " out of " + std::to_string(devicesCount) + ")", req);
        }

        ctl_device_adapter_handle_t hDevice = hDevices[deviceInd];
        free(hDevices);

        // Create base response body with device index
        json::object body_base = {};
        add_value(body_base, reqElements.query, "device_index", deviceInd);

        if (!reqElements.target.empty()) {                                      // /device/{index}/...

            if (str_starts_with_erase(&reqElements.target, "temp/"))            // /device/{index}/temp
                return handle_temp(std::move(req), reqElements, hDevice, body_base);
            else if (str_starts_with_erase(&reqElements.target, "fan/"))        // /device/{index}/fan
                return handle_fan(std::move(req), reqElements, hDevice, body_base);
            else if (str_starts_with_erase(&reqElements.target, "freq/"))       // /device/{index}/freq
                return handle_freq(std::move(req), reqElements, hDevice, body_base);
            else
                return not_found(req.target(), req);

        } else {                                                    // /device/{index}
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            // Merge device properties into body
            for (auto& prop : get_device_properties(hDevice, ctlResult, reqElements.query)) {
                body_base[prop.key()] = prop.value();
            }

            if (ctlResult == CTL_RESULT_SUCCESS)
                return create_response(req, body_base);
            else
                return server_error(enum_name(ctlResult), req);
        }
    } else {                                                        // /device
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        json::object body = {};

        add_value(body, reqElements.query, "count", devicesCount);

        json::array devices;
        for(int i = 0; i < devicesCount; i++) {
            devices.push_back(get_device_properties(hDevices[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hDevices);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body, reqElements.query, "devices", devices);

        free(hDevices);
        return create_response(req, body);
    }
}

