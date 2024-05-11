#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;

//varying vec3 v_texcoord;
varying mediump vec4 vColor;

//! [0]
void main()
{
    // Set fragment color from texture
    gl_FragColor = vColor;
}
//! [0]

