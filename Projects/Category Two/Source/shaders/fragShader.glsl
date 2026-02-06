#version 330 core

struct Material 
{
    vec3 ambientColor;
    float ambientStrength;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
}; 

struct LightSource 
{
    vec3 position;	
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float focalStrength;
    float specularIntensity;
};

#define TOTAL_LIGHTS 4

in vec3 fragmentPosition;
in vec3 fragmentVertexNormal;
in vec2 fragmentTextureCoordinate;
in vec4 fragmentPosLightSpace;

out vec4 outFragmentColor;

uniform bool bUseTexture=false;
uniform bool bUseLighting=false;
uniform vec4 objectColor = vec4(1.0f);
uniform sampler2D objectTexture;
uniform sampler2D depthMap;
uniform vec3 viewPosition;
uniform vec2 UVscale = vec2(1.0f, 1.0f);
uniform LightSource lightSources[TOTAL_LIGHTS];
uniform Material material;

// function prototypes
vec3 CalcLightSource(LightSource light, vec3 lightNormal, vec3 vertexPosition, vec3 viewDirection);
float CalcShadow(vec4 fragPosLightSpace, vec3 lightNormal, vec3 lightDir);

void main()
{
   if(bUseLighting == true)
   {
      // properties
      vec3 lightNormal = normalize(fragmentVertexNormal);
      vec3 viewDirection = normalize(viewPosition - fragmentPosition);
      vec3 phongResult = vec3(0.0f);

      for(int i = 0; i < TOTAL_LIGHTS; i++)
      {
         phongResult += CalcLightSource(lightSources[i], lightNormal, fragmentPosition, viewDirection); 
      }   
    
      if(bUseTexture == true)
      {
         vec4 textureColor = texture(objectTexture, fragmentTextureCoordinate * UVscale);
         outFragmentColor = vec4(phongResult * textureColor.xyz, 1.0);
      }
      else
      {
         outFragmentColor = vec4(phongResult * objectColor.xyz, objectColor.w);
      }
   }
   else 
   {
      if(bUseTexture == true)
      {
         outFragmentColor = texture(objectTexture, fragmentTextureCoordinate * UVscale);
      }
      else
      {
         outFragmentColor = objectColor;
      }
   }
}

// calculates the color when using a directional light.
vec3 CalcLightSource(LightSource light, vec3 lightNormal, vec3 vertexPosition, vec3 viewDirection)
{
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;

   //**Calculate Ambient lighting**

   ambient = light.ambientColor + (material.ambientColor * material.ambientStrength);

   //**Calculate Diffuse lighting**

   // Calculate distance (light direction) between light source and fragments/pixels
   vec3 lightDirection = normalize(light.position - vertexPosition); 

   vec3 halfwayDir = normalize(lightDirection + viewDirection);

   // Calculate diffuse impact by generating dot product of normal and light
   float impact = max(dot(lightNormal, lightDirection), 0.0);
   // Generate diffuse material color   
   diffuse = impact * material.diffuseColor * light.diffuseColor;

   //**Calculate Specular lighting**

   // Calculate reflection vector
   vec3 reflectDir = reflect(-lightDirection, lightNormal);
   // Calculate specular component
   float specularComponent = 0.0;
   specularComponent = pow(max(dot(lightNormal, halfwayDir), 0.0), 16.0);
   specular = (light.specularIntensity * material.shininess) * specularComponent * material.specularColor;

   // calculate shadow
   float shadow = CalcShadow(fragmentPosLightSpace, lightNormal, lightDirection);

   return(ambient + (1.0 - shadow) * (diffuse + specular));
}

float CalcShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
   // perform perspective divide
   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
   // transform to [0,1] range
   projCoords = projCoords * 0.5 + 0.5;
   // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
   float closestDepth = texture(depthMap, projCoords.xy).r; 
   // get depth of current fragment from light's perspective
   float currentDepth = projCoords.z;

   float bias = max(0.003 * (1.0 - dot(normal, lightDir)), 0.002);  
   // check whether current frag pos is in shadow
   float shadow = 0.0;
   vec2 texelSize = 1.0 / textureSize(depthMap, 0);
   for(int x = -1; x <= 1; ++x)
   {
      for(int y = -1; y <= 1; ++y)
      {
         float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
         shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
      }    
   }
   shadow /= 15.0; 

   if(projCoords.z > 1.0)
   {
      shadow = 0.0;
   }
      
   return shadow;
}