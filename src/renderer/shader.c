#include "../renderer.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

static GLuint shader_programs[NUM_SHADER_PROGRAMS];

static const char* read_file(const char *path)
{
    FILE* ptr;
    char* content;
    ptr = fopen(path, "r");
    fseek(ptr, 0, SEEK_END);
    i32 len = ftell(ptr);
    if (len == 0) {
        printf("File %s is empty", path);
        exit(1);
    }
    fseek(ptr, 0, SEEK_SET);
    content = calloc(len+1, sizeof(char));
    fread(content, 1, len, ptr);
    fclose(ptr);
    content[len] = '\0';
    return content;
}

static u32 compile(GLenum type, const char *path)
{
    u32 shader;
    const char* shader_code;
    char info_log[512];
    i32 success;
    DIR* dir = opendir(path);
    if (ENOENT == errno) {
        printf("File %s does not exist", path);
        exit(1);
    }
    closedir(dir);
    shader = glCreateShader(type);
    shader_code = read_file(path);
    glShaderSource(shader, 1, &shader_code, NULL);
    free((char*)shader_code);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        puts(path);
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
    return shader;
}

static void link(ShaderProgramEnum id)
{
    char info_log[512];
    i32 success;
    glLinkProgram(shader_programs[id]);
    glGetProgramiv(shader_programs[id], GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_programs[id], 512, NULL, info_log);
        printf(info_log);
        exit(1);
    }
}

static void attach(ShaderProgramEnum id, u32 shader)
{
    glAttachShader(shader_programs[id], shader);
}

static void detach(ShaderProgramEnum id, u32 shader)
{
    glDetachShader(shader_programs[id], shader);
}

static void delete(u32 shader)
{
    glDeleteShader(shader);
}

static void compile_shader_program_game(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/game.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/game.frag");
    attach(SHADER_PROGRAM_GAME, vert);
    attach(SHADER_PROGRAM_GAME, frag);
    link(SHADER_PROGRAM_GAME);
    detach(SHADER_PROGRAM_GAME, vert);
    detach(SHADER_PROGRAM_GAME, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_GAME);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_GAME, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_GAME, UBO_INDEX_MATRICES, "Matrices");
}

static void compile_shader_program_gui(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/gui.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/gui.frag");
    attach(SHADER_PROGRAM_GUI, vert);
    attach(SHADER_PROGRAM_GUI, frag);
    link(SHADER_PROGRAM_GUI);
    detach(SHADER_PROGRAM_GUI, vert);
    detach(SHADER_PROGRAM_GUI, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_GUI);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_GUI, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_GUI, UBO_INDEX_WINDOW, "Window");
}

void shader_program_compile(ShaderProgramEnum program)
{
    switch (program) {
        case SHADER_PROGRAM_GAME:
            compile_shader_program_game();
            break;
        case SHADER_PROGRAM_GUI:
            compile_shader_program_gui();
            break;
        default:
            printf("Unrecognized program %x\n", program);
            break;
    }
}

void shader_init(void)
{
    for (i32 i = 0; i < NUM_SHADER_PROGRAMS; i++)
        shader_programs[i] = glCreateProgram();
    
    shader_program_compile(SHADER_PROGRAM_GAME);
    shader_program_compile(SHADER_PROGRAM_GUI);
}

void shader_use(ShaderProgramEnum id)
{
    glUseProgram(shader_programs[id]);
}

void shader_cleanup(void)
{
    for (i32 i = 0; i < NUM_SHADER_PROGRAMS; i++)
        glDeleteProgram(shader_programs[i]);
}

GLuint shader_get_uniform_location(ShaderProgramEnum program, const char* identifier)
{
    return glGetUniformLocation(shader_programs[program], identifier);
}

void shader_bind_uniform_block(ShaderProgramEnum program, u32 index, const char* identifier)
{
    glUniformBlockBinding(shader_programs[program], glGetUniformBlockIndex(shader_programs[program], identifier), index);
}
