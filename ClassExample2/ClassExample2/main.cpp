#include <iostream>
#include "display.h"
#include "inputManager.h"
#include <algorithm>

Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");	
Scene scn(glm::vec3(0.0f, 0.0f, -15.0f), CAM_ANGLE, relation, NEAR, FAR);

int main(int argc, char** argv)
{
	auto maxFaces = 100;
	

	initCallbacks(display);
	//scn.addShape("./res/objs/bigbox.obj","./res/textures/box0.bmp");
	//scn.addShape("./res/objs/testboxNoUV.obj");
	//scn.addShape("./res/objs/bigbox.obj","./res/textures/box0.bmp", maxFaces);
	scn.addShape("./res/objs/monkey3.obj", "./res/textures/box0.bmp", maxFaces);
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

	// Testing:

	/*using namespace std;

	multimap<int, int> testmap;

	testmap.insert(make_pair(1, 1));
	testmap.insert(make_pair(1, 5));
	testmap.insert(make_pair(2, 2));
	testmap.insert(make_pair(2, 3));
	testmap.insert(make_pair(1, 3));
	testmap.insert(make_pair(1, 4));
	testmap.insert(make_pair(2, 1));
	testmap.insert(make_pair(2, 4));


	auto it2 = testmap.equal_range(1);
	for (auto it = it2.first; it != it2.second; it++)
	{
		cout << it->first << " " << it->second << endl;
	}

	system("pause");*/
}
