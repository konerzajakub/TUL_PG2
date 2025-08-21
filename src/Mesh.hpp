// Mesh.hpp

#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "assets.hpp"
#include "ShaderProgram.hpp"
        
// tøída pro Mesh
class Mesh {
public:
    // mesh data
    std::vector<vertex> vertices;   // pole vrcholù (vertexù)
    std::vector<GLuint> indices;    // pole indexù (jaké vrcholy tvoøí trojúhelníky)
    glm::vec3 origin{};             // lokální origin point meshe
    glm::vec3 orientation{};        // lokální orientace (natoèení) mesh
    
    GLuint texture_id{0};               // ID textury v pamìti (automaticky texture id=0 => žádná textura)
    GLenum primitive_type = GL_POINT;   // typ "primitiva" (GL_TRIANGLES, GL_LINES, GL_POINTS)
    
    // mesh material
    glm::vec4 ambient_material{1.0f};   // (barva ve stínu)
    glm::vec4 diffuse_material{1.0f};   // (hlavní barva)
    glm::vec4 specular_material{1.0f};  // (barva lesku)
    float reflectivity{1.0f};           // odleskovost
    
    // indexované vykreslení (vkládání do VRAM)
	Mesh(GLenum primitive_type, std::vector<vertex> const & vertices, std::vector<GLuint> const & indices, glm::vec3 const & origin, glm::vec3 const & orientation, GLuint const texture_id = 0):
        primitive_type(primitive_type),
        vertices(vertices),
        indices(indices),
        origin(origin),
        orientation(orientation),
        texture_id(texture_id)
    {
        // 1. Vytvoøení VAO (Vertex Array Object)
        glCreateVertexArrays(1, &VAO);

        // 2. Naplnìní VBO daty z "vertices" (Vertex Buffer Object)
        glCreateBuffers(1, &VBO);
        glNamedBufferStorage(VBO, this->vertices.size() * sizeof(vertex), this->vertices.data(), 0);

        // 3. Naplnìní EBO daty (Element Buffer Object)
        glCreateBuffers(1, &EBO);
        glNamedBufferStorage(EBO, this->indices.size() * sizeof(GLuint), this->indices.data(), 0);

        // 4. Propojení VBO, EBO s VAO
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(vertex));
        glVertexArrayElementBuffer(VAO, EBO);

        // 5. Nastavení formátu vertex atributù
        glEnableVertexArrayAttrib(VAO, 0);
        glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position)); // vertex
        glVertexArrayAttribBinding(VAO, 0, 0);

        glEnableVertexArrayAttrib(VAO, 1);
        glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, normal)); // normála
        glVertexArrayAttribBinding(VAO, 1, 0);

        glEnableVertexArrayAttrib(VAO, 2);
        glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texCoords)); // text. souøadnice
        glVertexArrayAttribBinding(VAO, 2, 0);
    };

    
    // GPU vykreslení
    void draw(ShaderProgram &shader, const glm::mat4& model_matrix) const {
 		if (VAO == 0) {
			std::cerr << "VAO not initialized!\n";
			return;
		}
 
        shader.setUniform("uM_m", model_matrix); // poslání matice modelu do shaderz
        
        if (texture_id != 0) {
            glBindTextureUnit(0, texture_id);   // pokud má mesh texturu -> pøipojit
        }

        glBindVertexArray(VAO); // pro vykreslení použít vertex data (VAO)
        glDrawElements(primitive_type, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)0); // vykreslení
        glBindVertexArray(0);   // odpojení
    }

    // uvolnìní prostøedkù
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