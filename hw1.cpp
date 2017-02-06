//modified by: Jonathan Roman
//date: 1-26-17
//purpose: Intro to git and graphics
//
//cs3350 Spring 2017 Lab-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

extern "C" {
	#include "fonts.h"
}

#define WINDOW_WIDTH  1100
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 99999
#define GRAVITY .7

#define rnd() (float)rand() / (float)RAND_MAX


//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape box[5];
    Shape circle;
    Particle particle[MAX_PARTICLES];
    int n;
    int bubbler;
    int mouse[2];
    Game() { n=0; bubbler=0; }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;

    //declare a box shape
    game.box[0].width = 100;
    game.box[0].height = 10;
    game.box[0].center.x = 400 + 5*65;
    game.box[0].center.y = 500 - 5*60;

    game.box[1].width = 100;
    game.box[1].height = 10;
    game.box[1].center.x = 300 + 5*65;
    game.box[1].center.y = 570 - 5*60;

    game.box[2].width = 100;
    game.box[2].height = 10;
    game.box[2].center.x = 200 + 5*65;
    game.box[2].center.y = 640 - 5*60;

    game.box[3].width = 100;
    game.box[3].height = 10;
    game.box[3].center.x = 100 + 5*65;
    game.box[3].center.y = 720 - 5*60;

    game.box[4].width = 100;
    game.box[4].height = 10;
    game.box[4].center.x = 0 + 5*65;
    game.box[4].center.y = 790 - 5*60;

    game.circle.radius = 200;
    game.circle.center.x =  600 + 5*65;
    game.circle.center.y = 250 - 5*60;



    //start animation
    while (!done) {
	while (XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tcannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
	std::cout << "\n\tno appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask | PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(254, 254, 254, 254);
    //To allow fonts to be viewed
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
    if (game->n >= MAX_PARTICLES)
	return;
    std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p[100];
    for (int i=0; i<100; i++){
	p[i] = &game->particle[game->n];
	p[i]->s.center.x = x;
	p[i]->s.center.y = y;
	p[i]->velocity.y = rnd() * 4.0;
	p[i]->velocity.x =  rnd() * 2.0 - .5;


	game->n++;
    }
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    //static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    for(int i=0; i<10; i++){
		makeParticle(game, e->xbutton.x, y);
	    }
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	int y = WINDOW_HEIGHT - e->xbutton.y;
	if(game->bubbler == 0){
	    game->mouse[0] = savex;
	    game->mouse[1] = y;
	}
	//   for(int i=0; i<5; i++){
	//     makeParticle(game, e->xbutton.x, y);
	// }
	/*		if (++n < 10)
			return;*/
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_b) {
	    game->bubbler = game->bubbler ^ 1;
	    /*if (game->bubbler != 0){
	    // the bubbler is on
	    makeParticle(game,-150 +5*65, 920 - 5*60);//game->mouse[0], game->mouse[1]);
	    }*/

	}

	if (key == XK_Escape) {
	    return 1;
	}

	//You may check other keys here.

    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
	return;

    /*    if (game->bubbler != 0){
    // the bubbler is on
    makeParticle(game,-150 +5*65, 920 - 5*60);//game->mouse[0], game->mouse[1]);
    }
    */

    for(int i=0; i<game->n; i++){
	p = &game->particle[i];
	p->velocity.y -= GRAVITY;
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;
	//check for collision with shapes...
	Shape *s;
	for (int j=0; j<5; j++){
	    s = &game->box[j];
	    if(p->s.center.y < s->center.y + s->height &&
		    p->s.center.y > s->center.y - s->height && 
		    p->s.center.x >= s->center.x - s->width &&
		    p->s.center.x <= s->center.x + s->width){

		p->s.center.y = s->center.y + s->height;
		// Velocity Pushing Particle When On Box
		p->velocity.y = (-p->velocity.y * rnd() -0.5f)/1.4;
		p->velocity.x += .06f;

	    }
	}

	float d0 = (p->s.center.x - game->circle.center.x);
	float d1 = (p->s.center.y - game->circle.center.y);

	d0 = (d0*d0);
	d1 = (d1*d1);
	float pdistance = sqrt(d0 + d1);

	if (pdistance <= game->circle.radius){
		p->velocity.x = -game->circle.center.x/(pdistance);
		p->velocity.y = game->circle.center.y/(pdistance);
		p->velocity.x = p->velocity.x;
		p->velocity.y = p->velocity.y/1.5;
	}

	//check for off-screen
	if (p->s.center.y < 0.0) {
	    std::cout << "off screen" << std::endl;

	    //removing a paricle 
	    game->particle[i] = game->particle[--game->n-1];
	}
    }
}


void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius){
	int i;
	int triangleAmount = 200; //# of triangles used to draw circle
	
	//GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * 3.125;
	
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y); // center of circle
		for(i = 0; i <= triangleAmount;i++) { 
			glVertex2f(
		            x + (radius * cos(i *  twicePi / triangleAmount)), 
			    y + (radius * sin(i * twicePi / triangleAmount))
			);
		}
	glEnd();
}

void render(Game *game)
{
    if (game->bubbler != 0){
	// the bubbler is on
	// Creating Particle(x-axis), (y-axis)
	makeParticle(game,0 +5*65, 850 - 5*60);//game->mouse[0], game->mouse[1]);
	makeParticle(game,3 +5*65, 850 - 5*60);//game->mouse[0], game->mouse[1]);
	makeParticle(game,7 +5*65, 850 - 5*60);//game->mouse[0], game->mouse[1]);
    }


    float w, h;
    Rect r;

    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    glColor3ub(1,1,1);
    drawFilledCircle(game->circle.center.x, game->circle.center.y, game->circle.radius);

    for (int i=0; i<5; i++){
	//draw box
	Shape *s;
	glColor3ub(1,1,1);
	s = &game->box[i];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();

	// Drawing Text Inside The Box
	r.bot = WINDOW_HEIGHT - 20;
	r.left = 10;
	r.center = 0;
	ggprint8b(&r, 16, 0x0000000, "Waterfall Model");

	r.bot = WINDOW_HEIGHT - 115;
	r.left = 280;
	r.center = 0;
	ggprint8b(&r, 16, 0xcccccc/*ffffff*/, "Requirements");

	r.bot = WINDOW_HEIGHT - 185;
	r.left = 400;
	r.center = 0;
	ggprint8b(&r, 16, 0xcccccc/*ffffff*/, "Design");

	r.bot = WINDOW_HEIGHT - 265;
	r.left = 500;
	r.center = 0;
	ggprint8b(&r, 16, 0xcccccc/*ffffff*/, "Coding");

	r.bot = WINDOW_HEIGHT - 335;
	r.left = 600;
	r.center = 0;
	ggprint8b(&r, 16, 0xcccccc/*ffffff*/, "Testing");

	r.bot = WINDOW_HEIGHT - 405;
	r.left = 680;
	r.center = 0;
	ggprint8b(&r, 16, 0xcccccc/*ffffff*/, "Maintenance");

    }
    //draw all particles here
    for(int i=0; i<game->n; i++){
	glPushMatrix();
	// To have two colored particles
	if (i%2 == 0){
	glColor3ub(14,146,252);
	}
	else{
	glColor3ub(14,196,252);
	}
	Vec *c = &game->particle[i].s.center;
	w = 4;
	h = 4;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
    }
}
