#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpenDriveTestCityBuilder.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;

UCLASS()
class OPENDRIVECITY_API AOpenDriveTestCityBuilder : public AActor
{
    GENERATED_BODY()

public:
    AOpenDriveTestCityBuilder();
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    UPROPERTY(EditAnywhere, Category="OpenDrive|City", meta=(ClampMin="1", ClampMax="20"))
    int32 GridX = 8;

    UPROPERTY(EditAnywhere, Category="OpenDrive|City", meta=(ClampMin="1", ClampMax="20"))
    int32 GridY = 8;

    UPROPERTY(EditAnywhere, Category="OpenDrive|City", meta=(ClampMin="500.0", ClampMax="10000.0"))
    float BlockSize = 2400.0f;

    UPROPERTY(EditAnywhere, Category="OpenDrive|City", meta=(ClampMin="200.0", ClampMax="2000.0"))
    float RoadWidth = 700.0f;

    UPROPERTY(EditAnywhere, Category="OpenDrive|City", meta=(ClampMin="0", ClampMax="400"))
    int32 MaxBuildings = 160;

private:
    void RebuildCity();

    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Roads;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Buildings;
};
