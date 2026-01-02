#include "../renderer.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

typedef struct {
    GLuint programs[NUM_SHADER_PROGRAMS];
} ShaderContext;

static ShaderContext shader_context;

static const char* read_file(const char *path)
{
    FILE* ptr;
    char* content;
    i32 len;
    ptr = fopen(path, "r");
    fseek(ptr, 0, SEEK_END);
    len = ftell(ptr);
    log_assert(len != 0, "File %s is empty", path);
    fseek(ptr, 0, SEEK_SET);
    content = st_calloc(len+1, sizeof(char));
    len = fread(content, 1, len, ptr);
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

    log_assert(errno != ENOENT, "File %s does not exist", path);

    closedir(dir);
    shader = glCreateShader(type);
    shader_code = read_file(path);
    glShaderSource(shader, 1, &shader_code, NULL);
    st_free((char*)shader_code);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        log_write(FATAL, "Failed to compile %s\n%s", path, info_log);
    }
    return shader;
}

static void link(ShaderProgramEnum id)
{
    char info_log[512];
    i32 success;
    glLinkProgram(shader_context.programs[id]);
    glGetProgramiv(shader_context.programs[id], GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shader_context.programs[id], 512, NULL, info_log);
        log_write(FATAL, "Failed to link shader %d\n%s", id, info_log);
    }
}

static void attach(ShaderProgramEnum id, u32 shader)
{
    glAttachShader(shader_context.programs[id], shader);
}

static void detach(ShaderProgramEnum id, u32 shader)
{
    glDetachShader(shader_context.programs[id], shader);
}

static void delete(u32 shader)
{
    glDeleteShader(shader);
}

static void compile_shader_program_screen(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/screen.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/screen.frag");
    attach(SHADER_PROGRAM_SCREEN, vert);
    attach(SHADER_PROGRAM_SCREEN, frag);
    link(SHADER_PROGRAM_SCREEN);
    detach(SHADER_PROGRAM_SCREEN, vert);
    detach(SHADER_PROGRAM_SCREEN, frag);
    delete(vert);
    delete(frag);
}

static void compile_shader_program_tile(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/tile.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/tile.frag");
    attach(SHADER_PROGRAM_TILE, vert);
    attach(SHADER_PROGRAM_TILE, frag);
    link(SHADER_PROGRAM_TILE);
    detach(SHADER_PROGRAM_TILE, vert);
    detach(SHADER_PROGRAM_TILE, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_TILE);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_TILE, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_TILE, UBO_INDEX_MATRICES, "Camera");
    shader_bind_uniform_block(SHADER_PROGRAM_TILE, UBO_INDEX_GAME_TIME, "GameTime");
    shader_use(SHADER_PROGRAM_NONE);
}

static void compile_shader_program_wall(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/wall.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/wall.frag");
    attach(SHADER_PROGRAM_WALL, vert);
    attach(SHADER_PROGRAM_WALL, frag);
    link(SHADER_PROGRAM_WALL);
    detach(SHADER_PROGRAM_WALL, vert);
    detach(SHADER_PROGRAM_WALL, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_WALL);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_WALL, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_WALL, UBO_INDEX_MATRICES, "Camera");
    shader_use(SHADER_PROGRAM_NONE);
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
    shader_use(SHADER_PROGRAM_NONE);
}

static void compile_shader_program_entity(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/entity.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/entity.frag");
    attach(SHADER_PROGRAM_ENTITY, vert);
    attach(SHADER_PROGRAM_ENTITY, frag);
    link(SHADER_PROGRAM_ENTITY);
    detach(SHADER_PROGRAM_ENTITY, vert);
    detach(SHADER_PROGRAM_ENTITY, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_ENTITY);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_ENTITY, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_ENTITY, UBO_INDEX_MATRICES, "Camera");
    shader_bind_uniform_block(SHADER_PROGRAM_ENTITY, UBO_INDEX_WINDOW, "Window");
    shader_use(SHADER_PROGRAM_NONE);
}

static void compile_shader_program_obstacle(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/obstacle.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/obstacle.frag");
    attach(SHADER_PROGRAM_OBSTACLE, vert);
    attach(SHADER_PROGRAM_OBSTACLE, frag);
    link(SHADER_PROGRAM_OBSTACLE);
    detach(SHADER_PROGRAM_OBSTACLE, vert);
    detach(SHADER_PROGRAM_OBSTACLE, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_OBSTACLE);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_OBSTACLE, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_OBSTACLE, UBO_INDEX_MATRICES, "Camera");
    shader_bind_uniform_block(SHADER_PROGRAM_OBSTACLE, UBO_INDEX_WINDOW, "Window");
    shader_use(SHADER_PROGRAM_NONE);
}

static void compile_shader_program_particle(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/particle.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/particle.frag");
    attach(SHADER_PROGRAM_PARTICLE, vert);
    attach(SHADER_PROGRAM_PARTICLE, frag);
    link(SHADER_PROGRAM_PARTICLE);
    detach(SHADER_PROGRAM_PARTICLE, vert);
    detach(SHADER_PROGRAM_PARTICLE, frag);
    delete(vert);
    delete(frag);
    shader_bind_uniform_block(SHADER_PROGRAM_PARTICLE, UBO_INDEX_MATRICES, "Camera");
    shader_bind_uniform_block(SHADER_PROGRAM_PARTICLE, UBO_INDEX_WINDOW, "Window");
}

static void compile_shader_program_projectile(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/projectile.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/projectile.frag");
    attach(SHADER_PROGRAM_PROJECTILE, vert);
    attach(SHADER_PROGRAM_PROJECTILE, frag);
    link(SHADER_PROGRAM_PROJECTILE);
    detach(SHADER_PROGRAM_PROJECTILE, vert);
    detach(SHADER_PROGRAM_PROJECTILE, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_PROJECTILE);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_PROJECTILE, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_PROJECTILE, UBO_INDEX_MATRICES, "Camera");
    shader_bind_uniform_block(SHADER_PROGRAM_PROJECTILE, UBO_INDEX_WINDOW, "Window");
    shader_use(SHADER_PROGRAM_NONE);
}

static void compile_shader_program_parjicle(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/parjicle.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/parjicle.frag");
    attach(SHADER_PROGRAM_PARJICLE, vert);
    attach(SHADER_PROGRAM_PARJICLE, frag);
    link(SHADER_PROGRAM_PARJICLE);
    detach(SHADER_PROGRAM_PARJICLE, vert);
    detach(SHADER_PROGRAM_PARJICLE, frag);
    delete(vert);
    delete(frag);
    shader_bind_uniform_block(SHADER_PROGRAM_PARJICLE, UBO_INDEX_MATRICES, "Camera");
    shader_bind_uniform_block(SHADER_PROGRAM_PARJICLE, UBO_INDEX_WINDOW, "Window");
}

static void compile_shader_program_shadow(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/shadow.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/shadow.frag");
    attach(SHADER_PROGRAM_SHADOW, vert);
    attach(SHADER_PROGRAM_SHADOW, frag);
    link(SHADER_PROGRAM_SHADOW);
    detach(SHADER_PROGRAM_SHADOW, vert);
    detach(SHADER_PROGRAM_SHADOW, frag);
    delete(vert);
    delete(frag);
    shader_bind_uniform_block(SHADER_PROGRAM_SHADOW, UBO_INDEX_MATRICES, "Camera");
}

static void compile_shader_program_minimap_circle(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/map_circle.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/map_circle.frag");
    attach(SHADER_PROGRAM_MINIMAP_CIRCLE, vert);
    attach(SHADER_PROGRAM_MINIMAP_CIRCLE, frag);
    link(SHADER_PROGRAM_MINIMAP_CIRCLE);
    detach(SHADER_PROGRAM_MINIMAP_CIRCLE, vert);
    detach(SHADER_PROGRAM_MINIMAP_CIRCLE, frag);
    delete(vert);
    delete(frag);
    shader_bind_uniform_block(SHADER_PROGRAM_MINIMAP_CIRCLE, UBO_INDEX_MINIMAP, "Minimap");
}

static void compile_shader_program_minimap_square(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, "assets/shaders/map_square.vert");
    frag = compile(GL_FRAGMENT_SHADER, "assets/shaders/map_square.frag");
    attach(SHADER_PROGRAM_MINIMAP_SQUARE, vert);
    attach(SHADER_PROGRAM_MINIMAP_SQUARE, frag);
    link(SHADER_PROGRAM_MINIMAP_SQUARE);
    detach(SHADER_PROGRAM_MINIMAP_SQUARE, vert);
    detach(SHADER_PROGRAM_MINIMAP_SQUARE, frag);
    delete(vert);
    delete(frag);
    shader_bind_uniform_block(SHADER_PROGRAM_MINIMAP_SQUARE, UBO_INDEX_MINIMAP, "Minimap");
}

void shader_program_compile(ShaderProgramEnum program)
{
    switch (program) {
        case SHADER_PROGRAM_SCREEN:
            compile_shader_program_screen();
            break;
        case SHADER_PROGRAM_TILE:
            compile_shader_program_tile();
            break;
        case SHADER_PROGRAM_WALL:
            compile_shader_program_wall();
            break;
        case SHADER_PROGRAM_GUI:
            compile_shader_program_gui();
            break;
        case SHADER_PROGRAM_ENTITY:
            compile_shader_program_entity();
            break;
        case SHADER_PROGRAM_PROJECTILE:
            compile_shader_program_projectile();
            break;
        case SHADER_PROGRAM_OBSTACLE:
            compile_shader_program_obstacle();
            break;
        case SHADER_PROGRAM_PARTICLE:
            compile_shader_program_particle();
            break;
        case SHADER_PROGRAM_PARJICLE:
            compile_shader_program_parjicle();
            break;
        case SHADER_PROGRAM_SHADOW:
            compile_shader_program_shadow();
            break;
        case SHADER_PROGRAM_MINIMAP_CIRCLE:
            compile_shader_program_minimap_circle();
            break;
        case SHADER_PROGRAM_MINIMAP_SQUARE:
            compile_shader_program_minimap_square();
            break;
        default:
            log_write(INFO, "Unrecognized program %x\n", program);
            break;
    }
}

void shader_init(void)
{
    shader_context.programs[0] = 0;
    for (i32 i = 1; i < NUM_SHADER_PROGRAMS; i++)
        shader_context.programs[i] = glCreateProgram();
    
    shader_program_compile(SHADER_PROGRAM_SCREEN);
    shader_program_compile(SHADER_PROGRAM_TILE);
    shader_program_compile(SHADER_PROGRAM_WALL);
    shader_program_compile(SHADER_PROGRAM_GUI);
    shader_program_compile(SHADER_PROGRAM_ENTITY);
    shader_program_compile(SHADER_PROGRAM_PROJECTILE);
    shader_program_compile(SHADER_PROGRAM_OBSTACLE);
    shader_program_compile(SHADER_PROGRAM_PARTICLE);
    shader_program_compile(SHADER_PROGRAM_PARJICLE);
    shader_program_compile(SHADER_PROGRAM_SHADOW);
    shader_program_compile(SHADER_PROGRAM_MINIMAP_CIRCLE);
    shader_program_compile(SHADER_PROGRAM_MINIMAP_SQUARE);
}

void shader_use(ShaderProgramEnum id)
{
    glUseProgram(shader_context.programs[id]);
}

void shader_cleanup(void)
{
    for (i32 i = 1; i < NUM_SHADER_PROGRAMS; i++)
        if (shader_context.programs[i] != 0)
            glDeleteProgram(shader_context.programs[i]);
}

GLuint shader_get_uniform_location(ShaderProgramEnum program, const char* identifier)
{
    return glGetUniformLocation(shader_context.programs[program], identifier);
}

void shader_bind_uniform_block(ShaderProgramEnum program, u32 index, const char* identifier)
{
    glUniformBlockBinding(shader_context.programs[program], 
                          glGetUniformBlockIndex(shader_context.programs[program], identifier), 
                          index);
}
