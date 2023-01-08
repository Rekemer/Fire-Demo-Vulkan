#include "Config.h"
#include "Renderer.h"

class App {

private:
	Renderer* graphicsEngine;
	GLFWwindow* window;

	double lastTime, currentTime;
	int numFrames;
	float frameTime;

	void build_glfw_window(int width, int height, bool debugMode);

	void calculateFrameRate();

public:
	App(int width, int height, bool debug);
	~App();
	void run();
}; 
