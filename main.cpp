/*****************************************************************************\
 | OpenGL                                                                      |
 |                                                                             |
 | Email: sh3g12 at soton dot ac dot uk                                        |
 | version 0.0.1                                                               |
 | Copyright Shakib Bin Hamid                                                  |
 |*****************************************************************************|
 |                                                                             |
 \*****************************************************************************/
#define _USE_MATH_DEFINES
#define GLEW_STATIC

#include <cmath>
#include <iostream>
#include <assert.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "gl_util.hpp"

// Window dimensions
GLuint WIDTH = 800, HEIGHT = 600;
GLFWwindow* window = nullptr;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLint stacks = 100;
GLint slices = 100;
GLfloat radius = 1.0f;

glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

std::vector<glm::vec3> * generateSphere (std::vector<glm::vec3> * vertices, std::vector<GLint> * indices, GLint Stacks, GLint Slices, GLfloat r);
void prepareSphereVAO(GLuint * VBO, GLuint * VAO, GLuint * EBO, std::vector<glm::vec3> verts, std::vector<GLint> idx, GLsizei stride, const void* pointer);
void drawSphere(Shader * sphereShader, GLuint * sphere_VAO, std::vector<GLint> * sphere_idx,
                GLuint * normal_VAO, std::vector<glm::vec3> * normal_verts,
                Shader * lampShader,
                GLint * objectColorLoc, GLint * lightColorLoc, GLint * lightPosLoc, GLint * viewPosLoc,
                GLint * q,
                GLuint count, glm::vec3 * locations, GLint * modelLoc, GLint * viewLoc, GLint * projLoc);
// The MAIN function, from here we start the application and run the game loop
int main() {
    
    // start glfw and glew with default settings
    assert(start_gl());
    
    // Build and compile our shader program
    Shader sphereShader("/Users/shakib-binhamid/Desktop/OpenGL Practice/OpenGL Tutorial Solo 1/OpenGL Tutorial Solo 1/shader.vs", "/Users/shakib-binhamid/Desktop/OpenGL Practice/OpenGL Tutorial Solo 1/OpenGL Tutorial Solo 1/shader.frag");
    
    Shader lampShader("/Users/shakib-binhamid/Desktop/OpenGL Practice/OpenGL Tutorial Solo 1/OpenGL Tutorial Solo 1/shader.vs", "/Users/shakib-binhamid/Desktop/OpenGL Practice/OpenGL Tutorial Solo 1/OpenGL Tutorial Solo 1/lamp.frag");
    
    /////// Sphere vertices, normals and indices generation  //////////////////////////////////////////
    std::vector<glm::vec3> sphere_verts;
    std::vector<GLint> sphere_idx;
    
    generateSphere( &sphere_verts, &sphere_idx, stacks, slices, radius);
    /////////////////  GET VAO READY FOR NORMALS  ////////////////////////
    
    GLuint sphere_VBO, sphere_VAO, sphere_EBO, normal_VAO;
    prepareSphereVAO(&sphere_VBO, &sphere_VAO, &sphere_EBO, sphere_verts, sphere_idx, 2 * sizeof(glm::vec3), (GLvoid*)0);
    prepareSphereVAO(&sphere_VBO, &normal_VAO, &sphere_EBO, sphere_verts, sphere_idx, sizeof(glm::vec3), (GLvoid*)0); // EBO and idx doesn't matter
    
    // where the cubes will appear in the world space
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.5f,  0.0f,  0.0f)
    };
    
    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(sphereShader.Program, "model");
    GLint viewLoc = glGetUniformLocation(sphereShader.Program, "view");
    GLint projLoc = glGetUniformLocation(sphereShader.Program, "projection");
    
    GLint objectColorLoc, lightColorLoc, lightPosLoc, viewPosLoc, q;
    q = glGetUniformLocation(sphereShader.Program, "q");
    
    // uniforms for lighting
    objectColorLoc = glGetUniformLocation(sphereShader.Program, "objectColor");
    lightColorLoc  = glGetUniformLocation(sphereShader.Program, "lightColor");
    lightPosLoc = glGetUniformLocation(sphereShader.Program, "lightPos");
    viewPosLoc = glGetUniformLocation(sphereShader.Program, "viewPos");
    
    // Game loop
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
        
        //lightPos.x = sin(glfwGetTime()) * 20.0f;
        //lightPos.y = cos(glfwGetTime()) * 20.0f;
        
        drawSphere(&sphereShader, &sphere_VAO, &sphere_idx,
                   &normal_VAO, &sphere_verts,
                   &lampShader,
                   &objectColorLoc, &lightColorLoc, &lightPosLoc, &viewPosLoc,
                   &q,
                   2, cubePositions, &modelLoc, &viewLoc, &projLoc);
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &sphere_VAO);
    glDeleteBuffers(1, &sphere_VBO);
    glDeleteBuffers(1, &sphere_EBO);
    
    glDeleteVertexArrays(1, &normal_VAO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

void drawSphere(Shader * sphereShader, GLuint * sphere_VAO, std::vector<GLint> * sphere_idx,
                GLuint * normal_VAO, std::vector<glm::vec3> * normal_verts,
                Shader * lampShader,
                GLint * objectColorLoc, GLint * lightColorLoc, GLint * lightPosLoc, GLint * viewPosLoc,
                GLint * q,
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
    
    for(GLuint i = 0; i < count; i++){
        // pass the model matrix to the shader
        
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
            glm::vec3 lightPos(100.0f, 0.0f, 100.0f);
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
        }else if (keys[GLFW_KEY_D]){
            // place the sphere in the right place
            sphereShader->Use();
            angle = (GLfloat)glfwGetTime() * (i + 1);
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, locations[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            glUniformMatrix4fv(*modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(*q, 4);
            glUniform3f(*lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(*objectColorLoc, 1.0f, 0.5f, 0.31f);
            // draw sphere
            glBindVertexArray(*sphere_VAO);
            glDrawElements(GL_TRIANGLES, (GLint)sphere_idx->size(), GL_UNSIGNED_INT, 0);
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
    }
    
    glBindVertexArray(0); // done drawing sphere, unload VAO
}

void prepareSphereVAO(GLuint * VBO, GLuint * VAO, GLuint * EBO, std::vector<glm::vec3> verts, std::vector<GLint> idx, GLsizei stride, const void* pointer){
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);
    
    glBindVertexArray(*VAO);
    
    // bind VBO and load vertex data on it
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * verts.size(), &verts[0], GL_STATIC_DRAW); // circle
    
    // bind EBO and load index data on it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * idx.size(), &idx[0], GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, pointer);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0); // Unbind VAO
}

std::vector<glm::vec3> * generateSphere (std::vector<glm::vec3> * vertices, std::vector<GLint> * indices, const GLint Stacks, const GLint Slices, const GLfloat r){
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
            
            // Push Back Vertex Data
            glm::vec3 v(x, y, z);
            vertices->push_back (v); // vertex
            vertices->push_back(v + glm::normalize(v) * 0.05f); // normal
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