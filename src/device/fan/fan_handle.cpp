#include "fan.hpp"

json::object
get_fan_properties(
    ctl_fan_handle_t &hFan,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_fan_properties_t pFan = { 0 };

    pFan.Size           = sizeof(ctl_fan_properties_t);
    pFan.Version        = 0;
    ctlResult = ctlFanGetProperties(hFan, &pFan);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "can_control"     , pFan.canControl);
    add_value(res, query, "supported_modes" , pFan.supportedModes); // TODO Change int to array of all enabled flags
    add_value(res, query, "supported_units" , pFan.supportedUnits);
    add_value(res, query, "max_rpm"         , pFan.maxRPM);
    add_value(res, query, "max_points"      , pFan.maxPoints);

    return res;
}

json::object
get_fan_state(
    ctl_fan_handle_t &hDevice,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    std::string unitStr;
    auto unitsIt = query.find("units");
    if (unitsIt != query.end()) {
        unitStr = std::any_cast<std::string>(query.at("units"));
    }

    auto units = magic_enum::enum_cast<ctl_fan_speed_units_t>(
        unitStr,
        magic_enum::case_insensitive
    ).value_or(CTL_FAN_SPEED_UNITS_RPM);

    int32_t speed;
    ctlResult = ctlFanGetState(hDevice, units, &speed);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "units", json::value_from(magic_enum::enum_name(units)));
    add_value(res, query, "speed", speed);

    return res;
}

http::message_generator
handle_fan(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice)
{
    ctl_result_t ctlResult;

    ctl_fan_handle_t *hFans = nullptr;
    uint32_t fanCount = 0;
    ctlEnumFans(hDevice, &fanCount, hFans);
    hFans = (ctl_fan_handle_t *)malloc(sizeof(ctl_fan_handle_t) * fanCount);
    ctlResult = ctlEnumFans(hDevice, &fanCount, hFans);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hFans);
        return server_error("ctlEnumFans: " + std::string(magic_enum::enum_name(ctlResult)), req);
    }

    json_body::value_type body;

    if (!reqElements.target.empty()) {
        size_t fanInd;
        try {
            fanInd = static_cast<size_t>(std::stoul(str_extract(&reqElements.target, '/')));
        }
        catch (std::invalid_argument) {
            free(hFans);
            return bad_request("Invalid fan index", req);
        }

        if (fanInd >= fanCount) {
            free(hFans);
            return bad_request("Fan index out of bounds (" + std::to_string(fanInd) + " out of " + std::to_string(fanCount) + ")", req);
        }

        ctl_fan_handle_t hFan = hFans[fanInd];
        free(hFans);

        if (!reqElements.target.empty()) {                                      // /device/{i}/fan/{index}/state
            if (str_starts_with_erase(&reqElements.target, "state/") && reqElements.target.empty()) {
                // Make sure we can handle the method
                if( req.method() != http::verb::get &&
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                body = get_fan_state(hFan, ctlResult, reqElements.query);
            } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/fan/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            body = get_fan_properties(hFan, ctlResult, reqElements.query);
        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body);
        else
            return server_error(enum_name(ctlResult), req);

    } else {                                                        // /device/{i}/fan
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        json::object body = {};

        add_value(body, reqElements.query, "count", fanCount);

        json::array fans;
        for(int i = 0; i < fanCount; i++) {
            fans.push_back(get_fan_properties(hFans[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hFans);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body, reqElements.query, "fans", fans);

        free(hFans);
        return create_response(req, body);
    }
}

