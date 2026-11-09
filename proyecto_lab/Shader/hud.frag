#version 330 core
in vec2 uv;
out vec4 color;

uniform sampler2D tex;
uniform float alpha;

void main()
{
    vec4 c = texture(tex, uv);
    color = vec4(c.rgb, c.a * alpha);
}
