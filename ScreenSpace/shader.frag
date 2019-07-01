#version 450

layout(location=0) in vec4 inColor;
layout(location=0) out vec4 outColor;

layout(std140) uniform Resolution
{
  vec4 resolution;
  vec4 camera_pos;
  vec4 camera_dir;
  vec4 camera_up;
  vec4 camera_side;

  vec4 light_pos;
  vec4 light_color;

  vec4 sky_color_light;
  vec4 sky_color;
};

// 球の距離関数
float sphere_d(vec3 p){
  const vec3 sphere_pos = vec3(0.0, 0.0, 3.0);
  const float r = 2.0;
  return length(p - sphere_pos) - r;
}

// 法線ベクトル
vec3 sphere_normal(vec3 pos){
  float delta = 0.001;
  return normalize( 
    vec3 (
      sphere_d(pos + vec3(delta, 0.0, 0.0)) - sphere_d(pos),
      sphere_d(pos + vec3(0.0, delta, 0.0)) - sphere_d(pos),
      sphere_d(pos + vec3(0.0, 0.0, delta)) - sphere_d(pos))
  );
}

struct Ray {
  vec3 pos;
  vec3 dir;
};

vec3 getColor(vec3 pos, vec3 normal, vec3 light_pos, vec3 light_color, vec3 camera_pos)
{
  // ambient Color
  // NormalベクトルのY軸方向の射影の長さを基準に色を決定する
  float NoY = dot(normal, vec3(0,1,0));
  // 0 - 1に正規化する
  float ambient_intencity = (NoY + 1.0) * 0.5;
  vec3 ambient = mix(sky_color_light.xyz, sky_color.xyz, ambient_intencity);

  // diffuse
  vec3 light_diff = light_pos - pos;
  float NoL = clamp(dot(normal, normalize(light_diff)), 0, 1.0);
  float d = length(light_diff) + 1.0;
  vec3 diffuse = (light_color * NoL) / (d*d);

  // edge
  //vec3 camera_dir = normalize(camera_pos - pos);
  //float NoV = clamp(dot(normal, camera_dir), 0, 1.0);
  //float edge_intencity = pow(1.0 - NoV, 2);
  //vec3 edge = light_color * edge_intencity * 0.1;

  //return max(diffuse, edge);
  return diffuse + ambient;
}

void main()
{
  //vec3 col = normalize(-camera_pos);
  //vec3 col = camera_pos.xyz;
  //outColor = vec4(col.x, col.y, col.z, 1.0);
  
  // 画面座標の正規化。
  vec2 pos = (gl_FragCoord.xy * 2.0 - resolution.xy) / max(resolution.x, resolution.y);

  // レイの位置、飛ぶ方向を定義する
  Ray ray;
  ray.pos = camera_pos.xyz;
  ray.dir = normalize(pos.x * camera_side.xyz + pos.y * camera_up.xyz + camera_dir.xyz);

  float t = 0.0, d;
  vec4 col = vec4(0, 0, 0, 0);

  // レイを飛ばす
  for(int i=0 ; i < 64 ; i++)
  {
    d = sphere_d(ray.pos);
	
	// ヒットした
	if(d < 0.001){
	  col = vec4(getColor(ray.pos, sphere_normal(ray.pos), light_pos.xyz, light_color.xyz, camera_pos.xyz), 1.0);
	  break;
	}

	// 次のレイは最小距離d * ray.dir のぶんだけ進める
	t += d;
	ray.pos = camera_pos.xyz + t * ray.dir;
  }

  outColor = col;
}