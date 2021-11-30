//
// Implementation for Yocto/Model
//

//
// LICENSE:
//
// Copyright (c) 2016 -- 2021 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "yocto_model.h"

#include <yocto/yocto_sampling.h>

#include <iostream>

#include "ext/perlin-noise/noise1234.h"
#define TRY 3
// -----------------------------------------------------------------------------
// USING DIRECTIVES
// -----------------------------------------------------------------------------
namespace yocto {

// using directives
using std::array;
using std::string;
using std::vector;
using namespace std::string_literals;

}  // namespace yocto

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR EXAMPLE OF PROCEDURAL MODELING
// -----------------------------------------------------------------------------
namespace yocto {

float noise(const vec3f& p) { return ::noise3(p.x, p.y, p.z); }
vec2f noise2(const vec3f& p) {
  return {noise(p + vec3f{0, 0, 0}), noise(p + vec3f{3, 7, 11})};
}
vec3f noise3(const vec3f& p) {
  return {noise(p + vec3f{0, 0, 0}), noise(p + vec3f{3, 7, 11}),
      noise(p + vec3f{13, 17, 19})};
}
float fbm(const vec3f& p, int octaves) {
  auto sum    = 0.0f;
  auto weight = 1.0f;
  auto scale  = 1.0f;
  for (auto octave = 0; octave < octaves; octave++) {
    sum += weight * fabs(noise(p * scale));
    weight /= 2;
    scale *= 2;
  }
  return sum;
}
float turbulence(const vec3f& p, int octaves) {
  auto sum    = 0.0f;
  auto weight = 1.0f;
  auto scale  = 1.0f;
  for (auto octave = 0; octave < octaves; octave++) {
    sum += weight * fabs(noise(p * scale));
    weight /= 2;
    scale *= 2;
  }
  return sum;
}
float ridge(const vec3f& p, int octaves) {
  auto sum    = 0.0f;
  auto weight = 0.5f;
  auto scale  = 1.0f;
  for (auto octave = 0; octave < octaves; octave++) {
    sum += weight * (1 - fabs(noise(p * scale))) * (1 - fabs(noise(p * scale)));
    weight /= 2;
    scale *= 2;
  }
  return sum;
}
float marmo(const vec3f& p, int octaves) { return 1 + sin() }

void add_polyline(shape_data& shape, const vector<vec3f>& positions,
    const vector<vec4f>& colors, float thickness = 0.0001f) {
  auto offset = (int)shape.positions.size();
  shape.positions.insert(
      shape.positions.end(), positions.begin(), positions.end());
  shape.colors.insert(shape.colors.end(), colors.begin(), colors.end());
  shape.radius.insert(shape.radius.end(), positions.size(), thickness);
  for (auto idx = 0; idx < positions.size() - 1; idx++) {
    shape.lines.push_back({offset + idx, offset + idx + 1});
  }
}

// vector<vec3i> make_density(vector<int> d_map, vector<vec3i> triangoli) {
//   vector<vec3i> out;
//   for (int i : range(triangoli.size())) {
//     // std::cout << d_map[i] << " ";
//     int x = d_map[i];
//     int y = 0;
//     while (y < x) {
//       out.push_back(triangoli[i]);
//       y++;
//       // std::cout << y << " ";
//     }
//   }
//   return out;
// }

void sample_shape(vector<vec3f>& positions, vector<vec3f>& normals,
    vector<vec2f>& texcoords, const shape_data& shape, int num,
    vector<float> d_map) {
  auto triangles  = shape.triangles;
  auto qtriangles = quads_to_triangles(shape.quads);
  triangles.insert(triangles.end(), qtriangles.begin(), qtriangles.end());
  auto cdf = sample_triangles_cdf(triangles, shape.positions);
  auto rng = make_rng(19873991);
  for (auto idx = 0; idx < num; idx++) {
    int ctrl = 0;

    while (ctrl < TRY) {
      auto [elem, uv] = sample_triangles(cdf, rand1f(rng), rand2f(rng));
      float r         = rand1f(rng);
      if (r <= d_map[elem]) {
        auto q = triangles[elem];
        positions.push_back(interpolate_triangle(shape.positions[q.x],
            shape.positions[q.y], shape.positions[q.z], uv));

        normals.push_back(normalize(interpolate_triangle(
            shape.normals[q.x], shape.normals[q.y], shape.normals[q.z], uv)));
        if (!texcoords.empty()) {
          texcoords.push_back(interpolate_triangle(shape.texcoords[q.x],
              shape.texcoords[q.y], shape.texcoords[q.z], uv));
        } else {
          texcoords.push_back(uv);
        }
        ctrl = TRY;
      }
      // std::cout << ctrl << " ";
      ctrl++;
    }
  }
}
pair<int, vec2f> mio_sample_triangles(vector<float> cdf, float r1, vec2f r2) {
  if (r1 <= cdf[0]) {
    // std::cout << cdf[0] << "ehy 0 ";
    return {0, sample_triangle(r2)};
  }
  float v = cdf[0];
  for (int l : range(1, (int)cdf.size())) {
    v += cdf[l];
    if (r1 <= v) {
      return {l, sample_triangle(r2)};
      // std::cout << l << " it is  it ";
    }
  }
  return {-1, {-1, -1}};
}
void sample_shape_d1(vector<vec3f>& positions, vector<vec3f>& normals,
    vector<vec2f>& texcoords, const shape_data& shape, int num,
    vector<float> d_map) {
  auto triangles  = shape.triangles;
  auto qtriangles = quads_to_triangles(shape.quads);
  triangles.insert(triangles.end(), qtriangles.begin(), qtriangles.end());
  auto          cdf1 = sample_triangles_cdf(triangles, shape.positions);
  vector<float> cdf2;
  float         sum = 0.0;
  for (int y : range(triangles.size())) {
    if (y < (shape.triangles.size() / 4)) {
      d_map.push_back(0.0);
    } else if (y < (shape.triangles.size()) * 2 / 4) {
      d_map.push_back(0.2);
    } else if (y < (shape.triangles.size()) * 3 / 4) {
      d_map.push_back(0.5);
    } else {
      d_map.push_back(1.0);
    }
  }
  for (auto b : range(cdf1.size())) {
    cdf2.push_back(cdf1[b] * d_map[b]);
    sum += cdf1[b] * d_map[b];
  }

  auto rng = make_rng(19873991);
  for (auto idx = 0; idx < num; idx++) {
    auto [elem, uv] = mio_sample_triangles(
        cdf2, rand1f(rng) * sum, rand2f(rng));
    auto q = triangles[elem];
    positions.push_back(interpolate_triangle(
        shape.positions[q.x], shape.positions[q.y], shape.positions[q.z], uv));
    normals.push_back(normalize(interpolate_triangle(
        shape.normals[q.x], shape.normals[q.y], shape.normals[q.z], uv)));
    if (!texcoords.empty()) {
      texcoords.push_back(interpolate_triangle(shape.texcoords[q.x],
          shape.texcoords[q.y], shape.texcoords[q.z], uv));
    } else {
      texcoords.push_back(uv);
    }
  }
}

void sample_shape(vector<vec3f>& positions, vector<vec3f>& normals,
    vector<vec2f>& texcoords, const shape_data& shape, int num) {
  auto triangles  = shape.triangles;
  auto qtriangles = quads_to_triangles(shape.quads);
  triangles.insert(triangles.end(), qtriangles.begin(), qtriangles.end());
  auto cdf = sample_triangles_cdf(triangles, shape.positions);
  auto rng = make_rng(19873991);
  for (auto idx = 0; idx < num; idx++) {
    auto [elem, uv] = sample_triangles(cdf, rand1f(rng), rand2f(rng));
    auto q          = triangles[elem];
    positions.push_back(interpolate_triangle(
        shape.positions[q.x], shape.positions[q.y], shape.positions[q.z], uv));
    normals.push_back(normalize(interpolate_triangle(
        shape.normals[q.x], shape.normals[q.y], shape.normals[q.z], uv)));
    if (!texcoords.empty()) {
      texcoords.push_back(interpolate_triangle(shape.texcoords[q.x],
          shape.texcoords[q.y], shape.texcoords[q.z], uv));
    } else {
      texcoords.push_back(uv);
    }
  }
}
bool isinquad(vec3f posizione, bbox2f quad) {
  // std::cout << posizione.x << " p " << quad.min.x << " min x " << quad.max.x
  //           << " max x " << posizione.z << " py " << quad.min.y << " min y "
  //           << quad.max.y << " max y ";
  return quad.min.x < posizione.x && posizione.x < quad.max.x &&
         quad.min.y < posizione.z && posizione.z < quad.max.y;
}
bool genera_punti(int k, vec3f posizione, float r, bbox2f quad,
    hash_grid& griglia, vector<int>& active_points, vector<vec3f>& posizioni) {
  vector<vec3f> out;
  rng_state     rng      = make_rng(1836);
  int           generati = 0;
  for (int i : range(k)) {
    vector<int> vicini;
    // genera un punto nell'anello intorno al punto in una distanza compresa tra
    // r e 2r
    int   sign1       = rand1f(rng) < 0.5 ? -1 : 1;
    int   sign2       = rand1f(rng) < 0.5 ? -1 : 1;
    vec3f spostamento = {
        sign1 * (r + rand1f(rng) * r), 0, sign2 * (r + rand1f(rng) * r)};
    vec3f p = posizione + spostamento;
    find_neighbors(griglia, vicini, p, r);

    if (isinquad(p, quad) && vicini.size() == 0) {
      // for (int i : range(100)) {
      //   std::cout << " genera ";
      // }
      posizioni.push_back(p);
      active_points.push_back(posizioni.size() - 1);
      insert_vertex(griglia, p);
      generati++;
    }
  }
  return generati == 0;
}
bbox2f dim_quad(vector<vec3f> positions) {  // funziona per piano xz
  bbox2f box;
  for (int i : range((int)positions.size())) {
    box.max.x = max(box.max.x, positions[i].x);
    box.max.y = max(box.max.y, positions[i].z);
    box.min.x = min(box.min.x, positions[i].x);
    box.min.y = min(box.min.y, positions[i].z);
  }
  return box;
}

void poisson(vector<vec3f>& positions, vector<vec3f>& normals,
    const shape_data& shape, float r) {
  auto box = dim_quad(shape.positions);
  // for (int i : range(100)) {
  //   std::cout << box.max.x << " Mx " << box.max.y << " My " << box.min.x
  //             << " mx " << box.min.y << " my ";
  // }
  // crea griglia
  hash_grid griglia = make_hash_grid(r / sqrt(2.0f));
  // crea active points
  vector<int> active_points;
  // inizializza modo di scelta casuale punto
  auto triangles  = shape.triangles;
  auto qtriangles = quads_to_triangles(shape.quads);
  triangles.insert(triangles.end(), qtriangles.begin(), qtriangles.end());
  auto cdf = sample_triangles_cdf(triangles, shape.positions);
  auto rng = make_rng(19873991);
  // scelta casuale di primo punto
  auto [elem1, uv1] = sample_triangles(cdf, rand1f(rng), rand2f(rng));
  auto q1           = triangles[elem1];
  auto p1           = interpolate_triangle(
      shape.positions[q1.x], shape.positions[q1.y], shape.positions[q1.z], uv1);
  positions.push_back(p1);
  normals.push_back(normalize(interpolate_triangle(
      shape.normals[q1.x], shape.normals[q1.y], shape.normals[q1.z], uv1)));
  // metti punto in griglia
  insert_vertex(griglia, p1);
  // metti punto in active_points
  active_points.push_back(0);
  // for (int i : range(100)) {
  //   std::cout << active_points.size() << "eseguo ";
  // }
  // loop fintanto chle la lista punti attivi non è vuota inserisci punti
  while (active_points.size() != 0) {
    // scegli indice dalla lista
    int rnd = rand1i(rng, active_points.size());
    // genera k=30 punti nell'intorno scelto
    auto elimina = genera_punti(30, positions[active_points[rnd]], r, box,
        griglia, active_points, positions);
    if (elimina) {
      active_points[rnd] = active_points[active_points.size() - 1];
      active_points.pop_back();
    }
    // std::cout << "eseguo ";
  }
  // for (auto idx = 0; idx < num; idx++) {
  //   auto [elem, uv] = ;
  //   auto q          = triangles[elem];
  //   positions.push_back(interpolate_triangle(
  //       shape.positions[q.x], shape.positions[q.y], shape.positions[q.z],
  //       uv));
  //   normals.push_back(normalize(interpolate_triangle(
  //       shape.normals[q.x], shape.normals[q.y], shape.normals[q.z], uv)));
  //   if (!texcoords.empty()) {
  //     texcoords.push_back(interpolate_triangle(shape.texcoords[q.x],
  //         shape.texcoords[q.y], shape.texcoords[q.z], uv));
  //   } else {
  //     texcoords.push_back(uv);
  //   }
  // }
}

void make_hair(
    shape_data& hair, const shape_data& shape, const hair_params& params) {
  vector<vec3f> posi;
  vector<vec4f> colors;
  vector<vec3f> normal;
  vector<vec2f> texc;
  sample_shape(posi, normal, texc, shape, 616);
  // poisson(posi, normal, shape, 0.005);
  // for (int i : range(50)) {
  //   std::cout << posi.size() << " ";
  // }
  int  size = posi.size();
  auto n1   = normal[0];
  normal.pop_back();
  // for (int i : range(100)) {
  //   std::cout << posi.size() << "  ";
  // }
  for (int i : range(posi.size())) {
    // std::cout << posi[i].x;
    colors.push_back({0.9, 0.9, 0.9, 0});
    normal.push_back(n1);
  }
  // entra in funzione

  float n = params.lenght / params.steps;
  for (int i = 0; i < posi.size(); i++) {
    // for (int i : range(100)) {
    //   std::cout << " 1for ";
    // }
    vector<vec3f> positions;
    vector<vec4f> colors;

    auto& pos = posi[i];
    positions.push_back(pos);

    auto& norm = normal[i];
    // for (int i : range(100)) {
    //   std::cout << params.steps << "  ";
    // }
    for (int j : range(params.steps)) {
      // for (int i : range(100)) {
      //   std::cout << " for ";
      // }
      vec3f next_pos = positions[j] + norm * n +
                       noise3(positions[j] * params.scale) * params.strength;

      positions.push_back(next_pos);
    }

    // for (int i : range(100)) {
    //   std::cout << " adding";
    // }

    add_polyline(hair, positions, colors);
  }
  auto tang = compute_normals(hair);

  for (int b : range(tang.size())) {
    hair.tangents.push_back(rgb_to_rgba(tang[b]));
  }
}
// // inizializza la griglia vuota
// vector<int> build_grid(float r, int n) {//
// rifaree---------------------------------
//   vector<int> out;
//   float       cell_size = r / sqrt(float(n));
//   int         vec_size  = ceil(1 / cell_size);
//   vec_size *= vec_size;
//   for (int i : range(vec_size)) {
//     out.push_back(-1);
//   }
//   return out;
// }

// // genera k punti
// vector<vec2f> genera_punti(float r, vec2f punto, int k) {
// //rifareeeee-----------------------
//   vector<vec2f> out;
//   rng_state     rng = make_rng(1836);
//   for (int i : range(k)) {
//     // genera un punto nell'anello intorno al punto in una distanza compresa
//     tra
//     // r e 2r
//     vec2f spostamento = {(r + rand1f(rng) * r), (r + rand1f(rng) * r)};
//     vec2f p           = punto + spostamento;
//     out.push_back(p);
//   }
//   return out;
// }
// // restituisce la cella in cui inserire il punto
// int cell_of(vec2f punto, float r) {}
// // controlla se il punto può essere messo nella griglia
// bool isnear(vec2f texcoord, float r, vector<int> v, vector<vec3f> positions)
// { //rifareeee--------------------
//   int n     = ceil(1 / (r / sqrt(2.0f)));
//   int cellx = floor(texcoord.x / (r / sqrt(2.0f)));
//   int celly = floor(texcoord.y / (r / sqrt(2.0f)));

//   return (v[cellx * n + celly + 1] == -1 ||
//              distance(positions[v[cellx * n + celly]],
//                  positions[v[cellx * n + celly + 1]]) <= r) &&
//          (v[cellx * n + celly - 1] == -1 ||
//              distance(positions[v[cellx * n + celly]],
//                  positions[v[cellx * n + celly - 1]]) <= r) &&
//          (v[(cellx + 1) * n + celly] == -1 ||
//              distance(positions[v[cellx * n + celly]],
//                  positions[v[(cellx + 1) * n + celly]]) <= r) &&
//          (v[(cellx - 1) * n + celly] == -1 ||
//              distance(positions[v[cellx * n + celly]],
//                  positions[v[(cellx - 1) * n + celly]]) <= r);
// }
// void func(){
// eval_texcoord(4)
// }

// void sample_shape_d(vector<vec3f>& positions, vector<vec3f>& normals,
//     vector<vec2f>& texcoords, const shape_data& shape, int num,
//     vector<int> d_map) {
//   auto triangles  = shape.triangles;
//   auto qtriangles = quads_to_triangles(shape.quads);
//   triangles.insert(triangles.end(), qtriangles.begin(), qtriangles.end());
//   auto cdf = sample_triangles_cdf(triangles, shape.positions);
//   auto rng = make_rng(19873991);
//   for (int y : range(triangles.size())) {
//     if (y < (shape.triangles.size() + shape.quads.size()) / 4) {
//       d_map.push_back(0);
//     } else if (y < (shape.triangles.size() + shape.quads.size()) * 2 / 4) {
//       d_map.push_back(2);
//     } else if (y < (shape.triangles.size() + shape.quads.size()) * 3 / 4) {
//       d_map.push_back(5);
//     } else {
//       d_map.push_back(10);
//     }
//   }

//   triangles = make_density(d_map, triangles);
//   for (auto idx = 0; idx < num; idx++) {
//     int  elem = rand1i(rng, triangles.size());
//     auto uv   = sample_triangle(rand2f(rng));
//     auto q    = triangles[elem];
//     positions.push_back(interpolate_triangle(
//         shape.positions[q.x], shape.positions[q.y], shape.positions[q.z],
//         uv));
//     normals.push_back(normalize(interpolate_triangle(
//         shape.normals[q.x], shape.normals[q.y], shape.normals[q.z], uv)));
//     if (!texcoords.empty()) {
//       texcoords.push_back(interpolate_triangle(shape.texcoords[q.x],
//           shape.texcoords[q.y], shape.texcoords[q.z], uv));
//     } else {
//       texcoords.push_back(uv);
//     }
//   }
// }

void make_terrain(shape_data& shape, const terrain_params& params) {
  // YOUR CODE GOES HERE

  // std::cout << "       "<< max_altezza << "ao frate questa è l'altezza zi";
  // for (auto a : range(shape.texcoords.size())) {
  //   std::cout << shape.texcoords[a].x << " " << shape.texcoords[a].y << ". ";
  // }
  for (auto v : range(shape.positions.size())) {
    auto& pos = shape.positions[v];

    // std::cout << pos.x << " " << pos.y << " " << pos.z << " f ";
    // std::cout << "       "<< max_altezza << "ao frate questa è l'altezza zi";
    auto altezza = ridge(pos * params.scale, params.octaves) * params.height *
                   (1 - length(pos - params.center) / params.size);
    pos += shape.normals[v] * altezza;
    if (pos.y <= (30.0f / 100.0f) * params.height) {
      // float perc = pos.y/params.height;
      // if(perc<=0.33f){
      // std::cout << "bot ";
      shape.colors.push_back(params.bottom);
    } else if (pos.y <= 60.0f / 100.0f * params.height) {
      // std::cout << "mid ";
      shape.colors.push_back(params.middle);
    } else {
      // std::cout << "top ";
      shape.colors.push_back(params.top);
    }

    // std::cout << m << "ok ";
  }
  shape.normals = compute_normals(shape);
  // std::cout << m << "ok ";
}

void make_displacement(shape_data& shape, const displacement_params& params) {
  // YOUR CODE GOES HERE

  for (auto v : range(shape.positions.size())) {
    auto& pos      = shape.positions[v];
    vec3f altezza1 = pos / params.height;
    pos += shape.normals[v] * params.height *
           turbulence(pos * params.scale, params.octaves);
    vec3f altezza2 = pos / params.height;

    shape.colors.push_back(interpolate_line(
        params.bottom, params.top, distance(altezza1, altezza2)));
  }
  shape.normals = compute_normals(shape);
}

void make_hair1(
    shape_data& hair, const shape_data& shape, const hair_params& params) {
  // YOUR CODE GOES HERE
  // vector<float> d_map;
  // for (int y : range(shape.triangles.size() + shape.quads.size())) {
  //   if (y < (shape.triangles.size() + shape.quads.size()) / 4) {
  //     d_map.push_back(0.0);
  //   } else if (y < (shape.triangles.size() + shape.quads.size()) * 2 / 4) {
  //     d_map.push_back(0.0);
  //   } else if (y < (shape.triangles.size() + shape.quads.size()) * 3 / 4) {
  //     d_map.push_back(0.1);
  //   } else {
  //     d_map.push_back(1.0);
  //   }
  // }
  vector<float> d_map;

  vector<vec3f> p    = shape.positions;
  vector<vec3f> no   = shape.normals;
  vector<vec2f> t    = shape.texcoords;
  int           size = p.size();
  sample_shape_d1(p, no, t, shape, params.num, d_map);

  float n = params.lenght / params.steps;
  for (int i = size; i < p.size(); i++) {
    vector<vec3f> positions;
    vector<vec4f> colors;

    auto& pos = p[i];
    positions.push_back(pos);
    colors.push_back(params.bottom);
    auto& norm = no[i];
    for (int j : range(params.steps)) {
      vec3f next_pos = positions[j] + norm * n +
                       noise3(positions[j] * params.scale) * params.strength;
      next_pos.y -= params.gravity;
      norm = normalize(next_pos - positions[j]);

      colors.push_back(interpolate_line(
          params.bottom, params.top, distance(next_pos, pos) / params.lenght));
      positions.push_back(next_pos);
    }
    colors[params.steps] = params.top;
    add_polyline(hair, positions, colors);
  }
  auto tang = compute_normals(hair);

  for (int b : range(tang.size())) {
    hair.tangents.push_back(rgb_to_rgba(tang[b]));
  }
}

void make_grass(scene_data& scene, const instance_data& object,
    const vector<instance_data>& grasses, const grass_params& params) {
  int         forma_n = object.shape;
  shape_data& forma   = scene.shapes[forma_n];

  vector<vec3f> p;
  vector<vec3f> no;
  vector<vec2f> t;
  sample_shape(p, no, t, forma, params.num);
  // YOUR CODE GOES HERE
  auto rng = make_rng(1987);
  for (auto i : range(p.size())) {
    // nuova istanza
    instance_data n_inst;
    // istanza erba
    auto  r    = rand1i(rng, grasses.size());
    auto& inst = grasses[r];
    // random
    float rd  = 0.9 + rand1f(rng) / 10;
    float rtz = 0.1 + rand1f(rng) / 10;
    float rty = rand1f(rng) * 2 * M_PI;

    n_inst.shape    = inst.shape;
    n_inst.material = inst.material;
    n_inst.frame.y  = no[i];
    n_inst.frame.x  = orthonormalize({1.0f, 0.0f, 0.0f}, no[i]);
    n_inst.frame.z  = cross(n_inst.frame.x, n_inst.frame.y);
    n_inst.frame.o  = p[i];
    auto scala      = scaling_frame({rd, rd, rd});
    auto ruotaz     = rotation_frame(n_inst.frame.z, rtz);
    auto ruotay     = rotation_frame(n_inst.frame.y, rty);
    n_inst.frame *= scala * ruotay * ruotaz;

    scene.instances.push_back(n_inst);
  }
}

}  // namespace yocto
