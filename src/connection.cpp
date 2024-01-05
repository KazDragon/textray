#include "connection.hpp"
#include <telnetpp/options/echo/server.hpp>
#include <telnetpp/options/mccp/codec.hpp>
#include <telnetpp/options/mccp/server.hpp>
#include <telnetpp/options/mccp/zlib/compressor.hpp>
#include <telnetpp/options/naws/client.hpp>
#include <telnetpp/options/suppress_ga/server.hpp>
#include <telnetpp/options/terminal_type/client.hpp>
#include <telnetpp/telnetpp.hpp>
#include <serverpp/tcp_socket.hpp>
#include <boost/make_unique.hpp>

namespace textray {

// ==========================================================================
// CONNECTION::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct connection::impl
{
  // ======================================================================
  // CONSTRUCTOR
  // ======================================================================
  explicit impl(serverpp::tcp_socket &&socket) : socket_(std::move(socket))
  {
    telnet_naws_client_.on_window_size_changed.connect(
        [this](auto &&width, auto &&height)
        { this->on_window_size_changed(width, height); });

    telnet_terminal_type_client_.on_terminal_type.connect(
        [this](auto &&type)
        {
          std::string user_type(type.begin(), type.end());
          this->on_terminal_type_detected(user_type);
        });

    telnet_terminal_type_client_.on_state_changed.connect(
        [this]()
        {
          if (telnet_terminal_type_client_.active())
          {
            telnet_terminal_type_client_.request_terminal_type();
          }
        });

    telnet_mccp_server_.on_state_changed.connect(
        [this]()
        {
          if (telnet_mccp_server_.active())
          {
            telnet_mccp_server_.start_compression();
          }
        });

    telnet_session_.install(telnet_echo_server_);
    telnet_session_.install(telnet_suppress_ga_server_);
    telnet_session_.install(telnet_naws_client_);
    telnet_session_.install(telnet_terminal_type_client_);
    telnet_session_.install(telnet_mccp_server_);

    // Send the required activations.
    telnet_echo_server_.activate();
    telnet_suppress_ga_server_.activate();
    telnet_naws_client_.activate();
    telnet_terminal_type_client_.activate();
    telnet_mccp_server_.activate();
  }

  // ======================================================================
  // IS_ALIVE
  // ======================================================================
  [[nodiscard]] bool is_alive() const
  {
    return socket_.is_alive();
  }

  // ======================================================================
  // CLOSE
  // ======================================================================
  void close()
  {
    socket_.close();
  }

  // ======================================================================
  // WRITE
  // ======================================================================
  void write(telnetpp::element const &data)
  {
    telnet_session_.write(data);
  }

  // ======================================================================
  // ASYNC_READ
  // ======================================================================
  void async_read(
      std::function<void(serverpp::bytes)> const &data_continuation,
      std::function<void()> const &read_complete_continuation)
  {
    telnet_session_.async_read(
        [=](telnetpp::bytes data)
        {
          if (data.empty())
          {
            read_complete_continuation();
          }
          else
          {
            data_continuation(data);
          }
        });
  }

  // ======================================================================
  // ON_WINDOW_SIZE_CHANGED
  // ======================================================================
  void on_window_size_changed(std::uint16_t width, std::uint16_t height) const
  {
    if (on_window_size_changed_)
    {
      on_window_size_changed_(width, height);
    }
  }

  // ======================================================================
  // ON_TERMINAL_TYPE_DETECTED
  // ======================================================================
  void on_terminal_type_detected(std::string const &type)
  {
    terminal_type_ = type;
    announce_terminal_type();
  }

  // ======================================================================
  // ANNOUNCE_TERMINAL_TYPE
  // ======================================================================
  void announce_terminal_type()
  {
    for (auto const &callback : terminal_type_requests_)
    {
      callback(terminal_type_);
    }

    terminal_type_requests_.clear();
  }

  serverpp::tcp_socket socket_;

  telnetpp::session telnet_session_{socket_};
  telnetpp::options::echo::server telnet_echo_server_{telnet_session_};
  telnetpp::options::suppress_ga::server telnet_suppress_ga_server_{
      telnet_session_};
  telnetpp::options::mccp::zlib::compressor telnet_mccp_compressor_;
  telnetpp::options::mccp::server telnet_mccp_server_{
      telnet_session_, telnet_mccp_compressor_};
  telnetpp::options::naws::client telnet_naws_client_{telnet_session_};
  telnetpp::options::terminal_type::client telnet_terminal_type_client_{
      telnet_session_};

  std::function<void(std::uint16_t, std::uint16_t)> on_window_size_changed_;

  std::string terminal_type_;
  std::vector<std::function<void(std::string)>> terminal_type_requests_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
connection::connection(serverpp::tcp_socket &&new_socket)
  : pimpl_(boost::make_unique<impl>(std::move(new_socket)))
{
}

// ==========================================================================
// MOVE CONSTRUCTOR
// ==========================================================================
connection::connection(connection &&other) noexcept = default;

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
connection::~connection() = default;

// ==========================================================================
// MOVE ASSIGNMENT
// ==========================================================================
connection &connection::operator=(connection &&other) noexcept = default;

// ==========================================================================
// IS_ALIVE
// ==========================================================================
bool connection::is_alive() const
{
  return pimpl_->is_alive();
}

// ==========================================================================
// CLOSE
// ==========================================================================
void connection::close()
{
  pimpl_->close();
}

// ==========================================================================
// ASYNC_READ
// ==========================================================================
void connection::async_read(
    std::function<void(serverpp::bytes)> const &data_continuation,
    std::function<void()> const &read_complete_continuation)
{
  pimpl_->async_read(data_continuation, read_complete_continuation);
}

// ==========================================================================
// WRITE
// ==========================================================================
void connection::write(serverpp::bytes data)
{
  pimpl_->write(data);
}

// ==========================================================================
// ASYNC_GET_TERMINAL_TYPE
// ==========================================================================
void connection::async_get_terminal_type(
    std::function<void(std::string const &)> const &continuation)
{
  pimpl_->terminal_type_requests_.emplace_back(continuation);
}

// ==========================================================================
// ON_WINDOW_SIZE_CHANGED
// ==========================================================================
void connection::on_window_size_changed(
    std::function<void(std::uint16_t, std::uint16_t)> const &continuation)
{
  pimpl_->on_window_size_changed_ = continuation;
}

}  // namespace textray
