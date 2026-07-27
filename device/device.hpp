#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "../http_server.hpp"

http::message_generator
handle_device(
    http::request<http::string_body>&& req,
    request_elements_t &reqElements,
    ctl_api_handle_t &hAPIHandle);

#endif // DEVICE_HPP