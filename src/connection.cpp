#include "connection.hpp"
#include "socket.hpp"
#include <telnetpp/telnetpp.hpp>
#include <telnetpp/options/echo/server.hpp>
#include <telnetpp/options/mccp/codec.hpp>
#include <telnetpp/options/mccp/server.hpp>
#include <telnetpp/options/mccp/zlib/compressor.hpp>
#include <telnetpp/options/naws/client.hpp>
#include <telnetpp/options/suppress_ga/server.hpp>
#include <telnetpp/options/terminal_type/client.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/placeholders.hpp>
#include <deque>
#include <string>
#include <utility>

namespace ma {

// ==========================================================================
// CONNECTION::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct connection::impl
    : public std::enable_shared_from_this<impl>
{
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(std::shared_ptr<ma::socket> const &socket)
      : socket_(socket)
    {
        telnet_naws_client_.on_window_size_changed.connect(
            [this](auto &&width, auto &&height, auto &&continuation)
            {
                this->on_window_size_changed(width, height);
            });

        telnet_terminal_type_client_.on_terminal_type.connect(
            [this](auto &&type, auto &&continuation)
            {
                std::string user_type(type.begin(), type.end());
                this->on_terminal_type_detected(user_type);
            });

        telnet_terminal_type_client_.on_state_changed.connect(
            [this](auto &&continuation)
            {
                if (telnet_terminal_type_client_.active())
                {
                    telnet_terminal_type_client_.request_terminal_type(continuation);
                }
            });

        telnet_mccp_server_.on_state_changed.connect(
            [this](auto &&continuation)
            {
                if (telnet_mccp_server_.active())
                {
                    telnet_mccp_server_.start_compression(continuation);
                }
            });

        telnet_session_.install(telnet_echo_server_);
        telnet_session_.install(telnet_suppress_ga_server_);
        telnet_session_.install(telnet_naws_client_);
        telnet_session_.install(telnet_terminal_type_client_);
        telnet_session_.install(telnet_mccp_server_);
        
        // Begin the keepalive process.  This sends regular heartbeats to the
        // client to help guard against his network settings timing him out
        // due to lack of activity.
        keepalive_timer_ =
            std::make_shared<boost::asio::deadline_timer>(
                std::ref(socket_->get_io_service()));
        schedule_keepalive();

        // Send the required activations.
        auto const &write_continuation = 
            [this](telnetpp::element const &elem)
            {
                this->write(elem);
            };

        telnet_echo_server_.activate(write_continuation);
        telnet_suppress_ga_server_.activate(write_continuation);
        telnet_naws_client_.activate(write_continuation);
        telnet_terminal_type_client_.activate(write_continuation);
        telnet_mccp_server_.activate(write_continuation);
    }

    // ======================================================================
    // START
    // ======================================================================
    void start()
    {
        schedule_next_read();
    }

    // ======================================================================
    // RAW_WRITE
    // ======================================================================
    void raw_write(telnetpp::bytes data)
    {
        telnet_mccp_compressor_(
            data,
            [this](telnetpp::bytes compressed_data, bool)
            {
                // TODO: socket/datastream needs fixing to take spans.
                std::vector<telnetpp::byte> packaged_data(
                    compressed_data.begin(), compressed_data.end());
                this->socket_->write(packaged_data);
            });
    }
    
    // ======================================================================
    // WRITE
    // ======================================================================
    void write(telnetpp::element const &data)
    {
        telnet_session_.send(
            data, 
            [this](telnetpp::bytes data)
            {
                this->raw_write(data);
            });
    }

    // ======================================================================
    // SCHEDULE_NEXT_READ
    // ======================================================================
    void schedule_next_read()
    {
        if (!socket_->is_alive())
        {
            return;
        }

        auto available = socket_->available();
        auto amount = available 
                    ? *available 
                    : ma::socket::input_size_type{1};
                    
        socket_->async_read(
            amount,
            [this, amount](auto &&data)
            {
                this->on_data(data);
            });
    }

    // ======================================================================
    // ON_DATA
    // ======================================================================
    void on_data(telnetpp::bytes data)
    {
        telnet_session_.receive(
            data, 
            [this](telnetpp::bytes data, auto &&send)
            {
                std::string app_data(data.begin(), data.end());
                on_data_read_(app_data);
            },
            [this](telnetpp::bytes data)
            {
                this->raw_write(data);
            });
            
        schedule_next_read();
    }
    
    // ======================================================================
    // ON_KEEPALIVE
    // ======================================================================
    void on_keepalive(boost::system::error_code const &error)
    {
        if (!error && socket_->is_alive())
        {
            telnet_session_.send(
                telnetpp::nop, 
                [this](telnetpp::bytes data)
                {
                    raw_write(data);
                });

            schedule_keepalive();
        }
    }

    // ======================================================================
    // SCHEDULE_KEEPALIVE
    // ======================================================================
    void schedule_keepalive()
    {
        keepalive_timer_->expires_from_now(boost::posix_time::seconds(30));
        keepalive_timer_->async_wait(
            [this](auto const &error_code)
            {
                this->on_keepalive(error_code);
            });
    }

    // ======================================================================
    // ON_WINDOW_SIZE_CHANGED
    // ======================================================================
    void on_window_size_changed(std::uint16_t width, std::uint16_t height)
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
    
    std::shared_ptr<ma::socket>                          socket_;

    std::function<void (std::string const &)>            on_data_read_;
    telnetpp::session                                    telnet_session_;
    telnetpp::options::echo::server                      telnet_echo_server_;
    telnetpp::options::suppress_ga::server               telnet_suppress_ga_server_;
    telnetpp::options::mccp::zlib::compressor            telnet_mccp_compressor_;
    telnetpp::options::mccp::server                      telnet_mccp_server_{telnet_mccp_compressor_};
    telnetpp::options::naws::client                      telnet_naws_client_;
    telnetpp::options::terminal_type::client             telnet_terminal_type_client_;
    
    std::function<void (std::uint16_t, std::uint16_t)>   on_window_size_changed_;
    std::shared_ptr<boost::asio::deadline_timer>         keepalive_timer_;

    std::string                                          terminal_type_;
    std::vector<std::function<void (std::string)>>       terminal_type_requests_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
connection::connection(std::shared_ptr<ma::socket> const &socket)
    : pimpl_(std::make_shared<impl>(socket))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
connection::~connection()
{
    disconnect();
}

// ==========================================================================
// START
// ==========================================================================
void connection::start()
{
    pimpl_->start();
}

// ==========================================================================
// WRITE
// ==========================================================================
void connection::write(std::string const &data)
{
    telnetpp::bytes telnet_data(
        reinterpret_cast<telnetpp::byte const*>(data.data()),
        data.size());

    pimpl_->write(telnet_data);
}

// ==========================================================================
// ON_DATA_READ
// ==========================================================================
void connection::on_data_read(
    std::function<void (std::string const &)> const &callback)
{
    pimpl_->on_data_read_ = callback;
}

// ==========================================================================
// ON_WINDOW_SIZE_CHANGED
// ==========================================================================
void connection::on_window_size_changed(
    std::function<void (std::uint16_t, std::uint16_t)> const &callback)
{
    pimpl_->on_window_size_changed_ = callback;
}

// ==========================================================================
// ON_SOCKET_DEATH
// ==========================================================================
void connection::on_socket_death(std::function<void ()> const &callback)
{
    pimpl_->socket_->on_death(callback);
}

// ==========================================================================
// DISCONNECT
// ==========================================================================
void connection::disconnect()
{
    if (pimpl_->keepalive_timer_ != nullptr)
    {
        boost::system::error_code unused_error_code;
        pimpl_->keepalive_timer_->cancel(unused_error_code);
    }

    pimpl_->socket_->close();
    pimpl_->socket_.reset();
}

// ==========================================================================
// ASYNC_GET_TERMINAL_TYPE
// ==========================================================================
void connection::async_get_terminal_type(
    std::function<void (std::string const &)> const &callback)
{
    pimpl_->terminal_type_requests_.push_back(callback);
}

}
