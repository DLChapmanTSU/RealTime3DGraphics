#include "Skybox.h"
#include "ImageLoader.h"

Skybox::Skybox(std::string r, std::string l, std::string t, std::string b, std::string k, std::string f, std::string o)
{
    m_texLocations[0] = r;
    m_texLocations[1] = l;
    m_texLocations[2] = t;
    m_texLocations[3] = b;
    m_texLocations[4] = k;
    m_texLocations[5] = f;
    m_objLocation = o;
}

void Skybox::Initialize()
{
    m_loader.GetMeshVector().clear();

    //Hard sets the vertices of the skybox, similarly to the cube in the renderer
    std::vector<GLfloat> vertices{
        -10.0f,  10.0f, -10.0f,
        -10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        -10.0f,  10.0f, -10.0f,
        10.0f,  10.0f, -10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
        10.0f, -10.0f,  10.0f
    };

    //Also hard codes the uvs
    std::vector<GLfloat> uvs{
        -10.0f,  -10.0f, -10.0f,
        -10.0f, 10.0f, -10.0f,
        10.0f, 10.0f, -10.0f,
        10.0f, 10.0f, -10.0f,
        10.0f,  -10.0f, -10.0f,
        -10.0f,  -10.0f, -10.0f,

        -10.0f, 10.0f,  10.0f,
        -10.0f, 10.0f, -10.0f,
        -10.0f,  -10.0f, -10.0f,
        -10.0f,  -10.0f, -10.0f,
        -10.0f,  -10.0f,  10.0f,
        -10.0f, 10.0f,  10.0f,

        10.0f, 10.0f, -10.0f,
        10.0f, 10.0f,  10.0f,
        10.0f,  -10.0f,  10.0f,
        10.0f,  -10.0f,  10.0f,
        10.0f,  -10.0f, -10.0f,
        10.0f, 10.0f, -10.0f,

        -10.0f, 10.0f,  10.0f,
        -10.0f,  -10.0f,  10.0f,
        10.0f,  -10.0f,  10.0f,
        10.0f,  -10.0f,  10.0f,
        10.0f, 10.0f,  10.0f,
        -10.0f, 10.0f,  10.0f,

        -10.0f,  -10.0f, -10.0f,
        10.0f,  -10.0f, -10.0f,
        10.0f,  -10.0f,  10.0f,
        10.0f,  -10.0f,  10.0f,
        -10.0f,  -10.0f,  10.0f,
        -10.0f,  -10.0f, -10.0f,

        -10.0f, 10.0f, 10.0f,
        -10.0f, 10.0f,  -10.0f,
        10.0f, 10.0f, 10.0f,
        10.0f, 10.0f, 10.0f,
        -10.0f, 10.0f,  -10.0f,
        10.0f, 10.0f,  -10.0f
    };


    //Sets relevant VBOs and binds them to a VAO
    GLuint vertexVBO;

    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint textureCoordsVBO;

    glGenBuffers(1, &textureCoordsVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textureCoordsVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * uvs.size(), uvs.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, textureCoordsVBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    //Sets texture parameters for a cubemap
    Helpers::ImageLoader image;

    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //Loads in textures for each side if they exist
    for (size_t i = 0; i < 6; i++)
    {
        if (image.Load(m_texLocations[i])) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum(i), 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());
        }
        else {
            std::cout << "Can't load sky texture" << std::endl;
        }
    }
}

void Skybox::Render(GLuint& p, glm::mat4 c)
{
    //Draws skybox using the skybox shader
    //Requires a unique cubemap sampler
    GLuint combined_xform_id = glGetUniformLocation(p, "combined_xform");
    glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(c));

    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);
    glUniform1i(glGetUniformLocation(p, "skySampler"), 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}