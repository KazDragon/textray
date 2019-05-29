#if 0
#include "context_impl.hpp"
#include "client.hpp"
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace ma {

// ==========================================================================
// CONTEXT_IMPL IMPLEMENTATION STRUCTURE
// ==========================================================================
struct context_impl::impl
{
    impl(
        boost::asio::io_service                       &io_service
      , std::shared_ptr<server>                        srv
      , std::shared_ptr<boost::asio::io_service::work> work)
      : server_(srv)
      , work_(work)
    {
    }
    
    // ======================================================================
    // ADD_CLIENT
    // ======================================================================
    void add_client(std::shared_ptr<client> const &cli)
    {
        clients_.push_back(cli);
    }

    // ======================================================================
    // REMOVE_CLIENT
    // ======================================================================
    void remove_client(std::shared_ptr<client> const &cli)
    {
        clients_.erase(
            std::remove(
                clients_.begin()
              , clients_.end()
              , cli)
          , clients_.end());
    }
    
    std::shared_ptr<server>                        server_;
    std::shared_ptr<boost::asio::io_service::work> work_;
    std::vector<std::shared_ptr<client>>           clients_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
context_impl::context_impl(
    boost::asio::io_service                        &io_service
  , std::shared_ptr<server>                         srv
  , std::shared_ptr<boost::asio::io_service::work>  work)
    : pimpl_(new impl(io_service, srv, work))
{
}
    
// ==========================================================================
// DESTRUCTOR
// ==========================================================================
context_impl::~context_impl()
{
}
    
// ==========================================================================
// GET_CLIENTS
// ==========================================================================
std::vector<std::shared_ptr<client>> context_impl::get_clients()
{
    return pimpl_->clients_;
}

// ==========================================================================
// ADD_CLIENT
// ==========================================================================
void context_impl::add_client(std::shared_ptr<client> const &cli)
{
    pimpl_->add_client(cli);
}

// ==========================================================================
// REMOVE_CLIENT
// ==========================================================================
void context_impl::remove_client(std::shared_ptr<client> const &cli)
{
    pimpl_->remove_client(cli);
}

// ==========================================================================
// SHUTDOWN
// ==========================================================================
void context_impl::shutdown()
{
    pimpl_->work_.reset();
    pimpl_->server_->shutdown();
}

}
#endif
