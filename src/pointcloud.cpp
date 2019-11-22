
#include <embree3/rtcore.h>
#include <iostream>

#include <array>
#include <fstream>
#include <unordered_set>
#include <vector>

#include <x86intrin.h>

#include "helper.hpp"
#include "scene_io.hpp"
#include "type.hpp"

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
#define _MM_DENORMALS_ZERO_ON (0x0040)
#define _MM_DENORMALS_ZERO_OFF (0x0000)
#define _MM_DENORMALS_ZERO_MASK (0x0040)
#define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif

// tmp, helper variables

static std::string output_file_name = "points.xyz";
static const std::string file_path = "/tmp/trans.txt";

int main(int argc, char *argv[]) {

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    std::cout << "Creating new device ..." << std::endl;
    auto device = rtcNewDevice("verbose=1");

    auto scene = rtcNewScene(device);

    std::cout << "add ground: " << add_ground_plane(scene, device) << std::endl;

    /* read transformation */
    auto transformation = read_file(file_path, 100);

    for (auto &&tran : transformation) {
        std::cout << "add Cube: " << add_cube(scene, device, tran) << std::endl;
    }

    rtcCommitScene(scene);

    Vec camera_orig{0, 0, 0};
    Vec u_axis{1, 0, 0};
    Vec v_axis{0, 1, 0};
    Vec w_axis{0, 0, 1};

    const unsigned width = 2048;
    const unsigned height = 2048;

    std::vector<std::vector<Vec>> hit_points(height, std::vector<Vec>(width, Vec{0, 0, 0}));

    int hit_count = 0;

    bool use_world_frame = false;
    bool verbose_mode = false;
    bool color_mode = false;

    if (argc >= 2) {
        const std::string world_opt = "-w";
        const std::string verbose_opt = "-v";
        const std::string color_opt = "-c";
        std::unordered_set<std::string> options;
        for (int c = 1; c < argc; ++c) {
            options.insert(argv[c]);
        }

        if (options.count(world_opt) == 1) {
            std::cout << "[Option]: Use world frame" << std::endl;
            use_world_frame = true;
            options.erase(world_opt);
        }

        if (options.count(verbose_opt) == 1) {
            std::cout << "[Option]: Verbose mode" << std::endl;
            verbose_mode = true;
            options.erase(verbose_opt);
        }

        if (options.count(color_opt) == 1) {
            std::cout << "[Option]: Color mode" << std::endl;
            color_mode = true;
            options.erase(color_opt);

            output_file_name = "points_with_color.txt";
        }

        if (!options.empty()) {
            for (auto &&opt : options) {
                std::cerr << "Unknown Option found:" << opt << std::endl;
            }
        }
    }

    std::ofstream ofs(output_file_name, std::ofstream::out);

    for (unsigned i = 0; i < width; ++i) {
        for (unsigned j = 0; j < height; ++j) {

            float dt_phi = ((float)i / width) * 2 * M_PI;
            float dt_theta = ((float)j / height) * M_PI;

            float u_scale = cos(dt_phi) * sin(dt_theta);
            float v_scale = sin(dt_phi) * sin(dt_theta);
            float w_scale = cos(dt_theta);

            Vec ray_dir = normalize((u_scale * u_axis) + (v_scale * v_axis) + (w_scale * w_axis));

            // std::cout << "du" << ray_dir.x << "\n";

            RTCRay ray;
            ray.org_x = camera_orig.x;
            ray.org_y = camera_orig.y;
            ray.org_z = camera_orig.z;
            ray.tnear = 0;
            ray.dir_x = ray_dir.x;
            ray.dir_y = ray_dir.y;
            ray.dir_z = ray_dir.z;
            ray.tfar = std::numeric_limits<float>::max();

            RTCRayHit rayhit;
            rayhit.ray = ray;
            rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

            // test intersection
            RTCIntersectContext context;
            rtcInitIntersectContext(&context);

            rtcIntersect1(scene, &context, &rayhit);

            if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
                if (verbose_mode) {
                    std::cout << "HIT: " << i << " " << j << " " << rayhit.ray.tfar << "\n";
                }

                auto final_pos = (rayhit.ray.tfar * ray_dir);

                if (use_world_frame) {
                    final_pos = final_pos + camera_orig;
                }

                // tmp
                hit_points[i][j].x = final_pos.x;
                hit_points[i][j].y = final_pos.y;
                hit_points[i][j].z = final_pos.z;

                ofs << final_pos.x << " " << final_pos.y << " " << final_pos.z << " ";

                if (color_mode) {
                    float color_scale = width / 256;
                    unsigned color_value = ((float)i / color_scale);
                    ofs << color_value << " " << color_value << " " << 0;
                }

                ofs << std::endl;

                ++hit_count;
            }
        }

        camera_orig.x += 0.01;
        camera_orig.y += 0.01;
    }

    std::cout << "total hit counts: " << hit_count << " number of samples: " << width * height << std::endl;

    // clean up ...
    std::cout << "Releasing scene ... " << std::endl;
    rtcReleaseScene(scene);
    std::cout << "Releasing device ..." << std::endl;
    rtcReleaseDevice(device);

    return 0;
}
