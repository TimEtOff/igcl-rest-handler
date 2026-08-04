#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "../../http_server.hpp"

http::message_generator
handle_memory(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice,
    json::object &body_base);

#endif // MEMORY_HPP