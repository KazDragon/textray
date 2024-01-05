#include "camera.hpp"
#include "overloaded.hpp"
#include <terminalpp/palette.hpp>
#include <vector2d.hpp>
#include <cmath>
#include <map>
#include <variant>

namespace {
terminalpp::attribute darken_high_colour(
    terminalpp::high_colour col, double percentage)
{
  auto const red_component =
      terminalpp::ansi::graphics::high_red_component(col.value_);
  auto const green_component =
      terminalpp::ansi::graphics::high_green_component(col.value_);
  auto const blue_component =
      terminalpp::ansi::graphics::high_blue_component(col.value_);

  auto const darkened_red_component =
      (static_cast<double>(red_component) * (100 - percentage)) / 100;
  auto const darkened_green_component =
      (static_cast<double>(green_component) * (100 - percentage)) / 100;
  auto const darkened_blue_component =
      (static_cast<double>(blue_component) * (100 - percentage)) / 100;

  return {terminalpp::high_colour(
      static_cast<terminalpp::byte>(darkened_red_component),
      static_cast<terminalpp::byte>(darkened_green_component),
      static_cast<terminalpp::byte>(darkened_blue_component))};
}

terminalpp::attribute darken_greyscale_colour(
    terminalpp::greyscale_colour col, double percentage)
{
  auto const greyscale_component =
      terminalpp::ansi::graphics::greyscale_component(col.shade_);
  auto const darkened_greyscale_component = static_cast<terminalpp::byte>(
      (static_cast<int>(greyscale_component) * (100 - percentage)) / 100);

  return {terminalpp::greyscale_colour{darkened_greyscale_component}};
}

terminalpp::attribute darken_true_colour(
    terminalpp::true_colour col, double percentage)
{
  auto const reduction = (256 * percentage) / 100;
  auto const darkened_red_component =
      std::max(static_cast<double>(col.red_) - reduction, double{0});
  auto const darkened_green_component =
      std::max(static_cast<double>(col.green_) - reduction, double{0});
  auto const darkened_blue_component =
      std::max(static_cast<double>(col.blue_) - reduction, double{0});

  return {terminalpp::true_colour{
      static_cast<terminalpp::byte>(darkened_red_component),
      static_cast<terminalpp::byte>(darkened_green_component),
      static_cast<terminalpp::byte>(darkened_blue_component)}};
}

terminalpp::attribute darken_low_colour(
    terminalpp::low_colour col, double percentage)
{
  if (col == terminalpp::graphics::colour::white)
  {
    return darken_greyscale_colour(
        terminalpp::greyscale_colour{23}, percentage);
  }
  else
  {
    static auto low_to_high_mapping =
        std::map<terminalpp::low_colour, terminalpp::true_colour>{
            {terminalpp::graphics::colour::black,
             terminalpp::true_colour{0, 0, 0}},
            {terminalpp::graphics::colour::red,
             terminalpp::true_colour{0xB8, 0x25, 0x0F}},
            {terminalpp::graphics::colour::green,
             terminalpp::true_colour{0, 0xFF, 0}},
            {terminalpp::graphics::colour::yellow,
             terminalpp::true_colour{0xFF, 0xFF, 0}},
            {terminalpp::graphics::colour::blue,
             terminalpp::true_colour{0, 0, 0xFF}},
            {terminalpp::graphics::colour::magenta,
             terminalpp::true_colour{0xFF, 0, 0xFF}},
            {terminalpp::graphics::colour::cyan,
             terminalpp::true_colour{0, 0xFF, 0xFF}},
            {terminalpp::graphics::colour::default_,
             terminalpp::true_colour{0, 0, 0}}};

    return darken_true_colour(low_to_high_mapping[col], percentage);
  }
}

terminalpp::attribute darken_colour(terminalpp::colour col, double percentage)
{
  return std::visit(
      overloaded{
          [percentage](terminalpp::low_colour const &col)
          { return darken_low_colour(col, percentage); },
          [percentage](terminalpp::high_colour const &col)
          { return darken_high_colour(col, percentage); },
          [percentage](terminalpp::greyscale_colour const &col)
          { return darken_greyscale_colour(col, percentage); },
          [percentage](terminalpp::true_colour const &col)
          { return darken_true_colour(col, percentage); }},
      col.value_);
}

double lerp0(int high, double percentage)
{
  return (high * percentage) / 100;
}

// Top glyph with dots blanked for height (so a height of 3.1 would only have
// lower dots, whereas 3.8 would have only high dots missing
terminalpp::glyph top_glyph(double const height)
{
  using namespace terminalpp::literals;  // NOLINT

  double const adjusted_height = 1 - (height - static_cast<int>(height));
  return adjusted_height < 0.25   ? R"(\U28C0)"_ete.glyph_
         : adjusted_height < 0.50 ? R"(\U28E4)"_ete.glyph_
         : adjusted_height < 0.75 ? R"(\U28F6)"_ete.glyph_
                                  : R"(\U28FF)"_ete.glyph_;
}

// The top ceiling glyph will be adjusted so that there is always
// 3/4 of a character's height between the top of the wall and the
// ceiling.
terminalpp::glyph top_ceiling_glyph(double const height)
{
  using namespace terminalpp::literals;  // NOLINT

  double const adjusted_height = 1 - (height - static_cast<int>(height));
  return adjusted_height < 0.25   ? R"(\U28FF)"_ete.glyph_
         : adjusted_height < 0.50 ? R"(\U283F)"_ete.glyph_
         : adjusted_height < 0.75 ? R"(\U281B)"_ete.glyph_
                                  : R"(\U2809)"_ete.glyph_;
}

// Likewise bottom glyphs miss dots from the bottom up.
terminalpp::glyph bottom_glyph(double const height)
{
  using namespace terminalpp::literals;  // NOLINT

  double const adjusted_height = height - static_cast<int>(height);
  return adjusted_height < 0.25   ? R"(\U2809)"_ete.glyph_
         : adjusted_height < 0.50 ? R"(\U281B)"_ete.glyph_
         : adjusted_height < 0.75 ? R"(\U283F)"_ete.glyph_
                                  : R"(\U28FF)"_ete.glyph_;
}

// Likewise bottom floor glyphs maintain 3/4 of a character's
// distance at all times.
terminalpp::glyph bottom_floor_glyph(double const height)
{
  using namespace terminalpp::literals;  // NOLINT

  double const adjusted_height = height - static_cast<int>(height);
  return adjusted_height < 0.25   ? R"(\U28FF)"_ete.glyph_
         : adjusted_height < 0.50 ? R"(\U28F6)"_ete.glyph_
         : adjusted_height < 0.75 ? R"(\U28E4)"_ete.glyph_
                                  : R"(\U28C0)"_ete.glyph_;
}

void render_ceiling(
    std::vector<terminalpp::string> &content, terminalpp::extent size)
{
  using namespace terminalpp::literals;  // NOLINT
  static auto const ceiling_glyph = R"(\U28FF)"_ete.glyph_;
  auto base_colour =
      terminalpp::colour{terminalpp::true_colour{0xDE, 0xC9, 0xC5}};

  auto const max_ceiling_row = size.height_ / 2;
  auto const dropoff_per_segment = 100 / (max_ceiling_row - 1);

  for (int row = 0; row < max_ceiling_row; ++row)
  {
    auto const dropoff = dropoff_per_segment * row;
    auto const col = darken_colour(base_colour, lerp0(90, dropoff));

    content.emplace_back(size.width_, terminalpp::element{ceiling_glyph, col});
  }
}

void render_floor(
    std::vector<terminalpp::string> &content, terminalpp::extent size)
{
  using namespace terminalpp::literals;  // NOLINT
  static auto const floor_glyph = R"(\U28FF)"_ete.glyph_;
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

void render_walls(
    std::vector<terminalpp::string> &content,
    textray::floorplan const &plan,
    textray::vector2d const &position,
    double heading,
    double fov)
{
  static constexpr double textel_aspect = 2.0;  // textel_height / textel_width
  static constexpr double wall_height = 1.0;  // height of walls, in world units

  // FoV has to be between 0 and 180 degrees (exclusive).
  assert(fov > 0.0001);
  assert(fov < M_PI - 0.0001);

  auto const view_height = static_cast<int>(content.size());
  if (view_height == 0)
  {
    return;
  }

  auto const view_width = static_cast<int>(content[0].size());
  if (view_width == 0)
  {
    return;
  }

  // identify components of a unit vector in the direction of the camera
  // heading and a plane perpendicular to it on which the textels(!) are
  // rendered.
  auto const dir = textray::vector2d::from_angle(heading);
  auto const right = textray::vector2d::from_angle(heading - M_PI / 2);

  // Calculate the linear scale of the vertical FoV based on the viewport's
  // aspect ratio (taking the textel aspect ratio into consideration as well).
  double const tan_half_fov = tan(fov / 2);
  double const fov_scale_y =
      tan_half_fov / view_width * view_height * textel_aspect;

  for (terminalpp::coordinate_type x = 0; x < view_width; ++x)
  {
    // calculate (normalized) ray direction
    double camera_x = 2 * (x + 0.5) / view_width
                      - 1;  // x-coordinate in camera space (range [-1,+1])
    textray::vector2d ray = normalize(dir / tan_half_fov + right * camera_x);

    auto map_x = static_cast<int>(position.x);
    auto map_y = static_cast<int>(position.y);

    // length of ray from current position to next x or y-side
    double side_dist_x;
    double side_dist_y;

    // length of ray from one x or y-side to next x or y-side
    double delta_dist_x = std::abs(1 / ray.x);
    double delta_dist_y = std::abs(1 / ray.y);

    // what direction to step in x or y-direction (either +1 or -1)
    int step_x;
    int step_y;

    // calculate step and initial sideDist
    if (ray.x < 0)
    {
      step_x = -1;
      side_dist_x = (position.x - map_x) * delta_dist_x;
    }
    else
    {
      step_x = 1;
      side_dist_x = (map_x + 1.0 - position.x) * delta_dist_x;
    }
    if (ray.y < 0)
    {
      step_y = -1;
      side_dist_y = (position.y - map_y) * delta_dist_y;
    }
    else
    {
      step_y = 1;
      side_dist_y = (map_y + 1.0 - position.y) * delta_dist_y;
    }

    // perform DDA (Digital Differential Analysis)
    double wall_dist;
    int side;
    do
    {
      // jump to next map square, OR in x-direction, OR in y-direction
      if (side_dist_x < side_dist_y)
      {
        wall_dist = side_dist_x;
        side_dist_x += delta_dist_x;
        map_x += step_x;
        side = 0;
      }
      else
      {
        wall_dist = side_dist_y;
        side_dist_y += delta_dist_y;
        map_y += step_y;
        side = 1;
      }

      // Check if ray has hit a wall
      //  TODO: fix when fill is more than a character code.
    } while (plan[map_y][map_x].fill.glyph_.character_ == 0);

    // Calculate distance projected on camera direction (direct distance along
    // ray will give fisheye effect!)
    auto const perp_wall_dist = dot(wall_dist * ray, dir);
    if (perp_wall_dist > 0.001)
    {
      // Calculate height of line to draw on screen.
      // Correct for the textel aspect ratio to make sure the height is correct
      // on the screen.
      auto line_height = view_height * wall_height / perp_wall_dist
                         / fov_scale_y / textel_aspect;

      // Calculate lowest and highest textel to fill in current stripe
      double draw_start = std::max(view_height / 2.0 - line_height / 2.0, 0.0);
      double draw_end = std::min(
          view_height / 2.0 + line_height / 2.0,
          static_cast<double>(view_height));

      using namespace terminalpp::literals;  // NOLINT
      static constexpr auto cube_glyph = R"(\U28FF)"_ete.glyph_;
      auto const high_glyph = top_glyph(draw_start);
      auto const low_glyph = bottom_glyph(draw_end);

      if (static_cast<int>(draw_start) > 0)
      {
        content[static_cast<int>(draw_start) - 1][x].glyph_ =
            top_ceiling_glyph(draw_start);
      }

      for (auto row = static_cast<terminalpp::coordinate_type>(draw_start);
           row < static_cast<terminalpp::coordinate_type>(draw_end);
           ++row)
      {
        auto const colour = terminalpp::colour{
            terminalpp::low_colour{static_cast<terminalpp::graphics::colour>(
                plan[map_y][map_x].fill.glyph_.character_)}};

        auto const darkest_distance = 7;
        auto const percentage_factor = 100 / darkest_distance;
        auto const distance =
            std::min(wall_dist, static_cast<double>(darkest_distance));
        auto const darkness_percentage = distance * percentage_factor;

        auto const darkened_colour =
            darken_colour(colour, lerp0(90, darkness_percentage));
        auto const brush = terminalpp::element{
            (row == static_cast<int>(draw_start)       ? high_glyph
             : row == (static_cast<int>(draw_end) - 1) ? low_glyph
                                                       : cube_glyph),
            darkened_colour};

        content[row][x] = brush;

        if (perp_wall_dist < 1.0)
        {
          content[row][x].attribute_.intensity_ =
              terminalpp::graphics::intensity::bold;
        }
        else if (perp_wall_dist > 2.5)
        {
          content[row][x].attribute_.intensity_ =
              terminalpp::graphics::intensity::faint;
        }
      }

      if (static_cast<int>(draw_end) < view_height)
      {
        content[static_cast<int>(draw_end)][x].glyph_ =
            bottom_floor_glyph(draw_end);
      }
    }
  }
}

void render_camera_image(
    terminalpp::extent size,
    munin::image &img,
    textray::floorplan const &plan,
    textray::vector2d const &position,
    double heading,
    double fov)
{
  std::vector<terminalpp::string> content;
  render_ceiling(content, size);
  render_floor(content, size);
  render_walls(content, plan, position, heading, fov);

  img.set_content(content);
}

}  // namespace

namespace textray {

camera::camera(
    std::shared_ptr<floorplan> plan,
    vector2d position,
    double heading,
    double fov)
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
  on_redraw({terminalpp::rectangle({}, get_size())});
}

void camera::set_fov(double fov)
{
  assert(fov > 0.0001);
  assert(fov < M_PI - 0.0001);
  fov_ = std::move(fov);
  on_redraw({terminalpp::rectangle({}, get_size())});
}

void camera::do_set_size(terminalpp::extent const &size)
{
  image_->set_size(size);
  basic_component::do_set_size(size);
}

void camera::do_draw(
    munin::render_surface &surface, terminalpp::rectangle const &region) const
{
  if (get_size() != terminalpp::extent(0, 0))
  {
    render_camera_image(
        get_size(), *image_, *floorplan_, position_, heading_, fov_);
    image_->draw(surface, region);
  }
}

}  // namespace textray
