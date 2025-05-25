#include <pch.h>
#include <Renderer/Sky/AtmosphereRenderer.h>

AtmosphereRenderer::AtmosphereRenderer(Renderer &renderer)
{
   bool success = Initialize(renderer);
   assert(success);
}

AtmosphereRenderer::~AtmosphereRenderer()
{
   Terminate();
}

bool AtmosphereRenderer::Initialize(Renderer &renderer)
{
   Device &device = renderer.GetDevice();

   // Shader modules
   ShaderModule vertexShader = ShaderModule::LoadShaderModule(device, "atmosphere.vert");
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(device, "atmosphere.frag");

   // Render pipeline
   {
      WGPUPipelineLayoutDescriptor plDesc = {};
      plDesc.bindGroupLayoutCount = 0;
      plDesc.bindGroupLayouts = nullptr;
      WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device.Get(), &plDesc);

      WGPURenderPipelineDescriptor rpDesc = {};
      rpDesc.label = WGPUStringView{"Atmosphere Render Pipeline", WGPU_STRLEN};
      rpDesc.layout = pipelineLayout;
      rpDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
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
         blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
         blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
         blend.color.operation = WGPUBlendOperation_Add;
         blend.alpha.srcFactor = WGPUBlendFactor_Zero;
         blend.alpha.dstFactor = WGPUBlendFactor_One;
         blend.alpha.operation = WGPUBlendOperation_Add;
         cts.blend = &blend;
      }
      cts.format = WGPUTextureFormat_RGBA8Unorm;
      cts.writeMask = WGPUColorWriteMask_All;
      fs.targets = &cts;
      rpDesc.fragment = &fs;

      _renderPipeline = new RenderPipeline(device, &rpDesc);
   }

   return true;
}

void AtmosphereRenderer::Terminate()
{
   delete _renderPipeline;
}

void AtmosphereRenderer::Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex)
{
   WGPURenderPassDescriptor rpDesc = {};
   rpDesc.colorAttachmentCount = 1;
   WGPURenderPassColorAttachment ca = {};
   ca.view = surfaceTextureView.Get();
   ca.loadOp = WGPULoadOp_Clear;
   ca.storeOp = WGPUStoreOp_Store;
   ca.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
   ca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   rpDesc.colorAttachments = &ca;
   rpDesc.depthStencilAttachment = nullptr;
   WGPURenderPassTimestampWrites rpTimestampWrites = {};
   renderer.GetProfiler().GetRenderPassTimestampWrites(profilerIndex, rpTimestampWrites);
   rpDesc.timestampWrites = &rpTimestampWrites;

   const RenderPassEncoder pass = RenderPassEncoder(encoder.BeginRenderPass(&rpDesc));

   // Render
   pass.SetPipeline(*_renderPipeline);
   pass.Draw(3, 1, 0, 0);
   pass.EndPass();
}
