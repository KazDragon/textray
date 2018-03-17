#include "camera.hpp"
#include <math.h>

static std::vector<terminalpp::string> render_background(terminalpp::extent size)
{
    terminalpp::string empty_raster_line;
    for (int column = 0; column < size.width; ++column)
    {
        empty_raster_line += ' ';
    }
    
    return std::vector<terminalpp::string>(size.height, empty_raster_line);
}

static void render_ceiling(std::vector<terminalpp::string> &content)
{
    static terminalpp::element const ceiling_brush = []()
    {
        terminalpp::element elem('=');
        elem.attribute_.foreground_colour_ = terminalpp::ansi::graphics::colour::white;
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

static void render_walls(
    std::vector<terminalpp::string> &content,
    ma::floorplan const& plan,
    ma::point position,
    double heading)
{
    const auto view_height = int(content.size());
    const auto view_width  = int(content[0].size());
    
    // identify components of a unit vector in the direction of the camera
    // heading and a plane perpendicular to it on which the textels(!) are
    // rendered.
    double dirx = cos(heading);
    double diry = sin(heading);
    double planex = cos(heading - M_PI/2);
    double planey = sin(heading - M_PI/2);
    
    for (terminalpp::coordinate_type x = 0; x < view_width; ++x)
    {
        //calculate ray position and direction
        double camerax = 2 * x / double(view_width) - 1; //x-coordinate in camera space

        double rayDirX = dirx / 2 + planex * camerax;
        double rayDirY = diry / 2 + planey * camerax;

        auto mapX = int(position.x);
        auto mapY = int(position.y);
        double posX = position.x;
        double posY = position.y;
        
        //length of ray from current position to next x or y-side
        double sideDistX;
        double sideDistY;
  
        //length of ray from one x or y-side to next x or y-side
        double deltaDistX = std::abs(1 / rayDirX);
        double deltaDistY = std::abs(1 / rayDirY);
  
        //what direction to step in x or y-direction (either +1 or -1)
        int stepX;
        int stepY;
  
        int hit = 0; //was there a wall hit?
        int side; //was a NS or a EW wall hit?
        
        //calculate step and initial sideDist
        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }
        
        //perform DDA (Digital Differential Analysis)
        while (hit == 0)
        {
            //jump to next map square, OR in x-direction, OR in y-direction
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
        
            //Check if ray has hit a wall
            // TODO: fix when fill is more than a character code.
            if (plan[mapY][mapX].fill.glyph_.character_ > 0) 
                hit = 1;
        } 

        //Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
        double perpWallDist;
        if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
        else           perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;

        //Calculate height of line to draw on screen
        int lineHeight = (int)(view_height / perpWallDist);
  
        //calculate lowest and highest pixel to fill in current stripe
        int drawStart = -lineHeight / 2 + view_height / 2;
        if(drawStart < 0)drawStart = 0;
        int drawEnd = lineHeight / 2 + view_height / 2;
        if(drawEnd >= view_height)drawEnd = view_height - 1;
        
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

static void render_camera_image(
    terminalpp::extent size,
    munin::image& img,
    ma::floorplan const& plan,
    ma::point position,
    double heading)
{
    auto content = render_background(size);
    render_ceiling(content); 
    render_floor(content);
    render_walls(content, plan, position, heading);
    
    
    img.set_content(content);    
}

namespace ma {

camera::camera(std::shared_ptr<floorplan> plan, point position, double heading)
  : image_(std::make_shared<munin::image>()),
    floorplan_(std::move(plan)),
    position_(std::move(position)),
    heading_(std::move(heading))
{
}

terminalpp::extent camera::do_get_preferred_size() const
{
    return image_->get_preferred_size();
}

void camera::move_to(point position, double heading)
{
    position_ = position;
    heading_ = heading;
    on_redraw({
        munin::rectangle({}, get_size())
    });
}

void camera::do_set_size(terminalpp::extent const &size)
{
    image_->set_size(size);
    basic_component::do_set_size(size);
}

void camera::do_draw(
    terminalpp::canvas_view &cv, munin::rectangle const &region) const
{
    if (get_size() != terminalpp::extent(0, 0))
    {
        render_camera_image(get_size(), *image_, *floorplan_, position_, heading_);
        image_->draw(cv, region);    
    }
}

}
