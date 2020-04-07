#version 330

uniform vec3 light_direction;
uniform vec3 light_color;

in vec4 f_position;
in vec3 f_normal;
in vec4 f_color;



void main()
{
    float light_irradiance = dot(f_normal, -light_direction)+0.25f;
    gl_FragColor.rgb = light_irradiance * light_color.rgb * f_color.rgb;
    gl_FragColor.a = 1.0f;
}
