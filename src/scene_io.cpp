
#include "scene_io.hpp"
#include "type.hpp"


namespace {


void apply_transformation(const std::array<std::array<float, 4>, 4> &trans,
                       Vertex &point) {

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

}



}


/* adds a ground plane to the scene */
unsigned int add_ground_plane(RTCScene scene, RTCDevice device) {
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



/* adds a cube to the scene */
unsigned int add_cube(RTCScene scene_i, RTCDevice device,
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
        apply_transformation(trans, vertices[i]);
    }

    //  rtcSetGeometryVertexAttributeCount(mesh,1);
    // rtcSetSharedGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(Vec3fa),8);

    rtcCommitGeometry(mesh);
    unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
    rtcReleaseGeometry(mesh);
    return geomID;
}

