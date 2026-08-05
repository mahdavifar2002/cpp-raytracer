#ifndef RTW_OBJ_LOADER_H
#define RTW_OBJ_LOADER_H

#include <iostream>

#include "material.h"

class face_t {
  public:
    point3 vertices[3];
    vec3 normals[3];
    double tex_u[3];
    double tex_v[3];
    int mat_id;
};

class rtw_obj {
  public:
    std::vector<face_t> faces;
    std::vector<shared_ptr<material>> materials;

    rtw_obj() {}

    // Loads obj data from the specified file. If the RTW_MODELS environment variable is
    // defined, looks only in that directory for the obj file. If the model was not found,
    // searches for the specified obj file first from the current directory, then in the
    // models/ subdirectory, then the _parent's_ models/ subdirectory, and then _that_
    // parent, and so on, for six levels up.
    rtw_obj(const std::string& directory, const std::string& filename) {
        auto model_dir = getenv("RTW_MODELS");

        // Hunt for the model file in some likely locations.
        if (model_dir && load(std::string(model_dir) + directory, filename)) return;
        if (load(directory, filename)) return;
        if (load("models/" + directory, filename)) return;
        if (load("../models/" + directory, filename)) return;
        if (load("../../models/" + directory, filename)) return;
        if (load("../../../models/" + directory, filename)) return;
        if (load("../../../../models/" + directory, filename)) return;
        if (load("../../../../../models/" + directory, filename)) return;
        if (load("../../../../../../models/" + directory, filename)) return;

        std::cerr << "ERROR: Could not load obj file: '" << directory + filename << "'.\n";
    }

    // Implementation in the `rtw_obj_loader.cc`
    bool load(const std::string& model_dir, const std::string& filename);
};

#endif