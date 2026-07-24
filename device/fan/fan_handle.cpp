#include "fan.hpp"
#include "magic_enum/magic_enum.hpp"

json::object get_fan_properties(ctl_fan_handle_t& hFan, ctl_result_t& ctlResult)
{
    json::object res = {};

    // Get Adapter Properties
    ctl_fan_properties_t pFan = { 0 };

    pFan.Size           = sizeof(ctl_fan_properties_t);
    pFan.Version        = 0;
    ctlResult = ctlFanGetProperties(hFan, &pFan);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    res["can_control"] = pFan.canControl;
    res["supported_modes"] = pFan.supportedModes; // TODO Change int to array of all enabled flags
    res["supported_units"] = pFan.supportedUnits;
    res["max_rpm"] = pFan.maxRPM;
    res["max_points"] = pFan.maxPoints;

    return res;
}

json::object get_fan_state(ctl_fan_handle_t& hDevice, ctl_result_t& ctlResult)
{
    json::object res = {};

    ctl_fan_speed_units_t units = CTL_FAN_SPEED_UNITS_RPM;
    int32_t speed;
    ctlResult = ctlFanGetState(hDevice, units, &speed);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    res["units"] = magic_enum::enum_name(units);
    res["speed"] = speed;

    return res;
}

http::message_generator
handle_fan(
    http::request<http::string_body>&& req,
    std::string& target,
    ctl_device_adapter_handle_t& hDevice)
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

    if (!target.empty()) {
        size_t pos = target.find_first_of('/');
        size_t tempInd = static_cast<size_t>(std::stoul(target.substr(0, pos)));
        target = target.substr(pos + 1);

        if (tempInd >= fanCount) {
            free(hFans);
            return bad_request("Fan index out of bounds (" + std::to_string(tempInd) + " out of " + std::to_string(fanCount) + ")", req);
        }

        ctl_fan_handle_t hFan = hFans[tempInd];
        free(hFans);

        if (!target.empty()) {                                      // /device/{i}/fan/{index}/state
            if (target.rfind("state/", 0) == 0) { // TODO For all, deny if target is not empty afer reaching endpoint
                // Make sure we can handle the method
                if( req.method() != http::verb::get &&
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                body = get_fan_state(hFan, ctlResult);
                if (ctlResult == CTL_RESULT_SUCCESS)
                    return get_response(req, body);
                else
                    return server_error(magic_enum::enum_name(ctlResult), req);
            } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/fan/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            body = get_fan_properties(hFan, ctlResult);
            if (ctlResult == CTL_RESULT_SUCCESS)
                return get_response(req, body);
            else
                return server_error(magic_enum::enum_name(ctlResult), req);
        }
    } else {                                                        // /device/{i}/fan
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        json::object body = {};

        body["count"] = fanCount;

        json::array fans;
        for(int i = 0; i < fanCount; i++) {
            fans.push_back(get_fan_properties(hFans[i], ctlResult));
            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hFans);
                return server_error(magic_enum::enum_name(ctlResult), req);
            }
        }

        body["fans"] = fans;

        free(hFans);
        return get_response(req, body);
    }
}

