#include "application.hpp"
#include "client.hpp"
#include "connection.hpp"
#include <serverpp/tcp_server.hpp>
#include <boost/make_unique.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <utility>

namespace textray {

// ==========================================================================
// APPLICATION::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct application::impl
{
 public:
  // ======================================================================
  // CONSTRUCTOR
  // ======================================================================
  impl(boost::asio::io_context &io_context, serverpp::port_identifier port)
    : server_(
        io_context,
        port,
        [this](serverpp::tcp_socket &&new_socket)
        { on_accept(std::move(new_socket)); }),
      io_context_(io_context)
  {
  }

  // ======================================================================
  // SHUTDOWN
  // ======================================================================
  void shutdown()
  {
    server_.shutdown();
    close_all_connections();
  }

 private:
  // ======================================================================
  // ON_ACCEPT
  // ======================================================================
  void on_accept(serverpp::tcp_socket &&new_socket)
  {
    auto new_client = boost::make_unique<client>(
        connection(std::move(new_socket)),
        io_context_,
        [this](client const &dead_client)
        { handle_closed_connection(dead_client); },
        [this]() { shutdown(); });

    auto clients_lock = std::unique_lock<std::mutex>(clients_mutex_);
    clients_.push_back(std::move(new_client));
  }

  // ======================================================================
  // CLOSE_ALL_CONNECTIONS
  // ======================================================================
  void close_all_connections()
  {
    auto clients_lock = std::unique_lock<std::mutex>(clients_mutex_);
    for (auto &connection : clients_)
    {
      connection->close();
    }
  }

  // ======================================================================
  // HANDLE_CLOSED_CONNECTION
  // ======================================================================
  void handle_closed_connection(client const &dead_client)
  {
    auto clients_lock = std::unique_lock<std::mutex>(clients_mutex_);
    auto dead_client_ptr = boost::find_if(
        clients_,
        [&dead_client](std::unique_ptr<client> const &current_client)
        { return &dead_client == current_client.get(); });

    if (dead_client_ptr != clients_.end())
    {
      clients_.erase(dead_client_ptr);
    }
  }

  serverpp::tcp_server server_;
  boost::asio::io_context &io_context_;

  std::mutex clients_mutex_;
  std::vector<std::unique_ptr<client>> clients_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
application::application(
    boost::asio::io_context &io_context, serverpp::port_identifier port)
  : pimpl_(boost::make_unique<impl>(io_context, port))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
application::~application() = default;

// ==========================================================================
// SHUTDOWN
// ==========================================================================
void application::shutdown()
{
  pimpl_->shutdown();
}

}  // namespace textray
