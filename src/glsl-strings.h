#ifndef __SHADERTOY_GLSLSTRINGS_H__
#define __SHADERTOY_GLSLSTRINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <common-header.glsl>
#include <fragment-footer.glsl>
#include <fragment-header.glsl>
#include <fragment-shader.glsl>
#include <vertex-shader.glsl>

#ifdef __cplusplus
}
#endif

#define GLSL_STRING(name) ((char *) __##name##_glsl)
#define GLSL_STRLEN(name) ((char *) __##name##_glsl_len)

#endif
