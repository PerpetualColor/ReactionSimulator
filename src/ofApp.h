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
		void restartSim();

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
			GLint productC;
			GLint productD;
			GLfloat deltaE;
		};

		vector<Particle> particles;
		vector<ParticleType> particleTypes;
		vector<Reaction> reactions;
		
		float fps;
		int nParticles;
		float radius;
		float avgEnergyTotal;
		int avgEnergyCount;
		float initialEnergy;
		
		ofxFloatSlider velocityMultiplier;
		ofxToggle drawParticles;
		ofxButton restart;
		ofxPanel gui;

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
