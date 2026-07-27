//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include "http_server.hpp"
#include "boost/json/value.hpp"
#include "device/device.hpp"
#include <any>
#include <string>
#include <vector>

ctl_api_handle_t hAPIHandle;

// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
    beast::string_view base,
    beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

http::response<json_body>
bad_request(
    beast::string_view why,
    http::request<http::string_body> const& req)
{
    http::response<json_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(".json"));
    res.keep_alive(req.keep_alive());
    res.body() = {{"message", "400: Bad request"}, {"details", std::string(why)}};
    res.prepare_payload();
    return res;
}

http::response<json_body>
not_found(
    beast::string_view target,
    http::request<http::string_body> const& req)
{
    http::response<json_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(".json"));
    res.keep_alive(req.keep_alive());
    res.body() = {{"message", "404: Not found"}, {"details", "The resource '" + std::string(target) + "' was not found."}};
    res.prepare_payload();
    return res;
};

http::response<json_body>
server_error(
    beast::string_view what,
    http::request<http::string_body> const& req)
{
    http::response<json_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(".json"));
    res.keep_alive(req.keep_alive());
    res.body() = {{"message", "500: Server error"}, {"details", "An error occurred: '" + std::string(what) + "'"}};
    res.prepare_payload();
    return res;
};

std::string
str_extract(
    std::string *origin,
    const char sep)
{
    std::string res;

    size_t pos = origin->find_first_of(sep);

    if (pos != std::string::npos)
    {
        res = origin->substr(0, pos);
        *origin = origin->substr(pos + 1);
    } else
    {
        res = origin->substr();
        *origin = "";
    }

    return res;
}

bool
str_starts_with_erase(
    std::string *origin,
    const std::string &search)
{
    if (origin->rfind(search, 0) == 0)
    {
        *origin = origin->substr(search.length());
        return true;
    } else {
        return false;
    }
}

bool
add_value(
    json::object &obj,
    const query_type &query,
    const std::string &key,
    const json::value &value)
{
    auto fieldsIt = query.find("fields");
    if (fieldsIt != query.end()) {
        std::vector<std::string> fields = std::any_cast<std::vector<std::string>>(fieldsIt->second);

        if (std::find(fields.begin(), fields.end(), key) != fields.end())
        {
            obj[key] = value;
            return true;
        }
        else {
            return false;
        }
    }
    else {
        obj[key] = value;
        return true;
    }
}

std::vector<std::string>
split_fields(
    std::string &fields)
{
    std::vector<std::string> res;

    while (!fields.empty())
        res.push_back(str_extract(&fields, ','));

    return res;
}

query_type
split_query(
    const std::string &query)
{
    query_type results;

    // Split into key value pairs separated by '&'.
    size_t prev_amp_index = 0;
    while(prev_amp_index != std::string::npos)
    {
        size_t amp_index = query.find_first_of('&', prev_amp_index);
        if (amp_index == std::string::npos)
            amp_index = query.find_first_of(';', prev_amp_index);

        std::string key_value_pair = query.substr(
            prev_amp_index,
            amp_index == std::string::npos ? query.size() - prev_amp_index : amp_index - prev_amp_index);
        prev_amp_index = amp_index == std::string::npos ? std::string::npos : amp_index + 1;

        size_t equals_index = key_value_pair.find_first_of('=');
        if(equals_index == std::string::npos)
        {
            continue;
        }
        else if (equals_index == 0)
        {
            std::string value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
            results[""] = value;
        }
        else
        {
            std::string key(key_value_pair.begin(), key_value_pair.begin() + equals_index);
            std::string value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
            if (key == "fields")
                results[key] = split_fields(value);
            else
                results[key] = value;
        }
    }

    return results;
}

http::message_generator
create_response(
    http::request<http::string_body> const& req,
    json_body::value_type body)
{
    // Respond to HEAD request
    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(".json"));
        res.content_length(json_body::size(body));
        res.keep_alive(req.keep_alive());
        return res;
    } else {
        http::response<json_body> res{
            std::piecewise_construct,
            std::make_tuple(body),
            std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(".json"));
        res.prepare_payload();
        res.keep_alive(req.keep_alive());
        return res;
    }
}

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
http::message_generator
handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return bad_request("Illegal request-target", req);

    std::string temp = req.target();
    request_elements_t reqElements;
    reqElements.target = str_extract(&temp, '?');
    reqElements.query = split_query(temp);

    if(reqElements.target.back() != '/')
        reqElements.target += '/';

    if (str_starts_with_erase(&reqElements.target, "/device/"))
    {
        return handle_device(std::move(req), reqElements, hAPIHandle);
    } else
    {
        return not_found(req.target(), req);
    }
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> req_;

public:
    // Take ownership of the stream
    session(
        tcp::socket&& socket,
        std::shared_ptr<std::string const> const& doc_root)
        : stream_(std::move(socket))
        , doc_root_(doc_root)
    {
    }

    // Start the asynchronous operation
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                          &session::do_read,
                          shared_from_this()));
    }

    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec == beast::error::timeout)
        {
            json_body::value_type timeout_body;
            timeout_body = {
                {"error", "request_timeout"},
                {"message", "The request timed out while reading."}
            };

            http::response<json_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(timeout_body)),
                std::make_tuple(http::status::request_timeout, 11)};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(".json"));
            res.keep_alive(false);
            res.prepare_payload();
            return send_response(std::move(res));
        }

        if(ec)
            return fail(ec, "read");

        // Send the response
        send_response(
            handle_request(*doc_root_, std::move(req_)));
    }

    void
    send_response(http::message_generator&& msg)
    {
        bool keep_alive = msg.keep_alive();

        // Write the response
        beast::async_write(
            stream_,
            std::move(msg),
            beast::bind_front_handler(
                &session::on_write, shared_from_this(), keep_alive));
    }

    void
    on_write(
        bool keep_alive,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(! keep_alive)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Read another request
        do_read();
    }

    void
    do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const& doc_root)
        : ioc_(ioc)
        , acceptor_(net::make_strand(ioc))
        , doc_root_(doc_root)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
            return; // To avoid infinite loop
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(
                std::move(socket),
                doc_root_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int http_run(void)
{
    // Check command line arguments.
    //if (argc != 5)
    //{
    //    std::cerr <<
    //        "Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
    //        "Example:\n" <<
    //        "    http-server-async 0.0.0.0 8080 . 1\n";
    //    return EXIT_FAILURE;
    //}
    //auto const address = net::ip::make_address(argv[1]);
    //auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    //auto const doc_root = std::make_shared<std::string>(argv[3]);
    //auto const threads = std::max<int>(1, std::atoi(argv[4]));

    ctl_init_args_t CtlInitArgs;

    CtlInitArgs.AppVersion = CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION);
    CtlInitArgs.flags = CTL_INIT_FLAG_USE_LEVEL_ZERO;
    CtlInitArgs.Size = sizeof(CtlInitArgs);
    CtlInitArgs.Version = 0;
    ZeroMemory(&CtlInitArgs.ApplicationUID, sizeof(ctl_application_id_t));

    ctl_result_t res = ctlInit(&CtlInitArgs, &hAPIHandle);
    if (res != CTL_RESULT_SUCCESS) {
        std::cout << "Can't initialize the API: " << magic_enum::enum_name(res) << std::endl;
        return EXIT_FAILURE;
    } else {

        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(9738);
        auto const doc_root = std::make_shared<std::string>(".");
        auto const threads = std::max<int>(1, 1);

        // The io_context is required for all I/O
        net::io_context ioc{threads};

        // Create and launch a listening port
        std::make_shared<listener>(
            ioc,
            tcp::endpoint{address, port},
            doc_root)->run();

        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for(auto i = threads - 1; i > 0; --i)
            v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
        ioc.run();

        return EXIT_SUCCESS;
    }
}
