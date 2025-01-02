#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ActorComponent.h"
#include "MyPrimitive.generated.h"

class FPrimitiveSceneProxy;

UCLASS(Blueprintable)
class SOMECUSTOM_API UMyPrimitiveComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;

	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override
	{
		if(Material)
		{
			Material->ConditionalPostLoad();
		}
		return Material.Get();
	}

	virtual  void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) override;

protected:
	UFUNCTION()
	void OnMaterialChanged();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnMaterialChanged)
	TObjectPtr<UMaterialInterface> Material;
	
};
