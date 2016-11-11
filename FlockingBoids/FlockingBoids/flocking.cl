// reference from 
// https://gamedevelopment.tutsplus.com/tutorials/3-simple-rules-of-flocking-behaviors-alignment-cohesion-and-separation--gamedev-3444
// https://github.com/okinp/clBoids/
// https://github.com/Edgeworth/Boids

#define alignDist 50
#define cohereDist 50
#define separateDist 30
#define arriveDist 30

#define MAX_SPEED 25

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

__kernel void update(__global const Boid *in_buffer, __global Boid *out_buffer, const int numBoids, const float time, Boid leaderBoid, int screenWidth, int screenHeight) {
	int gid = get_global_id(0);

	///// Arrive
	float2 steerArrive = (float2)(0.0f, 0.0f);
	float2 direction = leaderBoid.position - in_buffer[gid].position;
	float dist = sqrt(direction.x * direction.x + direction.y * direction.y);
	float targetSpeed = 0.0f;
	if (dist > arriveDist)
		targetSpeed = MAX_SPEED;
	else
		targetSpeed = MAX_SPEED * dist / 75.0f;

	if (!(dist < arriveDist))
	{
		float2 targetVelocity = direction;
		float targetVelocityMag = sqrt(targetVelocity.x * targetVelocity.x + targetVelocity.y * targetVelocity.y);
		targetVelocity /= targetVelocityMag;
		targetVelocity *= targetSpeed;
		steerArrive = targetVelocity - in_buffer[gid].velocity;
		steerArrive /= 0.1f;

		float steerArrivalMag = sqrt(steerArrive.x * steerArrive.x + steerArrive.y * steerArrive.y);
		if (steerArrivalMag > 0.9)
		{
			steerArrive /= steerArrivalMag;
			//steerArrive *= MAX_SPEED;
		}
	}
	///////

	

	float2 steerAlign = (float2)(0.0f, 0.0f);
	float2 steerCohere = (float2)(0.0f, 0.0f);
	float2 steerSeparate = (float2)(0.0f, 0.0f);
	int alignCount = 0;
	int cohereCount = 0;
	int separateCount = 0;
	for (int i = gid+1; i < numBoids; i++) {
		float2 d = in_buffer[gid].position - in_buffer[i].position;
		float dist = sqrt(d.x * d.x + d.y * d.y);

		// alignment
		//if (dist > 0 && dist < alignDist) {
		//	steerAlign += in_buffer[i].velocity;
		//	alignCount += 1;
		//}

		//if (dist > 0 && dist < cohereDist) {
		//	steerCohere += in_buffer[i].position;
		//	cohereCount += 1;
		//}

		if (dist > 0 && dist < separateDist) {
			float2 diff = in_buffer[i].position - in_buffer[gid].position;
			/*float mag = sqrt(diff.x * diff.x + diff.y * diff.y);
			diff = diff / mag;*/
			steerSeparate += diff;
			separateCount += 1;
		}
	}

	//if (alignCount > 0) {
	//	steerAlign = steerAlign / alignCount;
	//	float mag = sqrt(steerAlign.x * steerAlign.x + steerAlign.y * steerAlign.y);
	//	if (mag != 0) {
	//		steerAlign /= mag;
	//	}
	//}

	//if (cohereCount > 0) {
	//	steerCohere = steerCohere / cohereCount;
	//	float mag = sqrt(steerCohere.x * steerCohere.x + steerCohere.y * steerCohere.y);
	//	if (mag != 0) {
	//		steerCohere /= mag;
	//	}
	//}


	if (separateCount > 0) {
		steerSeparate = steerSeparate / separateCount;
		steerSeparate *= -1;
		float mag = sqrt(steerSeparate.x * steerSeparate.x + steerSeparate.y * steerSeparate.y);
		if (mag != 0) {
			steerSeparate /= mag;
		}
	}

	out_buffer[gid].velocity = in_buffer[gid].velocity;
	//out_buffer[gid].velocity += steerAlign;
	//out_buffer[gid].velocity += steerCohere;
	out_buffer[gid].velocity += steerArrive;
	out_buffer[gid].velocity += (steerSeparate * 2);


	//// normalize
	float mag = sqrt(out_buffer[gid].velocity.x * out_buffer[gid].velocity.x +
		out_buffer[gid].velocity.y * out_buffer[gid].velocity.y);
	if (mag > 0)
		out_buffer[gid].velocity /= mag;
	out_buffer[gid].velocity *= MAX_SPEED; // max velocity


	// update position
	out_buffer[gid].position = out_buffer[gid].velocity * time + in_buffer[gid].position;
	if (out_buffer[gid].position.x < 0)
		out_buffer[gid].position.x += screenWidth-20;
	else if (out_buffer[gid].position.y < 0)
		out_buffer[gid].position.y += screenHeight-20;
	else if (out_buffer[gid].position.x > screenWidth)
		out_buffer[gid].position.x -= screenWidth+20;
	else if (out_buffer[gid].position.y > screenHeight)
		out_buffer[gid].position.y -= screenHeight+20;

	out_buffer[gid].acceleration *= 0;
	out_buffer[gid].maxSpeed = 25.0f;
	out_buffer[gid].maxForce = 0.9f;
}