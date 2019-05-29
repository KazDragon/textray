#include "application.hpp"
#include "connection.hpp"
/*
#include "camera.hpp"
#include "client.hpp"
#include "context_impl.hpp"
#include <munin/aligned_layout.hpp>
#include <munin/container.hpp>
*/
#include <serverpp/tcp_server.hpp>
#include <boost/make_unique.hpp>
/*
#include <map>
*/
#include <utility>

namespace ma {

// ==========================================================================
// APPLICATION::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct application::impl
{
public :
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(serverpp::port_identifier port)
      : server_(port)
    {
    }

    // ======================================================================
    /// RUN
    // ======================================================================
    void run()
    {
        server_.accept(
            [this](serverpp::tcp_socket &&new_socket)
            {
                on_accept(std::move(new_socket));
            });
    }

    // ======================================================================
    // SHUTDOWN
    // ======================================================================
    void shutdown()
    {
        server_.shutdown();
    }

private :
    // ======================================================================
    // ADD_PENDING_CONNECTION
    // ======================================================================
    connection &create_pending_connection(serverpp::tcp_socket &&new_socket)
    {
        auto pending_connections_lock =
            std::unique_lock<std::mutex>(pending_connections_mutex_);

        pending_connections_.push_back(
            boost::make_unique<connection>(std::move(new_socket)));
        return *pending_connections_.back();
    }

    // ======================================================================
    // ON_ACCEPT
    // ======================================================================
    void on_accept(serverpp::tcp_socket &&new_socket)
    {
        auto &pending_connection = 
            create_pending_connection(std::move(new_socket));
    }
/*
    void on_accept(std::shared_ptr<ma::socket> const &socket)
    {
        // Create the connection and client structures for the socket.
        auto connection = std::make_shared<ma::connection>(socket);
        pending_connections_.push_back(connection);
        
        // Before creating a client object, we first negotiate some
        // knowledge about the connection.  Set up the callbacks for this.
        connection->on_socket_death(
            [this, wp=std::weak_ptr<ma::connection>(connection)] 
            {
                this->on_connection_death(wp);
            });
    
        connection->on_window_size_changed(
            [this, wp=std::weak_ptr<ma::connection>(connection)]
            (auto w, auto h) 
            {
                this->on_window_size_changed(wp, w, h);
            });

        connection->async_get_terminal_type(
            [this, 
             ws=std::weak_ptr<ma::socket>(socket),
             wc=std::weak_ptr<ma::connection>(connection)]
            (auto const &type)
            {
                this->on_terminal_type(ws, wc, type);
            });

        connection->start();
    }

    // ======================================================================
    // ON_TERMINAL_TYPE
    // ======================================================================
    void on_terminal_type(
        std::weak_ptr<ma::socket>     weak_socket
      , std::weak_ptr<ma::connection>  weak_connection
      , std::string const                   &terminal_type)
    {
        printf("Terminal type is: \"%s\"\n", terminal_type.c_str());
        
        auto socket =     weak_socket.lock();
        auto connection = weak_connection.lock();
        
        if (socket != NULL && connection != NULL)
        {
            printf("Clearing pending connection\n");
            auto pending_connection =
                std::find(
                    pending_connections_.begin()
                  , pending_connections_.end()
                  , connection);
            
            // There is a possibility that this is a stray terminal type.
            // If so, ignore it.
            if (pending_connection == pending_connections_.end())
            {
                return;
            }
            
            pending_connections_.erase(pending_connection);

            auto client =
                std::make_shared<ma::client>(std::ref(io_service_));
            client->connect(connection);
            
            client->on_connection_death(bind(
                &impl::on_client_death
              , this
              , std::weak_ptr<ma::client>(client)));

  
            context_->add_client(client);

            // If the window's size has been set by the NAWS process,
            // then update it to that.  Otherwise, use the standard 80,24.
            auto psize = pending_sizes_.find(connection);
            
            if (psize != pending_sizes_.end())
            {
                client->set_window_size(
                    psize->second.first
                  , psize->second.second);
                pending_sizes_.erase(connection);
            }
            else
            {
                client->set_window_size(80, 24);
            }
        }
    }
    
    // ======================================================================
    // ON_CONNECTION_DEATH
    // ======================================================================
    void on_connection_death(std::weak_ptr<ma::connection> const &weak_connection)
    {
        auto connection = weak_connection.lock();
    
        if (connection != NULL)
        {
            pending_connections_.erase(remove(
                    pending_connections_.begin()
                  , pending_connections_.end()
                  , connection)
              , pending_connections_.end());
            pending_sizes_.erase(connection);
        }
    }
    
    // ======================================================================
    // ON_CLIENT_DEATH
    // ======================================================================
    void on_client_death(std::weak_ptr<ma::client> &weak_client)
    {
        auto client = weak_client.lock();
        
        if (client != NULL)
        {
            context_->remove_client(client);
        }
    }

    // ======================================================================
    // ON_WINDOW_SIZE_CHANGED
    // ======================================================================
    void on_window_size_changed(
        std::weak_ptr<ma::connection> weak_connection,
        std::uint16_t                 width,
        std::uint16_t                 height)
    {
        // This is only called during the negotiation process.  We save
        // the size so that it can be given to the client once the process
        // has completed.
        auto connection = weak_connection.lock();
        
        if (connection != NULL)
        {
            pending_sizes_[connection] = std::make_pair(width, height);
        }
    }
    */

    serverpp::tcp_server server_;

    std::mutex pending_connections_mutex_;
    std::vector<std::unique_ptr<ma::connection>> pending_connections_;

    /*
    // A vector of clients whose connections are being negotiated.
    std::vector<std::shared_ptr<ma::connection>> pending_connections_;
    std::map<
        std::shared_ptr<ma::connection>, 
        std::pair<std::uint16_t, std::uint16_t>> pending_sizes_; 
    */
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
application::application(serverpp::port_identifier port)
    : pimpl_(boost::make_unique<impl>(port))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
application::~application()
{
}

// ==========================================================================
// RUN
// ==========================================================================
void application::run()
{
    pimpl_->run();
}

// ==========================================================================
// SHUTDOWN
// ==========================================================================
void application::shutdown()
{
    pimpl_->shutdown();
}

}
