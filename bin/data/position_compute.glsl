#version 440

const float PI = 3.1415926535;

struct Particle {
	vec2 pos;
	vec2 vel;
	vec4 mass;
	vec4 color;
	vec4 collision;
};

layout(std140, binding = 0) buffer particle {
	Particle particles[];
};

layout(std140, binding = 1) buffer particle2 {
	Particle particlesOrig[];
};

layout(std140, binding = 2) buffer colors {
	vec4 particleColors[];
};

uniform int nParticles;
uniform float lastFrameTime;
uniform float radius;
uniform float velocityMult;

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
void main() {
	// check for collision
	bool hasCollided = false;
	vec2 impulse = vec2(0, 0);
	float minDist = -1;
	particles[gl_GlobalInvocationID.x].color = particleColors[int(particles[gl_GlobalInvocationID.x].collision.y)];
	particles[gl_GlobalInvocationID.x].collision.x = -1;

	for (int i = 0; i < nParticles; i++) {
		if (i != gl_GlobalInvocationID.x) {
			float dist = distance(particlesOrig[gl_GlobalInvocationID.x].pos, particlesOrig[i].pos);
			
			if (dist <= radius*2) {
				if (dist < minDist || minDist == -1) {
					minDist = dist;
					Particle p1 = particlesOrig[gl_GlobalInvocationID.x];
					Particle p2 = particlesOrig[i];

					// combination collision turns it red
					if (p1.mass.y < 0) {
						p1.mass.y = 360 - ((-1 * int(p1.mass.y)) % 360);
					} else {
						p1.mass.y = int(p1.mass.y) % 360;
					}

					if (p2.mass.y < 0) {
						p2.mass.y = 360 - ((-1 * int(p2.mass.y)) % 360);
					} else {
						p2.mass.y = int(p2.mass.y) % 360;
					}
					float colAngle = atan(p2.pos.y - p1.pos.y, p2.pos.x - p1.pos.x) + 2*PI;
					colAngle *= 180.0/PI;
					colAngle = mod(colAngle, 360);
					float colAngle2 = mod(colAngle + 180, 360);
					if (abs(p1.mass.y - colAngle) < p1.mass.w / 2 && (abs(p2.mass.y - colAngle2)) < p2.mass.w / 2) {
						// particles[gl_GlobalInvocationID.x].color = vec4(1.0, 0, 0, 1.0);
						particles[gl_GlobalInvocationID.x].collision.x = i;
					}
					
					float lengthSquared = pow(distance(p1.pos, p2.pos), 2);
					if (lengthSquared == 0) {
						lengthSquared = pow(2 * radius, 2);
					}

					particles[gl_GlobalInvocationID.x].vel = 
					velocityMult *
					(p1.vel - 
					(2 * p2.mass.x / (p1.mass.x + p2.mass.x)) * 
					(dot(p1.vel - p2.vel, p1.pos - p2.pos) / (lengthSquared)) *
					(p1.pos - p2.pos));
					// move the particles out of radius
					vec2 dir = normalize(p1.pos - p2.pos);
					float amount = (2 * radius - dist)/2;
					particles[gl_GlobalInvocationID.x].pos += dir * amount;

				}
				
			}
		}
	}
	// particles[gl_GlobalInvocationID.x].color = vec4(colorFac*length(particles[gl_GlobalInvocationID.x].vel), colorFac*length(particles[gl_GlobalInvocationID.x].vel), colorFac*length(particles[gl_GlobalInvocationID.x].vel), 255);

	vec2 vel = particles[gl_GlobalInvocationID.x].vel;
	particles[gl_GlobalInvocationID.x].pos.xy += vel * lastFrameTime;
	Particle p = particles[gl_GlobalInvocationID.x];
	if (p.pos.x < 0.0) {
		particles[gl_GlobalInvocationID.x].pos.x = 0.0;
			particles[gl_GlobalInvocationID.x].vel.x *= -1.0;
	}
	if (p.pos.x > 1.0) {
		particles[gl_GlobalInvocationID.x].pos.x = 1.0;
			particles[gl_GlobalInvocationID.x].vel.x *= -1.0;
	}
	if (p.pos.y < 0.0) {
		particles[gl_GlobalInvocationID.x].pos.y = 0.0;
			particles[gl_GlobalInvocationID.x].vel.y *= -1.0;
	}
	if (p.pos.y > 1.0) {
		particles[gl_GlobalInvocationID.x].pos.y = 1.0;
			particles[gl_GlobalInvocationID.x].vel.y *= -1.0;
	}

	particles[gl_GlobalInvocationID.x].mass.y += particles[gl_GlobalInvocationID.x].mass.z * lastFrameTime;
}
