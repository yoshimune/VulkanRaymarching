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

  vec4 light_dir;
  vec4 light_color;

  vec4 sky_color_light;
  vec4 sky_color;
};

// ���̋����֐�
float sphere_d(vec3 rp){
  vec3 sp = vec3(0,0,0);
  float r = 1.0;
  return length(rp - sp) - r;
}

vec3 calcReflectionDir(vec3 pos, vec3 dir)
{
  // �@���Z�o
  const float eps = 0.0001;
  const vec2 h = vec2(eps, 0);
  vec3 normal = normalize (vec3 (sphere_d(pos + h.xyy) - sphere_d(pos - h.xyy),
                          sphere_d(pos + h.yxy) - sphere_d(pos - h.yxy),
						  sphere_d(pos + h.yyx) - sphere_d(pos - h.yyx)));
  return normalize(reflect(dir, normal));
}

// Box
float box_d(vec3 rp)
{
  vec3 bp = vec3(0, 2.0, 0);
  vec3 b = vec3(0.5, 0.5, 0.5);
  vec3 d = abs(rp - bp) - b;
  return length(max(d,0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

// Round Box
float rbox_d(vec3 rp)
{
  float r = 0.1;
  vec3 bp = vec3(0, -2.0, 0);
  vec3 b = vec3(0.5, 0.5, 0.5);
  vec3 d = abs(rp - bp) - b;
  return length(max(d, 0.0)) - r + min(max(d.x, max(d.y, d.z)), 0.0);
}

// Torus
float torus_d(vec3 rp)
{
  vec2 t = vec2(0.5, 0.2);
  vec3 p = rp - vec3(2.0, 0, 0);
  vec2 q = vec2(length (p.xy) - t.x, p.z);
  return length(q) - t.y;
}

// Hexagonal Prism
float hexPrizm_d(vec3 rp)
{
  vec2 h = vec2(0.5, 0.25);
  vec3 p = rp - vec3(-2.0, 0, 0);
  const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
  p = abs(p);
  p.xy -= 2.0 * min(dot(k.xy, p.xy), 0.0) * k.xy;
  vec2 d = vec2(
    length(p.xy - vec2(clamp(p.x, -k.z*h.x, k.z*h.x), h.x))*sign(p.y-h.x), p.z-h.y);
  return min(max(d.x, d.y), 0.0) + length(max(d,0.0)) - 0.1;
}

// Octahedron
float octahedron_d(vec3 rp)
{
  float s = 0.5;
  vec3 p = abs(rp - vec3(0, 2.0, 0));
  return ((p.x+p.y+p.z-s)*0.57735027);
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

// �����֐��i�����j
float distance(vec3 pos)
{
  return min(min(min(octahedron_d(pos), rbox_d(pos)), torus_d(pos)), hexPrizm_d(pos));
}

// �@��
vec3 calcNormal(vec3 pos)
{
  const float eps = 0.0001;
  const vec2 h = vec2(eps, 0);
  return normalize (vec3 (distance(pos + h.xyy) - distance(pos - h.xyy),
                          distance(pos + h.yxy) - distance(pos - h.yxy),
						  distance(pos + h.yyx) - distance(pos - h.yyx)));
}

struct Ray {
  vec3 pos;
  vec3 dir;
};

// �F�����肷��i���C�e�B���O�j
vec3 getColor(vec3 pos, vec3 normal, vec3 light_dir, vec3 light_color)
{
  float u = (floor(mod(pos.x * 4.0, 2.0)) - 0.5) * 2; // -1 or 1 �͈̔͂ɕϊ�
  float v = (floor(mod(pos.y * 4.0, 2.0)) - 0.5) * 2;
  float w = (floor(mod(pos.z * 4.0, 2.0)) - 0.5) * 2;
  vec3 albedo = mix(vec3(0.9, 0.5, 0.8), vec3(0.45, 0.25, 0.4), u*v*w);

  // ambient Color
  // Normal�x�N�g����Y�������̎ˉe�̒�������ɐF�����肷��
  float NoY = dot(normal, vec3(0,1,0));
  // 0 - 1�ɐ��K������
  float ambient_intencity = (NoY + 1.0) * 0.5;
  vec3 ambient = mix(sky_color_light.xyz, sky_color.xyz, ambient_intencity);

  // diffuse
  float NoL = dot(normal, light_dir);
  vec3 diffuse = max(light_color * NoL, vec3(0));

  return albedo * (diffuse + ambient);
}

// �F�����肷��i���ʃ��C�e�B���O�j
vec3 getColor_plane(vec3 pos, vec3 normal, vec3 light_dir, vec3 light_color)
{
  float u = (floor(mod(pos.x, 2.0)) - 0.5) * 2; // -1 or 1 �͈̔͂ɕϊ�
  float v = (floor(mod(pos.z, 2.0)) - 0.5) * 2;
  vec3 albedo = mix(vec3(0.9, 0.9, 0.9), vec3(0.5, 0.5, 0.5), u*v);

  // ambient Color
  // Normal�x�N�g����Y�������̎ˉe�̒�������ɐF�����肷��
  float NoY = dot(normal, vec3(0,1,0));
  // 0 - 1�ɐ��K������
  float ambient_intencity = (NoY + 1.0) * 0.5;
  vec3 ambient = mix(sky_color_light.xyz, sky_color.xyz, ambient_intencity) * albedo;

  // diffuse
  float NoL = dot(normal, light_dir);
  vec3 diffuse = max(light_color * NoL, vec3(0)) * albedo;

  return albedo * (diffuse + ambient);
}

// �X�J�C�{�b�N�X�̐F�����肷��
vec3 skyBoxColor(vec3 ray_dir)
{
  float s = (dot(ray_dir, vec3(0,1,0)) + 1.0) * 0.5;
  return mix(sky_color_light.xyz, sky_color.xyz, s);
}

void main()
{
  // ��ʍ��W�̐��K���B
  vec2 pos = ((gl_FragCoord.xy * 2.0 - resolution.xy) / max(resolution.x, resolution.y) * vec2(1, -1));

  // ���C�̈ʒu�A��ԕ������`����
  Ray ray;
  ray.pos = camera_pos.xyz;
  ray.dir = normalize(pos.x * camera_side.xyz + pos.y * camera_up.xyz + camera_dir.xyz);

  float t = 0.0, d, d2, rd;
  vec4 col = vec4(skyBoxColor(ray.dir), 1.0);

  // ���C���΂�
  for(int i=0 ; i < 256 ; i++)
  {
    rd = sphere_d(ray.pos);
	if (rd < 0.001) {
	  ray.dir = calcReflectionDir(ray.pos, ray.dir);
	}

    d = distance(ray.pos);

	// �q�b�g����
	if(d < 0.001){
	  col = vec4(getColor(ray.pos, calcNormal(ray.pos), light_dir.xyz, light_color.xyz), 1.0);
	  break;
	}

	// ����
	d2 = planey_d(ray.pos);
	// �q�b�g����
	if(d2 < 0.001) {
	  col = vec4(getColor_plane(ray.pos, calcPlaneNormal(ray.pos), light_dir.xyz, light_color.xyz), 1.0);
	  break;
	}

	// ���̃��C�͍ŏ�����d * ray.dir �̂Ԃ񂾂��i�߂�
	t += min(d, d2);
	ray.pos = camera_pos.xyz + t * ray.dir;
  }

  outColor = col;
}
