#if 0
#pragma once

#include "core.hpp"
#include <terminalpp/ansi/control_sequence.hpp>
#include <terminalpp/ansi/mouse.hpp>
#include <terminalpp/virtual_key.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ma {

class socket;

//* =========================================================================
/// \brief An connection to a socket that abstracts away details about the
/// protocols used.
//* =========================================================================
class connection
{
public :
    //* =====================================================================
    /// \brief Create a connection object that uses the passed socket as
    /// a communications point, and calls the passed function whenever data
    /// is received.
    //* =====================================================================
    connection(std::shared_ptr<ma::socket> const &socket);

    //* =====================================================================
    /// \brief Destructor.
    //* =====================================================================
    ~connection();

    //* =====================================================================
    /// \brief Starts reading from the other end of the connection.  Until
    /// this is called, no reads will take place.
    //* =====================================================================
    void start();

    //* =====================================================================
    /// \brief Writes data to the connection.
    //* =====================================================================
    void write(std::string const &data);

    //* =====================================================================
    /// \brief Set a function to be called when data arrives from the
    /// connection.
    //* =====================================================================
    void on_data_read(
        std::function<void (std::string const &)> const &callback);
    
    //* =====================================================================
    /// \brief Set a function to be called when the window size changes.
    //* =====================================================================
    void on_window_size_changed(
        std::function<void (std::uint16_t, std::uint16_t)> const &callback);

    //* =====================================================================
    /// \brief Set up a callback to be called when the underlying socket
    /// dies.
    //* =====================================================================
    void on_socket_death(std::function<void ()> const &callback);

    //* =====================================================================
    /// \brief Disconnects the socket.
    //* =====================================================================
    void disconnect();

    //* =====================================================================
    /// \brief Asynchronously retrieves the terminal type of the connection.
    //* =====================================================================
    void async_get_terminal_type(
        std::function<void (std::string const &)> const &callback);

private :
    struct impl;
    std::shared_ptr<impl> pimpl_;
};

}

#endif