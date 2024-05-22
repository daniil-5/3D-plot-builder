#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying mediump vec4 vColor;

void main()
{
    // Set fragment color from texture
    gl_FragColor = vColor;
}


