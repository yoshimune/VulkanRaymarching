#version 450

layout(location=0) in vec4 inColor;
layout(location=0) out vec4 outColor;

layout(binding = 0) uniform BasicInfo
{
  vec4 resolution;
  vec4 camera_pos;
  vec4 camera_dir;
  vec4 camera_up;
  vec4 camera_side;

  vec4 light_dir;
  vec4 light_color;

  vec4 sky_color_light;
  vec4 sky_color;
};

layout(binding = 1) uniform Materials
{
  vec4 sphere;	// xyz:position w:radius
  vec4 box;		// xyz:position w:size
  vec4 torus_pos;
  vec4 torus_size;
  vec4 hexPrizm_pos;
  vec4 hexPrizm_size;
  vec4 octahedron; // xyz:position w:size
  float l;	// linear interpolation sphere and box
}mat;

layout(binding = 2) uniform Transform
{
  mat4 rotation_sphere;
  mat4 rotation_torus1;
  mat4 rotation_torus2;
  mat4 rotation_torus3;
}transform;

vec3 rotate(vec3 p, mat4 rotation)
{
  vec4 pos = vec4(p, 0);
  pos = rotation * pos;
  return pos.xyz;
}

// 球の距離関数
float sphere_d(vec3 p){
  float sphere = length(p) - mat.sphere.w;

  vec3 b = vec3(mat.box.w);
  vec3 d = abs(p) - b;
  float box = length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);

  return mix(sphere, box, mat.l);
}

// Round Box
float rbox_d(vec3 p)
{
  float r = 0.1;
  vec3 b = vec3(mat.box.w);
  vec3 d = abs(p) - b;
  return length(max(d, 0.0)) - r + min(max(d.x, max(d.y, d.z)), 0.0);
}

// Torus
float torus_d(vec3 p)
{
  vec2 q = vec2(length(p.xy) - mat.torus_size.x, p.z);
  float d1 = length(q) - mat.torus_size.y;

  vec2 q2 = vec2(length(p.yz) - mat.torus_size.x, p.x);
  float d2 = length(q2) - mat.torus_size.y;

  vec2 q3 = vec2(length(p.xz) - mat.torus_size.x, p.y);
  float d3 = length(q3) - mat.torus_size.y;

  return min(min(d1, d2), d3);
}

// Hexagonal Prism
float hexPrizm_d(vec3 p)
{
  vec2 h = mat.hexPrizm_size.xy;
  const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
  p = abs(p);
  p.xy -= 2.0 * min(dot(k.xy, p.xy), 0.0) * k.xy;
  vec2 d = vec2(
    length(p.xy - vec2(clamp(p.x, -k.z * h.x, k.z * h.x), h.x)) * sign(p.y - h.x), p.z-h.y);
  return min(max(d.x, d.y), 0.0) + length(max(d,0.0)) - 0.1;
}

// Octahedron
float octahedron_d(vec3 p)
{
  p = abs(p);
  return ((p.x+p.y+p.z-mat.octahedron.w)*0.57735027);
}


// Plane - Y
float planey_d(vec3 rp)
{
  return dot(rp, vec3(0, 1.0, 0)) + 3.0;
}

vec3 calcPlaneNormal(vec3 pos)
{
  const float eps = 0.0001;
  const vec2 h = vec2(eps, 0);
  return normalize (vec3 (planey_d(pos + h.xyy) - planey_d(pos - h.xyy),
                          planey_d(pos + h.yxy) - planey_d(pos - h.yxy),
						  planey_d(pos + h.yyx) - planey_d(pos - h.yyx)));
}

vec3 reflectionPlane(vec3 pos, vec3 dir)
{
  // 法線算出
  const float eps = 0.0001;
  const vec2 h = vec2(eps, 0);
  vec3 normal = normalize (vec3 (planey_d(pos + h.xyy) - planey_d(pos - h.xyy),
                          planey_d(pos + h.yxy) - planey_d(pos - h.yxy),
						  planey_d(pos + h.yyx) - planey_d(pos - h.yyx)));
  return normalize(reflect(dir, normal));
}

// 距離関数（総合）
float distanceFunc(vec3 pos)
{
  return torus_d(rotate(pos - mat.torus_pos.xyz, transform.rotation_torus1));
}


// 法線
vec3 calcNormal(vec3 pos)
{
  const float eps = 0.0001;
  const vec2 h = vec2(eps, 0);
  return normalize (vec3 (distanceFunc(pos + h.xyy) - distanceFunc(pos - h.xyy),
                          distanceFunc(pos + h.yxy) - distanceFunc(pos - h.yxy),
						  distanceFunc(pos + h.yyx) - distanceFunc(pos - h.yyx)));
}

// 反射距離関数
float reflectionDistance(vec3 pos)
{
  return sphere_d(rotate(pos - mat.sphere.xyz, transform.rotation_sphere));
}

// 反射ベクトル算出
vec3 calcReflectionDir(vec3 pos, vec3 dir)
{
  // 法線算出
  const float eps = 0.0001;
  const vec2 h = vec2(eps, 0);
  vec3 normal = normalize (vec3 (reflectionDistance(pos + h.xyy) - reflectionDistance(pos - h.xyy),
                          reflectionDistance(pos + h.yxy) - reflectionDistance(pos - h.yxy),
						  reflectionDistance(pos + h.yyx) - reflectionDistance(pos - h.yyx)));
  return normalize(reflect(dir, normal));
}

struct Ray {
  vec3 pos;
  vec3 dir;
  vec3 color;
};

vec3 getAlbedo(vec3 pos)
{
  pos = rotate(pos, transform.rotation_torus1);
  float u = (floor(mod(pos.x * 2.0, 2.0)) - 0.5) * 2; // -1 or 1 の範囲に変換
  float v = (floor(mod(pos.y * 2.0, 2.0)) - 0.5) * 2;
  float w = (floor(mod(pos.z * 2.0, 2.0)) - 0.5) * 2;
  return mix(vec3(0.9, 0.5, 0.8), vec3(0.45, 0.25, 0.4), u*v*w);
}

// 色を決定する（ライティング）
vec3 getColor(vec3 pos, vec3 normal, vec3 light_dir, vec3 light_color)
{
  vec3 albedo = getAlbedo(pos);

  // ambient Color
  // NormalベクトルのY軸方向の射影の長さを基準に色を決定する
  float NoY = dot(normal, vec3(0,1,0));
  // 0 - 1に正規化する
  float ambient_intencity = (NoY + 1.0) * 0.5;
  vec3 ambient = mix(sky_color_light.xyz, sky_color.xyz, ambient_intencity);

  // diffuse
  float NoL = dot(normal, light_dir);
  vec3 diffuse = max(light_color * NoL, vec3(0));

  return albedo * (diffuse + ambient);
}

vec3 getColor_plane(vec3 pos)
{
  float u = (floor(mod(pos.x, 2.0)) - 0.5) * 2; // -1 or 1 の範囲に変換
  float v = (floor(mod(pos.z, 2.0)) - 0.5) * 2;
  return mix(vec3(0.7, 0.9, 0.7), vec3(0.5, 0.7, 0.5), u*v);
}

// スカイボックスの色を決定する
vec3 skyBoxColor(vec3 ray_dir)
{
  //float s = (dot(ray_dir, vec3(0,1,0)) + 1.0) * 0.5;
  float s = clamp(dot(ray_dir, vec3(0,1,0)), 0, 1);
  return mix(sky_color_light.xyz, sky_color.xyz, s);
}

// fog
vec3 fog(float depth, vec3 dir, vec3 color)
{
  float minLength = 30;
  float maxLength = 50;
  float w = clamp((depth - minLength) / (maxLength - minLength), 0, 1);
  return mix(color, skyBoxColor(dir), w);
}

vec3 getRay(Ray ray)
{
  float d, dr1, dr2;
  float depth = 1000;
  vec3 col = vec3(0,0,0);

  // レイを飛ばす
  for(int i=0 ; i < 256 ; i++)
  {

    d = distanceFunc(ray.pos);

	// ヒットした
	if(d < 0.001){
	  col = getColor(ray.pos, calcNormal(ray.pos), light_dir.xyz, light_color.xyz);
	  depth = min(depth, distance(camera_pos.xyz, ray.pos));
	  break;
	}

    dr1 = reflectionDistance(ray.pos);
	
	// ヒットした
	if (dr1 < 0.001) {
	  ray.dir = calcReflectionDir(ray.pos, ray.dir);
	  ray.color *= vec3(0.8,0.8,0.9);
	  depth = min(depth, distance(camera_pos.xyz, ray.pos));
	}
	else {
	  d = min(d, dr1);
	}

	
	// 平面
	// ヒットした
	dr2 = planey_d(ray.pos);
	if(dr2 < 0.001) {
	  ray.dir = reflectionPlane(ray.pos, ray.dir);
	  ray.color *= getColor_plane(ray.pos);
	  depth = min(depth, distance(camera_pos.xyz, ray.pos));
	}
	else{
	  d = min(d, dr2);
	}

	col = skyBoxColor(ray.dir);

	// 次のレイは最小距離d * ray.dir のぶんだけ進める
	ray.pos += ray.dir * d;
  }

  return fog(depth, ray.dir, col * ray.color);
}

void main()
{
  // 画面座標の正規化。
  vec2 pos = ((gl_FragCoord.xy * 2.0 - resolution.xy) / max(resolution.x, resolution.y) * vec2(1, -1));

  // レイの位置、飛ぶ方向を定義する
  Ray ray;
  ray.pos = camera_pos.xyz;
  ray.dir = normalize(pos.x * camera_side.xyz + pos.y * camera_up.xyz + camera_dir.xyz);
  ray.color = vec3(1.0,1.0,1.0);
  
  vec4 col = vec4(getRay(ray), 1.0);

  outColor = col;
}
