#pragma once

#include <serverpp/core.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>

namespace textray {

//* =========================================================================
/// \brief A class that implements the main engine for the server.
/// \param port - The server will be set up on this port identifier.
//* =========================================================================
class application final  // NOLINT
{
 public:
  application(
      boost::asio::io_context &io_context, serverpp::port_identifier port);
  ~application();

  void shutdown();

 private:
  struct impl;
  std::unique_ptr<impl> pimpl_;
};

}  // namespace textray
