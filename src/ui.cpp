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
    
    ImGui::Spacing();
    ImGui::Spacing();

    Label("Sigma_t");
    ImGui::DragFloat("##Sigma_t", &m_sigma_t, 0.5f);
    m_sigma_t = glm::max(m_sigma_t, 1.0f);
    Label("Omega");
    ImGui::SliderFloat("##Omega", &m_omega, 0.0f, 2.0f);
    Label("Background color");
    ImGui::ColorEdit3("##Background color", &backgroundColor.r);

    ImGui::Spacing();
    ImGui::Spacing();

    Label("Light color");
    ImGui::ColorEdit3("##Light color", &m_lightColor.r);
    Label("Light intensity");
    ImGui::SliderFloat("##Light intensity", &m_lightIntensity, 0.0f, 10.0f);
    m_lightIntensity = glm::max(m_lightIntensity, 0.0f);

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
    glUniform1f(glGetUniformLocation(shaderProgramId, "stepSize"), m_stepSize);
    glUniform1i(glGetUniformLocation(shaderProgramId, "useNoise"), m_useNoise);
    glUniform1f(glGetUniformLocation(shaderProgramId, "sigma_t"), m_sigma_t);
    glUniform1f(glGetUniformLocation(shaderProgramId, "omega"), m_omega);
    glUniform3f(glGetUniformLocation(shaderProgramId, "backgroundColor"), backgroundColor.r, backgroundColor.g, backgroundColor.b);

    glUniform1f(glGetUniformLocation(shaderProgramId, "lightIntensity"), m_lightIntensity);
    glUniform3f(glGetUniformLocation(shaderProgramId, "lightColor"), m_lightColor.r, m_lightColor.g, m_lightColor.b);
}