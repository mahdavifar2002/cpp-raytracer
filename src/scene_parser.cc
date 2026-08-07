#include "rtweekend.h"
#include "scene_parser.h"
#include "sphere.h"
#include "bvh.h"
#include "quad.h"
#include "constant_medium.h"
#include "material.h"
#include "texture.h"

// Include the heavy JSON library ONLY in the translation unit
#include "external/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// --- Anonymous Namespace ---
// Everything in here is hidden from the rest of the C++ project.
namespace {

    vec3 parse_vec3(const json& j) {
        return vec3(j[0].get<double>(), j[1].get<double>(), j[2].get<double>());
    }

    color parse_color(const json& j) {
        return color(j[0].get<double>(), j[1].get<double>(), j[2].get<double>());
    }

    // Forward declare parse_texture if it needs to call itself recursively
    shared_ptr<texture> parse_texture(const json& j);

    shared_ptr<texture> parse_texture(const json& j) {
        std::string type = j["type"].get<std::string>();
        if (type == "solid") {
            return make_shared<solid_color>(parse_color(j["color"]));
        }
        else if (type == "checker") {
            if (j.contains("even"))
                return make_shared<checker_texture>(j["scale"].get<double>(), parse_texture(j["even"]), parse_texture(j["odd"]));
            if (j.contains("c1"))
                return make_shared<checker_texture>(j["scale"].get<double>(), parse_color(j["c1"]), parse_color(j["c2"]));
        }
        else if (type == "image") {
            return make_shared<image_texture>(j["filename"].get<std::string>().c_str());
        }
        else if (type == "noise") {
            return make_shared<noise_texture>(j["scale"].get<double>());
        }
        return make_shared<solid_color>(color(1, 0, 1));
    }

    shared_ptr<material> parse_material(const json& j) {
        std::string type = j["type"].get<std::string>();
        if (type == "lambertian") {
            if (j.contains("albedo")) return make_shared<lambertian>(parse_color(j["albedo"]));
            if (j.contains("texture")) return make_shared<lambertian>(parse_texture(j["texture"]));
        }
        else if (type == "metal") {
            return make_shared<metal>(parse_color(j["albedo"]), j["fuzz"].get<double>());
        }
        else if (type == "dielectric") {
            return make_shared<dielectric>(j["refraction_index"].get<double>());
        }
        else if (type == "diffuse_light") {
            if (j.contains("emit")) return make_shared<diffuse_light>(parse_color(j["emit"]));
            return make_shared<diffuse_light>(parse_texture(j["texture"]));
        }
        return make_shared<lambertian>(color(1, 0, 1));
    }

    // Forward declare parse_hittable if it needs to call itself recursively
    shared_ptr<hittable> parse_hittable(const json& j);

    shared_ptr<hittable> parse_hittable(const json& j) {
        std::string type = j["type"].get<std::string>();

        if (type == "sphere") {
            auto mat = parse_material(j["material"]);
            if (j.contains("center2")) {
                return make_shared<sphere>(parse_vec3(j["center"]), parse_vec3(j["center2"]), j["radius"].get<double>(), mat);
            } else {
                return make_shared<sphere>(parse_vec3(j["center"]), j["radius"].get<double>(), mat);
            }
        }
        else if (type == "quad") {
            return make_shared<quad>(parse_vec3(j["Q"]), parse_vec3(j["u"]), parse_vec3(j["v"]), parse_material(j["material"]));
        }
        else if (type == "box") {
            return box(parse_vec3(j["a"]), parse_vec3(j["b"]), parse_material(j["material"]));
        }
        else if (type == "mesh") {
            shared_ptr<material> mat_override = nullptr;
    
            // Only parse the material if it explicitly exists in the JSON
            if (j.contains("material")) {
                mat_override = parse_material(j["material"]);
            }

            return mesh(j["filename"].get<std::string>().c_str(), mat_override, j.value("scale", 1.0));
        }
        else if (type == "translate") {
            return make_shared<translate>(parse_hittable(j["object"]), parse_vec3(j["offset"]));
        }
        else if (type == "rotate_x") {
            return make_shared<rotate_x>(parse_hittable(j["object"]), j["angle"].get<double>());
        }
        else if (type == "rotate_y") {
            return make_shared<rotate_y>(parse_hittable(j["object"]), j["angle"].get<double>());
        }
        else if (type == "rotate_z") {
            return make_shared<rotate_z>(parse_hittable(j["object"]), j["angle"].get<double>());
        }
        else if (type == "constant_medium") {
            if (j.contains("color")) {
                return make_shared<constant_medium>(parse_hittable(j["object"]), j["density"].get<double>(), parse_color(j["color"]));
            }
        }
        else if (type == "bvh_node") {
            hittable_list list;
            for (const auto& item : j["objects"]) {
                list.add(parse_hittable(item));
            }
            return make_shared<bvh_node>(list);
        }

        std::cerr << "Unknown hittable type: " << type << "\n";
        return nullptr;
    }

} // --- End Anonymous Namespace ---


// --- Class Implementation ---

scene_parser::scene_parser(const std::string& filename) : filename(filename) {}

bool scene_parser::parse() {
    auto scene_dir = getenv("RTW_SCENES");

    // Hunt for the scene file in some likely locations.
    if (scene_dir && load(std::string(scene_dir) + "/" + filename)) return true;
    if (load(filename)) return true;
    if (load("scenes/" + filename)) return true;
    if (load("../scenes/" + filename)) return true;
    if (load("../../scenes/" + filename)) return true;
    if (load("../../../scenes/" + filename)) return true;
    if (load("../../../../scenes/" + filename)) return true;
    if (load("../../../../../scenes/" + filename)) return true;
    if (load("../../../../../../scenes/" + filename)) return true;

    std::cerr << "ERROR: Could not open scene file: '" << filename << "'.\n";

    return false;
}

bool scene_parser::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
   
    // Load JSON locally inside the function!
    json scene_data;
    file >> scene_data;

    // 1. Parse Camera
    if (scene_data.contains("camera")) {
        auto c = scene_data["camera"];
        cam.aspect_ratio      = c.value("aspect_ratio", 16.0 / 9.0);
        cam.image_width       = c.value("image_width", 400);
        cam.samples_per_pixel = c.value("samples_per_pixel", 100);
        cam.exposure          = c.value("exposure", 1.0);
        cam.max_depth         = c.value("max_depth", 6);
        cam.vfov              = c.value("vfov", 40.0);
       
        if (c.contains("lookfrom")) cam.lookfrom = parse_vec3(c["lookfrom"]);
        if (c.contains("lookat"))   cam.lookat = parse_vec3(c["lookat"]);
        if (c.contains("vup"))      cam.vup = parse_vec3(c["vup"]);
       
        cam.defocus_angle = c.value("defocus_angle", 0.0);
        cam.focus_dist    = c.value("focus_dist", 10.0);

        if (c.contains("background")) {
            cam.background = parse_texture(c["background"]);
        } else {
            cam.background = make_shared<solid_color>(color(0,0,0));
        }
    }

    // 2. Parse World
    if (scene_data.contains("world")) {
        for (const auto& item : scene_data["world"]) {
            world.add(parse_hittable(item)); // Calls the anonymous namespace function
        }
    }
   
    world = hittable_list(make_shared<bvh_node>(world));

    return true;
}

void scene_parser::set_samples_per_pixel(int samples) {
    cam.samples_per_pixel = samples;
}

void scene_parser::set_width(int width) {
    cam.image_width = width;
}

void scene_parser::render_scene(const std::string& output_filename) {
    cam.render(world, output_filename);
}