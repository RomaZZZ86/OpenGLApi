#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform;
uniform vec2 position;

void main()
{    
     
	vec4 scaledVertex  = (transform * vec4(aPos, 1.0))+vec4(position,0.0,1.0);
	gl_Position  = vec4(2.0*scaledVertex.x - 1.0,
                                1.0 - 2.0*scaledVertex.y,
                                0.0, 1.0);
	//TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	TexCoord = vec2(aPos.x, aPos.y);
	
}

