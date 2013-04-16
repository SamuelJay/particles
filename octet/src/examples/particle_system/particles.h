class particles{

public:
  particles();
  void init(int _texture, float x, float y, float w, float h);
  void move();
  void render (texture_shader &shader, mat4 &camera_to_world);
   
private:
  int max_distance_squared;
   enum {
    NUM_PARTICLES=4000
   };
  //where each particle is (might be a less expensive way to do this)
  mat4 model_to_projection[NUM_PARTICLES]; 
  mat4 model_to_world[NUM_PARTICLES];
  float half_width;
  float half_height;
  int texture;
  bool enabled[NUM_PARTICLES];
  float distance_squared;
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
    float rand_x;
    float rand_y;
    float rand_z;
    velocities[i]= vec4(rand_x = randomizer.get(-0.5f, 0.5f), rand_y = randomizer.get(-0.5f, 0.5f), rand_z= randomizer.get(-0.5f, 0.5f), 0);
    model_to_world[i].loadIdentity();
    model_to_world[i].translate(emitter.x(), emitter.y(), emitter.z());
    enabled[i] = true;
    
    /*printf("\nmodel to world starts:\n");
    printf("0X  : %d  0X  : %d  0Z  :  %d\n",
    model_to_world[i][0][0] , model_to_world[i][0][1], model_to_world[i][0][2], model_to_world[i][0][3]);
    printf("0X  : %d  0X  : %d  0Z  :  %d\n",
    model_to_world[i][1][0] ,model_to_world[i][1][1], model_to_world[i][1][2], model_to_world[i][1][3]);
    printf("0X  : %d  0X  : %d  0Z  :  %d\n",
    model_to_world[i][2][0] , model_to_world[i][2][1], model_to_world[i][2][2], model_to_world[i][2][3]);
    printf("0X  : %d  0X  : %d  0Z  :  %d\n",
    model_to_world[i][3][0] ,model_to_world[i][3][1], model_to_world[i][3][2], model_to_world[i][3][3]);*/
  }
  texture = _texture;
  //same size and texture for all particles
  half_width = w * 0.5f;
  half_height = h * 0.5f;  
}

void particles::move() {
  for (int i = 0; i < NUM_PARTICLES; i++) { 

    positions[i]+=velocities[i];
    model_to_world[i].translate(velocities[i].x(), velocities[i].y(), velocities[i].z());

    distance_squared = (positions[i].x() * positions[i].x()) + (positions[i].y() * positions[i].y()) + (positions[i].z() * positions[i].z());

    if(distance_squared>max_distance_squared) {
      model_to_world[i].loadIdentity();
      model_to_world[i].translate(emitter.x(), emitter.y(), emitter.z());
      positions[i] = emitter;
      
     /* printf("\nmodel to world out:\n");
      printf("0X  : %d  0X  : %d  0Z  :  %d\n",
      model_to_world[i][0][0] , model_to_world[i][0][1], model_to_world[i][0][2], model_to_world[i][0][3]);
      printf("0X  : %d  0X  : %d  0Z  :  %d\n",
      model_to_world[i][1][0] ,model_to_world[i][1][1], model_to_world[i][1][2], model_to_world[i][1][3]);
      printf("0X  : %d  0X  : %d  0Z  :  %d\n",
      model_to_world[i][2][0] , model_to_world[i][2][1], model_to_world[i][2][2], model_to_world[i][2][3]);
      printf("0X  : %d  0X  : %d  0Z  :  %d\n",
      model_to_world[i][3][0] ,model_to_world[i][3][1], model_to_world[i][3][2], model_to_world[i][3][3]);*/
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