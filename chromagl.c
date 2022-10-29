#include "chromagl.h"

const char *vxshad = 
	"#version 450 core\n"
	"layout(location = 0) in vec3 position;"
	""
	"layout(location = 1) in vec4 acolor;"
	"layout(location = 2) in vec4 offset;"
	"out vec4 vacolor;"
	"out float op;"
	"layout(location = 0) uniform mat4 modelViewMatrix;"
	"layout(location = 1) uniform mat4 projectionMatrix;"
	"layout(location = 2) uniform vec3 lineSize;"
	"void main() {"
	"vacolor = acolor;"
	"op = offset.a;"
	"vec4 modelViewPosition = modelViewMatrix * vec4(offset.xyz + position*lineSize, 1.0);"
	"gl_Position = projectionMatrix * modelViewPosition;"
	"}";

const char *fgshad =
	"#version 450 core\n"
	"in vec4 vacolor;"
	"in float op;"
	"out vec4 FragColor;"
	"layout(location = 3) uniform float tipFrac;"
	"layout(location = 4) uniform vec2 resolution;"
	"layout(location = 5) uniform float ablend;"
	"layout(location = 6) uniform float alen1;"
	"layout(location = 7) uniform float alen2;"
	"layout(location = 8) uniform vec3 acolor1[5];"
	"layout(location = 13) uniform vec3 acolor2[5];"
	"float map(float v, float f1, float f2, float t1, float t2) { return (v-f1)*(t2-t1)/(f2-f1);}"

	"float min( vec3 a ) { return min( a.x, min( a.y, a.z ) ); }"
	"float max( vec3 a ) { return max( a.x, max( a.y, a.z ) ); }"
	"vec3 mixa( vec3 col1, vec3 col2, float gradient ) { float m = ( max( col1 ) + max( col2 ) ) / 2.; vec3 c = ( col1 + col2 ) * .5; float d = 2. * abs( gradient - .5 ) * min( c ); c = ( c - d ) / ( 1. - d ); c *= m / max( c ); float s = step( .5, gradient ); gradient *= 2.; return ( 1. - s ) * mix( col1, c, gradient ) + s * mix( c, col2, gradient - 1. ); }"	

	"void main() {"
	"FragColor = vacolor;"
	"FragColor.a *= op;"
	""
	"float div1 = 1.0/(alen1-1.0);"
	"float div2 = 1.0/(alen2-1.0);"
	"float x = gl_FragCoord.x / resolution.x;"
	"vec3 color1 = mix(acolor1[0],acolor1[1], smoothstep(0.0, div1, x));"
	"color1 = mix(color1,acolor1[2], smoothstep(div1, div1*2.0, x));"
	"color1 = mix(color1,acolor1[3], smoothstep(div1*2.0, div1*3.0, x));"
	"color1 = mix(color1,acolor1[4], smoothstep(div1*3.0, div1*4.0, x));"
	"vec3 color2 = mix(acolor2[0],acolor2[1], smoothstep(0.0, div2, x));"
	"color2 = mix(color2,acolor2[2], smoothstep(div2, div2*2.0, x));"
	"color2 = mix(color2,acolor2[3], smoothstep(div2*2.0, div2*3.0, x));"
	"color2 = mix(color2,acolor2[4], smoothstep(div2*3.0, div2*4.0, x));"
	"FragColor *= vec4(mixa(color1/255.0, color2/255.0, ablend), 1.0);"
	"float qq = clamp(map(vacolor.a,tipFrac,1.0,0.0,1.0),0.0,1.0);"
	//"if (op > (200.0/255.0) && vacolor.a > (tipFrac)) {"
	//"float condition = max(sign(op - (200.0/255.0)), 0.0) * max(sign(vacolor.a-tipFrac), 0.0);"
	"float condition = max(sign(op - (200.0/255.0)), 0.0);"
	"FragColor.rgb = mix(FragColor.rgb,vec3(1.0),qq*condition);"
	"float bottom = smoothstep(0.0, resolution.y/2.0, gl_FragCoord.y);"
	"float top = smoothstep(resolution.y, resolution.y-resolution.y/4.0, gl_FragCoord.y);"
	"FragColor.rgb *= (top*0.7)+0.3;"
	//"FragColor.rgb *= (bottom*1.0)+0.0;"
	"FragColor.a = mix(0.01, FragColor.a, bottom);"
	"}";

// basic normalized quad - xyz position format
float ver[] = {
	0.0,0.0,0.0,
	1.0,0.0,0.0,
	0.0,1.0,0.0,

//	0.0,1.0,0.0,
//	1.0,0.0,0.0,
	1.0,1.0,0.0
};

// basic quad - rgba color format (vertical gradient)
float col[] = {
	1.0,1.0,1.0,0.0,
	1.0,1.0,1.0,0.0,
	1.0,1.0,1.0,1.0,

//	1.0,1.0,1.0,1.0,
//	1.0,1.0,1.0,0.0,
	1.0,1.0,1.0,1.0
};

float *lineBuffer;
int lineBufferCount = 0;

GLuint program1;
GLuint vao;
GLuint bufferOffset;

void initChromaGL() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// create shaders and program
	GLuint vxshadc = makeShader(vxshad, GL_VERTEX_SHADER);
	GLuint fgshadc = makeShader(fgshad, GL_FRAGMENT_SHADER);
	program1 = glCreateProgram();
	glAttachShader(program1, vxshadc);
	glAttachShader(program1, fgshadc);
	glLinkProgram(program1);

	// generate buffers and load data
	glGenVertexArrays(1, &vao);
	GLuint bufferPosition;
	GLuint bufferAlphaColor;
	glGenBuffers(1, &bufferPosition);
	glGenBuffers(1, &bufferAlphaColor);
	glGenBuffers(1, &bufferOffset);

	// basic normalized quad
	glBindBuffer(GL_ARRAY_BUFFER, bufferPosition);
	glBindVertexArray(vao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ver), ver, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vertical gradient vtx colors
	glBindBuffer(GL_ARRAY_BUFFER, bufferAlphaColor);
	glBindVertexArray(vao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(col), col, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// line offsets (divisor)
	glBindBuffer(GL_ARRAY_BUFFER, bufferOffset);
	glBindVertexArray(vao);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(ver), col, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	glBindVertexArray(0);
}

void drawChromaGL(int screenWidth, int screenHeight, float *proMatrix, float *mvMatrix) {
	if (lineCount > lineBufferCount) {
		blockAlloc(lineCount, &lineBufferCount, 4*sizeof(float), (void **)&lineBuffer);
	}

	// serialize line buffer
	serializeLineList();
	// upload it to gpu
	glBindBuffer(GL_ARRAY_BUFFER, bufferOffset);
	glBufferData(GL_ARRAY_BUFFER, lineCount * 4 * sizeof(float), lineBuffer, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// next frame
	glUseProgram(program1);
	glUniformMatrix4fv(UNI_PM, 1, GL_FALSE, proMatrix);
	glUniformMatrix4fv(UNI_MVM, 1, GL_FALSE, mvMatrix);
	glUniform1f(UNI_TIPFRAC, (1.0 - (tipHeight / (float)lineHeight)));
	glUniform3f(UNI_LINESIZE, lineWidth, lineHeight, 1.0);
	glUniform1f(UNI_ALEN1, alen1);
	glUniform1f(UNI_ALEN2, alen2);
	glUniform1f(UNI_ABLEND, coTime / 255.0);
	glUniform2f(UNI_RESOLUTION, screenWidth, screenHeight);
	glUniform3fv(UNI_ACOLOR1, 5, (float *)acolor1);
	glUniform3fv(UNI_ACOLOR2, 5, (float *)acolor2);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao);
	//glDrawArrays(GL_TRIANGLES,0,6);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, lineCount);
}

void serializeLineList() {
	int index = 0;
	for (int i = 0; i < lineCount; i++) {
		lineBuffer[index++] = linelist[i].x;
		lineBuffer[index++] = linelist[i].y;
		lineBuffer[index++] = -100+(0.3*linelist[i].alpha*255);
		lineBuffer[index++] = linelist[i].alpha;
	}
}
