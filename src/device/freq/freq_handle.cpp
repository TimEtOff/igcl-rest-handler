#include "boost/beast/http/verb.hpp"
#include "boost/json/array.hpp"
#include "boost/json/object.hpp"
#include "boost/json/value_from.hpp"
#include "freq.hpp"
#include "magic_enum/magic_enum.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

json::object
get_freq_properties(
    ctl_freq_handle_t &hFreq,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_freq_properties_t pFreq = { 0 };

    pFreq.Size           = sizeof(ctl_freq_properties_t);
    pFreq.Version        = 0;
    ctlResult = ctlFrequencyGetProperties(hFreq, &pFreq);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "type"            , json::value_from(magic_enum::enum_name(pFreq.type)));
    add_value(res, query, "can_control"     , pFreq.canControl);
    add_value(res, query, "min"             , pFreq.min);
    add_value(res, query, "max"             , pFreq.max);

    return res;
}

json::object
get_freq_clocks(
    ctl_freq_handle_t &hFreq,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    double *clocks = nullptr;
    uint32_t clocksCount = 0;
    ctlFrequencyGetAvailableClocks(hFreq, &clocksCount, clocks);
    clocks = (double *)malloc(sizeof(double) * clocksCount);
    ctlResult = ctlFrequencyGetAvailableClocks(hFreq, &clocksCount, clocks);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "count",          clocksCount);

    json::array frequencies;
    for (size_t i = 0; i < clocksCount; i++) {
        frequencies.push_back(clocks[i]);
    }
    add_value(res, query, "frequencies",    frequencies);

    return res;
}

json::object
get_freq_range(
    ctl_freq_handle_t &hFreq,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_freq_range_t range = { 0 };

    range.Size           = sizeof(ctl_freq_range_t);
    range.Version        = 0;
    ctlResult = ctlFrequencyGetRange(hFreq, &range);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "min", range.min);
    add_value(res, query, "max", range.max);

    return res;
}

std::string
set_freq_range(
    ctl_freq_handle_t &hFreq,
    ctl_result_t &ctlResult,
    const request_elements_t &reqElements)
{
    if (!reqElements.body.is_object())
        return "Missing body";

    const json::object &body = reqElements.body.as_object();

    // Get Adapter Properties
    ctl_freq_range_t range = { 0 };

    range.Size           = sizeof(ctl_freq_range_t);
    range.Version        = 0;
    ctlResult = ctlFrequencyGetRange(hFreq, &range);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return enum_name(ctlResult);

    boost::system::error_code ec;
    if (body.contains("min") && body.at("min").is_number())
    {
        float min = body.at("min").to_number<float>(ec);
        if (ec)
            return "Body: wrong format for min number";
        else
            range.min = min;
    }

    if (body.contains("max") && body.at("max").is_number())
    {
        float max = body.at("max").to_number<float>(ec);
        if (ec)
            return "Body: wrong format for max number";
        else
            range.max = max;
    }

    ctlResult = ctlFrequencySetRange(hFreq, &range);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return enum_name(ctlResult);

    return "";
}

json::object
get_freq_state(
    ctl_freq_handle_t &hFreq,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_freq_state_t state = { 0 };

    state.Size           = sizeof(ctl_freq_range_t);
    state.Version        = 0;
    ctlResult = ctlFrequencyGetState(hFreq, &state);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "current_voltage",    state.currentVoltage);
    add_value(res, query, "request",            state.request);
    add_value(res, query, "tdp",                state.tdp);
    add_value(res, query, "efficient",          state.efficient);
    add_value(res, query, "actual",             state.actual);
    add_value(res, query, "throttle_reasons",   state.throttleReasons);

    return res;
}

json::object
get_freq_throttle_time(
    ctl_freq_handle_t &hFreq,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_freq_throttle_time_t throttleT = { 0 };

    throttleT.Size           = sizeof(ctl_freq_range_t);
    throttleT.Version        = 0;
    ctlResult = ctlFrequencyGetThrottleTime(hFreq, &throttleT);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "throttle_time",  throttleT.throttleTime);
    add_value(res, query, "timestamp",      throttleT.timestamp);

    return res;
}

http::message_generator
handle_freq(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice,
    json::object &body_base)
{
    ctl_result_t ctlResult;

    ctl_freq_handle_t *hFreqs = nullptr;
    uint32_t freqCount = 0;
    ctlEnumFrequencyDomains(hDevice, &freqCount, hFreqs);
    hFreqs = (ctl_freq_handle_t *)malloc(sizeof(ctl_freq_handle_t) * freqCount);
    ctlResult = ctlEnumFrequencyDomains(hDevice, &freqCount, hFreqs);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hFreqs);
        return server_error("ctlEnumFrequencyDomains: " + std::string(magic_enum::enum_name(ctlResult)), req);
    }

    if (!reqElements.target.empty()) {
        size_t freqInd;
        try {
            freqInd = static_cast<size_t>(std::stoul(str_extract(&reqElements.target, '/')));
        }
        catch (std::invalid_argument) {
            free(hFreqs);
            return bad_request("Invalid freq domain index", req);
        }

        if (freqInd >= freqCount) {
            free(hFreqs);
            return bad_request("Freq domain index out of bounds (" + std::to_string(freqInd) + " out of " + std::to_string(freqCount) + ")", req);
        }

        ctl_freq_handle_t hFreq = hFreqs[freqInd];
        free(hFreqs);

        // Add freq index to base body
        add_value(body_base, reqElements.query, "freq_index", freqInd);

        if (!reqElements.target.empty()) {
            if (str_starts_with_erase(&reqElements.target, "clocks/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/freq/{index}/clocks
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_freq_clocks(hFreq, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else if (str_starts_with_erase(&reqElements.target, "range/") && reqElements.target.empty()) {
                if ( req.method() == http::verb::put) {             // /device/{i}/freq/{index}/range
                    std::string result = set_freq_range(hFreq, ctlResult, reqElements);
                    if (!result.empty())
                        return bad_request(result, req);
                    if (ctlResult != CTL_RESULT_SUCCESS)
                        return server_error(enum_name(ctlResult), req);

                } else if ( req.method() != http::verb::get &&
                            req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_freq_range(hFreq, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else if (str_starts_with_erase(&reqElements.target, "state/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/freq/{index}/state
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_freq_state(hFreq, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else if (str_starts_with_erase(&reqElements.target, "throttle/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/freq/{index}/throttle
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_freq_throttle_time(hFreq, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/freq/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            for (auto& prop : get_freq_properties(hFreq, ctlResult, reqElements.query))
                body_base[prop.key()] = prop.value();

        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body_base);
        else
            return server_error(enum_name(ctlResult), req);

    } else {                                                        // /device/{i}/freq
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        add_value(body_base, reqElements.query, "count", freqCount);

        json::array domains;
        for(int i = 0; i < freqCount; i++) {
            domains.push_back(get_freq_properties(hFreqs[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hFreqs);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body_base, reqElements.query, "domains", domains);

        free(hFreqs);
        return create_response(req, body_base);
    }
}

