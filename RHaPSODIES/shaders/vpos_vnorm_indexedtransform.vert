#version 430 compatibility

out int instance_id;

const int ARRAY_SIZE = 4864; // 64*2*(22+16)
uniform uint transform_offset;

layout (std430, binding = 2) buffer TransformBlock {
  mat4 model_transform[ARRAY_SIZE];
};

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;

out vec3 position_viewspace;
out vec3 normal_viewspace;

void main(){
	instance_id = gl_InstanceID;
	gl_Position = gl_ModelViewProjectionMatrix *
		model_transform[gl_InstanceID + transform_offset] *
		vec4(vertexPosition_modelspace, 1);

	position_viewspace = (gl_ModelViewMatrix * vec4(vertexPosition_modelspace, 1)).xyz;
	normal_viewspace   = gl_NormalMatrix * vertexNormal_modelspace;
}
