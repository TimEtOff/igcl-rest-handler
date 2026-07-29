#include "boost/beast/http/verb.hpp"
#include "boost/json/array.hpp"
#include "boost/json/object.hpp"
#include "fan.hpp"
#include "magic_enum/magic_enum.hpp"
#include <cstdint>
#include <string>

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
    ctl_fan_handle_t &hFan,
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
    ctlResult = ctlFanGetState(hFan, units, &speed);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "units", json::value_from(magic_enum::enum_name(units)));
    add_value(res, query, "speed", speed);

    return res;
}

json::object
get_fan_config(
    ctl_fan_handle_t &hFan,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    ctl_fan_config_t cFan;
    cFan.Size           = sizeof(ctl_fan_config_t);
    cFan.Version        = 0;
    ctlResult = ctlFanGetConfig(hFan, &cFan);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "mode", json::value_from(magic_enum::enum_name(cFan.mode)));
    add_value(res, query, "speed_fixed", {
        {"speed", cFan.speedFixed.speed},
        {"units", magic_enum::enum_name(cFan.speedFixed.units)}
    });

    json::array table;
    for (size_t i = 0; i < cFan.speedTable.numPoints; i++) {
        table.push_back({
            {"temperature", cFan.speedTable.table[i].temperature},
            {"speed", {
                {"speed", cFan.speedTable.table[i].speed.speed},
                {"units", magic_enum::enum_name(cFan.speedTable.table[i].speed.units)}
            }}
        });
    }
    add_value(res, query, "speed_table", {
        {"num_points", cFan.speedTable.numPoints},
        {"table", table}
    });

    return res;
}

std::string
edit_fan_config(
    ctl_fan_handle_t &hFan,
    ctl_result_t &ctlResult,
    const request_elements_t &reqElements)
{
    if (!reqElements.body.is_object())
        return "Missing body";

    const json::object &body = reqElements.body.as_object();
    auto mode = magic_enum::enum_cast<ctl_fan_speed_mode_t>(
        body.contains("mode") ? body.at("mode").as_string() : "",
        magic_enum::case_insensitive
    ).value_or(CTL_FAN_SPEED_MODE_DEFAULT);

    switch (mode) {
        case CTL_FAN_SPEED_MODE_DEFAULT:
            ctlResult = ctlFanSetDefaultMode(hFan);
            break;
        case CTL_FAN_SPEED_MODE_FIXED:
            {
                ctl_fan_speed_t speed;
                speed.Size      = sizeof(ctl_fan_speed_t);
                speed.Version   = 0;

                if (!(body.contains("speed") && body.at("speed").is_object()))
                    return "Body: missing speed object";
                const json::object &speedObj = body.at("speed").as_object();

                if (!(speedObj.contains("speed") && speedObj.at("speed").is_int64()))
                    return "Body: missing speed.speed number";
                speed.speed = speedObj.at("speed").as_int64();

                speed.units = magic_enum::enum_cast<ctl_fan_speed_units_t>(
                    speedObj.contains("units") ? speedObj.at("units").as_string() : "",
                    magic_enum::case_insensitive
                ).value_or(CTL_FAN_SPEED_UNITS_RPM);

                ctlResult = ctlFanSetFixedSpeedMode(hFan, &speed);
                break;
            }
        case CTL_FAN_SPEED_MODE_TABLE:
            {
                ctl_fan_speed_table_t table;
                table.Size      = sizeof(ctl_fan_speed_table_t);
                table.Version   = 0;

                if (!(body.contains("table") && body.at("table").is_object()))
                    return "Body: missing table object";
                const json::object &tableObj = body.at("table").as_object();

                if (!(tableObj.contains("num_points") && tableObj.at("num_points").is_int64()))
                    return "Body: missing table.num_points number";
                table.numPoints = tableObj.at("num_points").as_int64();

                if (!(tableObj.contains("table") && tableObj.at("table").is_array()))
                    return "Body: missing table.table array";
                const json::array &tableArray = tableObj.at("table").as_array();

                if (table.numPoints >= 0)
                {
                    if (table.numPoints > tableArray.size() || table.numPoints > CTL_FAN_TEMP_SPEED_PAIR_COUNT)
                        return "Body: table.num_points is higher than table.table size";

                    for (size_t i = 0; i < table.numPoints; i++) {
                        ctl_fan_temp_speed_t &tempSpeed = table.table[i];

                        if (!tableArray.at(i).is_object())
                            return "Body: missing object in table.table array";
                        const json::object &tempSpeedObj = tableArray.at(i).as_object();

                        boost::system::error_code ec;
                        if (!(tempSpeedObj.contains("temperature") && tempSpeedObj.at("temperature").is_number()))
                            return "Body: missing temperature number in table.table array object";
                        tempSpeed.temperature = tempSpeedObj.at("temperature").to_number<uint32_t>(ec);
                        if (ec)
                            return "Body: wrong format for temperature number in table table array object";

                        if (!(tempSpeedObj.contains("speed") && tempSpeedObj.at("speed").is_object()))
                            return "Body: missing speed object in table.table array object";
                        const json::object &speedObj = tempSpeedObj.at("speed").as_object();

                        if (!(speedObj.contains("speed") && speedObj.at("speed").is_int64()))
                            return "Body: missing speed.speed number in table.table array object";
                        tempSpeed.speed.speed = speedObj.at("speed").as_int64();

                        tempSpeed.speed.units = magic_enum::enum_cast<ctl_fan_speed_units_t>(
                            speedObj.contains("units") ? speedObj.at("units").as_string() : "",
                            magic_enum::case_insensitive
                        ).value_or(CTL_FAN_SPEED_UNITS_RPM);
                    }
                }

                ctlResult = ctlFanSetSpeedTableMode(hFan, &table);
                break;
            }
    }

    return "";
}

http::message_generator
handle_fan(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice,
    json::object &body_base)
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

        // Add fan index to base body
        add_value(body_base, reqElements.query, "fan_index", fanInd);

        if (!reqElements.target.empty()) {
            if (str_starts_with_erase(&reqElements.target, "state/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/fan/{index}/state
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_fan_state(hFan, ctlResult, reqElements.query)) {
                    body_base[prop.key()] = prop.value();
                }
            } else if (str_starts_with_erase(&reqElements.target, "config/") && reqElements.target.empty()) {
                if ( req.method() == http::verb::put) {             // /device/{i}/fan/{index}/config
                    std::string result = edit_fan_config(hFan, ctlResult, reqElements);
                    if (!result.empty())
                        return bad_request(result, req);
                    if (ctlResult != CTL_RESULT_SUCCESS)
                        return server_error(enum_name(ctlResult), req);

                } else if(  req.method() != http::verb::get &&
                            req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_fan_config(hFan, ctlResult, reqElements.query)) {
                    body_base[prop.key()] = prop.value();
                }

             } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/fan/{index}
            // Make sure we can handle the method
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            for (auto& prop : get_fan_properties(hFan, ctlResult, reqElements.query)) {
                body_base[prop.key()] = prop.value();
            }
        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body_base);
        else
            return server_error(enum_name(ctlResult), req);

    } else {                                                        // /device/{i}/fan
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        add_value(body_base, reqElements.query, "count", fanCount);

        json::array fans;
        for(int i = 0; i < fanCount; i++) {
            fans.push_back(get_fan_properties(hFans[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hFans);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body_base, reqElements.query, "fans", fans);

        free(hFans);
        return create_response(req, body_base);
    }
}

