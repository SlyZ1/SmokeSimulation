#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "app.hpp"
#include "camera.hpp"
#include "shader_program.hpp"

using namespace std;

int frameCount = 0;
unsigned int VBO, VAO, EBO;
ShaderProgram shaderProg;
Camera camera(0.1, 0.3);
App app;

string getShaderSource(const char *filepath){
    ifstream file(filepath);

    if (!file.is_open()) {
        cerr << "Erreur: impossible d'ouvrir le fichier " << filepath << endl;
        return "";
    }

    stringstream shaderText;
    shaderText << file.rdbuf(); 
    return shaderText.str();
}

void init(){
    app.init(800, 600, "Smoke Simulation");
    app.toggleCursor(false);

    shaderProg.create();
    shaderProg.load(GL_VERTEX_SHADER, "src/shaders/mainVertex.glsl");
    shaderProg.load(GL_FRAGMENT_SHADER, "src/shaders/mainFrag.glsl");
    shaderProg.link();

    vector<float> vertices = {
        1.f,  1.f, 0.0f,
        1.f, -1.f, 0.0f,
        -1.f, -1.f, 0.0f,
        -1.f,  1.f, 0.0f
    };
    vector<unsigned int> indices = {
        0, 1, 3,
        1, 2, 3
    };  
    tie(VBO, VAO, EBO) = ShaderProgram::addData(vertices, indices);
    ShaderProgram::linkData(3, sizeof(float), 0);
}

void handleCamera(){
    if (!app.cursorIsHidden()){
        camera.hasStoppedMoving();
        return;
    }

    camera.move(
        app.keyPressed(GLFW_KEY_W),
        app.keyPressed(GLFW_KEY_S),
        app.keyPressed(GLFW_KEY_D),
        app.keyPressed(GLFW_KEY_A),
        app.keyPressed(GLFW_KEY_SPACE),
        app.keyPressed(GLFW_KEY_LEFT_CONTROL),
        app.keyPressed(GLFW_KEY_LEFT_SHIFT)
    );
    camera.rotate(app.mouseX(), app.mouseY());
}

void render(){
    GLuint camPosLoc = glGetUniformLocation(shaderProg.id(), "camera.pos");
    GLuint camLookLoc = glGetUniformLocation(shaderProg.id(), "camera.lookDir");
    glUniform3f(camPosLoc, camera.position().x, camera.position().y, camera.position().z);
    glUniform3f(camLookLoc, camera.lookDir().x, camera.lookDir().y, camera.lookDir().z);

    GLuint texSizeLoc = glGetUniformLocation(shaderProg.id(), "texSize");
    glUniform2f(texSizeLoc, 800, 600);

    shaderProg.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void inputs(){
    if (app.keyPressedOnce(GLFW_KEY_P, frameCount)){
        app.toggleCursor(app.cursorIsHidden());
    }
}

void end(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shaderProg.destroy();
    app.terminate();
}

int main(){
    init();
    while(!app.shouldClose())
    {
        app.startFrame(frameCount);

        handleCamera();
        render();
        inputs();

        frameCount++;
        app.endFrame();
    }
    end();
    return EXIT_SUCCESS;
}