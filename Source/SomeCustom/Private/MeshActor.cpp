#include "MeshActor.h"

#include "MyShader.h"
#include "Interfaces/IPluginManager.h"

AMeshActor::AMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	RootComponent = MeshComponent;
}

void AMeshActor::BeginPlay()
{
	Super::BeginPlay();

	if(!RenderTarget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Render Target set in MeshActor!"));
		this->SetActorTickEnabled(false);
	}
	else if(!MeshComponent)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Mesh Component set in MeshActor!"));
		this->SetActorTickEnabled(false);
	}
}

void AMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<FVector3f> Vertices = GetVertices(this);
	UTextureRenderTarget2D* RenderTargetParam = RenderTarget;

	TShaderMapRef<FMyComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	ENQUEUE_RENDER_COMMAND(ComputeShader)(
		[ComputeShader,
		RenderTargetParam,
		Vertices](FRHICommandListImmediate& RHICmdList)
		{
			ComputeShader->BuildAndExecuteGraph(RHICmdList, RenderTargetParam, Vertices);
		});
}

TArray<FVector3f> AMeshActor::GetVertices(AActor* Actor)
{
	TArray<FVector3f> MeshVertices;

	TArray<UStaticMeshComponent*> StaticMeshComponents = TArray<UStaticMeshComponent*>();
	Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	UStaticMesh* StaticMesh = StaticMeshComponents[0]->GetStaticMesh();

	// Check if the static mesh is null
	if (!StaticMesh)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("StaticMesh[0] was null for actor: ")).Append(Actor->GetName()));
		return MeshVertices;
	}

	// Check if this static mesh has a LOD 
	if (StaticMesh->GetRenderData()->LODResources.Num() <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Each mesh must have the supplied LOD To Use"));
		return MeshVertices;
	}

	FPositionVertexBuffer* VertexBuffer = &StaticMesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
	for (uint32 VertIdx = 0; VertIdx < VertexBuffer->GetNumVertices(); VertIdx++)
	{
		// Get this vertex
		FVector3f VertexLS = VertexBuffer->VertexPosition(VertIdx);

		// Transform from local to world space
		FVector3f VertexWS = static_cast<FVector3f>(Actor->GetTransform().TransformPosition(static_cast<FVector>(VertexLS)));

		// Add it to the array we'll return
		MeshVertices.Add(VertexWS);	// NOTE: .Add can be pretty slow!
	}

	return MeshVertices;
}

