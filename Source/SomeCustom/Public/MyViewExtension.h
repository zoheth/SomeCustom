#pragma once

#include "SceneViewExtension.h"

class FMyViewExtension : public FSceneViewExtensionBase
{
public:
	explicit FMyViewExtension(const FAutoRegister& AutoRegister);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};

	virtual void PreInitViews_RenderThread(FRDGBuilder& GraphBuilder) override;;

	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;

	virtual void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;

	void Capture() { this->bCapture = true; }

	void Save() { this->bSave = true; }
private:
	TArray< FRDGBufferRef> OutputColorBuffers;
	TArray< TRefCountPtr<FRDGPooledBuffer>> PooledBuffers;


	bool bCapture = false;

	bool bSave = false;
};
