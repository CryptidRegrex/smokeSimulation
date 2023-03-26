#define GLFW_INCLUDE_NONE
#define point(x, y)  ((x) + (y) * nodes)
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include "linmath.h"


// settings
const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 512;

float mouseX, mouseY, mouseX0, mouseY0;

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

Particle* ParticleCreation(int nodes, int diffusion, int viscosity, float timeS) {
    Particle* quad = malloc(sizeof(*quad));

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
    //for (int x = 0; x < nodes; x++)
    //{
    //    printf("densityAFTER: %f\n", *quad->density);
    //    printf("densityBEFORE: %f\n", *quad->density0);
    //}
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
        //Basically we are saying if b == 2 or y then we want to create the exact opposite velocity that of course being the negative value.
        x[point(i, 0)] = b == 2 ? -x[point(i, 1)] : x[point(i, 1)]; //Looking at the y top row
        //The nodes - 1 is looking at the bottom row of the array
        x[point(i, nodes - 1)] = b == 2 ? -x[point(i, nodes - 2)] : x[point(i, nodes - 2)]; //Looking at the y bottom row
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


/// <summary>
/// This method is going to handle solving for each value by taking a combination of each value next to it and 
/// solving for the median
/// </summary>
/// <param name="b"></param>
/// <param name="x"></param>
/// <param name="x0"></param>
/// <param name="a"></param>
/// <param name="c"></param>
/// <param name="iter"></param>
/// <param name="nodes"></param>
static void lin_solve(int b, float* x, float* x0, float a, float c, int iter, int nodes)
{
    float cRecip = 1.0 / c;
    for (int j = 1; j < nodes - 1; j++) {
        for (int i = 1; i < nodes - 1; i++) {
            x[point(i, j)] =
                (x0[point(i, j)]
                    + a * (x[point(i + 1, j)]
                        + x[point(i - 1, j)]
                        + x[point(i, j + 1)]
                        + x[point(i, j - 1)]
                        )) * cRecip;
        }
    }
    setBoundaries(b, x, nodes);
}

static void diffuse(int b, float* x, float* x0, float diff, float dt, int iter, int nodes)
{
    float a = dt * diff * (nodes - 2) * (nodes - 2);
    lin_solve(b, x, x0, a, 1 + 6 * a, iter, nodes);
}


/// <summary>
/// 
/// </summary>
/// <param name="velocX"></param>
/// <param name="velocY"></param>
/// <param name="p"></param>
/// <param name="div"></param>
/// <param name="iter"></param>
/// <param name="nodes"></param>
static void project(float* velocX, float* velocY, float* p, float* div, int iter, int nodes)
{

    for (int j = 1; j < nodes - 1; j++) {
        for (int i = 1; i < nodes - 1; i++) {
            div[point(i, j)] = -0.5f * (
                velocX[point(i + 1, j)]
                - velocX[point(i - 1, j)]
                + velocY[point(i, j + 1)]
                - velocY[point(i, j - 1)]
                ) / nodes;
            p[point(i, j)] = 0;
        }
    }

    setBoundaries(0, div, nodes);
    setBoundaries(0, p, nodes);
    lin_solve(0, p, div, 1, 6, iter, nodes);

    for (int j = 1; j < nodes - 1; j++) {
        for (int i = 1; i < nodes - 1; i++) {
            velocX[point(i, j)] -= 0.5f * (p[point(i + 1, j)]
                - p[point(i - 1, j)]) * nodes;
            velocY[point(i, j)] -= 0.5f * (p[point(i, j + 1)]
                - p[point(i, j - 1)]) * nodes;
        }
    }

    setBoundaries(1, velocX, nodes);
    setBoundaries(2, velocY, nodes);
}

/// <summary>
/// 
/// </summary>
/// <param name="b"></param>
/// <param name="d"></param>
/// <param name="d0"></param>
/// <param name="velocX"></param>
/// <param name="velocY"></param>
/// <param name="dt"></param>
/// <param name="nodes"></param>
static void advect(int b, float* d, float* d0, float* velocX, float* velocY,  float dt, int nodes)
{
    float i0, i1, j0, j1;

    float dtx = dt * (nodes - 2);
    float dty = dt * (nodes - 2);

    float s0, s1, t0, t1;
    float tmp1, tmp2, x, y;

    float Nfloat = nodes;
    float ifloat, jfloat;
    int i, j;


    for (j = 1, jfloat = 1; j < nodes - 1; j++, jfloat++) {
        for (i = 1, ifloat = 1; i < nodes - 1; i++, ifloat++) {
            tmp1 = dtx * velocX[point(i, j)];
            tmp2 = dty * velocY[point(i, j)];
            x = ifloat - tmp1;
            y = jfloat - tmp2;

            if (x < 0.5f) x = 0.5f;
            if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
            i0 = floorf(x);
            i1 = i0 + 1.0f;
            if (y < 0.5f) y = 0.5f;
            if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
            j0 = floorf(y);
            j1 = j0 + 1.0f;


            s1 = x - i0;
            s0 = 1.0f - s1;
            t1 = y - j0;
            t0 = 1.0f - t1;


            int i0i = i0;
            int i1i = i1;
            int j0i = j0;
            int j1i = j1;


            d[point(i, j)] =
                s0 * (t0 * d0[point(i0i, j0i)])
                + (t1 * d0[point(i0i, j0i)])
                + s1 * (t0 * d0[point(i1i, j1i)])
                + (t1 * d0[point(i1i, j1i)]);

        }
    }
    setBoundaries(b, d, nodes);
}


void particleStep(Particle* quad)
{
    int nodes = quad->size;
    float visc = quad->viscosity;
    float diff = quad->diffusion;
    float dt = quad->timeStep;
    float* Vx = quad->VelX;
    float* Vy = quad->VelY;
    float* Vx0 = quad->VelX0;
    float* Vy0 = quad->VelY0;
    float* s = quad->density0;
    float* density = quad->density;

    diffuse(1, Vx0, Vx, visc, dt, 4, nodes);
    diffuse(2, Vy0, Vy, visc, dt, 4, nodes);

    project(Vx0, Vy0, Vx, Vy, 4, nodes);

    advect(1, Vx, Vx0, Vx0, Vy0, dt, nodes);
    advect(2, Vy, Vy0, Vx0, Vy0, dt, nodes);


    project(Vx, Vy, Vx0, Vy0, 4, nodes);

    diffuse(0, s, density, diff, dt, 4, nodes);
    advect(0, density, s, Vx, Vy,  dt, nodes);
}

//void renderValues() {
//    for (int i = 0; i < p->size)
//}



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

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    //float xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseX = xpos;
    mouseY = ypos;
    //printf("x and y position of mouse: %f, %f\n", xpos, ypos);
}




void draw(Particle* p)
{
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    // GLint mvp_location, vpos_location, vcol_location;

    //For rendering densities
    //for (int i = 0; i < p->size; i++)
    //{
    //    for (int j = 0; j < p->size; i++)
    //    {
    //        int nodes = p->size;
    //        int x = SCR_WIDTH;
    //        int y = SCR_WIDTH;
    //        int d = p->density[point(x, y)];
    //    }
    //}

    float vertices[] = {
        300.0f, 300.0f, 0.0f
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

    Particle* p = ParticleCreation(256, 0, 0, 0.1f);
    
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
    float x = 0.1f, y = 0.1f, z = 0.0f;

    

    while (!glfwWindowShouldClose(window))
    {
        float tStep = (((float)rand() / (float)(RAND_MAX)) * 0.1f);
        //printf("tStep Value: %f", tStep);
        float timeS = glfwGetTime();
        float deltaTime = timeS - time;
        int bool = 1;



        //Point
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        //glfwGetCursorPos(window, &xpos, &ypos);
        //printf("x and y position of mouse: %f, %f\n", xpos, ypos);



        for (int i = 0; i < p->size; i++)
        {
            addDensity(p, mouseX, mouseY, 100.0f);
            mouseX0 = mouseX;
            mouseY0 = mouseY;
            glfwSetCursorPosCallback(window, cursor_position_callback);
            float preX = mouseX - mouseX0;
            float preY = mouseY - mouseY0;
            addVelocity(p, mouseX, mouseY, preX, preY);
        }

        for (int i = 0; i < p->size; i++)
        {
            printf("Test density %f, \n", p->density[i]);
        }
        //addDensity(p, mouseX, mouseY, 100.0f);
        //mouseX0 = mouseX;
        //mouseY0 = mouseY;
        //glfwSetCursorPosCallback(window, cursor_position_callback);
        //float preX = mouseX - mouseX0;
        //float preY = mouseY - mouseY0;
        //addVelocity(p, mouseX, mouseY, preX, preY);
        //printf("mouseX, mouseX0, mouseY, mouseY0: %f, %f, %f, %f\n", mouseX, mouseX0, mouseY, mouseY0);




        //glViewport(0, 0, width, height);
        //glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(program);

        //Initilize all particles with a vector
        //addVelocity(p, x, y, x+0.1f, y+0.1f);

        //x = x + tStep * deltaTime;
        //y = y + tStep * deltaTime;
        //*p->vertX = x;
        //*p->vertY = y;

        particleStep(p);
        //printf("densityAFTER: %f\n", *p->density);
        //printf("densityBEFORE: %f\n", *p->density0);
        for (int i = 0; i < p->size; i++)
        {
            printf("Test density %f, \n", p->density[i]);
        }
        //for (int x = 0; x < p->size; x++)
        //{
        //    printf("densityAFTER: %f\n", *p->density);
        //    printf("densityBEFORE: %f\n", *p->density0);
        //}

        //particleStep(p);
        //while (bool == 1)
        //{
            //printf("Value of size in particle %d\n", p->size);
        draw(p);
        //}
        
        //printf("velocities of x and y: %f, %f\n", *p->VelX, *p->VelY);
        //printf("Densities after and before: %f, %f\n", *p->density, *p->density0);

        
        //if (x > 400.0f)
        //{
        //    x = 0.0f;
        //}
        //for (int x = 0; x < p->size; x++)
        //{
        //    draw(*p->VelX, *p->VelY);
        //}
        
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