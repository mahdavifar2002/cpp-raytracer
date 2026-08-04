#include "scene_parser.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string scene_file = "scene.json";
    std::string output_file = "image.ppm";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-c" || arg == "--scene") && i + 1 < argc) {
            scene_file = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_file = argv[++i];
        }
    }

    std::cerr << "Loading scene from: " << scene_file << "\n";

    // Build and render
    scene_parser parser(scene_file);
    parser.parse();
    parser.render_scene(output_file);

    return 0;
}