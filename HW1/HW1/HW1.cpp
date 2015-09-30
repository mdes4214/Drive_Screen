#include <stdlib.h>
#include <gluit.h>
#include "svl/svl.h"

#pragma comment(lib,"svl-vc9.lib")
#pragma comment (lib, "gluit-vc9.lib")
#pragma comment (lib, "glpngd-vc9.lib")

int gw = 800;
int gh = 600;
float x, z;    // current pos
float current_origin_x, current_origin_z;

float angle;
float _distance;
float speed = 0.0;
float maxspeed = 5.0;

char title[40];
Vec3 point[3];
int view_motion = 0;
int count = 0;

GLMmodel *car;


void reshape(int w, int h)
{
	gw = w;
	gh = h;
}

void grid(int s)
{
#define MIN -20
#define MAX 20

	int i;
	glColor3ub(255, 255, 255);
	glBegin(GL_LINES);
	for(i = MIN; i <= MAX; i += s){
		glVertex3i(i, 0, MIN);
		glVertex3i(i, 0, MAX);
		glVertex3i(MIN, 0, i);
		glVertex3i(MAX, 0, i);
	}
	glEnd();

	int space = (MAX - MIN) / 4;
	glColor3f(1, 1, 0);
	glBegin(GL_QUADS);
	for(i = MIN + space; i <= MAX - space; i += space){
		glVertex3f(MIN + space - 0.5, 0.05, i - 0.5);
		glVertex3f(MAX - space + 0.5, 0.05, i - 0.5);
		glVertex3f(MAX - space + 0.5, 0.05, i + 0.5);
		glVertex3f(MIN + space - 0.5, 0.05, i + 0.5);
		glVertex3f(i - 0.5, 0.05, MAX - space);
		glVertex3f(i - 0.5, 0.05, MIN + space);
		glVertex3f(i + 0.5, 0.05, MIN + space);
		glVertex3f(i + 0.5, 0.05, MAX - space);
	}
	glEnd();
}

void display()
{
#define min(a, b) ((a < b)? a: b)

	glutSetWindowTitle(title);

	//Viewport_main
	glViewport(0, gh / 3.0, gw, gh / 3.0 * 2);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, 2, 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	

	Vec3 tpoint[3], vpoint[3], vspoint[3];

	x = current_origin_x + _distance * cos(angle * vl_pi / 180);
	z = current_origin_z + _distance * sin(angle * vl_pi / 180);

	for(int i = 0; i < 3; i++){
		tpoint[i] = proj(HTrans4(Vec3(x, 0, z)) * HRot4(Vec3(0, -1, 0), angle / 180 * vl_pi) * Vec4(point[i], 1));
		vpoint[i] = proj(HTrans4(Vec3(x, 0, z)) * HRot4(Vec3(0, -1, 0), (angle - 21) / 180 * vl_pi) * Vec4(point[i], 1));
		vspoint[i] = proj(HTrans4(Vec3(x, 0, z)) * HRot4(Vec3(0, -1, 0), angle / 180 * vl_pi) * HTrans4(Vec3(0, 0, -3)) * Vec4(point[i], 1));
	}

	if(!view_motion)
		gluLookAt((vpoint[0][0] + vpoint[2][0]) / 2, vpoint[0][1] + 2, (vpoint[0][2] + vpoint[2][2]) / 2, tpoint[1][0], tpoint[1][1] + 1, tpoint[1][2], 0, 1, 0);
	else
		gluLookAt((vspoint[0][0] + vspoint[2][0]) / 2, vspoint[0][1] + 1, (vspoint[0][2] + vspoint[2][2]) / 2, (tpoint[0][0] + tpoint[2][0]) / 2, tpoint[1][1], (tpoint[0][2] + tpoint[2][2]) / 2, 0, 1, 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_LIGHTING);

	grid(1);

	//glColor3f(0, 1, 1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glPushMatrix();
	glTranslatef(x, 0.35, z);
	glRotatef(angle - 90, 0,-1,0);
	glmDraw(car, GLM_MATERIAL | GLM_SMOOTH);
	glPopMatrix();
	glDisable(GL_LIGHTING);
		
	//Viewport_Text
	glViewport(0, 0, gw * 0.75, gh / 3.0);
	glColor3ub (255, 255, 255);
	BEGIN_2D_OVERLAY(15,15);
	drawstr(1, 13, "Object: (X, Y, Z) = (%.1f, 0, %.1f)", x, z);
	if(!view_motion)
		drawstr(1, 11, "Camera: (X, Y, Z) = (%.1f, %.1f, %.1f)", (vpoint[0][0] + vpoint[2][0]) / 2, vpoint[0][1] + 2, (vpoint[0][2] + vpoint[2][2]) / 2);
	else
		drawstr(1, 11, "Camera: (X, Y, Z) = (%.1f, %.1f, %.1f)", (vspoint[0][0] + vspoint[2][0]) / 2, vspoint[0][1] + 1, (vspoint[0][2] + vspoint[2][2]) / 2);
	drawstr(1, 9, "Speed: %.1f", speed);
	drawstr(1, 7, "Time: %02d:%02d", count / 60, count % 60);
	drawstr(1, 5, "Key:");
	drawstr(2, 3, "[s] to change the mode.");
	drawstr(2, 1, "[up, down] to +/- the speed. [left, right] to turn left and right.");
	END_2D_OVERLAY();

	//Viewport_Map_border
	glViewport(gw * 0.75, 0, gw / 4.0, gh / 3.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt((vpoint[0][0] + vpoint[2][0]) / 2, vpoint[0][1] + 5, (vpoint[0][2] + vpoint[2][2]) / 2, (tpoint[1][0] + (tpoint[0][0] + tpoint[2][0]) / 2) / 2, tpoint[1][1], (tpoint[1][2] + (tpoint[0][2] + tpoint[2][2]) / 2) / 2, 0, 1, 0);

	glColor3ub(220, 220, 220);
	glBegin(GL_QUADS);
	glVertex3i(-300, -2, -300);
	glVertex3i(-300, -2, 300);
	glVertex3i(300, -2, 300);
	glVertex3i(300, -2, -300);
	glEnd();


	//Viewport_Map
	//glViewport(gw * 0.875 - min(gw * 0.1875, gh / 4.0) / 2.0, gh / 3.0 / 2.0 - min(gw * 0.1875, gh / 4.0) / 2.0, min(gw * 0.1875, gh / 4.0), min(gw * 0.1875, gh / 4.0));     //維持小地圖比例的版本
	glViewport(gw / 1.28, gh / 24, gw * 0.1875, gh / 4.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, 1, 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt((vpoint[0][0] + vpoint[2][0]) / 2, vpoint[0][1] + 5, (vpoint[0][2] + vpoint[2][2]) / 2, (tpoint[1][0] + (tpoint[0][0] + tpoint[2][0]) / 2) / 2, tpoint[1][1], (tpoint[1][2] + (tpoint[0][2] + tpoint[2][2]) / 2) / 2, 0, 1, 0);

	glColor3ub(0, 0, 0);
	glBegin(GL_QUADS);
	glVertex3i(-1000, -1, -1000);
	glVertex3i(-1000, -1, 1000);
	glVertex3i(1000, -1, 1000);
	glVertex3i(1000, -1, -1000);
	glEnd();

	grid(2);
	glColor3f(1, 0, 0);
	glPushMatrix();
	glBegin(GL_TRIANGLES);
	tpoint[0][1]++;
	tpoint[1][1]++;
	tpoint[2][1]++;
	for(int i = 0; i < 3; i++)
		glVertex3dv(tpoint[i].Ref());
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}

void idle(void)
{
	static int last;
	int now;
	float dt;

	if(last == 0){
		last = glutGet(GLUT_ELAPSED_TIME);
		return;
	}
	now = glutGet(GLUT_ELAPSED_TIME);
	dt = (now - last) / 1000.;
	last = now;

	_distance += speed*dt;
	glutPostRedisplay();
}

void timer(int dummy)
{
	glutTimerFunc(1000, timer, 0);
	count++;
	glutPostRedisplay();
}

void special(int key, int mx, int my)
{
	switch(key){
	case GLUT_KEY_UP:
		if (speed < maxspeed)
			speed += 0.5;
		break;
	case GLUT_KEY_DOWN:
		if (speed > -5.0)
			speed -= 0.5;
		break;
	case GLUT_KEY_RIGHT:
		current_origin_x = x; current_origin_z = z;
		_distance = 0.0;
		angle += 2;
		break;
	case GLUT_KEY_LEFT:
		current_origin_x = x; current_origin_z = z;
		_distance = 0.0;
		angle -= 2;
		break;
	}
}

void keyboard(unsigned char key, int x, int y)
{
	if(key == 's' || key == 'S'){
		view_motion ^= 1;
		if(!view_motion)
			sprintf(title, "HW1 [s] -- Normal Mode");
		else
			sprintf(title, "HW1 [s] -- Sideview Mode");
		glutPostRedisplay();
	}
}

void init()
{
	point[0] = Vec3(0, 0, 0.5);
	point[1] = Vec3(1.5, 0, 0);
	point[2] = Vec3(0, 0, -0.5);

	
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2, 0.2, 0.2, 1);
	car = glmReadOBJ("porsche.obj");
	glmUnitize(car);
	glmFacetNormals(car);
	glmVertexNormals(car, 90);
	
}


int main (int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(800,600);

	sprintf(title, "HW1 [s] -- Normal Mode");
	glutCreateWindow(title);

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutTimerFunc(0, timer, 0);

	glutMainLoop();
}

