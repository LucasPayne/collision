%option noyywrap nodefault
%{
#include "gen_shader_blocks.h"
%}

%%

"#define ".*"\n" { yylval = new_symbol(yytext); return HASHDEFINE; }
"block" { return BLOCK; }
"struct" { return STRUCT; }
"{" { return LEFTBRACE; }
"}" { return RIGHTBRACE; }
"[" { return LEFTBRACKET; }
"]" { return RIGHTBRACKET; }
";" { return SEMICOLON; }
[_A-z0-9]+ { yylval = new_symbol(yytext); return IDENTIFIER; }
" "|"\t"|"\n" { /* eat whitespace */ }
<<EOF>> { return LEX_DONE; }
. { return LEX_ERROR; }

%%

    /*
block Standard3D {
    mat4x4 mvp_matrix;
};

block StandardLoopWindow {
    float aspect_ratio;
    float time;
};

block DirectionalLights {
    #define MAX_NUM_DIRECTIONAL_LIGHTS 32
    struct DirectionalLights DirectionalLight {
        bool is_active;
        vec3 direction;
        vec4 color;
    };
    DirectionalLight directional_lights[MAX_NUM_DIRECTIONAL_LIGHTS];
};
    */
