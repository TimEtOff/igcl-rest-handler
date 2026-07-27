#ifndef TEMP_HPP
#define TEMP_HPP

#include "../../http_server.hpp"

http::message_generator
handle_temp(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_device_adapter_handle_t &hDevice);

#endif // TEMP_HPP