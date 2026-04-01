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

struct NoiseData {
    float bnoise;
    uint seed;
};

uniform Camera camera;
uniform vec2 texSize;
uniform uint frame;

uniform float stepSize;
uniform bool useNoise;
uniform sampler2D blueNoise;

#pragma include "./rand.glsl"
#pragma include "./intersections.glsl"
#pragma include "./smokeFrag.glsl"

#pragma FDECLARE
// RAND.GLSL
void initSeed(uvec2 pos, uint frame);
float rand(inout uint seed);
float rand_bn(ivec2 pixel, int frame);

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

float computeTransmittance(Ray ray, AABB box, inout NoiseData nd){
    float transmittance = 1;
    float ss = stepSize;
    if (useNoise){
        ray.origin += ray.dir * nd.bnoise * ss;
    }
    while(isInAABB(ray.origin, box)){

        float rho = sampleConstantDensity(ray.origin);
        transmittance *= beerLambert(ss, rho);

        ray.origin += ray.dir * ss;
    }
    return transmittance;
}

vec4 intersect(Ray ray, inout NoiseData nd){
    AABB box = AABB(vec3(-1), vec3(1));
    float t = intersectAABB(ray, box);

    if (isInAABB(ray.origin, box))
        t = 0;

    if (t >= 0){
        ray.origin += ray.dir * (t + 1e-4);
        float transmittance = computeTransmittance(ray, box, nd);
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
    vec2 uv = (vClipPos.xy + vec2(1)) * 0.5;
    float bnoise = rand_bn(ivec2(ratio(uv) * texSize.y), int(frame));
    NoiseData nd = NoiseData(bnoise, seed);

    Ray ray;
    ray.origin = camera.pos;
    ray.dir = camera.lookDir;
    ray = fovRay(pos, ray);

    FragColor = intersect(ray, nd);
}