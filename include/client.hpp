#pragma once

#include <memory>

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
        std::function<void (client const&)> const &connection_died);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~client();

private :
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

