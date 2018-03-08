#pragma once

#include "floorplan.hpp"
#include "point.hpp"
#include "vector2d.hpp"
#include <munin/basic_component.hpp>
#include <munin/image.hpp>
#include <memory>

namespace ma {
    
class camera : public munin::basic_component
{
public :
    camera(std::shared_ptr<floorplan> plan, point position, double heading);

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
        terminalpp::canvas_view &cv, munin::rectangle const &region) const override;
    
    std::shared_ptr<munin::image> image_;
    std::shared_ptr<floorplan> floorplan_;
    point position_;
    double heading_;
};

}
