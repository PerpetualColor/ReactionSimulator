#version 440

struct Particle {
	vec2 pos;
	vec2 vel;
	vec4 mass;
	vec4 color;
};

layout(std140, binding = 0) buffer particle {
	Particle particles[];
};

layout(std140, binding = 1) buffer particle2 {
	Particle particlesOrig[];
};

uniform int nParticles;
uniform float lastFrameTime;
uniform float radius;

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
void main() {
	// check for collision
	
	for (int i = 0; i < nParticles; i++) {
		if (i != gl_GlobalInvocationID.x) {
			float dist = distance(particlesOrig[gl_GlobalInvocationID.x].pos, particlesOrig[i].pos);
			if ( dist <= radius*2) {
				Particle p1 = particlesOrig[gl_GlobalInvocationID.x];
				Particle p2 = particlesOrig[i];
				particles[gl_GlobalInvocationID.x].vel = 
				p1.vel - 
				(2 * p2.mass.x / (p1.mass.x + p2.mass.x)) * 
				(dot(p1.vel - p2.vel, p1.pos - p2.pos) / (pow(length(p1.pos - p2.pos), 2))) *
				(p1.pos - p2.pos);
				// move the particles out of radius
				vec2 dir = normalize(p1.pos - p2.pos);
				float amount = (2 * radius - dist)/2;
				particles[gl_GlobalInvocationID.x].pos += dir * amount;
			}
		}
	}
	particles[gl_GlobalInvocationID.x].color = vec4(5*abs(particles[gl_GlobalInvocationID.x].vel), 0, 255);

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