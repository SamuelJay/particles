
class particle_app : public app {

public:    
  particle_app(int argc, char **argv) : app(argc, argv) {}
  void app_init();
  void simulate();
  void draw_world(int, int, int, int); 

private:
  mat4 cameraToWorld;
  texture_shader texture_shader_;
  particles particles;
};

void particle_app::app_init() {
  texture_shader_.init();
  // set up the matrices with a camera 5 units from the origin
  cameraToWorld.loadIdentity();
  cameraToWorld.translate(0, 0, 3);
  //GLuint sprite_texture = resource_manager::get_texture_handle(GL_RGBA, "assets/particle/stock_gradient_yellow_red.png");
  GLuint sprite_texture = resource_manager::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
  //sprites[0].init(sprite_texture,  0, 0, 0.25f, 0.25f);
  particles.init(sprite_texture,0, 0, 0.25f, 0.25f);
}

void particle_app::simulate() {

}

void particle_app::draw_world(int x, int y, int w, int h) {
  particles.move();
  // set a viewport - includes whole window area
  glViewport(x, y, w, h);
  // clear the background to black
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
  glDisable(GL_DEPTH_TEST);
  // allow alpha blend (transparency when alpha channel is 0)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // draw all the sprites
  particles.render(texture_shader_, cameraToWorld);
  //particles.render(texture_shader_, cameraToWorld);
  // move the listener with the camera
  vec4 &cpos = cameraToWorld.w();
  alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
}