#version 430 core
out vec4 FragColor;
in vec4 vClipPos;

struct Camera {
    vec3 pos;
    vec3 lookDir;
};

struct AABB {
    vec3 min;
    vec3 max;
};

struct Ray {
    vec3 origin;
    vec3 dir;
};

uniform Camera camera;
uniform vec2 texSize;

#pragma include "./intersections.glsl"
#pragma include "./smokeFrag.glsl"
#pragma FDECLARE
// INTERSECTIONS.GLSL
bool intersectAABB(Ray ray, AABB box);
#pragma FEND

// current code
vec4 intersect(Ray ray){
    
    AABB box = AABB(vec3(-1), vec3(1));
    if (intersectAABB(ray, box)){
        return vec4(1);
    }
    return vec4(0);
}

Ray fovRay(vec2 pos, Ray ray){
    float fov = radians(mix(50, 90, 0 / 8.0));
    vec3 forward = normalize(camera.lookDir);

    vec3 worldUp = abs(forward.y) < 0.999
                 ? vec3(0,1,0)
                 : vec3(0,0,1);

    vec3 right = normalize(cross(forward, worldUp));
    vec3 up    = cross(right, forward);

    float tanHalfFov = tan(fov * 0.5);

    vec3 dir = forward + (right * pos.x + up * pos.y) * tanHalfFov;
    ray.dir = normalize(dir);
    return ray;
}

vec2 ratio(vec2 vec){
    return vec2(vec.x * texSize.x / texSize.y, vec.y);
}

void main()
{
    vec2 pos = ratio(vClipPos.xy);
    Ray ray;
    ray.origin = camera.pos;
    ray.dir = camera.lookDir;
    ray = fovRay(pos, ray);

    FragColor = intersect(ray);
}