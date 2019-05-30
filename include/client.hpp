#pragma once

#include <memory>
/*
#include "core.hpp"
#include <boost/asio/io_service.hpp>
#include <functional>
#include <vector>
*/

namespace ma {

class connection;

class client
{
public :
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    explicit client(
        connection &&cnx, 
        std::function<void ()> const &connection_died);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~client();

    //* =====================================================================
    /// \brief Sets the connection on which this client operates.
    //* =====================================================================
    //void connect(std::shared_ptr<connection> const &cnx);

    //* =====================================================================
    /// \brief Sets the title of the client's window
    //* =====================================================================
    //void set_window_title(std::string const &title);

    //* =====================================================================
    /// \brief Sets the size of the client's window
    //* =====================================================================
    //void set_window_size(std::uint16_t width, std::uint16_t height);

    //* =====================================================================
    /// \brief Disconnects the client from the server.
    //* =====================================================================
    //void disconnect();

    //* =====================================================================
    /// \brief Sets up a callback for if the client's connection dies.
    //* =====================================================================
    //void on_connection_death(std::function<void ()> const &callback);

private :
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

