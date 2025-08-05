#pragma once
inline bool g_AimAssistEnabled = false;

void RenderGUI() {
    ImGui::Begin("Ghost Client");

    ImGui::Text("Bedrock Hack Client");
    ImGui::Checkbox("Aim Assist", &g_AimAssistEnabled);
    ImGui::Text("F6 = Toggle GUI");

    ImGui::End();
}