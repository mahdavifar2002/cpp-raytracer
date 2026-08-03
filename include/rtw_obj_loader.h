#ifndef RTW_OBJ_LOADER_H
#define RTW_OBJ_LOADER_H

#include <iostream>

class face_t {
  public:
    point3 vertices[3];
    vec3 normals[3];
    double tex_u;
    double tex_v;
};

class rtw_obj {
  public:
    std::vector<face_t> faces;

    rtw_obj() {}

    // Loads obj data from the specified file. If the RTW_MODELS environment variable is
    // defined, looks only in that directory for the obj file. If the model was not found,
    // searches for the specified obj file first from the current directory, then in the
    // models/ subdirectory, then the _parent's_ models/ subdirectory, and then _that_
    // parent, and so on, for six levels up.
    rtw_obj(const char* obj_filename) {
        auto filename = std::string(obj_filename);
        auto model_dir = getenv("RTW_MODELS");

        // Hunt for the model file in some likely locations.
        if (model_dir && load(std::string(model_dir) + "/" + obj_filename)) return;
        if (load(filename)) return;
        if (load("models/" + filename)) return;
        if (load("../models/" + filename)) return;
        if (load("../../models/" + filename)) return;
        if (load("../../../models/" + filename)) return;
        if (load("../../../../models/" + filename)) return;
        if (load("../../../../../models/" + filename)) return;
        if (load("../../../../../../models/" + filename)) return;

        std::cerr << "ERROR: Could not load obj file: '" << obj_filename << "'.\n";
    }

    // Implementation in the `rtw_obj_loader.cc`
    bool load(const std::string& filename);
};

#endif