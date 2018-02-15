#include "server.hpp"
#include "socket.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>

namespace ma {

// ==========================================================================
// SERVER::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct server::impl
    : public std::enable_shared_from_this<server::impl>
{
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(boost::asio::io_service             &io_service,
         std::uint16_t                        port,
         server::accept_handler const        &on_accept)
      : io_service_(io_service),
        acceptor_(
            io_service,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        on_accept_(on_accept)
    {
    }

    // ======================================================================
    // HANDLE_ACCEPT
    // ======================================================================
    void handle_accept(
        std::shared_ptr<boost::asio::ip::tcp::socket> const &new_socket,
        boost::system::error_code const &error)
    {
        if (!error)
        {
// Really need to get a logging library.
#if DEBUG_SERVER
            printf("Connection from: %s\n",
                new_socket->remote_endpoint().address().to_string().c_str());
#endif

            auto sck = std::make_shared<ma::socket>(new_socket);

            on_accept_(sck);

            schedule_accept();
        }
    }

    // ======================================================================
    // SCHEDULE_ACCEPT
    // ======================================================================
    void schedule_accept()
    {
        auto new_socket =
            std::make_shared<boost::asio::ip::tcp::socket>(io_service_);

        acceptor_.async_accept(
            *new_socket.get(),
            [this, new_socket](
                boost::system::error_code const &ec)
            {
                handle_accept(new_socket, ec);
            });
    }

    // ======================================================================
    // CANCEL
    // ======================================================================
    void cancel()
    {
        boost::system::error_code unused_error_code;
        acceptor_.close(unused_error_code);
    }

    boost::asio::io_service        &io_service_;
    boost::asio::ip::tcp::acceptor  acceptor_;
    server::accept_handler          on_accept_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
server::server(
    boost::asio::io_service     &io_service
  , uint16_t                     port
  , accept_handler const        &on_accept)
    : pimpl_(new impl(io_service, port, on_accept))
{
    pimpl_->schedule_accept();
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
server::~server()
{
    // Ensure that the server is stopped.  This allows pimpl_ to be destroyed
    // (eventually) in all cases.
    pimpl_->cancel();
}

// ==========================================================================
// SHUTDOWN
// ==========================================================================
void server::shutdown()
{
    pimpl_->cancel();
}

}
