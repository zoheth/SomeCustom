// Fill out your copyright notice in the Description page of Project Settings.


#include "BarChartComponent.h"


// Sets default values for this component's properties
UBarChartComponent::UBarChartComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBarChartComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UBarChartComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBarChartComponent::GenerateBarChart(float Width, float Height, float Depth)
{
	FDynamicMesh3 Mesh;

	FVector3d Vertices[8];

	// 定义八个顶点的位置，根据Width, Height, Depth计算
	Vertices[0] = FVector3d(0, 0, 0);
	Vertices[1] = FVector3d(Width, 0, 0);
	Vertices[2] = FVector3d(Width, Height, 0);
	Vertices[3] = FVector3d(0, Height, 0);
	Vertices[4] = FVector3d(0, 0, Depth);
	Vertices[5] = FVector3d(Width, 0, Depth);
	Vertices[6] = FVector3d(Width, Height, Depth);
	Vertices[7] = FVector3d(0, Height, Depth);

	for (int i = 0; i < 8; ++i)
	{
		Mesh.AppendVertex(Vertices[i]);
	}

	// 定义柱体的面（Triangles）
	Mesh.AppendTriangle(0, 1, 2);
	Mesh.AppendTriangle(0, 2, 3);
	Mesh.AppendTriangle(4, 5, 6);
	Mesh.AppendTriangle(4, 6, 7);
	Mesh.AppendTriangle(0, 4, 7);
	Mesh.AppendTriangle(0, 7, 3);
	Mesh.AppendTriangle(1, 5, 6);
	Mesh.AppendTriangle(1, 6, 2);
	Mesh.AppendTriangle(0, 1, 5);
	Mesh.AppendTriangle(0, 5, 4);
	Mesh.AppendTriangle(2, 6, 7);
	Mesh.AppendTriangle(2, 7, 3);

	
}

