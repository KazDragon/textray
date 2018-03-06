#pragma once

#include "core.hpp"

namespace ma {

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
    /// Constructs a vector2d, leaving the values uninitialised.
    //* =====================================================================
    constexpr vector2d()
      : x{},
        y{}
    {
    }

    //* =====================================================================
    /// \brief Constructor
    /// \par
    /// Constructs a vector2d from a passed in x co-ordinate and a passed in
    /// y co-ordinate.
    //* =====================================================================
    constexpr vector2d(double x_coordinate, double y_coordinate)
      : x(x_coordinate),
        y(y_coordinate)
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

}