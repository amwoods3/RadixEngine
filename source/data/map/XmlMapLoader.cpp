#include <radix/data/map/XmlMapLoader.hpp>

#include <iostream>
#include <vector>

#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>

#include <radix/data/map/XmlHelper.hpp>
#include <radix/data/map/XmlTriggerHelper.hpp>
#include <radix/env/Environment.hpp>

#include <radix/entities/Trigger.hpp>
#include <radix/entities/Player.hpp>
#include <radix/entities/StaticModel.hpp>
#include <radix/entities/LightSource.hpp>
#include <radix/entities/LiquidVolume.hpp>
#include <radix/entities/ReferencePoint.hpp>

#include <radix/World.hpp>
#include <radix/data/model/MeshLoader.hpp>
#include <radix/data/material/MaterialLoader.hpp>

using namespace std;

namespace radix {

XmlMapLoader::XmlMapLoader(World &w, const std::list<CustomTrigger>& customTriggers) :
  MapLoader(w),
  rootHandle(nullptr),
  customTriggers(customTriggers) {
}

void XmlMapLoader::load(const std::string &path) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError error = doc.LoadFile(path.c_str());

  if (error == 0) {
    tinyxml2::XMLHandle docHandle(&doc);
    tinyxml2::XMLElement *element = docHandle.FirstChildElement().ToElement();
    rootHandle = tinyxml2::XMLHandle(element);

    extractMaterials();
    extractSpawn();
    extractDoor();
    extractModels();
    extractLights();
    extractWalls();
    extractAcids();
    extractDestinations();
    extractTriggers();
    Util::Log(Info, "XmlMapLoader") << "Map " << path << " loaded";
  } else {
    Util::Log(Error, "XmlMapLoader") << "Failed to load map " << Environment::getDataDir()
                       << "/" << path << ".xml";
  }
}

void XmlMapLoader::extractMaterials() {
  tinyxml2::XMLElement *matIdxElm = rootHandle.FirstChildElement("materials").ToElement();

  if (matIdxElm) {
    tinyxml2::XMLElement *matElm = matIdxElm->FirstChildElement("material");
    if (matElm) {
      do {
        int mid = -1;
        matElm->QueryIntAttribute("id", &mid);
        if (mid == -1) {
          Util::Log(Error) << "Invalid Material ID in map.";
          continue;
        }
        std::string name = matElm->Attribute("name");
        if (name.length() > 0) {
          world.materials[mid] = MaterialLoader::getMaterial(name);
        } else {
          Util::Log(Error) << "Name is mandatory for material tag.";
          continue;
        }
      } while ((matElm = matElm->NextSiblingElement("material")) != nullptr);
    }
  }
}

void XmlMapLoader::extractSpawn() {
  tinyxml2::XMLElement *spawnElement = rootHandle.FirstChildElement("spawn").ToElement();

  if (spawnElement) {
    entities::ReferencePoint &start = world.entityManager.create<entities::ReferencePoint>();
    Vector3f position;
    XmlHelper::extractPosition(spawnElement, position);
    start.setPosition(position);
    entities::Player &player = world.getPlayer();
    XmlHelper::extractRotation(spawnElement, player.headAngle);
    player.setPosition(position);
  } else {
    throw std::runtime_error("No spawn position defined.");
  }
}

/**
 * Extract light element containing position (x, y, z) and colour (r, g, b) attributes
 */
void XmlMapLoader::extractLights() {
  Vector3f lightPos;
  Vector3f lightColor;
  float distance, energy, specular;
  tinyxml2::XMLElement* lightElement = rootHandle.FirstChildElement("light").ToElement();

  if (lightElement) {
    do {
      XmlHelper::extractPosition(lightElement, lightPos);
      XmlHelper::extractColor(lightElement, lightColor);

      lightElement->QueryFloatAttribute("distance", &distance);
      lightElement->QueryFloatAttribute("energy", &energy);
      if (lightElement->QueryFloatAttribute("specular", &specular) == tinyxml2::XML_NO_ATTRIBUTE) {
        specular = 0;
      }

      entities::LightSource &light = world.entityManager.create<entities::LightSource>();
      light.setPosition(lightPos);
      light.color = lightColor;
      light.distance = distance;
      light.energy = energy;
      light.specular = specular;
    } while ((lightElement = lightElement->NextSiblingElement("light")) != nullptr);
  }
}

void XmlMapLoader::extractDoor() {
  tinyxml2::XMLElement *endElement = rootHandle.FirstChildElement("end").ToElement();

  if (endElement) {
    entities::StaticModel &door = world.entityManager.create<entities::StaticModel>();
    Vector3f position;
    XmlHelper::extractPosition(endElement, position);
    door.setPosition(position);
    Vector3f angles;
    XmlHelper::extractRotation(endElement, angles);
    door.setOrientation(Quaternion().fromAero(angles));
    door.material = MaterialLoader::loadFromXML("door/door");
    door.mesh = MeshLoader::getMesh("Door.obj");
  }
}

void XmlMapLoader::extractWalls() {
  tinyxml2::XMLElement *wallBoxElement = rootHandle.FirstChildElement("wall").ToElement();

  if (wallBoxElement) {
    do {
      entities::StaticModel &wall = world.entityManager.create<entities::StaticModel>();

      Vector3f position;
      XmlHelper::extractPosition(wallBoxElement, position);
      wall.setPosition(position);
      Vector3f angles;
      XmlHelper::extractRotation(wallBoxElement, angles);
      wall.setOrientation(Quaternion().fromAero(angles));
      Vector3f scale;
      XmlHelper::extractScale(wallBoxElement, scale);
      wall.setScale(scale);

      int mid = -1;
      wallBoxElement->QueryIntAttribute("material", &mid);
      wall.material = world.materials[mid];
      wall.material.scaleU = wall.material.scaleV = 2.f;
      wall.mesh = MeshLoader::getPortalBox(wall);
      wall.setRigidBody(0, std::make_shared<btBoxShape>(static_cast<btVector3>(scale/2)));
    } while ((wallBoxElement = wallBoxElement->NextSiblingElement("wall")) != nullptr);
  }
}

void XmlMapLoader::extractAcids() {
  tinyxml2::XMLElement *acidElement = rootHandle.FirstChildElement("acid").ToElement();

  if (acidElement) {
    do {
      entities::LiquidVolume &acid = world.entityManager.create<entities::LiquidVolume>();

      Vector3f position;
      XmlHelper::extractPosition(acidElement, position);
      acid.setPosition(position);
      Vector3f angles;
      XmlHelper::extractRotation(acidElement, angles);
      acid.setOrientation(Quaternion().fromAero(angles));
      Vector3f scale;
      XmlHelper::extractScale(acidElement, scale);
      acid.setScale(scale);

      acid.material = MaterialLoader::loadFromXML("fluid/acid00");
      acid.mesh = MeshLoader::getPortalBox(acid);
    } while ((acidElement = acidElement->NextSiblingElement("acid")) != nullptr);
  }
}

void XmlMapLoader::extractDestinations() {
  tinyxml2::XMLElement *destinationElement = rootHandle.FirstChildElement("destination")
    .ToElement();

  if (destinationElement) {
    do {
      Destination destination;
      XmlHelper::extractPosition(destinationElement, destination.position);
      XmlHelper::extractRotation(destinationElement, destination.rotation);
      std::string name = XmlHelper::extractStringMandatoryAttribute(destinationElement, "name");

      world.destinations.insert(std::make_pair(name, destination));
    } while ((destinationElement = destinationElement->NextSiblingElement("destination"))
             != nullptr);
  }
}

void XmlMapLoader::extractTriggers() {
  tinyxml2::XMLElement *triggerElement = rootHandle.FirstChildElement("trigger").ToElement();

  if (triggerElement) {
    do {
      //! [Creating an Entity.]
      entities::Trigger &trigger = world.entityManager.create<entities::Trigger>();

      //! [Creating an Entity.]
      Vector3f position;
      XmlHelper::extractPosition(triggerElement, position);
      trigger.setPosition(position);
      Vector3f angles;
      XmlHelper::extractRotation(triggerElement, angles);
      trigger.setOrientation(Quaternion().fromAero(angles));
      Vector3f scale;
      XmlHelper::extractScale(triggerElement, scale);
      trigger.setScale(scale);
      XmlTriggerHelper::extractTriggerActions(trigger, triggerElement, customTriggers);

    } while ((triggerElement = triggerElement->NextSiblingElement("trigger")) != nullptr);
  }
}

void XmlMapLoader::extractModels() {
  int mid = -1;
  string mesh("none");
  tinyxml2::XMLElement *modelElement = rootHandle.FirstChildElement("model").ToElement();
  if (modelElement){
    do {
      modelElement->QueryIntAttribute("material", &mid);
      mesh = modelElement->Attribute("mesh");

      entities::StaticModel &model = world.entityManager.create<entities::StaticModel>();
      Vector3f position;
      XmlHelper::extractPosition(modelElement, position);
      model.setPosition(position);
      Vector3f angles;
      XmlHelper::extractRotation(modelElement, angles);
      model.setOrientation(Quaternion().fromAero(angles));
      model.material = world.materials[mid];
      model.mesh = MeshLoader::getMesh(mesh);
    } while ((modelElement = modelElement->NextSiblingElement("model")) != nullptr);
  }
}
} /* namespace radix */
