#include "power.hpp"
#include <cstdint>
#include <string>

json::object
get_power_properties(
    ctl_pwr_handle_t &hPower,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_power_properties_t properties = { 0 };

    properties.Size           = sizeof(ctl_power_properties_t);
    properties.Version        = 0;
    ctlResult = ctlPowerGetProperties(hPower, &properties);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "can_control"     , properties.canControl);
    add_value(res, query, "default_limit"   , properties.defaultLimit);
    add_value(res, query, "min_limit"       , properties.minLimit);
    add_value(res, query, "max_limit"       , properties.maxLimit);
    return res;
}

json::object
get_power_counter(
    ctl_pwr_handle_t &hPower,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    ctl_power_energy_counter_t counter = { 0 };

    counter.Size              = sizeof(ctl_power_energy_counter_t);
    counter.Version           = 0;
    ctlResult = ctlPowerGetEnergyCounter(hPower, &counter);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "energy"      , counter.energy);
    add_value(res, query, "timestamp"   , counter.timestamp);
    return res;
}

json::object
get_power_limits(
    ctl_pwr_handle_t &hPower,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    ctl_power_limits_t limits;
    limits.Size          = sizeof(ctl_power_limits_t);
    limits.Version       = 0;
    ctlResult = ctlPowerGetLimits(hPower, &limits);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "sustained_power_limit", {
        {"enabled"  , limits.sustainedPowerLimit.enabled},
        {"power"    , limits.sustainedPowerLimit.power},
        {"interval" , limits.sustainedPowerLimit.interval}
    });
    add_value(res, query, "burst_power_limit", {
        {"enabled"  , limits.burstPowerLimit.enabled},
        {"power"    , limits.burstPowerLimit.power}
    });
    add_value(res, query, "peak_power_limit", {
        {"power_ac" , limits.peakPowerLimits.powerAC},
        {"power_dc" , limits.peakPowerLimits.powerDC}
    });

    return res;
}

std::string
set_power_limits(
    ctl_pwr_handle_t &hPower,
    ctl_result_t &ctlResult,
    const request_elements_t &reqElements)
{
    ctl_power_limits_t limits;
    limits.Size          = sizeof(ctl_power_limits_t);
    limits.Version       = 0;
    ctlResult = ctlPowerGetLimits(hPower, &limits);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return enum_name(ctlResult);

    if (!reqElements.body.is_object())
        return "Missing body";

    const json::object &body = reqElements.body.as_object();

    if (body.contains("sustained_power_limit") && body.at("sustained_power_limit").is_object())
    {
        const json::object &limitObj = body.at("sustained_power_limit").as_object();

        if (limitObj.contains("enabled") && limitObj.at("enabled").is_bool())
            limits.sustainedPowerLimit.enabled = limitObj.at("enabled").as_bool();

        if (limitObj.contains("power") && limitObj.at("power").is_int64())
            limits.sustainedPowerLimit.power = limitObj.at("power").as_int64();

        if (limitObj.contains("interval") && limitObj.at("interval").is_int64())
            limits.sustainedPowerLimit.interval = limitObj.at("interval").as_int64();
    }

    if (body.contains("burst_power_limit") && body.at("burst_power_limit").is_object())
    {
        const json::object &limitObj = body.at("burst_power_limit").as_object();

        if (limitObj.contains("enabled") && limitObj.at("enabled").is_bool())
            limits.burstPowerLimit.enabled = limitObj.at("enabled").as_bool();

        if (limitObj.contains("power") && limitObj.at("power").is_int64())
            limits.burstPowerLimit.power = limitObj.at("power").as_int64();
    }

    if (body.contains("peak_power_limit") && body.at("peak_power_limit").is_object())
    {
        const json::object &limitObj = body.at("peak_power_limit").as_object();

        if (limitObj.contains("power_ac") && limitObj.at("power_ac").is_int64())
            limits.peakPowerLimits.powerAC = limitObj.at("power_ac").as_int64();

        if (limitObj.contains("power_dc") && limitObj.at("power_dc").is_int64())
            limits.peakPowerLimits.powerDC = limitObj.at("power_dc").as_int64();
    }

    ctlResult = ctlPowerSetLimits(hPower, &limits);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return enum_name(ctlResult);

    return "";
}

http::message_generator
handle_power(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice,
    json::object &body_base)
{
    ctl_result_t ctlResult;

    ctl_pwr_handle_t *hDomains = nullptr;
    uint32_t domainCount = 0;
    ctlEnumPowerDomains(hDevice, &domainCount, hDomains);
    hDomains = (ctl_pwr_handle_t *)malloc(sizeof(ctl_pwr_handle_t) * domainCount);
    ctlResult = ctlEnumPowerDomains(hDevice, &domainCount, hDomains);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hDomains);
        return server_error("ctlEnumPowerDomains: " + std::string(enum_name(ctlResult)), req);
    }

    if (!reqElements.target.empty()) {
        size_t domainInd;
        try {
            domainInd = static_cast<size_t>(std::stoul(str_extract(&reqElements.target, '/')));
        }
        catch (std::invalid_argument) {
            free(hDomains);
            return bad_request("Invalid power domain index", req);
        }

        if (domainInd >= domainCount) {
            free(hDomains);
            return bad_request("Power domain index out of bounds (" + std::to_string(domainInd) + " out of " + std::to_string(domainCount) + ")", req);
        }

        ctl_pwr_handle_t hPower = hDomains[domainInd];
        free(hDomains);

        // Add fan index to base body
        add_value(body_base, reqElements.query, "power_index", domainInd);

        if (!reqElements.target.empty()) {
            if (str_starts_with_erase(&reqElements.target, "counter/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/power/{index}/counter
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_power_counter(hPower, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else if (str_starts_with_erase(&reqElements.target, "limits/") && reqElements.target.empty()) {
                if ( req.method() == http::verb::put) {             // /device/{i}/power/{index}/limits
                    std::string result = set_power_limits(hPower, ctlResult, reqElements);
                    if (!result.empty())
                        return bad_request(result, req);
                    if (ctlResult != CTL_RESULT_SUCCESS)
                        return server_error(enum_name(ctlResult), req);

                } else if(  req.method() != http::verb::get &&
                            req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_power_limits(hPower, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

             } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/power/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            for (auto& prop : get_power_properties(hPower, ctlResult, reqElements.query))
                body_base[prop.key()] = prop.value();
        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body_base);
        else
            return server_error(enum_name(ctlResult), req);

    } else {                                                        // /device/{i}/power
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        add_value(body_base, reqElements.query, "count", domainCount);

        json::array fans;
        for(int i = 0; i < domainCount; i++) {
            fans.push_back(get_power_properties(hDomains[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hDomains);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body_base, reqElements.query, "domains", fans);

        free(hDomains);
        return create_response(req, body_base);
    }
}

