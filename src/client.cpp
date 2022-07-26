#include "client.hpp"
#include "connection.hpp"
#include "camera.hpp"
#include "floorplan.hpp"
#include "vector2d.hpp"
#include "ui.hpp"

#include <terminalpp/terminal.hpp>
#include <terminalpp/canvas.hpp>
#include <munin/window.hpp>

#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <atomic>
#include <cmath>

namespace textray {

namespace {

floorplan level_map = {{
 { 1, 1, 2, 2, 3, 3, 4, 4 },
 { 3, 0, 0, 0, 0, 0, 0, 4 },
 { 3, 0, 0, 0, 5, 0, 0, 4 },
 { 4, 2, 0, 0, 0, 0, 0, 5 },
 { 4, 2, 0, 0, 0, 0, 0, 5 },
 { 5, 0, 0, 0, 0, 0, 0, 6 },
 { 5, 0, 0, 1, 0, 0, 0, 6 },
 { 7, 0, 0, 0, 0, 0, 0, 7 },
 { 7, 4, 4, 2, 2, 5, 5, 9 }
}};

// ======================================================================
// TO_RADIANS
// ======================================================================
constexpr double to_radians(double angle_degrees)
{
    return angle_degrees * M_PI / 180;
}

}

// ==========================================================================
// CLIENT IMPLEMENTATION STRUCTURE
// ==========================================================================
class client::impl
{
public :
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(
        connection &&cnx, 
        boost::asio::io_context &io_context,
        std::function<void ()> const &connection_died,
        std::function<void ()> const &shutdown)
      : connection_{std::move(cnx)},
        channel_{connection_},
        terminal_{channel_, create_behaviour()},
        strand_(io_context),
        connection_died_(connection_died),
        shutdown_(shutdown),
        canvas_({80, 24}),
        floorplan_(std::make_shared<floorplan>(level_map)),
        position_({3, 2}),
        heading_(to_radians(210)),
        fov_(90),
        ui_(std::make_shared<ui>(floorplan_, position_, heading_, to_radians(fov_))),
        window_(terminal_, ui_),
        repaint_requested_(false)
    {
        terminal_ << terminalpp::hide_cursor();

        connection_.async_get_terminal_type(
            [](std::string const &terminal_type)
            {
                std::cout << "connection from terminal: " 
                          << terminal_type << "\n";
            });

        connection_.on_window_size_changed(
            [&](std::uint16_t width, std::uint16_t height)
            {
                window_width_ = width;
                window_height_ = height;
                window_size_changed(width, height);
            });

        window_.on_repaint_request.connect(
            [this]
            {
                repaint_requested_ = true;
                strand_.post([this]{on_repaint();});
            });
        window_.on_repaint_request();

        schedule_next_read();
    }

    ~impl()
    {
        terminal_ << terminalpp::show_cursor();
    }
    
    void close()
    {
        connection_.close();
    }

private:
    // ======================================================================
    // SCHEDULE_NEXT_READ
    // ======================================================================
    void schedule_next_read()
    {
        terminal_.async_read(
            [this](terminalpp::tokens data)
            {
                for (auto const &token : data)
                {
                    std::visit(
                        [this](auto const &ev)
                        {
                            event(ev);
                        },
                        token);
                }

                if (terminal_.is_alive())
                {
                    schedule_next_read();
                }
            });
    }

    // ======================================================================
    // WINDOW_SIZE_CHANGED
    // ======================================================================
    void window_size_changed(std::uint16_t width, std::uint16_t height)
    {
        if (canvas_.size() != terminalpp::extent{width, height})
        {
            canvas_ = terminalpp::canvas({width, height});
            terminal_.set_size({width, height});
        }
            
        window_.on_repaint_request();
    }

    // ======================================================================
    // MOVE_DIRECTION
    // ======================================================================
    void move_direction(double angle)
    {
        static constexpr auto velocity = 0.25;

        auto const proposed_position = 
            position_ + vector2d::from_angle(angle) * velocity;

        bool const proposal_is_within_bounds = 
            proposed_position.x >= 0 && proposed_position.x < (*floorplan_)[0].size()
         && proposed_position.y >= 0 && proposed_position.y < (*floorplan_).size();

        bool const proposal_is_in_valid_space =
            proposal_is_within_bounds
         && (*floorplan_)[proposed_position.y][proposed_position.x].fill.glyph_.character_ == 0;

        if (proposal_is_in_valid_space)
        {
            position_ = proposed_position;
            ui_->move_camera_to(position_, heading_);
        }
    }

    // ======================================================================
    // MOVE_FORWARD
    // ======================================================================
    void move_forward()
    {
        move_direction(heading_);
    }

    // ======================================================================
    // MOVE_BACKWARD
    // ======================================================================
    void move_backward()
    {
        move_direction(heading_ + M_PI);
    }

    // ======================================================================
    // MOVE_LEFT
    // ======================================================================
    void move_left()
    {
        move_direction(heading_ + M_PI/2);
    }

    // ======================================================================
    // MOVE_RIGHT
    // ======================================================================
    void move_right()
    {
        move_direction(heading_ - M_PI/2);
    }

    // ======================================================================
    // ROTATE_LEFT
    // ======================================================================
    void rotate_left()
    {
        heading_ += to_radians(15);
        ui_->move_camera_to(position_, heading_);
    }
    
    // ======================================================================
    // ROTATE_RIGHT
    // ======================================================================
    void rotate_right()
    {
        heading_ -= to_radians(15);
        ui_->move_camera_to(position_, heading_);
    }

    // ======================================================================
    // ZOOM_IN
    // ======================================================================
    void zoom_in()
    {
        fov_ = std::max(5.0, fov_ - 5.0);
        ui_->set_camera_fov(to_radians(fov_));
    }

    // ======================================================================
    // ZOOM_OUT
    // ======================================================================
    void zoom_out()
    {
        fov_ = std::min(175.0, fov_ + 5.0);
        ui_->set_camera_fov(to_radians(fov_));
    }

    // ======================================================================
    // ZOOM_OUT
    // ======================================================================
    void reset_zoom()
    {
        fov_ = 90;
        ui_->set_camera_fov(to_radians(fov_));
    }

    // ======================================================================
    // QUIT
    // ======================================================================
    void quit()
    {
        connection_.close();
    }

    // ======================================================================
    // SHUTDOWN
    // ======================================================================
    void shutdown()
    {
        shutdown_();
    }

    // ======================================================================
    // KEYPRESS_EVENT
    // ======================================================================
    bool keypress_event(terminalpp::virtual_key const &vk)
    {
        static struct {
            terminalpp::vk key;
            void (impl::*handle)();
        } const handlers[] =
        {
            { terminalpp::vk::lowercase_q, &impl::rotate_left   },
            { terminalpp::vk::lowercase_e, &impl::rotate_right  },
            { terminalpp::vk::lowercase_w, &impl::move_forward  },
            { terminalpp::vk::lowercase_s, &impl::move_backward },
            { terminalpp::vk::lowercase_a, &impl::move_left     },
            { terminalpp::vk::lowercase_d, &impl::move_right    },
            { terminalpp::vk::lowercase_z, &impl::zoom_in       },
            { terminalpp::vk::lowercase_x, &impl::zoom_out      },
            { terminalpp::vk::lowercase_c, &impl::reset_zoom    },
            { terminalpp::vk::uppercase_q, &impl::quit          },
            { terminalpp::vk::uppercase_p, &impl::shutdown      },
        };
        
        auto handler = boost::find_if(
            handlers,
            [&vk](auto const &handler)
            {
                return vk.key == handler.key;
            });
            
        if (handler != std::end(handlers))
        {
            (this->*handler->handle)();
            return true;
        }
        else
        {
            return false;
        }
    }

    void event(boost::any const &ev)
    {
        // General key handler for movement
        bool consumed = false;
        auto *vk = boost::any_cast<terminalpp::virtual_key>(&ev);
        
        if (vk != nullptr)
        {
            consumed = keypress_event(*vk);
        }

        if (!consumed)
        {
            window_.event(ev);
        }
    }

    static terminalpp::behaviour create_behaviour()
    {
        terminalpp::behaviour behaviour;
        behaviour.supports_basic_mouse_tracking = true;
        behaviour.supports_window_title_bel = true;
        
        return behaviour;
    }

    static serverpp::bytes string_to_bytes(std::string const &str)
    {
        return serverpp::bytes(
            reinterpret_cast<serverpp::byte const*>(str.c_str()),
            str.size());
    }

    static std::string bytes_to_string(serverpp::bytes data)
    {
        return std::string(
            reinterpret_cast<char const*>(data.data()),
            data.size());
    }

    void on_repaint()
    {
        bool b = true;
        if (repaint_requested_.compare_exchange_strong(b, false))
        {
            window_.repaint(canvas_);
        }
    }

    class connection_channel
    {
    public:
        connection_channel(connection &cnx)
          : connection_(cnx)
        {
        }

        void async_read(std::function<void (serverpp::bytes)> const &callback)
        {
            connection_.async_read(
                [this](serverpp::bytes data) {
                    cache_.append(data.begin(), data.end());
                },
                [=]()
                {
                    serverpp::byte_storage callback_data;
                    std::swap(cache_, callback_data);
                    callback(callback_data);
                });
        }

        void write(serverpp::bytes data)
        {
            connection_.write(data);
        }

        bool is_alive()
        {
            return connection_.is_alive();
        }

        void close()
        {
            connection_.close();
        }

    private:
        connection &connection_;
        serverpp::byte_storage cache_;
    };

    connection connection_;
    connection_channel channel_;
    boost::asio::io_context::strand strand_;
    std::function<void ()> shutdown_;
    std::function<void ()> connection_died_;
    terminalpp::terminal terminal_;
    terminalpp::canvas canvas_;
    
    std::shared_ptr<floorplan> floorplan_;
    vector2d position_;
    double heading_;
    double fov_;

    std::uint16_t window_width_{80};
    std::uint16_t window_height_{24};

    std::shared_ptr<ui> ui_;
    munin::window window_;

    std::atomic<bool> repaint_requested_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
client::client(
    connection &&cnx,
    boost::asio::io_context &io_context,
    std::function<void (client const &)> const &connection_died,
    std::function<void ()> const &shutdown)
  : pimpl_(boost::make_unique<impl>(
        std::move(cnx), 
        io_context,
        [this, connection_died]()
        {
            connection_died(*this);
        },
        shutdown))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
client::~client()
{
}

// ==========================================================================
// CLOSE
// ==========================================================================
void client::close()
{
    pimpl_->close();
}

}

