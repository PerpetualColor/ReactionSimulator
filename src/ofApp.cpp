#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	nParticles = 1000;
	radius = 0.01;
	energyMax = 1;
	updateCount = 0;
	avgEnergyTotal = 0;
	avgEnergyCount = 0;

	gui.setup();
	gui.add(velocityMultiplier.setup("Velocity Multiplier", 1.00, 1, 1.01));
	gui.add(drawParticles.setup("Draw Particles", true));

	compute.setupShaderFromFile(GL_COMPUTE_SHADER, "position_compute.glsl");
	compute.linkProgram();
	
	particles.resize(nParticles);
	
	nParticles = 0;
	for (auto &p : particles) {
		p.pos.x = ofRandom(1.0);
		p.pos.y = ofRandom(1.0);
		p.vel.set(ofRandom(0.5) - 0.25, ofRandom(0.5) - 0.25);
		p.mass_angle_angularVel_reactiveAngle.set(1.0, ofRandom(360), ofRandom(360), 30.0);
		nParticles++;
	}

	float e = 0;
	for (auto &p : particles) {
		e += 0.5 * p.mass_angle_angularVel_reactiveAngle.x * p.vel.lengthSquared();
	}
	initialEnergy = e;
	
	
	/*particles[0].pos.x = 0.3;
	particles[0].pos.y = 0.501;
	particles[0].vel.set(0.05, 0);
	particles[0].mass.set(1.0, 0, 0, 0);
	particles[1].pos.x = 0.7;
	particles[1].pos.y = 0.499;
	particles[1].vel.set(-0.05, 0);
	particles[1].mass.set(1.0, 0, 0, 0);
	particles[1].color = ofColor(255, 0, 0, 255);
	*/

	/*
	particles[0].pos.set(0, 0.5);
	particles[0].vel.set(1, 0);
	particles[0].mass_angle_angularVel_reactiveAngle.set(1.0, 90, 180, 40);

	particles[1].pos.set(1, 0.5);
	particles[1].vel.set(-1, 0);
	particles[1].mass_angle_angularVel_reactiveAngle.set(1.0, -90, 180.0, 40.0);
	*/
	
	
	particleBuffer1.allocate(particles, GL_DYNAMIC_DRAW);
	particleBuffer2.allocate(particles, GL_DYNAMIC_DRAW);
	particleBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	particleBuffer1.bindBase(GL_SHADER_STORAGE_BUFFER, 0);

}

//--------------------------------------------------------------
void ofApp::update(){
	
	fps = ofGetFrameRate();

	compute.begin();
	compute.setUniform1i("nParticles", nParticles);
	compute.setUniform1f("lastFrameTime", 1.0/60);
	compute.setUniform1f("radius", radius);
	compute.setUniform1f("velocityMult", velocityMultiplier);

	compute.dispatchCompute((particles.size() + 1024 - 1) / 1024, 1, 1);
	compute.end();

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(Particle), &(particles[0]));
	particleBuffer1.copyTo(particleBuffer2);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		cout << "OpenGL Error: " << err << endl;
	}
	if (updateCount % 4 == 0) {
		float e = 0;
		for (auto &p : particles) {
			e += 0.5 * p.mass_angle_angularVel_reactiveAngle.x * (powf(p.vel.x, 2) + powf(p.vel.y, 2));
			if (isnan((powf(p.vel.x, 2) + powf(p.vel.y, 2)))) {
				cout << "e is nan: " << p.mass_angle_angularVel_reactiveAngle.x << ", " << (powf(p.vel.x, 2) + powf(p.vel.y, 2)) << endl;
			}
		}
		if (e > energyMax) {
			float oldEMax = energyMax;
			energyMax = e;
			for (auto &v : energyLevels) {
				v = v * oldEMax / energyMax;
			}
		}

		avgEnergyTotal += e;
		avgEnergyCount++;

		if (energyLevels.size() < 60) {
			energyLevels.push_back(e);
		}
		else {
			rotate(energyLevels.begin(), energyLevels.begin() + 1, energyLevels.end());
			energyLevels[energyLevels.size() - 1] = e;
		}
		// limit the slope
		float m = (e - energyLevels[energyLevels.size()-5])/4;
		float velMultScaleFac = abs(initialEnergy - e);
		if (velMultScaleFac/initialEnergy > 0.005) { 
			if (e < initialEnergy && m < 0.0) {
				velocityMultiplier = velocityMultiplier + (0.0003 * velMultScaleFac);
			} else if (e > initialEnergy && m > 0.0) {
				velocityMultiplier = velocityMultiplier - (0.0003 * velMultScaleFac);
			}
		} else {
			velocityMultiplier = 1.0;
		}

	}
	updateCount++;

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(255);
	ofFill();
	ofPushMatrix();
	ofRotateDeg(180);
	float scaleFac = min(ofGetWidth()-300, ofGetHeight());
	if (drawParticles) {
		for (auto &p : particles) {
			ofPath arc;
			arc.setStrokeWidth(5);
			arc.setStrokeColor(ofColor(0));
			arc.setFillColor(ofColor(0));
			arc.arc(-300+p.pos.x * scaleFac - scaleFac, p.pos.y * scaleFac - scaleFac, radius * scaleFac + 2, radius * scaleFac + 2, p.mass_angle_angularVel_reactiveAngle.y + 90 - (0.5 * 30), p.mass_angle_angularVel_reactiveAngle.y + 90 + (0.5 * 30));
			arc.close();
			arc.draw();
			ofSetColor(p.color);
			ofDrawCircle(-300+p.pos.x * scaleFac - scaleFac, p.pos.y * scaleFac - scaleFac, radius * scaleFac);	
		}
	}
	ofPopMatrix();
	ofSetColor(0);
	ofDrawBitmapString("Framerate: " + ofToString(fps, 0), 10, 10);

	// create graph of energy
	ofPolyline energyLine;
	int energyGraphHeight = 100;

	for (int i = 0; i < energyLevels.size(); i++) {
		energyLine.addVertex(i * 5, 100 + energyGraphHeight - (energyLevels[i] * energyGraphHeight / energyMax));
	}
	energyLine.draw();
	ofDrawBitmapString("Correct Avg. Energy: " + ofToString(initialEnergy, 2), 10, 40);
	ofDrawBitmapString("Current Avg. Energy: " + ofToString(energyLevels[energyLevels.size() - 1], 2), 10, 80);
	ofDrawBitmapString("Overall Avg. Energy: " + ofToString(avgEnergyTotal/avgEnergyCount, 2), 10, 60);
	ofDrawBitmapString("Energy Slope: " + ofToString(energyLevels[energyLevels.size() - 1] - energyLevels[energyLevels.size() - 2], 2), 10, 180);

	gui.setPosition(10, 250);
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	compute.setupShaderFromFile(GL_COMPUTE_SHADER, "position_compute.glsl");
	compute.linkProgram();
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
