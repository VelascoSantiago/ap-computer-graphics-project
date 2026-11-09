#version 330 core
// HUD 2D: un cuadro (0..1) colocado con 'rect' = (x, y, ancho, alto) en pixeles, origen arriba-izquierda
layout (location = 0) in vec2 pos;

uniform vec4 rect;
uniform vec2 screen;   // tamaño del framebuffer en pixeles

out vec2 uv;

void main()
{
    vec2 p = rect.xy + pos * rect.zw;
    gl_Position = vec4(p.x / screen.x * 2.0 - 1.0, 1.0 - p.y / screen.y * 2.0, 0.0, 1.0);
    uv = pos;
}
