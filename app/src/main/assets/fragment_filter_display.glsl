#version 300 es
precision mediump float;

out vec4 outColor;
in vec2 vTextCoord;
uniform sampler2D yTexture;
uniform sampler2D uTexture;
uniform sampler2D vTexture;

vec4 yuvToRGBAColor(sampler2D ySampler, sampler2D uSampler, sampler2D vSampler, vec2 textCoord) {
    vec3 yuv;
    vec3 rgb;
    //分别取yuv各个分量的采样纹理
    yuv.x = texture(ySampler, textCoord).g;
    yuv.y = texture(uSampler, textCoord).g - 0.5;
    yuv.z = texture(vSampler, textCoord).g - 0.5;
    rgb = mat3(
    1.0, 1.0, 1.0,
    0.0, -0.39465, 2.03211,
    1.13983, -0.5806, 0.0
    ) * yuv;
    vec4 nColor = vec4(rgb.r, rgb.g, rgb.b, 1);
    return nColor;
}

void main() {
    vec4 nColor = yuvToRGBAColor(yTexture, uTexture, vTexture, vTextCoord);
    outColor = nColor;
}
