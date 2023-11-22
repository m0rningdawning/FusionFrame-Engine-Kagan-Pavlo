#version 330 core

out vec4 OutColor;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

uniform vec3 CameraPos;
uniform float FarPlane;
uniform float NearPlane;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_metalic0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_metalic1;

uniform vec4 albedo;
uniform float metallic;
uniform float roughness;

uniform float FogIntesityUniform;
uniform vec3 FogColor;

uniform int disableclaymaterial[4];

void main()
{
    vec3 texturecolor;
     
    if(disableclaymaterial[0] == 1)
    {
      texturecolor = albedo.rgb;
    }
    else
    {
      texturecolor = texture(texture_diffuse0, FinalTexCoord).rgb;
    }

    float roughnessmap;

    if(disableclaymaterial[1] == 1)
    {
      roughnessmap = roughness;
    }
    else
    {
      roughnessmap = texture(texture_specular0, FinalTexCoord).r;
    }

    vec3 resultnormal;

    if(disableclaymaterial[2] == 1)
    {
        resultnormal = normalize(Normal);
    }
    else
    {
        resultnormal = texture(texture_normal0,FinalTexCoord).rgb;
        resultnormal = resultnormal * 2.0f - 1.0f;
        resultnormal = normalize(TBN * resultnormal);
    }


    float metalicmap;

    if(disableclaymaterial[3] == 1)
    {
      metalicmap = metallic;
    }
    else
    {
      metalicmap = texture(texture_metalic0, FinalTexCoord).r;
    }


   vec3 LightPosition = vec3(0.0f, 1.9f,0.0f);
   vec3 LightColor = vec3(1.0f,1.0f,1.0f);
   float Ambient = 0.20f;

   vec3 specularColor = LightColor;

   vec3 N = resultnormal;
   vec3 L = LightPosition - CurrentPos;
   vec3 LDR = normalize(L);

   float LightDistance = length(L);
   float a = 0.2f;
   float b = 0.1f;
   float intensity = 6.0f / (a * LightDistance * LightDistance + b * LightDistance + 1.0f);
   
   vec3 V = CameraPos - CurrentPos;
   vec3 H = normalize(L + V);

   float diffuse = max(dot(N,LDR),0.0f);
   vec3 specular = pow(max(dot(N,H),0.0f),32.0f) * specularColor;

   float DeltaPlane = FarPlane - NearPlane;
   float distanceFromCamera = distance(CameraPos,CurrentPos) / DeltaPlane;

   float FogIntensity = distanceFromCamera * distanceFromCamera * FogIntesityUniform;

   OutColor = vec4((texturecolor * LightColor * ((diffuse  * intensity + Ambient) + roughnessmap * specular  * intensity)) + (FogColor * FogIntensity),1.0f);
}