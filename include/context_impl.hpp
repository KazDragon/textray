#if 0
#pragma once

#include "context.hpp"

namespace ma {

class client;

//* =========================================================================
/// \brief Describes the context in which the server runs.
//* =========================================================================
class context_impl : public context
{
public :
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    context_impl(
        boost::asio::io_service                       &io_service
      , std::shared_ptr<server>                        srv
      , std::shared_ptr<boost::asio::io_service::work> work);
    
    //* =====================================================================
    /// \brief Denstructor
    //* =====================================================================
    virtual ~context_impl();
    
    //* =====================================================================
    /// \brief Retrieves a list of clients currently connected to the server.
    //* =====================================================================
    virtual std::vector<std::shared_ptr<client>> get_clients();

    //* =====================================================================
    /// \brief Adds a client to the list of clients currently connected
    /// to the server.
    //* =====================================================================
    virtual void add_client(std::shared_ptr<client> const &cli);

    //* =====================================================================
    /// \brief Removes a client from the list of clients currently
    /// connected to the server.
    //* =====================================================================
    virtual void remove_client(std::shared_ptr<client> const &cli);
    
    //* =====================================================================
    /// \brief Enacts a server shutdown.
    //* =====================================================================
    virtual void shutdown();
    
private :
    struct impl;
    std::shared_ptr<impl> pimpl_;
};

}
#endif