#include <pch.h>
#include <Renderer/Cloth/ClothRenderer.h>
#include <Application/Application.h>
#include <Application/Editor.h>
#include <Utils/FreeCamera.h>
#include <Renderer/Cloth/ClothParticleSystem.h>


ClothRenderer::ClothRenderer(Renderer &renderer):
   _clothParticleSystem(new ClothParticleSystem())
{
   bool success = Initialize(renderer);
   assert(success);
}

ClothRenderer::~ClothRenderer()
{
   Terminate();
}

bool ClothRenderer::Initialize(Renderer &renderer)
{
   // Initialize particles and constraints
   int width = 10;
   int height = 10;
   _clothParticleSystem->InitializeDemo(width, height);

   Device &device = renderer.GetDevice();

   // Shader modules
   ShaderModule &vertexShader = ShaderModule::LoadShaderModule(device, "cloth.vert");
   ShaderModule &fragmentShader = ShaderModule::LoadShaderModule(device, "cloth.frag");

   std::vector<VertexData> vertices;
   std::vector<uint32_t> indices;
   GenerateVertexData(vertices, indices);
   // Buffers
   {
      _vertexBuffer = new Buffer(device, WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst, sizeof(VertexData) * vertices.size());
      renderer.UploadBufferData(*_vertexBuffer, vertices.data(), sizeof(VertexData) * vertices.size());

      _indexBuffer = new Buffer(device, WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst, sizeof(uint32_t) * indices.size());
      renderer.UploadBufferData(*_indexBuffer, indices.data(), sizeof(uint32_t) * indices.size());

      _cameraUniformBuffer = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(CameraUniform));
      renderer.UploadBufferData(*_cameraUniformBuffer, &_cameraUniform, sizeof(CameraUniform));
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
      plDesc.bindGroupLayouts = &_cameraUniformBindGroup->GetLayout();
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device.Get(), &plDesc);

      WGPURenderPipelineDescriptor rpDesc = {};
      rpDesc.label = WGPUStringView{"ClothRenderer Render Pipeline", WGPU_STRLEN};
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
      std::vector<WGPUVertexAttribute> vas = {};
      vas.resize(2);
      // Position
      vas[0].format = WGPUVertexFormat_Float32x3;
      vas[0].offset = 0;
      vas[0].shaderLocation = 0;
      // Normal
      vas[1].format = WGPUVertexFormat_Float32x3;
      vas[1].offset = sizeof(VertexData::position);
      vas[1].shaderLocation = 1;

      WGPUVertexBufferLayout vbl = {};
      vbl.arrayStride = sizeof(VertexData);
      vbl.stepMode = WGPUVertexStepMode_Vertex;
      vbl.attributeCount = 2;
      vbl.attributes = vas.data();
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

void ClothRenderer::Terminate() const
{
   delete _renderPipeline;
   delete _cameraUniformBindGroup;
   delete _cameraUniformBuffer;
   delete _vertexBuffer;
   delete _indexBuffer;
   delete _clothParticleSystem;
}

void ClothRenderer::Update(const float dt) const
{
   _clothParticleSystem->Update(dt);
}

void ClothRenderer::Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, const uint32_t profilerIndex)
{
   WGPURenderPassDescriptor rpDesc = {};
   rpDesc.colorAttachmentCount = 1;
   WGPURenderPassColorAttachment ca = {};
   ca.view = surfaceTextureView.Get();
   ca.loadOp = WGPULoadOp_Load;
   ca.storeOp = WGPUStoreOp_Store;
   ca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   rpDesc.colorAttachments = &ca;
   rpDesc.depthStencilAttachment = nullptr;
   WGPURenderPassTimestampWrites rpTimestampWrites = {};
   renderer.GetProfiler().GetRenderPassTimestampWrites(profilerIndex, rpTimestampWrites);
   rpDesc.timestampWrites = &rpTimestampWrites;
   RenderPassEncoder renderPassEncoder = RenderPassEncoder(encoder.BeginRenderPass(&rpDesc));

   FreeCamera &camera = Application::GetInstance().GetEditor().GetCamera();

   // Update vertex buffer with new particle positions
   std::vector<VertexData> vertices;
   std::vector<uint32_t> indices;
   GenerateVertexData(vertices, indices);
   renderer.UploadBufferData(*_vertexBuffer, vertices.data(), sizeof(VertexData) * vertices.size());
   renderer.UploadBufferData(*_indexBuffer, indices.data(), sizeof(uint32_t) * indices.size());

   // Update uniforms.
   _cameraUniform.view = camera.GetViewMatrix();
   _cameraUniform.proj = camera.GetProjectionMatrix();
   renderer.UploadBufferData(*_cameraUniformBuffer, &_cameraUniform, sizeof(_cameraUniform));

   // Render
   renderPassEncoder.SetPipeline(*_renderPipeline);
   renderPassEncoder.SetVertexBuffer(0, *_vertexBuffer);
   renderPassEncoder.SetIndexBuffer(*_indexBuffer, WGPUIndexFormat_Uint32);
   renderPassEncoder.SetBindGroup(0, *_cameraUniformBindGroup);
   renderPassEncoder.DrawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
   renderPassEncoder.EndPass();
}

void ClothRenderer::GenerateVertexData(std::vector<VertexData> &outVertices, std::vector<uint32_t> &outIndices) const
{
   const vec2 dimensions = _clothParticleSystem->GetDimensions();
   const auto width = static_cast<size_t>(dimensions.x);
   const auto height = static_cast<size_t>(dimensions.y);

   const ClothParticleSystem::ParticleData *particles;
   size_t particleCount = 0;
   _clothParticleSystem->GetParticles(particles, particleCount);

   outVertices.resize(particleCount);
   for (size_t i = 0; i < particleCount; ++i)
   {
      outVertices[i].position = particles[i].position;
      outVertices[i].normal = vec3(0.0f);
   }

   outIndices.clear();
   for (size_t i = 0; i < width - 1; ++i)
   {
      for (size_t j = 0; j < height - 1; ++j)
      {
         size_t i0 = i * height + j;
         size_t i1 = (i + 1) * height + j;
         size_t i2 = i * height + (j + 1);
         size_t i3 = (i + 1) * height + (j + 1);

         // Triangle 1
         outIndices.push_back(static_cast<uint32_t>(i0));
         outIndices.push_back(static_cast<uint32_t>(i1));
         outIndices.push_back(static_cast<uint32_t>(i2));

         // Triangle 2
         outIndices.push_back(static_cast<uint32_t>(i2));
         outIndices.push_back(static_cast<uint32_t>(i1));
         outIndices.push_back(static_cast<uint32_t>(i3));

         // Face normals
         vec3 n1 = normalize(cross(outVertices[i1].position - outVertices[i0].position,
                                   outVertices[i2].position - outVertices[i0].position));
         vec3 n2 = normalize(cross(outVertices[i1].position - outVertices[i2].position,
                                   outVertices[i3].position - outVertices[i2].position));

         outVertices[i0].normal += n1;
         outVertices[i1].normal += n1;
         outVertices[i2].normal += n1;

         outVertices[i2].normal += n2;
         outVertices[i1].normal += n2;
         outVertices[i3].normal += n2;
      }
   }

   // Normalize accumulated normals
   for (auto &v: outVertices)
   {
      v.normal = normalize(v.normal);
   }
}

