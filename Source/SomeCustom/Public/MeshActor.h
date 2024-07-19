#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshActor.generated.h"

UCLASS()
class AMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AMeshActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* RenderTarget;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	TArray<FVector3f> GetVertices(AActor* Actor);
};