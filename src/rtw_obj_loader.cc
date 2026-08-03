#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tiny_obj_loader.h"

#include "rtweekend.h"
#include "rtw_obj_loader.h"

bool rtw_obj::load(const std::string& filename) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, reader_config)) {
        // if (!reader.Error().empty()) std::cerr << "TinyObjReader: " << reader.Error();
        return false;
    }

    if (!reader.Warning().empty()) { std::cout << "TinyObjReader: " << reader.Warning(); }

    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            // Create the face object.
            face_t face;
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
    
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                double vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                double vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                double vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                face.vertices[v] = point3(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    double nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    double ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    double nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                    face.normals[v] = vec3(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    double tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    double ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                    face.tex_u = tx;
                    face.tex_v = ty;
                }

                // Optional: vertex colors
                // double red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // double green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // double blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }
            
            faces.push_back(face);
            index_offset += fv;

            // per-face material
            int id = shapes[s].mesh.material_ids[f];
        }
    }

    return true;
}