#pragma once

const char LABLE_OUTLINE_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);
uniform vec4 u_effectColor;
uniform vec4 u_textColor;
uniform float u_effectType; // 0: None (Draw text), 1: Outline, 2: Shadow

void main()
{
    vec4 sample = texture2D(u_texture, v_texcoord0);
    // fontAlpha == 1 means the area of solid text (without edge)
    // fontAlpha == 0 means the area outside text, including outline area
    // fontAlpha == (0, 1) means the edge of text
    float fontAlpha = sample.a;

    // outlineAlpha == 1 means the area of 'solid text' and 'solid outline'
    // outlineAlpha == 0 means the transparent area outside text and outline
    // outlineAlpha == (0, 1) means the edge of outline
    float outlineAlpha = sample.r;

    if (u_effectType == 0.0) // draw text
    {
        gl_FragColor = v_color0 * vec4(u_textColor.rgb, u_textColor.a * fontAlpha);
    }
    else if (u_effectType == 1.0) // draw outline
    {
        // multipy (1.0 - fontAlpha) to make the inner edge of outline smoother and make the text itself transparent.
        gl_FragColor = v_color0 * vec4(u_effectColor.rgb, u_effectColor.a * outlineAlpha * (1.0 - fontAlpha));
    }
    else // draw shadow
    {
        gl_FragColor = v_color0 * vec4(u_effectColor.rgb, u_effectColor.a * outlineAlpha);
    }
}
)";
