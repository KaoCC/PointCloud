
#include <embree3/rtcore.h>
#include <iostream>

#include <vector>
#include <cmath>
#include <fstream>
#include <unordered_set>

#include <x86intrin.h>

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
    #define _MM_DENORMALS_ZERO_ON   (0x0040)
    #define _MM_DENORMALS_ZERO_OFF  (0x0000)
    #define _MM_DENORMALS_ZERO_MASK (0x0040)
    #define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif



// tmp, helper const

const std::string output_file_name = "points.txt";


// tmp, helper struct

struct Vertex   { float x,y,z,r;  };
struct Triangle { int v0, v1, v2; };

struct Vec {

    Vec(float xx, float yy, float zz) : x(xx), y(yy), z(zz), w(1.0f) {}

    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1.0f;
};


// tmp, helper ops

Vec operator +( const Vec& a, const Vec& b ) { return Vec(a.x + b.x, a.y + b.y, a.z + b.z); }
Vec operator *( float f, const Vec& b ) { return Vec(f * b.x, f * b.y, f * b.z); }
Vec operator *( const Vec& a, float f ) { return Vec(a.x * f  , a.y * f  , a.z * f  ); }
float dot( const Vec& a, const Vec& b ) {return a.x * b.x + a.y * b.y + a.z * b.z;}
Vec normalize( const Vec& a )  { return a * (1.0 / sqrt(dot(a,a))) ; }

// tmp, helper function

/* adds a ground plane to the scene */
static unsigned int addGroundPlane (RTCScene scene, RTCDevice device)
{
  /* create a triangulated plane with 2 triangles and 4 vertices */
  RTCGeometry mesh = rtcNewGeometry (device, RTC_GEOMETRY_TYPE_TRIANGLE);

  /* set vertices */
  Vertex* vertices = (Vertex*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(Vertex),4);
  vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
  vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
  vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
  vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;

  /* set triangles */
  Triangle* triangles = (Triangle*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(Triangle),2);
  triangles[0].v0 = 0; triangles[0].v1 = 1; triangles[0].v2 = 2;
  triangles[1].v0 = 1; triangles[1].v1 = 3; triangles[1].v2 = 2;
  
  rtcCommitGeometry(mesh);
  unsigned int geomID = rtcAttachGeometry(scene, mesh);
  rtcReleaseGeometry(mesh);
  return geomID;
}






int main (int argc, char* argv[]) {

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    std::cout << "Creating new device ..." << std::endl;
    auto device = rtcNewDevice("verbose=1");


    auto scene = rtcNewScene(device);

    std::cout << "add ground: " << addGroundPlane(scene, device) << std::endl;

    rtcCommitScene(scene);

    Vec camera_orig (0, 0, 0);
    Vec u_axis(1, 0, 0);
    Vec v_axis(0, 1, 0);
    Vec w_axis(0, 0, 1);


    const unsigned width = 128;
    const unsigned height = 128;

    std::vector<std::vector<Vec>> hit_points (height, std::vector<Vec>(width, Vec(0, 0, 0)));

    int hit_count = 0;
    std::ofstream ofs (output_file_name, std::ofstream::out);
    bool use_world_frame = false;
    bool verbose_mode = false;

    if (argc >= 2) {
        const std::string world_opt = "-w";
        const std::string verbose_opt = "-v";
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

        if (!options.empty()) {
            for (auto&& opt : options) {
                std::cerr << "Unknown Option found:" << opt << std::endl;
            }
        }
    }

    for (unsigned i = 0; i < width; ++i) {
        for (unsigned j = 0; j < height; ++j) {

            float dt_x = ((float)i / width) * 2 * M_PI;
            float dt_y = ((float)j / height) * M_PI;

            float u_scale = cos(dt_x) * sin (dt_y);
            float v_scale = cos(dt_y);
            float w_scale = sin(dt_x) * sin(dt_y);

            Vec ray_dir = normalize( (u_scale * u_axis) + (v_scale * v_axis) + (w_scale * w_axis) );

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
                    std::cout << "HIT: " << i << " " << j << " " << rayhit.ray.tfar <<"\n";
                }

                auto final_pos = (rayhit.ray.tfar * ray_dir);

                if (use_world_frame) {
                    final_pos = final_pos + camera_orig;
                }

                // tmp
                hit_points[i][j].x = final_pos.x;
                hit_points[i][j].y = final_pos.y;
                hit_points[i][j].z = final_pos.z;

                ofs << final_pos.x << " " << final_pos.y << " " << final_pos.z << " " << 2 * i << " " << 2 * i << " " << 2 * i << std::endl;

                ++hit_count;
            }
        }

        camera_orig.x += 0.05;
        camera_orig.z += 0.01;
    }

    std::cout << "total hit counts: " << hit_count << " number of samples: " << width * height << std::endl;


    // clean up ... 
    std::cout << "Releasing scene ... " << std::endl;
    rtcReleaseScene(scene);
    std::cout << "Releasing device ..." << std::endl;
    rtcReleaseDevice(device);

    return 0;
}