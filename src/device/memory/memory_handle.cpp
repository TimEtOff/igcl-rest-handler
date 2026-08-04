#include "boost/json/value_from.hpp"
#include "memory.hpp"
#include "magic_enum/magic_enum.hpp"
#include <cstdint>
#include <string>

json::object
get_mem_properties(
    ctl_mem_handle_t &hMem,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_mem_properties_t pMem = { 0 };

    pMem.Size           = sizeof(ctl_mem_properties_t);
    pMem.Version        = 0;
    ctlResult = ctlMemoryGetProperties(hMem, &pMem);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "type"            , json::value_from(magic_enum::enum_name(pMem.type)));
    add_value(res, query, "location"        , json::value_from(magic_enum::enum_name(pMem.location)));
    add_value(res, query, "physical_size"   , pMem.physicalSize);
    add_value(res, query, "bus_width"       , pMem.busWidth);
    add_value(res, query, "num_channels"    , pMem.numChannels);

    return res;
}

json::object
get_mem_state(
    ctl_mem_handle_t &hMem,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_mem_state_t state = { 0 };

    state.Size           = sizeof(ctl_mem_state_t);
    state.Version        = 0;
    ctlResult = ctlMemoryGetState(hMem, &state);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "free", state.free);
    add_value(res, query, "size", state.size);

    return res;
}

json::object
get_mem_bandwidth(
    ctl_mem_handle_t &hMem,
    ctl_result_t &ctlResult,
    const query_type &query = query_type())
{
    json::object res = {};

    // Get Adapter Properties
    ctl_mem_bandwidth_t bandwidth = { 0 };

    bandwidth.Size           = sizeof(ctl_mem_bandwidth_t);
    bandwidth.Version        = 0;
    ctlResult = ctlMemoryGetBandwidth(hMem, &bandwidth);

    if (ctlResult != CTL_RESULT_SUCCESS)
        return res;

    add_value(res, query, "max_bandwidth"   , bandwidth.maxBandwidth);
    add_value(res, query, "timestamp"       , bandwidth.timestamp);
    add_value(res, query, "read_counter"    , bandwidth.readCounter);
    add_value(res, query, "write_counter"   , bandwidth.writeCounter);

    return res;
}

http::message_generator
handle_memory(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice,
    json::object &body_base)
{
    ctl_result_t ctlResult;

    ctl_mem_handle_t *hMems = nullptr;
    uint32_t memCount = 0;
    ctlEnumMemoryModules(hDevice, &memCount, hMems);
    hMems = (ctl_mem_handle_t *)malloc(sizeof(ctl_mem_handle_t) * memCount);
    ctlResult = ctlEnumMemoryModules(hDevice, &memCount, hMems);

    if (ctlResult != CTL_RESULT_SUCCESS) {
        free(hMems);
        return server_error("ctlEnumMemoryModules: " + std::string(enum_name(ctlResult)), req);
    }

    if (!reqElements.target.empty()) {
        size_t memInd;
        try {
            memInd = static_cast<size_t>(std::stoul(str_extract(&reqElements.target, '/')));
        }
        catch (std::invalid_argument) {
            free(hMems);
            return bad_request("Invalid module index", req);
        }

        if (memInd >= memCount) {
            free(hMems);
            return bad_request("Module index out of bounds (" + std::to_string(memInd) + " out of " + std::to_string(memCount) + ")", req);
        }

        ctl_mem_handle_t hMem = hMems[memInd];
        free(hMems);

        // Add module index to base body
        add_value(body_base, reqElements.query, "module_index", memInd);

        if (!reqElements.target.empty()) {
            if (str_starts_with_erase(&reqElements.target, "state/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/memory/{index}/state
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_mem_state(hMem, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else if (str_starts_with_erase(&reqElements.target, "bandwidth/") && reqElements.target.empty()) {
                if( req.method() != http::verb::get &&              // /device/{i}/memory/{index}/bandwidth
                    req.method() != http::verb::head)
                    return bad_request("Unknown HTTP-method", req);

                for (auto& prop : get_mem_bandwidth(hMem, ctlResult, reqElements.query))
                    body_base[prop.key()] = prop.value();

            } else
                return not_found(req.target(), req);
        } else {                                                    // /device/{i}/memory/{index}
            if( req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return bad_request("Unknown HTTP-method", req);

            for (auto& prop : get_mem_properties(hMem, ctlResult, reqElements.query))
                body_base[prop.key()] = prop.value();
        }

        if (ctlResult == CTL_RESULT_SUCCESS)
            return create_response(req, body_base);
        else
            return server_error(enum_name(ctlResult), req);

    } else {                                                        // /device/{i}/memory
        // Make sure we can handle the method
        if( req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return bad_request("Unknown HTTP-method", req);

        add_value(body_base, reqElements.query, "count", memCount);

        json::array mems;
        for(int i = 0; i < memCount; i++) {
            mems.push_back(get_mem_properties(hMems[i], ctlResult));

            if (ctlResult != CTL_RESULT_SUCCESS) {
                free(hMems);
                return server_error(enum_name(ctlResult), req);
            }
        }

        add_value(body_base, reqElements.query, "modules", mems);

        free(hMems);
        return create_response(req, body_base);
    }
}

