#version 430 core

in vec4 vColor; 
in vec2 vWorldPos;
out vec4 FragColor;

uniform bool u_fog_enabled;
uniform vec2 u_fog_center_world;
uniform float u_fog_start_radius;
uniform float u_fog_end_radius;
uniform vec4 u_fog_color;

void main()
{
    vec4 final_color = vColor;

    if (u_fog_enabled)
    {
        float distance_to_center = distance(vWorldPos, u_fog_center_world);
        float fog_factor = smoothstep(u_fog_start_radius, u_fog_end_radius, distance_to_center);
        final_color.rgb = mix(final_color.rgb, u_fog_color.rgb, fog_factor);
    }

    FragColor = final_color;
}