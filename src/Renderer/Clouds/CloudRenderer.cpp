#include <pch.h>
#include <Renderer/Clouds/CloudRenderer.h>

#include <Application/Application.h>
#include <Application/Editor.h>

#include <Renderer/Clouds/CloudModel.h>
#include <Utils/FreeCamera.h>
#include <Utils/Noise.h>

CloudRenderer::CloudRenderer(Device* device, Queue* queue) :
   _device(device),
   _queue(queue)
{
   bool success = Initialize();
}

CloudRenderer::~CloudRenderer()
{
   Terminate();
}

bool CloudRenderer::Initialize()
{
   // Shader modules
   ShaderModule vertexShader = ShaderModule::LoadShaderModule(_device, "clouds.vert");
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(_device, "clouds.frag");

   // Uniform buffer
   _uCameraData = new Buffer(_device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CameraData));
   _uResolution = new Buffer(_device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(ResolutionData));
   _uCloudRenderSettings = new Buffer(_device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CloudRenderSettings));

   // Clouds model
   _cloudsModel = new CloudsModel();

   // Sampler
   {
      WGPUSamplerDescriptor samplerDesc = {};
      samplerDesc.addressModeU = WGPUAddressMode_Repeat;
      samplerDesc.addressModeV = WGPUAddressMode_Repeat;
      samplerDesc.addressModeW = WGPUAddressMode_Repeat;
      samplerDesc.minFilter = WGPUFilterMode_Linear;
      samplerDesc.magFilter = WGPUFilterMode_Linear;
      samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
      samplerDesc.maxAnisotropy = 1;
      _uCloudSampler = new Sampler(_device, &samplerDesc);
   }

   // Cloud noise texture.
   {
      WGPUTextureDescriptor texDesc = {};
      texDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
      texDesc.dimension = WGPUTextureDimension_3D;
      vec3 texDim = _cloudsModel->GetTextureDimensions();
      texDesc.size.width = texDim.x;
      texDesc.size.height = texDim.y;
      texDesc.size.depthOrArrayLayers = texDim.z;
      texDesc.format = WGPUTextureFormat_RGBA8Unorm;
      texDesc.mipLevelCount = 1;
      texDesc.sampleCount = 1;
      _cloudTexture = new Texture(_device, &texDesc);
      if (!_cloudTexture->IsValid())
      {
         std::cerr << "Failed to create cloud texture." << std::endl;
         return false;
      }
      WGPUTextureViewDescriptor viewDesc = {};
      viewDesc.format = texDesc.format;
      viewDesc.dimension = WGPUTextureViewDimension_3D;
      viewDesc.baseMipLevel = 0;
      viewDesc.mipLevelCount = 1;
      viewDesc.baseArrayLayer = 0;
      viewDesc.arrayLayerCount = 1;
      _cloudTextureView = new TextureView(_cloudTexture->Get(), &viewDesc);

      // Upload data to the texture
      vec3 texSize = _cloudsModel->GetTextureDimensions();
      uint32_t texWidth = static_cast<uint32_t>(texSize.x);
      uint32_t texHeight = static_cast<uint32_t>(texSize.y);
      uint32_t texDepth = static_cast<uint32_t>(texSize.z);
      WGPUExtent3D copyExtent = { texWidth, texHeight, texDepth };
      _cloudTexture->UploadData(_queue, _cloudsModel->GetCloudFBM(), _cloudsModel->GetCloudFBMSize(), &copyExtent);
   }

   // Bind groups
   {
      // Bind group layout
      std::vector<WGPUBindGroupLayoutEntry> bglEntries(5);

      // cloudTexture (binding 0)
      bglEntries[0].binding = 0;
      bglEntries[0].visibility = WGPUShaderStage_Fragment;
      bglEntries[0].texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
      bglEntries[0].texture.viewDimension = WGPUTextureViewDimension_3D;
      bglEntries[0].texture.multisampled = WGPUOptionalBool_False;

      // cloudSampler (binding 1)
      bglEntries[1].binding = 1;
      bglEntries[1].visibility = WGPUShaderStage_Fragment;
      bglEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;

      // camera (binding 2)
      bglEntries[2].binding = 2;
      bglEntries[2].visibility = WGPUShaderStage_Fragment;
      bglEntries[2].buffer.type = WGPUBufferBindingType_Uniform;
      bglEntries[2].buffer.minBindingSize = sizeof(CameraData);

      // resolution (binding 3)
      bglEntries[3].binding = 3;
      bglEntries[3].visibility = WGPUShaderStage_Fragment;
      bglEntries[3].buffer.type = WGPUBufferBindingType_Uniform;
      bglEntries[3].buffer.minBindingSize = sizeof(ResolutionData);

      // cloud render settings (binding 4)
      bglEntries[4].binding = 4;
      bglEntries[4].visibility = WGPUShaderStage_Fragment;
      bglEntries[4].buffer.type = WGPUBufferBindingType_Uniform;
      bglEntries[4].buffer.minBindingSize = sizeof(CloudRenderSettings);

      // Bind group entries
      std::vector<WGPUBindGroupEntry> bgEntries(5);
      bgEntries[0] = {};
      bgEntries[0].binding = 0;
      bgEntries[0].textureView = _cloudTextureView->Get();
      bgEntries[1].binding = 1;
      bgEntries[1].sampler = _uCloudSampler->Get();
      bgEntries[2].binding = 2;
      bgEntries[2].buffer = _uCameraData->Get();
      bgEntries[2].size = sizeof(CameraData);
      bgEntries[3].binding = 3;
      bgEntries[3].buffer = _uResolution->Get();
      bgEntries[3].size = sizeof(ResolutionData);
      bgEntries[4].binding = 4;
      bgEntries[4].buffer = _uCloudRenderSettings->Get();
      bgEntries[4].size = sizeof(CloudRenderSettings);

      _bindGroup = new BindGroup(_device, { bglEntries, bgEntries });
   }

   // Create render pipeline
   {
      WGPUPipelineLayoutDescriptor plDesc = {};
      plDesc.bindGroupLayoutCount = 1;
      plDesc.bindGroupLayouts = _bindGroup->GetLayout();
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(_device->Get(), &plDesc);

      WGPURenderPipelineDescriptor rpDesc = {};
      std::string rpLabel = "CloudRenderer Render Pipeline";
      rpDesc.label = WGPUStringView{ rpLabel.data(), rpLabel.length() };
      rpDesc.layout = pipelineLayout;
      rpDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
      rpDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
      rpDesc.primitive.frontFace = WGPUFrontFace_CW;
      rpDesc.primitive.cullMode = WGPUCullMode_None;
      rpDesc.multisample.count = 1;
      rpDesc.multisample.mask = 0xFFFFFFFF;
      rpDesc.multisample.alphaToCoverageEnabled = false;
      // TODO:
      rpDesc.depthStencil = nullptr;

      // Vertex state
      WGPUVertexState vs = {};
      vs.module = vertexShader.Get();
      std::string vsEntryPoint = "main";
      vs.entryPoint = WGPUStringView{ vsEntryPoint.data(), vsEntryPoint.length() };
      rpDesc.vertex = vs;

      // Fragment state
      WGPUFragmentState fs = {};
      fs.module = fragmentShader.Get();
      std::string fsEntryPoint = "main";
      fs.entryPoint = WGPUStringView{ fsEntryPoint.data(), fsEntryPoint.length() };
      fs.targetCount = 1;
      WGPUColorTargetState cts = {};
      {
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

      _pipeline = new RenderPipeline(_device, &rpDesc);
   }

   return true;
}

void CloudRenderer::Terminate()
{
   delete _pipeline;
   delete _bindGroup;
   delete _uCloudSampler;
   delete _cloudTextureView;
   delete _cloudTexture;
   delete _uCameraData;
   delete _uResolution;
   delete _cloudsModel;
   delete _uCloudRenderSettings;
}

void CloudRenderer::Render(CommandEncoder* encoder, TextureView* surfaceTextureView)
{
   // Collect CloudBounds nodes
   Application* app = &Application::GetInstance();
   ResolutionData resolution = { vec2(app->GetWindowWidth(), app->GetWindowHeight()) };
   FreeCamera& camera = app->GetEditor()->GetCamera();

   _shaderParams.view = camera.GetViewMatrix();
   _shaderParams.proj = camera.GetProjectionMatrix();
   _shaderParams.pos = camera.GetPosition();
   _uCameraData->UploadData(_device, &_shaderParams, sizeof(CameraData));
   _uResolution->UploadData(_device, &resolution, sizeof(ResolutionData));
   _uCloudRenderSettings->UploadData(_device, &Settings, sizeof(CloudRenderSettings));

   WGPURenderPassDescriptor rpDesc = {};
   WGPURenderPassColorAttachment colorAttachment{};
   {
      colorAttachment.view = surfaceTextureView->Get();
      colorAttachment.loadOp = WGPULoadOp_Load;
      colorAttachment.storeOp = WGPUStoreOp_Store;
      colorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
   }
   rpDesc.colorAttachmentCount = 1;
   rpDesc.colorAttachments = &colorAttachment;

   RenderPassEncoder pass = *encoder->BeginRenderPass(&rpDesc);
   pass.SetPipeline(_pipeline);
   pass.SetBindGroup(0, _bindGroup);
   pass.Draw(3, 1, 0, 0);
   pass.EndPass();
}
