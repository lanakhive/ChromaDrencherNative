add_library(gl-matrix
	mat3.c
	mat4.c
	quat.c
	str.c
	vec3.c
	gl-matrix.h
)

target_include_directories(gl-matrix PUBLIC .)