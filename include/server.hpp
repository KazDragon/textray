#pragma once

#include "core.hpp"
#include <functional>
#include <memory>

namespace boost { namespace asio {
    class io_service;
}}

namespace ma {

class socket;

//* =========================================================================
/// \brief Implements a tcp/ip server.
/// \par Usage
/// Construct, passing a Boost.Asio io_service, a port number, and a function
/// to call whenever a new connection is made.  The handler for these
/// connections will be called in the io_service's run() method.  To stop the
/// server and cancel any pending acceptance, call shutdown().
//* =========================================================================
class server
{
public :
    using accept_handler = 
        std::function<void (std::shared_ptr<socket> const &)>;

    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    server(boost::asio::io_service &io_service
         , std::uint16_t            port
         , accept_handler const    &on_accept);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
   ~server();

    //* =====================================================================
    /// \brief Shuts the server down by no longer accepting incoming
    /// sockets.
    //* =====================================================================
    void shutdown();

private :
    struct impl;
    std::shared_ptr<impl> pimpl_;
};

}
