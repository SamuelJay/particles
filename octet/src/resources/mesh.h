////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 3D mesh container
//

class collision {
  static bool unit_test_triange_box_collision() {
    vec4 centre(3, 3, 3, 0);
    float size = 2;
    
    vec4 tris[] = {
      // horizontal triangles at centre
      vec4(-1.0f, -1.0f, 3.0f, 1.0f),   vec4( 0.5f, -1.0f, 3.0f, 1.0f),   vec4( 0.5f, 0.5f, 3.0f, 1.0f),
      vec4(-1.0f, -1.0f, 3.0f, 1.0f),   vec4( 1.0f, -1.0f, 3.0f, 1.0f),   vec4( 1.0f, 1.0f, 3.0f, 1.0f),
      vec4(-1.0f, -1.0f, 3.0f, 1.0f),   vec4( 2.0f, -1.0f, 3.0f, 1.0f),   vec4( 2.0f, 2.0f, 3.0f, 1.0f),
    };

    for (int i = 0; i != sizeof(tris)/sizeof(vec4)/3; ++i) {
      vec4 pos[] = { tris[i*3+0], tris[i*3+1], tris[i*3+2] };
      bool hit = triangle_cube_collision(centre, size, pos);
    }
    return true;
  }

  static float fmin(float a, float b) { return a < b ? a : b; }
  static float fmax(float a, float b) { return a > b ? a : b; }

  static bool edge(const vec4 &p0, const vec4 &p1, const vec4 &p2, int y, int z, float size) {
    float dist_p0 = p0[z] * p1[y] - p0[y] * p1[z];
    float dist_p2 = ( p1[y] - p0[y] ) * p2[z] - ( p1[z] - p0[z] ) * p2[y];
    float dist_cube = fabsf( ( p1[y] - p0[y] ) + ( p1[z] - p0[z] ) ) * size;
    return fmin(dist_p0, dist_p2) > dist_cube || fmax(dist_p0, dist_p2) < -dist_cube;
  }

public:
  static bool triangle_cube_collision(const vec4 &centre, float size, vec4 *pos) {
    vec4 p0 = pos[0] - centre;
    vec4 p1 = pos[1] - centre;
    vec4 p2 = pos[2] - centre;

    // points on triangles vs faces on cube
    vec4 min = p0.min(p1.min(p2));
    vec4 max = p0.max(p1.max(p2));
    if (
      min.x() < size && max.x() > -size &&
      min.y() < size && max.y() > -size &&
      min.z() < size && max.z() > -size
    ) {
      return false;
    }

    // points on cube vs. face of triangle
    vec4 normal = (p1 - p0).cross(p2 - p0);
    float dist_face = fabsf(normal.dot(p0));
    float dist_cube = ( fabsf(normal.x()) + fabsf(normal.y()) + fabsf(normal.z()) ) * size;
    if (dist_face > dist_cube) {
      return false;
    }

    return (
      edge(p0, p1, p2, 0, 1, size) &&
      edge(p0, p1, p2, 1, 2, size) &&
      edge(p0, p1, p2, 2, 0, size) &&
      edge(p1, p2, p0, 0, 1, size) &&
      edge(p1, p2, p0, 1, 2, size) &&
      edge(p1, p2, p0, 2, 0, size) &&
      edge(p2, p0, p1, 0, 1, size) &&
      edge(p2, p0, p1, 1, 2, size) &&
      edge(p2, p0, p1, 2, 0, size)
    );
  }

  static bool unit_test() {
    return unit_test_triange_box_collision();
  }
};

class mesh : public mesh_state {

public:
  // default constructor makes a blank mesh.
  mesh() {
  }

  void make_cube(float size = 1.0f) {
    release();
    mesh_builder b;
    
    b.init(6*4, 6*6);

    b.add_cube(size);
    b.get_mesh_state(*this);
  }

  void make_aa_box(float x, float y, float z) {
    release();
    mesh_builder b;
    
    b.init(6*4, 6*6);
    b.scale(x, y, z);

    b.add_cube(1.0f);
    b.get_mesh_state(*this);
  }

  void make_plane(float size = 1.0f, unsigned nx=1, unsigned ny=1) {
    release();
    mesh_builder b;
    b.init(nx*ny*2, nx*ny*6);
    b.add_plane(size, nx, ny);
    b.get_mesh_state(*this);
  }

  void make_sphere(float radius=1.0f, unsigned slices=16, unsigned stacks=16) {
    release();
    mesh_builder b;
    
    b.init(stacks*(slices+1), stacks*slices*6);

    b.add_sphere(radius, slices, stacks, 8);
    b.get_mesh_state(*this);
  }

  void make_cone(float radius=1.0f, float height=1.0f, unsigned slices=32, unsigned stacks=1) {
    release();
    mesh_builder b;
    
    b.init(stacks*slices, stacks*slices*6+1);

    b.add_cone(radius, height, slices, stacks);
    b.get_mesh_state(*this);
  }

  void make_collada_mesh(collada_builder &builder, const char *id) {
    builder.get_mesh_state(*this, id);
  }

  void make_normal_visualizer(const mesh &source, float length, unsigned normal_attr) {
    release();
    mesh_builder builder;
    
    builder.init(source.get_num_vertices()*2, source.get_num_vertices()*2);

    unsigned pos_slot = source.get_slot(mesh_state::attribute_pos);
    unsigned normal_slot = source.get_slot(normal_attr);
    for (unsigned i = 0; i != source.get_num_vertices(); ++i) {
      vec4 pos = source.get_value(pos_slot, i);
      vec4 normal = source.get_value(normal_slot, i);
      unsigned idx = builder.add_vertex(pos, normal, 0, 0);
      builder.add_vertex(pos + normal * length, normal, 0, 0);
      builder.add_index(idx);
      builder.add_index(idx+1);
    }

    builder.get_mesh_state(*this);
    set_mode( GL_LINES );
  }

  // make the normal, tangent, bitangent space for each vertex
  void add_3d_normals(const mesh &source) {
    release();
    if (source.get_use_vbo() || source.get_mode() != GL_TRIANGLES || source.get_num_indices() % 3 != 0) {
      printf("warning: make_3d_normals expected triangles\n");
      return;
    }

    //s = source.s; 

    // add tangent and bitangent
    unsigned stride = source.get_stride();
    unsigned tangent_slot = add_attribute(mesh_state::attribute_tangent, 3, GL_FLOAT, stride);
    unsigned bitangent_slot = add_attribute(mesh_state::attribute_bitangent, 3, GL_FLOAT, stride + 12);

    unsigned vsize = (stride + 24) * source.get_num_vertices();
    unsigned isize = mesh_state::kind_size(get_index_type()) * source.get_num_indices();
    allocate(vsize, isize, false);
    set_params(stride + 24, source.get_num_indices(), source.get_num_vertices(), source.get_mode(), source.get_index_type());
    copy_indices(source);

    unsigned pos_slot = source.get_slot(mesh_state::attribute_pos);
    unsigned uv_slot = source.get_slot(mesh_state::attribute_uv);
    //unsigned normal_slot = source.get_slot(mesh_state::attribute_normal);

    for (unsigned slot = 0; slot != source.get_num_slots(); ++slot) {
      for (unsigned i = 0; i != source.get_num_vertices(); ++i) {
        vec4 value = source.get_value(slot, i);
        set_value(slot, i, value);
      }
    }

    for (unsigned i = 0; i != source.get_num_vertices(); ++i) {
      vec4 value(0, 0, 0, 0);
      set_value(tangent_slot, i, value);
      set_value(bitangent_slot, i, value);
    }

    for (unsigned i = 0; i != source.get_num_indices(); i += 3) {
      unsigned idx[3] = {
        source.get_index(i),
        source.get_index(i+1),
        source.get_index(i+2)
      };
      vec4 uv0 = source.get_value(uv_slot, idx[0]);
      vec4 uv1 = source.get_value(uv_slot, idx[1]);
      vec4 uv2 = source.get_value(uv_slot, idx[2]);
      vec4 pos0 = source.get_value(pos_slot, idx[0]);
      vec4 pos1 = source.get_value(pos_slot, idx[1]);
      vec4 pos2 = source.get_value(pos_slot, idx[2]);
      vec4 duv1 = uv1 - uv0;
      vec4 duv2 = uv2 - uv0;
      vec4 dpos1 = pos1 - pos0;
      vec4 dpos2 = pos2 - pos0;

      // solve:
      //   duv1.x() * tangent + duv1.y() * bitangent = dpos1
      //   duv2.x() * tangent + duv2.y() * bitangent = dpos2

      //float rdet = 1.0f / (duv1.x() * duv2.y() - duv1.y() * duv2.x());
      float rdet = 1.0f;
      vec4 tangent = (dpos1 * duv2.y() - dpos2 * duv1.y()) * rdet;
      vec4 bitangent = (dpos2 * duv1.x() - dpos1 * duv2.x()) * rdet;

      /*vec4 normal = source.get_value(normal_slot, idx[0]);
      printf("normal=%s\n", normal.toString());
      printf("tangent=%s\n", tangent.toString());
      printf("bitangent=%s\n", bitangent.toString());
      printf("duv1.x() * tangent + duv1.y() * bitangent=%s\n", (tangent * duv1.x() + bitangent * duv1.y()).toString());
      printf("dpos1=%s\n", dpos1.toString());
      printf("duv2.x() * tangent + duv2.y() * bitangent=%s\n", (tangent * duv2.x() + bitangent * duv2.y()).toString());
      printf("dpos2=%s\n", dpos2.toString());*/

      for (unsigned j = 0; j != 3; ++j) {
        vec4 new_tangent = tangent + get_value(tangent_slot, idx[j]);
        vec4 new_bitangent = bitangent + get_value(bitangent_slot, idx[j]);
        set_value(tangent_slot, idx[j], new_tangent);
        set_value(bitangent_slot, idx[j], new_bitangent);
      }
    }

    // normalize
    for (unsigned i = 0; i != source.get_num_vertices(); ++i) {
      vec4 new_tangent = get_value(tangent_slot, i).xyz().normalize();
      vec4 new_bitangent = get_value(bitangent_slot, i).xyz().normalize();
      set_value(tangent_slot, i, new_tangent);
      set_value(bitangent_slot, i, new_bitangent);
    }
  }

  void render() {
    unsigned char *base = 0;
    unsigned char *index_base = 0;
    begin_render(&base, &index_base);

    for (unsigned slot = 0; slot != get_num_slots(); ++slot) {
      unsigned size = get_size(slot);
      unsigned kind = get_kind(slot);
      unsigned attr = get_attr(slot);
      unsigned offset = get_offset(slot);
      glVertexAttribPointer(attr, size, kind, GL_FALSE, get_stride(), (void*)(base + offset));
      glEnableVertexAttribArray(attr);
    }

    glDrawElements(get_mode(), get_num_indices(), get_index_type(), (GLvoid*)index_base);

    for (unsigned slot = 0; slot != get_num_slots(); ++slot) {
      unsigned attr = get_attr(slot);
      glDisableVertexAttribArray(attr);
    }
  }

  void transform(unsigned attr, mat4 &matrix) {
    if (get_use_vbo() || get_num_vertices() == 0) {
      return;
    }

    unsigned slot = get_slot(attr);
    for (unsigned i = 0; i != get_num_vertices(); ++i) {
      vec4 pos = get_value(slot, i);
      vec4 tpos = pos * matrix;
      set_value(slot, i, tpos);
    }
  }

  void copy(const mesh &source) {
    copy_format(source);
    copy_params(source);

    if (get_use_vbo() || get_num_vertices() == 0) {
      return;
    }

    size_t vsize = source.get_vertices_size();
    size_t isize = source.get_indices_size();
    allocate(vsize, isize, false);
    copy_indices(source);
    copy_vertices(source);
  }

  void dump(FILE *file) {
    dump(file);
  }

  bool ray_cast(const vec4 &org, const vec4 &dir, int indices[], vec4 &bary) {
    //static FILE *file = fopen("c:/tmp/3.txt","w");
    //fprintf(file, "org=%s dir=%s\n", org.toString(), dir.toString());
    unsigned pos_slot = get_slot(mesh_state::attribute_pos);
    for (unsigned i = 0; i != get_num_indices(); i += 3) {
      unsigned idx[3] = {
        get_index(i),
        get_index(i+1),
        get_index(i+2)
      };

      vec4 a = get_value(pos_slot, idx[0]) - org;
      vec4 b = get_value(pos_slot, idx[1]) - org;
      vec4 c = get_value(pos_slot, idx[2]) - org;
      vec4 d = dir;

      //fprintf(file, "a=%s b=%s c=%s d=%s\n", a.toString(), b.toString(), c.toString(), d.toString());

      // solve [ba, bb, bc, bd] * [[ax, ay, az, 1], [bx, by, bz, 1], [cx, cy, cz, 1], [dx, dy, dz, 0]] = [0, 0, 0, 1]
      float det = (
        -a.x()*b.y()*d.z()+a.x()*b.z()*d.y()
        +a.x()*c.y()*d.z()-a.x()*c.z()*d.y()
        +a.y()*b.x()*d.z()-a.y()*b.z()*d.x()
        -a.y()*c.x()*d.z()+a.y()*c.z()*d.x()
        -a.z()*b.x()*d.y()+a.z()*b.y()*d.x()
        +a.z()*c.x()*d.y()-a.z()*c.y()*d.x()
        -b.x()*c.y()*d.z()+b.x()*c.z()*d.y()
        +b.y()*c.x()*d.z()-b.y()*c.z()*d.x()
        -b.z()*c.x()*d.y()+b.z()*c.y()*d.x()
      );

      vec4 bary1 = vec4(
        -b.x()*c.y()*d.z()+b.x()*c.z()*d.y()+b.y()*c.x()*d.z()-b.y()*c.z()*d.x()-b.z()*c.x()*d.y()+b.z()*c.y()*d.x(),
        a.x()*c.y()*d.z()-a.x()*c.z()*d.y()-a.y()*c.x()*d.z()+a.y()*c.z()*d.x()+a.z()*c.x()*d.y()-a.z()*c.y()*d.x(),
        -a.x()*b.y()*d.z()+a.x()*b.z()*d.y()+a.y()*b.x()*d.z()-a.y()*b.z()*d.x()-a.z()*b.x()*d.y()+a.z()*b.y()*d.x(),
        a.x()*b.y()*c.z()-a.x()*b.z()*c.y()-a.y()*b.x()*c.z()+a.y()*b.z()*c.x()+a.z()*b.x()*c.y()-a.z()*b.y()*c.x()
      );

      vec4 bary2 = bary1 * det;

      if (bary2.x() >= 0 && bary2.y() >= 0 && bary2.z() >= 0 && bary2.w() >= 0) {
        //fprintf(file, "!! %d %d %d  %s\n", idx[0], idx[1], idx[2], (bary1/det).toString());
        indices[0] = idx[0];
        indices[1] = idx[1];
        indices[2] = idx[2];
        bary = bary1 / det;
        return true;
      }
    }
    //fclose(file);
    //exit(1);
    return false;
  }

  void get_aabb(const mat4 &modelToWorld, vec4 &min, vec4 &max) {
    unsigned pos_slot = get_slot(mesh_state::attribute_pos);

    {
      vec4 v = get_value(pos_slot, 0) * modelToWorld;
      min = v;
      max = v;
    }
  
    for (unsigned i = 1; i < get_num_vertices(); ++i) {
      vec4 v = get_value(pos_slot, i) * modelToWorld;
      min = min.min(v);
      max = max.max(v);
    }
  }

  template <class T > void get_triangles(const mat4 &modelToWorld, int max_depth, T &out) {
    unsigned pos_slot = get_slot(mesh_state::attribute_pos);

    for (unsigned i = 0; i != get_num_indices(); i += 3) {
      unsigned idx[3] = {
        get_index(i),
        get_index(i+1),
        get_index(i+2)
      };

      vec4 p0 = get_value(pos_slot, idx[0]) * modelToWorld;
      vec4 p1 = get_value(pos_slot, idx[1]) * modelToWorld;
      vec4 p2 = get_value(pos_slot, idx[2]) * modelToWorld;
      out.triangle(i, p0, p1, p2);
    }
  }

};

