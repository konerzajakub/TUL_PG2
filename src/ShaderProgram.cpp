#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
	std::vector<GLuint> shader_ids;

    // compile shaders and store IDs for linker
	shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
	shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

	// link all compiled shaders into shader_program 
    ID = link_shader(shader_ids);
}

void ShaderProgram::setUniform(const std::string& name, const float val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const int val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4 in_vec4) {
    auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform4fv(loc, 1, glm::value_ptr(in_vec4));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4 val) {
    auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::getShaderInfoLog(const GLuint obj) {
    int log_len;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_len);
    std::string log(log_len, ' ');
    glGetShaderInfoLog(obj, log_len, &log_len, &log[0]);
    return log;
}

std::string ShaderProgram::getProgramInfoLog(const GLuint obj) {
    int log_len;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_len);
    std::string log(log_len, ' ');
    glGetProgramInfoLog(obj, log_len, &log_len, &log[0]);
    return log;
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, const GLenum type) {
	GLuint shader_h = glCreateShader(type); // 1. vytvoøení shader objektu

    std::string shader_string = textFileRead(source_file); // 2. naètení zdrojáku
    const char* shader_src = shader_string.c_str();
    glShaderSource(shader_h, 1, &shader_src, NULL); // 3. nastavení zdrojáku

    glCompileShader(shader_h);	// 4. kompilace shaderu

    GLint status;
    glGetShaderiv(shader_h, GL_COMPILE_STATUS, &status); // 5. kontrola statusu
    if (status == GL_FALSE) {
        std::string log = getShaderInfoLog(shader_h);	// 6. pokud chyba, získá log
        std::cerr << "Compile failure in shader: " << source_file << '\n' << log << '\n';
        throw std::runtime_error("Shader compilation failed."); // vyhození výjimky
    }

	return shader_h;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
	GLuint prog_h = glCreateProgram(); // 1. vytvoøí program objekt

	for (const auto id : shader_ids)
		glAttachShader(prog_h, id);		// 2. pøipojení všech shaderù

	glLinkProgram(prog_h);				// 3. linkuje program

    GLint status;
    glGetProgramiv(prog_h, GL_LINK_STATUS, &status); // 4. kontroluje status linkování
    if (status == GL_FALSE) {
        std::string log = getProgramInfoLog(prog_h);	// 5. pokuid chyba, získá log
        std::cerr << "Linker failure: " << log << '\n';
        throw std::runtime_error("Shader linking failed."); // 6. hodí výjimku
    }

    for (const auto id : shader_ids)
		glDeleteShader(id); // uvolnìní shaderù z pamìti

	return prog_h;	// vrátí handle na program
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filename) {
    std::cout << "Loading shader: " << filename.string() << std::endl; // debug print
	std::ifstream file(filename);
	if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename.string() << std::endl;
		throw std::runtime_error(std::string("Error opening file: ") + filename.string());
    }
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}