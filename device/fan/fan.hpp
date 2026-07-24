#ifndef FAN_HPP
#define FAN_HPP

#include "../../http_server.hpp"

http::message_generator
handle_fan(
    http::request<http::string_body>&& req,
    std::string& target,
    ctl_device_adapter_handle_t& hDevice);

#endif // FAN_HPP