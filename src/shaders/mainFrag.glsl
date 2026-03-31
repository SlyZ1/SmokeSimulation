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
uniform uint frame;

uniform float stepSize;
uniform bool useNoise;

#pragma include "./rand.glsl"
#pragma include "./intersections.glsl"
#pragma include "./smokeFrag.glsl"

#pragma FDECLARE
// RAND.GLSL
void initSeed(uvec2 pos, uint frame);
float rand(inout uint seed);

// INTERSECTIONS.GLSL
float intersectAABB(Ray ray, AABB box);
bool isInAABB(vec3 point, AABB box);
#pragma FEND

float beerLambert(float distance, float rho){
    return exp(-distance * rho);
}

float sampleConstantDensity(vec3 pos){
    return 0.5;
}

float sampleSphereDensity(vec3 pos){
    return pow(max(1 - length(pos), 0), 0.5) * 2;
}

float computeTransmittance(Ray ray, AABB box, inout uint seed){
    float transmittance = 1;
    while(isInAABB(ray.origin, box)){
        float noise = 0;
        if (useNoise) noise = (rand(seed) * 2 - 1) * stepSize;
        ray.origin += ray.dir * noise;

        if (isInAABB(ray.origin, box)){
            float rho = sampleConstantDensity(ray.origin);
            transmittance *= beerLambert(stepSize, rho);
        }

        ray.origin += ray.dir * stepSize;
    }
    return transmittance;
}

vec4 intersect(Ray ray, inout uint seed){
    AABB box = AABB(vec3(-1), vec3(1));
    float t = intersectAABB(ray, box);

    if (isInAABB(ray.origin, box))
        t = 0;

    if (t >= 0){
        ray.origin += ray.dir * (t + 1e-4);
        float transmittance = computeTransmittance(ray, box, seed);
        return (1 - transmittance) * vec4(1);
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
    uint seed = initSeed(uvec2(gl_FragCoord.xy), frame);

    vec2 pos = ratio(vClipPos.xy);
    Ray ray;
    ray.origin = camera.pos;
    ray.dir = camera.lookDir;
    ray = fovRay(pos, ray);

    FragColor = intersect(ray, seed);
}