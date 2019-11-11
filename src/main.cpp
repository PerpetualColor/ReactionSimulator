#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	// uses openGL 4.3 for compute shader
	ofGLWindowSettings settings;
	settings.setGLVersion(4, 3);
	settings.windowMode = OF_WINDOW;
	settings.setSize(1100, 800);
	ofCreateWindow(settings);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	auto app = new ofApp();
	app->initialSetup = true;
	ofRunApp(app);

}
