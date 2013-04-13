class particles{

public:
  particles();
  void init(int _texture, float x, float y, float w, float h);
  void move();
  void render (texture_shader &shader, mat4 &camera_to_world);
   
private:
  int max_distance_squared;
  #define NUM_PARTICLES 3
  //where each particle is (might be a less expensive way to do this)
  mat4 model_to_projection[NUM_PARTICLES]; 
  mat4 model_to_world[NUM_PARTICLES];
  float half_width;
  float half_height;
  int texture;
  bool enabled[NUM_PARTICLES];
  float distance;
  vec4 emitter;
  vec4 distance_vector;
  // the position is used to calculate the distance between the particles and the emitter
  vec4 positions[NUM_PARTICLES];
  vec4 velocities[NUM_PARTICLES];
  // random number generator
  class random randomizer;
};

particles::particles() {
  
}

void particles::init(int _texture, float x, float y, float w, float h) {
  max_distance_squared=16;
  //give the particles there starting positions
  emitter=vec4(0, 0, 0, 0);
  for (int i = 0; i < NUM_PARTICLES; i++) {
    //set starting position (will become an emitter)
    positions[i]= emitter;
    //set starting velocity
    float j;
    velocities[i]= vec4(0, j = randomizer.get(0.0f, 0.5f), 0, 0);
    model_to_world[i].loadIdentity();
    model_to_world[i].translate(emitter.x(), emitter.y(), 0);
    enabled[i] = true;
  }
  texture = _texture;
  //same size and texture for all particles
  half_width = w * 0.5f;
  half_height = h * 0.5f;  
}

void particles::move() {
  for (int i = 0; i < NUM_PARTICLES; i++) { 

    positions[i]+=velocities[i];
    model_to_world[i].translate(velocities[i].x(), velocities[i].y(), 0);

    distance = (positions[i].x() * positions[i].x()) + (positions[i].y() * positions[i].y());

    if(distance>max_distance_squared) {
      model_to_world[i].loadIdentity();
      model_to_world[i].translate(emitter.x(), emitter.y(), 0);
      positions[i] = emitter;
    }

  }
}



void particles::render(texture_shader &shader, mat4 &camera_to_world) {
  
  
  //all the vertices uvs, and texture stuff are just done once they are just dranwe many times at all the different positions 
  //an array of the positions of the corners of the particle in 3D
  float vertices[] = {
    -half_width, -half_height, 0,
    half_width, -half_height, 0,
    half_width,  half_height, 0,
    -half_width,  half_height, 0,
  };
  
  static const float uvs[] = {
    0,  0,
    1,  0,
    1,  1,
    0,  1,
  };
  glVertexAttribPointer(mesh_state::attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
  glEnableVertexAttribArray(mesh_state::attribute_pos);
  glVertexAttribPointer(mesh_state::attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)uvs );
  glEnableVertexAttribArray(mesh_state::attribute_uv);
  //set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  for (int i=0; i<NUM_PARTICLES; i++) {
    //build a projection matrix for each particle again mabye not the best way to do this: model -> world -> camera -> projection
    //the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    model_to_projection[i] = mat4::build_projection_matrix(model_to_world[i], camera_to_world);
    shader.render(model_to_projection[i], 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
  
}