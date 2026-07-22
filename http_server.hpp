#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <magic_enum/magic_enum.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <charconv>
#include <thread>
#include <vector>

#include "json_body.hpp"
#include "igcl/igcl_api.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

beast::string_view mime_type(beast::string_view path);

std::string path_cat(beast::string_view base, beast::string_view path);

void fail(beast::error_code ec, char const* what);

int http_run(void);

// Returns a bad request response
http::response<http::string_body>
bad_request(
    beast::string_view why,
    http::request<http::string_body> const& req);

// Returns a not found response
http::response<http::string_body>
not_found(
    beast::string_view target,
    http::request<http::string_body> const& req);

// Returns a server error response
http::response<http::string_body>
server_error(
    beast::string_view what,
    http::request<http::string_body> const& req);

http::message_generator
get_response(
    http::request<http::string_body> const& req,
    json_body::value_type body);

std::string getReadableVersion(uint64_t integer);

#endif // HTTP_SERVER_HPP
