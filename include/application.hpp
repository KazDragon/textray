#pragma once

#include <serverpp/tcp_server.hpp>
#include <memory>

namespace ma {
    
//* =========================================================================
/// \brief A class that implements the main engine for the server.
/// \param port - The server will be set up on this port number.
//* =========================================================================
class application final
{
public :
    application(serverpp::port_number port);
    ~application();
    
    void run();
    void shutdown();

private :
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}
