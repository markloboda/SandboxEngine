#include <Renderer/GridRenderer.h>
#include <Application/Application.h>
#include <Application/Editor.h>
#include <Utils/FreeCamera.h>

GridRenderer::GridRenderer(GLFWwindow* window, Device* device) :
   _window(window),
   _vertexShader(ShaderModule::LoadShaderModule(device, "grid.vert.spv")),
   _fragmentShader(ShaderModule::LoadShaderModule(device, "grid.frag.spv")),
   _device(device)
{
   bool success = Initialize(device);
   assert(success);
}

GridRenderer::~GridRenderer()
{
   Terminate();
}

bool GridRenderer::Initialize(Device* device)
{
   _device = device;

   // Vertex buffer
   float vertexData[] = { 0.0f, 1.0f };
   _vertexBuffer = new Buffer(device, WGPUBufferUsage_Vertex, sizeof(vertexData), vertexData);

   // Uniform buffer
   _uniformBuffer = new Buffer(device, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, sizeof(GridUniforms), &_uniformsData);

   // Create render pipeline
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

      BindGroupDesc bgDesc = { bglEntries, bgEntries };
      _uniformsBindGroup = new BindGroup(device, bgDesc);

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
      vs.module = _vertexShader.Get();
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
      fs.module = _fragmentShader.Get();
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

void GridRenderer::Render(RenderPassEncoder* renderPassEncoder)
{
   FreeCamera& camera = Application::GetInstance().GetEditor()->GetCamera();

   GridUniforms uniforms = {};
   uniforms.view = camera.GetViewMatrix();
   uniforms.proj = camera.GetProjectionMatrix();
   uniforms.gridSize = 100.0f;
   uniforms.gridSpacing = 1.0f;
   uniforms.numHorizontal = 100;
   uniforms.numVertical = 100;
   _uniformBuffer->UploadData(_device, &uniforms, sizeof(uniforms));

   // Render
   renderPassEncoder->SetPipeline(_renderPipeline);
   renderPassEncoder->SetVertexBuffer(0, _vertexBuffer);

   renderPassEncoder->SetBindGroup(0, _uniformsBindGroup);
   renderPassEncoder->Draw(2, uniforms.numHorizontal + uniforms.numVertical, 0, 0);
}

