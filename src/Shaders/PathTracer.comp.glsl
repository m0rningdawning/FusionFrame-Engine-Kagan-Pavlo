#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x=32,local_size_y=32,local_size_z=1) in;
layout(rgba32f,binding=0) uniform image2D image;

layout (location = 1) uniform samplerBuffer MinBounds;
layout (location = 2) uniform samplerBuffer MaxBounds;
layout (location = 3) uniform samplerBuffer ChildIndicies;
layout (location = 4) uniform samplerBuffer TriangleIndicies;
layout (location = 5) uniform samplerBuffer TriangleCounts;

layout (location = 6) uniform vec2 WindowSize;
layout (location = 7) uniform vec3 CameraPosition;
layout (location = 8) uniform mat4 ProjectionViewMat;
layout (location = 9) uniform float Time;

layout (location = 10) uniform samplerBuffer ModelAlbedos;
layout (location = 11) uniform samplerBuffer ModelRoughness;

layout (location = 12) uniform samplerBuffer TriangleUVS;
layout (location = 13) uniform samplerBuffer TriangleNormals;
layout (location = 14) uniform samplerBuffer TrianglePositions;

layout (location = 15) uniform int ModelCount;
layout (location = 16) uniform samplerCube EnvironmentCubeMap;
layout (location = 17) uniform int ProgressiveRenderedFrameCount;

readonly layout(std430,binding=9) restrict buffer ModelTextureHandles
{
   uint64_t TexturesHandles[];
};

readonly layout(std430,binding=8) restrict buffer ModelMatricesData
{
  mat4 ModelMatrices[]; 
};

/*
struct Light
{
  vec4 Position;
  vec4 Color;
  int Type;
  float Intensity;
  float Radius;
  int ShadowMapIndex;
};

layout (location = 14) uniform int LightCount;

layout(std430 , binding = 4) restrict buffer LightsDatas
{
    Light Lights[];
};
*/

#define BACKGROUND_COLOR vec4(1.0f)
const float pos_infinity = uintBitsToFloat(0x7F800000);
const float neg_infinity = uintBitsToFloat(0xFF800000);
const float PI = 3.14159265359;
const vec3 UP_VECTOR = vec3(0.0f,1.0f,0.0f);
const float Epsilon = 0.0001f;

struct IntersectionData
{
   float t;
   vec2 uv;
};

struct RayData
{
   vec3 Position;
   vec3 Normal;
   vec4 Albedo;
   vec2 uv;
   float Roughness;
   float Light;
};

vec3 LIGHT_DIRECTION = vec3(0.1f,0.4f,-0.7f);
vec3 LIGHT_COLOR = vec3(0.9f,0.9f,0.6f);
float LIGHT_INTENSITY = 10.0f;
int NodesToProcess[27];

IntersectionData RayTriangleIntersection(vec3 rayOrigin,vec3 rayDirection,vec3 vertex0,vec3 vertex1,vec3 vertex2)
{
   IntersectionData result;
   result.t = -1.0f;
   result.uv = vec2(0.0f);

   const float EPSILON = 1e-8;
   vec3 e1 = vertex1 - vertex0;
   vec3 e2 = vertex2 - vertex0;

   vec3 h = cross(rayDirection,e2);
   float determinant = dot(h,e1);

   if(determinant > -EPSILON && determinant < EPSILON)
   {
     return result;
   }

   float invDeterminant = 1.0f / determinant;
  
   vec3 T = rayOrigin - vertex0;
   vec3 Q = cross(T,e1);

   float u = dot(h,T) * invDeterminant;
   if(u < 0.0f || u > 1.0f)
   {
     return result;
   }

   float v = dot(Q,rayDirection) * invDeterminant;
   if(v < 0.0f || (u + v) > 1.0f)
   {
     return result;
   }
   
   float t = dot(Q,e2) * invDeterminant;
   if(t > EPSILON)
   {
     result.t = t;
     result.uv = vec2(u,v);
   }
   return result;
}

float RayAABBIntersection(in vec3 rayOrigin,in vec3 rayDirectionInv,in vec3 BoxMin,in vec3 BoxMax)
{
   vec3 tMin = (BoxMin.xyz - rayOrigin) * rayDirectionInv;
   vec3 tMax = (BoxMax.xyz - rayOrigin) * rayDirectionInv;

   vec3 t1 = min(tMin,tMax);
   vec3 t2 = max(tMin,tMax);
  
   float NearDistance = max(max(t1.x,t1.y),t1.z);
   float FarDistance = min(min(t2.x,t2.y),t2.z);

   return (FarDistance >= NearDistance && FarDistance > 0) ? NearDistance : pos_infinity;
}

vec4 RayTraceSphere(in vec3 ro,in vec3 rd,float sphereRadius)
{
    vec3 rayOrigin = ro;
    vec3 rayDirection = rd;
    float SphereRadius = sphereRadius;

    float a = dot(rayDirection, rayDirection);
    float b = dot(rayDirection.xyz,rayOrigin);
    float c = dot(rayOrigin,rayOrigin) - (SphereRadius*SphereRadius);

    float discr = (b * b) - (a*c); 

    if(discr >= 0.0f)
    {
      float t0 = (-b - sqrt(discr)) / 2 * a;
      float t1 = (-b + sqrt(discr)) / 2 * a;

      vec3 HitCoordinate0 = ro + rd * t0;
      vec3 HitCoordinate1 = ro + rd * t1;

      vec3 ViewDirection = normalize(HitCoordinate1 - ro);
      vec3 LightDirection = vec3(sin(Time),0.6f,0.5f);
      vec3 Normal = normalize(-HitCoordinate0);
      float Ambient = 0.01f;

      vec3 H = normalize(ViewDirection + LightDirection);
      float LdotN = dot(LightDirection,Normal);
      float HdotN = pow(max(0.0f,dot(H,Normal)),32.0f);
      
      //return vec4(vec3(Ambient + HdotN + LdotN),1.0f);
      return vec4(1.0f,0.0f,0.0,1.0f);
    }
    return BACKGROUND_COLOR;
}


RayData TraverseBVH(in vec3 rayOrigin,in vec3 rayDirection,in vec3 InvRayDirection,inout float ClosestDistance)
{
     int StackIndex = 0; 
     NodesToProcess[StackIndex] = 0;
     StackIndex++;

     int CurrentIndex;
     
     RayData data;
     data.Normal = vec3(0.0f);
     data.Position = vec3(0.0f);
     data.Albedo = vec4(0.0f,0.0f,0.0f,1.0f);
     data.uv = vec2(0.0f);
     data.Roughness = 0.0f;
     data.Light = 0.0f;

     vec4 v0 = vec4(0.0f);
     vec3 v1 = vec3(0.0f);
     vec3 v2 = vec3(0.0f);

     vec3 n0 = vec3(0.0f);
     vec3 n1 = vec3(0.0f);
     vec3 n2 = vec3(0.0f);

     vec2 uv0 = vec2(0.0f);
     vec2 uv1 = vec2(0.0f);
     vec2 uv2 = vec2(0.0f);

     vec3 Normal = vec3(0.0f);
     vec3 Tangent;
     vec3 Bitangent;
     vec3 up;
     vec2 TriangleUV = vec2(0.0f);
     vec2 TriangleTextureUV = vec2(0.0f);

     mat3 TBNmat = mat3(1.0);

     uint64_t TextureHandle;

     float DistanceChild0 = 0.0f;
     float DistanceChild1 = 0.0f;  
     
     int CurrentModelID = -1;
     int ChildIndex = -1;

     int NearChildIndex = -1;
     int FarChildIndex = -1;

     while(StackIndex > 0)
     {
        StackIndex--;
        CurrentIndex = NodesToProcess[StackIndex];
        
        ChildIndex = int(texelFetch(ChildIndicies,CurrentIndex).r);
        if(ChildIndex == -1)
        {
            int TriangleIndex = int(texelFetch(TriangleIndicies,CurrentIndex).r);
	        int TriCount = int(texelFetch(TriangleCounts,CurrentIndex).r);
            for(int i = TriangleIndex;i < TriangleIndex + TriCount;i++)
            {
                v0 = texelFetch(TrianglePositions,i * 3).xyzw;
                v1 = texelFetch(TrianglePositions,i * 3 + 1).xyz;
                v2 = texelFetch(TrianglePositions,i * 3 + 2).xyz;

                CurrentModelID = int(v0.w);
                
                IntersectionData result = RayTriangleIntersection(rayOrigin,rayDirection.xyz,v0.xyz,v1,v2);

                if(result.t > 0.0f)
                {
                    if(result.t < ClosestDistance)
                    {
                        TriangleUV = result.uv;
                        
                        n0 = texelFetch(TriangleNormals,i * 3).xyz;
                        n1 = texelFetch(TriangleNormals,i * 3 + 1).xyz;
                        n2 = texelFetch(TriangleNormals,i * 3 + 2).xyz;
                        Normal = normalize((1.0f - TriangleUV.x - TriangleUV.y) * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);

                        uv0 = texelFetch(TriangleUVS,i * 3).xy;
                        uv1 = texelFetch(TriangleUVS,i * 3 + 1).xy;
                        uv2 = texelFetch(TriangleUVS,i * 3 + 2).xy;
                        TriangleTextureUV = (1.0f - TriangleUV.x - TriangleUV.y) * uv0 + TriangleUV.x * uv1 + TriangleUV.y * uv2;

                        TextureHandle = TexturesHandles[CurrentModelID];

                        if (TextureHandle != 0) {
                            data.Albedo = vec4(texture(sampler2D(TextureHandle),TriangleTextureUV).xyz,1.0f);
                        }
                        else
                        {
                            data.Albedo = texelFetch(ModelAlbedos,CurrentModelID);
                        } 
                        
                        TextureHandle = TexturesHandles[CurrentModelID + ModelCount];

                        if (TextureHandle != 0) {
                            vec3 up = abs(Normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
                            Tangent = normalize(cross(up, Normal)); 
                            Bitangent = normalize(cross(Normal, Tangent));

                            TBNmat = mat3(Tangent,Bitangent,Normal);
                            Normal = texture(sampler2D(TextureHandle),TriangleTextureUV).xyz * 2.0f - 1.0f;
                            Normal = normalize(TBNmat * Normal);
                            data.Normal = Normal;
                        }
                        else
                        {
                            data.Normal = Normal;
                        }

                        TextureHandle = TexturesHandles[CurrentModelID + ModelCount * 2];

                        if (TextureHandle != 0) {
                            data.Roughness = texture(sampler2D(TextureHandle),TriangleTextureUV).x;
                        }
                        else
                        {
                             data.Roughness = texelFetch(ModelRoughness,CurrentModelID).x;
                        } 

                        ClosestDistance = result.t;
                        data.Position = rayOrigin + rayDirection * result.t;
                        
                        data.Light = max(0.0f,dot(LIGHT_DIRECTION,Normal));
                        data.uv = TriangleTextureUV;
                    }  
                }
            }
        }
        else
        {
               DistanceChild0 = RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,ChildIndex).xyz,texelFetch(MaxBounds,ChildIndex).xyz);
               DistanceChild1 = RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,ChildIndex + 1).xyz,texelFetch(MaxBounds,ChildIndex + 1).xyz);
           
               if (DistanceChild0 < ClosestDistance || DistanceChild1 < ClosestDistance) {
                    if(DistanceChild0 >= DistanceChild1)
                    {
                        if(DistanceChild0 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex;
                            StackIndex++;
                        }
                        if(DistanceChild1 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex + 1;
                            StackIndex++;
                        }
                    }
                    else
                    {
                        if(DistanceChild1 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex + 1;
                            StackIndex++;
                        }
                        if(DistanceChild0 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex;
                            StackIndex++;
                        }
                    } 
                }
        }
     }
     return data;
}

float DistributionGGX(vec3 N , vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N,H),0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV , float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N , vec3 V , vec3 L , float roughness)
{
    float NdotV = max(dot(N,V),0.0);
    float NdotL = max(dot(N,L),0.0);
    float ggx2 = GeometrySchlickGGX(NdotV,roughness);
    float ggx1 = GeometrySchlickGGX(NdotL,roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta , vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta,0.0,1.0),5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 


vec3 PBRshade(vec3 N, vec3 Albedo, float roughness, float Metalic, vec3 CameraPos, vec3 Position, float shadow,inout vec3 F) {
    vec3 V = normalize(CameraPos - Position);
    float NdotV = max(dot(N, V), 0.0);

    vec3 F0 = mix(vec3(0.04), Albedo, Metalic);
    vec3 L = normalize(LIGHT_DIRECTION);
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    float NdotL = max(dot(N, L), 0.0);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - Metalic);

    vec3 specular = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    vec3 radiance = LIGHT_COLOR;

    return (1.0 - shadow) * (kD * Albedo / PI + specular) * radiance * LIGHT_INTENSITY * NdotL;
}

float Hash(inout int x)
{
	x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return float(x & 0xFFFFFF) / float(0x1000000); 
}

vec3 SampleSemiSphere(vec2 seed,vec3 Normal)
{
   float phi = 2.0f * PI * seed.x;
   float cosTheta = sqrt(seed.y);       
   float sinTheta = sqrt(1.0f - seed.y); 

   vec3 SampledVector = vec3(cos(phi) * sinTheta,
                             sin(phi) * sinTheta,
                             cosTheta);

   vec3 Tangent = abs(Normal.z) < 0.999 ? normalize(cross(vec3(0.0, 0.0, 1.0), Normal)) 
                                          : normalize(cross(vec3(1.0, 0.0, 0.0), Normal));   
   vec3 Bitangent = normalize(cross(Tangent,Normal));
                              
   return normalize(SampledVector.x * Tangent + SampledVector.y * Bitangent + SampledVector.z * Normal);
}

float RayTraceShadows(in vec3 CurrentPosition,in vec3 LightDirection,in float LightDiameter,inout uint seed)
{
     vec3 SampleLightDirection = vec3(0.0f);
     float Shadow = 0.0f;
     const int SampleCount = int(5.0f);
     float ClosestDistance = pos_infinity;
     for(int i = 0;i < SampleCount;i++)
     {
       SampleLightDirection = mix(LightDirection,SampleSemiSphere(vec2(Hash(seed),Hash(seed)),LightDirection),LightDiameter);

       vec3 InvRayDirection = 1.0f / (SampleLightDirection);
       TraverseBVH(CurrentPosition + Epsilon * SampleLightDirection , SampleLightDirection,InvRayDirection,ClosestDistance);
       Shadow += float(ClosestDistance != pos_infinity);
       ClosestDistance = pos_infinity;
     }
     Shadow /= SampleCount;
   
     return Shadow;
}

void TraceRay(in vec3 rayOrigin,in vec3 rayDirection,inout float ClosestDistance,inout RayData data,inout RayData ResultData,in int SampleCount,inout uint seed,in vec3 F,in vec3 SurfaceAlbedo,in float SurfaceRoughness,in vec3 CameraPosition)
{
    vec3 Origin = rayOrigin;
    vec3 Direction = rayDirection;
    vec3 InvRayDirection = 1.0f / Direction;
    float isShadow = 0.0f;
    float EnvironmentIntensity = 0.15f;
    vec3 reflectionStrength;
    for(int i = 0;i < SampleCount;i++)
    {
        data = TraverseBVH(Origin + Epsilon * Direction,Direction,InvRayDirection,ClosestDistance);
      
        if(ClosestDistance == pos_infinity)
        {
            reflectionStrength = (1.0f - F) * SurfaceRoughness;
            ResultData.Albedo.xyz += mix(texture(EnvironmentCubeMap,Direction).xyz,SurfaceAlbedo,reflectionStrength);
            ResultData.Light += EnvironmentIntensity;
            ClosestDistance = pos_infinity;
            break;
        }

        isShadow = RayTraceShadows(data.Position,LIGHT_DIRECTION,0.05f,seed);
      
        ResultData.Light += data.Light * (1.0f - isShadow) * (1.0f - data.Roughness);
      
        ResultData.Albedo.xyz += PBRshade(data.Normal, data.Albedo.xyz, data.Roughness, 0.0f, CameraPosition, data.Position, isShadow, F);
        //ResultData.Albedo.xyz = mix(ResultData.Albedo.xyz,SurfaceAlbedo,SurfaceRoughness);    
        ResultData.Position += data.Position;
        ResultData.Normal += data.Normal;

        ClosestDistance = pos_infinity;
        Origin = data.Position;
        Direction = reflect(Direction,data.Normal);
        InvRayDirection = 1.0f / Direction;
    }
}

RayData RayTraceIndirectLight(in vec3 rayOrigin,in vec3 rayDirection,in vec3 SurfaceAlbedo,in float SurfaceRoughness,in float Metallic,in float Shadow,inout uint seed,in vec3 F,in vec3 CameraPosition)
{
   RayData ResultData;
   ResultData.Normal = vec3(0.0f);
   ResultData.Position = vec3(0.0f);
   ResultData.Albedo = vec4(0.0f,0.0f,0.0f,1.0f);
   ResultData.uv = vec2(0.0f);
   ResultData.Roughness = 0.0f;
   ResultData.Light = 0.0f;

   RayData data;
   vec3 SampleVector = vec3(0.0f);

   float ClosestDistance = pos_infinity;
   vec3 InvRayDirection = vec3(0.0f);

   //int SampleCount = max(1,int(SurfaceRoughness * 6.0f));
   int SampleCount = max(1,min(30,int(SurfaceRoughness * ProgressiveRenderedFrameCount)));
   const int BounceCount = 7;
   float invSampleCount = 1.0f / float(SampleCount);
   float weight = 0.0f;

   for(int i = 0;i < SampleCount;i++)
   {
      SampleVector = mix(rayDirection,SampleSemiSphere(vec2(Hash(seed),Hash(seed)),rayDirection),SurfaceRoughness * SurfaceRoughness);
      SampleVector = mix(SampleVector, rayDirection, Metallic);
      SampleVector = normalize(SampleVector);

      TraceRay(rayOrigin,SampleVector,ClosestDistance,data,ResultData,BounceCount,seed,F,SurfaceAlbedo,SurfaceRoughness,rayOrigin);
   }
   ResultData.Light *= invSampleCount;
   ResultData.Albedo *= invSampleCount;
   //ResultData.Albedo.xyz = mix(ResultData.Albedo.xyz,SurfaceAlbedo,F);    
   //ResultData.Albedo.xyz = (ResultData.Albedo.xyz + SurfaceAlbedo) * 0.5f;
   ResultData.Albedo.xyz *= ResultData.Light;

   ResultData.Roughness *= invSampleCount;
   ResultData.Position *= invSampleCount;
   ResultData.Normal *= invSampleCount;

   return ResultData;
}

vec3 PathTrace(in vec3 rayOrigin,in vec3 rayDirection,in ivec2 pixelPos)
{
   float ClosestDistance = pos_infinity;
   vec3 InvRayDirection = 1.0f / (rayDirection);
   RayData data = TraverseBVH(rayOrigin,rayDirection,InvRayDirection,ClosestDistance);
   
   if(ClosestDistance == pos_infinity)
   {
      return vec3(0.0f);
   }
   uint seed = pixelPos.x * pixelPos.y * uint(Time);

   vec3 Color = vec3(0.0f);
   float IsShadow = RayTraceShadows(data.Position,LIGHT_DIRECTION,0.05f,seed);

   vec3 F = vec3(0.0f);

   vec3 directLight = PBRshade(data.Normal, data.Albedo.xyz, data.Roughness, 0.0f, rayOrigin, data.Position, float(IsShadow), F);
   vec3 indirectLight = RayTraceIndirectLight(data.Position, data.Normal,data.Albedo.xyz,data.Roughness,0.0f, IsShadow, seed,F,rayOrigin).Albedo.xyz;
   vec3 finalColor = (directLight + indirectLight) * 0.5f;
   //return indirectLight;
   return finalColor;
}

void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec2 uv = (vec2(pos)+0.5f)/WindowSize;

    vec2 uvND = uv * 2.0f - 1.0f;

    vec3 rayOrigin = CameraPosition;
    vec4 rayDir4 = inverse(ProjectionViewMat) * vec4(uvND, 1.0, 1.0);
    rayDir4 = vec4(rayDir4.xyz / rayDir4.w,1.0f);
    vec3 rayDirection = normalize(rayDir4.xyz - rayOrigin.xyz);

    vec3 result = PathTrace(rayOrigin,rayDirection.xyz,pos);

    if(ProgressiveRenderedFrameCount >= 0)
    {
       float weight = 1.0 / float(ProgressiveRenderedFrameCount + 1);
       vec4 PreviousFrame = imageLoad( image, pos ).rgba;
       result = mix(PreviousFrame.xyz,result,weight);
    }

    imageStore( image, pos,vec4(result,1.0f));
}