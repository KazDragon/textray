// ==========================================================================
// Paradice Client
//
// Copyright (C) 2009 Matthew Chaplain, All Rights Reserved.
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
#include "client.hpp"
#include "camera.hpp"
#include "connection.hpp"
#include "floorplan.hpp"
#include "lambda_visitor.hpp"
#include "vector2d.hpp"
#include <munin/compass_layout.hpp>
#include <munin/container.hpp>
#include <munin/filled_box.hpp>
#include <munin/grid_layout.hpp>
#include <munin/window.hpp>
#include <terminalpp/ansi_terminal.hpp>
#include <terminalpp/canvas.hpp>
#include <terminalpp/canvas_view.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <string>
#include <math.h>
namespace ma {

namespace {

floorplan level_map = {{
 { 1, 1, 2, 5, 3 },
 { 3, 0, 0, 0, 4 },
 { 4, 2, 0, 0, 5 },
 { 5, 0, 0, 0, 6 },
 { 7, 4, 2, 5, 9 }

}};

}

/*
 { 7, 7, 2, 5, 9 }
 { 5, 0, 0, 0, 6 },
 { 4, 2, 0, 0, 5 },
 { 3, 0, 0, 0, 4 },
 { 1, 1, 2, 5, 3 },
*/

// ==========================================================================
// CLIENT IMPLEMENTATION STRUCTURE
// ==========================================================================
class client::impl
{
    static double to_radians(double angle_degrees)
    {
        return angle_degrees * M_PI / 180;
    }

    static terminalpp::behaviour create_behaviour()
    {
        terminalpp::behaviour behaviour;
        behaviour.can_use_eight_bit_control_codes = true;
        behaviour.supports_basic_mouse_tracking = true;
        behaviour.supports_window_title_bel = true;
        
        return behaviour;
    }

public :
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(
        client                  &self,
        boost::asio::io_service &io_service)
      : self_(self),
        canvas_({80, 24}),
        terminal_(impl::create_behaviour()),
        floorplan_(std::make_shared<floorplan>(level_map)),
        position_({3, 2}),
        heading_(to_radians(210)),
        fov_(90),
        camera_(std::make_shared<camera>(floorplan_, position_, heading_, to_radians(fov_)))
    {
        using namespace terminalpp::literals;
        auto const status_text = std::vector<terminalpp::string> {
            "\\[0\\]4AMMO: 16"_ets,
            "\\[0\\]4LEVEL: 4"_ets
        };
        auto const fill = "\\[0\\]4 "_ets[0];

        auto status_bar  = std::make_shared<munin::image>(status_text, fill);
        auto status_fill = std::make_shared<munin::filled_box>(fill);
        auto status_line = std::make_shared<munin::container>();
        status_line->set_layout(munin::make_compass_layout());
        status_line->add_component(status_bar, munin::compass_layout::heading::west);
        status_line->add_component(status_fill, munin::compass_layout::heading::centre);
        
        auto ui = std::make_shared<munin::container>();
        ui->set_layout(munin::make_compass_layout());
        ui->add_component(camera_, munin::compass_layout::heading::centre);
        ui->add_component(status_line, munin::compass_layout::heading::south);
        window_ = std::make_shared<munin::window>(ui);
    }

    // ======================================================================
    // SET_CONNECTION
    // ======================================================================
    void connect(std::shared_ptr<connection> cnx)
    {
        connection_ = std::move(cnx);

        // CONNECTION CALLBACKS
        connection_->on_data_read(
            [this](std::string const &data)
            {
                for(auto &elem : terminal_.read(data))
                {
                    detail::visit_lambdas(
                        elem,
                        [this](auto const &elem)
                        {
                            event(elem);
                        });
                };
            });

        connection_->on_window_size_changed(
            [this](auto const &width, auto const &height)
            {
                this->on_window_size_changed(width, height);
            });

        // WINDOW CALLBACKS
        window_->on_repaint_request.connect(
            [this]()
            {
                this->on_repaint();
            });

        /*
        window_->enable_mouse_tracking();
        window_->use_alternate_screen_buffer();
        */
        on_repaint();
    }

    // ======================================================================
    // SET_WINDOW_TITLE
    // ======================================================================
    void set_window_title(std::string const &title)
    {
        /*
        {
            std::unique_lock<std::mutex> lock(dispatch_queue_mutex_);
            dispatch_queue_.push_back(bind(
                &munin::window::set_title, window_, title));
        }

        strand_.post(bind(&impl::dispatch_queue, shared_from_this()));
        */
    }

    // ======================================================================
    // SET_WINDOW_SIZE
    // ======================================================================
    void set_window_size(std::uint16_t width, std::uint16_t height)
    {
        if (canvas_.size() != terminalpp::extent{width, height})
        {
            canvas_ = terminalpp::canvas({width, height});
        }
        
        on_repaint();
    }

    // ======================================================================
    // DISCONNECT
    // ======================================================================
    void disconnect()
    {
        connection_->disconnect();
    }

    // ======================================================================
    // ON_CONNECTION_DEATH
    // ======================================================================
    void on_connection_death(std::function<void ()> const &callback)
    {
        connection_->on_socket_death(callback);
    }

private :
    // ======================================================================
    // MOVE_DIRECTION
    // ======================================================================
    void move_direction(double angle)
    {
        constexpr auto VELOCITY = 0.25;
        position_ += vector2d::from_angle(angle) * VELOCITY;
        camera_->move_to(position_, heading_);
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
        camera_->move_to(position_, heading_);
    }
    
    // ======================================================================
    // ROTATE_RIGHT
    // ======================================================================
    void rotate_right()
    {
        heading_ -= to_radians(15);
        camera_->move_to(position_, heading_);
    }

    // ======================================================================
    // ZOOM_IN
    // ======================================================================
    void zoom_in()
    {
        fov_ = std::max(5.0, fov_ - 5.0);
        camera_->set_fov(to_radians(fov_));
    }

    // ======================================================================
    // ZOOM_OUT
    // ======================================================================
    void zoom_out()
    {
        fov_ = std::min(175.0, fov_ + 5.0);
        camera_->set_fov(to_radians(fov_));
    }

    // ======================================================================
    // ZOOM_OUT
    // ======================================================================
    void reset_zoom()
    {
        fov_ = 90;
        camera_->set_fov(to_radians(fov_));
    }

    // ======================================================================
    // KEYPRESS_EVENT
    // ======================================================================
    bool keypress_event(terminalpp::virtual_key const &vk)
    {
        if (vk.key == terminalpp::vk::lowercase_q)
        {
            rotate_left();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_e)
        {
            rotate_right();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_w)
        {
            move_forward();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_s)
        {
            move_backward();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_a)
        {
            move_left();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_d)
        {
            move_right();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_z)
        {
            zoom_in();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_x)
        {
            zoom_out();
            return true;
        }
        else if (vk.key == terminalpp::vk::lowercase_c)
        {
            reset_zoom();
            return true;
        }
        
        return false;
    }
    
    // ======================================================================
    // EVENT
    // ======================================================================
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
            window_->event(ev);
        }
    }

    // ======================================================================
    // ON_WINDOW_SIZE_CHANGED
    // ======================================================================
    void on_window_size_changed(std::uint16_t width, std::uint16_t height)
    {
        set_window_size(width, height);
    }

    // ======================================================================
    // ON_REPAINT
    // ======================================================================
    void on_repaint()
    {
        connection_->write(window_->repaint(canvas_, terminal_));
    }

    client                                 &self_;

    std::shared_ptr<connection>             connection_;
    std::shared_ptr<munin::window>          window_;
    
    terminalpp::canvas                      canvas_;
    terminalpp::ansi_terminal               terminal_;
    
    std::shared_ptr<floorplan>              floorplan_;
    vector2d                                position_;
    double                                  heading_;
    double                                  fov_;

    std::shared_ptr<camera>                 camera_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
client::client(
    boost::asio::io_service &io_service)
{
    pimpl_ = std::make_shared<impl>(
        std::ref(*this), std::ref(io_service));
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
client::~client()
{
}

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

}
