////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Common shader code - compiles and links shaders

class shader {
  GLuint program_;
public:
  shader() {}

  GLuint program() { return program_; }
  
  void init(const char *vs, const char *fs) {
    printf("creating shader program\n");

    GLsizei length;
    char buf[256];
    // create our vertex shader and compile it
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    glGetShaderInfoLog(vertex_shader, sizeof(buf), &length, buf);
    puts(buf);
    
    // create our fragment shader and compile it
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    glGetShaderInfoLog(fragment_shader, sizeof(buf), &length, buf);
    puts(buf);

    // assemble the program for use by glUseProgram
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    // standardize the attribute slots    
    glBindAttribLocation(program, mesh_state::attribute_pos, "pos");
    glBindAttribLocation(program, mesh_state::attribute_normal, "normal");
    glBindAttribLocation(program, mesh_state::attribute_tangent, "tangent");
    glBindAttribLocation(program, mesh_state::attribute_bitangent, "bitangent");
    glBindAttribLocation(program, mesh_state::attribute_color, "color");
    glBindAttribLocation(program, mesh_state::attribute_uv, "uv");
    glLinkProgram(program);
    program_ = program;
    glGetProgramInfoLog(program, sizeof(buf), &length, buf);
    puts(buf);
  }
  
  // use the program we have compiled in init()
  void render() {
    glUseProgram(program_);
  }
};

