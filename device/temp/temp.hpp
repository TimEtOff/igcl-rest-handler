#ifndef TEMP_HPP
#define TEMP_HPP

#include "../../http_server.hpp"

http::message_generator
handle_temp(
    http::request<http::string_body>&& req,
    std::string& target,
    ctl_device_adapter_handle_t& hDevice);

#endif // TEMP_HPP