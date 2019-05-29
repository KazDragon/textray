#if 0
#pragma once

#include "core.hpp"
#include <boost/asio/io_service.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace munin {
    class component;
}

namespace ma {

class connection;

class client
{
public :
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    client(
        boost::asio::io_service &io_service);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~client();

    //* =====================================================================
    /// \brief Sets the connection on which this client operates.
    //* =====================================================================
    void connect(std::shared_ptr<connection> const &cnx);

    //* =====================================================================
    /// \brief Sets the title of the client's window
    //* =====================================================================
    void set_window_title(std::string const &title);

    //* =====================================================================
    /// \brief Sets the size of the client's window
    //* =====================================================================
    void set_window_size(std::uint16_t width, std::uint16_t height);

    //* =====================================================================
    /// \brief Disconnects the client from the server.
    //* =====================================================================
    void disconnect();

    //* =====================================================================
    /// \brief Sets up a callback for if the client's connection dies.
    //* =====================================================================
    void on_connection_death(std::function<void ()> const &callback);

private :
    class impl;
    std::shared_ptr<impl> pimpl_;
};

}
#endif
