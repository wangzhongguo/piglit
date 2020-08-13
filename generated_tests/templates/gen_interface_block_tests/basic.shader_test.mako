[require]
GLSL >= 1.50
% for ext in extensions:
${ext}
% endfor

[vertex shader]
#version 150
% for ext in extensions:
#extension ${ext}: require
% endfor

${declare_uniform_types}

${layout}out VS_OUTPUT {
${declare_vs_output_types}
} vs_output;

void main()
{
${assign_vs_output_types}
}

[fragment shader]
#version 150
% for ext in extensions:
#extension ${ext}: require
% endfor

${layout}in VS_OUTPUT {
${declare_vs_output_types}
} vs_output;

out vec4 fs_output;

% if 'GL_ARB_gpu_shader_fp64' in extensions:
vec4 convert(in double val)
{
    return vec4(val);
}
% endif
% if 'GL_ARB_gpu_shader_int64' in extensions:
vec4 convert(in int64_t val)
{
    return vec4(val);
}
vec4 convert(in uint64_t val)
{
    return vec4(val);
}
% endif
vec4 convert(in float val)
{
    return vec4(val);
}

${calc_color}
void main()
{
    fs_output = vec4(0);
${calc_output_color}
}

[test]
link success
draw rect -1 -1 2 2
