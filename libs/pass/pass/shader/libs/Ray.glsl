#ifndef RAY_GLSL
#define RAY_GLSL

struct Ray{
  vec3 origin;
  vec3 direction;
};

Ray getRay(vec3 origin, vec3 target, bool invertDirection = false) {
  Ray ray;
  ray.origin = origin;
  ray.direction = invertDirection ? normalize(origin-target)
                                  : normalize(target-origin);
  return ray;
}

#endif // RAY_GLSL
