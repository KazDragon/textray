#pragma once

#include "core.hpp"
#include <serverpp/core.hpp>
#include <functional>
#include <memory>

/*
#include <terminalpp/ansi/control_sequence.hpp>
#include <terminalpp/ansi/mouse.hpp>
#include <terminalpp/virtual_key.hpp>
#include <string>
#include <utility>
#include <vector>
*/

namespace serverpp {
class tcp_socket;
}

namespace ma {

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
    explicit connection(serverpp::tcp_socket &&socket);

    //* =====================================================================
    /// \brief Move constructor
    //* =====================================================================
    connection(connection &&other) noexcept;

    //* =====================================================================
    /// \brief Destructor.
    //* =====================================================================
    ~connection();

    //* =====================================================================
    /// \brief Move assignment
    //* =====================================================================
    connection &operator=(connection &&other) noexcept;

    //* =====================================================================
    /// \brief Asynchronously reads from the connection, calling the
    ///        supplied continuation with the results.
    ///
    /// \note If the continuation received 0 bytes, then the connection has
    ///       died.
    //* =====================================================================
    void async_read(
        std::function<void (serverpp::bytes)> const &continuation);

    //* =====================================================================
    /// \brief Writes to the connection.
    //* =====================================================================
    void write(serverpp::bytes data);

#if 0
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
#endif

private :
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}
