#pragma once

#include "floorplan.hpp"
#include "point.hpp"
#include "vector2d.hpp"
#include <munin/component.hpp>
#include <memory>

namespace ma {
    
class camera : public munin::component
{
public :
    camera(std::shared_ptr<floorplan> plan, point position, double heading);

private :
    void do_draw(
        terminalpp::canvas_view &cv, munin::rectangle const &region) const override;
    
    std::shared_ptr<floorplan> floorplan_;
    point position_;
    double heading_;
};

}