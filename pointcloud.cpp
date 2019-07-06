
#include <embree3/rtcore.h>
#include <iostream>



#include <x86intrin.h>

#if !defined(_MM_SET_DENORMALS_ZERO_MODE)
    #define _MM_DENORMALS_ZERO_ON   (0x0040)
    #define _MM_DENORMALS_ZERO_OFF  (0x0000)
    #define _MM_DENORMALS_ZERO_MASK (0x0040)
    #define _MM_SET_DENORMALS_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (x)))
#endif



// tmp, helper struct

struct Vertex   { float x,y,z,r;  };
struct Triangle { int v0, v1, v2; };



// tmp, helper function

/* adds a ground plane to the scene */
unsigned int addGroundPlane (RTCScene scene, RTCDevice device)
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



int main () {

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    std::cout << "Creating new device ..." << std::endl;
    auto device = rtcNewDevice("verbose=1");


    auto scene = rtcNewScene(device);

    std::cout << "add ground: " << addGroundPlane(scene, device) << std::endl;

    rtcCommitScene(scene);


    

    // clean up ... 

    std::cout << "Releasing scene ... " << std::endl;
    rtcReleaseScene(scene);
    std::cout << "Releasing device ..." << std::endl;
    rtcReleaseDevice(device);

    return 0;
}