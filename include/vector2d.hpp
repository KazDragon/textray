#pragma once

#include <cmath>

namespace textray {

//* =========================================================================
/// \brief A class that represents a vector in space.
/// \par
/// A class that represents a vector in space, where x is the co-ordinate
/// along the horizontal axis and y being the co-ordinate along the vertical
/// axis.
//* =========================================================================
struct vector2d
{
  //* =====================================================================
  /// \brief Default Constructor
  /// \par
  /// Constructs a vector2d, leaving the values uninitialized.
  //* =====================================================================
  constexpr vector2d() : x{}, y{}
  {
  }

  //* =====================================================================
  /// \brief Constructor
  /// \par
  /// Constructs a vector2d from a passed in x co-ordinate and a passed in
  /// y co-ordinate.
  //* =====================================================================
  constexpr vector2d(double x_coordinate, double y_coordinate)
    : x(x_coordinate), y(y_coordinate)
  {
  }

  //* =====================================================================
  /// \brief Addition
  //* =====================================================================
  constexpr vector2d &operator+=(vector2d const &rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  //* =====================================================================
  /// \brief Subtraction
  //* =====================================================================
  constexpr vector2d &operator-=(vector2d const &rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  //* ==========================================================================
  /// \brief Length
  //* ==========================================================================
  [[nodiscard]] double length() const
  {
    return std::sqrt(x * x + y * y);
  }

  //* ==========================================================================
  /// \brief Construct from angle, in radians
  //* ==========================================================================
  static vector2d from_angle(double angle)
  {
    return {std::cos(angle), std::sin(angle)};
  }

  double x;
  double y;
};

// ==========================================================================
// OPERATOR==(vector2d,vector2d)
// ==========================================================================
constexpr bool operator==(vector2d const &lhs, vector2d const &rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

// ==========================================================================
// OPERATOR!=(vector2d,vector2d)
// ==========================================================================
constexpr bool operator!=(vector2d const &lhs, vector2d const &rhs)
{
  return !(lhs == rhs);
}

// ==========================================================================
// OPERATOR+(vector2d,vector2d)
// ==========================================================================
constexpr vector2d operator+(vector2d lhs, vector2d const &rhs)
{
  return lhs += rhs;
}

// ==========================================================================
// OPERATOR-(vector2d,vector2d)
// ==========================================================================
constexpr vector2d operator-(vector2d lhs, vector2d const &rhs)
{
  return lhs -= rhs;
}

// ==========================================================================
// OPERATOR*(double,vector2d)
// ==========================================================================
constexpr vector2d operator*(double lhs, vector2d const &rhs)
{
  return {lhs * rhs.x, lhs * rhs.y};
}

// ==========================================================================
// OPERATOR*(vector2d,double)
// ==========================================================================
constexpr vector2d operator*(vector2d const &lhs, double rhs)
{
  return {lhs.x * rhs, lhs.y * rhs};
}

// ==========================================================================
// OPERATOR/(vector2d, double)
// ==========================================================================
constexpr vector2d operator/(vector2d const &lhs, double rhs)
{
  return {lhs.x / rhs, lhs.y / rhs};
}

// ==========================================================================
// dot(vector2d,vector2d)
// ==========================================================================
constexpr double dot(vector2d const &lhs, vector2d const &rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

// ==========================================================================
// normalize(vector2d)
// ==========================================================================
inline vector2d normalize(vector2d const &v)
{
  return v / v.length();
}

}  // namespace textray
