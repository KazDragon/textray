#include "camera.hpp"

static constexpr int camera_height = 48;
static constexpr int camera_width  = 64;

static void render_camera_image(
    munin::image& img,
    ma::floorplan const& plan,
    ma::point position,
    double heading)
{
    terminalpp::string empty_raster_line;
    for (int column = 0; column < camera_width; ++column)
    {
        empty_raster_line += 'q';
    }
    
    std::vector<terminalpp::string> content{camera_height, empty_raster_line};
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

void camera::do_draw(
    terminalpp::canvas_view &cv, munin::rectangle const &region) const
{
    image_->draw(cv, region);    
}

}
