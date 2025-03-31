#include <pch.h>
#include <Renderer/Clouds/CloudRenderer.h>

CloudRenderer::CloudRenderer(Device* device) :
   _device(device)
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
   _uniformBuffer = new Buffer(_device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(ShaderParams), nullptr);

   // Vertex buffer
   {
      uint32_t vertexData[3] = { 0, 1, 2 };
      _vertexBuffer = new Buffer(_device, WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst, sizeof(vertexData), vertexData);
   }

   // Noise texture
   {
      WGPUTextureDescriptor texDesc = {};
      texDesc.dimension = WGPUTextureDimension_2D;
      texDesc.format = WGPUTextureFormat_RGBA8Unorm;
      texDesc.size = { 512, 512, 1 };
      texDesc.mipLevelCount = 1;
      texDesc.sampleCount = 1;
      texDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
      _noiseTexture = new Texture(_device, &texDesc);

      WGPUTextureViewDescriptor texViewDesc = {};
      texViewDesc.dimension = WGPUTextureViewDimension_2D;
      texViewDesc.format = WGPUTextureFormat_RGBA8Unorm;
      texViewDesc.mipLevelCount = 1;
      texViewDesc.baseMipLevel = 0;
      texViewDesc.arrayLayerCount = 1;
      texViewDesc.baseArrayLayer = 0;
      texViewDesc.usage = WGPUTextureUsage_TextureBinding;
      _noiseTextureView = new TextureView(_noiseTexture->Get(), &texViewDesc);
   }

   // Sampler
   {
      WGPUSamplerDescriptor samplerDesc = {};
      samplerDesc.addressModeU = WGPUAddressMode_Repeat;
      samplerDesc.addressModeV = WGPUAddressMode_Repeat;
      samplerDesc.minFilter = WGPUFilterMode_Linear;
      samplerDesc.maxAnisotropy = 1;
      _sampler = new Sampler(_device, &samplerDesc);
   }

   // Bind groups
   {
      // Bind group layout
      std::vector<WGPUBindGroupLayoutEntry> bglEntries(3);

      // Uniform buffer (binding 0)
      bglEntries[0].binding = 0;
      bglEntries[0].visibility = WGPUShaderStage_Fragment;
      bglEntries[0].buffer.type = WGPUBufferBindingType_Uniform;

      // Texture (binding 1)
      bglEntries[1].binding = 1;
      bglEntries[1].visibility = WGPUShaderStage_Fragment;
      bglEntries[1].texture.sampleType = WGPUTextureSampleType_Float;
      bglEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;

      // Sampler (binding 2)
      bglEntries[2].binding = 2;
      bglEntries[2].visibility = WGPUShaderStage_Fragment;
      bglEntries[2].sampler.type = WGPUSamplerBindingType_Filtering;

      // Bind group entries
      std::vector<WGPUBindGroupEntry> bgEntries(3);
      bgEntries[0] = {};
      bgEntries[0].binding = 0;
      bgEntries[0].buffer = _uniformBuffer->Get();
      bgEntries[0].offset = 0;
      bgEntries[0].size = sizeof(ShaderParams);
      bgEntries[1].binding = 1;
      bgEntries[1].textureView = _noiseTextureView->Get();
      bgEntries[2].binding = 2;
      bgEntries[2].sampler = _sampler->Get();

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
      rpDesc.depthStencil = nullptr;

      // Vertex state
      WGPUVertexState vs = {};
      vs.module = vertexShader.Get();
      std::string vsEntryPoint = "main";
      vs.entryPoint = WGPUStringView{ vsEntryPoint.data(), vsEntryPoint.length() };

      WGPUVertexBufferLayout vbLayout = {};
      vbLayout.arrayStride = sizeof(uint32_t);
      vbLayout.stepMode = WGPUVertexStepMode_Vertex;
      vbLayout.attributeCount = 1;
      WGPUVertexAttribute va = {};
      va.format = WGPUVertexFormat_Uint32;
      va.offset = 0;
      va.shaderLocation = 0;
      vbLayout.attributes = &va;
      vs.bufferCount = 1;
      vs.buffers = &vbLayout;
      rpDesc.vertex = vs;

      // Fragment state
      WGPUFragmentState fs = {};
      fs.module = fragmentShader.Get();
      std::string fsEntryPoint = "main";
      fs.entryPoint = WGPUStringView{ fsEntryPoint.data(), fsEntryPoint.length() };
      fs.targetCount = 1;
      WGPUColorTargetState cts = {};
      cts.format = WGPUTextureFormat_RGBA8Unorm;
      fs.targets = &cts;
      rpDesc.fragment = &fs;

      _pipeline = new RenderPipeline(_device, &rpDesc);
   }

   return true;
}


void CloudRenderer::Render(CommandEncoder* encoder, TextureView* surfaceTextureView)
{
   _uniformBuffer->UploadData(_device, &_shaderParams, sizeof(ShaderParams));

   WGPURenderPassDescriptor rpDesc = {};
   WGPURenderPassColorAttachment colorAttachment{};
   colorAttachment.view = surfaceTextureView->Get();
   colorAttachment.loadOp = WGPULoadOp_Load;
   colorAttachment.storeOp = WGPUStoreOp_Store;
   rpDesc.colorAttachmentCount = 1;
   rpDesc.colorAttachments = &colorAttachment;

   RenderPassEncoder pass = RenderPassEncoder(encoder, &rpDesc);
   pass.SetPipeline(_pipeline);
   pass.SetVertexBuffer(0, _vertexBuffer);
   pass.SetBindGroup(0, _bindGroup);
   pass.Draw(3, 1, 0, 0);
   pass.EndPass();
}


void CloudRenderer::Terminate()
{
   delete _pipeline;
   delete _bindGroup;
   delete _sampler;
   delete _noiseTextureView;
   delete _noiseTexture;
   delete _uniformBuffer;
}
