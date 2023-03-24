#define GLFW_INCLUDE_NONE
#define point(x, y)  ((x) + (y) * nodes)
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include "linmath.h"





// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float i = 0.0f;

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

    //location of particle
    float* vertX;
    float* vertY; 

    //color of particle
    float* r;
    float* g;
    float* b;

};

typedef struct Particle Particle;

Particle* ParticleCreation(int size, int diffusion, int viscosity, float timeS) {
    Particle* quad = malloc(sizeof(*quad));
    int nodes = size;

    quad->size = nodes;
    quad->timeStep = timeS;
    quad->diffusion = diffusion;
    quad->viscosity = viscosity;
    //Current density
    quad->density = calloc(nodes * nodes, sizeof(float));
    //previous density
    quad->density0 = calloc(nodes * nodes, sizeof(float));
    //Current velocities X and Y
    quad->VelX = calloc(nodes * nodes, sizeof(float));
    quad->VelY = calloc(nodes * nodes, sizeof(float));
    //previous velocities X and Y
    quad->VelX0 = calloc(nodes * nodes, sizeof(float));
    quad->VelY0 = calloc(nodes * nodes, sizeof(float));
    //Current vertex of pixel
    quad->vertX = calloc(nodes * nodes, sizeof(float));
    quad->vertY = calloc(nodes * nodes, sizeof(float));
    //Current color of the pixle
    quad->r = calloc(nodes * nodes, sizeof(float));
    quad->g = calloc(nodes * nodes, sizeof(float));
    quad->b = calloc(nodes * nodes, sizeof(float));

    return quad;
}



//void addParticles(int size, float* x, float* s, float dt)
//{
//    int i, nodes = (size + 2) * (size + 2);
//
//    for (i = 0; i < nodes; i++)
//    {
//        x[i] += dt * s[i];
//    }
//}

void addDensity(Particle* quad, int x, int y, float amount) {
    int nodes = quad->size;
    quad->density[point(x, y)] += amount;
}

/// <summary>
/// This function is going to be the 
/// </summary>
/// <param name="quad"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="xChange"></param>
/// <param name="yChange"></param>
void addVelocity(Particle* quad, int x, int y, float xChange, float yChange) {
    int nodes = quad->size;
    int location = point(x, y);

    quad->VelX[location] += xChange;
    quad->VelY[location] += yChange;
}

/// <summary>
/// This function is setting the boundaries of the "Particles". Each particle is made up of an array of tiny "boxes" or points that contain
/// vector information. The boundary's outter box is going to constantly match the outer array of boxes as to for the arrays to contain themselves.
/// </summary>
/// <param name="b"></param>
/// <param name="x"></param>
/// <param name="nodes"></param>

static void setBoundaries(int b, float* x, int nodes) {

    //handles boundaries for x axis

    for (int i = 1; i < nodes - 1; i++) {
        //we use i, 0 (0 being the top row of the array). 
        //Basically we are saying if b == 2 or x then we want to create the exact opposite velocity that of course being the negative value.
        x[point(i, 0)] = b == 2 ? -x[point(i, 1)] : x[point(i, 1)];
        //The nodes - 1 is looking at the bottom of the array
        x[point(i, nodes - 1)] = b == 2 ? -x[point(i, nodes - 2)] : x[point(i, nodes - 2)];
    }


    for (int j = 1; j < nodes - 1; j++) {
        x[point(0, j)] = b == 1 ? -x[point(1, j)] : x[point(1, j)];
        x[point(nodes - 1, j)] = b == 1 ? -x[point(nodes - 2, j)] : x[point(nodes - 2, j)];
    }

    //Sets bounds for and checks each outer column and row. This includes the corners
    x[point(0, 0)] = 0.5f * (x[point(1, 0)] + x[point(0, 1)]);
    x[point(0, nodes - 1)] = 0.5f * (x[point(1, nodes - 1)] + x[point(0, nodes - 2)]);
    x[point(nodes - 1, 0)] = 0.5f * (x[point(nodes - 2, 0)] + x[point(nodes - 1, 1)]);
    x[point(nodes - 1, nodes - 1)] = 0.5f * (x[point(nodes - 2, nodes - 1)] + x[point(nodes - 1, nodes - 2)]);

}

createVertexShader()
{

}




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



void draw(float x, float y)
{
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    // GLint mvp_location, vpos_location, vcol_location;
    float vertices[] = {
        x, y, 0.0f
    };
    //================================================openGL pipeline============================================
    //STEP 1 
    //Vertex Input
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glPointSize(10);
    glDrawArrays(GL_POINTS, 0, 100 * 100);
    glDisableVertexAttribArray(0);
}

int main(void)
{


    Particle* p = ParticleCreation(256, 1, 0, 0);
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



    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SmokeSim", NULL, NULL);
    if (!window)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    //Checking for keystrokes in the window
    glfwSetKeyCallback(window, key_callback);
    //Setting window context for shaders
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    //Limits the number of swapped buffers that can happen. This will reduce screen tearing
    glfwSwapInterval(1);
    
    //setting up time
    float time = glfwGetTime();
    float speed = .1f;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    //vert* v = setupVerts(i, i, i, i, i);

    //printf(*p)

    

    while (!glfwWindowShouldClose(window))
    {
        float tStep = (((float)rand() / (float)(RAND_MAX)) * 0.1f);
        printf("tStep Value: %f", tStep);
        float timeS = glfwGetTime();
        float deltaTime = timeS - time;

        //Point
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        //glViewport(0, 0, width, height);
        //glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(program);
        // 
        //Initilize all particles with a vector
        addDensity(p, x, y, 1);
        
        addVelocity(p, 0.0f, 0.0f, x, y);

        x = x + tStep * deltaTime;
        y = y + tStep * deltaTime;
        //*p->vertX = x;
        //*p->vertY = y;
        
        if (x > 400.0f)
        {
            x = 0.0f;
        }
        for (int x = 0; x < p->size; x++)
        {
            draw(*p->VelX, *p->VelY);
        }
        
        //glDrawArrays(GL_POINT, 0, 1);

        //We are switch the back and front buffers to render the entire frame and swawpt them
        glfwSwapBuffers(window);

        //glfwSwapInterval(1);
        //This will process events by polling and waiting for event sot happen.
        glfwPollEvents();

    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(0);
}