#pragma once

namespace textray {

//* =========================================================================
/// \brief A class that represents a point in space.
/// \par
/// A class that represents a point in space, where x is the co-ordinate
/// along the horizontal axis and y being the co-ordinate along the vertical
/// axis.
//* =========================================================================
struct point
{
  //* =====================================================================
  /// \brief Default Constructor
  /// \par
  /// Constructs a point, leaving the values uninitialized.
  //* =====================================================================
  constexpr point() : x{}, y{}
  {
  }

  //* =====================================================================
  /// \brief Constructor
  /// \par
  /// Constructs a point from a passed in x co-ordinate and a passed in
  /// y co-ordinate.
  //* =====================================================================
  constexpr point(double x_coordinate, double y_coordinate)
    : x(x_coordinate), y(y_coordinate)
  {
  }

  //* =====================================================================
  /// \brief Addition
  //* =====================================================================
  constexpr point &operator+=(point const &rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  //* =====================================================================
  /// \brief Subtraction
  //* =====================================================================
  constexpr point &operator-=(point const &rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  double x;
  double y;
};

// ==========================================================================
// OPERATOR==(point,point)
// ==========================================================================
constexpr bool operator==(point const &lhs, point const &rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

// ==========================================================================
// OPERATOR!=(point,point)
// ==========================================================================
constexpr bool operator!=(point const &lhs, point const &rhs)
{
  return !(lhs == rhs);
}

// ==========================================================================
// OPERATOR+(point,point)
// ==========================================================================
constexpr point operator+(point lhs, point const &rhs)
{
  return lhs += rhs;
}

// ==========================================================================
// OPERATOR-(point,point)
// ==========================================================================
constexpr point operator-(point lhs, point const &rhs)
{
  return lhs -= rhs;
}

}  // namespace textray