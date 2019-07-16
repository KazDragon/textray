#pragma once

#include <boost/asio/io_context.hpp>
#include <memory>

namespace textray {

class connection;

class client
{
public :
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    explicit client(
        connection &&cnx, 
        boost::asio::io_context &io_context,
        std::function<void (client const&)> const &connection_died,
        std::function<void ()> const &shutdown);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~client();

    //* =====================================================================
    /// \brief Closes the connection.
    //* =====================================================================
    void close();

private :
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

