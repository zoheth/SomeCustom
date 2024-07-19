#include "ViewExtensionActor.h"

#include "MyViewExtension.h"


void AViewExtensionActor::BeginPlay()
{
	PrimaryActorTick.bCanEverTick = true;
	Super::BeginPlay();
	PostProcessSceneViewExtension = FSceneViewExtensions::NewExtension<FMyViewExtension>();
	
}

void AViewExtensionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Counter > 100)
		return;
	if (Counter == 100)
		PostProcessSceneViewExtension->Save();
	PostProcessSceneViewExtension->Capture();
	Counter++;
	
}



