#include "OpenDriveTestCityBuilder.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOpenDriveTestCityBuilder::AOpenDriveTestCityBuilder()
{
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Roads = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Roads"));
    Roads->SetupAttachment(SceneRoot);
    Buildings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Buildings"));
    Buildings->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (PlaneMesh.Succeeded()) Roads->SetStaticMesh(PlaneMesh.Object);
    if (CubeMesh.Succeeded()) Buildings->SetStaticMesh(CubeMesh.Object);
}

void AOpenDriveTestCityBuilder::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RebuildCity();
}

void AOpenDriveTestCityBuilder::RebuildCity()
{
    Roads->ClearInstances();
    Buildings->ClearInstances();

    const int32 SafeGridX = FMath::Clamp(GridX, 1, 20);
    const int32 SafeGridY = FMath::Clamp(GridY, 1, 20);
    const int32 SafeMaxBuildings = FMath::Clamp(MaxBuildings, 0, 400);
    const float SafeBlockSize = FMath::Clamp(BlockSize, 500.0f, 10000.0f);
    const float SafeRoadWidth = FMath::Clamp(RoadWidth, 200.0f, 2000.0f);

    int32 BuildingCount = 0;
    for (int32 X = 0; X < SafeGridX; ++X)
    {
        for (int32 Y = 0; Y < SafeGridY; ++Y)
        {
            const FVector CellLocation(X * SafeBlockSize, Y * SafeBlockSize, 0.0f);
            const float PlaneScale = SafeBlockSize / 100.0f;
            Roads->AddInstance(FTransform(FRotator::ZeroRotator, CellLocation, FVector(PlaneScale, PlaneScale, 1.0f)));

            if (BuildingCount >= SafeMaxBuildings)
            {
                continue;
            }
            const int32 HeightBand = (X * 31 + Y * 17) % 7;
            const float BuildingHeight = 700.0f + HeightBand * 180.0f;
            const float Footprint = FMath::Max(300.0f, SafeBlockSize - SafeRoadWidth * 1.5f);
            const FVector BuildingLocation = CellLocation + FVector(0.0f, 0.0f, BuildingHeight * 0.5f);
            const FVector BuildingScale(Footprint / 100.0f, Footprint / 100.0f, BuildingHeight / 100.0f);
            Buildings->AddInstance(FTransform(FRotator::ZeroRotator, BuildingLocation, BuildingScale));
            ++BuildingCount;
        }
    }
}
