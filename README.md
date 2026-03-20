## Rendering with OpenGL

I built a scene using OpenGL, combining basic meshes, texturing them, and adding lights.

## Setting Up Meshes and Textures

I was given source code, which contained all the necessary mesh data for the primitive shapes. I transformed those meshes to replicate the forms from the photo, but I realized that I couldn't render the photo album spine without sacrificing its curve. Referencing the code to render a full cylinder, I added code to render a half cylinder, only rendering half of the faces.

```c++
glDrawArrays(GL_TRIANGLE_STRIP, 72, 72);    // Half of sides
```

![Screenshot of various forms to resemble the bottle, album, and puzzle box. Each component is brightly colored for visiblity.](https://arkatu.com/arkatech/assets/img/opengl_05.png)

For most of the textures, I took photos of the objects and edited the images.

However, the original code had each face of a cube use the same texture. I added two new cube meshes that used texture atlases, one for the photo album and one for the top of the puzzle box. I edited the top textures and side textures together and caluclated the coordinates where each stopped and started.

![Texture atlas for the photo album, showing the front and back textures.](https://arkatu.com/arkatech/assets/img/opengl_12.jpg)

I intitally edited the puzzle box photos to be stacked vertically, but the textures didn't render. I realized that the images probably have dimensions that are a power of 2, so I edited the photos horizontally, which ended up working.

![Texture atlas for the puzzle box, showing the top and side textures.](https://arkatu.com/arkatech/assets/img/opengl_11.jpg)

I also needed to add a switch statement to allow other wrapping options.

```c++
// Edited to allow other texture wrapping options
switch (wrapping) {
    case mirrored_repeat:
        // mirrored repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        break;
    case clamp_to_edge:
        // clamp to edge
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case clamp_to_border:
        // clamp to border
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        break;
    default:
        // repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
}
```

![Screenshot of scene, showing the textured objects against a marble plane with lighting.](https://arkatu.com/arkatech/assets/img/opengl_09.png)

![Side view of scene with the glass of the bottle more visible.](https://arkatu.com/arkatech/assets/img/opengl_10.png)

## Adding Shadows

Later, I wanted to implement cast shadows. I had two lights in the scene, a point light and an ATMO light. Because I was learning how depth maps worked, I decided to only use the point light when calculating shadows. I generated a depth map from the perspective of the point light,

```c++
// Calculate lightspace matrix for shaders
float near_plane = 0.0f, far_plane = 40.0f;

glm::mat4 lightProjection = glm::ortho(
    -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

glm::mat4 lightView = glm::lookAt(
    glm::vec3(-10.0f, 4.0f, 2.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

glm::mat4 lightSpaceMatrix = lightProjection * lightView;

... 

// Pass in matrix to depth shader
g_DepthShaderManager->use();
g_DepthShaderManager->setMat4Value("lightSpaceMatrix", lightSpaceMatrix);

// Clear the frame and z buffers
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Render to depth map
glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
glClear(GL_DEPTH_BUFFER_BIT);

g_SceneManager->RenderScene("depthMap");
```

![Screenshot of generated depth map](https://arkatu.com/arkatech/assets/img/opengl_15.png)

In the main fragment shader, I used the depth map to calculate what fragments were in shadow.

```glsl
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
```

## Results
![Screenshot of scene, showing the objects casting shadows on the marble plane.](https://arkatu.com/arkatech/assets/img/opengl_14.png)

## Improvements
Right now, the cast shadows are calculated as directional shadows instead of point shadows. Becaues the point light should cast shadows around itself, the current directional shadows don't behave as you might expect. Point shadows could be rendered using a cube map. Each face of the cube would hold a depth map, which would be generated based on the direction of the face. Shadows would then be calculated based on the projection of the cube map's faces, giving the illusion of point shadows. I would like to implement point shadows to make the light behave more realistically.
