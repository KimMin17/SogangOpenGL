#version 330

// Modified from a shader in OpenGL 4 Shading Language Cookbook - Third Edition by D. Wolff

uniform vec4 light_position[4];    // Light position in eye coords.
uniform vec3 light_La[4];          // Ambient light intesity
uniform vec3 light_L[4];           // Diffuse and specular light intensity
uniform vec4 light_att[4];

uniform struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
} Material;

in vec3 Position;
in vec3 Normal;
layout( location = 0 ) out vec4 FragColor;

vec3 phongModel( vec3 position, vec3 n ) {
    vec3 total = vec3(0.0);
    float att_eff = 1.0;
    vec3 L_EC;
    for(int i = 0; i < 4; i++) {
        L_EC = light_position[i].xyz - position;
        if(light_att[i].w != 0.0) {
            vec4 tmp_vec4;
		    tmp_vec4.x = 1.0;
			tmp_vec4.z = dot(L_EC, L_EC);
			tmp_vec4.y = sqrt(tmp_vec4.z);
			tmp_vec4.w = 0.0;

            att_eff = 1.0/dot(tmp_vec4, light_att[i]);
        }
        vec3 ambient = light_La[i] * Material.Ka;
        vec3 s = normalize( light_position[i].xyz - position );
        float sDotN = max( dot(s,n), 0.0 );
        vec3 diffuse = Material.Kd * sDotN;
        vec3 spec = vec3(0.0);
        if( sDotN > 0.0 ) {
            vec3 v = normalize(-position.xyz);
            vec3 r = reflect( -s, n );
            spec = Material.Ks * pow( max( dot(r, v), 0.0 ), Material.Shininess );
        }
        total += att_eff * (ambient + light_L[i] * (diffuse + spec));
    }
    return total;
}

void main() {
    FragColor = vec4(phongModel(Position, normalize(Normal)), 1.0);
}