#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyViewExtension.h"
#include "ViewExtensionActor.generated.h"

UCLASS()
class AViewExtensionActor : public AActor
{
	GENERATED_BODY()

public:

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
private:
	TSharedPtr< class FMyViewExtension, ESPMode::ThreadSafe > PostProcessSceneViewExtension;

	uint32_t Counter = 0;
};