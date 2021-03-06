//[FRAGMENT SHADER]
#version 330 
 

in vec3 OutNormal;

out vec4 FragColor;
 
void main()
{
	FragColor = 
		vec4(
			vec3(0,1,0) * max(dot(vec3( 1.0f, 1.0f, 0.0f ),OutNormal),0),
			1.0f
		);
}