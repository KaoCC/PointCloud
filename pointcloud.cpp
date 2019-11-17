
#include <embree3/rtcore.h>
#include <iostream>

#include <array>
#include <cmath>
#include <fstream>
#include <unordered_set>
#include <vector>

#include <x86intrin.h>

#include "helper.hpp"

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
#define _MM_DENORMALS_ZERO_ON (0x0040)
#define _MM_DENORMALS_ZERO_OFF (0x0000)
#define _MM_DENORMALS_ZERO_MASK (0x0040)
#define _MM_SET_DENORMALS_ZERO_MODE(x)                                         \
    (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif

// tmp, helper variables

static std::string output_file_name = "points.xyz";
static const std::string file_path = "/tmp/trans.txt";

// tmp, helper struct

struct Vertex {
    float x;
    float y;
    float z;
    float r = 1.0f;
};

struct Triangle {
    int v0, v1, v2;
};

struct Vec {

    Vec(float xx, float yy, float zz) : x(xx), y(yy), z(zz), w(1.0f) {}

    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1.0f;
};

// tmp, helper ops

Vec operator+(const Vec &a, const Vec &b) {
    return Vec(a.x + b.x, a.y + b.y, a.z + b.z);
}
Vec operator*(float f, const Vec &b) { return Vec(f * b.x, f * b.y, f * b.z); }
Vec operator*(const Vec &a, float f) { return Vec(a.x * f, a.y * f, a.z * f); }
float dot(const Vec &a, const Vec &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vec normalize(const Vec &a) { return a * (1.0 / sqrt(dot(a, a))); }

// tmp, helper function

/* adds a ground plane to the scene */
static unsigned int addGroundPlane(RTCScene scene, RTCDevice device) {
    /* create a triangulated plane with 2 triangles and 4 vertices */
    RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    /* set vertices */
    Vertex *vertices = (Vertex *)rtcSetNewGeometryBuffer(
        mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 4);
    vertices[0].x = -10;
    vertices[0].y = -10;
    vertices[0].z = -2;
    vertices[1].x = -10;
    vertices[1].y = +10;
    vertices[1].z = -2;
    vertices[2].x = +10;
    vertices[2].y = -10;
    vertices[2].z = -2;
    vertices[3].x = +10;
    vertices[3].y = +10;
    vertices[3].z = -2;

    /* set triangles */
    Triangle *triangles = (Triangle *)rtcSetNewGeometryBuffer(
        mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 2);
    triangles[0].v0 = 0;
    triangles[0].v1 = 1;
    triangles[0].v2 = 2;
    triangles[1].v0 = 1;
    triangles[1].v1 = 3;
    triangles[1].v2 = 2;

    rtcCommitGeometry(mesh);
    unsigned int geomID = rtcAttachGeometry(scene, mesh);
    rtcReleaseGeometry(mesh);
    return geomID;
}

static void applyTrans(const std::array<std::array<float, 4>, 4> &trans,
                       Vertex &point) {

    /*std::cout << std::endl;
    for(auto&& row : trans) {
        for (auto&& ele : row) {
            std::cout << ele << " ";
        }
        std::cout << std::endl;
    }*/

    // std::cout << point.x << " " << point.y << " " << point.z << std::endl;

    float x_t = trans[0][0] * point.x + trans[0][1] * point.y +
                trans[0][2] * point.z + trans[0][3] * 1;
    float y_t = trans[1][0] * point.x + trans[1][1] * point.y +
                trans[1][2] * point.z + trans[1][3] * 1;
    float z_t = trans[2][0] * point.x + trans[2][1] * point.y +
                trans[2][2] * point.z + trans[2][3] * 1;
    // last should always be [0, 0, 0, 1] * [x, y, z, 1]^T = 1

    point.x = x_t;
    point.y = y_t;
    point.z = z_t;
    // point.w should be 1

    // std::cout << point.x << " " << point.y << " " << point.z << std::endl;
}

/* adds a cube to the scene */
static unsigned int addCube(RTCScene scene_i, RTCDevice device,
                            const std::array<std::array<float, 4>, 4> &trans) {
    /* create a triangulated cube with 12 triangles and 8 vertices */
    RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    /* set vertices and vertex colors */
    Vertex *vertices = (Vertex *)rtcSetNewGeometryBuffer(
        mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 8);

    vertices[0].x = -1;
    vertices[0].y = -1;
    vertices[0].z = -1;
    vertices[1].x = -1;
    vertices[1].y = -1;
    vertices[1].z = +1;
    vertices[2].x = -1;
    vertices[2].y = +1;
    vertices[2].z = -1;
    vertices[3].x = -1;
    vertices[3].y = +1;
    vertices[3].z = +1;
    vertices[4].x = +1;
    vertices[4].y = -1;
    vertices[4].z = -1;
    vertices[5].x = +1;
    vertices[5].y = -1;
    vertices[5].z = +1;
    vertices[6].x = +1;
    vertices[6].y = +1;
    vertices[6].z = -1;
    vertices[7].x = +1;
    vertices[7].y = +1;
    vertices[7].z = +1;

    /* set triangles and face colors */
    int tri = 0;
    Triangle *triangles = (Triangle *)rtcSetNewGeometryBuffer(
        mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 12);

    // left side
    triangles[tri].v0 = 0;
    triangles[tri].v1 = 1;
    triangles[tri].v2 = 2;
    tri++;
    triangles[tri].v0 = 1;
    triangles[tri].v1 = 3;
    triangles[tri].v2 = 2;
    tri++;

    // right side
    triangles[tri].v0 = 4;
    triangles[tri].v1 = 6;
    triangles[tri].v2 = 5;
    tri++;
    triangles[tri].v0 = 5;
    triangles[tri].v1 = 6;
    triangles[tri].v2 = 7;
    tri++;

    // bottom side
    triangles[tri].v0 = 0;
    triangles[tri].v1 = 4;
    triangles[tri].v2 = 1;
    tri++;
    triangles[tri].v0 = 1;
    triangles[tri].v1 = 4;
    triangles[tri].v2 = 5;
    tri++;

    // top side
    triangles[tri].v0 = 2;
    triangles[tri].v1 = 3;
    triangles[tri].v2 = 6;
    tri++;
    triangles[tri].v0 = 3;
    triangles[tri].v1 = 7;
    triangles[tri].v2 = 6;
    tri++;

    // front side
    triangles[tri].v0 = 0;
    triangles[tri].v1 = 2;
    triangles[tri].v2 = 4;
    tri++;
    triangles[tri].v0 = 2;
    triangles[tri].v1 = 6;
    triangles[tri].v2 = 4;
    tri++;

    // back side
    triangles[tri].v0 = 1;
    triangles[tri].v1 = 5;
    triangles[tri].v2 = 3;
    tri++;
    triangles[tri].v0 = 3;
    triangles[tri].v1 = 5;
    triangles[tri].v2 = 7;
    tri++;

    // apply transformation
    const size_t cube_size = 8;
    for (size_t i = 0; i < cube_size; ++i) {
        applyTrans(trans, vertices[i]);
    }

    //  rtcSetGeometryVertexAttributeCount(mesh,1);
    // rtcSetSharedGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),8);

    rtcCommitGeometry(mesh);
    unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
    rtcReleaseGeometry(mesh);
    return geomID;
}

int main(int argc, char *argv[]) {

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    std::cout << "Creating new device ..." << std::endl;
    auto device = rtcNewDevice("verbose=1");

    auto scene = rtcNewScene(device);

    std::cout << "add ground: " << addGroundPlane(scene, device) << std::endl;

    // test
    /* std::array<std::array<float, 4>, 4> tran;

    // I
    tran[0][0] = 1;
    tran[1][1] = 1;
    tran[2][2] = 1;
    tran[3][3] = 1;

    // T
    tran[0][3] = 4;
    tran[1][3] = 4;
    tran[2][3] = 4; */

    /* read trans */

    auto transformation = readFile(file_path, 100);

    for (auto &&tran : transformation) {
        std::cout << "add Cube: " << addCube(scene, device, tran) << std::endl;
    }

    rtcCommitScene(scene);

    Vec camera_orig(0, 0, 0);
    Vec u_axis(1, 0, 0);
    Vec v_axis(0, 1, 0);
    Vec w_axis(0, 0, 1);

    const unsigned width = 2048;
    const unsigned height = 2048;

    std::vector<std::vector<Vec>> hit_points(
        height, std::vector<Vec>(width, Vec(0, 0, 0)));

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

            Vec ray_dir = normalize((u_scale * u_axis) + (v_scale * v_axis) +
                                    (w_scale * w_axis));

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
                    std::cout << "HIT: " << i << " " << j << " "
                              << rayhit.ray.tfar << "\n";
                }

                auto final_pos = (rayhit.ray.tfar * ray_dir);

                if (use_world_frame) {
                    final_pos = final_pos + camera_orig;
                }

                // tmp
                hit_points[i][j].x = final_pos.x;
                hit_points[i][j].y = final_pos.y;
                hit_points[i][j].z = final_pos.z;

                ofs << final_pos.x << " " << final_pos.y << " " << final_pos.z
                    << " ";

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

    std::cout << "total hit counts: " << hit_count
              << " number of samples: " << width * height << std::endl;

    // clean up ...
    std::cout << "Releasing scene ... " << std::endl;
    rtcReleaseScene(scene);
    std::cout << "Releasing device ..." << std::endl;
    rtcReleaseDevice(device);

    return 0;
}
