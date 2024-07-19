#include "MyViewExtension.h"

#include "ClearQuad.h"
#include "FXRenderingUtils.h"
#include "RenderGraphBuilder.h"
#include "SceneView.h"
#include "SceneRendererInterface.h"
#include "PostProcess/PostProcessInputs.h"
#include "RHIGPUReadback.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#include "MyShader.h"

FMyViewExtension::FMyViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
{
}

void SavePooledBufferDataToImage(FRDGBuilder& GraphBuilder, TRefCountPtr<FRDGPooledBuffer> PooledBuffer, const int32 i)
{
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("SavePooledBufferDataToImage"),
		ERDGPassFlags::None,
		[PooledBuffer, i](FRHICommandListImmediate& RHICmdList)
		{
			check(PooledBuffer.IsValid());

			const uint32 BufferSize = PooledBuffer->GetSize();
			FRHIGPUBufferReadback BufferReadback(TEXT("BufferReadback"));
			BufferReadback.EnqueueCopy(RHICmdList, PooledBuffer->GetRHI(), BufferSize);

			RHICmdList.BlockUntilGPUIdle();
			RHICmdList.FlushResources();

			void* BufferData = BufferReadback.Lock(BufferSize);
			int32 Width = 640;
			int32 Height = 480;
			TArray<uint8> ImageData;
			ImageData.AddUninitialized(Width * Height * 4);
			FMemory::Memcpy(ImageData.GetData(), BufferData, BufferSize);
			BufferReadback.Unlock();

			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

			ImageWrapper->SetRaw(ImageData.GetData(), ImageData.Num(), Width, Height, ERGBFormat::RGBA, 8);

			const FString FilePath = FString::Printf(TEXT("C:/Temp/OutputColorBuffer%d.png"), i);

			FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(100), *FilePath);
		});
}

void FMyViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View,
	const FPostProcessingInputs& Inputs)
{
	check(IsInRenderingThread());

	const FIntRect PrimaryViewRect = UE::FXRenderingUtils::GetRawViewRectUnsafe(View);
	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

	TShaderMapRef<FMyShaderVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FMyShaderPS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	FMyShaderPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FMyShaderPS::FParameters>();

	PassParameters->MyColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);
	PassParameters->RenderTargets[0] = SceneColorRenderTarget.GetRenderTargetBinding();
	PassParameters->SourceTexture = SceneColor.Texture;
	PassParameters->SourceTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	const FScreenPassTextureViewport Viewport(SceneColorRenderTarget.Texture);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("MyShader"),
		PassParameters,
		ERDGPassFlags::Raster,
		[
			&View,
				VertexShader,
				PixelShader,
				PassParameters,
				Viewport
		](FRHICommandList& RHICmdList)
		{
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);

			const FIntRect OutputRect = Viewport.Rect;
			RHICmdList.SetViewport(OutputRect.Min.X, OutputRect.Min.Y, 0.0f, OutputRect.Max.X, OutputRect.Max.Y, 1.0f);

			RHICmdList.DrawPrimitive(0, 2, 1);

		});
}

void FMyViewExtension::PreInitViews_RenderThread(FRDGBuilder& GraphBuilder)
{
	if (bSave)
	{
		for (int32 i = 0; i < PooledBuffers.Num(); i++)
		{
			SavePooledBufferDataToImage(GraphBuilder, PooledBuffers[i], i);
		}
		bSave = false;

	}
}

void FMyViewExtension::PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily)
{
	if (!bCapture)
	{
		return;
	}
	check(IsInRenderingThread());

	FRDGTextureRef ViewFamilyTexture = TryCreateViewFamilyTexture(GraphBuilder, InViewFamily);
	if (!ViewFamilyTexture)
	{
		return;
	}

	TShaderMapRef<FCaptureComputeShader> ComputeShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));

	FCaptureComputeShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FCaptureComputeShader::FParameters>();

	OutputColorBuffers.Emplace(GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), 640 * 480), TEXT("OutputColorBuffer")));

	PassParameters->OutputColorBuffer = GraphBuilder.CreateUAV(OutputColorBuffers.Last());
	PassParameters->SourceTexture = ViewFamilyTexture;
	PassParameters->SourceTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("CaptureComputeShader"),
		ComputeShader,
		PassParameters,
		FIntVector(80, 60, 1)
	);

	PooledBuffers.Emplace();
	GraphBuilder.QueueBufferExtraction(OutputColorBuffers.Last(), &PooledBuffers.Last());

	bCapture = false;

}
