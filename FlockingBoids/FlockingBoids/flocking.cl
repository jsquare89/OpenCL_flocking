// reference from 
// https://gamedevelopment.tutsplus.com/tutorials/3-simple-rules-of-flocking-behaviors-alignment-cohesion-and-separation--gamedev-3444
// https://github.com/okinp/clBoids/
// https://github.com/Edgeworth/Boids

#define alignDist 50
#define cohereDist 50
#define separateDist 50

typedef struct {
	float2 position, velocity, acceleration; 
	float maxForce, maxSpeed;
} Boid;

__kernel void hello_kernel(__global const float *a, 
	__global const float *b, 
	__global float *result) {
	int gid = get_global_id(0);

	result[gid] = a[gid] + b[gid];
}

__kernel void update(__global const Boid *in_buffer, __global Boid *out_buffer, const int numBoids, const float time) {
	int gid = get_global_id(0);

	// update position
	out_buffer[gid].position = in_buffer[gid].velocity * time + in_buffer[gid].position;


	float2 steerAlign = (float)(0.0f, 0.0f);
	float2 steerCohere = (float)(0.0f, 0.0f);
	float2 steerSeparate = (float)(0.0f, 0.0f);
	int alignCount = 0;
	int cohereCount = 0;
	int separateCount = 0;
	for (int i = gid+1; i < numBoids; i++) {
		float2 d = in_buffer[gid].position - in_buffer[i].position;
		float dist = sqrt(d.x * 2 + d.y * 2);

		// alignment
		if (dist > 0 && dist < alignDist) {
			steerAlign += in_buffer[i].velocity;
			alignCount += 1;
		}

		if (dist > 0 && dist < cohereDist) {
			steerCohere += in_buffer[i].position;
			cohereCount += 1;
		}

		if (dist > 0 && dist < separateDist) {
			float2 diff = in_buffer[gid].position - in_buffer[i].position;
			float mag = sqrt(diff.x * diff.x + diff.y * diff.y);
			diff = diff / mag;
			steerSeparate += diff;
			separateCount += 1;
		}
	}

	if (alignCount > 0) {
		steerAlign = steerAlign / alignCount;
		float mag = sqrt(steerAlign.x * steerAlign.x + steerAlign.y * steerAlign.y);
		if (mag != 0) {
			steerAlign /= mag;
		}
	}

	if (cohereCount > 0) {
		steerCohere = steerCohere / cohereCount;
		float mag = sqrt(steerCohere.x * steerCohere.x + steerCohere.y * steerCohere.y);
		if (mag != 0) {
			steerCohere /= mag;
		}
	}

	if (separateCount > 0) {
		steerSeparate = steerSeparate / separateCount;
		steerSeparate *= -1;
		float mag = sqrt(steerSeparate.x * steerSeparate.x + steerSeparate.y * steerSeparate.y);
		if (mag != 0) {
			steerCohere /= mag;
		}
	}

	out_buffer[gid].velocity = in_buffer[gid].velocity;
	out_buffer[gid].velocity += steerAlign;
	out_buffer[gid].velocity += steerCohere;
	out_buffer[gid].velocity += steerSeparate;


	//// normalize
	float mag = sqrt(out_buffer[gid].velocity.x * out_buffer[gid].velocity.x +
		out_buffer[gid].velocity.y * out_buffer[gid].velocity.y);
	out_buffer[gid].velocity.x /= mag;
	out_buffer[gid].velocity.y /= mag;
	out_buffer[gid].velocity *= 50.0f; // max velocity
}