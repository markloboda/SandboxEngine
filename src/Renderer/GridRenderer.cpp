#include <pch.h>
#include <Renderer/GridRenderer.h>
#include <Application/Application.h>
#include <Application/Editor.h>
#include <Utils/FreeCamera.h>

GridRenderer::GridRenderer(Renderer* renderer)
{
   bool success = Initialize(renderer);
   assert(success);
}

GridRenderer::~GridRenderer()
{
   Terminate();
}

bool GridRenderer::Initialize(Renderer* renderer)
{
   Device* device = renderer->GetDevice();

   // Shader modules
   ShaderModule vertexShader = ShaderModule::LoadShaderModule(device, "grid.vert");
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(device, "grid.frag");

   // Buffers
   {
      float vertexData[] = { -0.5, 0.5f };
      _vertexBuffer = new Buffer(device, WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst, sizeof(vertexData));
      renderer->UploadBufferData(_vertexBuffer, vertexData, sizeof(vertexData));
      _uniformBuffer = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(GridUniforms));
      renderer->UploadBufferData(_uniformBuffer, &_uniforms, sizeof(GridUniforms));
   }

   // Bind groups
   {
      std::vector<WGPUBindGroupLayoutEntry> bglEntries = {};
      bglEntries.resize(1);
      bglEntries[0].binding = 0;
      bglEntries[0].visibility = WGPUShaderStage_Vertex;
      bglEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
      bglEntries[0].buffer.minBindingSize = sizeof(GridUniforms);
      bglEntries[0].buffer.hasDynamicOffset = false;

      std::vector<WGPUBindGroupEntry> bgEntries;
      bgEntries.resize(1);
      bgEntries[0].binding = 0;
      bgEntries[0].buffer = _uniformBuffer->Get();
      bgEntries[0].offset = 0;
      bgEntries[0].size = sizeof(GridUniforms);

      _uniformsBindGroup = new BindGroup(device, { bglEntries, bgEntries });
   }

   // Create render pipeline
   {
      WGPUPipelineLayoutDescriptor plDesc = {};
      std::string plLabel = "GridRenderer Pipeline Layout";
      plDesc.label = WGPUStringView{ plLabel.data(), plLabel.length() };
      plDesc.bindGroupLayoutCount = 1;
      plDesc.bindGroupLayouts = _uniformsBindGroup->GetLayout();
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device->Get(), &plDesc);

      WGPURenderPipelineDescriptor rpDesc = {};
      std::string rpLabel = "GridRenderer Render Pipeline";
      rpDesc.label = WGPUStringView{ rpLabel.data(), rpLabel.length() };
      rpDesc.layout = pipelineLayout;
      rpDesc.primitive.topology = WGPUPrimitiveTopology_LineList;
      rpDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
      rpDesc.primitive.frontFace = WGPUFrontFace_CW;
      rpDesc.primitive.cullMode = WGPUCullMode_Front;
      rpDesc.multisample.count = 1;
      rpDesc.multisample.mask = 0xFFFFFFFF;
      rpDesc.multisample.alphaToCoverageEnabled = false;
      rpDesc.depthStencil = nullptr;

      // Vertex state
      WGPUVertexAttribute va = {};
      va.format = WGPUVertexFormat_Float32;
      va.offset = 0;
      va.shaderLocation = 0;

      WGPUVertexBufferLayout vbl = {};
      vbl.arrayStride = sizeof(float);
      vbl.stepMode = WGPUVertexStepMode_Vertex;
      vbl.attributeCount = 1;
      vbl.attributes = &va;
      WGPUVertexState vs = {};
      vs.module = vertexShader.Get();
      std::string vsEntryPoint = "main";
      vs.entryPoint = WGPUStringView{ vsEntryPoint.data(), vsEntryPoint.length() };
      vs.bufferCount = 1;
      vs.buffers = &vbl;
      rpDesc.vertex = vs;

      // Fragment state
      WGPUColorTargetState cts = {};
      cts.format = WGPUTextureFormat_RGBA8Unorm;
      cts.writeMask = WGPUColorWriteMask_All;
      WGPUFragmentState fs = {};
      fs.module = fragmentShader.Get();
      std::string fsEntryPoint = "main";
      fs.entryPoint = WGPUStringView{ fsEntryPoint.data(), fsEntryPoint.length() };
      fs.targetCount = 1;
      fs.targets = &cts;
      rpDesc.fragment = &fs;

      _renderPipeline = new RenderPipeline(device, &rpDesc);
   }

   return true;
}

void GridRenderer::Terminate()
{
   delete _renderPipeline;
   delete _uniformsBindGroup;
   delete _vertexBuffer;
   delete _uniformBuffer;
}

void GridRenderer::Render(Renderer* renderer, CommandEncoder* encoder, TextureView* surfaceTextureView)
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

   FreeCamera& camera = Application::GetInstance().GetEditor()->GetCamera();

   // Update uniforms.
   _uniforms.view = camera.GetViewMatrix();
   _uniforms.proj = camera.GetProjectionMatrix();
   renderer->UploadBufferData(_uniformBuffer, &_uniforms, sizeof(_uniforms));

   // Render
   renderPassEncoder.SetPipeline(_renderPipeline);
   renderPassEncoder.SetVertexBuffer(0, _vertexBuffer);
   renderPassEncoder.SetBindGroup(0, _uniformsBindGroup);
   renderPassEncoder.Draw(2, _uniforms.numHorizontal + _uniforms.numVertical, 0, 0);
   renderPassEncoder.EndPass();
}

