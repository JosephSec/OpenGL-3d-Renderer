#include <Engine/Raycast.hpp>
#include <System.hpp>
#include <Renderer.hpp>


struct Triangle {
  Vec3 a, b, c;
};
std::optional<Vec3> TriangleIntersect(const Vec3 &rayOrigin, const Vec3 &rayDir, const Triangle& triangle) {
  constexpr float epsilon = std::numeric_limits<float>::epsilon();

  Vec3 edge1 = triangle.b - triangle.a;
  Vec3 edge2 = triangle.c - triangle.a;
  Vec3 ray_cross_e2 = Vec3::cross(rayDir, edge2);
  float det = Vec3::dot(edge1, ray_cross_e2);

  if(det > -epsilon && det < epsilon) return {};

  float inv_det = 1.0 / det;
  Vec3 s = rayOrigin - triangle.a;
  float u = inv_det * Vec3::dot(s, ray_cross_e2);

  if((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u-1) > epsilon)) return {};

  Vec3 s_cross_e1 = Vec3::cross(s, edge1);
  float v = inv_det * Vec3::dot(rayDir, s_cross_e1);

  if((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon)) return {};

  float t = inv_det * Vec3::dot(edge2, s_cross_e1);

  if(t > epsilon) return std::optional<Vec3>(Vec3(rayOrigin + rayDir * t));
  return {};
}



std::map<float, GameObject*> GetGameObjectsByBounds(const Raycast* rayCastHit) {
  std::map<float, GameObject*> value;
  
  float closest = -INFINITY;
  for(GameObject& object : Renderer::objects) {
    if(object.mesh == nullptr) continue;

    closest = -INFINITY;
    for(const Vec3& vert : object.mesh->vertices) {
      const float dist = vert.magnitude();
      if(dist > closest) closest = dist;
    }

    Vec3 L = object.transform.position - rayCastHit->origin;
    float tca = Vec3::dot(L, rayCastHit->direction);
    float d2 = Vec3::dot(L, L) - tca * tca;
    float r2 = closest * closest;

    if (d2 > r2) continue; //Missed mesh bounds
    value.insert({L.magnitude(), &object});
  }
  return value;
}


Raycast::Raycast(const Vec3& _origin, const Vec3& _direction) : origin(_origin), direction(_direction) {
  update();
}

void Raycast::update() {
  std::map<float, GameObject*> objects = GetGameObjectsByBounds(this);
  std::map<float, Vec3> intersectMap;
  for(const auto& [curObjectDist, curObject] : objects) {
    const Mesh* curMesh = curObject->mesh;
    const unsigned int faceC = curMesh->indecies.size()/3;
    for(int i = 0; i < faceC; i++) {
      const unsigned int triangleStart = i*3;
      Triangle triangle {
        ApplyTransform(curMesh->vertices[curMesh->indecies[triangleStart]], curObject->transform, false),
        ApplyTransform(curMesh->vertices[curMesh->indecies[triangleStart+1]], curObject->transform, false),
        ApplyTransform(curMesh->vertices[curMesh->indecies[triangleStart+2]], curObject->transform, false)
      };
      
      const std::optional<Vec3> intersect = TriangleIntersect(origin, direction, triangle);
      if(intersect) intersectMap.insert({(*intersect - origin).magnitude(), *intersect});
    }

    if(intersectMap.size() == 0) continue; //Ray missed curObject

    object = curObject;

    for(const auto& [intersectDist, intersectPoint] : intersectMap)
      intersects.push_back(intersectPoint);

    point = intersects[0];
    return;
  }

  object = nullptr;
  intersects.clear();
  point = Vec3zero;
}



#include <Engine/TransformGizmo.hpp>

std::map<float, Gizmo*> GetGizmosByBounds(const Editorcast* editorCastHit) {
  std::map<float, Gizmo*> value;
  
  float closest = -INFINITY;
  for(Gizmo& gizmo : Gizmos::gizmos) {
    closest = -INFINITY;
    for(const Vec3& vert : gizmo.mesh.vertices) {
      const float dist = vert.magnitude();
      if(dist > closest) closest = dist;
    }

    Vec3 L = gizmo.transform.position - editorCastHit->origin;
    float tca = Vec3::dot(L, editorCastHit->direction);
    float d2 = Vec3::dot(L, L) - tca * tca;
    float r2 = closest * closest;

    if (d2 > r2) continue; //Missed mesh bounds
    value.insert({L.magnitude(), &gizmo});
  }
  return value;
}


Editorcast::Editorcast(const Vec3& _origin, const Vec3& _direction) : origin(_origin), direction(_direction) {
  update();
}

void Editorcast::update() {
  std::map<float, Gizmo*> gizmos = GetGizmosByBounds(this);
  std::map<float, std::pair<Vec3, unsigned int>> intersectMap;
  for(const auto& [curObjectDist, curGizmo] : gizmos) {
    const Mesh& curMesh = curGizmo->mesh;
    const unsigned int faceC = curMesh.indecies.size()/3;
    for(int i = 0; i < faceC; i++) {
      const unsigned int triangleStart = i*3;
      Triangle triangle {
        ApplyTransform(curMesh.vertices[curMesh.indecies[triangleStart]], curGizmo->transform, false),
        ApplyTransform(curMesh.vertices[curMesh.indecies[triangleStart+1]], curGizmo->transform, false),
        ApplyTransform(curMesh.vertices[curMesh.indecies[triangleStart+2]], curGizmo->transform, false)
      };
      
      const std::optional<Vec3> intersect = TriangleIntersect(origin, direction, triangle);
      if(intersect) intersectMap.insert({(*intersect - origin).magnitude(), {*intersect, i}});
    }

    if(intersectMap.size() == 0) continue; //Ray missed curObject

    gizmo = curGizmo;
    point = intersectMap.begin()->second.first;
    faceIndex = intersectMap.begin()->second.second;
    return;
  }

  gizmo = nullptr;

  point = Vec3zero;
  faceIndex = -1;
}