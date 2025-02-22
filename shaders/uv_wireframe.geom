#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

in vec2 TexCoords[];
in vec3 WorldPos[];
in float ViewDistance[];

uniform sampler2D edgeMap;
uniform float minLineWidth = 1.0;
uniform float maxLineWidth = 3.0;
uniform float minDistance = 1.0;
uniform float maxDistance = 10.0;

out float LineWidth;

void EmitEdge(int i, int j)
{
    // Sample edge texture at both UV coordinates
    float edge1 = texture(edgeMap, TexCoords[i]).r;
    float edge2 = texture(edgeMap, TexCoords[j]).r;
    
    // If either point is on an edge
    if (edge1 > 0.5 || edge2 > 0.5) {
        // Calculate average view distance for this edge
        float avgDistance = (ViewDistance[i] + ViewDistance[j]) * 0.5;
        
        // Calculate line width based on distance
        float t = clamp((avgDistance - minDistance) / (maxDistance - minDistance), 0.0, 1.0);
        LineWidth = mix(minLineWidth, maxLineWidth, t);
        
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = gl_in[j].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
}

void main()
{
    // Check each edge of the triangle for UV discontinuity
    EmitEdge(0, 1);
    EmitEdge(1, 2);
    EmitEdge(2, 0);
} 