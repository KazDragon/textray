#include "camera.hpp"
#include <math.h>
#include <vector2d.hpp>

static void render_ceiling(
    std::vector<terminalpp::string> &content,
    terminalpp::extent size)
{
    using namespace terminalpp::literals;
    static auto const ceiling_brush = "\\[7="_ets[0];
    
    auto const max_ceiling_row = size.height / 2;

    for (int row = 0; row < max_ceiling_row; ++row)
    {
        content.emplace_back(size.width, ceiling_brush);
    }
}

static void render_floor(
    std::vector<terminalpp::string> &content,
    terminalpp::extent size)
{
    using namespace terminalpp::literals;
    static auto const floor_brush = "\\[4#"_ets[0];
    
    auto const min_floor_row = size.height / 2;
    for (int row = min_floor_row; row < size.height; ++row)
    {
        content.emplace_back(size.width, floor_brush);
    }
}

static void render_walls(
    std::vector<terminalpp::string> &content,
    textray::floorplan const& plan,
    textray::vector2d const& position,
    double heading,
    double fov)
{
    static const double TEXTEL_ASPECT = 2.0;  // textel_height / textel_width
    static const double WALL_HEIGHT   = 1.0;  // height of walls, in world units

    // FoV has to be between 0 and 180 degrees (exclusive).
    assert(fov > 0.0001);
    assert(fov < M_PI - 0.0001);
    
    const auto view_height = int(content.size());
    if (view_height == 0)
    {
        return;
    }
    
    const auto view_width  = int(content[0].size());
    if (view_width == 0)
    {
        return;
    }
    
    // identify components of a unit vector in the direction of the camera
    // heading and a plane perpendicular to it on which the textels(!) are
    // rendered.
    const auto dir   = textray::vector2d::from_angle(heading);
    const auto right = textray::vector2d::from_angle(heading - M_PI/2);
    
    // Calculate the linear scale of the vertical FoV based on the viewport's aspect ratio
    // (taking the textel aspect ratio into consideration as well).
    const double tanHalfFov = tan(fov / 2);
    const double fovScaleY = tanHalfFov / view_width * view_height * TEXTEL_ASPECT;

    for (terminalpp::coordinate_type x = 0; x < view_width; ++x)
    {
        // calculate (normalized) ray direction
        double camerax = 2 * (x + 0.5) / view_width - 1; // x-coordinate in camera space (range [-1,+1])
        textray::vector2d ray = normalize(dir / tanHalfFov + right * camerax);
        
        auto mapX = int(position.x);
        auto mapY = int(position.y);
        
        //length of ray from current position to next x or y-side
        double sideDistX;
        double sideDistY;
  
        //length of ray from one x or y-side to next x or y-side
        double deltaDistX = std::abs(1 / ray.x);
        double deltaDistY = std::abs(1 / ray.y);
  
        //what direction to step in x or y-direction (either +1 or -1)
        int stepX;
        int stepY;
        
        //calculate step and initial sideDist
        if (ray.x < 0)
        {
            stepX = -1;
            sideDistX = (position.x - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - position.x) * deltaDistX;
        }
        if (ray.y < 0)
        {
            stepY = -1;
            sideDistY = (position.y - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - position.y) * deltaDistY;
        }
        
        //perform DDA (Digital Differential Analysis)
        double wallDist;
        int side;
        do
        {
            //jump to next map square, OR in x-direction, OR in y-direction
            if (sideDistX < sideDistY)
            {
                wallDist = sideDistX;
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                wallDist = sideDistY;
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
        
            //Check if ray has hit a wall
            // TODO: fix when fill is more than a character code.
        } while (plan[mapY][mapX].fill.glyph_.character_ == 0);

        // Calculate distance projected on camera direction (direct distance along ray will give fisheye effect!)
        const auto perpWallDist = dot(wallDist * ray, dir);
        if (perpWallDist > 0.001)
        {
            // Calculate height of line to draw on screen.
            // Correct for the textel aspect ratio to make sure the height is correct on the screen.
            auto lineHeight = view_height * WALL_HEIGHT / perpWallDist / fovScaleY / TEXTEL_ASPECT;
  
            // Calculate lowest and highest textel to fill in current stripe
            int drawStart = std::max( (int)round(view_height / 2.0 - lineHeight / 2), 0);
            int drawEnd   = std::min( (int)round(view_height / 2.0 + lineHeight / 2), view_height - 1);
        
            for (terminalpp::coordinate_type row = drawStart; row < drawEnd; ++row)
            {
                terminalpp::element brush('o');
                brush.attribute_.foreground_colour_ = terminalpp::ansi::graphics::colour(
                  plan[mapY][mapX].fill.glyph_.character_);

                if (side == 0)
                {
                    brush.attribute_.polarity_ = terminalpp::ansi::graphics::polarity::negative;
                }

                content[row][x] = brush;
            }
        }
    }
}

static void render_camera_image(
    terminalpp::extent size,
    munin::image& img,
    textray::floorplan const& plan,
    textray::vector2d const& position,
    double heading,
    double fov)
{
    std::vector<terminalpp::string> content;
    render_ceiling(content, size);
    render_floor(content, size);
    render_walls(content, plan, position, heading, fov);
    
    img.set_content(content);    
}

namespace textray {

camera::camera(std::shared_ptr<floorplan> plan, vector2d position, double heading, double fov)
  : image_(std::make_shared<munin::image>()),
    floorplan_(std::move(plan)),
    position_(std::move(position)),
    heading_(std::move(heading)),
    fov_(std::move(fov))
{
}

terminalpp::extent camera::do_get_preferred_size() const
{
    return image_->get_preferred_size();
}

void camera::move_to(vector2d position, double heading)
{
    position_ = std::move(position);
    heading_ = std::move(heading);
    on_redraw({
        terminalpp::rectangle({}, get_size())
    });
}

void camera::set_fov(double fov)
{
    assert(fov > 0.0001);
    assert(fov < M_PI - 0.0001);
    fov_ = std::move(fov);
    on_redraw({
        terminalpp::rectangle({}, get_size())
    });
}

void camera::do_set_size(terminalpp::extent const &size)
{
    image_->set_size(size);
    basic_component::do_set_size(size);
}

void camera::do_draw(
    munin::render_surface &surface, 
    terminalpp::rectangle const &region) const
{
    if (get_size() != terminalpp::extent(0, 0))
    {
        render_camera_image(get_size(), *image_, *floorplan_, position_, heading_, fov_);
        image_->draw(surface, region);    
    }
}

}
