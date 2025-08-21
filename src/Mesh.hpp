// Mesh.hpp

#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "assets.hpp"
#include "ShaderProgram.hpp"
        
// t��da pro Mesh
class Mesh {
public:
    // mesh data
    std::vector<vertex> vertices;   // pole vrchol� (vertex�)
    std::vector<GLuint> indices;    // pole index� (jak� vrcholy tvo�� troj�heln�ky)
    glm::vec3 origin{};             // lok�ln� origin point meshe
    glm::vec3 orientation{};        // lok�ln� orientace (nato�en�) mesh
    
    GLuint texture_id{0};               // ID textury v pam�ti (automaticky texture id=0 => ��dn� textura)
    GLenum primitive_type = GL_POINT;   // typ "primitiva" (GL_TRIANGLES, GL_LINES, GL_POINTS)
    
    // mesh material
    glm::vec4 ambient_material{1.0f};   // (barva ve st�nu)
    glm::vec4 diffuse_material{1.0f};   // (hlavn� barva)
    glm::vec4 specular_material{1.0f};  // (barva lesku)
    float reflectivity{1.0f};           // odleskovost
    
    // indexovan� vykreslen� (vkl�d�n� do VRAM)
	Mesh(GLenum primitive_type, std::vector<vertex> const & vertices, std::vector<GLuint> const & indices, glm::vec3 const & origin, glm::vec3 const & orientation, GLuint const texture_id = 0):
        primitive_type(primitive_type),
        vertices(vertices),
        indices(indices),
        origin(origin),
        orientation(orientation),
        texture_id(texture_id)
    {
        // 1. Vytvo�en� VAO (Vertex Array Object)
        glCreateVertexArrays(1, &VAO);

        // 2. Napln�n� VBO daty z "vertices" (Vertex Buffer Object)
        glCreateBuffers(1, &VBO);
        glNamedBufferStorage(VBO, this->vertices.size() * sizeof(vertex), this->vertices.data(), 0);

        // 3. Napln�n� EBO daty (Element Buffer Object)
        glCreateBuffers(1, &EBO);
        glNamedBufferStorage(EBO, this->indices.size() * sizeof(GLuint), this->indices.data(), 0);

        // 4. Propojen� VBO, EBO s VAO
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(vertex));
        glVertexArrayElementBuffer(VAO, EBO);

        // 5. Nastaven� form�tu vertex atribut�
        glEnableVertexArrayAttrib(VAO, 0);
        glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position)); // vertex
        glVertexArrayAttribBinding(VAO, 0, 0);

        glEnableVertexArrayAttrib(VAO, 1);
        glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, normal)); // norm�la
        glVertexArrayAttribBinding(VAO, 1, 0);

        glEnableVertexArrayAttrib(VAO, 2);
        glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texCoords)); // text. sou�adnice
        glVertexArrayAttribBinding(VAO, 2, 0);
    };

    
    // GPU vykreslen�
    void draw(ShaderProgram &shader, const glm::mat4& model_matrix) const {
 		if (VAO == 0) {
			std::cerr << "VAO not initialized!\n";
			return;
		}
 
        shader.setUniform("uM_m", model_matrix); // posl�n� matice modelu do shaderz
        
        if (texture_id != 0) {
            glBindTextureUnit(0, texture_id);   // pokud m� mesh texturu -> p�ipojit
        }

        glBindVertexArray(VAO); // pro vykreslen� pou��t vertex data (VAO)
        glDrawElements(primitive_type, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)0); // vykreslen�
        glBindVertexArray(0);   // odpojen�
    }

    // uvoln�n� prost�edk�
	void clear(void) {
        texture_id = 0;
        primitive_type = GL_POINT;
        
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);

        VBO = 0;
        EBO = 0;
        VAO = 0;
        
        vertices.clear();
        indices.clear();
    };

private:
    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
     unsigned int VAO{0}, VBO{0}, EBO{0};
};