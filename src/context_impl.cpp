// ==========================================================================
// Paradice Context Implementation
//
// Copyright (C) 2010 Matthew Chaplain, All Rights Reserved.
//
// Permission to reproduce, distribute, perform, display, and to prepare
// derivitive works from this file under the following conditions:
//
// 1. Any copy, reproduction or derivitive work of any part of this file 
//    contains this copyright notice and licence in its entirety.
//
// 2. The rights granted to you under this license automatically terminate
//    should you attempt to assert any patent claims against the licensor 
//    or contributors, which in any way restrict the ability of any party 
//    from using this software or portions thereof in any form under the
//    terms of this license.
//
// Disclaimer: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
//             KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
//             WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
//             PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
//             OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
//             OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//             OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//             SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
// ==========================================================================
#include "context_impl.hpp"
#include "client.hpp"
//#include "hugin/user_interface.hpp"
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
