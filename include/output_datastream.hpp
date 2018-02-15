#pragma once

#include <functional>
#include <vector>

namespace ma {

//* =========================================================================
/// \interface output_datastream
/// \brief An interface that models an output datastream.
//* =========================================================================
template <class WriteValue>
struct output_datastream
{
    typedef WriteValue                       value_type;
    typedef std::vector<value_type>          storage_type;
    typedef typename storage_type::size_type size_type;
    typedef std::function<void (size_type)>  callback_type;

    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    virtual ~output_datastream() {}

    //* =====================================================================
    /// \brief Perform a synchronous write to the stream.
    /// \return the number of objects written to the stream.
    /// Write an array of WriteValues to the stream.
    //* =====================================================================
    virtual size_type write(std::vector<value_type> const& values) = 0;

    //* =====================================================================
    /// \brief Schedules an asynchronous write to the stream.
    ///
    /// Writes an array of WriteValues to the stream.  Returns immediately.
    /// Calls callback upon completion of the write operation, passing
    /// the amount of data written as a value.
    /// \warning async_write MAY NOT return the amount of data written
    /// synchronously, since this invalidates a set of operations.
    //* =====================================================================
    virtual void async_write(
        std::vector<value_type> const &values
      , callback_type const           &callback) = 0;

    //* =====================================================================
    /// \brief Check to see if the underlying stream is still alive.
    /// \return true if the underlying stream is alive, false otherwise.
    //* =====================================================================
    virtual bool is_alive() const = 0;
};

}
