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
#include "connection.hpp"
#include "context.hpp"
#include "lambda_visitor.hpp"
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

namespace ma {

namespace {
    /*
    #define PARADICE_CMD_ENTRY_NOP(name) \
        { name,        NULL                                  , 0, 0 }

    #define PARADICE_CMD_ENTRY(func) \
        { #func,        [](auto ctx, auto args, auto player) \
                        {do_##func(ctx, args, player);}, \
                        0, 0 \
        }

    #define PARADICE_CMD_ALIAS(func, alias) \
        { alias,        [](auto ctx, auto args, auto player) \
                        {do_##func(ctx, args, player);}, \
                        0, 0 \
        }

    #define PARADICE_ADMIN_ENTRY(func, level) \
        { #func,        [](auto ctx, auto args, auto player) \
                        {do_##func(ctx, args, player);}, \
                        (level), 0 \
        }

    #define PARADICE_GM_ENTRY(func, level) \
        { #func,        [](auto ctx, auto args, auto player) \
                        {do_##func(ctx, args, player);}, \
                        0, (level) \
        }

    typedef std::function<void (
        std::shared_ptr<paradice::context> ctx,
        std::string                        args,
        std::shared_ptr<paradice::client>  player)> paradice_command;

    static struct command
    {
        std::string      command_;
        paradice_command function_;
        odin::u32        admin_level_required_;
        odin::u32        gm_level_required_;
    } const command_list[] =
    {
        PARADICE_CMD_ENTRY_NOP("!")

      , PARADICE_CMD_ENTRY(say)
      , PARADICE_CMD_ALIAS(say, ".")

      , PARADICE_CMD_ENTRY(whisper)
      , PARADICE_CMD_ALIAS(whisper, ">")

      , PARADICE_CMD_ENTRY(emote)
      , PARADICE_CMD_ALIAS(emote, ":")

      , PARADICE_CMD_ENTRY(set)
      , PARADICE_CMD_ENTRY(title)
      , PARADICE_CMD_ALIAS(title, "surname")
      , PARADICE_CMD_ENTRY(prefix)
      , PARADICE_CMD_ALIAS(prefix, "honorific")
      , PARADICE_CMD_ENTRY(roll)
      , PARADICE_CMD_ENTRY(rollprivate)
      , PARADICE_CMD_ENTRY(showrolls)
      , PARADICE_CMD_ENTRY(clearrolls)

      , PARADICE_CMD_ENTRY(help)

      , PARADICE_CMD_ENTRY(password)
      , PARADICE_CMD_ENTRY(quit)
      , PARADICE_CMD_ENTRY(logout)

      , PARADICE_GM_ENTRY(gm, 100)

      , PARADICE_ADMIN_ENTRY(admin_set_password, 100)
      , PARADICE_ADMIN_ENTRY(admin_shutdown,     100)
    };

    #undef PARADICE_CMD_ENTRY_NOP
    #undef PARADICE_CMD_ENTRY
    #undef PARADICE_CMD_ALIAS
*/
}

// ==========================================================================
// CLIENT IMPLEMENTATION STRUCTURE
// ==========================================================================
class client::impl
    : public std::enable_shared_from_this<client::impl>
{
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
        boost::asio::io_service &io_service,
        std::shared_ptr<context> ctx)
      : self_(self),
        context_(ctx),
        canvas_({80, 24}),
        terminal_(impl::create_behaviour())
    {
        auto ui = std::make_shared<munin::container>();
        ui->set_layout(std::unique_ptr<munin::layout>(new munin::grid_layout({1, 1})));
        ui->add_component(std::make_shared<munin::filled_box>('x'));
        window_ = std::make_shared<munin::window>(ui);

    }

    // ======================================================================
    // SET_CONNECTION
    // ======================================================================
    void set_connection(std::shared_ptr<connection> cnx)
    {
        connection_ = cnx;

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
                            window_->event(elem);
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
        // USER INTERFACE CALLBACKS
        user_interface_->on_input_entered.connect(
            [this](auto const &input)
            {
                this->on_input_entered(input);
            });

        user_interface_->on_login.connect(
            [this](auto const &name, auto const &pwd)
            {
                this->on_login(name, pwd);
            });

        user_interface_->on_new_account.connect(
            [this]
            {
                this->on_new_account();
            });

        user_interface_->on_account_created.connect(
            [this](auto const &name, auto const &pwd, auto const &pwd_verify)
            {
                this->on_account_created(name, pwd, pwd_verify);
            });

        user_interface_->on_account_creation_cancelled.connect(
            [this]
            {
                this->on_account_creation_cancelled();
            });

        user_interface_->on_new_character.connect(
            [this]
            {
                this->on_new_character();
            });

        user_interface_->on_character_selected.connect(
            [this](auto const &idx)
            {
                this->on_character_selected(idx);
            });

        user_interface_->on_character_created.connect(
            [this](auto const &name, auto const &is_gm)
            {
                this->on_character_created(name, is_gm);
            });

        user_interface_->on_character_creation_cancelled.connect(
            [this]
            {
                this->on_character_creation_cancelled();
            });

        user_interface_->on_gm_tools_back.connect(
            [this]
            {
                this->on_gm_tools_back();
            });

        user_interface_->on_gm_fight_beast.connect(
            [this](auto const &beast)
            {
                this->on_gm_fight_beast(beast);
            });

        user_interface_->on_gm_fight_encounter.connect(
            [this](auto const &encounter)
            {
                this->on_gm_fight_encounter(encounter);
            });

        user_interface_->on_help_closed.connect(
            [this]
            {
                this->on_help_closed();
            });

        user_interface_->on_password_changed.connect(
            [this](
                auto const &old_pwd,
                auto const &new_pwd,
                auto const &new_pwd_verify)
            {
                this->on_password_changed(old_pwd, new_pwd, new_pwd_verify);
            });

        user_interface_->on_password_change_cancelled.connect(
            [this]
            {
                this->on_password_change_cancelled();
            });

        set_window_title("Paradice9");

        auto content = window_->get_content();
        content->set_layout(munin::make_grid_layout(1, 1));
        content->add_component(user_interface_);
        content->set_focus();

        window_->enable_mouse_tracking();
        window_->use_alternate_screen_buffer();
        */
        on_repaint();
    }

    // ======================================================================
    // GET_WINDOW
    // ======================================================================
    std::shared_ptr<munin::window> get_window()
    {
        return window_;
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
    // ON_WINDOW_SIZE_CHANGED
    // ======================================================================
    void on_window_size_changed(std::uint16_t width, std::uint16_t height)
    {
        printf("Window size: %d,%d\n", width, height);
        set_window_size(width, height);
    }

    // ======================================================================
    // ON_REPAINT
    // ======================================================================
    void on_repaint()
    {
        auto data = window_->repaint(canvas_, terminal_);
        printf("writing %lu bytes of window data\n", data.size());
        connection_->write(data);
    }

    client                                 &self_;
    std::shared_ptr<context>                context_;

    std::shared_ptr<connection>             connection_;
    std::shared_ptr<munin::window>          window_;
    
    terminalpp::canvas                      canvas_;
    terminalpp::ansi_terminal               terminal_;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
client::client(
    boost::asio::io_service &io_service
  , std::shared_ptr<context> ctx)
{
    pimpl_ = std::make_shared<impl>(
        std::ref(*this), std::ref(io_service), ctx);
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
void client::set_connection(std::shared_ptr<connection> const &cnx)
{
    pimpl_->set_connection(cnx);
}

// ==========================================================================
// GET_WINDOW
// ==========================================================================
/*
std::shared_ptr<munin::window> client::get_window()
{
    return pimpl_->get_window();
}
*/

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
