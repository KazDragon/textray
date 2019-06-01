#include "application.hpp"
#include "connection.hpp"
#include "client.hpp"
#include <serverpp/tcp_server.hpp>
#include <boost/make_unique.hpp>
#include <utility>

namespace ma {

// ==========================================================================
// APPLICATION::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct application::impl
{
public :
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(serverpp::port_identifier port)
      : server_(port)
    {
    }

    // ======================================================================
    /// RUN
    // ======================================================================
    void run()
    {
        server_.accept(
            [this](serverpp::tcp_socket &&new_socket)
            {
                on_accept(std::move(new_socket));
            });
    }

    // ======================================================================
    // SHUTDOWN
    // ======================================================================
    void shutdown()
    {
        server_.shutdown();
    }

private :
    // ======================================================================
    // ON_ACCEPT
    // ======================================================================
    void on_accept(serverpp::tcp_socket &&new_socket)
    {
        auto new_client = boost::make_unique<client>(
            connection(std::move(new_socket)),
            []{});

        auto clients_lock = std::unique_lock<std::mutex>(clients_mutex_);
        clients_.push_back(std::move(new_client));
    }

    serverpp::tcp_server server_;

    std::mutex clients_mutex_;
    std::vector<std::unique_ptr<client>> clients_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
application::application(serverpp::port_identifier port)
    : pimpl_(boost::make_unique<impl>(port))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
application::~application()
{
}

// ==========================================================================
// RUN
// ==========================================================================
void application::run()
{
    pimpl_->run();
}

// ==========================================================================
// SHUTDOWN
// ==========================================================================
void application::shutdown()
{
    pimpl_->shutdown();
}

}
