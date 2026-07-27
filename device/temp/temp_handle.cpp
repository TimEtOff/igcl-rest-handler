#include "temp.hpp"

json::object
get_temp_properties(
    ctl_temp_handle_t &hSensor,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_temp_properties_t pSensor = { 0 };

    pSensor.Size           = sizeof(ctl_temp_properties_t);
    pSensor.Version        = 0;
    ctlResult = ctlTemperatureGetProperties(hSensor, &pSensor);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "type"               , json::value_from(magic_enum::enum_name(pSensor.type)));
    add_value(res, query, "max_temperature"    , pSensor.maxTemperature);

    return res;
}

json::object
get_temp_state(
    ctl_temp_handle_t &hDevice,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    double temp;
    ctlResult = ctlTemperatureGetState(hDevice, &temp);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "temperature", temp);

    return res;
}

http::message_generator
handle_temp(
    http::request<http::string_body>&& req,
    std::string &target,
    const query_type &query,
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
        return server_error("ctlEnumTemperatureSensors: " + std::string(magic_enum::enum_name(ctlResult)), req);
    }

    json_body::value_type body;

    if (!target.empty()) {
        size_t tempInd;
        try {
            tempInd = static_cast<size_t>(std::stoul(str_extract(&target, '/')));
        }
        catch (std::invalid_argument) {
            free(hTemps);
            return bad_request("Invalid temp sensor index", req);
        }

        if (tempInd >= sensorCount) {
            free(hTemps);
            return bad_request("Temp sensor index out of bounds (" + std::to_string(tempInd) + " out of " + std::to_string(sensorCount) + ")", req);
        }

        ctl_temp_handle_t hTemp = hTemps[tempInd];
        free(hTemps);

        if (!target.empty()) {                                      // /device/{i}/temp/{index}/state
            if (str_starts_with_erase(&target, "state/") && target.empty()) {
                if( req.method() != http::verb::get &&
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                body = get_temp_state(hTemp, ctlResult, query);
            } else
                return not_found(req.target(), req);

        } else {                                                    // /device/{i}/temp/{index}
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            body = get_temp_properties(hTemp, ctlResult, query);
        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body);
        else
            return server_error(enum_name(ctlResult), req);
    } else {                                                        // /device/{i}/temp
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        json::object body = {};

        add_value(body, query, "count", sensorCount);

        json::array sensors;
        for(int i = 0; i < sensorCount; i++) {
            sensors.push_back(get_temp_properties(hTemps[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hTemps);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body, query, "sensors", sensors);

        free(hTemps);
        return create_response(req, body);
    }
}

