#version 450

// 入力情報
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inColor;

// 出力情報
layout(location=0) out vec4 outColor;

// ビルトイン変数
// gl_Positionに頂点変換後の内容を書き込む必要がある
out gl_PerVertex
{
  vec4 gl_Position;
};

void main()
{
  gl_Position = vec4(inPos, 1.0);
  outColor = vec4(inColor, 1.0);
}