#pragma once

#include "floorplan.hpp"
#include "vector2d.hpp"
#include <munin/basic_component.hpp>
#include <munin/image.hpp>
#include <memory>

namespace textray {
    
class camera : public munin::basic_component
{
public :
    //* =====================================================================
    /// \brief Constructor
    /// \param position position of the camera on the floorplan.
    /// \param heading view direction of the camera, in radians.
    /// \param fov horizontal field of view of the camera, in radians.
    //* =====================================================================
    camera(std::shared_ptr<floorplan> plan, vector2d position, double heading, double fov);

    //* =====================================================================
    /// \brief Move to the specified position and heading.
    //* =====================================================================
    void move_to(vector2d position, double heading);

    //* =====================================================================
    /// \brief Set horizontal field of view, in radians
    //* =====================================================================
    void set_fov(double fov);

private :
    //* =====================================================================
    /// \brief Called by get_preferred_size().  Derived classes must override
    /// this function in order to get the size of the component in a custom
    /// manner.
    //* =====================================================================
    terminalpp::extent do_get_preferred_size() const override;

    //* =====================================================================
    /// \brief Called by draw().  Derived classes must override this function
    /// in order to draw onto the passed context.  A component must only draw
    /// the part of itself specified by the region.
    ///
    /// \param ctx the context in which the component should draw itself.
    /// \param region the region relative to this component's origin that
    /// should be drawn.
    //* =====================================================================
    void do_draw(
        munin::render_surface &surface, 
        terminalpp::rectangle const &region) const override;

    void do_set_size(terminalpp::extent const &size) override;
    
    std::shared_ptr<munin::image> image_;
    std::shared_ptr<floorplan> floorplan_;
    vector2d position_;
    double heading_;
    double fov_;
};

}
