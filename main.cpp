#include "library.h"

double mousePosX, mousePosY;
void resize(int,int);
void display();
void keyboard(unsigned char key, int x, int y);
void glutMouse(int button, int state, int x, int y);
void glutMotion(int, int);
void Timer(int unused);

int main(int argc, char** argv){
	glutInit(&argc, argv);
    // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowSize(width , height);
    glutInitWindowPosition( 100, 100 );
    glutCreateWindow("Mass Spring Demo");

    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_DEPTH_BUFFER_BIT);
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutTimerFunc(timeStep, Timer, 0);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(glutMouse);
    glutMotionFunc(glutMotion);

    glutMainLoop();
    return 0;
}

void display(){
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glPolygonMode(GL_BACK,GL_FILL);
    // glPolygonMode(GL_FRONT,GL_FILL);

    glColor3f(255,0,0);
    glRectf(-25,25,25,-25);

    glutSwapBuffers();
}
void resize(int w, int h) {
    gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-width/2,width/2,-height/2,height/2,1,-1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Timer(int unused)
{
    glutPostRedisplay();
    glutTimerFunc(timeStep, Timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            fprintf(out,"Thank you.\n");
            exit(0);
            break;
        default:
            break;
 	}
 }

 void glutMouse(int button, int state, int x, int y){
    curMouseX = x, curMouseY = y;
    switch ( button )
    {
        case GLUT_LEFT_BUTTON:

            if ( state == GLUT_DOWN )
            {
                mousePosX = x;
                mousePosY = y;
            }
            else if ( state == GLUT_UP )
            {
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if ( state == GLUT_DOWN ){
                mousePosX = x;
                mousePosY = y;
            }
            else if (state == GLUT_UP){
            }
            break;
        case 3:break;
        default:break;
    }
    return;
}
void glutMotion(int x, int y){
    return;
}