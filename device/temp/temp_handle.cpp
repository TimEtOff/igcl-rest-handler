
#include "temp.hpp"

json::object get_temp_properties(ctl_temp_handle_t& hDevice, ctl_result_t& ctlResult)
{
    json::object res = {};

    // Get Adapter Properties
    ctl_temp_properties_t pSensor = { 0 };

    pSensor.Size           = sizeof(ctl_temp_properties_t);
    pSensor.Version        = 0;
    ctlResult = ctlTemperatureGetProperties(hDevice, &pSensor);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    res["type"] = magic_enum::enum_name(pSensor.type);
    res["max_temperature"] = pSensor.maxTemperature;

    return res;
}

json::object get_temp_state(ctl_temp_handle_t& hDevice, ctl_result_t& ctlResult)
{
    json::object res = {};

    // Get Adapter Properties
    double temp;
    ctlResult = ctlTemperatureGetState(hDevice, &temp);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    res["temperature"] = temp;

    return res;
}

http::message_generator
handle_temp(
    http::request<http::string_body>&& req,
    std::string& target,
    ctl_device_adapter_handle_t& hDevice)
{
    ctl_result_t ctlResult;

    ctl_temp_handle_t *hTemps = nullptr;
    uint32_t sensorCount = 0;
    ctlEnumTemperatureSensors(hDevice, &sensorCount, hTemps);
    hTemps = (ctl_temp_handle_t *)malloc(sizeof(ctl_temp_handle_t) * sensorCount);
    ctlResult = ctlEnumTemperatureSensors(hDevice, &sensorCount, hTemps);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hTemps);
        return server_error("ctlEnumerateDevices: " + std::string(magic_enum::enum_name(ctlResult)), req);
    }

    json_body::value_type body;

    if (!target.empty()) {
        size_t pos = target.find_first_of('/');
        size_t tempInd = static_cast<size_t>(std::stoul(target.substr(0, pos)));
        target = target.substr(pos + 1);

        if (tempInd >= sensorCount) {
            free(hTemps);
            return bad_request("Device index out of bounds (" + std::to_string(tempInd) + " out of " + std::to_string(sensorCount) + ")", req);
        }

        ctl_temp_handle_t hDevice = hTemps[tempInd];
        free(hTemps);

        if (!target.empty()) {                                      // /device/{i}/temp/{index}/state
            if (target.rfind("state/", 0) == 0) {
                // Make sure we can handle the method
                if( req.method() != http::verb::get &&
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                body = get_temp_state(hDevice, ctlResult);
                if (ctlResult == CTL_RESULT_SUCCESS)
                    return get_response(req, body);
                else
                    return server_error(magic_enum::enum_name(ctlResult), req);
            } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/temp/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            body = get_temp_properties(hDevice, ctlResult);
            if (ctlResult == CTL_RESULT_SUCCESS)
                return get_response(req, body);
            else
                return server_error(magic_enum::enum_name(ctlResult), req);
        }
    } else {                                                        // /device/{i}/temp
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        json::object body = {};

        body["count"] = sensorCount;

        json::array sensors;
        for(int i = 0; i < sensorCount; i++) {
            sensors.push_back(get_temp_properties(hTemps[i], ctlResult));
            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hTemps);
                return server_error(magic_enum::enum_name(ctlResult), req);
            }
        }

        body["sensors"] = sensors;

        free(hTemps);
        return get_response(req, body);
    }
}

