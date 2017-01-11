#pragma once
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <iostream>
#include <stdio.h>
#include <Eigen/Dense>
#include <Eigen/LU>
#include <Eigen/Geometry>
#include <math.h>
FILE *out=stdout;

using namespace std;
using namespace Eigen;

GLdouble rotMatrix[16] =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

/* set global variables */
int width=1000, height=1000;
int curMouseX, curMouseY;
float PI = 3.141592f;
float Near = 5.0f, Far = 700.0f;
float trackballRadius;// = 80.0f;
double fov = 45.0;
unsigned timeStep = 30;

typedef Vector2f V2;
#define glV2(x,y) glVertex3f(x,y,0)
#define V2P(P) glVertex2f(P(0), P(1))