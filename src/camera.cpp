#include "camera.hpp"
#include <terminalpp/palette.hpp>
#include <map>
#include <math.h>
#include <vector2d.hpp>

static terminalpp::colour darken_high_colour(terminalpp::high_colour col, int percentage)
{
    auto const red_component = terminalpp::ansi::graphics::high_red_component(col.value_);
    auto const green_component = terminalpp::ansi::graphics::high_green_component(col.value_);
    auto const blue_component = terminalpp::ansi::graphics::high_blue_component(col.value_);

    auto const darkened_red_component = terminalpp::byte((int(red_component) * (100 - percentage)) / 100);
    auto const darkened_green_component = terminalpp::byte((int(green_component) * (100 - percentage)) / 100);
    auto const darkened_blue_component = terminalpp::byte((int(blue_component) * (100 - percentage)) / 100);

    return terminalpp::high_colour(
        darkened_red_component,
        darkened_green_component,
        darkened_blue_component);
}

static terminalpp::colour darken_greyscale_colour(terminalpp::greyscale_colour col, int percentage)
{
    auto const greyscale_component = terminalpp::ansi::graphics::greyscale_component(col.shade_);
    auto const darkened_greyscale_component = terminalpp::byte((int(greyscale_component) * (100 - percentage)) / 100);

    return terminalpp::greyscale_colour{darkened_greyscale_component};
}

static terminalpp::colour darken_low_colour(terminalpp::low_colour col, int percentage)
{  
    if (col == terminalpp::graphics::colour::white)
    {
        return darken_greyscale_colour(terminalpp::greyscale_colour{23}, percentage);
    }
    else
    {
        static auto low_to_high_mapping = 
            std::map<terminalpp::low_colour, terminalpp::high_colour> {
                { terminalpp::graphics::colour::black,    terminalpp::high_colour{0, 0, 0}},
                { terminalpp::graphics::colour::red,      terminalpp::high_colour{5, 0, 0}},
                { terminalpp::graphics::colour::green,    terminalpp::high_colour{0, 5, 0}},
                { terminalpp::graphics::colour::yellow,   terminalpp::high_colour{5, 5, 0}},
                { terminalpp::graphics::colour::blue,     terminalpp::high_colour{0, 0, 5}},
                { terminalpp::graphics::colour::magenta,  terminalpp::high_colour{5, 0, 5}},
                { terminalpp::graphics::colour::cyan,     terminalpp::high_colour{0, 5, 5}},
                { terminalpp::graphics::colour::default_, terminalpp::high_colour{0, 0, 0}}
            };
        
        return darken_high_colour(low_to_high_mapping[col], percentage);
    }
}

static terminalpp::colour darken_colour(terminalpp::colour col, int percentage)
{
    switch (col.type_)
    {
        case terminalpp::colour::type::low: 
            return darken_low_colour(col.low_colour_, percentage);
            break;

        case terminalpp::colour::type::high: 
            return darken_high_colour(col.high_colour_, percentage); 
            break;

        case terminalpp::colour::type::greyscale: 
            return darken_greyscale_colour(col.greyscale_colour_, percentage); 
            break;
    }
}

static int lerp0(int high, int percentage)
{
    return (high * percentage) / 100;
}

static void render_ceiling(
    std::vector<terminalpp::string> &content,
    terminalpp::extent size)
{
    using namespace terminalpp::literals;
    static auto const ceiling_glyph = "\\U28FF"_ete.glyph_;
    auto base_colour = terminalpp::palette::grey93;
    
    auto const max_ceiling_row = size.height_ / 2;
    auto const dropoff_per_segment = 100 / (max_ceiling_row - 1);

    for (int row = 0; row < max_ceiling_row; ++row)
    {
        auto const dropoff = dropoff_per_segment * row;
        auto const col = darken_colour(base_colour, lerp0(90, dropoff));
        
        content.emplace_back(size.width_, terminalpp::element{ceiling_glyph, col});
    }
}

static void render_floor(
    std::vector<terminalpp::string> &content,
    terminalpp::extent size)
{
    using namespace terminalpp::literals;
    static auto const floor_glyph = "\\U28FF"_ete.glyph_;
    auto base_colour = terminalpp::palette::blue100;
    
    auto const min_floor_row = size.height_ / 2;
    auto const dropoff_per_segment = 100 / ((size.height_ - min_floor_row) - 1);

    for (int row = min_floor_row; row < size.height_; ++row)
    {
        auto const dropoff = dropoff_per_segment * (row - min_floor_row);
        auto const col = darken_colour(base_colour, 90 - lerp0(90, dropoff));

        content.emplace_back(size.width_, terminalpp::element{floor_glyph, col});
    }
}

static void render_walls(
    std::vector<terminalpp::string> &content,
    textray::floorplan const& plan,
    textray::vector2d const& position,
    double heading,
    double fov)
{
    static constexpr double textel_aspect = 2.0;  // textel_height / textel_width
    static constexpr double wall_height   = 1.0;  // height of walls, in world units

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
    const double fovScaleY = tanHalfFov / view_width * view_height * textel_aspect;

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
            auto lineHeight = view_height * wall_height / perpWallDist / fovScaleY / textel_aspect;
  
            // Calculate lowest and highest textel to fill in current stripe
            int drawStart = std::max( (int)round(view_height / 2.0 - lineHeight / 2), 0);
            int drawEnd   = std::min( (int)round(view_height / 2.0 + lineHeight / 2), view_height);
        
            using namespace terminalpp::literals;
            static const auto cube_glyph = "\\U28FF"_ete.glyph_;

            for (terminalpp::coordinate_type row = drawStart; row < drawEnd; ++row)
            {
                auto const colour = terminalpp::colour{terminalpp::low_colour{
                    terminalpp::graphics::colour(plan[mapY][mapX].fill.glyph_.character_)}};

                auto const darkest_distance = 5;
                auto const percentage_factor = 100 / darkest_distance;
                auto const distance = std::min(int(perpWallDist), darkest_distance);
                auto const darkness_percentage = distance * percentage_factor;

                auto const darkened_colour = darken_colour(colour, lerp0(90, darkness_percentage));
                auto const brush  = terminalpp::element{cube_glyph, darkened_colour};

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
