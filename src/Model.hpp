#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

// tøída pro model
class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    glm::mat4 model_matrix{1.0f};
    bool transparent = false;
    glm::vec4 diffuse_color = glm::vec4(1.0f);

    Model() = default;

    // naètení meshes
    Model(const std::filesystem::path & filename) {
        name = filename.stem().string();

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        
        // naètení OBJ
        if (!loadOBJ(filename.string().c_str(), positions, uvs, normals)) {
            std::cerr << "Failed to load model: " << filename << std::endl;
            return;
        }

        std::vector<vertex> vertices;
        for (size_t i = 0; i < positions.size(); ++i) {
            vertices.push_back({positions[i], normals[i], uvs[i]});
        }

        std::vector<GLuint> indices;
        for (size_t i = 0; i < positions.size(); ++i) {
            indices.push_back(static_cast<GLuint>(i));
        }

        meshes.emplace_back(GL_TRIANGLES, vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
    }

    // update position etc. based on running time
    void update(const float delta_t) {
        // origin += glm::vec3(3,0,0) * delta_t; // s = s0 + v*dt
    }

    void draw(ShaderProgram& shader) {
        // call draw() on mesh (all meshes)
        for (auto const& mesh : meshes) {
            mesh.draw(shader, model_matrix);
        }
    }

    void setTexture(GLuint texture_id) {
        for (auto& mesh : meshes) {
            mesh.texture_id = texture_id;
        }
    }

    void clear() {
        for (auto& mesh : meshes) {
            mesh.clear();
        }
        meshes.clear();
    }

    void translate(const glm::vec3& v) {
		model_matrix = glm::translate(model_matrix, v);
	}

	void rotate(float angle_rad, const glm::vec3& axis) {
		model_matrix = glm::rotate(model_matrix, angle_rad, axis);
	}

	void scale(const glm::vec3& v) {
		model_matrix = glm::scale(model_matrix, v);
	}

	void setMatrix(const glm::mat4& m) {
		model_matrix = m;
	}

	const glm::mat4& getMatrix() const {
		return model_matrix;
	}
};