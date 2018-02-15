#pragma once

#include <boost/optional.hpp>
#include <functional>
#include <vector>

namespace ma {

//* =========================================================================
/// \interface input_datastream
/// \brief An interface that models an input datastream.
//* =========================================================================
template <class ReadValue>
struct input_datastream
{
    typedef ReadValue                                  value_type;
    typedef std::vector<ReadValue>                     storage_type;
    typedef typename storage_type::size_type           size_type;
    typedef std::function<void (storage_type const &)> callback_type;

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    virtual ~input_datastream() {}

    //* =====================================================================
    /// \brief Returns the number of objects that are available to be read.
    /// \return the number of objects that are available to be read without
    /// blocking, or no value if this is unknown and a call to read() might
    /// block.  Note: if a stream is serving any asynchronous read requests,
    /// it must return at most the amount already buffered by the stream,
    /// otherwise that it might block. The value 0 must only be returned
    /// in the case that there is no more data to be read, and the stream is
    /// dead.
    /// \warning This operation MUST NOT block.
    //* =====================================================================
    virtual boost::optional<size_type> available() const = 0;

    //* =====================================================================
    /// \brief Performs a synchronous read on the stream.
    /// \return an array of values read frmo the stream.
    /// Reads up to size items from the stream and returns them in a
    /// vector.  This may block, unless a previous call to available()
    /// since the last read() yielded a positive value, which was less than or
    /// equal to size, in which case it MUST NOT block.
    //* =====================================================================
    virtual storage_type read(size_type size) = 0;

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
        size_type            size
      , callback_type const &callback) = 0;

    //* =====================================================================
    /// \brief Check to see if the underlying stream is still alive.
    /// \return true if the underlying stream is alive, false otherwise.
    //* =====================================================================
    virtual bool is_alive() const = 0;
};

}
