#version 330 core

in vec2 TexCoords;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D entityTexture;
uniform bool useTexture;

void main()
{
    if (useTexture)
    {
        vec4 texColor = texture(entityTexture, TexCoords);
        // Discard transparent pixels
        if (texColor.a < 0.1)
            discard;
        // Multiply texture by vertex color for tinting
        FragColor = texColor * vColor;
    }
    else
    {
        // No texture, just use vertex color
        FragColor = vColor;
    }
}
