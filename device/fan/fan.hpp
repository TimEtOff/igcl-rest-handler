#ifndef FAN_HPP
#define FAN_HPP

#include "../../http_server.hpp"

http::message_generator
handle_fan(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice);

#endif // FAN_HPP