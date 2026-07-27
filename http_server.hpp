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
#include <map>

#include "json_body.hpp"
#include "igcl/igcl_api.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef std::map<std::string, std::any> query_type;

typedef struct _request_elements_t
{
    std::string target;
    query_type query;
} request_elements_t;

// ctl_result_t values range is too wide for magic_enum, so it's hardcoded
std::string enum_name(
    ctl_result_t value) noexcept;

beast::string_view mime_type(beast::string_view path);

std::string path_cat(beast::string_view base, beast::string_view path);

void fail(beast::error_code ec, char const* what);

int http_run(void);

// Returns a bad request response
http::response<json_body>
bad_request(
    beast::string_view why,
    http::request<http::string_body> const& req);

// Returns a not found response
http::response<json_body>
not_found(
    beast::string_view target,
    http::request<http::string_body> const& req);

// Returns a server error response
http::response<json_body>
server_error(
    beast::string_view what,
    http::request<http::string_body> const& req);

    std::string
str_extract(
    std::string *origin,
    const char sep);

bool
str_starts_with_erase(
    std::string *origin,
    const std::string &search);

bool
add_value(
    json::object &obj,
    const query_type &query,
    const std::string &key,
    const json::value &value);

http::message_generator
create_response(
    http::request<http::string_body> const& req,
    json_body::value_type body);

std::string getReadableVersion(uint64_t integer);

#endif // HTTP_SERVER_HPP
