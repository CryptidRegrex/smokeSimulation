#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 





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
    cube->VelX = calloc(node * node, sizeof(float));
    cube->VelY = calloc(node * node, sizeof(float));
    cube->VelX0 = calloc(node * node, sizeof(float));
    cube->VelY0 = calloc(node * node, sizeof(float));

}





int main(void)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SmokeSim", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

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