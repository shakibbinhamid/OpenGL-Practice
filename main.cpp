/*****************************************************************************\
 | OpenGL Coursework 1                                                         |
 |                                                                             |
 | Email: sh3g12 at soton dot ac dot uk                                        |
 | version 0.0.1                                                               |
 | Copyright Shakib Bin Hamid                                                  |
 |*****************************************************************************|
 | This is where everything comes together.                                    |
 | I've written all the sphere calculation and drawing commands here           |
 \*****************************************************************************/
#define _USE_MATH_DEFINES
#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)

#include <cmath>
#include <iostream>
#include <assert.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "gl_util.hpp"

///////////////////////////////// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////////

// Window dimensions
GLuint WIDTH = 800, HEIGHT = 600;
GLFWwindow* window = nullptr;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// for spheres
GLint stacks = 100;
GLint slices = 100;
GLfloat radius = 1.0f;

// light source position
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

//////////////////////////////// PROTOTYPES //////////////////////////////////////////////////////////////////////////////////////////

// generates a sphere and populates vertices, indices. q2verts are only used in question 2
std::vector<GLfloat> * generateSphere (std::vector<GLfloat> * vertices, std::vector<GLfloat> * q2Verts, std::vector<GLint> * indices, GLint Stacks, GLint Slices, GLfloat r);
// generates a cone and populates vertices.
std::vector<GLfloat> * generateCone(std::vector<GLfloat> * verts, const GLint Stacks, const GLint Slices);
// Prepares a vao, vbo and ebo with the data
void prepareVAO(GLuint * VAO, GLuint * VBO, GLuint * EBO,
                std::vector<GLfloat> verts, std::vector<GLint> idx,
                GLuint aCount, GLuint aLoc[], GLint size[], GLsizei vStride[], const void* vOffset[]);
// loads up a texture in texture1
void prepareTexture(GLuint * texture1, const char *fname, int * width, int * height, int * comp);
// drawing command for a sphere. more like a thunk
void drawSphere(Shader * sphereShader, GLuint * sphere_VAO, std::vector<GLint> * sphere_idx,
                GLuint * normal_VAO, std::vector<GLfloat> * normal_verts,
                GLuint * cone_VAO, std::vector<GLfloat> * cone_verts, std::vector<GLint> * cone_idx,
                Shader * lampShader,
                GLint * objectColorLoc, GLint * lightColorLoc, GLint * lightPosLoc, GLint * viewPosLoc,
                GLint * q, GLuint * texture,
                GLuint count, glm::vec3 * locations, GLint * modelLoc, GLint * viewLoc, GLint * projLoc);

////////////////////////////// MAIN FUNCTION /////////////////////////////////////////////////////////////////////////////////////////

// The MAIN function, from here we start the application and run the game loop
int main() {
    
    // start glfw and glew with default settings
    assert(start_gl());
    
    // Build and compile our shader program
    Shader sphereShader("shaders/shader.vs", "shaders/shader.frag");
    
    Shader lampShader("shaders/shader.vs", "shaders/lamp.frag");
    
    /////// Sphere vertices, normals and indices generation  //////////////////////////////////////////
    
    std::vector<GLfloat> sphere_verts, q2Verts, cone_verts;
    std::vector<GLint> sphere_idx;
    
    generateCone(&cone_verts, stacks, slices);
    generateSphere( &sphere_verts, &q2Verts, &sphere_idx, stacks, slices, radius);
    
    std::vector<GLint> cone_idx(sphere_idx);
    
    /////////////////  DECLARATIONS  ////////////////////////
    
    GLuint sphere_VBO, sphere_VAO, sphere_EBO, normal_VAO, normal_VBO, cone_VAO, cone_VBO, cone_EBO;
    
    /////////////////  GET VAO READY FOR CONE  ////////////////////////////////////////////////////////
    GLuint aLoc[3] = {0};
    GLint size[3] = {3};
    GLsizei vStride[3] = {3 * sizeof(GLfloat)};
    const void* vOffset[3] = {(GLvoid*)0};
    
    prepareVAO(&cone_VAO, &cone_VBO, &cone_EBO, cone_verts, cone_idx, 1, aLoc, size, vStride, vOffset);

    /////////////////  GET VAO READY FOR SPHERE  //////////////////////////////////////////////////////
    aLoc[0] = 0; aLoc[1] = 1; aLoc[2] = 2;
    size[0] = size[1] = 3; size[2] = 2;
    vStride[0] = vStride[1] = vStride[2] = 8 * sizeof(GLfloat);
    vOffset[0] = (GLvoid*)0; vOffset[1] = (GLvoid*)(3 * sizeof(GLfloat)); vOffset[2] = (GLvoid*)(6 * sizeof(GLfloat));
    
    prepareVAO(&sphere_VAO, &sphere_VBO, &sphere_EBO, sphere_verts, sphere_idx, 3, aLoc, size, vStride, vOffset);
    
    /////////////////  GET VAO READY FOR NORMALS (Q2)  ////////////////////////////////////////////////
    
    aLoc[0] = 0;
    size[0] = 3;
    vStride[0] = 3 * sizeof(GLfloat);
    vOffset[0] = (GLvoid*)0;
    
    prepareVAO(&normal_VAO, &normal_VBO, nullptr, q2Verts, std::vector<GLint>() , 1, aLoc, size, vStride, vOffset);
    
    /////////////////  GET Textures ready  ////////////////////////////////////////////////////////////
    
    GLuint texture1;
    int width, height, comp;
    
    prepareTexture(&texture1, "images/earth.jpg", &width, &height, &comp);

    /////////////////  The positions for the spheres in q4  ////////////////////////////////////////////
    // where the cubes will appear in the world space
    glm::vec3 cubePositions[] = {
        glm::vec3(1.5f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    };
    
    /////////////////  Uniform variables for MVP in VS  /////////////////////////////////////////////////
    
    GLint modelLoc = glGetUniformLocation(sphereShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(sphereShader.Program, "view");
    GLint projLoc = glGetUniformLocation(sphereShader.Program, "projection");
    
    // The question number to switch
    GLint q = glGetUniformLocation(sphereShader.Program, "q");
    
    // uniforms for lighting
    GLint objectColorLoc = glGetUniformLocation(sphereShader.Program, "objectColor");
    GLint lightColorLoc  = glGetUniformLocation(sphereShader.Program, "lightColor");
    GLint lightPosLoc = glGetUniformLocation(sphereShader.Program, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(sphereShader.Program, "viewPos");
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check if any events have been activated (key pressed, mouse moved)
        glfwPollEvents();
        do_movement();
        
        // Clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        lightPos.x = sin(glfwGetTime()) * 0.1;
        lightPos.y = cos(glfwGetTime()) * 0.1;
        
        drawSphere(&sphereShader, &sphere_VAO, &sphere_idx,
                   &normal_VAO, &sphere_verts,
                   &cone_VAO, &cone_verts, &cone_idx,
                   &lampShader,
                   &objectColorLoc, &lightColorLoc, &lightPosLoc, &viewPosLoc,
                   &q, &texture1,
                   2, cubePositions, &modelLoc, &viewLoc, &projLoc);
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Deallocate
    glDeleteVertexArrays(1, &sphere_VAO);
    glDeleteBuffers(1, &sphere_VBO);
    glDeleteBuffers(1, &sphere_EBO);
    
    glDeleteVertexArrays(1, &normal_VAO);
    glDeleteVertexArrays(1, &normal_VBO);
    
    glDeleteVertexArrays(1, &cone_VAO);
    glDeleteBuffers(1, &cone_VBO);
    glDeleteBuffers(1, &cone_EBO);
    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

//////////////////////////////////////// DRAWING COMMAND //////////////////////////////////////////////////////////////////////////////////

void drawSphere(Shader * sphereShader, GLuint * sphere_VAO, std::vector<GLint> * sphere_idx,
                GLuint * normal_VAO, std::vector<GLfloat> * normal_verts,
                GLuint * cone_VAO, std::vector<GLfloat> * cone_verts, std::vector<GLint> * cone_idx,
                Shader * lampShader,
                GLint * objectColorLoc, GLint * lightColorLoc, GLint * lightPosLoc, GLint * viewPosLoc,
                GLint * q, GLuint * texture,
                GLuint count, glm::vec3 * locations, GLint * modelLoc, GLint * viewLoc, GLint * projLoc) {
    
    // Activate shader
    sphereShader->Use();
    
    glUniform3f(*viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z); // camera position for spec light
    glUniform3f(*lightColorLoc,  1.0f, 0.5f, 1.0f); // color of the light source

    // Pass the view and projection matrices to the shader
    glm::mat4 model; // model
    glm::mat4 view = camera.GetViewMatrix(); // Camera/View transformation
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f); // Projection
    glUniformMatrix4fv(*viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(*projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    GLfloat angle;// angle to rotate the object in model space
    
        if(keys[GLFW_KEY_B]) {
            // draw sphere
            sphereShader->Use();
            glBindVertexArray(*sphere_VAO);
            glUniform1i(*q, 2);
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_LINE_STRIP, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            
            // draw normals
            glUniform1i(*q, 3);
            glBindVertexArray(*normal_VAO);
            glDrawArrays(GL_LINES, 0, (GLint)normal_verts->size());
            glBindVertexArray(0);
        } else if(keys[GLFW_KEY_C]){
            // place the sphere in the right place
            glm::vec3 lightPos(0.0f, 0.0f, 100.0f);
            sphereShader->Use();
            model = glm::translate(model, glm::vec3(0.0f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(*q, 4);
            glUniform3f(*lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(*objectColorLoc, 1.0f, 0.5f, 0.31f);
            // draw sphere
            glBindVertexArray(*sphere_VAO);
            glDrawElements(GL_TRIANGLES, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        } else if (keys[GLFW_KEY_D]){
            // place the sphere in the right place
            sphereShader->Use();
            glBindVertexArray(*sphere_VAO);
            glUniform1i(*q, 4);
            glUniform3f(*lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(*objectColorLoc, 1.0f, 0.5f, 0.31f);
            
            angle = (GLfloat)glfwGetTime() * 2;
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, locations[0]);
            model = glm::scale(model, glm::vec3(0.2f));
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            // draw sphere
            glDrawElements(GL_TRIANGLES, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, locations[0]);
            model = glm::scale(model, glm::vec3(0.2f));
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(*objectColorLoc, 0.31f, 0.5f, 1.00f);
            
            // draw sphere
            glDrawElements(GL_TRIANGLES, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(1.8f, glm::sin(glfwGetTime() + 300), 0.0f));
            model = glm::rotate(model, angle, glm::vec3(-1.0f, 0.5f, 1.0f));
            model = glm::scale(model, glm::vec3(0.1f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(*objectColorLoc, 0.5f, 0.31f, 1.00f);
            glBindVertexArray(*cone_VAO);
            glDrawElements(GL_TRIANGLES, (GLint)cone_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(glm::cos(glfwGetTime() + 300), 1.8f, 0.0f));
            model = glm::rotate(model, angle, glm::vec3(-1.0f, 0.5f, 1.0f));
            model = glm::scale(model, glm::vec3(0.1f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(*objectColorLoc, 0.5f, 0.31f, 1.00f);
            glBindVertexArray(*cone_VAO);
            glDrawElements(GL_TRIANGLES, (GLint)cone_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            
            // place light source in the right place
            lampShader->Use();
            GLint mLoc = glGetUniformLocation(lampShader->Program, "model");
            GLint vLoc  = glGetUniformLocation(lampShader->Program, "view");
            GLint pLoc  = glGetUniformLocation(lampShader->Program, "projection");
            glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));
            model = glm::mat4();
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.1f)); // Make it a smaller cube
            glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(model));
            // draw the light source
            glBindVertexArray(*sphere_VAO);
            glDrawElements(GL_TRIANGLES, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        } else if (keys[GLFW_KEY_E]) {
            // place sphere in right place
            glm::vec3 lightPos(100.0f, 0.0f, 100.0f);
            sphereShader->Use();
            model = glm::translate(model, glm::vec3(0.0f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(*q, 5);
            glUniform3f(*lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(*objectColorLoc, 1.0f, 0.5f, 0.31f);
            
            // Bind Textures using texture units
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, *texture);
            glUniform1i(glGetUniformLocation(sphereShader->Program, "ourTexture1"), 0);
            
            // draw sphere
            glBindVertexArray(*sphere_VAO);
            glDrawElements(GL_TRIANGLES, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        } else{
            // place sphere in right place
            sphereShader->Use();
            glUniform1i(*q, 1);
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            // draw shpere
            glBindVertexArray(*sphere_VAO);
            glDrawElements(GL_LINE_STRIP, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    
    glBindVertexArray(0); // done drawing sphere, unload VAO
}

///////////////////////////////// HELPER FUNCTIONS //////////////////////////////////////////////////////////////////////////

/*
 Will connect rgba texture to GL_TEXTURE_2D, GL_REPEAT (both s, t), GL_LINEAR filtering
 */
void prepareTexture(GLuint * texture, const char *fname, int * width, int * height, int * comp){
    
    // where the image data will be loaded
    unsigned char* image;
    
    // generate and bind to the GL_TEXTURE_2D object
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load, create texture and generate mipmaps
    image = stbi_load(fname, width, height, comp, STBI_rgb_alpha);
    if(image == nullptr)
        throw(std::string("Failed to load texture"));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // delete image data
    stbi_image_free(image);
    // unbind texture when done
    glBindTexture(GL_TEXTURE_2D, 0);
}

void prepareVAO(GLuint * VAO, GLuint * VBO, GLuint * EBO,
                      std::vector<GLfloat> verts, std::vector<GLint> idx,
                      GLuint aCount, GLuint aLoc[], GLint size[], GLsizei vStride[], const void* vOffset[]){
    
    // generate the vao's and vbo's
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    if (EBO != nullptr) glGenBuffers(1, EBO);
    
    // bind the vao as current
    glBindVertexArray(*VAO);
    
    // bind VBO and load vertex data on it
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), &verts[0], GL_STATIC_DRAW);
    
    // if there is an ebo bind EBO and load index data on it
    if (EBO != nullptr) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * idx.size(), &idx[0], GL_STATIC_DRAW);
    }
    
    // prepare the vao
    for (GLint i = 0; i < aCount; i++) {
        glVertexAttribPointer(i, size[i], GL_FLOAT, GL_FALSE, vStride[i], vOffset[i]);
        glEnableVertexAttribArray(i);
    }
    // Unbind vao as we're done pointing attributes
    glBindVertexArray(0);
}

/*
 Generates a sphere and populates the vertices, indices based on how many 'stacks' and 'slices' are needed.
 It is a UV sphere.
 vertices contain position, normal, texcord
 q2 verts just contain position, normal
 */
std::vector<GLfloat> * generateSphere (std::vector<GLfloat> * vertices, std::vector<GLfloat> * q2Verts, std::vector<GLint> * indices,
                                       const GLint Stacks, const GLint Slices, const GLfloat r){
    for (int i = 0; i <= Stacks; ++i){
        float V   = i / (float) Stacks;
        float phi = V * glm::pi <float> ();
        // Loop Through Slices
        for (int j = 0; j <= Slices; ++j){
            float U = j / (float) Slices;
            float theta = U * (glm::pi <float> () * 2);
            
            // Calc The Vertex Positions
            float x = r * cosf (theta) * sinf (phi);
            float y = r * cosf (phi);
            float z = r * sinf (theta) * sinf (phi);
            
            // vertices for sphere
            glm::vec3 v(x, y, z);
            glm::vec3 n(v + glm::normalize(v) * 0.05f);
            vertices->push_back (v.x); // vertex
            vertices->push_back (v.y);
            vertices->push_back (v.z);
            
            vertices->push_back (n.x); // normal
            vertices->push_back (n.y);
            vertices->push_back (n.z);
            
            vertices->push_back (U); // texcord
            vertices->push_back (V);
            
            // special for q2
            q2Verts->push_back (v.x); // vertex
            q2Verts->push_back (v.y);
            q2Verts->push_back (v.z);
            
            q2Verts->push_back (n.x); // normal
            q2Verts->push_back (n.y);
            q2Verts->push_back (n.z);
        }
    }
    
    for (int i = 0; i < Slices * Stacks + Slices; ++i){
        indices->push_back (i);
        indices->push_back (i + Slices + 1);
        indices->push_back (i + Slices);
        
        indices->push_back (i + Slices + 1);
        indices->push_back (i);
        indices->push_back (i + 1);
    }
    
    return vertices;
}

/* 
 Generates a cone based on the following equations -
        x(theta, r) = r x sin(theta)
        y(theta, r) = r x cos(theta)
        z(theta, r) = r
 */
std::vector<GLfloat> * generateCone(std::vector<GLfloat> * verts, const GLint Stacks, const GLint Slices){
    
    GLfloat hInc = 1.0f / Slices;
    GLfloat tInc = (2 * glm::pi<GLfloat>()) / Stacks;
    GLfloat h = 1.0f, theta;
    
    for (GLint i = 0; i < Slices; i++){
        h -= hInc;
        theta = 0.0f;
        for (GLint j = 0; j < Stacks; j++){
            theta += tInc;
            GLfloat x = h * glm::sin(theta);
            GLfloat y = h * glm::cos(theta);
            
            verts->push_back(x);
            verts->push_back(y);
            verts->push_back(h);
        }
    }
    
    return verts;
}