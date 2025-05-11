#include <pch.h>
#include <Renderer/Cloth/ClothRenderer.h>
#include <Application/Application.h>
#include <Application/Editor.h>
#include <Utils/FreeCamera.h>
#include <Renderer/Cloth/ClothParticleSystem.h>


ClothRenderer::ClothRenderer(Renderer *renderer):
   _clothParticleSystem(new ClothParticleSystem())
{
   bool success = Initialize(renderer);
   assert(success);
}

ClothRenderer::~ClothRenderer()
{
   Terminate();
}

bool ClothRenderer::Initialize(Renderer *renderer)
{
   // Initialize particles and constraints
   _clothParticleSystem->InitializeDemo(15, 15);

   Device *device = renderer->GetDevice();

   // Shader modules
   ShaderModule vertexShader = ShaderModule::LoadShaderModule(device, "cloth.vert");
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(device, "cloth.frag");

   const ClothParticleSystem::ParticleData* particles;
   size_t particleCount = 0;
   _clothParticleSystem->GetParticles(particles, particleCount);
   // Buffers
   {
      _vertexBuffer = new Buffer(device, WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst, sizeof(vec3) * particleCount);
      // upload particle positions
      std::vector<vec3> vertexData;
      vertexData.reserve(particleCount * 3);
      for (size_t i = 0; i < particleCount; ++i)
      {
         const ClothParticleSystem::ParticleData &particle = particles[i];
         vertexData.push_back(particle.position);
      }
      renderer->UploadBufferData(_vertexBuffer, vertexData.data(), sizeof(vec3) * vertexData.size());

      _cameraUniformBuffer = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CameraUniform));
      renderer->UploadBufferData(_cameraUniformBuffer, &_cameraUniform, sizeof(CameraUniform));
   }

   // Bind groups
   {
      std::vector<WGPUBindGroupLayoutEntry> bglEntries = {};
      bglEntries.resize(1);
      bglEntries[0].binding = 0;
      bglEntries[0].visibility = WGPUShaderStage_Vertex;
      bglEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
      bglEntries[0].buffer.minBindingSize = sizeof(CameraUniform);
      bglEntries[0].buffer.hasDynamicOffset = false;

      std::vector<WGPUBindGroupEntry> bgEntries;
      bgEntries.resize(1);
      bgEntries[0].binding = 0;
      bgEntries[0].buffer = _cameraUniformBuffer->Get();
      bgEntries[0].offset = 0;
      bgEntries[0].size = sizeof(CameraUniform);

      _cameraUniformBindGroup = new BindGroup(device, {bglEntries, bgEntries});
   }

   // Render pipeline
   {
      WGPUPipelineLayoutDescriptor plDesc = {};
      plDesc.label = WGPUStringView{"ClothRenderer Pipeline Layout", WGPU_STRLEN};
      plDesc.bindGroupLayoutCount = 1;
      plDesc.bindGroupLayouts = _cameraUniformBindGroup->GetLayout();
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device->Get(), &plDesc);

      WGPURenderPipelineDescriptor rpDesc = {};
      rpDesc.label = WGPUStringView{"ClothRenderer Render Pipeline", WGPU_STRLEN};
      rpDesc.layout = pipelineLayout;
      rpDesc.primitive.topology = WGPUPrimitiveTopology_PointList;
      rpDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
      rpDesc.primitive.frontFace = WGPUFrontFace_CW;
      rpDesc.primitive.cullMode = WGPUCullMode_None;

      rpDesc.multisample.count = 1;
      rpDesc.multisample.mask = 0xFFFFFFFF;
      rpDesc.multisample.alphaToCoverageEnabled = false;
      rpDesc.depthStencil = nullptr;

      // Vertex state
      WGPUVertexAttribute va = {};
      va.format = WGPUVertexFormat_Float32x3;
      va.offset = 0;
      va.shaderLocation = 0;

      WGPUVertexBufferLayout vbl = {};
      vbl.arrayStride = sizeof(vec3);
      vbl.stepMode = WGPUVertexStepMode_Vertex;
      vbl.attributeCount = 1;
      vbl.attributes = &va;
      WGPUVertexState vs = {};
      vs.module = vertexShader.Get();
      vs.entryPoint = WGPUStringView{"main", WGPU_STRLEN};
      vs.bufferCount = 1;
      vs.buffers = &vbl;
      rpDesc.vertex = vs;

      // Fragment state
      WGPUColorTargetState cts = {};
      cts.format = WGPUTextureFormat_RGBA8Unorm;
      cts.writeMask = WGPUColorWriteMask_All;
      WGPUFragmentState fs = {};
      fs.module = fragmentShader.Get();
      fs.entryPoint = WGPUStringView{"main", WGPU_STRLEN};
      fs.targetCount = 1;
      fs.targets = &cts;
      rpDesc.fragment = &fs;

      _renderPipeline = new RenderPipeline(device, &rpDesc);
   }

   return true;
}

void ClothRenderer::Terminate()
{
   delete _renderPipeline;
   delete _cameraUniformBindGroup;
   delete _cameraUniformBuffer;
   delete _vertexBuffer;
}

void ClothRenderer::Update(float dt)
{
   _clothParticleSystem->Update(dt);
}

void ClothRenderer::Render(Renderer *renderer, CommandEncoder *encoder, TextureView *surfaceTextureView)
{
   WGPURenderPassDescriptor rpDesc = {};
   rpDesc.colorAttachmentCount = 1;
   WGPURenderPassColorAttachment ca = {};
   ca.view = surfaceTextureView->Get();
   ca.loadOp = WGPULoadOp_Load;
   ca.storeOp = WGPUStoreOp_Store;
   ca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   rpDesc.colorAttachments = &ca;
   rpDesc.depthStencilAttachment = nullptr;
   RenderPassEncoder renderPassEncoder = RenderPassEncoder(encoder->BeginRenderPass(&rpDesc));

   FreeCamera &camera = Application::GetInstance().GetEditor()->GetCamera();

   // Update vertex buffer with new particle positions
   std::vector<vec3> positions;
   _clothParticleSystem->GetVertexPositions(positions);
   renderer->UploadBufferData(_vertexBuffer, positions.data(), sizeof(vec3) * positions.size());

   // Update uniforms.
   _cameraUniform.view = camera.GetViewMatrix();
   _cameraUniform.proj = camera.GetProjectionMatrix();
   renderer->UploadBufferData(_cameraUniformBuffer, &_cameraUniform, sizeof(_cameraUniform));

   // Render
   renderPassEncoder.SetPipeline(_renderPipeline);
   renderPassEncoder.SetVertexBuffer(0, _vertexBuffer);
   renderPassEncoder.SetBindGroup(0, _cameraUniformBindGroup);
   renderPassEncoder.Draw((uint32_t)positions.size(), 1, 0, 0);
   renderPassEncoder.EndPass();
}
