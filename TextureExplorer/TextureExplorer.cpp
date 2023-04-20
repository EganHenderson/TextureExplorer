/* This file implements a 2D texture explorer. This creates a window with a procedural texture that the user can then select between variations of.
* A window is created, algorithms are run for R, B, and G to determine the colour value. These are then drawn on screen pixel by pixel.
* The algorithms take in x and y values, these are determined by the pixel's location in the set coordinate system based on the pixel's location in the window.
* The user can select between 10 textures, and select a specific texture for R, G, and B. As well, the user can turn off any of the three channels if desired.
* This application implements the abililty to change the coordinate system used to determine colour values.
* The user can also select a random texture, which takes a random algorithm for R, G, and B.
* The user is able to save the texture to a PNG file.
* Lastly, the user is able to easily quit the application.
*/
#define FILENAME "texture.png"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <freeglut.h>
#include <FreeImage.h>
#include <time.h>
#include <iostream>
using namespace std;

//the pixel structure
typedef struct
{
	GLubyte r, g, b;
} pixel;

//the global structure
typedef struct
{
	int w, h; //w for width and h for height of the window
	int rForm, gForm, bForm; //an integer determining which formula to use to calculate red, green, and blue respectively
	float xMin, xMax, yMin, yMax; //the minimums and maximums of x and y for the coordinate system
} glob;
glob global;

//enums for the menu buttons/options
enum {
	MENU_SAVE, MENU_QUIT, MENU_RANDOM, MENU_CHANGE, MENU_T0, MENU_T1, MENU_T2, MENU_T3, MENU_T4, MENU_T5, MENU_T6, MENU_T7, MENU_T8, MENU_T9,
	MENU_R0, MENU_R1, MENU_R2, MENU_R3, MENU_R4, MENU_R5, MENU_R6, MENU_R7, MENU_R8, MENU_R9, MENU_ROFF,
	MENU_G0, MENU_G1, MENU_G2, MENU_G3, MENU_G4, MENU_G5, MENU_G6, MENU_G7, MENU_G8, MENU_G9, MENU_GOFF,
	MENU_B0, MENU_B1, MENU_B2, MENU_B3, MENU_B4, MENU_B5, MENU_B6, MENU_B7, MENU_B8, MENU_B9, MENU_BOFF
};

//returns the value of the formula based on the switch case. global.rForm is set using menuFunc
//these are all unique formulas to return a float for the R value of the RGB colour
float redFormula(float x, float y)
{
	switch (global.rForm)
	{
	case 0:
		return y == 0 ? 0 : x * 7 / y;
	case 1:
		return cos(y) == 0 ? 0 : sin(x) / cos(y);
	case 2:
		return cos(y * x) == 0 ? 0 : y - x / cos(y * x);
	case 3:
		if (x < 0)
			x *= -1;
		if (y < 0)
			y *= -1;
		return sqrt(x) * cos(sqrt(y));
	case 4:
		return pow(y, 2) * cos(x);
	case 5:
		return tan(pow(x, y));
	case 6:
		if (y < 0)
			y *= -1;
		return tan(sqrt(y)) == 0 ? 0 : x * y / tan(sqrt(y));
	case 7:
		return cos(x * cos(y)) * tan(x);
	case 8:
		return sin(sin(x * y) * sin(x) * sin(y)) * 2;
	case 9:
		return y == 0 ? 0 : sin(x) * cos(x) * tan(x) / y;
	case 10:
		return 0;
	}

	return 0;
}

//returns the value of the formula based on the switch case. global.gForm is set using menuFunc
//these are all unique formulas to return a float for the G value of the RGB colour
float greenFormula(float x, float y)
{
	switch (global.gForm)
	{
	case 0:
		return tan(x) * cos(y);
	case 1:
		return sin(tan(x * y));
	case 2:
		return 37 * x + y;
	case 3:
		return y == 0 ? 0.0 : x / tan(21) / y;
	case 4:
		if (x < 0)
			x *= -1;
		if (y < 0)
			y *= -1;
		return tan(x * y) == 0 ? 0 : sqrt(x * y) / tan(x * y);
	case 5:
		return tan(x) * cos(y) * sin(x * y);
	case 6:
		if (tan(x * y) < 0)
			x *= -1;
		return sin(cos(sqrt(tan(x * y))));
	case 7:
		return cos(x) == 0 ? 0 : y / cos(x);
	case 8:
		return sin(tan(pow(x, y)));
	case 9:
		if (x < 0)
			x *= -1;
		if (y < 0)
			y *= -1;
		return sin(x) == 0 ? 0 : sqrt(x) * sqrt(y) / sin(x);
	case 10:
		return 0;
	}

	return 0;
}

//returns the value of the formula based on the switch case. global.bForm is set using menuFunc
//these are all unique formulas to return a float for the B value of the RGB colour
float blueFormula(float x, float y)
{
	switch (global.bForm)
	{
	case 0:
		return x == 0 ? 1 : sin(y) / x;
	case 1:
		return x == 0 ? 0 : y - x / x;
	case 2:
		return tan(x * sin(y) * y);
	case 3:
		return y == 0 ? 0 : cos(x) / y;
	case 4:
		if (y == 0)
			return 0;
		return sin(x / y) == 0 ? 0 : x * tan(x) * cos(y) / sin(x / y);
	case 5:
		return cos(sin(y)) * cos(sin(x));
	case 6:
		if (sin(x) < 0)
			x *= -1;
		return sqrt(sin(x)) * y;
	case 7:
		return x == 0 ? 0 : pow(sin(x), y) / x;
	case 8:
		return 0.215f * sin(x + y);
	case 9:
		return x + tan(1.1265f * y);
	case 10:
		return 0;
	}

	return 0;
}

//draw each pixel in the window based on calculated values for RGB from different algorithms
void draw_texture()
{
	//set the x and y values to the minimum of the coordinate system
	float x = global.xMin;
	float y = global.yMin;

	//begin drawing points on screen, as we are drawing pixel by pixel
	glBegin(GL_POINTS);

	//loop through all pixels in the window
	for (int w = 0; w < global.w; w++)
	{
		y = global.yMin;

		for (int h = 0; h < global.h; h++)
		{
			//set the colour of the pixel based on the formulas, then create that pixel in the window
			glColor3f(redFormula(x, y), greenFormula(x, y), blueFormula(x, y));
			glVertex2f(x, y);

			y += ((global.yMax - global.yMin) / (float)global.h);
		}

		x += ((global.xMax - global.xMin) / (float)global.w);
	}

	glEnd();
	glFlush();
}

//saves the current window to a PNG file called FILENAME (default - texture.png)
void save()
{
	//sets up the size of our pixel*, then reads the pixel data from the window to pixels
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	size_t size = 3 * width * height;
	pixel* pixels = (pixel*)malloc(size * sizeof(pixel));
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	FIBITMAP* image;
	RGBQUAD aPixel;

	//use FreeImage to create a PNG file of the data in pixels
	if (pixels)
	{
		image = FreeImage_Allocate(width, height, 24, 0, 0, 0);
		if (!image)
		{
			perror("FreeImage_Allocate");
			return;
		}
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				aPixel.rgbRed = pixels[y * width + x].r;
				aPixel.rgbGreen = pixels[y * width + x].g;
				aPixel.rgbBlue = pixels[y * width + x].b;

				FreeImage_SetPixelColor(image, x, y, &aPixel);
			}
		}
		if (!FreeImage_Save(FIF_PNG, image, FILENAME, 0)) {
			perror("FreeImage_Save");
		}

		FreeImage_Unload(image);
	}
}

//set random values between 0 and 9 for the formulas of R, G, and B. Then, tell glut to redraw the screen to show the new texture
void random()
{
	global.rForm = rand() % 10;
	global.gForm = rand() % 10;
	global.bForm = rand() % 10;

	glutPostRedisplay();
}

//change the values of the global.xMin,xMax,yMin,yMax to create a new coordinate system
void changeCoordinates()
{
	float newXMin, newXMax, newYMin, newYMax;

	//have the user enter a new value for the x minimum, ensuring it is between -1000000000 and 100000000
	cout << "Enter new X minimum: ";
	cin >> newXMin;

	while (newXMin < -1000000000 || newXMin > 1000000000)
	{
		cout << "Please enter a float between -1,000,000,000 and 1,000,000,000, exclusive: ";
		cin >> newXMin;
	}

	//have the user enter a new value for the x maximum, ensuring it is between -1000000000 and 100000000. also ensuring it is greater than the x minimum
	cout << "Enter new X maximum: ";
	cin >> newXMax;

	while (newXMax < -1000000000 || newXMax > 1000000000)
	{
		cout << "Please enter a float between -1,000,000,000 and 1,000,000,000, exclusive: ";
		cin >> newXMax;
	}

	while (newXMin > newXMax)
	{
		cout << "X maximum must be greater than X minimum! ";
		cout << "Enter new X maximum: ";
		cin >> newXMax;
	}

	//have the user enter a new value for the y minimum, ensuring it is between -1000000000 and 100000000
	cout << "Enter new Y minimum: ";
	cin >> newYMin;

	while (newYMin < -1000000000 || newYMin > 1000000000)
	{
		cout << "Please enter a float between -1,000,000,000 and 1,000,000,000, exclusive: ";
		cin >> newYMin;
	}

	//have the user enter a new value for the y maximum, ensuring it is between -1000000000 and 100000000. also ensuring it is greater than the y minimum
	cout << "Enter new Y maximum: ";
	cin >> newYMax;

	while (newYMax < -1000000000 || newYMax > 1000000000)
	{
		cout << "Please enter a float between -1,000,000,000 and 1,000,000,000, exclusive: ";
		cin >> newYMax;
	}

	while (newYMin > newYMax)
	{
		cout << "Y maximum must be greater than Y minimum!";
		cout << "Enter new Y maximum: ";
		cin >> newYMax;
	}

	//set the new values of the coordinate system, and set the display to show the changes
	global.xMin = newXMin;
	global.xMax = newXMax;
	global.yMin = newYMin;
	global.yMax = newYMax;
	glLoadIdentity();
	gluOrtho2D(global.xMin, global.xMax, global.yMin, global.yMax);
	glutPostRedisplay();
}

/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1B:
	case'q':
	case 'Q':
		exit(0);
		break;
	case 's':
	case 'S':
		save();
		break;
	case 'r':
	case 'R':
		random();
		break;
	case 'c':
	case 'C':
		changeCoordinates();
		break;
	}
}//keyboard

 //Glut menu callback function
void menuFunc(int value)
{
	switch (value)
	{
	case MENU_QUIT:
		exit(0);
		break;
	case MENU_SAVE:
		save();
		break;
	case MENU_RANDOM:
		random();
		break;
	case MENU_CHANGE:
		changeCoordinates();
		break;
	case MENU_T0:
		global.rForm = 0;
		global.gForm = 0;
		global.bForm = 0;
		break;
	case MENU_T1:
		global.rForm = 1;
		global.gForm = 1;
		global.bForm = 1;
		break;
	case MENU_T2:
		global.rForm = 2;
		global.gForm = 2;
		global.bForm = 2;
		break;
	case MENU_T3:
		global.rForm = 3;
		global.gForm = 3;
		global.bForm = 3;
		break;
	case MENU_T4:
		global.rForm = 4;
		global.gForm = 4;
		global.bForm = 4;
		break;
	case MENU_T5:
		global.rForm = 5;
		global.gForm = 5;
		global.bForm = 5;
		break;
	case MENU_T6:
		global.rForm = 6;
		global.gForm = 6;
		global.bForm = 6;
		break;
	case MENU_T7:
		global.rForm = 7;
		global.gForm = 7;
		global.bForm = 7;
		break;
	case MENU_T8:
		global.rForm = 8;
		global.gForm = 8;
		global.bForm = 8;
		break;
	case MENU_T9:
		global.rForm = 9;
		global.gForm = 9;
		global.bForm = 9;
		break;
	case MENU_R0:
		global.rForm = 0;
		break;
	case MENU_R1:
		global.rForm = 1;
		break;
	case MENU_R2:
		global.rForm = 2;
		break;
	case MENU_R3:
		global.rForm = 3;
		break;
	case MENU_R4:
		global.rForm = 4;
		break;
	case MENU_R5:
		global.rForm = 5;
		break;
	case MENU_R6:
		global.rForm = 6;
		break;
	case MENU_R7:
		global.rForm = 7;
		break;
	case MENU_R8:
		global.rForm = 8;
		break;
	case MENU_R9:
		global.rForm = 9;
		break;
	case MENU_ROFF:
		global.rForm = 10;
		break;
	case MENU_G0:
		global.gForm = 0;
		break;
	case MENU_G1:
		global.gForm = 1;
		break;
	case MENU_G2:
		global.gForm = 2;
		break;
	case MENU_G3:
		global.gForm = 3;
		break;
	case MENU_G4:
		global.gForm = 4;
		break;
	case MENU_G5:
		global.gForm = 5;
		break;
	case MENU_G6:
		global.gForm = 6;
		break;
	case MENU_G7:
		global.gForm = 7;
		break;
	case MENU_G8:
		global.gForm = 8;
		break;
	case MENU_G9:
		global.gForm = 9;
		break;
	case MENU_GOFF:
		global.gForm = 10;
		break;
	case MENU_B0:
		global.bForm = 0;
		break;
	case MENU_B1:
		global.bForm = 1;
		break;
	case MENU_B2:
		global.bForm = 2;
		break;
	case MENU_B3:
		global.bForm = 3;
		break;
	case MENU_B4:
		global.bForm = 4;
		break;
	case MENU_B5:
		global.bForm = 5;
		break;
	case MENU_B6:
		global.bForm = 6;
		break;
	case MENU_B7:
		global.bForm = 7;
		break;
	case MENU_B8:
		global.bForm = 8;
		break;
	case MENU_B9:
		global.bForm = 9;
		break;
	case MENU_BOFF:
		global.bForm = 10;
		break;
	}

	glutPostRedisplay();
}//menuFunc

//show the keys for actions in the terminal
void show_keys()
{
	printf("Q:quit\nS:save\nR:random\nC:change coordinates\n");
}

//Glut menu set up
void init_menu()
{
	//menu for selecting overall texture
	int texture_menu = glutCreateMenu(&menuFunc);
	glutAddMenuEntry("Texture 0", MENU_T0);
	glutAddMenuEntry("Texture 1", MENU_T1);
	glutAddMenuEntry("Texture 2", MENU_T2);
	glutAddMenuEntry("Texture 3", MENU_T3);
	glutAddMenuEntry("Texture 4", MENU_T4);
	glutAddMenuEntry("Texture 5", MENU_T5);
	glutAddMenuEntry("Texture 6", MENU_T6);
	glutAddMenuEntry("Texture 7", MENU_T7);
	glutAddMenuEntry("Texture 8", MENU_T8);
	glutAddMenuEntry("Texture 9", MENU_T9);

	//menu for selecting which algorithm to use for the red channel
	int red_menu = glutCreateMenu(&menuFunc);
	glutAddMenuEntry("Red 0", MENU_R0);
	glutAddMenuEntry("Red 1", MENU_R1);
	glutAddMenuEntry("Red 2", MENU_R2);
	glutAddMenuEntry("Red 3", MENU_R3);
	glutAddMenuEntry("Red 4", MENU_R4);
	glutAddMenuEntry("Red 5", MENU_R5);
	glutAddMenuEntry("Red 6", MENU_R6);
	glutAddMenuEntry("Red 7", MENU_R7);
	glutAddMenuEntry("Red 8", MENU_R8);
	glutAddMenuEntry("Red 9", MENU_R9);
	glutAddMenuEntry("Red Off", MENU_ROFF);

	//menu for selecting which algorithm to use for the green channel
	int green_menu = glutCreateMenu(&menuFunc);
	glutAddMenuEntry("Green 0", MENU_G0);
	glutAddMenuEntry("Green 1", MENU_G1);
	glutAddMenuEntry("Green 2", MENU_G2);
	glutAddMenuEntry("Green 3", MENU_G3);
	glutAddMenuEntry("Green 4", MENU_G4);
	glutAddMenuEntry("Green 5", MENU_G5);
	glutAddMenuEntry("Green 6", MENU_G6);
	glutAddMenuEntry("Green 7", MENU_G7);
	glutAddMenuEntry("Green 8", MENU_G8);
	glutAddMenuEntry("Green 9", MENU_G9);
	glutAddMenuEntry("Green Off", MENU_GOFF);

	//menu for selecting which algorithm to use for the blue channel
	int blue_menu = glutCreateMenu(&menuFunc);
	glutAddMenuEntry("Blue 0", MENU_B0);
	glutAddMenuEntry("Blue 1", MENU_B1);
	glutAddMenuEntry("Blue 2", MENU_B2);
	glutAddMenuEntry("Blue 3", MENU_B3);
	glutAddMenuEntry("Blue 4", MENU_B4);
	glutAddMenuEntry("Blue 5", MENU_B5);
	glutAddMenuEntry("Blue 6", MENU_B6);
	glutAddMenuEntry("Blue 7", MENU_B7);
	glutAddMenuEntry("Blue 8", MENU_B8);
	glutAddMenuEntry("Blue 9", MENU_B9);
	glutAddMenuEntry("Blue Off", MENU_BOFF);

	//menu for overall menu options
	int main_menu = glutCreateMenu(&menuFunc);
	glutAddSubMenu("Texture", texture_menu);
	glutAddSubMenu("R", red_menu);
	glutAddSubMenu("G", green_menu);
	glutAddSubMenu("B", blue_menu);
	glutAddMenuEntry("Change Coordinates", MENU_CHANGE);
	glutAddMenuEntry("Random Texture", MENU_RANDOM);
	glutAddMenuEntry("Save", MENU_SAVE);
	glutAddMenuEntry("Quit", MENU_QUIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//what runs the whole show
int main(int argc, char** argv)
{
	srand(static_cast <unsigned> (time(0))); //set a new random seed

	global.w = 500; //width and height set to 500 x 500
	global.h = 500;

	//set default coordinates, these make the formulas I came up with look alright in my opinion. fun stuff can happen at other coordinates though!
	global.xMin = -100.0;
	global.xMax = 100.0;
	global.yMin = -100.0;
	global.yMax = 100.0;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

	glutInitWindowSize(global.w, global.h);
	glutCreateWindow("SUPER TEXTURE EXPLORER");
	glShadeModel(GL_SMOOTH);
	glutKeyboardFunc(keyboard);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(global.xMin, global.xMax, global.yMin, global.yMax);

	init_menu();
	show_keys();

	glutDisplayFunc(draw_texture);

	glutMainLoop();
}