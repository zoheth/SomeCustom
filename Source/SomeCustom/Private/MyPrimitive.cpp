#include "MyPrimitive.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "Engine/CollisionProfile.h"
#include "SceneInterface.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "MaterialDomain.h"
#include "UObject/UObjectIterator.h"
#include "StaticMeshResources.h"
#include "RHICommandList.h"
#include "Rendering/StaticLightingSystemInterface.h"

class FMyPrimitiveSceneProxy : public FPrimitiveSceneProxy
{
public:
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FMyPrimitiveSceneProxy(UMyPrimitiveComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, VertexFactory(GetScene().GetFeatureLevel(), "FMyPrimitiveSceneProxy")
	{
		bWillEverBeLit = false;

		TArray<FDynamicMeshVertex> Vertices;
		TArray<uint32> Indices;

		Vertices.SetNum(3);
		Vertices[0].Position = FVector3f(0, 0, 0);
		Vertices[1].Position = FVector3f(0, 100, 0);
		Vertices[2].Position = FVector3f(100, 0, 0);

		Vertices[0].Color = FColor(1, 0, 0);
		Vertices[1].Color = FColor(0, 1, 0);
		Vertices[2].Color = FColor(0, 0, 1);

		FVector3f Normal = FVector3f(0, 0, 1); 

		Vertices[0].TangentZ = Normal;
		Vertices[1].TangentZ = Normal;
		Vertices[2].TangentZ = Normal;

		Indices.Add(0);
		Indices.Add(1);
		Indices.Add(2);

		Material = InComponent->GetMaterial(0);
		if (Material == nullptr)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
		IndexBuffer.Indices = Indices;

		VertexBuffers.InitFromDynamicVertex(&VertexFactory, Vertices);
		BeginInitResource(&IndexBuffer);
	}

	virtual ~FMyPrimitiveSceneProxy()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_CustomSceneProxy_GetDynamicMeshElements);

		FMatrix EffectiveLocalToWorld = GetLocalToWorld();

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];

				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = false;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = Material->GetRenderProxy();

				FMatrix ScaleMatrix = FScaleMatrix(FVector(1.0f)).RemoveTranslation();
				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();

				DynamicPrimitiveUniformBuffer.Set(
					Collector.GetRHICommandList(),
					EffectiveLocalToWorld,
					EffectiveLocalToWorld,
					GetBounds(),
					GetLocalBounds(),
					true,
					false,
					AlwaysHasVelocity()
				);
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Mesh.CastShadow = true;
				Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && (View->Family->EngineShowFlags.BillboardSprites);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
		return Result;
	}

	virtual void OnTransformChanged(FRHICommandListBase& RHICmdList) override
	{
		Origin = GetLocalToWorld().GetOrigin();
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}


private:
	UMaterialInterface* Material;

	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;

	FLocalVertexFactory VertexFactory;

	FVector Origin;
};

FPrimitiveSceneProxy* UMyPrimitiveComponent::CreateSceneProxy()
{
	return new FMyPrimitiveSceneProxy(this);
}

FBoxSphereBounds UMyPrimitiveComponent::CalcBounds(const FTransform& LocalToWorld) const
{

	return FBoxSphereBounds(FBox(FVector(0, 0, 0), FVector(100, 100, 1))).TransformBy(LocalToWorld);
}

void UMyPrimitiveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UMyPrimitiveComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{

	if(Material)
	{
		OutMaterials.Add(Material);
	}
}

void UMyPrimitiveComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* InMaterial)
{
	if (UMaterialInterface* PreviousMaterial = GetMaterial(ElementIndex))
	{
		PreviousMaterial->OnRemovedAsOverride(this); // Notify the previous material that it has been replaced
	}

	if (InMaterial)
	{
		InMaterial->OnAssignedAsOverride(this); // Notify the new material that it has been assigned
	}

	// Mark the render state as dirty to trigger a re-render
	MarkRenderStateDirty();

	// Update physical materials if necessary
	FBodyInstance* BodyInst = GetBodyInstance();
	if (BodyInst && BodyInst->IsValidBodyInstance())
	{
		BodyInst->UpdatePhysicalMaterials();
	}

#if WITH_EDITOR
	// In the editor, we might need to update static lighting
	if (!IsCompiling())
	{
		FStaticLightingSystemInterface::OnPrimitiveComponentUnregistered.Broadcast(this);
		if (HasValidSettingsForStaticLighting(false))
		{
			FStaticLightingSystemInterface::OnPrimitiveComponentRegistered.Broadcast(this);
		}
	}
#endif
}

void UMyPrimitiveComponent::OnMaterialChanged()
{
	SetMaterial(0, Material);
}
