#version 330 core

struct PointLight {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
uniform PointLight pointLight;

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

//uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool displayDepth;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation()
{
    // get vector between fragment position and light position
    vec3 fragToLight = fs_in.FragPos - pointLight.position;
     //use the fragment to light vector to sample from the depth map    
     float closestDepth = texture(depthMap, fragToLight).r;
     //it is currently in linear range between [0,1], let's re-transform it back to original depth value
     closestDepth *= far_plane;
     //now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    
	//test for shadows
	float bias = 0.15; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

	if(currentDepth > 8.0f && shadow != 0.0f)
	{
		//PCF
		shadow = 0.0f;
		int samples = 20;
		float viewDistance = length(viewPos - fs_in.FragPos);
		float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
		for(int i = 0; i < samples; ++i)
			{
			float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
			closestDepth *= far_plane;   // undo mapping [0;1]
			if(currentDepth - bias > closestDepth)
				shadow += 1.0;
			}
		shadow /= float(samples);
	}
    
// display closestDepth as debug (to visualize depth cubemap)
	if(displayDepth){
		FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
	}
        
    return shadow;
}

void main()
{
	// properties
	vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

	vec3 lightDir = normalize(pointLight.position - fs_in.FragPos);

    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);

    // Blinn-Phong specular reflection
    vec3 reflectDir = reflect(-lightDir, norm);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

    // attenuation
    float distance    = length(pointLight.position - fs_in.FragPos);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance)); 
				 
    // combine results
    vec3 ambient  = pointLight.ambient  * vec3(texture(diffuseTexture, fs_in.TexCoords));
    vec3 diffuse  = pointLight.diffuse  * diff * vec3(texture(diffuseTexture, fs_in.TexCoords));
    vec3 specular = pointLight.specular * spec * vec3(0.3f);//vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

	// Calculate Shadows
	float shadows = ShadowCalculation();
	vec3 lighting = (ambient + (1.0 - shadows) * (diffuse + specular)) * color;  

    if(!displayDepth){
		FragColor = vec4(lighting, 1.0);
	}
}