#pragma once

#include "floorplan.hpp"
#include "vector2d.hpp"
#include <munin/composite_component.hpp>
#include <memory>

namespace textray {
    
class ui : public munin::composite_component
{
public :
    ui(std::shared_ptr<floorplan> plan, vector2d position, double heading, double fov);
    ~ui();
    
    void move_camera_to(vector2d const &position, double heading);
    void set_camera_fov(double fov);
    
private :
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}