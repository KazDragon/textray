#include "client.hpp"
#include "connection.hpp"

#include "camera.hpp"
#include "floorplan.hpp"
#include "lambda_visitor.hpp"
#include "vector2d.hpp"
#include "ui.hpp"

#include <munin/window.hpp>
#include <terminalpp/ansi_terminal.hpp>
#include <terminalpp/canvas.hpp>

#include <boost/make_unique.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <cmath>

namespace ma {

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

enum class connection_state
{
    init,
    setup,
    main,
    dead
};

// ======================================================================
// STATE STRUCTURE
// ======================================================================
struct state
{
    virtual ~state() = default;
    virtual connection_state handle_data(serverpp::bytes data) = 0;
    virtual connection_state terminal_type(std::string const &type) = 0;
    virtual connection_state window_size_changed(
        std::uint16_t width, std::uint16_t height) = 0;
};

// ======================================================================
// SETUP STATE
// ======================================================================
struct setup_state final : state
{
public:
    setup_state()
    {
        printf("Entered setup state\n");
    }

    void on_discarded_data(
        std::function<void (serverpp::bytes)> const &callback)
    {
        on_discarded_data_ = callback;
    }

    connection_state handle_data(serverpp::bytes data) override
    {
        on_discarded_data_(data);
        return connection_state::setup;
    }

    connection_state terminal_type(std::string const &type) override
    {
        printf("Received terminal type: %s\n", type.c_str());
        return connection_state::main;
    }

    connection_state window_size_changed(
        std::uint16_t width, std::uint16_t height) override
    {
        printf("setup: received window size [%d,%d]\n", width, height);
        return connection_state::setup;
    }

private:
    std::function<void (serverpp::bytes)> on_discarded_data_;
};

// ======================================================================
// MAIN STATE
// ======================================================================
struct main_state final : state
{
public:
    main_state(connection &cnx)
        : connection_(cnx),
        terminal_(main_state::create_behaviour()),
        canvas_({80, 24}),
        floorplan_(std::make_shared<floorplan>(level_map)),
        position_({3, 2}),
        heading_(to_radians(210)),
        fov_(90),
        ui_(std::make_shared<ui>(floorplan_, position_, heading_, to_radians(fov_))),
        window_(ui_)
    {
        printf("Entered main state\n");
        window_.on_repaint_request.connect([this]{on_repaint();});
        on_repaint();
    }

    connection_state handle_data(serverpp::bytes data) override
    {
        boost::for_each(
            terminal_.read(bytes_to_string(data)),
            [this](auto const &token)
            {
                detail::visit_lambdas(
                    token,
                    [this](auto const &elem)
                    {
                        this->event(elem);
                    });
            });

        return (data.empty() && !connection_.is_alive())
             ? connection_state::dead 
             : connection_state::main;
    }

    connection_state terminal_type(std::string const &type) override
    {
        printf("Terminal type = %s\n", type.c_str());
        return connection_state::main;
    }

    connection_state window_size_changed(
        std::uint16_t width, std::uint16_t height) override
    {
        printf("main: received window size [%d,%d]\n", width, height);

        if (canvas_.size() != terminalpp::extent{width, height})
        {
            canvas_ = terminalpp::canvas({width, height});
        }
            
        on_repaint();

        return connection_state::main;
    }

private:
    // ======================================================================
    // MOVE_DIRECTION
    // ======================================================================
    void move_direction(double angle)
    {
        constexpr auto VELOCITY = 0.25;
        position_ += vector2d::from_angle(angle) * VELOCITY;
        ui_->move_camera_to(position_, heading_);
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
    // KEYPRESS_EVENT
    // ======================================================================
    bool keypress_event(terminalpp::virtual_key const &vk)
    {
        printf("Received keypress event\n");
        static struct {
            terminalpp::vk key;
            void (main_state::*handle)();
        } const handlers[] =
        {
            { terminalpp::vk::lowercase_q, &main_state::rotate_left   },
            { terminalpp::vk::lowercase_e, &main_state::rotate_right  },
            { terminalpp::vk::lowercase_w, &main_state::move_forward  },
            { terminalpp::vk::lowercase_s, &main_state::move_backward },
            { terminalpp::vk::lowercase_a, &main_state::move_left     },
            { terminalpp::vk::lowercase_d, &main_state::move_right    },
            { terminalpp::vk::lowercase_z, &main_state::zoom_in       },
            { terminalpp::vk::lowercase_x, &main_state::zoom_out      },
            { terminalpp::vk::lowercase_c, &main_state::reset_zoom    },
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
        printf("Received event\n");
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
        behaviour.can_use_eight_bit_control_codes = true;
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
        auto const &output_bytes = string_to_bytes(window_.repaint(canvas_, terminal_));
        printf("Repainting %d bytes\n", output_bytes.size());
        connection_.write(output_bytes);
    }

    connection &connection_;
    terminalpp::ansi_terminal terminal_;
    terminalpp::canvas canvas_;
    
    std::shared_ptr<floorplan> floorplan_;
    vector2d position_;
    double heading_;
    double fov_;

    std::shared_ptr<ui> ui_;

    munin::window window_;
};

// ======================================================================
// DEAD STATE
// ======================================================================
struct dead_state final : state
{
    dead_state()
    {
        printf("Entered dead state\n");
    }
    
    connection_state handle_data(serverpp::bytes data) override
    {
        return connection_state::dead;
    }

    connection_state terminal_type(std::string const &type) override
    {
        return connection_state::dead;
    }

    connection_state window_size_changed(
        std::uint16_t width, std::uint16_t height) override
    {
        printf("dead: received window size [%d,%d]\n", width, height);
        return connection_state::dead;
    }
};

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
    impl(connection &&cnx, std::function<void ()> const &connection_died)
      : connection_(std::move(cnx)),
        connection_died_(connection_died)
    {
        connection_.async_get_terminal_type(
            [&](std::string const &type)
            {
                enter_state(state_->terminal_type(type));
            });

        connection_.on_window_size_changed(
            [&](std::uint16_t width, std::uint16_t height)
            {
                printf("Window size change to [%d,%d]\n", width, height);
                window_width_ = width;
                window_height_ = height;
                enter_state(state_->window_size_changed(width, height));
            });

        enter_state(connection_state::setup);
    }

private :

    // ======================================================================
    // ENTER_SETUP_STATE
    // ======================================================================
    void enter_setup_state()
    {
        auto new_state = boost::make_unique<setup_state>();
        new_state->on_discarded_data(
            [this](serverpp::bytes data)
            {
                discarded_data_.insert(
                    discarded_data_.end(),
                    data.begin(),
                    data.end());
            });

        state_ = std::move(new_state);
    }

    // ======================================================================
    // ENTER_MAIN_STATE
    // ======================================================================
    void enter_main_state()
    {
        state_ = boost::make_unique<main_state>(std::ref(connection_));

        serverpp::byte_storage discarded_data;
        discarded_data_.swap(discarded_data);

        enter_state(state_->handle_data(discarded_data));
        enter_state(state_->window_size_changed(window_width_, window_height_));
    }

    // ======================================================================
    // ENTER_DEAD_STATE
    // ======================================================================
    void enter_dead_state()
    {
        state_ = boost::make_unique<dead_state>();
    }

    // ======================================================================
    // ENTER_STATE
    // ======================================================================
    void enter_state(connection_state new_state)
    {
        auto old_state = connection_state_;
        connection_state_ = new_state;

        if (new_state != old_state)
        {
            switch (new_state)
            {
                case connection_state::setup:
                    enter_setup_state();
                    break;

                case connection_state::main:
                    enter_main_state();
                    break;

                case connection_state::dead:
                    enter_dead_state();
                    return;
            }
        }

        schedule_next_read();
    }

    // ======================================================================
    // SCHEDULE_NEXT_READ
    // ======================================================================
    void schedule_next_read()
    {
        connection_.async_read(
            [this](serverpp::bytes data)
            {
                printf("Handling %d bytes\n", data.size());
                enter_state(state_->handle_data(data));
            },
            [this]()
            {
                if (connection_.is_alive())
                {
                    schedule_next_read();
                }
                else
                {
                    printf("Connection died\n");
                    enter_state(connection_state::dead);
                }
            });
    }

   connection connection_;
   std::function<void ()> connection_died_;

   connection_state connection_state_{connection_state::init};
   std::unique_ptr<state> state_;

   std::uint16_t window_width_{80};
   std::uint16_t window_height_{24};

   serverpp::byte_storage discarded_data_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
client::client(
    connection &&cnx,
    std::function<void ()> const &connection_died)
  : pimpl_(boost::make_unique<impl>(std::move(cnx), connection_died))
{
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
client::~client()
{
}

/*
// ==========================================================================
// SET_CONNECTION
// ==========================================================================
void client::connect(std::shared_ptr<connection> const &cnx)
{
    pimpl_->connect(cnx);
}

// ==========================================================================
// SET_WINDOW_TITLE
// ==========================================================================
void client::set_window_title(std::string const &title)
{
    pimpl_->set_window_title(title);
}

// ==========================================================================
// SET_WINDOW_SIZE
// ==========================================================================
void client::set_window_size(std::uint16_t width, std::uint16_t height)
{
    pimpl_->set_window_size(width, height);
}

// ==========================================================================
// DISCONNECT
// ==========================================================================
void client::disconnect()
{
    //pimpl_->get_window()->use_normal_screen_buffer();
    //pimpl_->get_window()->disable_mouse_tracking();
    pimpl_->disconnect();
}

// ==========================================================================
// ON_CONNECTION_DEATH
// ==========================================================================
void client::on_connection_death(std::function<void ()> const &callback)
{
    pimpl_->on_connection_death(callback);
}
*/

}

