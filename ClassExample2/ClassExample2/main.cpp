#include <iostream>
#include "display.h"
#include "inputManager.h"

Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");	
Scene scn(glm::vec3(0.0f, 0.0f, -15.0f), CAM_ANGLE, relation, NEAR, FAR);

int main(int argc, char** argv)
{
	auto maxFaces = 400;

	initCallbacks(display);
	

	//scn.addShape("./res/objs/testboxNoUV.obj");
	//scn.addShape("./res/objs/testboxNoUV.obj", 10);

	scn.addShape("./res/objs/monkey3.obj");
	scn.addShape("./res/objs/monkey3.obj", maxFaces);

	scn.addShader("./res/shaders/basicShader");
	scn.addShader("./res/shaders/pickingShader");

	scn.shapeTransformation(0, Scene::xLocalTranslate, -4.0f);
	scn.shapeTransformation(1, Scene::xLocalTranslate, 4.0f);

	while(!display.toClose())
	{
		display.Clear(0.7f, 0.7f, 0.7f, 1.0f);
		scn.draw(0,0,true);
		display.SwapBuffers();
		display.pullEvent();
	}

	return 0;
}
