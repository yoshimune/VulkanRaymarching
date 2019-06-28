#version 450

layout(location=0) in vec4 inColor;
layout(location=0) out vec4 outColor;

layout(binding=0) uniform Resolution
{
  vec2 resolution;
  vec3 camera_pos;
  vec3 camera_dir;
  vec3 camera_up;
  vec3 camera_side;
};

// 球の距離関数
float sphere_d(vec3 p){
  const vec3 sphere_pos = vec3(0.0, 0.0, 3.0);
  const float r = 1.0;
  return length(p - sphere_pos) - r;
}

struct Ray {
  vec3 pos;
  vec3 dir;
};

void main()
{
  // 画面座標の正規化。
  vec2 pos = (gl_FragCoord.xy * 2.0 - resolution) / max(resolution.x, resolution.y);

  // レイの位置、飛ぶ方向を定義する
  Ray ray;
  ray.pos = camera_pos;
  ray.dir = normalize(pos.x * camera_side + pos.y * camera_up + camera_dir);

  float t = 0.0, d;
  vec4 col = vec4(0);

  // レイを飛ばす
  for(int i=0 ; i < 64 ; i++)
  {
    d = sphere_d(ray.pos);
	
	// ヒットした
	if(d < 0.001){
	  col = vec4(1.0);
	  break;
	}

	// 次のレイは最小距離d * ray.dir のぶんだけ進める
	t += d;
	ray.pos = camera_pos + t * ray.dir;
  }

  outColor = vec4(camera_side.x, camera_side.y, camera_side.z, 1.0);
  //outColor = vec4(pos.x, 0.0, pos.y, 1.0);
}