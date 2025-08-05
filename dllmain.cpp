#include <Windows.h>
#include <thread>
#include "Vec3.h"

// Stubs für Speicher-Adressen → DU MUSST DIESE FINDEN
uintptr_t baseAddress = (uintptr_t)GetModuleHandle(NULL);
uintptr_t localPlayerPtr = 0x12345678;  // Beispieladresse – anpassen!
uintptr_t entityListPtr = 0x87654321;   // Beispieladresse – anpassen!

// Lies Daten aus dem Spielspeicher (direkt, da internal)
template<typename T>
T Read(uintptr_t address) {
    return *(T*)address;
}

template<typename T>
void Write(uintptr_t address, T value) {
    *(T*)address = value;
}

void AimAssistMain() {
    while (true) {
        if (GetAsyncKeyState(VK_LBUTTON) & 1) {
            // Lies LocalPlayer Daten
            Player player;
            player.position = Read<Vec3>(localPlayerPtr + 0x10);  // Beispieloffset
            player.yaw = Read<float>(localPlayerPtr + 0x20);       // Beispieloffset
            player.pitch = Read<float>(localPlayerPtr + 0x24);     // Beispieloffset

            // Lies EntityList
            std::vector<Entity*> entities;
            for (int i = 0; i < 64; i++) {
                uintptr_t entityBase = Read<uintptr_t>(entityListPtr + i * 0x8);
                if (!entityBase) continue;

                Entity* e = new Entity();
                e->position = Read<Vec3>(entityBase + 0x10);  // Beispieloffset
                e->isAlive = Read<bool>(entityBase + 0x30);
                e->isEnemy = Read<bool>(entityBase + 0x34);
                entities.push_back(e);
            }

            // Ziel finden
            Entity* target = GetClosestTarget(player, entities, 30.0f); // FOV

            if (target) {
                Vec3 targetAngles = CalcAngle(player.position, target->position);
                SmoothAim(player, targetAngles, 6.0f); // Smoothing

                // Schreibe neue Blickrichtung
                Write<float>(localPlayerPtr + 0x20, player.yaw);
                Write<float>(localPlayerPtr + 0x24, player.pitch);
            }

            for (auto e : entities) delete e;
        }

        Sleep(10);
    }
}

DWORD WINAPI MainThread(HMODULE hModule) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    std::cout << "[+] AimAssist injected.\n";

    std::thread(AimAssistMain).detach();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}