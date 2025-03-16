#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

// set uniform according to name 
// https://docs.gl/gl4/glUniform

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
	std::vector<GLuint> shader_ids;

    // compile shaders and store IDs for linker
	shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
	shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

	// link all compiled shaders into shader_program 
    ID = link_shader(shader_ids);
}

// Get the shader program ID
GLuint ShaderProgram::getID() const {
	return ID;
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
	GLint log_length = 0;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		std::string log(log_length, '\0');
		glGetShaderInfoLog(obj, log_length, nullptr, &log[0]);
		return log;
	}
	return "";
}

std::string ShaderProgram::getProgramInfoLog(const GLuint obj) {
	GLint log_length = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		std::string log(log_length, '\0');
		glGetProgramInfoLog(obj, log_length, nullptr, &log[0]);
		return log;
	}
	return "";
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, const GLenum type) {
	GLuint shader_h = glCreateShader(type);
	std::string source_code = textFileRead(source_file);
	const char* source_code_ptr = source_code.c_str();
	glShaderSource(shader_h, 1, &source_code_ptr, nullptr);
	glCompileShader(shader_h);

	GLint compile_status = 0;
	glGetShaderiv(shader_h, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE) {
		std::string log = getShaderInfoLog(shader_h);
		glDeleteShader(shader_h);
		throw std::runtime_error("Shader compilation failed: " + log);
	}

	return shader_h;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
	GLuint prog_h = glCreateProgram();

	for (const auto& id : shader_ids)
		glAttachShader(prog_h, id);

	glLinkProgram(prog_h);

	GLint link_status = 0;
	glGetProgramiv(prog_h, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE) {
		std::string log = getProgramInfoLog(prog_h);
		glDeleteProgram(prog_h);
		throw std::runtime_error("Shader program linking failed: " + log);
	}

	for (const auto& id : shader_ids) {
		glDetachShader(prog_h, id);
		glDeleteShader(id);
	}

	return prog_h;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filename) {
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error(std::string("Error opening file: ") + filename.string());
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
