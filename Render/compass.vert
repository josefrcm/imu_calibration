#version 330

uniform mat4 proj_view_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

in vec4 v_position;
in vec3 v_normal;
in vec4 v_color;

out vec4 f_position;
out vec3 f_normal;
out vec4 f_color;



void main()
{
    f_position = model_matrix * v_position;
    f_normal = normal_matrix * v_normal;
    f_color = v_color / 255.0f;
    gl_Position = proj_view_matrix * model_matrix * v_position;
}
