#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		struct Particle {
			ofVec2f pos;
			ofVec2f vel;
			ofVec4f mass_angle_angularVel_reactiveAngle;
			ofFloatColor color;
		};

		vector<Particle> particles;
		float fps;
		int nParticles;
		float radius;
		float avgEnergyTotal;
		int avgEnergyCount;
		float initialEnergy;
		
		ofxFloatSlider velocityMultiplier;
		ofxToggle drawParticles;
		ofxPanel gui;

		ofBufferObject particleBuffer1;
		ofBufferObject particleBuffer2;
		ofShader compute;
		ofShader renderUpdate;
		vector<float> energyLevels;
		float energyMax;
	private:
		int updateCount;
		
};
