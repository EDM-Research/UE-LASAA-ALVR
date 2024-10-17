// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AlignmentComponent.generated.h"

class AAnchor;

UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASAA_API UAlignmentComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:	
	UAlignmentComponent();

	// class to spawn external anchors
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	TSubclassOf<AActor> m_extAnchorClass;
	// class to spawn internal anchors
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	TSubclassOf<AAnchor> m_intAnchorClass;

	// is executed every frame if tick is enabled
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// changes the alignment mode: weighted and static
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	FString changeMode();

	// aligns the cloud once
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	bool align();

	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	// the alignment mode: 0 - weighted, 1 - static
	int mode;

private:
	// indicates whether the anchors are localized
	bool localizedAnchors = false;
	// anchors to load from memory
	int numAnchorsToLoad = -1;
};
