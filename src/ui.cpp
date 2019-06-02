#include "ui.hpp"
#include "camera.hpp"
#include <munin/compass_layout.hpp>
#include <munin/filled_box.hpp>
#include <munin/image.hpp>
#include <munin/view.hpp>

namespace ma {
 
struct ui::impl
{
    impl(std::shared_ptr<floorplan> plan, vector2d position, double heading, double fov)
      : camera_(std::make_shared<camera>(plan, position, heading, fov))
    {
    }
    
    std::shared_ptr<camera> camera_;
};

ui::ui(std::shared_ptr<floorplan> plan, vector2d position, double heading, double fov)
  : pimpl_(new impl(plan, position, heading, fov))
{
    using namespace terminalpp::literals;
    auto const status_text = std::vector<terminalpp::string> {
        "\\<340\\>002Movement: asdw.  Rotation: qe"_ets,
        "\\<340\\>002Zoom: zx. Reset zoom: c"_ets
    };
    
    auto const fill = "\\>002 "_ets[0];

    auto const quit_text = std::vector<terminalpp::string> {
        "\\<340\\>002    Quit: Q"_ets,
        "\\<340\\>002Shutdown: P"_ets
    };
    
    auto status_bar  = std::make_shared<munin::image>(status_text, fill);
    auto status_quit = std::make_shared<munin::image>(quit_text, fill);
    auto status_fill = std::make_shared<munin::filled_box>(fill);
    auto status_line = std::make_shared<munin::container>();
    status_line->set_layout(munin::make_compass_layout());
    status_line->add_component(status_bar, munin::compass_layout::heading::west);
    status_line->add_component(status_fill, munin::compass_layout::heading::centre);
    status_line->add_component(status_quit, munin::compass_layout::heading::east);
    
    set_layout(munin::make_compass_layout());
    add_component(pimpl_->camera_, munin::compass_layout::heading::centre);
    add_component(status_line, munin::compass_layout::heading::south);
}

ui::~ui()
{
}
    
void ui::move_camera_to(vector2d const &position, double heading)
{
    pimpl_->camera_->move_to(position, heading);
}

void ui::set_camera_fov(double fov)
{
    pimpl_->camera_->set_fov(fov);
}

}