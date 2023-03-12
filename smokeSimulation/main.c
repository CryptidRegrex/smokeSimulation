#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include "linmath.h"





// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct Particle {

    //All of this will be needed to create our particles value
    int size;

    float* density;  //Represented as rho
    //We use the m, mass Divided by V, volume to achieve density
    float* density0;

    float diffusion;
    float viscosity; //Represented as mu
    //We use mu*Delta Squared * u. mu is our friction value and (Delta Squared * u) is the Laplacian of the velocity.
    //Higher vescosity equates to slower flow

    float timeStep;
    float* VelX; //Current velocity represented as u
    float* VelY; //Current velocity represented as u

    float* VelX0; //Previous velocity represented as u
    float* VelY0; //Previous velocity represented as u

};

typedef struct Particle Particle;

Particle* ParticleCreation(int size, int diffusion, int viscosity, float timeS) {
    Particle* cube = malloc(sizeof(*cube));
    int node = size;

    cube->size = node;
    cube->timeStep = timeS;
    cube->diffusion = diffusion;
    cube->viscosity = viscosity;
    cube->density0 = calloc(node * node, sizeof(float));
    cube->density = calloc(node * node, sizeof(float));
    //Current velocities X and Y
    cube->VelX = calloc(node * node, sizeof(float));
    cube->VelY = calloc(node * node, sizeof(float));
    //previous velocities X and Y
    cube->VelX0 = calloc(node * node, sizeof(float));
    cube->VelY0 = calloc(node * node, sizeof(float));

    return cube;
}

void addDensity(Particle* cube, int x, int y, float amount) {

}


static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";


int main(void)
{
    Particle* p = ParticleCreation(256, 1, 0, 0);
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;



    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SmokeSim", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    //TESTING
    float vertices[] = {
    0.5f, 0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f

    };


    //render OpenGL
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    //Stopping point

    //VERTEX INPUT
    //setting up our vertex buffer object
    unsigned int VAO;
    //Generate buffer ID
    //glGenBuffers(1, &VAO);
    //Bind multiple buffers at once
    //glBindBuffer(GL_ARRAY_BUFFER, VAO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    //VERTEX INPUT

    //VERTEX SHADER / FRAGMENT SHADER

    while (!glfwWindowShouldClose(window))
    {
        int width = 800, height = 600;
        glfwGetFramebufferSize(window, &width, &height);






        //We are switch the back and front buffers to render the entire frame and swawpt them
        glfwSwapBuffers(window);
        //Limits the number of swapped buffers that can happen. This will reduce screen tearing
        glfwSwapInterval(1);
        //This will process events by polling and waiting for event sot happen.
        glfwPollEvents();

        //glEnableVertexAttribArray(0);
        //glEnableVertexAttribArray(4);

        glClearColor(0.1f, 0.1f, 0.1f, 0.5f);



        

        //render(window);
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        //GLint a = 0;
        //GLsizei b = 256;
        //glDrawArrays(GL_LINES, a, b);
        //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        // input
        // -----
   

        // render
        // ------
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        
    }
    glfwTerminate();
}