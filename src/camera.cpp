#include "camera.hpp"
#include "overloaded.hpp"
#include <terminalpp/palette.hpp>
#include <map>
#include <math.h>
#include <vector2d.hpp>
#include <variant>

static terminalpp::attribute darken_high_colour(terminalpp::high_colour col, double percentage)
{
    auto const red_component = terminalpp::ansi::graphics::high_red_component(col.value_);
    auto const green_component = terminalpp::ansi::graphics::high_green_component(col.value_);
    auto const blue_component = terminalpp::ansi::graphics::high_blue_component(col.value_);

    auto const darkened_red_component   = (double(red_component) * (100 - percentage)) / 100;
    auto const darkened_green_component = (double(green_component) * (100 - percentage)) / 100;
    auto const darkened_blue_component  = (double(blue_component) * (100 - percentage)) / 100;

    return {terminalpp::high_colour(
        darkened_red_component,
        darkened_green_component,
        darkened_blue_component)};
}

static terminalpp::attribute darken_greyscale_colour(terminalpp::greyscale_colour col, double percentage)
{
    auto const greyscale_component = terminalpp::ansi::graphics::greyscale_component(col.shade_);
    auto const darkened_greyscale_component = terminalpp::byte((int(greyscale_component) * (100 - percentage)) / 100);

    return {terminalpp::greyscale_colour{darkened_greyscale_component}};
}

static terminalpp::attribute darken_true_colour(terminalpp::true_colour col, double percentage)
{
    auto const darkened_red_component = (double(col.red_) * (100 - percentage)) / 100;
    auto const darkened_green_component = (double(col.green_) * (100 - percentage)) / 100;
    auto const darkened_blue_component = (double(col.blue_) * (100 - percentage)) / 100;

    return {terminalpp::true_colour{
        terminalpp::byte(darkened_red_component),
        terminalpp::byte(darkened_green_component),
        terminalpp::byte(darkened_blue_component)
    }};
}

static terminalpp::attribute darken_low_colour(terminalpp::low_colour col, double percentage)
{  
    if (col == terminalpp::graphics::colour::white)
    {
        return darken_greyscale_colour(terminalpp::greyscale_colour{23}, percentage);
    }
    else
    {
        static auto low_to_high_mapping = 
            std::map<terminalpp::low_colour, terminalpp::true_colour> {
                { terminalpp::graphics::colour::black,    terminalpp::true_colour{0, 0, 0}},
                { terminalpp::graphics::colour::red,      terminalpp::true_colour{0xB8, 0x25, 0x0F}},
                { terminalpp::graphics::colour::green,    terminalpp::true_colour{0, 0xFF, 0}},
                { terminalpp::graphics::colour::yellow,   terminalpp::true_colour{0xFF, 0xFF, 0}},
                { terminalpp::graphics::colour::blue,     terminalpp::true_colour{0, 0, 0xFF}},
                { terminalpp::graphics::colour::magenta,  terminalpp::true_colour{0xFF, 0, 0xFF}},
                { terminalpp::graphics::colour::cyan,     terminalpp::true_colour{0, 0xFF, 0xFF}},
                { terminalpp::graphics::colour::default_, terminalpp::true_colour{0, 0, 0}}
            };
        
        return darken_true_colour(low_to_high_mapping[col], percentage);
    }
}

static terminalpp::attribute darken_colour(terminalpp::colour col, double percentage)
{
    return std::visit(overloaded{
        [percentage](terminalpp::low_colour const &col) {
            return darken_low_colour(col, percentage);
        },
        [percentage](terminalpp::high_colour const &col) {
            return darken_high_colour(col, percentage);
        },
        [percentage](terminalpp::greyscale_colour const &col) {
            return darken_greyscale_colour(col, percentage);
        },
        [percentage](terminalpp::true_colour const &col) {
            return darken_true_colour(col, percentage);
        }},
        col.value_);
}

static double lerp0(int high, double percentage)
{
    return (high * percentage) / 100;
}

// Top glyph with dots blanked for height (so a height of 3.1 would only have lower dots, whereas
// 3.8 would have only high dots missing
static terminalpp::glyph top_glyph(double const height)
{
    using namespace terminalpp::literals;

    double const adjusted_height = 1 - (height - int(height));
    return adjusted_height < 0.25 ? "\\U28C0"_ete.glyph_
         : adjusted_height < 0.50 ? "\\U28E4"_ete.glyph_
         : adjusted_height < 0.75 ? "\\U28F6"_ete.glyph_
                                  : "\\U28FF"_ete.glyph_;
}

// The top ceiling glyph will be adjusted so that there is always
// 3/4 of a character's height between the top of the wall and the
// ceiling.
static terminalpp::glyph top_ceiling_glyph(double const height)
{
    using namespace terminalpp::literals;

    double const adjusted_height = 1 - (height - int(height));
    return adjusted_height < 0.25 ? "\\U28FF"_ete.glyph_
         : adjusted_height < 0.50 ? "\\U283F"_ete.glyph_
         : adjusted_height < 0.75 ? "\\U281B"_ete.glyph_
                                  : "\\U2809"_ete.glyph_;
}

// Likewise bottom glyphs miss dots from the bottom up.
static terminalpp::glyph bottom_glyph(double const height)
{
    using namespace terminalpp::literals;

    double const adjusted_height = height - int(height);
    return adjusted_height < 0.25 ? "\\U2809"_ete.glyph_
         : adjusted_height < 0.50 ? "\\U281B"_ete.glyph_
         : adjusted_height < 0.75 ? "\\U283F"_ete.glyph_
                                  : "\\U28FF"_ete.glyph_;
}

// Likewise bottom floor glyphs maintain 3/4 of a character's
// distance at all times.
static terminalpp::glyph bottom_floor_glyph(double const height)
{
    using namespace terminalpp::literals;

    double const adjusted_height = height - int(height);
    return adjusted_height < 0.25 ? "\\U28FF"_ete.glyph_
         : adjusted_height < 0.50 ? "\\U28F6"_ete.glyph_
         : adjusted_height < 0.75 ? "\\U28E4"_ete.glyph_
                                  : "\\U28C0"_ete.glyph_;
}

static void render_ceiling(
    std::vector<terminalpp::string> &content,
    terminalpp::extent size)
{
    using namespace terminalpp::literals;
    static auto const ceiling_glyph = "\\U28FF"_ete.glyph_;
    auto base_colour = terminalpp::colour{terminalpp::true_colour{0xDE, 0xC9, 0xC5}};
    
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
    auto base_colour = terminalpp::true_colour{0x18, 0x2B, 0x8C};
    
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
            double drawStart = std::max(view_height / 2.0 - lineHeight / 2.0, 0.0);
            double drawEnd   = std::min(view_height / 2.0 + lineHeight / 2.0, double(view_height));
        
            using namespace terminalpp::literals;
            static auto const cube_glyph = "\\U28FF"_ete.glyph_;
            auto const high_glyph = top_glyph(drawStart);
            auto const low_glyph  = bottom_glyph(drawEnd);

            if (int(drawStart) > 0)
            {
                content[int(drawStart) - 1][x].glyph_ = top_ceiling_glyph(drawStart);
            }

            for (terminalpp::coordinate_type row = int(drawStart); row < int(drawEnd); ++row)
            {
                auto const colour = terminalpp::colour{terminalpp::low_colour{
                    terminalpp::graphics::colour(plan[mapY][mapX].fill.glyph_.character_)}};

                auto const darkest_distance = 7;
                auto const percentage_factor = 100 / darkest_distance;
                auto const distance = std::min(wallDist, double(darkest_distance));
                auto const darkness_percentage = distance * percentage_factor;

                auto const darkened_colour = darken_colour(colour, lerp0(90, darkness_percentage));
                auto const brush  = terminalpp::element{
                    (row == int(drawStart) ? high_glyph : row == (int(drawEnd) - 1) ? low_glyph : cube_glyph), 
                    darkened_colour};

                content[row][x] = brush;

                if (perpWallDist < 1.0)
                {
                    content[row][x].attribute_.intensity_ = terminalpp::graphics::intensity::bold;
                }
                else if (perpWallDist > 2.5)
                {
                    content[row][x].attribute_.intensity_ = terminalpp::graphics::intensity::faint;
                }
            }

            if (int(drawEnd) < view_height)
            {
                content[int(drawEnd)][x].glyph_ = bottom_floor_glyph(drawEnd);
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
