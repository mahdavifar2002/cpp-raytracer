#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include <string>

class scene_parser {
  public:
    scene_parser(const std::string& filename);

    bool parse();
    bool load(const std::string& filename);
    void render_scene(const std::string& output_filename);

  private:
    std::string filename;
    camera cam;
    hittable_list world;
};

#endif