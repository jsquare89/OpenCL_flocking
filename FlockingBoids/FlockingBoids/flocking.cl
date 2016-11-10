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

__kernel void update(__global const Boid *in_buffer, __global Boid *out_buffer, const int numBoids) {
	int gid = get_global_id(0);
	out_buffer[gid] = in_buffer[gid];
}