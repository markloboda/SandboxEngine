#include <pch.h>

CloudRenderer::CloudRenderer(Renderer &renderer, CloudsModel &cloudsModel)
{
   bool success = Initialize(renderer, cloudsModel);
   assert(success);
}

CloudRenderer::~CloudRenderer()
{
   Terminate();
}

bool CloudRenderer::Initialize(Renderer &renderer, CloudsModel &cloudsModel)
{
   Device &device = renderer.GetDevice();

   // Shader modules
   ShaderModule vertexShader = ShaderModule::LoadShaderModule(device, "clouds.vert");
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(device, "clouds.frag");

   // Uniform buffer
   _uCameraData = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CameraData));
   _uResolution = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(ResolutionData));
   _uCloudRenderSettings = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CloudRenderSettings));
   _uCloudRenderWeather = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CloudRenderWeather));

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

      WGPUSamplerDescriptor cloudBaseLowFreqSamplerDesc = {};
      cloudBaseLowFreqSamplerDesc.addressModeU = WGPUAddressMode_MirrorRepeat;
      cloudBaseLowFreqSamplerDesc.addressModeV = WGPUAddressMode_MirrorRepeat;
      cloudBaseLowFreqSamplerDesc.addressModeW = WGPUAddressMode_MirrorRepeat;
      cloudBaseLowFreqSamplerDesc.minFilter = WGPUFilterMode_Linear;
      cloudBaseLowFreqSamplerDesc.magFilter = WGPUFilterMode_Linear;
      cloudBaseLowFreqSamplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
      cloudBaseLowFreqSamplerDesc.maxAnisotropy = 1;
      _uCloudBaseLowFreqSampler = new Sampler(device, &cloudBaseLowFreqSamplerDesc);

      WGPUSamplerDescriptor cloudBaseHighFreqSamplerDesc = {};
      cloudBaseHighFreqSamplerDesc.addressModeU = WGPUAddressMode_MirrorRepeat;
      cloudBaseHighFreqSamplerDesc.addressModeV = WGPUAddressMode_MirrorRepeat;
      cloudBaseHighFreqSamplerDesc.addressModeW = WGPUAddressMode_MirrorRepeat;
      cloudBaseHighFreqSamplerDesc.minFilter = WGPUFilterMode_Linear;
      cloudBaseHighFreqSamplerDesc.magFilter = WGPUFilterMode_Linear;
      cloudBaseHighFreqSamplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
      cloudBaseHighFreqSamplerDesc.maxAnisotropy = 1;
      _uCloudBaseHighFreqSampler = new Sampler(device, &cloudBaseHighFreqSamplerDesc);
   }

   // Texture views
   {
      {
         const CloudsModel::CloudTextureData &weatherMapData = cloudsModel.GetWeatherMapTexture();

         WGPUTextureDescriptor textureDesc = {};
         textureDesc.label = {"Weather Map", WGPU_STRLEN};
         textureDesc.dimension = WGPUTextureDimension_2D;
         textureDesc.size.width = weatherMapData.width;
         textureDesc.size.height = weatherMapData.height;
         textureDesc.size.depthOrArrayLayers = 1;
         textureDesc.sampleCount = 1;
         textureDesc.mipLevelCount = 1;
         textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
         textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
         _weatherMapTexture = new Texture(device, &textureDesc);

         WGPUExtent3D copyExtent = {weatherMapData.width, weatherMapData.height, 1};
         size_t dataSize = weatherMapData.width * weatherMapData.height * weatherMapData.channels * sizeof(uint8_t);
         renderer.UploadTextureData(*_weatherMapTexture, weatherMapData.data, dataSize, &copyExtent);

         WGPUTextureViewDescriptor viewDesc = {};
         viewDesc.format = _weatherMapTexture->GetFormat();
         viewDesc.dimension = WGPUTextureViewDimension_2D;
         viewDesc.baseMipLevel = 0;
         viewDesc.mipLevelCount = 1;
         viewDesc.baseArrayLayer = 0;
         viewDesc.arrayLayerCount = 1;
         _weatherMapTextureView = new TextureView(_weatherMapTexture->Get(), &viewDesc);
      } {
         CloudsModel::CloudTextureData *cloudTextureData;
         cloudsModel.CreateNewLowFreqNoiseTexture(cloudTextureData);

         WGPUTextureDescriptor textureDesc = {};
         textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
         textureDesc.dimension = WGPUTextureDimension_3D;
         textureDesc.size.width = cloudTextureData->width;
         textureDesc.size.height = cloudTextureData->height;
         textureDesc.size.depthOrArrayLayers = cloudTextureData->depth;
         textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
         textureDesc.mipLevelCount = 1;
         textureDesc.sampleCount = 1;
         Texture texture(device, &textureDesc);

         WGPUExtent3D copyExtent = {cloudTextureData->width, cloudTextureData->height, cloudTextureData->depth};
         size_t dataSize = cloudTextureData->width * cloudTextureData->height * cloudTextureData->depth * cloudTextureData->channels * sizeof(uint8_t);
         renderer.UploadTextureData(texture, cloudTextureData->data, dataSize, &copyExtent);

         WGPUTextureViewDescriptor viewDesc = {};
         viewDesc.format = texture.GetFormat();
         viewDesc.dimension = WGPUTextureViewDimension_3D;
         viewDesc.baseMipLevel = 0;
         viewDesc.mipLevelCount = 1;
         viewDesc.baseArrayLayer = 0;
         viewDesc.arrayLayerCount = 1;
         _cloudBaseLowFreqTextureView = new TextureView(texture.Get(), &viewDesc);

         delete cloudTextureData;
      } {
         CloudsModel::CloudTextureData *cloudTextureData;
         cloudsModel.GenerateBaseHighFreqNoiseTexture(cloudTextureData);

         WGPUTextureDescriptor textureDesc = {};
         textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
         textureDesc.dimension = WGPUTextureDimension_3D;
         textureDesc.size.width = cloudTextureData->width;
         textureDesc.size.height = cloudTextureData->height;
         textureDesc.size.depthOrArrayLayers = cloudTextureData->depth;
         textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
         textureDesc.mipLevelCount = 1;
         textureDesc.sampleCount = 1;
         Texture texture(device, &textureDesc);

         WGPUExtent3D copyExtent = {cloudTextureData->width, cloudTextureData->height, cloudTextureData->depth};
         size_t dataSize = cloudTextureData->width * cloudTextureData->height * cloudTextureData->depth * cloudTextureData->channels * sizeof(uint8_t);
         renderer.UploadTextureData(texture, cloudTextureData->data, dataSize, &copyExtent);

         WGPUTextureViewDescriptor viewDesc = {};
         viewDesc.format = texture.GetFormat();
         viewDesc.dimension = WGPUTextureViewDimension_3D;
         viewDesc.baseMipLevel = 0;
         viewDesc.mipLevelCount = 1;
         viewDesc.baseArrayLayer = 0;
         viewDesc.arrayLayerCount = 1;
         _cloudBaseHighFreqTextureView = new TextureView(texture.Get(), &viewDesc);

         delete cloudTextureData;
      }
   }

   // Bind groups
   {
      {
         std::vector<WGPUBindGroupLayoutEntry> bglEntries(6);

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

         // lowFreqCloudTexture (binding 2)
         bglEntries[2].binding = 2;
         bglEntries[2].visibility = WGPUShaderStage_Fragment;
         bglEntries[2].texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
         bglEntries[2].texture.viewDimension = WGPUTextureViewDimension_3D;
         bglEntries[2].texture.multisampled = WGPUOptionalBool_False;

         // lowFreqCloudSampler (binding 3)
         bglEntries[3].binding = 3;
         bglEntries[3].visibility = WGPUShaderStage_Fragment;
         bglEntries[3].sampler.type = WGPUSamplerBindingType_Filtering;

         // highFreqCloudTexture (binding 4)
         bglEntries[4].binding = 4;
         bglEntries[4].visibility = WGPUShaderStage_Fragment;
         bglEntries[4].texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
         bglEntries[4].texture.viewDimension = WGPUTextureViewDimension_3D;
         bglEntries[4].texture.multisampled = WGPUOptionalBool_False;

         // highFreqCloudSampler (binding 5)
         bglEntries[5].binding = 5;
         bglEntries[5].visibility = WGPUShaderStage_Fragment;
         bglEntries[5].sampler.type = WGPUSamplerBindingType_Filtering;

         std::vector<WGPUBindGroupEntry> bgEntries(6);

         bgEntries[0].binding = 0;
         bgEntries[0].textureView = _weatherMapTextureView->Get();
         bgEntries[1].binding = 1;
         bgEntries[1].sampler = _weatherMapSampler->Get();
         bgEntries[2].binding = 2;
         bgEntries[2].textureView = _cloudBaseLowFreqTextureView->Get();
         bgEntries[3].binding = 3;
         bgEntries[3].sampler = _uCloudBaseLowFreqSampler->Get();
         bgEntries[4].binding = 4;
         bgEntries[4].textureView = _cloudBaseHighFreqTextureView->Get();
         bgEntries[5].binding = 5;
         bgEntries[5].sampler = _uCloudBaseHighFreqSampler->Get();

         _texturesBindGroup = new BindGroup(device, {bglEntries, bgEntries});
      } {
         std::vector<WGPUBindGroupLayoutEntry> bglEntries(4);

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

         // cloud render weather (binding 3)
         bglEntries[3].binding = 3;
         bglEntries[3].visibility = WGPUShaderStage_Fragment;
         bglEntries[3].buffer.type = WGPUBufferBindingType_Uniform;
         bglEntries[3].buffer.minBindingSize = sizeof(CloudRenderWeather);

         std::vector<WGPUBindGroupEntry> bgEntries(4);

         bgEntries[0].binding = 0;
         bgEntries[0].buffer = _uCameraData->Get();
         bgEntries[0].size = sizeof(CameraData);
         bgEntries[1].binding = 1;
         bgEntries[1].buffer = _uResolution->Get();
         bgEntries[1].size = sizeof(ResolutionData);
         bgEntries[2].binding = 2;
         bgEntries[2].buffer = _uCloudRenderSettings->Get();
         bgEntries[2].size = sizeof(CloudRenderSettings);
         bgEntries[3].binding = 3;
         bgEntries[3].buffer = _uCloudRenderWeather->Get();
         bgEntries[3].size = sizeof(CloudRenderWeather);

         _dataBindGroup = new BindGroup(device, {bglEntries, bgEntries});
      }
   }

   // Create render pipeline
   {
      // Bind group layouts array
      WGPUBindGroupLayout bindGroupLayouts[2];
      bindGroupLayouts[0] = _texturesBindGroup->GetLayout();
      bindGroupLayouts[1] = _dataBindGroup->GetLayout();

      WGPUPipelineLayoutDescriptor plDesc = {};
      plDesc.bindGroupLayoutCount = 2;
      plDesc.bindGroupLayouts = bindGroupLayouts;
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device.Get(), &plDesc);

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
      WGPUColorTargetState cts = {};
      WGPUBlendState blend = {}; {
         blend.alpha.srcFactor = WGPUBlendFactor_Zero;
         blend.alpha.dstFactor = WGPUBlendFactor_One;
         blend.alpha.operation = WGPUBlendOperation_Add;
         blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
         blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
         blend.color.operation = WGPUBlendOperation_Add;
         cts.blend = &blend;
      }
      cts.format = WGPUTextureFormat_RGBA8Unorm;
      cts.writeMask = WGPUColorWriteMask_All;
      fs.targets = &cts;
      rpDesc.fragment = &fs;

      _pipeline = new RenderPipeline(device, &rpDesc);
   }

   return true;
}

void CloudRenderer::Terminate() const
{
   delete _pipeline;
   delete _texturesBindGroup;
   delete _uCloudBaseLowFreqSampler;
   delete _cloudBaseLowFreqTextureView;
   delete _uCameraData;
   delete _uResolution;
   delete _uCloudRenderWeather;
   delete _uCloudRenderSettings;
   delete _uCloudBaseHighFreqSampler;
   delete _cloudBaseHighFreqTextureView;
   delete _weatherMapSampler;
   delete _weatherMapTextureView;
   delete _weatherMapTexture;
   delete _dataBindGroup;
}

void CloudRenderer::Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, const CloudsModel &cloudsModel,
                           int profilerIndex)
{
   // Collect CloudBounds nodes
   const Application *app = &Application::GetInstance();
   const ResolutionData resolution = {vec2(app->GetWindowWidth(), app->GetWindowHeight())};
   FreeCamera &camera = app->GetRuntime().GetActiveCamera();

   _shaderParams.view = camera.GetViewMatrix();
   _shaderParams.proj = camera.GetProjectionMatrix();
   _shaderParams.pos = camera.GetPosition();
   renderer.UploadBufferData(*_uCameraData, &_shaderParams, sizeof(CameraData));
   renderer.UploadBufferData(*_uResolution, &resolution, sizeof(ResolutionData));
   renderer.UploadBufferData(*_uCloudRenderSettings, &Settings, sizeof(CloudRenderSettings));
   renderer.UploadBufferData(*_uCloudRenderWeather, &Weather, sizeof(CloudRenderWeather));

   // Update weather map.
   const CloudsModel::CloudTextureData &weatherMapData = cloudsModel.GetWeatherMapTexture();
   WGPUExtent3D copyExtent = {weatherMapData.width, weatherMapData.height, 1};
   size_t dataSize = weatherMapData.width * weatherMapData.height * weatherMapData.channels * sizeof(uint8_t);
   renderer.UploadTextureData(*_weatherMapTexture, weatherMapData.data, dataSize, &copyExtent);

   WGPURenderPassDescriptor rpDesc = {};
   WGPURenderPassColorAttachment cp{}; {
      cp.view = surfaceTextureView.Get();
      cp.loadOp = WGPULoadOp_Load;
      cp.storeOp = WGPUStoreOp_Store;
      cp.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
      cp.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   }
   rpDesc.colorAttachmentCount = 1;
   rpDesc.colorAttachments = &cp;
   rpDesc.depthStencilAttachment = nullptr;
   WGPURenderPassTimestampWrites rpTimestampWrites = {};
   renderer.GetProfiler().GetRenderPassTimestampWrites(profilerIndex, rpTimestampWrites);
   rpDesc.timestampWrites = &rpTimestampWrites;

   RenderPassEncoder pass = RenderPassEncoder(encoder.BeginRenderPass(&rpDesc));
   pass.SetPipeline(*_pipeline);
   pass.SetBindGroup(0, *_texturesBindGroup);
   pass.SetBindGroup(1, *_dataBindGroup);
   pass.Draw(3, 1, 0, 0);
   pass.EndPass();
}
