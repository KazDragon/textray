#pragma once

#include <boost/asio/io_service.hpp>
#include <memory>

namespace ma {
    
//* =========================================================================
/// \brief A class that implements the main engine for the server.
/// \param io_service - The engine will be run within using the dispatch
///        mechanisms of this object.
/// \brief work - A "work" object.  While this is not reset, the threads'
///        run() methods will not terminate.  Resetting this work object
///        is part of the shutdown protocol.
/// \brief port - The server will be set up on this port number.
//* =========================================================================
class application
{
public :
    application(
        boost::asio::io_service                        &io_service
      , std::shared_ptr<boost::asio::io_service::work>  work
      , unsigned int                                    port);
    
private :
    struct impl;
    std::shared_ptr<impl> pimpl_;
};

}
