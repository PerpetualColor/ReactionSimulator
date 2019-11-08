#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	nParticles = 1000;
	radius = 0.01;

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
	
	
	/*particles[0].pos.x = 0.3;
	particles[0].pos.y = 0.501;
	particles[0].vel.set(0.05, 0);
	particles[0].mass.set(1.0, 0, 0, 0);
	particles[1].pos.x = 0.7;
	particles[1].pos.y = 0.499;
	particles[1].vel.set(-0.05, 0);
	particles[1].mass.set(1.0, 0, 0, 0);
	particles[1].color = ofColor(255, 0, 0, 255);*/
	
	
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

	compute.dispatchCompute((particles.size() + 1024 - 1) / 1024, 1, 1);
	compute.end();

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(Particle), &(particles[0]));
	particleBuffer1.copyTo(particleBuffer2);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		cout << "OpenGL Error: " << err << endl;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(255);
	ofFill();
	ofPushMatrix();
	ofRotateDeg(180);
	float scaleFac = min(ofGetWidth(), ofGetHeight());
	for (auto &p : particles) {
		ofPath arc;
		arc.setStrokeWidth(5);
		arc.setStrokeColor(ofColor(0));
		arc.setFillColor(ofColor(0));
		arc.arc(p.pos.x * scaleFac - scaleFac, p.pos.y * scaleFac - scaleFac, radius * scaleFac + 2, radius * scaleFac + 2, p.mass_angle_angularVel_reactiveAngle.y + 90 - (0.5 * 30), p.mass_angle_angularVel_reactiveAngle.y + 90 + (0.5 * 30));
		arc.close();
		arc.draw();
		ofSetColor(p.color);
		ofDrawCircle(p.pos.x * scaleFac - scaleFac, p.pos.y * scaleFac - scaleFac, radius * scaleFac);
	}
	ofPopMatrix();
	ofSetColor(0);
	ofDrawBitmapString("Framerate: " + ofToString(fps, 0), 10, 10);
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
