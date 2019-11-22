
#pragma once

#include <array>
#include <string>
#include <vector>

// return a vector of Matrix (4*4) of frame_id, vector[0] means the trans.
// matrix of object[0]

// R00 R01 R02 T1
// R10 R11 R12 T2
// R20 R21 R22 T3
// 0    0   0   1

std::vector<std::array<std::array<float, 4>, 4>>
read_file(const std::string &file_path, unsigned frame_id);



