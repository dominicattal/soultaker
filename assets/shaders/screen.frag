#version 430 core
out vec4 FragColor;
  
in vec2 tex_coords;

uniform sampler2D screenTex;

void main()
{ 
    FragColor = texture(screenTex, tex_coords);
}
