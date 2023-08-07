#version 400

uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float specular_exponent;

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat3 u_ModelViewMatrixInvTrans;  

uniform vec4 u_Light_loc[4];
uniform vec3 u_Light_La[4];
uniform vec3 u_Light_L[4];
uniform vec4 u_Light_att[4];

const float zero_f = 0.0f;
const float one_f = 1.0f;

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

out vec4 v_shaded_color;

vec3 lighting_equation(in vec3 position, in vec3 n) {
    vec3 total = vec3(0.0);
    float att_eff = 1.0;
    vec3 L_EC;
    for(int i = 0; i < 4; i++) {
        L_EC = u_Light_loc[i].xyz - position;
        if(u_Light_att[i].w != 0.0) {
            vec4 tmp_vec4;
		    tmp_vec4.x = one_f;
			tmp_vec4.z = dot(L_EC, L_EC);
			tmp_vec4.y = sqrt(tmp_vec4.z);
			tmp_vec4.w = zero_f;

            att_eff = one_f/dot(tmp_vec4, u_Light_att[i]);
        }
    	vec3 ambient = u_Light_La[i] * ambient_color;
        vec3 s = normalize( u_Light_loc[i].xyz - position );
        float sDotN = max( dot(s,n), 0.0 );
        vec3 diffuse = diffuse_color * sDotN;
    
        vec3 spec = vec3(0.0);
        if( sDotN > 0.0 ) {
            vec3 v = normalize(-position.xyz);
            vec3 r = reflect( -s, n );
            spec = specular_color * pow( max( dot(r, v), 0.0 ), specular_exponent );
        }
        total += att_eff * (ambient + u_Light_L[i] * (diffuse + spec));
    }
    return total;
}

void main(void) {	
	vec3 position_EC = vec3(u_ModelViewMatrix * vec4(a_position, one_f));
	vec3 normal_EC = normalize(u_ModelViewMatrixInvTrans * a_normal);  

	v_shaded_color = vec4(lighting_equation(position_EC, normal_EC), one_f);
	gl_Position = u_ModelViewProjectionMatrix * vec4(a_position, one_f);
}