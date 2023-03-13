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

static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};


static const char* vertex_shader_text =
"#version 110\n"
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
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";


static void error_callback(int error, const char* description)
{
    printf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

}


int main(void)
{


    //Particle* p = ParticleCreation(256, 1, 0, 0);
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;



    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SmokeSim", NULL, NULL);
    if (!window)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    ////TESTING
    //float vertices[] = {
    //0.5f, 0.5f, 0.0f,
    //-0.5f, 0.5f, 0.0f,
    //-0.5f, -0.5f, 0.0f,
    //0.5f, -0.5f, 0.0f

    //};


    //Vertex Input
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Vertex Shader to handle vertices
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    //Check success of shader compile
    int success;
    char log[1024];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 1024, NULL, log);
        printf("Vertex Shader compiliation failed\n");
    }

    //Fragment Shader - this will calculate color output to the pixels
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    //Create Shader program to link vertex shader and fragment shader
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //Check success of shader link
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, log);
    }

    //setting values to link vertex attributes
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    //Linking vertex attributes
    glEnableVertexAttribArray(vpos_location);
    //parameters 1 - vertex attribute to configure, 2 - vec size 1|2|3|4, 3 - data type|vec are always floats, 4 - Normalize; Only for integer input, 
    //5 - stride value or space between consecutive vertex attributes, 6 - offset of position data begins
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 2));
    //Stopping point

    //VERTEX INPUT
    //setting up our vertex buffer object
    //unsigned int VAO;
    //Generate buffer ID
    //glGenBuffers(1, &VAO);
    //Bind multiple buffers at once
    //glBindBuffer(GL_ARRAY_BUFFER, VAO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
    //VERTEX INPUT

    //VERTEX SHADER / FRAGMENT SHADER

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;


        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float)glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);


        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);





        //We are switch the back and front buffers to render the entire frame and swawpt them
        glfwSwapBuffers(window);
        //Limits the number of swapped buffers that can happen. This will reduce screen tearing
        //glfwSwapInterval(1);
        //This will process events by polling and waiting for event sot happen.
        glfwPollEvents();

        //glEnableVertexAttribArray(0);
        //glEnableVertexAttribArray(4);




        

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
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(0);
}