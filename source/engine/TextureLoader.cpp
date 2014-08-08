#include "TextureLoader.hpp"

#include <GL/glew.h>
#include <utility>

#include "environment/Environment.hpp"
#include "stb_image.c"

namespace glPortal {

std::map<std::string, Texture> TextureLoader::textureCache = {};

Texture TextureLoader::getTexture(std::string path) {
  path = Environment::getDataDir() + path;
  if(textureCache.find(path) != textureCache.end()) {
    return textureCache.at(path);
  }
  int width, height, bytes;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &bytes, 0);
  Texture texture = uploadTexture(data, width, height, bytes);
  textureCache.insert(std::pair<std::string, Texture>(path, texture));
  return texture;
}

Texture TextureLoader::uploadTexture(unsigned char* data, int width, int height, int bytes) {
  Texture texture;
  GLuint handle;
  glGenTextures(1, &handle);
  glBindTexture(GL_TEXTURE_2D, handle);

  if(bytes == 3) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  }
  if(bytes == 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  }
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
  texture.handle = handle;
  return texture;
}

} /* namespace glPortal */
