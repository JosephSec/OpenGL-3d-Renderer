#include <Engine/Physics.hpp>

#include <Engine/Editor.hpp>
#include <Engine/SceneManager.hpp>


//public
float Physics::fixedDeltaTime = 1.0 / 50.0;

glm::vec3 Physics::gravity = glm::vec3(0,-9.806,0);
//public


//private
float Physics::fixedUpdateCountdown;
//private


void Physics::init() {
  fixedUpdateCountdown = fixedDeltaTime;
}
void Physics::update() {
  fixedUpdateCountdown -= Editor::deltaTime;
  while(fixedUpdateCountdown <= 0) {
    fixedUpdateCountdown += fixedDeltaTime;
    
    for(Rigidbody &rbA : SceneManager::scene.rigidbodys) {
      rbA.fixedUpdate();
      
      for(const Rigidbody &rbB : SceneManager::scene.rigidbodys) {
        if(&rbB == &rbA) continue;

        const float distance = glm::length(rbA.transform.position - rbB.transform.position);
        if(distance > (rbA.radius + rbB.radius + 1e-10f)) continue;

        const glm::vec3 a = rbA.velocity * (rbA.mass - rbB.mass) / (rbA.mass + rbB.mass);
        const glm::vec3 b = rbB.velocity * (2 * rbB.mass) / (rbA.mass + rbB.mass);
        rbA.velocity = a + b;
      }
    }
  }
}