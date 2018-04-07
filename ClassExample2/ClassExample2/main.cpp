#include <iostream>
#include "display.h"
#include "inputManager.h"
#include <algorithm>

Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");	
Scene scn(glm::vec3(0.0f, 0.0f, -15.0f), CAM_ANGLE, relation, NEAR, FAR);

int main(int argc, char** argv)
{
	/*auto maxFaces = 1000;
	if(argc > 1)
	{
		
	}
	initCallbacks(display);
	//scn.addShape("./res/objs/bigbox.obj","./res/textures/box0.bmp");
	//scn.addShape("./res/objs/testboxNoUV.obj");
	scn.addShape("./res/objs/monkey3.obj","./res/textures/box0.bmp");
	//scn.addShape("./res/objs/monkey3.obj", "./res/textures/box0.bmp", maxFaces);
	scn.addShader("./res/shaders/basicShader");
	scn.addShader("./res/shaders/pickingShader");

	while(!display.toClose())
	{
		display.Clear(0.7f, 0.7f, 0.7f, 1.0f);
		scn.draw(0,0,true);
		display.SwapBuffers();
		display.pullEvent();
	}

	return 0;*/

	// Testing:
	using namespace glm;
	
	Edge e1;
	e1.error = 1;
	Edge e2;
	e2.error = 2;
	Edge e3;
	e3.error = 3;
	Edge e4;
	e4.error = 4;
	Edge e5;
	e5.error = 5;

	std::vector<Edge> v;
	v.push_back(e5);
	v.push_back(e2);
	v.push_back(e3);
	v.push_back(e4);
	v.push_back(e1);

	/*for (int i = 0; i < v.size(); i++)
	{
		std::cout << v[i].error << std::endl;
	}
	std::cout << std::endl;

	std::make_heap(v.begin(), v.end());

	for (int i = 0; i < v.size(); i++)
	{
		std::cout << v[i].error << std::endl;
	}
	std::cout << std::endl;

	for (int i = 0; i < 5; i++)
	{
		std::cout << v.front().error << std::endl;
 

		std::pop_heap(v.begin(),v.end()-i);
	}*/

	for (int i = 0; i < 5; ++i)
	{
		std::cout << i << std::endl;
	}


	system("pause");
}
