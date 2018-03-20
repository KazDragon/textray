#pragma once

#include <memory>
#include <vector>

namespace ma {

class client;

//* =========================================================================
/// \brief Describes the interface for a context in which a server can run.
//* =========================================================================
class context
{
public :
    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    virtual ~context() {}

    //* =====================================================================
    /// \brief Retrieves a list of clients currently connected to the server.
    //* =====================================================================
    virtual std::vector<std::shared_ptr<client>> get_clients() = 0;

    //* =====================================================================
    /// \brief Adds a client to the list of clients currently connected
    /// to the server.
    //* =====================================================================
    virtual void add_client(std::shared_ptr<client> const &cli) = 0;

    //* =====================================================================
    /// \brief Removes a client from the list of clients currently
    /// connected to the server.
    //* =====================================================================
    virtual void remove_client(std::shared_ptr<client> const &cli) = 0;

    //* =====================================================================
    /// \brief Enacts a server shutdown.
    //* =====================================================================
    virtual void shutdown() = 0;
};

}
