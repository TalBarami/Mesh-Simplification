#include <iostream>
#include "display.h"
#include "inputManager.h"

Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");	
Scene scn(glm::vec3(0.0f, 0.0f, -15.0f), CAM_ANGLE, relation, NEAR, FAR);

int main(int argc, char** argv)
{
	initCallbacks(display);
	//scn.addShape("./res/objs/bigbox.obj","./res/textures/box0.bmp");
	//scn.addShape("./res/objs/testboxNoUV.obj");
	scn.addShape("./res/objs/monkey3.obj","./res/texture/box0.bmp");
	scn.addShader("./res/shaders/basicShader");
	scn.addShader("./res/shaders/pickingShader");

	while(!display.toClose())
	{
		display.Clear(0.7f, 0.7f, 0.7f, 1.0f);
		scn.draw(0,0,true);
		display.SwapBuffers();
		display.pullEvent();
	}

	return 0;
}
