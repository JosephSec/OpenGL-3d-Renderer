#pragma once

#include <vector>
#include <fstream>
#include <System/Debug.hpp>
#include <SFML/System/Clock.hpp>


class System {
public:
  static string PATH;

  static sf::Clock timeClock;
  static float deltaTime;

  static std::vector<string> assets;


  static void init();
  static void update();


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