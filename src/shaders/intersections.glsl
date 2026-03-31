bool intersectAABB(Ray ray, AABB box)
{
    float tmin = 0;
    float tmax = 1e20;
    
    for (int i = 0; i < 3; i++) {
        if (abs(ray.dir[i]) < 1e-8f) {
            continue;
        }
        
        float invD = 1 / ray.dir[i];
        float t0 = (box.min[i] - ray.origin[i]) * invD;
        float t1 = (box.max[i] - ray.origin[i]) * invD;
        
        if (invD < 0.0f) {
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }
        
        tmin = max(tmin, t0);
        tmax = min(tmax, t1);
        
        if (tmax < tmin) {
            return false;
        }
    }
    
    return true;
}