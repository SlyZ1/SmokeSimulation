#include "ui.hpp"

void UI::toggle(){
    m_show = !m_show;
}

void UI::Label(const char* label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
}

void UI::BeginTwoColumnLayout()
{
    ImGui::BeginTable("##layout", 2, ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100);
    ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
}

void UI::EndTwoColumnLayout()
{
    ImGui::EndTable();
}

void UI::renderApp(){

}

void UI::renderRender(){
    BeginTwoColumnLayout();

    Label("Step size");
    ImGui::SliderFloat("##Step size", &m_stepSize, 0.001f, .4f);
    Label("Use noise");
    ImGui::Checkbox("##Use noise", &m_useNoise);

    EndTwoColumnLayout();
}

void UI::renderSimulation(){

}

void UI::render(){
    if (!m_show) return;

    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_MenuBar;
    ImGui::Begin("Parameters", (bool*)NULL, flags);

    if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("    App    "))
        {
            renderApp();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("   Render   "))
        {
            renderRender();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(" Simulation "))
        {
            renderSimulation();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UI::updateGPU(GLuint shaderProgramId){
    int stepSizeLoc = glGetUniformLocation(shaderProgramId, "stepSize");
    glUniform1f(stepSizeLoc, m_stepSize);
    int useNoiseLoc = glGetUniformLocation(shaderProgramId, "useNoise");
    glUniform1i(useNoiseLoc, m_useNoise);
}