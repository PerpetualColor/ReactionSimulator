#include "ofApp.h"
#include <set>

//--------------------------------------------------------------
void ofApp::setup(){
	nParticles = 1000;
	radius = 0.01;
	energyMax = 1;
	updateCount = 0;
	avgEnergyTotal = 0;
	avgEnergyCount = 0;

	particleTypes.resize(5);
	// particle A
	particleTypes[1] = ParticleType{ 60, ofColor(255, 0, 0), 1.0 };
	// particle B
	particleTypes[2] = ParticleType{ 120, ofColor(0, 255, 0), 2.0 };
	// particle C
	particleTypes[3] = ParticleType{ 180, ofColor(0, 0, 255), 1.5 };
	// particle D
	particleTypes[4] = ParticleType{ 120, ofColor(255, 255, 0), 1.5 };

	vector<ofFloatColor> particleColors;
	particleColors.resize(particleTypes.size());
	for (int i = 0; i < particleTypes.size(); i++) {
		particleColors[i] = particleTypes[i].color;
	}

	// A + A -> B, deltaE = 0
	reactions.push_back(Reaction{
		1, 1, 2, 0, 0
	});
	// A + B -> C + D
	reactions.push_back(Reaction{
		1, 2, 3, 4, 0
	});

	restart.addListener(this, &ofApp::restartSim);
	gui.setup();
	gui.add(velocityMultiplier.setup("Velocity Multiplier", 1.00, 0.99, 1.01));
	gui.add(drawParticles.setup("Draw Particles", true));
	gui.add(restart.setup("Restart"));

	compute.setupShaderFromFile(GL_COMPUTE_SHADER, "position_compute.glsl");
	compute.linkProgram();
	
	particles.resize(nParticles);

	for (auto &p : particles) {
		p.pos.x = ofRandom(1.0);
		p.pos.y = ofRandom(1.0);
		p.vel.set(ofRandom(0.5) - 0.25, ofRandom(0.5) - 0.25);
		p.mass_angle_angularVel_reactiveAngle.set(1.0, ofRandom(360), ofRandom(720) - 360, particleTypes[1].reactiveAngle);
		p.collision_state.set(0, 1, 0, 0);
	}
	
	

	/*
	particles[0].pos.set(0.5, 0.5);
	particles[0].vel.set(0, 0);
	particles[0].mass_angle_angularVel_reactiveAngle.set(1, 45, 360, 30);
	*/
	
	/*
	particles[0].pos.x = 0.3;
	particles[0].pos.y = 0.5;
	particles[0].vel.set(0.05, 0);
	particles[0].mass_angle_angularVel_reactiveAngle.set(1.0, -90, 0, 30);
	particles[1].color = ofColor(0, 0, 0, 255);
	particles[1].pos.x = 0.7;
	particles[1].pos.y = 0.5;
	particles[1].vel.set(-0.05, 0);
	particles[1].mass_angle_angularVel_reactiveAngle.set(1.0, 90, 0, 30);
	particles[1].color = ofColor(0, 0, 0, 255);
	*/

	/*
	particles[0].pos.set(0, 0.5);
	particles[0].vel.set(1, 0);
	particles[0].mass_angle_angularVel_reactiveAngle.set(1.0, 90, 180, 40);

	particles[1].pos.set(1, 0.5);
	particles[1].vel.set(-1, 0);
	particles[1].mass_angle_angularVel_reactiveAngle.set(1.0, -90, 180.0, 40.0);
	*/
	
	colorBuffer.allocate(particleColors, GL_STATIC_READ);
	colorBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	particleBuffer1.allocate(particles, GL_DYNAMIC_DRAW);
	particleBuffer2.allocate(particles, GL_DYNAMIC_DRAW);
	particleBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	particleBuffer1.bindBase(GL_SHADER_STORAGE_BUFFER, 0);

	float e = 0;
	for (auto &p : particles) {
		e += 0.5 * p.mass_angle_angularVel_reactiveAngle.x * p.vel.lengthSquared();
	}
	initialEnergy = e;

}

void ofApp::restartSim() {
	nParticles = 1000;
	radius = 0.01;
	energyMax = 1;
	updateCount = 0;
	avgEnergyTotal = 0;
	avgEnergyCount = 0;

	compute.setupShaderFromFile(GL_COMPUTE_SHADER, "position_compute.glsl");
	compute.linkProgram();

	particles.resize(nParticles);

	for (auto &p : particles) {
		p.pos.x = ofRandom(1.0);
		p.pos.y = ofRandom(1.0);
		p.vel.set(ofRandom(0.5) - 0.25, ofRandom(0.5) - 0.25);
		p.mass_angle_angularVel_reactiveAngle.set(1.0, ofRandom(360), ofRandom(720) - 360, 30.0);
		p.collision_state.set(0, 1, 0, 0);
	}


	/*
	particles[0].pos.x = 0.3;
	particles[0].pos.y = 0.5;
	particles[0].vel.set(0.05, 0);
	particles[0].mass_angle_angularVel_reactiveAngle.set(1.0, -90, 0, 30);
	particles[1].color = ofColor(0, 0, 0, 255);
	particles[1].pos.x = 0.7;
	particles[1].pos.y = 0.5;
	particles[1].vel.set(-0.05, 0);
	particles[1].mass_angle_angularVel_reactiveAngle.set(1.0, 90, 0, 30);
	particles[1].color = ofColor(0, 0, 0, 255);
	*/

	/*
	particles[0].pos.set(0, 0.5);
	particles[0].vel.set(1, 0);
	particles[0].mass_angle_angularVel_reactiveAngle.set(1.0, 90, 180, 40);

	particles[1].pos.set(1, 0.5);
	particles[1].vel.set(-1, 0);
	particles[1].mass_angle_angularVel_reactiveAngle.set(1.0, -90, 180.0, 40.0);
	*/


	particleBuffer1.setData(particles, GL_DYNAMIC_DRAW);
	particleBuffer2.setData(particles, GL_DYNAMIC_DRAW);
	particleBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	particleBuffer1.bindBase(GL_SHADER_STORAGE_BUFFER, 0);

	float e = 0;
	for (auto &p : particles) {
		e += 0.5 * p.mass_angle_angularVel_reactiveAngle.x * p.vel.lengthSquared();
	}
	initialEnergy = e;
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
	// handle combination collisions
	vector<int> removedIndices;
	set<int> unavailableIndices;

	// collision index is a particle which is not unavailable
	for (int i = 0; i < particles.size(); i++) {
		int idx = int(particles[i].collision_state.x);
		if (idx != -1 && unavailableIndices.find(idx) == unavailableIndices.end()) {
			// search the reaction table for a valid reaction
			bool reactionExists = false;
			Reaction curReaction;

			int state1 = particles[i].collision_state.y;
			int state2 = particles[idx].collision_state.y;
			for (auto &r : reactions) {
				if ((r.reactantA == state1 && r.reactantB == state2) || (r.reactantA == state2 && r.reactantB == state1)) {
					reactionExists = true;
					curReaction = r;
				}
			}

			if (reactionExists) {
				unavailableIndices.insert(idx);
				unavailableIndices.insert(i);

				// only one product
				if (curReaction.productC == 0 || curReaction.productD == 0) {
					removedIndices.push_back(idx);
					// momentum is conserved, will be stored in the current particle, mass is the sum of the old ones, reaction angle is 0
					float totalMass = particles[i].mass_angle_angularVel_reactiveAngle.x + particles[idx].mass_angle_angularVel_reactiveAngle.x;
					ofVec2f newVel = ((particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel) + (particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel)) / totalMass;

					float deltaE = 0.5 * ((totalMass * newVel.lengthSquared()) -
						((particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel.lengthSquared()) +
						(particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel.lengthSquared())));
					initialEnergy += deltaE;

					particles[i].vel = newVel;
					particles[i].mass_angle_angularVel_reactiveAngle.x += particles[idx].mass_angle_angularVel_reactiveAngle.x;
					int newState = max(curReaction.productC, curReaction.productD);
					particles[i].mass_angle_angularVel_reactiveAngle.w = particleTypes[newState].reactiveAngle;
					particles[i].collision_state.y = newState;
				}
				// two products; one particle will be turned into each
				else {
					// change the properties of each; this is where it is important to have accurate mass values in the vector
					particles[i].mass_angle_angularVel_reactiveAngle.x = particleTypes[curReaction.productC].mass;
					particles[i].mass_angle_angularVel_reactiveAngle.w = particleTypes[curReaction.productC].reactiveAngle;
					particles[i].collision_state.y = curReaction.productC;

					particles[idx].mass_angle_angularVel_reactiveAngle.x = particleTypes[curReaction.productD].mass;
					particles[idx].mass_angle_angularVel_reactiveAngle.w = particleTypes[curReaction.productD].reactiveAngle;
					particles[idx].collision_state.y = curReaction.productD;

					float kInitial = (0.5 * particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel.lengthSquared())
						 + (0.5 * particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel.lengthSquared());
					float velMultFac = sqrtf((kInitial - curReaction.deltaE) / kInitial);
					particles[i].vel *= velMultFac;
					particles[idx].vel *= velMultFac;
					
				}
			}
		}
	}

	sort(removedIndices.begin(), removedIndices.end(), greater<int>());

	for (int i = 0; i < removedIndices.size(); i++) {
		particles[removedIndices[i]] = particles.back();
		particles.pop_back();
	}

	if (particles.size() < nParticles) {
		cout << "particles: " << particles.size() << endl;
	}

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


		// limit the slope and manage the velocityMultiplier
		float m;
		if (energyLevels.size() < 6) {
			m = 0;
		}
		else {
			m = (e - energyLevels[energyLevels.size() - 5]) / 4;
		}
		float velMultScaleFac = abs(initialEnergy - e);
		if (velMultScaleFac/initialEnergy > 0.005) { 
			velMultScaleFac += abs((avgEnergyTotal / avgEnergyCount) - initialEnergy);
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
	
	nParticles = particles.size();
	particleBuffer1.setData(particles, GL_DYNAMIC_DRAW);

	particleBuffer1.copyTo(particleBuffer2);

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(255);
	ofFill();
	float scaleFac = min(ofGetWidth()-300, ofGetHeight()-300);
	if (drawParticles) {
		for (auto &p : particles) {
			if (p.mass_angle_angularVel_reactiveAngle.w > 0) {
				ofPath arc;
				arc.setStrokeWidth(5);
				arc.setStrokeColor(ofColor(0));
				arc.setFillColor(ofColor(0));
				arc.arc(300 + p.pos.x * scaleFac, (1 - p.pos.y) * scaleFac, radius * scaleFac + 2, radius * scaleFac + 2, -p.mass_angle_angularVel_reactiveAngle.y - (0.5 * p.mass_angle_angularVel_reactiveAngle.w), -p.mass_angle_angularVel_reactiveAngle.y + (0.5 * p.mass_angle_angularVel_reactiveAngle.w));
				arc.close();
				arc.draw();
			}
			ofSetColor(p.color);
			ofDrawCircle(300 + p.pos.x * scaleFac, (1 - p.pos.y) * scaleFac, radius * scaleFac);	
		}
	}
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
	ofDrawBitmapString("Energy Slope: " + ofToString((energyLevels.size() > 2 ? energyLevels[energyLevels.size() - 1] - energyLevels[energyLevels.size() - 2] : 0), 2), 10, 180);

	gui.setPosition(10, 190);
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
