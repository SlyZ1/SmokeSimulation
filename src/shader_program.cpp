#include "shader_program.hpp"

ShaderProgram::ShaderProgram() : m_shaderProgram(0) { }

void ShaderProgram::create(){
    m_shaderProgram = glCreateProgram();
}

// Extracts path from lines like : #pragma include "path"
fs::path ShaderProgram::extractPath(const string& line){
    int start = line.find_first_of("\"") + 1;
    int end = line.find_last_of("\"");
    return fs::path(line.substr(start, end - start));
}

string ShaderProgram::getShaderSource(const char* path){
    ifstream file(path);
    stringstream ss;
    string line;

    if (!file.is_open()) {
        cerr << "Erreur: impossible d'ouvrir le fichier " << path << endl;
        return "";
    }

    bool isForwardDeclaration = false;

    while (getline(file, line)) {
        // Includes
        if (line.find("#pragma include") != std::string::npos) {
            fs::path includePath = fs::path(path).parent_path() / extractPath(line);
            ss << getShaderSource(includePath.string().c_str());
        // Forward declarations
        } else if (line == "#pragma FDECLARE") {
            isForwardDeclaration = true;
        } else if (line == "#pragma FEND") {
            isForwardDeclaration = false;
        // Normal lines
        } else {
            if (!isForwardDeclaration) ss << line << "\n";
        }
    }

    return ss.str();
}

void ShaderProgram::load(int type, const char *path){
    m_types.push_back(type);
    m_paths.push_back(path);

    //Create shader
    unsigned int shader;
    shader = glCreateShader(type);
    m_shaders.push_back(shader);
    
    //Compile shader
    string shaderSourceString = getShaderSource(path); 
    const char *shaderSource = shaderSourceString.c_str();
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    //Check for errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //Add to program
    glAttachShader(m_shaderProgram, shader);
}

void ShaderProgram::reload(){
    destroy();

    create();
    for (size_t i = 0; i < m_paths.size(); i++)
    {
        //Create shader
        unsigned int shader;
        shader = glCreateShader(m_types[i]);
        m_shaders.push_back(shader);
        
        //Compile shader
        string shaderSourceString = getShaderSource(m_paths[i]); 
        const char *shaderSource = shaderSourceString.c_str();
        glShaderSource(shader, 1, &shaderSource, NULL);
        glCompileShader(shader);

        //Check for errors
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
        }

        //Add to program
        glAttachShader(m_shaderProgram, shader);
    }
    link();
}

void ShaderProgram::link(){
    //Link
    glLinkProgram(m_shaderProgram);

    //Check for errors
    int success;
    char infoLog[512];
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        cerr << "ShaderProgram linking failed : " << infoLog << endl;
    }

    GLint status;
    glValidateProgram(m_shaderProgram);
    glGetProgramiv(m_shaderProgram, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        char log[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, log);
        std::cerr << "Program invalid: " << log << std::endl;
    }
    
    //Delete shaders
    for(const int shader : m_shaders){
        glDeleteShader(shader);
    }
    m_shaders.clear();
}

void ShaderProgram::use(){
    glUseProgram(m_shaderProgram);
}

void ShaderProgram::dispatch(GLuint x, GLuint y, GLuint z){
    glDispatchCompute(x, y, z);
}

void ShaderProgram::barrier(GLbitfield memBarrier){
    glMemoryBarrier(memBarrier);
}

void ShaderProgram::destroy(){
    glDeleteProgram(m_shaderProgram);
}

unsigned int ShaderProgram::id(){
    return m_shaderProgram;
}