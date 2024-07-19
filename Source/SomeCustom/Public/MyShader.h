#pragma once

#include "CoreMinimal.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "ShaderParameterStruct.h"


class FMyComputeShader : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FMyComputeShader)
	SHADER_USE_PARAMETER_STRUCT(FMyComputeShader, FGlobalShader)
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, Vertices)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	void BuildAndExecuteGraph(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, TArray<FVector3f> InVertices);
};


class FCaptureComputeShader : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCaptureComputeShader)
	SHADER_USE_PARAMETER_STRUCT(FCaptureComputeShader, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SourceTextureSampler)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint>, OutputColorBuffer)
	END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};


BEGIN_SHADER_PARAMETER_STRUCT(FMyShaderParameters, )
	SHADER_PARAMETER(FVector4f, MyColor)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, SourceTextureSampler)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

class FMyShaderVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FMyShaderVS);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

class FMyShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FMyShaderPS);
	SHADER_USE_PARAMETER_STRUCT(FMyShaderPS, FGlobalShader);

	using FParameters = FMyShaderParameters;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};