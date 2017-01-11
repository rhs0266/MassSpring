#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <iostream>
#include <stdio.h>
#include "rhs_math.h"
#include <cmath>
#include <algorithm>
#define TETRA 1
#define HEXA 2
#define OCTA 3
#define DODECA 4
#define ICOSA 5
FILE *out=stdout;

using namespace std;

GLdouble rotMatrix[16] =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

/* set global variables */
int width=700, height=700;
int curMouseX, curMouseY;
int type, dist[6]={0,20,30,40,40,45};
bool leftButton = false;
bool rightButton = false;
bool seekFlag, showStdLine = true;
GLfloat mousePosX, mousePosY;
float PI = 3.141592f, r;
float Near = 5.0f, Far = 700.0f;
float trackballRadius;// = 80.0f;
float buf[701][701];
double fov = 45.0;

position translate;
position pv,v;
matrix rot_mat;
quater Tot;

/* vectors that makes the rotation and translation of the cube */
position eye = position(0.0f, 0.0f, 100.0f);
position ori = position(0.0f, 0.0f, 0.0f);
float rot[3] = { 0.0f, 0.0f, 0.0f };

void loadGlobalCoord()
{
    glLoadIdentity();
    position eye_new, ori_new, up_new;
    eye_new = calc_rotate(Tot,eye) + translate;
    ori_new = translate;
    up_new = calc_rotate(Tot, position(0,1,0));
    gluLookAt(eye_new.p[0], eye_new.p[1], eye_new.p[2], ori_new.p[0], ori_new.p[1], ori_new.p[2], up_new.p[0], up_new.p[1], up_new.p[2]);

    glMultMatrixd(rotMatrix);
}


//------------------------------------------------------------------------
// Moves the screen based on mouse pressed button
//------------------------------------------------------------------------
position ray(position A, position B, position C, float r){ // A+kB on sphere
    position a = B;
    position b = C-A;
    position p = a * (a%b);
    // a is unit vector
    // p is projection vector of b to a

    float dist_p = norm(p);
    position e = b - p; // p + e = b
    float dist_e = norm(e);
    
    if (dist_e >= 0.9999f * r){
        float max_angle = atan2(r, sqrt(norm(b)*norm(b) - r*r));
        position axis = b * p;
        axis = axis / norm(axis);
        quater Q = quater(cos(max_angle/2), axis.p[0]*sin(max_angle/2), axis.p[1]*sin(max_angle/2), axis.p[2]*sin(max_angle/2));
        p = calc_rotate(Q,b);
        p = p / norm(p) * sqrt(norm(b)*norm(b) - r*r);
    }else{
        p = p / norm(p) * (norm(p) - sqrt(r*r - norm(e)*norm(e)));
    }
    return A+p;
}

void print(position x, string name){
    cout << name;
    fprintf(out," = %.3lf, %.3lf, %.3lf\n",x.p[0],x.p[1],x.p[2]);
}

position D2toD3(int x,int y){ // return position of intercept b/w cam & cos(center of sphere)
    x-=width/2; y=height/2-y;
    float camDist = height/2.0 / tan((fov/2.0)*PI/180.0);
    position vec = position((float)x,(float)y,0.0) - position(0.0,0.0,camDist);
    vec=vec/norm(vec);
    vec = calc_rotate(Tot, vec);

    position eye_new, ori_new; 
    eye_new = calc_rotate(Tot,eye) + translate;
    ori_new = translate;
    return ray(eye_new, vec, ori_new, trackballRadius);
}

void glutMotion(int x, int y)
{
    trackballRadius = eye.p[2] * 0.4 * (fov/45);
    pv=D2toD3(mousePosX, mousePosY);
    v =D2toD3(x,y);
    mousePosX = x;
    mousePosY = y;
    curMouseX = x, curMouseY = y;
    
    float theta = angle(v, pv);
    if (leftButton){
        position cross = pv*v;
        if (norm(cross)<=eps) return;
        cross = cross/norm(cross);
        quater Q = quater(cos(theta/2), cross.p[0]*sin(theta/2), cross.p[1]*sin(theta/2), cross.p[2]*sin(theta/2));
        Q = Q.inverse();
        Tot = Q * Tot;
    }
    return;
}

position getPointOnPlane(int x,int y){
    double proj[16],model[16];
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    float winX,winY;
    GLfloat winZ;
    winX = (float)x;
    winY = height - (float)y;
    glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
    winZ/=0.00390619;
    if (winZ>=1-1e-4){ // Pointer doesn't meet object's surface
        return position(9999,9999,9999);
    }
    winZ = 2.0*winZ - 1.0;
    winZ = ((-2.0*Far*Near)/(Far-Near)) / (winZ - (Far + Near) / (Far - Near));
    
    int x2=x-width/2, y2=height/2-y;
    float camDist = height/2.0 / tan((fov/2.0)*PI/180.0);
    position vec = position((float)x2,(float)y2,0.0) - position(0.0,0.0,camDist);
    vec=vec/norm(vec);
    vec = vec * (winZ / abs(vec.p[2]));
    return eye + vec;
}

//------------------------------------------------------------------------
// Function that handles mouse input
//------------------------------------------------------------------------
void glutMouse(int button, int state, int x, int y)
{
    curMouseX = x, curMouseY = y;
    switch ( button )
    {
        case GLUT_LEFT_BUTTON:

            if ( state == GLUT_DOWN )
            {
                mousePosX = x;
                mousePosY = y;
                leftButton = true;
                if (seekFlag == true){
                    seekFlag = false;
                    position newCenter = getPointOnPlane(x,y);
                    if (9999-1e-4<=newCenter.p[0] && newCenter.p[0]<=9999+1e-4){
                    }else{
                        translate = translate + calc_rotate(Tot, (newCenter - ori));
                    }
                } 
            }
            else if ( state == GLUT_UP )
            {
                leftButton = false;
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if ( state == GLUT_DOWN ){
                mousePosX = x;
                mousePosY = y;
                rightButton = true;
            }
            else if (state == GLUT_UP){
                rightButton = false;
            }
            break;
        case 3:break;
        default:break;
    }
    return;
}
int TOP=5;
float ANGLE = -30.0;
float dt=0.0f, alpha=0.005;
bool last=false;
void Draw_dodecahedron(int level){
    float len = 10.0f;
    float PI = 3.141592f, x, y;
    float turn=-36.0f;
    float tl[5][2]={{len,0.0f},
                    {len+len*cos(72.0f*PI/180.0), len*sin(72.0f*PI/180.0f)},
                    {len/2.0f, len*(sin(72.0f*PI/180.0f)+sin(36.0f*PI/180.0f))},
                    {-len*cos(72.0f*PI/180.0f), len*sin(72.0f*PI/180.0f)},
                    {0,0}};
    int i;
    if (level==4) last=false;
    if (level==0) return;
    
    glColor3f(0,0,0.6f);
    glBegin(GL_TRIANGLES);
    {
        for (i=0;i<=2;i++){
            glVertex3f(0,0,0);
            glVertex3f(tl[i][0],tl[i][1],0);
            glVertex3f(tl[i+1][0],tl[i+1][1],0);
        }
    }
    glEnd();
    glColor3f(1,1,1);
    glBegin(GL_LINE_STRIP);
    {
        for (i=0;i<=5;i++){
            glVertex3f(tl[i%5][0],tl[i%5][1],-0.1f);
        }
    }
    glEnd();


    for (i=0;i<5;i++){
        if (level==2 && last==true) continue;
        else if (level==2 && last==false && i!=1) continue;
        else if (level==2 && last==false && i==1){ last=true; turn=36.0f; }
        else if (level!=4 && i>=1) continue;
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-63.435*dt, -len*cos(72.0f*PI/180.0), len*sin(72.0f*PI/180.f), 0.0f);
            Draw_dodecahedron(level-1);
            glRotatef( 63.435*dt, -len*cos(72.0f*PI/180.0), len*sin(72.0f*PI/180.f), 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
        turn+=72.0f;
    }
}
void Draw_tetrahedron(int level){
    float len = 10.0f;
    float PI = 3.141592f, x, y;
    float turn= 60.0f;
    float tl[3][2]={{len,0.0f},
                    {len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.0f)},
                    {0,0}};
    int i;
    if (level==0) return;
    
    glColor3f(0,0.6f,0);
    glBegin(GL_TRIANGLES);
    {
        for (i=0;i<3;i++){
            glVertex3f(tl[i][0],tl[i][1],0.0f);
        }
    }
    glEnd();

    glColor3f(1,1,1);
    glBegin(GL_LINE_STRIP);
    {
        for (i=0;i<=3;i++){
            glVertex3f(tl[i%3][0],tl[i%3][1],-0.1f);
        }
    }
    glEnd();


    for (i=0;i<3;i++){
        if (level==1) continue;
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-109.472*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            Draw_tetrahedron(level-1);
            glRotatef( 109.472*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
        turn+=120.0f;
    }
}
void Draw_hexahedron(int level){
    float len = 10.0f;
    float PI = 3.141592f, x, y;
    float turn= 0.0f;
    float tl[4][2]={{len,0.0f},
                    {len,len},
                    {0,len},
                    {0,0}};
    int i;
    if (level==3) last=false;;
    if (level==0) return;
    
    glColor3f(0.6f,0.6f,0);
    glBegin(GL_QUADS);
    {
        for (i=0;i<4;i++){
            glVertex3f(tl[i][0],tl[i][1],0.0f);
        }
    }
    glEnd();

    glColor3f(1,1,1);
    glBegin(GL_LINE_STRIP);
    {
        for (i=0;i<=4;i++){
            glVertex3f(tl[i%4][0],tl[i%4][1],-0.1f);
        }
    }
    glEnd();


    for (i=0;i<4;i++){
        if (level==1) continue;
        else if (level==2 && i!=0) continue;
        else if (level==2 && i==0 && last==true) continue;
        else if (level==2 && i==0 && last==false) last=true;
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-90.0*dt, 0, 1.0f, 0.0f);
            Draw_hexahedron(level-1);
            glRotatef( 90.0*dt, 0, 1.0f, 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
        turn+=90.0f;
    }
}

void Draw_octahedron(int level){
    float len = 10.0f;
    float PI = 3.141592f, x, y;
    float turn= 60.0f;
    float tl[3][2]={{len,0.0f},
                    {len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.0f)},
                    {0,0}};
    int i;
    if (level==0) return;
    
    glColor3f(0.6f,0.0f,0.6f);
    glBegin(GL_TRIANGLES);
    {
        for (i=0;i<3;i++){
            glVertex3f(tl[i][0],tl[i][1],0.0f);
        }
    }
    glEnd();

    glColor3f(1,1,1);
    glBegin(GL_LINE_STRIP);
    {
        for (i=0;i<=3;i++){
            glVertex3f(tl[i%3][0],tl[i%3][1],-0.1f);
        }
    }
    glEnd();
    if (level==-1) return;

    
    if (level%2==0){ i=0; turn=60.0f; }
    else{ i=2; turn=300.0f; }
    {
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-70.528*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            Draw_octahedron(level-1);
            glRotatef( 70.528*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
    }
    if (level==6){ i=2; turn=300.0f; }
    if (level==1){ i=0; turn=60.0f; }
    if (level==6 || level==1){
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-70.528*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            Draw_octahedron(-1);
            glRotatef( 70.528*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
    }
}

void Draw_icosahedron(int level){
    float len = 10.0f;
    float PI = 3.141592f, x, y;
    float turn= 60.0f;
    float tl[3][2]={{len,0.0f},
                    {len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.0f)},
                    {0,0}};
    int i;
    if (level==0) return;
    
    glColor3f(0.5f,0.4f,0.2f);
    glBegin(GL_TRIANGLES);
    {
        for (i=0;i<3;i++){
            glVertex3f(tl[i][0],tl[i][1],0.0f);
        }
    }
    glEnd();

    glColor3f(1,1,1);
    glBegin(GL_LINE_STRIP);
    {
        for (i=0;i<=3;i++){
            glVertex3f(tl[i%3][0],tl[i%3][1],-0.1f);
        }
    }
    glEnd();
    if (level==-1) return;

    
    if (level%2==0){ i=0; turn=60.0f; }
    else{ i=2; turn=300.0f; }
    {
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-41.815315*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            Draw_icosahedron(level-1);
            glRotatef( 41.815315*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
    }
    if (level%2==0){ i=2; turn=300.0f; }
    else{ i=0; turn=60.0f; }
    {
        glPushMatrix();
        {
            glTranslatef( tl[i][0], tl[i][1], 0.0f);
            glRotatef( turn, 0.0f, 0.0f, 1.0f);
            glRotatef(-41.815315*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            Draw_icosahedron(-1);
            glRotatef( 41.815315*dt, len*cos(60.0f*PI/180.0), len*sin(60.0f*PI/180.f), 0.0f);
            glRotatef(-turn, 0.0f, 0.0f, 1.0f);
            glTranslatef(-tl[i][0],-tl[i][1], 0.0f);
        }
        glPopMatrix();
    }
}

typedef float F;
GLfloat vert[]={0,0,0, 10,0,0, 10,10,0, 0,10,0};

void Draw_Sphere(){
    float r = trackballRadius;

    FOR (i, 1, 90){ // 360/90 * i ~ 360/90 * (i+1)
        glColor3f(0.4,0.4,0.4);
        glBegin(GL_POINTS);
        FOR (j,1,4){
            int idx=4*i + j;
            position t = position(cos(idx*PI/180)*r, sin(idx*PI/180)*r, 0.0f)+translate;
            t = calc_rotate(Tot, t);
            glVertex3f(t.p[0],t.p[1],t.p[2]);
        }
        glEnd();
    }
}

void Draw_Mouse(){
    float r = trackballRadius;
    position mouse = D2toD3(curMouseX, curMouseY) - translate;
    float phi = atan2(sqrt(r*r-mouse.p[2]*mouse.p[2]), mouse.p[2]);
    float theta = atan2(mouse.p[1]/sin(phi)/r, mouse.p[0]/sin(phi)/r);

    glColor3f(0.7,0.7,0.7);
    glBegin(GL_POINTS);
    for (float t=-0.2;t<=+0.2;t+=0.001){
        position T = position(r*cos(theta+t)*sin(phi), r*sin(theta+t)*sin(phi), r*cos(phi));
        glVertex3f(T.p[0],T.p[1],T.p[2]);
    }
    for (float t=-0.05;t<=+0.05;t+=0.001){
        position T = position(r*cos(theta)*sin(phi+t), r*sin(theta)*sin(phi+t), r*cos(phi+t));
        glVertex3f(T.p[0],T.p[1],T.p[2]);
    }
    glEnd();
}

void display() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (GLfloat)width / (GLfloat)height, Near, Far);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_BACK,GL_FILL);
    glPolygonMode(GL_FRONT,GL_LINE);
    loadGlobalCoord();

    Draw_Sphere();
    Draw_Mouse();

    if (showStdLine){
        glBegin(GL_LINE_STRIP);
        position newori = calc_rotate(Tot, ori) + translate;
        position newup= calc_rotate(Tot, position(0,1,0));
        glVertex3f(newori.p[0],newori.p[1],newori.p[2]);
        newori = newori + newup * 10;
        glVertex3f(newori.p[0],newori.p[1],newori.p[2]);
        glEnd();
    }

    switch (type){
        case TETRA:
            glTranslatef(-4,-4,-4);
            Draw_tetrahedron(2);
            break;
        case HEXA:
            glTranslatef(-5,-5,-5);
            Draw_hexahedron(3);
            break;
        case OCTA:
            glTranslatef(-5,-5,-5);
            glTranslatef(-15+15*dt,0,0);
            Draw_octahedron(6);
            break;
        case DODECA:
            glTranslatef(-7,-7,-7);
            glTranslatef(-20+20*dt,0,-20+20*dt);
            Draw_dodecahedron(4);
            break;
        case ICOSA:
            glTranslatef(-8,-8,-8);
            glTranslatef(-20+20*dt,0,0);
            Draw_icosahedron(10);
        default:
            break;
    }
    glutSwapBuffers();
}

void resize(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (GLfloat)w / (GLfloat)h, .1f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        fprintf(out,"Thank you.\n");
        exit(0);
        break;
    case '1': // tetrahedron
        dt=0.0f;
        type = TETRA;
        break;
    case '2': // hexahedron
        dt=0.0f;
        type=HEXA;
        break;
    case '3': // octahedron
        dt=0.0f;
        type=OCTA;
        break;
    case '4': // dodecahedron
        dt=0.0f;
        type=DODECA;
        break;
    case '5': // icosahedron
        dt=0.0f;
        type=ICOSA;
        break;
    case ' ': // spacebar for resetting view
        eye=position(0.0f,0.0f,100.0f);
        ori=position(0.0f,0.0f,0.0f);
        rot[0]=0.0f; rot[1]=0.0f; rot[2]=0.0f;
        translate = position(0,0,0);
        fov=45;
        Tot=quater();
        break;
    case 'w': // 'w' view up translate
        translate = translate + calc_rotate(Tot, position(0,-0.5,0));
        break;
    case 's': // 's' view down translate
        translate = translate + calc_rotate(Tot, position(0,+0.5,0));
        break;
    case 'a': // 'a' view left translate
        translate = translate + calc_rotate(Tot, position(+0.5,0,0));
        break;
    case 'd': // 'd' view right translate
        translate = translate + calc_rotate(Tot, position(-0.5,0,0));
        break;
    case 'f': // 'f' ready for getting mousepoint for 'seek'
        seekFlag = true;
        break;
    case 'b': // 'b' move camera backward to show all
        translate = position(0,0,0);
        eye=position(0.0f,0.0f,30.0f / tan((fov/2)*PI/180.0));
        //cout << 30.0f / tan((fov/2)*PI/180.0) << "\n";
        break;
    case 'l': // 'l' show Standard Line for check center of rotation
        showStdLine = !showStdLine;
        break;
    case '[': // '[' view dolly in
        if (eye.p[2]<=5) break;
        eye.p[2]-=1;
        break;
    case ']': // ']' view dolly out
        if (eye.p[2]>=300) break;
        eye.p[2]+=1;
        break;
    case ';': // ';' view zoom in
        if (fov<=5) break;
        fov -= 1.0;
        break;
    case '\'': // ''' view zoom out
        if (fov>=90) break;
        fov += 1.0;
        break;
    default:
        break;
    }
}


unsigned timeStep = 30;
void Timer(int unused)
{
    //if (dt>0.995f) alpha=-0.005f;
    //if (dt<0.005f) alpha=0.005f; 
    if (dt<=0.995f) dt+=alpha;
    glutPostRedisplay();
    glutTimerFunc(timeStep, Timer, 0);
}

void ManualPrint(){
    fprintf(out,"MANUAL FOR PROGRAM, 2014-16371 Ryu Ho Seok\n");
    fprintf(out,"------------------\n");
    fprintf(out,"[ 1 ] : tetrahedron\n");
    fprintf(out,"[ 2 ] : hexahedron\n");
    fprintf(out,"[ 3 ] : octahedron\n");
    fprintf(out,"[ 4 ] : dodecahedron\n");
    fprintf(out,"[ 5 ] : icosahedron\n");
    fprintf(out,"------------------\n");
    fprintf(out,"[ a ] : move left\n");
    fprintf(out,"[ s ] : move down\n");
    fprintf(out,"[ d ] : move right\n");
    fprintf(out,"[ w ] : move up\n");
    fprintf(out,"------------------\n");
    fprintf(out,"[ [ ] : dolly in,  exists maximum range\n");
    fprintf(out,"[ ] ] : dolly out, exists maximum range\n");
    fprintf(out,"[ ; ] : zoom in,   exists maximum range\n");
    fprintf(out,"[ ' ] : zoom out,  exists maximum range\n");
    fprintf(out,"[ b ] : show all,  if cannot see anything, press 'space bar'\n");
    fprintf(out,"[ f ] : seek, after press f, click mouse on the surface. if not, nothing happens\n");
    fprintf(out,"[ l ] : show or hide standard line for checking the center of rotation\n");
    fprintf(out,"------------------\n");
    fprintf(out,"[ space bar ] : reset fov, cam dist, translate, rotation\n");
    fprintf(out,"[ esc ] : exit program.\n");
}

int main(int argc, char **argv) {
    ManualPrint();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width , height);
    glutInitWindowPosition( 50, 0 );
    glutCreateWindow("Platonic Solid");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_BUFFER_BIT);
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutTimerFunc(timeStep, Timer, 0);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(glutMouse);
    glutMotionFunc(glutMotion);

    glutMainLoop();
    return 0;
}
