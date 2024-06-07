#version 450

layout(location = 0) in vec4 v_frag_color;

layout(location = 0) out vec4 o_out_color;

void main() {
    o_out_color = v_frag_color;
}
