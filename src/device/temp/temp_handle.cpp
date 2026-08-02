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
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice,
    json::object &body_base)
{
    ctl_result_t ctlResult;

    ctl_temp_handle_t *hTemps = nullptr;
    uint32_t sensorCount = 0;
    ctlEnumTemperatureSensors(hDevice, &sensorCount, hTemps);
    hTemps = (ctl_temp_handle_t *)malloc(sizeof(ctl_temp_handle_t) * sensorCount);
    ctlResult = ctlEnumTemperatureSensors(hDevice, &sensorCount, hTemps);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hTemps);
        return server_error("ctlEnumTemperatureSensors: " + std::string(enum_name(ctlResult)), req);
    }

    if (!reqElements.target.empty()) {
        size_t tempInd;
        try {
            tempInd = static_cast<size_t>(std::stoul(str_extract(&reqElements.target, '/')));
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

        // Add temp index to base body
        add_value(body_base, reqElements.query, "temp_index", tempInd);

        if (!reqElements.target.empty()) {                                      // /device/{i}/temp/{index}/state
            if (str_starts_with_erase(&reqElements.target, "state/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_temp_state(hTemp, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();
            } else
                return not_found(req.target(), req);

        } else {                                                    // /device/{i}/temp/{index}
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            for (auto& prop : get_temp_properties(hTemp, ctlResult, reqElements.query))
                body_base[prop.key()] = prop.value();
        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body_base);
        else
            return server_error(enum_name(ctlResult), req);
    } else {                                                        // /device/{i}/temp
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        add_value(body_base, reqElements.query, "count", sensorCount);

        json::array sensors;
        for(int i = 0; i < sensorCount; i++) {
            sensors.push_back(get_temp_properties(hTemps[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hTemps);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body_base, reqElements.query, "sensors", sensors);

        free(hTemps);
        return create_response(req, body_base);
    }
}

