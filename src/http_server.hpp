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
#include <iomanip>
#include <fstream>
#include <ctime>
#include <memory>
#include <string>
#include <sstream>
#include <charconv>
#include <future>
#include <thread>
#include <vector>
#include <map>
#include <any>

#include "http/json_body.hpp"
#include "../igcl/igcl_api.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef std::map<std::string, std::any> query_type;

typedef struct _request_elements_t
{
    std::string target;
    query_type query;
    json_body::value_type body;
} request_elements_t;

// ctl_result_t values range is too wide for magic_enum, so it's hardcoded
std::string enum_name(
    ctl_result_t value) noexcept;

beast::string_view mime_type(beast::string_view path);

std::string path_cat(beast::string_view base, beast::string_view path);

void
fail(beast::error_code ec, char const* what);

void
fail(const std::string &message, char const* what);

void
info(const std::string &message, char const* what);

int http_run(void);

http::response<json_body>
status_response(
    const std::string &details,
    http::request<http::string_body> const& req,
    http::status status,
    const std::string &statusStr = "");

// Returns a bad request response
http::response<json_body>
bad_request(
    const std::string &why,
    http::request<http::string_body> const& req);

// Returns a not found response
http::response<json_body>
not_found(
    const std::string &target,
    http::request<http::string_body> const& req);

// Returns a server error response
http::response<json_body>
server_error(
    const std::string &what,
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

extern std::atomic<bool> runServer;
extern std::atomic<unsigned short> serverPort;
extern std::atomic<bool> allowEdit;
extern std::shared_ptr<std::basic_ofstream<char, std::char_traits<char>>> appLog;

std::string getReadableVersion(uint64_t integer);

#endif // HTTP_SERVER_HPP
