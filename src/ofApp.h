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

		void radiusChanged(float &newRadius);
		void loadFile();
		void framerateLimiterToggle(bool &newState);

		struct Particle {
			ofVec2f pos;
			ofVec2f vel;
			ofVec4f mass_angle_angularVel_reactiveAngle;
			ofFloatColor color;
			ofVec4f collision_state;
		};

		struct ParticleType {
			float reactiveAngle;
			ofFloatColor color;
			float mass;
		};

		struct Reaction {
			GLint reactantA;
			GLint reactantB;
			vector<int> products;
			GLfloat activationEnergy;
			GLfloat deltaE;
		};

		vector<Particle> particles;
		vector<ParticleType> particleTypes;
		vector<Reaction> reactions;
		vector<vector<float>> concentrationData;
		
		bool initialSetup;
		
		float fps;
		int nParticles;
		float radius;
		float avgEnergyTotal;
		int avgEnergyCount;
		float initialEnergy;

		ofxPanel gui;
		ofxFloatSlider guiAdjustVelocity;
		ofxToggle guiDrawParticles;
		ofxButton guiRestart;
		ofxToggle guiPause;
		ofxToggle guiLimitFramerate;

		ofxPanel particleGui;
		ofParameter<float> guiParticleRadius;
		vector<ofParameter<int>> initParticleCountsData;
		ofParameterGroup initParticleCounts;

		ofBufferObject particleBuffer1;
		ofBufferObject particleBuffer2;
		ofBufferObject colorBuffer;

		ofShader compute;
		ofShader renderUpdate;
		vector<float> energyLevels;
		float energyMax;
	private:
		int updateCount;
};
