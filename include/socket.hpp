#pragma once

#include "core.hpp"
#include "datastream.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <memory>

namespace ma {

class socket : public byte_stream
{
public :
    //* =====================================================================
    /// \brief Constructor
    //* =====================================================================
    socket(
        std::shared_ptr<boost::asio::ip::tcp::socket> const &socket);

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    virtual ~socket();

    //* =====================================================================
    /// \brief Close the socket, allowing no further transmission.
    //* =====================================================================
    void close();

    //* =====================================================================
    /// \brief Returns the number of objects that are available to be read.
    /// \return the number of objects that are available to be read without
    /// blocking, or no value if this is unknown and a call to read() might
    /// block.  The value 0 must only be returned in the case that there is
    /// no more data to be read, and the stream is dead.
    /// \warning This operation MUST NOT block.
    //* =====================================================================
    virtual boost::optional<input_size_type> available() const;

    //* =====================================================================
    /// \brief Performs a synchronous read on the stream.
    /// \return an array of values read frmo the stream.
    /// Reads up to size items from the stream and returns them in a
    /// vector.  This may block, unless a previous call to available()
    /// since the last read() yielded a positive value, which was less than or
    /// equal to size, in which case it MUST NOT block.
    //* =====================================================================
    virtual input_storage_type read(input_size_type size);

    //* =====================================================================
    /// \brief Schedules an asynchronous read on the stream.
    ///
    /// Sends a request to read size elements from the stream.  Returns
    /// immediately.  Calls callback upon completion of the read operation,
    /// passing an array of the values read as a value.
    /// \warning async_read MUST NOT return the read values synchronously,
    /// since this invalidates a set of operations.
    //* =====================================================================
    virtual void async_read(
        input_size_type            size
      , input_callback_type const &callback);

    //* =====================================================================
    /// \brief Perform a synchronous write to the stream.
    /// \return the number of objects written to the stream.
    /// Write an array of odin::u8s to the stream.
    //* =====================================================================
    virtual output_size_type write(output_storage_type const& values);

    //* =====================================================================
    /// \brief Schedules an asynchronous write to the stream.
    ///
    /// Writes an array of odin::u8s to the stream.  Returns immediately.
    /// Calls callback upon completion of the write operation, passing
    /// the amount of data written as a value.
    /// \warning async_write MAY NOT return the amount of data written
    /// synchronously, since this invalidates a set of operations.
    //* =====================================================================
    virtual void async_write(
        output_storage_type  const &values
      , output_callback_type const &callback);

    //* =====================================================================
    /// \brief Check to see if the underlying stream is still alive.
    /// \return true if the underlying stream is alive, false otherwise.
    //* =====================================================================
    virtual bool is_alive() const;

    //* =====================================================================
    /// \brief Retrieve the io_service instance that this socket uses for
    /// synchronisation.
    //* =====================================================================
    boost::asio::io_service &get_io_service();

    //* =====================================================================
    /// \brief Register a callback to be performed when the socket is closed.
    //* =====================================================================
    void on_death(std::function<void ()> const &callback);

private :
    struct impl;
    std::shared_ptr<impl> pimpl_;
};

}
