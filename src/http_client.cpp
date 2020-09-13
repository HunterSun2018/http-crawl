#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace asio = boost::asio;     // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
using boost::asio::awaitable;
using boost::asio::detached;
using boost::asio::use_awaitable;

awaitable<void> do_session(
    std::string const &host,
    std::string const &port,
    std::string const &target,
    int version,
    asio::io_context &ioc,
    ssl::context &ctx)
{
    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
    {
        std::cerr << "failed in SSL_set_tlsext_host_name" << std::endl;
        co_return;
    }

    // Look up the domain name
    auto const results = co_await resolver.async_resolve(host, port, use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    co_await get_lowest_layer(stream).async_connect(results, use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    co_await stream.async_handshake(ssl::stream_base::client, use_awaitable);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    co_await http::async_write(stream, req, use_awaitable);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer b;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    co_await http::async_read(stream, b, res, use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Gracefully close the stream
    beast::get_lowest_layer(stream).cancel();

    co_await stream.async_shutdown(use_awaitable);

    // Rationale:
    // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error

    co_return;
}

void run()
{
    try
    {
        boost::asio::io_context io_context(1);
        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv12_client};

        // This holds the root certificate used for verification
        load_root_certificates(ctx);

        // Verify the remote server's certificate
        ctx.set_verify_mode(ssl::verify_none);

        // boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        // signals.async_wait([&](auto, auto) { io_context.stop(); });

        // Launch the asynchronous operation
        co_spawn(
            io_context,
            [=, &io_context, &ctx]() mutable {
                return do_session(host, port, target, version, std::ref(io_context), std::ref(ctx));
            },
            detached);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }
}