#pragma once
#include "input_datastream.hpp"
#include "output_datastream.hpp"
#include "core.hpp"

namespace ma {

//* =========================================================================
/// \interface datastream
/// \brief An interface that models an input/output datastream.
//* =========================================================================
template <class ReadValue, class WriteValue>
struct datastream
  : input_datastream<ReadValue>,
    output_datastream<WriteValue>
{
    // Some helpful typedefs to allow derived classes to reduce ambiguity.
    typedef typename input_datastream<ReadValue>::value_type      input_value_type;
    typedef typename input_datastream<ReadValue>::storage_type    input_storage_type;
    typedef typename input_datastream<ReadValue>::size_type       input_size_type;
    typedef typename input_datastream<ReadValue>::callback_type   input_callback_type;

    typedef typename output_datastream<WriteValue>::value_type    output_value_type;
    typedef typename output_datastream<WriteValue>::storage_type  output_storage_type;
    typedef typename output_datastream<WriteValue>::size_type     output_size_type;
    typedef typename output_datastream<WriteValue>::callback_type output_callback_type;

    // We must re-define is_alive here to remove an ambiguity of clients as to
    // whether they are calling input_datastream<>::is_alive() or
    // output_datastream<>::is_alive(), even though they're both the same 
    // function.

    //* =====================================================================
    /// \brief Check to see if the underlying stream is still alive.
    /// \return true if the underlying stream is alive, false otherwise.
    //* =====================================================================
    virtual bool is_alive() const = 0;
};

// Some handy aliases for everyone to use.
using byte_stream = datastream<byte, byte>;
using char_stream = datastream<char, char>;

}
