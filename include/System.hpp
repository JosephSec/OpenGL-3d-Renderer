#pragma once

#include <vector>
#include <fstream>
#include <optional>
#include <SFML/System/Clock.hpp>

#include <System/Debug.hpp>
#include <System/Vec3.hpp>

#include <Engine/Raycast.hpp>


class System {
public:
  static string PATH;

  static sf::Clock timeClock;
  static float deltaTime;

  static std::vector<string> assets;
  
  static Raycast rayCastHit;


  static void init();
  static void update();
  static void end();


private:
  static void GenerateReleaseAssets() {
    std::ofstream file(PATH+"/assets/Release.txt");
    if(!file.is_open()) {
      Debug::error("Couldnt generate Release assets file");
      return;
    }

    for(int i = 0; i < assets.size(); i++) file << assets[i] << '\n';

    file.close();
  }
};


static Vec3 GetClosestPointToAxis(const Vec3& axisOrigin, const Vec3& axisDir, const Vec3& rayOrigin, const Vec3& rayDir) {
  Vec3 w0 = axisOrigin - rayOrigin;

  float a = Vec3::dot(axisDir, axisDir);
  float b = Vec3::dot(axisDir, rayDir);
  float c = Vec3::dot(rayDir, rayDir);
  float d = Vec3::dot(axisDir, w0);
  float e = Vec3::dot(rayDir, w0);

  float denom = a*c - b*b;
  if(fabs(denom) < 1e-6f) return axisOrigin;

  float t = (b*e - c*d) / denom;
  return axisOrigin + axisDir * t;
}