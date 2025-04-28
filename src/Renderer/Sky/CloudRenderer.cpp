#include <pch.h>
#include <Renderer/Sky/CloudRenderer.h>

#include <Application/Application.h>
#include <Application/Editor.h>

#include <Renderer/Sky/CloudModel.h>
#include <Utils/FreeCamera.h>
#include <Utils/Noise.h>

CloudRenderer::CloudRenderer(Renderer *renderer)
{
   bool success = Initialize(renderer);
   assert(success);
}

CloudRenderer::~CloudRenderer()
{
   Terminate();
}

bool CloudRenderer::Initialize(Renderer *renderer)
{
   Device *device = renderer->GetDevice();

   // Shader modules
   ShaderModule vertexShader = ShaderModule::LoadShaderModule(device, "clouds.vert");
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(device, "clouds.frag");

   // Uniform buffer
   _uCameraData = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CameraData));
   _uResolution = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(ResolutionData));
   _uCloudRenderSettings = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CloudRenderSettings));

   // Clouds model
   _cloudsModel = new CloudsModel(renderer);

   // Sampler
   {
      WGPUSamplerDescriptor weatherMapSamplerDesc = {};
      weatherMapSamplerDesc.addressModeU = WGPUAddressMode_ClampToEdge;
      weatherMapSamplerDesc.addressModeV = WGPUAddressMode_ClampToEdge;
      weatherMapSamplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;
      weatherMapSamplerDesc.minFilter = WGPUFilterMode_Linear;
      weatherMapSamplerDesc.magFilter = WGPUFilterMode_Linear;
      weatherMapSamplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
      weatherMapSamplerDesc.maxAnisotropy = 1;
      _weatherMapSampler = new Sampler(device, &weatherMapSamplerDesc);

      WGPUSamplerDescriptor cloudBaseSamplerDesc = {};
      cloudBaseSamplerDesc.addressModeU = WGPUAddressMode_Repeat;
      cloudBaseSamplerDesc.addressModeV = WGPUAddressMode_Repeat;
      cloudBaseSamplerDesc.addressModeW = WGPUAddressMode_Repeat;
      cloudBaseSamplerDesc.minFilter = WGPUFilterMode_Linear;
      cloudBaseSamplerDesc.magFilter = WGPUFilterMode_Linear;
      cloudBaseSamplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
      cloudBaseSamplerDesc.maxAnisotropy = 1;
      _uCloudBaseSampler = new Sampler(device, &cloudBaseSamplerDesc);
   }

   // Texture views
   {
      {
         Texture *weatherMap = _cloudsModel->GetWeatherMapTexture();
         WGPUTextureViewDescriptor viewDesc = {};
         viewDesc.format = weatherMap->GetFormat();
         viewDesc.dimension = WGPUTextureViewDimension_2D;
         viewDesc.baseMipLevel = 0;
         viewDesc.mipLevelCount = 1;
         viewDesc.baseArrayLayer = 0;
         viewDesc.arrayLayerCount = 1;
         _weatherMapTextureView = new TextureView(weatherMap->Get(), &viewDesc);
      } {
         Texture *cloudBaseTexture = _cloudsModel->GetBaseNoiseTexture();
         WGPUTextureViewDescriptor viewDesc = {};
         viewDesc.format = cloudBaseTexture->GetFormat();
         viewDesc.dimension = WGPUTextureViewDimension_3D;
         viewDesc.baseMipLevel = 0;
         viewDesc.mipLevelCount = 1;
         viewDesc.baseArrayLayer = 0;
         viewDesc.arrayLayerCount = 1;
         _cloudBaseTextureView = new TextureView(cloudBaseTexture->Get(), &viewDesc);
      }
   }

   // Bind groups
   {
      {
         std::vector<WGPUBindGroupLayoutEntry> bglEntries(4);

         // weatherMap (binding 0)
         bglEntries[0].binding = 0;
         bglEntries[0].visibility = WGPUShaderStage_Fragment;
         bglEntries[0].texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
         bglEntries[0].texture.viewDimension = WGPUTextureViewDimension_2D;
         bglEntries[0].texture.multisampled = WGPUOptionalBool_False;

         // weatherMapSampler (binding 1)
         bglEntries[1].binding = 1;
         bglEntries[1].visibility = WGPUShaderStage_Fragment;
         bglEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;

         // cloudTexture (binding 2)
         bglEntries[2].binding = 2;
         bglEntries[2].visibility = WGPUShaderStage_Fragment;
         bglEntries[2].texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
         bglEntries[2].texture.viewDimension = WGPUTextureViewDimension_3D;
         bglEntries[2].texture.multisampled = WGPUOptionalBool_False;

         // cloudSampler (binding 3)
         bglEntries[3].binding = 3;
         bglEntries[3].visibility = WGPUShaderStage_Fragment;
         bglEntries[3].sampler.type = WGPUSamplerBindingType_Filtering;

         std::vector<WGPUBindGroupEntry> bgEntries(4);

         bgEntries[0].binding = 0;
         bgEntries[0].textureView = _weatherMapTextureView->Get();
         bgEntries[1].binding = 1;
         bgEntries[1].sampler = _weatherMapSampler->Get();
         bgEntries[2].binding = 2;
         bgEntries[2].textureView = _cloudBaseTextureView->Get();
         bgEntries[3].binding = 3;
         bgEntries[3].sampler = _uCloudBaseSampler->Get();

         _texturesBindGroup = new BindGroup(device, {bglEntries, bgEntries});
      } {
         std::vector<WGPUBindGroupLayoutEntry> bglEntries(3);

         // camera (binding 0)
         bglEntries[0].binding = 0;
         bglEntries[0].visibility = WGPUShaderStage_Fragment;
         bglEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
         bglEntries[0].buffer.minBindingSize = sizeof(CameraData);

         // resolution (binding 1)
         bglEntries[1].binding = 1;
         bglEntries[1].visibility = WGPUShaderStage_Fragment;
         bglEntries[1].buffer.type = WGPUBufferBindingType_Uniform;
         bglEntries[1].buffer.minBindingSize = sizeof(ResolutionData);

         // cloud render settings (binding 2)
         bglEntries[2].binding = 2;
         bglEntries[2].visibility = WGPUShaderStage_Fragment;
         bglEntries[2].buffer.type = WGPUBufferBindingType_Uniform;
         bglEntries[2].buffer.minBindingSize = sizeof(CloudRenderSettings);

         std::vector<WGPUBindGroupEntry> bgEntries(3);

         bgEntries[0].binding = 0;
         bgEntries[0].buffer = _uCameraData->Get();
         bgEntries[0].size = sizeof(CameraData);
         bgEntries[1].binding = 1;
         bgEntries[1].buffer = _uResolution->Get();
         bgEntries[1].size = sizeof(ResolutionData);
         bgEntries[2].binding = 2;
         bgEntries[2].buffer = _uCloudRenderSettings->Get();
         bgEntries[2].size = sizeof(CloudRenderSettings);

         _dataBindGroup = new BindGroup(device, {bglEntries, bgEntries});
      }
   }

   // Create render pipeline
   {
      // Bind group layouts array
      WGPUBindGroupLayout bindGroupLayouts[2];
      bindGroupLayouts[0] = *_texturesBindGroup->GetLayout();
      bindGroupLayouts[1] = *_dataBindGroup->GetLayout();

      WGPUPipelineLayoutDescriptor plDesc = {};
      plDesc.bindGroupLayoutCount = 2;
      plDesc.bindGroupLayouts = bindGroupLayouts;
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device->Get(), &plDesc);

      WGPURenderPipelineDescriptor rpDesc = {};
      rpDesc.label = WGPUStringView{"CloudRenderer Render Pipeline", WGPU_STRLEN};
      rpDesc.layout = pipelineLayout;
      rpDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
      rpDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
      rpDesc.primitive.frontFace = WGPUFrontFace_CW;
      rpDesc.primitive.cullMode = WGPUCullMode_None;
      rpDesc.multisample.count = 1;
      rpDesc.multisample.mask = 0xFFFFFFFF;
      rpDesc.multisample.alphaToCoverageEnabled = false;
      rpDesc.depthStencil = nullptr;

      // Vertex state
      WGPUVertexState vs = {};
      vs.module = vertexShader.Get();
      vs.entryPoint = WGPUStringView{"main", WGPU_STRLEN};
      rpDesc.vertex = vs;

      // Fragment state
      WGPUFragmentState fs = {};
      fs.module = fragmentShader.Get();
      fs.entryPoint = WGPUStringView{"main", WGPU_STRLEN};
      fs.targetCount = 1;
      WGPUColorTargetState cts = {}; {
         WGPUBlendState blend = {};
         blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
         blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
         blend.color.operation = WGPUBlendOperation_Add;
         blend.alpha.srcFactor = WGPUBlendFactor_Zero;
         blend.alpha.dstFactor = WGPUBlendFactor_One;
         blend.alpha.operation = WGPUBlendOperation_Add;
         cts.format = WGPUTextureFormat_RGBA8Unorm;
         cts.blend = &blend;
         cts.writeMask = WGPUColorWriteMask_All;
      }
      fs.targets = &cts;
      rpDesc.fragment = &fs;

      _pipeline = new RenderPipeline(device, &rpDesc);
   }

   return true;
}

void CloudRenderer::Terminate()
{
   delete _pipeline;
   delete _texturesBindGroup;
   delete _uCloudBaseSampler;
   delete _cloudBaseTextureView;
   delete _uCameraData;
   delete _uResolution;
   delete _cloudsModel;
   delete _uCloudRenderSettings;
}

void CloudRenderer::Render(Renderer *renderer, CommandEncoder *encoder, TextureView *surfaceTextureView)
{
   // Collect CloudBounds nodes
   Application *app = &Application::GetInstance();
   ResolutionData resolution = {vec2(app->GetWindowWidth(), app->GetWindowHeight())};
   FreeCamera &camera = app->GetEditor()->GetCamera();

   _shaderParams.view = camera.GetViewMatrix();
   _shaderParams.proj = camera.GetProjectionMatrix();
   _shaderParams.pos = camera.GetPosition();
   renderer->UploadBufferData(_uCameraData, &_shaderParams, sizeof(CameraData));
   renderer->UploadBufferData(_uResolution, &resolution, sizeof(ResolutionData));
   renderer->UploadBufferData(_uCloudRenderSettings, &Settings, sizeof(CloudRenderSettings));

   WGPURenderPassDescriptor rpDesc = {};
   WGPURenderPassColorAttachment cp{}; {
      cp.view = surfaceTextureView->Get();
      cp.loadOp = WGPULoadOp_Load;
      cp.storeOp = WGPUStoreOp_Store;
      cp.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
      cp.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   }
   rpDesc.colorAttachmentCount = 1;
   rpDesc.colorAttachments = &cp;
   rpDesc.depthStencilAttachment = nullptr;

   RenderPassEncoder pass = RenderPassEncoder(encoder->BeginRenderPass(&rpDesc));
   pass.SetPipeline(_pipeline);
   pass.SetBindGroup(0, _texturesBindGroup);
   pass.SetBindGroup(1, _dataBindGroup);
   pass.Draw(3, 1, 0, 0);
   pass.EndPass();
}
