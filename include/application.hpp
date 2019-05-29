#pragma once

#include <serverpp/core.hpp>
#include <memory>

namespace ma {
    
//* =========================================================================
/// \brief A class that implements the main engine for the server.
/// \param port - The server will be set up on this port identifier.
//* =========================================================================
class application final
{
public :
    application(serverpp::port_identifier port);
    ~application();
    
    void run();
    void shutdown();

private :
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}
