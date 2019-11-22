#pragma once

#include <array>
#include <embree3/rtcore.h>

unsigned int add_ground_plane(RTCScene scene, RTCDevice device);
unsigned int add_cube(RTCScene scene_i, RTCDevice device,
                            const std::array<std::array<float, 4>, 4> &trans);

