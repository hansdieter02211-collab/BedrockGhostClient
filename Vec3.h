#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3 operator-(const Vec3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    float Length() const {
        return sqrtf(x * x + y * y + z * z);
    }
};