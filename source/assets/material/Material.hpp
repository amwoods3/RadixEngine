#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <string>
#include <vector>
#include <assets/texture/Texture.hpp>

namespace glPortal {

class Material {
public:
  std::string name, fancyname;
  Texture diffuse, specular, normal, height;
  float scaleU = 1, scaleV = 1;

  bool portalable, portalBump;
  std::string kind;
  std::vector<std::string> tags;

  int tileScale;
};

} /* namespace glPortal */

#endif /* MATERIAL_HPP */
