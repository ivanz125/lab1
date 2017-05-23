/*
--------CONTROLS----------
* Set Ellipsoid as active shape - U
* Set Cone as active shape - I
* Set both shapes as active - B
* Change size - +/-
* Move - arrows
* Rotate - WASD QE
* Change rotation mode:
* Self axes - 1
* Global axes - 2
* Active shape around not active - 3
* Change colors - C
* Change roatation angle step - M
*/

#include <GL/glut.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>

#define GL_BGR   0x80E0
#define pi (3.14)

#define ELLIPSOID (0)
#define CONE (1)
#define	BOTH (2)

enum Axis{
	X,Y,Z
};

typedef struct _Point{
	float x;
	float y;
	float z;

	_Point(): x(0), y(0), z(0){}
	_Point(float x, float y, float z): x(x), y(y), z(z){};
} Point;

typedef struct _Triangle{
	Point a;
	Point b;
	Point c;

	_Triangle();
	_Triangle(Point a, Point b, Point c): a(a), b(b), c(c){};
} Triangle;

// Size scale
float sizeEllipsoid = 0.5;
float sizeCone = 0.5;

// Color (RGB 0-1)
float colorEllipsoid[3];
float colorCone[3];

// Position
Point positionEllipsoid;
Point positionCone;

// Current rotation angle
int angleEllipsoid[2];
int angleCone[2];

int timer = 0;
int state = ELLIPSOID;
int rotateMode = 0;
int angleStep = 4;
int xpos, ypos;


Point getMiddle(Point a, Point b){
	Point c;
	c.x = (a.x + b.x) / 2;
	c.y = (a.y + b.y) / 2;
	c.z = (a.z + b.z) / 2;
	return c;
}


inline float sqr(float a){
	return a*a;
}

void normalizePoint(Point& p, const Point& center, const float a, const float b, const float c){
	float dx = p.x - center.x;
	float dy = p.y - center.y;
	float dz = p.z - center.z;
	float ellipsoidEq = (sqr(dx) / sqr(a)) + (sqr(dy) / sqr(b)) + (sqr(dz) / sqr(c));
	float k = sqrt(1 / ellipsoidEq);
	dx *= k;
	dy *= k;
	dz *= k;
	p.x = center.x + dx;
	p.y = center.y + dy;
	p.z = center.z + dz;
}

void normalizeTriangle(Triangle& t, Point& center, const float a, const float b, const float c){
	normalizePoint(t.a, center, a, b, c);
	normalizePoint(t.b, center, a, b, c);
	normalizePoint(t.c, center, a, b, c);
}

void getTriangles(std::vector<Triangle>& v, Triangle t, int recLevel){
	Point top = t.a;
	Point left = t.b;
	Point right = t.c;
	Point top_left = getMiddle(top, left);
	Point top_right = getMiddle(top, right);
	Point right_left = getMiddle(right, left);
	Triangle t1(top, top_left, top_right);
	Triangle t2(right_left, top_left, top_right);
	Triangle t3(right_left, left, top_left);
	Triangle t4(right_left, right, top_right);
	if (recLevel != 0){
		getTriangles(v, t1, recLevel - 1);
		getTriangles(v, t2, recLevel - 1);
		getTriangles(v, t3, recLevel - 1);
		getTriangles(v, t4, recLevel - 1);
	}
	else {
		v.push_back(t1);
		v.push_back(t2);
		v.push_back(t3);
		v.push_back(t4);
	}
}

void init(void) {
	positionEllipsoid.x = -1.0;
	positionEllipsoid.y = 0.0;
	positionEllipsoid.z = 0.0;
	positionCone.x = 1.0;
	positionCone.y = 0.0;
	positionCone.z = 0.0;

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 20.2, 20.0, 30.0, 0.0 };
	glClearColor(0.13, 0.59, 0.95, 1);
	glShadeModel(GL_FLAT);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
}

void timerFunction(int a) {
	if (timer < 350) {
		timer++;
	}

	glutPostRedisplay();
	glutTimerFunc(25, timerFunction, a);
}

void drawAxes() {
	glTranslatef(0.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glPushMatrix();
	glPopMatrix();

	glBegin(GL_LINES);
	glVertex3f(-10.0f, 0.0f, 0.0f);
	glVertex3f(10.0f, 0.0f, 0.0f);

	glVertex3f(0.0f, -10.0f, 0.0f);
	glVertex3f(0.0f, 10.0f, 0.0f);
	glEnd();
}

void drawCone(float size) {
	glTranslatef(positionCone.x, positionCone.y, positionCone.z);
	glColor3f(colorCone[0], colorCone[1], colorCone[2]);
	glPushMatrix();
	glPopMatrix();

	glRotatef(angleCone[0], 1.0, 0.0, 0.0);
	glRotatef(angleCone[1], 0.0, 1.0, 0.0);

	int CONE_PIECES = 30;
	
	glBegin(GL_POLYGON);
	glNormal3f(0, 0, 0.5f);

	float angle = 0;
	float angleStep = 360.0 / CONE_PIECES / 180.0 * pi;
	for (int i = 0; i < CONE_PIECES; i++){
		float x = size * cos(angle);
		float y = size * sin(angle);
		angle += angleStep;
		glVertex3f(x, y, 0.5*size);
	}
	glEnd();
	
	glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 2);
	for (int i = 0; i < CONE_PIECES; i++){
		float x = size * cos(angle);
		float y = size * sin(angle);
		angle += angleStep;
		glVertex3f(x, y, 0.5*size);
	}
	glEnd();
}

void drawEllipsoid(float size){
	glTranslatef(positionEllipsoid.x, positionEllipsoid.y, positionEllipsoid.z);
	glColor3f(colorEllipsoid[0], colorEllipsoid[1], colorEllipsoid[2]);
	glPushMatrix();
	glPopMatrix();

	glRotatef(angleEllipsoid[0], 1.0, 0.0, 0.0);
	glRotatef(angleEllipsoid[1], 0.0, 1.0, 0.0);

	// Make two pyramids, split sides into small triangles and normalize distance of each point relative to center

	// Upper pyramid
	Triangle t1(Point(0, 1, 0), Point(1, 0, 1), Point(-1, 0, 1));
	Triangle t2(Point(0, 1, 0), Point(1, 0, -1), Point(-1, 0, -1));
	Triangle t3(Point(0, 1, 0), Point(1, 0, 1), Point(1, 0, -1));
	Triangle t4(Point(0, 1, 0), Point(-1, 0, 1), Point(-1, 0, -1));
	// Lower pyramid
	Triangle t5(Point(0, -1, 0), Point(1, 0, 1), Point(-1, 0, 1));
	Triangle t6(Point(0, -1, 0), Point(1, 0, -1), Point(-1, 0, -1));
	Triangle t7(Point(0, -1, 0), Point(1, 0, 1), Point(1, 0, -1));
	Triangle t8(Point(0, -1, 0), Point(-1, 0, 1), Point(-1, 0, -1));

	std::vector< std::vector<Triangle> > vt(8, std::vector<Triangle>());
	
	int recLevel = 4;
	getTriangles(vt[0], t1, recLevel);
	getTriangles(vt[1], t2, recLevel);
	getTriangles(vt[2], t3, recLevel);
	getTriangles(vt[3], t4, recLevel);
	getTriangles(vt[4], t5, recLevel);
	getTriangles(vt[5], t6, recLevel);
	getTriangles(vt[6], t7, recLevel);
	getTriangles(vt[7], t8, recLevel);

	// Ellipsoid parameters
	Point center(0.0f, 0.0f, 0.0f);
	float a = size * 0.75f; // x-radius
	float b = size * 1.9f; // y-radius
	float c = size * 0.97f; // z-radius

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < vt.size(); i++){
		for (int j = 0; j < vt[i].size(); j++){
			Triangle ct = vt[i][j];
			normalizeTriangle(ct, center, a, b, c);
			glVertex3f(ct.a.x, ct.a.y, ct.a.z);
			glVertex3f(ct.b.x, ct.b.y, ct.b.z);
			glVertex3f(ct.c.x, ct.c.y, ct.c.z);
		}
	}
	glEnd();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 4.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);

	glScalef(1.0, 1.0, 1.0);

	drawAxes();

	// Ellipsoid
	glPushMatrix();
	glRotatef(1.0, 5.0, 0.0, 0.0);
	drawCone(sizeCone);
	glPopMatrix();

	// Cone
	glPushMatrix();
	glRotatef(11.0, 3.0, 0.0, 0.0);
	drawEllipsoid(sizeEllipsoid);
	glPopMatrix();

	glFlush();
}

void rotateGlobalEllipsoid(int axis, int angle, Point center){
	float angle_x, angle_y, angle_z, radius;
	if (axis == Y){
		radius = sqrt(sqr(center.x - positionEllipsoid.x) + sqr(center.z - positionEllipsoid.z));
		if (radius == 0){
			angleEllipsoid[1] += angle;
			return;
		}
		angle_x = acos((-center.x + positionEllipsoid.x) / radius);
		if ((-center.z + positionEllipsoid.z) < 0) angle_x = 2 * pi - angle_x;
		angle_x += angle*pi / 180; 
		positionEllipsoid.x = center.x + radius*cos(angle_x);
		positionEllipsoid.z = center.z + radius*sin(angle_x);
	}
	else if (axis == X){
		radius = sqrt(sqr(center.y - positionEllipsoid.y) + sqr(center.z - positionEllipsoid.z));
		if (radius == 0){
			angleEllipsoid[0] += angle;
			return;
		}
		angle_y = acos((-center.y + positionEllipsoid.y) / radius);
		if ((-center.z + positionEllipsoid.z) < 0) angle_y = 2 * pi - angle_y;
		angle_y += angle*pi / 180;
		positionEllipsoid.y = center.y + radius*cos(angle_y);
		positionEllipsoid.z = center.z + radius*sin(angle_y);
	}
	else {
		radius = sqrt(sqr(center.x - positionEllipsoid.x) + sqr(center.y - positionEllipsoid.y));
		if (radius == 0){
			angleEllipsoid[2] += angle;
			return;
		}
		angle_z = acos((-center.y + positionEllipsoid.y) / radius);
		if ((-center.x + positionEllipsoid.x) < 0) angle_z = 2 * pi - angle_z;
		angle_z += angle*pi / 180;
		positionEllipsoid.y = center.y + radius*cos(angle_z);
		positionEllipsoid.x = center.x + radius*sin(angle_z);
	}
}

void rotateGlobalEllipsoid(int axis, int angle){
	rotateGlobalEllipsoid(axis, angle, Point(0, 0, 0));
}

void rotateEllipsoidAroundCone(int axis, int angle){
	rotateGlobalEllipsoid(axis, angle, positionCone);
}

void rotateGlobalCone(int axis, int angle, Point center){
	float angle_x, angle_y, angle_z, radius;
	if (axis == Y){
		radius = sqrt(sqr(center.x - positionCone.x) + sqr(center.z - positionCone.z));
		if (radius == 0){
			angleCone[1] += angle;
			return;
		}
		angle_x = acos((-center.x + positionCone.x) / radius);
		if ((-center.z + positionCone.z) < 0) angle_x = 2 * pi - angle_x;
		angle_x += angle*pi / 180; 
		positionCone.x = center.x + radius*cos(angle_x);
		positionCone.z = center.z + radius*sin(angle_x);
	}
	else if (axis == X){
		radius = sqrt(sqr(center.y - positionCone.y) + sqr(center.z - positionCone.z));
		if (radius == 0){
			angleCone[0] += angle;
			return;
		}
		angle_y = acos((-center.y + positionCone.y) / radius);
		if ((-center.z + positionCone.z) < 0) angle_y = 2 * pi - angle_y;
		angle_y += (float)(angle*pi) / 180;
		positionCone.y = center.y + radius*cos(angle_y);
		positionCone.z = center.z + radius*sin(angle_y);
	}
	else {
		radius = sqrt(sqr(center.x - positionCone.x) + sqr(center.y - positionCone.y));
		if (radius == 0){
			angleCone[2] += angle;
			return;
		}
		angle_z = acos((-center.y + positionCone.y) / radius);
		if ((-center.x + positionCone.x) < 0) angle_z = 2 * pi - angle_z;
		angle_z += angle*pi / 180;
		positionCone.y = center.y + radius*cos(angle_z);
		positionCone.x = center.x + radius*sin(angle_z);
	}
}

void rotateGlobalCone(int axis, int angle){
	rotateGlobalCone(axis, angle, Point(0, 0, 0));
}

void rotateConeAroundEllipsoid(int axis, int angle){
	rotateGlobalCone(axis, angle, positionEllipsoid);
}

void reshape(int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
	glMatrixMode(GL_MODELVIEW);

}

void setColor(){
	float r,g,b;
	std::cout << "Enter color of ellipsoid (R G B, 0-255)\nCurrent color: "
		<< (int)(colorEllipsoid[0]*255) << " " << (int)(colorEllipsoid[1]*255) << " " << (int)(colorEllipsoid[2]*255) << "\n";
	std::cin >> r >> g >> b;
	colorEllipsoid[0] = r / 255.0f;
	colorEllipsoid[1] = g / 255.0f;
	colorEllipsoid[2] = b / 255.0f;
	std::cout << "Enter color of cone (R G B, 0-255)\nCurrent color: "
		<< (int)(colorCone[0]*255) << " " << (int)(colorCone[1]*255) << " " << (int)(colorCone[2]*255) << "\n";
	std::cin >> r >> g >> b;
	colorCone[0] = r / 255.0f;
	colorCone[1] = g / 255.0f;
	colorCone[2] = b / 255.0f;
}

void setAngle(){
	std::cout << "Enter rotating angle step (1-60). Current step: " << angleStep << "\n";
	std::cin >> angleStep;
	angleStep = std::max(angleStep, 1);
	angleStep = std::min(angleStep, 60);
}

void rotateLocal(int coordIndex, int positive){
	if (coordIndex == 2) return;
	switch (state){
		case ELLIPSOID: angleEllipsoid[coordIndex] += positive * angleStep; break;
		case CONE: angleCone[coordIndex] += positive * angleStep; break;
		case BOTH: angleEllipsoid[coordIndex] += positive * angleStep; angleCone[coordIndex] += positive * angleStep; break;
	}
}

void rotateGlobal(Axis axis, int angle){
	switch (state){
		case ELLIPSOID: rotateGlobalEllipsoid(axis, angle); break;
		case CONE: rotateGlobalCone(axis, angle); break;
		case BOTH: rotateGlobalEllipsoid(axis, angle); rotateGlobalCone(X, angle); break;
	}
}

void rotateAround(Axis axis, int angle){
	switch (state){
		case ELLIPSOID: rotateEllipsoidAroundCone(axis, angle); break;
		case CONE: rotateConeAroundEllipsoid(axis, angle); break;
	}
}

void controlKeyboard(unsigned char key, int x, int y){
	switch (key){
	case 'c': setColor(); break;
	case 'm': setAngle(); break;
	case 'i': state = CONE; break;
	case 'u': state = ELLIPSOID; break;
	case 'b': state = BOTH; break;
	case '1': rotateMode = 0; break;
	case '2': rotateMode = 1; break;
	case '3': rotateMode = 2; break;
	case '+':
		switch (state){
		case ELLIPSOID: if (sizeEllipsoid <= 1) sizeEllipsoid += 0.1; break;
		case CONE: if (sizeCone <= 1) sizeCone += 0.1; break;
		case BOTH:
			if (sizeEllipsoid <= 1) sizeEllipsoid += 0.1;
			if (sizeCone <= 1) sizeCone += 0.1;
			break;
		}
		break;
	case '-':
		switch (state){
		case ELLIPSOID: if (sizeEllipsoid >= 0.2) sizeEllipsoid -= 0.1; break;
		case CONE: if (sizeCone >= 0.2) sizeCone -= 0.1; break;
		case BOTH:
			if (sizeEllipsoid >= 0.2) sizeEllipsoid -= 0.1;
			if (sizeCone >= 0.2) sizeCone -= 0.1;
			break;
		}
		break;
	case 'w':
		if (rotateMode == 0) rotateLocal(0, 1);
		else if (rotateMode == 1) rotateGlobal(X, angleStep);
		else rotateAround(X, angleStep);
		break;
	case 's':
		if (rotateMode == 0) rotateLocal(0, -1);
		else if (rotateMode == 1) rotateGlobal(X, -angleStep);
		else rotateAround(X, -angleStep);
		break;
	case 'd':
		if (rotateMode == 0) rotateLocal(1, 1);
		else if (rotateMode == 1) rotateGlobal(Y, angleStep);
		else rotateAround(Y, angleStep);
		break;
	case 'a':
		if (rotateMode == 0) rotateLocal(1, -1);
		else if (rotateMode == 1) rotateGlobal(Y, -angleStep);
		else rotateAround(Y, -angleStep);
		break;
	case 'q':
		if (rotateMode == 0) rotateLocal(2, -1);
		else if (rotateMode == 1) rotateGlobal(Z, -angleStep);
		else rotateAround(Z, -angleStep);
		break;
	case 'e':
		if (rotateMode == 0) rotateLocal(2, 1);
		else if (rotateMode == 1) rotateGlobal(Z, angleStep);
		else rotateAround(Z, angleStep);
		break;
	}

}

void specialKeyboard(int key, int x, int y)
{
	switch (key){
	case GLUT_KEY_DOWN:
		switch (state){
		case ELLIPSOID: positionEllipsoid.y -= 0.1; break;
		case CONE: positionCone.y -= 0.1; break;
		case BOTH: positionEllipsoid.y -= 0.1; positionCone.y -= 0.1; break;
		}
		break;
	case GLUT_KEY_UP:
		switch (state){
		case ELLIPSOID: positionEllipsoid.y += 0.1; break;
		case CONE: positionCone.y += 0.1; break;
		case BOTH: positionEllipsoid.y += 0.1; positionCone.y += 0.1; break;
		}
		break;
	case GLUT_KEY_LEFT:
		switch (state){
		case ELLIPSOID: positionEllipsoid.x -= 0.1; break;
		case CONE: positionCone.x -= 0.1; break;
		case BOTH: positionEllipsoid.x -= 0.1; positionCone.x -= 0.1; break;
		}
		break;
	case GLUT_KEY_RIGHT:
		switch (state){
		case ELLIPSOID: positionEllipsoid.x += 0.1; break;
		case CONE: positionCone.x += 0.1; break;
		case BOTH: positionEllipsoid.x += 0.1; positionCone.x += 0.1; break;
		}
		break;
	}

}


int main(int argc, char** argv) 
{
	// Initial color
	colorEllipsoid[0] = 1;
	colorEllipsoid[1] = 0;
	colorEllipsoid[2] = 0;
	colorCone[0] = 0;
	colorCone[1] = 1;
	colorCone[2] = 0;


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(200, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(controlKeyboard);
	glutSpecialFunc(specialKeyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(25, timerFunction, 0);
	glutMainLoop();
	return 0;
}
