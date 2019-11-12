#include "ofApp.h"
#include <set>
#include <fstream>

//--------------------------------------------------------------
void ofApp::setup() {
	// zero all values
	particleGui = {};
	initParticleCounts = {};
	concentrationData = {};
	concentrationRawData = {};
	particles.resize(0);

	loadFile();

	nParticles = 0;
	energyMax = 1;
	updateCount = 0;
	avgEnergyTotal = 0;
	avgEnergyCount = 0;
	maxNTypeParticle = 0;

	vector<ofFloatColor> particleColors;
	particleColors.resize(particleTypes.size());
	for (int i = 0; i < particleTypes.size(); i++) {
		particleColors[i] = particleTypes[i].color;
	}

	if (initialSetup) {
		radius = 0.01;
		guiRestart.addListener(this, &ofApp::setup);
		guiParticleRadius.addListener(this, &ofApp::radiusChanged);
		guiLimitFramerate.addListener(this, &ofApp::framerateLimiterToggle);
		gui.setup();
		gui.add(guiPause.setup("Pause", false));
		gui.add(guiLimitFramerate.setup("Limit framerate", true));
		gui.add(guiAdjustVelocity.setup("Velocity Multiplier", 1.00, 0.99, 1.01));
		gui.add(guiDrawParticles.setup("Draw Particles", true));
		gui.add(guiRestart.setup("Restart"));
		gui.add(guiUseMoleFraction.setup("Mole Fraction/Raw Data", true));
		gui.setName("Debug");

		// ofSetFrameRate(60);
		ofSetVerticalSync(true);

		initParticleCountsData.resize(particleTypes.size() - 1);
		for (int i = 0; i < initParticleCountsData.size(); i++) {
			if (i == 0) {
				initParticleCounts.add(initParticleCountsData[i].set(string("Initial ") + (char)('A' + i), 1000, 0, 10000));
			}
			else {
				initParticleCounts.add(initParticleCountsData[i].set(string("Initial ") + (char)('A' + i), 0, 0, 10000));
			}
		}
	}
	else {
		while (initParticleCountsData.size() < particleTypes.size() - 1) {
			initParticleCountsData.push_back(0);
			initParticleCountsData.back().set(0);
		}
		while (initParticleCountsData.size() > particleTypes.size() - 1) {
			initParticleCountsData.pop_back();
		}
		for (int i = 0; i < initParticleCountsData.size(); i++) {
			initParticleCounts.add(initParticleCountsData[i].set(string("Initial ") + (char)('A' + i), initParticleCountsData[i], 0, 10000));
		}
	}

	particleGui.setup();
	particleGui.add(guiParticleRadius.set("Particle Radius", radius, 0.001, 0.05));
	particleGui.add(initParticleCounts);
	particleGui.setName("Particle Info");

	addEnergyGui.setup();
	guiAddEnergy.addListener(this, &ofApp::addEnergy);
	addEnergyGui.add(guiEnergyToAdd.setup("Amount",0, -100, 100));
	addEnergyGui.add(guiAddEnergy.setup("Add"));

	compute.setupShaderFromFile(GL_COMPUTE_SHADER, "position_compute.glsl");
	compute.linkProgram();

	particles.resize(nParticles);

	for (int i = 0; i < initParticleCountsData.size(); i++) {
		for (int j = 0; j < initParticleCountsData[i]; j++) {
			Particle p;
			p.pos.x = ofRandom(1.0);
			p.pos.y = ofRandom(1.0);
			p.vel.set(ofRandom(0.5) - 0.25, ofRandom(0.5) - 0.25);
			p.mass_angle_angularVel_reactiveAngle.set(particleTypes[i + 1].mass, ofRandom(720) - 360, ofRandom(720) - 360, particleTypes[i + 1].reactiveAngle);
			p.collision_state.set(0, i + 1, 0, 0);
			concentrationData[int(p.collision_state.y)].back() += 1; 
			nParticles++;
			particles.push_back(p);
		}
	}

	for (int i = 1; i < concentrationData.size(); i++) {
		concentrationRawData[i].back() = concentrationData[i].back();
		if (concentrationRawData[i].back() > maxNTypeParticle) {
			maxNTypeParticle = concentrationRawData[i].back();
		}
		concentrationData[i].back() /= nParticles;
	}

	if (initialSetup) {
		colorBuffer.allocate(particleColors, GL_STATIC_READ);
		particleBuffer1.allocate(particles, GL_DYNAMIC_DRAW);
		particleBuffer2.allocate(particles, GL_DYNAMIC_DRAW);
	}
	else {
		colorBuffer.setData(particleColors, GL_STATIC_READ);
		particleBuffer1.setData(particles, GL_DYNAMIC_DRAW);
		particleBuffer2.setData(particles, GL_DYNAMIC_DRAW);
	}
	colorBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	particleBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	particleBuffer1.bindBase(GL_SHADER_STORAGE_BUFFER, 0);

	float e = 0;
	for (auto &p : particles) {
		e += 0.5 * p.mass_angle_angularVel_reactiveAngle.x * p.vel.lengthSquared();
	}
	initialEnergy = e;

	initialSetup = false;

}

void ofApp::framerateLimiterToggle(bool &newState) {
	ofSetVerticalSync(newState);
}

void ofApp::addEnergy() {
	initialEnergy += guiEnergyToAdd;
}

void ofApp::loadFile() {
	ifstream inputFile;
	inputFile.open("reaction_data.txt");
	particleTypes = {};
	reactions = {};
	if (inputFile.fail()) {
		cout << "File failed" << endl;
		particleTypes.resize(5);
		// particle A
		particleTypes[1] = ParticleType{ 60, ofColor(255, 0, 0), 1.0 };
		// particle B
		particleTypes[2] = ParticleType{ 120, ofColor(0, 255, 0), 2.0 };
		// particle C
		particleTypes[3] = ParticleType{ 180, ofColor(0, 0, 255), 1.5 };
		// particle D
		particleTypes[4] = ParticleType{ 120, ofColor(255, 255, 0), 1.5 };

		// A + A -> B, deltaE = 0
		reactions.push_back(Reaction{
			1, 1, { 2 }, 0, 0
			});
		// A + B -> C + D, deltaE = 0.01
		reactions.push_back(Reaction{
			1, 2, {3, 4}, 0.05, 0
			});
	}
	else {
		cout << "Using file" << endl;
		string line;
		while (getline(inputFile, line)) {
			istringstream inputStream(line);
			string type;
			inputStream >> type;
			if (type == "Reaction") {
				char a, b;
				float eA, dE;
				inputStream >> eA;
				inputStream >> dE;
				inputStream >> a;
				inputStream >> b;
				inputStream.ignore(100, ' ');
				inputStream.ignore(100, ' ');
				vector<char> productsChars;
				while (!inputStream.eof()) {
					char p;
					
					inputStream >> p;
					inputStream.ignore(1);
					productsChars.push_back(p);
				}
				
				vector<int> products;
				for (auto c : productsChars) {
					if (c != '0') {
						products.push_back(c + 1 - 'A');
					}
				}

				int A = a + 1 - 'A';
				int B = b + 1 - 'A';
				reactions.push_back(Reaction{
					A, B, products, eA, dE
					});

				cout << "reaction: " << A << " + " << B << " -> ";
				for (auto i : products) {
					cout << i << " ";
				}
				cout << endl;
			}
			else if (type == "Particle") {
				char A;
				inputStream >> A;
				int id = A - 'A' + 1;
				if (particleTypes.size() < id + 1) {
					particleTypes.resize(id + 1);
				}
				float angle, colorR, colorG, colorB, mass;
				inputStream >> angle;
				inputStream >> colorR;
				inputStream >> colorG;
				inputStream >> colorB;
				inputStream >> mass;
				particleTypes[id] = ParticleType{ angle, ofColor(colorR, colorG, colorB), mass };
			}
		}
	}

	concentrationData.resize(particleTypes.size());
	concentrationRawData.resize(particleTypes.size());
	for (int i = 1; i < concentrationData.size(); i++) {
		concentrationData[i].push_back(0);
		concentrationRawData[i].push_back(0);
	}

}

void ofApp::radiusChanged(float &newRadius) {
	radius = newRadius;
}

//--------------------------------------------------------------
void ofApp::update() {

	fps = ofGetFrameRate();

	if (!guiPause) {

		compute.begin();
		compute.setUniform1i("nParticles", nParticles);
		compute.setUniform1f("lastFrameTime", 1.0 / 60);
		compute.setUniform1f("radius", radius);
		compute.setUniform1f("velocityMult", guiAdjustVelocity);

		compute.dispatchCompute((particles.size() + 1024 - 1) / 1024, 1, 1);
		compute.end();

		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, nParticles * sizeof(Particle), &(particles[0]));
		// handle combination collisions
		vector<int> removedIndices;
		set<int> unavailableIndices;


		// add zeroed concentration values
		for (int i = 1; i < concentrationData.size(); i++) {
			concentrationData[i].push_back(0);
			concentrationRawData[i].push_back(0);
		}

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
					// perform a reaction

					if (curReaction.products.size() == 1) {
						// Temperature is a measure of the particle's velocity. However, conservation of momentum cannot be violated.
						// deltaE of the energy lost is accounted for by the chemical bonds. The rest becomes angular momentum
						float totalMass = particles[i].mass_angle_angularVel_reactiveAngle.x + particles[idx].mass_angle_angularVel_reactiveAngle.x;
						ofVec2f vFinal = ((particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel) + (particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel)) / totalMass;
						float kFinal = 0.5 * totalMass * vFinal.lengthSquared();
						float kInitial = 0.5 * (particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel.lengthSquared() - particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel.lengthSquared());
						float energyChange = kFinal - kInitial;

						if (-energyChange > curReaction.activationEnergy) {
							// the reaction occurs
							unavailableIndices.insert(i);
							unavailableIndices.insert(idx);
							removedIndices.push_back(idx);

							particles[i].mass_angle_angularVel_reactiveAngle.x = totalMass;
							particles[i].vel = vFinal;
							int productType = curReaction.products[0];

							particles[i].mass_angle_angularVel_reactiveAngle.w = particleTypes[productType].reactiveAngle;
							particles[i].collision_state.y = productType;

							initialEnergy -= curReaction.deltaE;
						}
					}
					else {
						// we know the final kinetic energy and momentum, just has to be divided among particles
						float kFinal = 0.5 * (particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel.lengthSquared() - particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel.lengthSquared());
						if (kFinal > curReaction.activationEnergy) {
							float totalMass = particles[i].mass_angle_angularVel_reactiveAngle.x + particles[idx].mass_angle_angularVel_reactiveAngle.x;

							ofVec2f vFinal = ((particles[i].mass_angle_angularVel_reactiveAngle.x * particles[i].vel) + (particles[idx].mass_angle_angularVel_reactiveAngle.x * particles[idx].vel));
							vFinal = vFinal.normalize();
							unavailableIndices.insert(i);
							unavailableIndices.insert(idx);
							removedIndices.push_back(idx);
							removedIndices.push_back(i);
							ofVec2f initPos = particles[i].pos;
							float kPart = kFinal / curReaction.products.size();
			
							// velocity is the required velocity to maintain kinetic energy in the direction of the collision if it were inelastic plus a random rotation
							for (int d = 0; d < curReaction.products.size(); d ++) {
								Particle p;
								p.pos = initPos + (2 * radius * ofVec2f(1, 0).rotate(d * 360 / curReaction.products.size()));
								p.mass_angle_angularVel_reactiveAngle.set(particleTypes[curReaction.products[d]].mass, ofRandom(720) - 360, ofRandom(720) - 360, particleTypes[curReaction.products[d]].reactiveAngle);
								p.vel = vFinal.rotate(ofRandom(90) - 45) * sqrt(2 * kPart / p.mass_angle_angularVel_reactiveAngle.x);
								p.collision_state.y = curReaction.products[d];

								particles.push_back(p);
							}

							initialEnergy -= curReaction.deltaE;
						}
					}
				}
			}
			// add to concentration of particle's type
			concentrationData[int(particles[i].collision_state.y)].back() += 1;
		}
		for (int i = 1; i < concentrationData.size(); i++) {
			concentrationRawData[i].back() = concentrationData[i].back();
			if (concentrationRawData[i].back() > maxNTypeParticle) {
				maxNTypeParticle = concentrationRawData[i].back();
			}
			concentrationData[i].back() /= nParticles;
		}

		sort(removedIndices.begin(), removedIndices.end(), greater<int>());

		for (int i = 0; i < removedIndices.size(); i++) {
			particles[removedIndices[i]] = particles.back();
			particles.pop_back();
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
			if (velMultScaleFac / initialEnergy > 0.005) {
				velMultScaleFac += abs((avgEnergyTotal / avgEnergyCount) - initialEnergy);
				if (e < initialEnergy && m < 0.0) {
					guiAdjustVelocity = guiAdjustVelocity + (0.0003 * velMultScaleFac);
				}
				else if (e > initialEnergy && m > 0.0) {
					guiAdjustVelocity = guiAdjustVelocity - (0.0003 * velMultScaleFac);
				}
			}
			else {
				guiAdjustVelocity = 1.0;
			}

		}
		updateCount++;

		nParticles = particles.size();
		particleBuffer1.setData(particles, GL_DYNAMIC_DRAW);

		particleBuffer1.copyTo(particleBuffer2);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofClear(0);
	ofFill();
	float scaleFac = min(ofGetWidth() - 900, ofGetHeight());
	if (guiDrawParticles) {
		for (auto &p : particles) {
			if (p.mass_angle_angularVel_reactiveAngle.w > 0 && p.mass_angle_angularVel_reactiveAngle.w < 360) {
				ofPath arc;
				arc.setStrokeWidth(5);
				arc.setStrokeColor(ofColor(255));
				arc.setFillColor(ofColor(255));
				arc.arc(300 + p.pos.x * scaleFac, (1 - p.pos.y) * scaleFac, radius * scaleFac + 2, radius * scaleFac + 2, -p.mass_angle_angularVel_reactiveAngle.y - (0.5 * p.mass_angle_angularVel_reactiveAngle.w), -p.mass_angle_angularVel_reactiveAngle.y + (0.5 * p.mass_angle_angularVel_reactiveAngle.w));
				arc.close();
				arc.draw();
			}
			ofSetColor(p.color);
			ofDrawCircle(300 + p.pos.x * scaleFac, (1 - p.pos.y) * scaleFac, radius * scaleFac);
		}
	}
	ofSetColor(255);
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
	ofDrawBitmapString("Overall Avg. Energy: " + ofToString(avgEnergyTotal / avgEnergyCount, 2), 10, 60);
	ofDrawBitmapString("Energy Slope: " + ofToString((energyLevels.size() > 2 ? energyLevels[energyLevels.size() - 1] - energyLevels[energyLevels.size() - 2] : 0), 2), 10, 180);

	// create graphs of particle concentrations

	int xOffset = 300 + 30 + scaleFac;
	ofDrawBitmapString("Particle Concentrations (mole fraction)", xOffset, 10);
	float xFac;
	if ((5 * (concentrationData[1].size() - 1)) < 300) {
		xFac = 1;
	}
	else {
		xFac = 300.0 / (5 * (concentrationData[1].size() - 1));
	}
	float graphHeight = (scaleFac - 40 - (concentrationData.size() * 30)) / (concentrationData.size() - 1);

	for (int i = 1; i < concentrationData.size(); i++) {
		ofPolyline curGraph;
		ofSetColor(particleTypes[i].color);
		for (int j = 0; j < concentrationData[i].size(); j++) {
			if (guiUseMoleFraction) {
				curGraph.addVertex(xOffset + (5 * j * xFac), 20 + (((graphHeight + 30) * i) - (concentrationData[i][j] * graphHeight)));
			}
			else {
				curGraph.addVertex(xOffset + (5 * j * xFac), 20 + (((graphHeight + 30) * i) - (concentrationRawData[i][j] * graphHeight / maxNTypeParticle)));
			}
		}
		curGraph.draw();
		if (guiUseMoleFraction) {
			ofDrawBitmapString(ofToString(concentrationData[i].back(), 2), (xOffset - 16) + (5 * (concentrationData[i].size() - 1) * xFac), 20 - 11 + (((graphHeight + 30) * i) - (concentrationData[i].back() * graphHeight)));
		}
		else {
			ofDrawBitmapString(ofToString(concentrationRawData[i].back()), (xOffset - 16) + (5 * (concentrationData[i].size() - 1) * xFac), 20 - 11 + (((graphHeight + 30) * i) - (concentrationRawData[i].back() * graphHeight / maxNTypeParticle)));
		}
		int markers = 4;
		for (int n = 0; n <= markers; n++) {
			if (guiUseMoleFraction) {
				ofDrawBitmapString(ofToString((float)n / (float)markers, 2), xOffset - 8, 20 + (((graphHeight + 30) * i) - (((float)n / markers) * graphHeight)));
			}
			else {
				ofDrawBitmapString(ofToString((float)n * maxNTypeParticle / (float)markers, 2), xOffset - 8, 20 + (((graphHeight + 30) * i) - (((float)n / markers) * graphHeight)));
			}

			ofDrawBitmapString(ofToString((1.0 / 60) * concentrationData[1].size() * (float)n / (float)markers, 2), xOffset + (n * (300 / markers)), 35 + ((graphHeight + 30) * i));
		}

	}
	ofSetColor(0);

	gui.setPosition(10, 190);
	gui.draw();

	particleGui.setPosition(10, 410);
	particleGui.draw();

	addEnergyGui.setPosition(10, 320);
	addEnergyGui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	compute.setupShaderFromFile(GL_COMPUTE_SHADER, "position_compute.glsl");
	compute.linkProgram();
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
