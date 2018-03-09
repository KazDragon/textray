#include "camera.hpp"

static std::vector<terminalpp::string> render_background()
{
    static constexpr int camera_height = 37;
    static constexpr int camera_width  = 173;

    terminalpp::string empty_raster_line;
    for (int column = 0; column < camera_width; ++column)
    {
        empty_raster_line += ' ';
    }
    
    return std::vector<terminalpp::string>{camera_height, empty_raster_line};
}

static void render_ceiling(std::vector<terminalpp::string> &content)
{
    static terminalpp::element const ceiling_brush = []()
    {
        terminalpp::element elem('=');
        elem.attribute_.foreground_colour_ = terminalpp::ansi::graphics::colour::red;
        elem.attribute_.intensity_ = terminalpp::ansi::graphics::intensity::bold;
        return elem;
    }();
    
    auto const max_ceiling_row = content.size() / 2;
    for (int row = 0; row < max_ceiling_row; ++row)
    {
        for (int column = 0; column < content[row].size(); ++column)
        {
            content[row][column] = ceiling_brush;
        }
    }
}

static void render_floor(std::vector<terminalpp::string> &content)
{
    static terminalpp::element const floor_brush = []()
    {
        terminalpp::element elem('_');
        elem.attribute_.foreground_colour_ = terminalpp::ansi::graphics::colour::yellow;
        return elem;
    }();
    
    auto const min_floor_row = content.size() / 2;
    for (int row = min_floor_row; row < content.size(); ++row)
    {
        for (int column = 0; column < content[row].size(); ++column)
        {
            content[row][column] = floor_brush;
        }
    }
}

static void render_camera_image(
    munin::image& img,
    ma::floorplan const& plan,
    ma::point position,
    double heading)
{
    auto content = render_background();    
    render_ceiling(content); 
    render_floor(content);
    
    
    
    img.set_content(content);    
}

namespace ma {

camera::camera(std::shared_ptr<floorplan> plan, point position, double heading)
  : image_(std::make_shared<munin::image>()),
    floorplan_(std::move(plan)),
    position_(std::move(position)),
    heading_(std::move(heading))
{
    render_camera_image(*image_, *floorplan_, position_, heading_);
}

terminalpp::extent camera::do_get_preferred_size() const
{
    return image_->get_preferred_size();
}

void camera::do_set_size(terminalpp::extent const &size)
{
    image_->set_size(size);
    basic_component::do_set_size(size);
}

void camera::do_draw(
    terminalpp::canvas_view &cv, munin::rectangle const &region) const
{
    image_->draw(cv, region);    
}

}
