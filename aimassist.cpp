#include "Vec3.h"
#include <Windows.h>
#include <vector>
#include <cmath>

struct Entity {
    Vec3 position;
    bool isAlive;
    bool isEnemy;
};

struct Player {
    Vec3 position;
    float yaw;
    float pitch;
};

// Berechne Zielwinkel von Spieler zu Ziel
Vec3 CalcAngle(const Vec3& src, const Vec3& dst) {
    Vec3 delta = dst - src;
    float hyp = sqrtf(delta.x * delta.x + delta.z * delta.z);
    Vec3 angles;
    angles.x = -atan2f(delta.y, hyp) * (180.0f / 3.14159265f); // Pitch
    angles.y = atan2f(delta.z, delta.x) * (180.0f / 3.14159265f) - 90.0f; // Yaw
    return angles;
}

// Suche das beste Ziel
Entity* GetClosestTarget(const Player& player, const std::vector<Entity*>& entities, float maxFOV) {
    float bestFOV = maxFOV;
    Entity* bestTarget = nullptr;

    for (auto* ent : entities) {
        if (!ent->isAlive || !ent->isEnemy) continue;
        Vec3 angleToTarget = CalcAngle(player.position, ent->position);
        float yawDiff = angleToTarget.y - player.yaw;
        float pitchDiff = angleToTarget.x - player.pitch;
        float fov = sqrtf(yawDiff * yawDiff + pitchDiff * pitchDiff);
        if (fov < bestFOV) {
            bestFOV = fov;
            bestTarget = ent;
        }
    }
    return bestTarget;
}

// Sanftes Nachziehen der Kamera
void SmoothAim(Player& player, const Vec3& targetAngles, float smoothing) {
    player.yaw += (targetAngles.y - player.yaw) / smoothing;
    player.pitch += (targetAngles.x - player.pitch) / smoothing;
}