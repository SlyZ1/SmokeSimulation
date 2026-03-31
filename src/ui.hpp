#include "app.hpp"

class UI {
    private:
        bool m_show = false;
        App m_app = {};

        // App

        // Render
        float m_stepSize = 0.1;
        bool m_useNoise = true;

        // Simulation

        void renderApp();
        void renderRender();
        void renderSimulation();
        void Label(const char* label);
        void BeginTwoColumnLayout();
        void EndTwoColumnLayout();

    public:
        UI() = default;
        UI(App app) : m_app(app) { 
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(300, m_app.getIo()->DisplaySize.y));
        };

        void toggle();
        void render();
        void updateGPU(GLuint shaderProgramId);
};