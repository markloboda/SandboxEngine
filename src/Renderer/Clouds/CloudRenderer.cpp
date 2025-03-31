#include <pch.h>
#include <Renderer/Clouds/CloudRenderer.h>

#include <Application/Application.h>
#include <Application/Editor.h>

#include <Renderer/Clouds/CloudBounds.h>
#include <Utils/FreeCamera.h>

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
   _uCameraData = new Buffer(_device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CameraData), nullptr);

   // Noise texture
   {
      WGPUTextureDescriptor texDesc = {};
      texDesc.dimension = WGPUTextureDimension_3D;
      texDesc.size = { 128, 128, 128 };
      texDesc.format = WGPUTextureFormat_R8Unorm;
      texDesc.mipLevelCount = 1;
      texDesc.sampleCount = 1;
      texDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
      _cloudTexture = new Texture(_device, &texDesc);

      WGPUTextureViewDescriptor texViewDesc = {};
      texViewDesc.dimension = WGPUTextureViewDimension_3D;
      texViewDesc.format = WGPUTextureFormat_R8Unorm;
      texViewDesc.mipLevelCount = 1;
      texViewDesc.baseMipLevel = 0;
      texViewDesc.arrayLayerCount = 1;
      texViewDesc.baseArrayLayer = 0;
      texViewDesc.aspect = WGPUTextureAspect_All;
      _cloudTextureView = new TextureView(_cloudTexture->Get(), &texViewDesc);

      // Generate noise data (simplified example)
      std::vector<uint8_t> noiseData(size_t(128 * 128 * 128));  // R8Unorm
      for (size_t i = 0; i < noiseData.size(); ++i)
      {
         // Generate proper noise values (0-255)
         uint8_t value = rand() % 256;
         noiseData[i] = value;
      }

      WGPUExtent3D copySize = { 128, 128, 128 };
      // Upload noise data to the texture
      _cloudTexture->UploadData(_queue->Get(), noiseData.data(), noiseData.size() * sizeof(uint8_t), &copySize, 1);
   }

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

   // Bind groups
   {
      // Bind group layout
      std::vector<WGPUBindGroupLayoutEntry> bglEntries(3);

      // cloudTexture (binding 0)
      bglEntries[0].binding = 0;
      bglEntries[0].visibility = WGPUShaderStage_Fragment;
      bglEntries[0].texture.sampleType = WGPUTextureSampleType_Float;
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

      // Bind group entries
      std::vector<WGPUBindGroupEntry> bgEntries(3);
      bgEntries[0] = {};
      bgEntries[0].binding = 0;
      bgEntries[0].textureView = _cloudTextureView->Get();
      bgEntries[1].binding = 1;
      bgEntries[1].sampler = _uCloudSampler->Get();
      bgEntries[2].binding = 2;
      bgEntries[2].buffer = _uCameraData->Get();
      bgEntries[2].size = sizeof(CameraData);

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


void CloudRenderer::Render(CommandEncoder* encoder, TextureView* surfaceTextureView)
{
   // Collect CloudBounds nodes
   Application* app = &Application::GetInstance();
   vec2 resolution = { app->GetWindowWidth(), app->GetWindowHeight() };
   FreeCamera& camera = app->GetEditor()->GetCamera();

   _shaderParams.view = camera.GetViewMatrix();
   _shaderParams.proj = camera.GetProjectionMatrix();
   _shaderParams.cameraPos = camera.GetPosition();
   _uCameraData->UploadData(_device, &_shaderParams, sizeof(CameraData));

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

   RenderPassEncoder pass = RenderPassEncoder(encoder, &rpDesc);
   pass.SetPipeline(_pipeline);
   pass.SetBindGroup(0, _bindGroup);
   pass.Draw(3, 1, 0, 0);
   pass.EndPass();
}


void CloudRenderer::Terminate()
{
   delete _pipeline;
   delete _bindGroup;
   delete _uCloudSampler;
   delete _cloudTextureView;
   delete _cloudTexture;
   delete _uCameraData;
}
