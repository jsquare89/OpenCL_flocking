// reference from 
// https://gamedevelopment.tutsplus.com/tutorials/3-simple-rules-of-flocking-behaviors-alignment-cohesion-and-separation--gamedev-3444
// https://github.com/okinp/clBoids/
// https://github.com/Edgeworth/Boids


#define MAX_SPEED 50
#define SEPARATION_RADIUS (50)
#define ARRIVE_TARGET_RADIUS (30)
#define ARRIVE_SLOW_RADIUS (75)
#define ARRIVE_TIME_TO_TARGET (0.1f)
#define SEPARATION_WEIGHT (3)
#define ARRIVAL_WEIGHT (1)

typedef struct {
	float2 position, velocity, acceleration;
} Boid;

float magnitude(float2 v) {
	return sqrt(v.x * v.x + v.y * v.y);;
}

float2 normalized(float2 v) {
	float mag = sqrt(v.x * v.x + v.y * v.y);
	if (mag > 0) {
		return v / mag;
	} else {
		return (float2)(0.0f, 0.0f);
	}
}

__kernel void update(__global const Boid *in_buffer, 
	__global Boid *out_buffer, const int numBoids, const float time, 
	Boid leaderBoid, int screenWidth, int screenHeight) {
	int gid = get_global_id(0);

	///// Arrive
	float2 steerArrive = (float2)(0.0f, 0.0f);
	float2 direction = leaderBoid.position - in_buffer[gid].position;
	float dist = magnitude(direction);

	float targetSpeed = 0.0f;
	if (dist > ARRIVE_TARGET_RADIUS)
		targetSpeed = MAX_SPEED;
	else if (dist == ARRIVE_TARGET_RADIUS)
		targetSpeed = MAX_SPEED * dist / ARRIVE_SLOW_RADIUS;

	if (dist >= ARRIVE_TARGET_RADIUS)
	{
		float2 targetVelocity = normalized(direction) * targetSpeed;
		steerArrive = targetVelocity - in_buffer[gid].velocity;
		steerArrive /= ARRIVE_TIME_TO_TARGET;

		if (magnitude(steerArrive) > MAX_SPEED)
		{
			steerArrive = normalized(steerArrive) * MAX_SPEED;
		}
	}
	///////

	///// Separation
	float2 steerSeparate = (float2)(0.0f, 0.0f);
	int separateCount = 0;
	for (int i = gid+1; i < numBoids; i++) {
		float dist = magnitude(in_buffer[i].position - in_buffer[gid].position);
		if (dist > 0 && dist < SEPARATION_RADIUS) {
			float2 diff = in_buffer[i].position - in_buffer[gid].position;
			steerSeparate += diff;
			separateCount += 1;
		}
	}

	if (separateCount > 0) {
		steerSeparate = steerSeparate / separateCount;
		steerSeparate *= -1;
		if (magnitude(steerSeparate) > MAX_SPEED) {
			steerSeparate = normalized(steerSeparate) * MAX_SPEED;
		}
	}
	///////

	out_buffer[gid].acceleration += (steerArrive * ARRIVAL_WEIGHT);
	out_buffer[gid].acceleration += (steerSeparate * SEPARATION_WEIGHT);

	// update position
	out_buffer[gid].velocity = in_buffer[gid].velocity;
	out_buffer[gid].velocity += out_buffer[gid].acceleration;

	if (magnitude(out_buffer[gid].velocity) > MAX_SPEED) {
		out_buffer[gid].velocity = normalized(out_buffer[gid].velocity) * MAX_SPEED;
	}

	out_buffer[gid].position = out_buffer[gid].velocity * time + in_buffer[gid].position;
	out_buffer[gid].acceleration *= 0; // reset

	// wrap border
	if (out_buffer[gid].position.x < 0)
		out_buffer[gid].position.x += screenWidth;
	else if (out_buffer[gid].position.y < 0)
		out_buffer[gid].position.y += screenHeight;
	else if (out_buffer[gid].position.x > screenWidth)
		out_buffer[gid].position.x -= screenWidth;
	else if (out_buffer[gid].position.y > screenHeight)
		out_buffer[gid].position.y -= screenHeight;

}