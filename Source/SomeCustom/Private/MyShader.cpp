#include "MyShader.h"

#include "RenderGraph.h"
#include "RenderTargetPool.h"
#include "Engine/TextureRenderTarget2D.h"

IMPLEMENT_GLOBAL_SHADER(FMyComputeShader, "/Shaders/Private/ComputeShader.usf", "MainCompute", SF_Compute);

IMPLEMENT_GLOBAL_SHADER(FCaptureComputeShader, "/Shaders/Private/CaptureShader.usf", "MainCS", SF_Compute);

IMPLEMENT_GLOBAL_SHADER(FMyShaderVS, "/Shaders/Private/TestShader.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FMyShaderPS, "/Shaders/Private/TestShader.usf", "MainPS", SF_Pixel);

void FMyComputeShader::BuildAndExecuteGraph(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, TArray<FVector3f> InVertices)
{
	check(IsInRenderingThread());

	FRDGBuilder GraphBuilder(RHICmdList);

	FParameters* PassParameters = GraphBuilder.AllocParameters<FParameters>();

	FRDGBufferRef VerticesBuffer = CreateStructuredBuffer(
		GraphBuilder,
		TEXT("VerticesBuffer"),
		sizeof(FVector3f),
		InVertices.Num(),
		InVertices.GetData(),
		sizeof(FVector3f) * InVertices.Num());

	FRDGBufferSRVRef VerticesSRV = GraphBuilder.CreateSRV(VerticesBuffer, PF_R32_UINT);
	PassParameters->Vertices = VerticesSRV;

	FRDGTextureDesc OutTextureDesc = FRDGTextureDesc::Create2D(
		FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY),
		PF_FloatRGBA,
		FClearValueBinding(),
		TexCreate_UAV,
		1,
		1
	);
	FRDGTextureRef OutTextureRef = GraphBuilder.CreateTexture(OutTextureDesc, TEXT("OutputTexture"));
	FRDGTextureUAVDesc OutTextureUAVDesc(OutTextureRef);
	PassParameters->OutputTexture = GraphBuilder.CreateUAV(OutTextureUAVDesc);

	TShaderMapRef<FMyComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("My Compute Pass"),
		ComputeShader,
		PassParameters,
		FIntVector(32, 32, 1)
	);

	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	GraphBuilder.QueueTextureExtraction(OutTextureRef, &PooledRenderTarget);

	GraphBuilder.Execute();

	RHICmdList.CopyTexture(
		PooledRenderTarget.GetReference()->GetRHI(),
		RenderTarget->GetRenderTargetResource()->GetTextureRHI(),
		FRHICopyTextureInfo()
	);
}
