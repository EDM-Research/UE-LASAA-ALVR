// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Anchor.generated.h"

class USceneComponent;

// this struct is used to store the data in a file
USTRUCT()
struct FAnchorStruct
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTransform> anchorTransforms;
	UPROPERTY()
	TArray<FString> anchorUuids;

	FTransform getTransform(FString id)
	{
		int idx;
		bool found = anchorUuids.Find(id, idx);
		if (found)
		{
			return anchorTransforms[idx];
		}
			return FTransform::Identity;
	}
};


// TO CREATE AN ANCHOR WITH ANOTHER SDK:
// Replace all OculusSDK calls with the appropiate alternative
UCLASS()
class LASAA_API AAnchor : public AActor
{

	GENERATED_BODY()

private:
	
	
	USceneComponent* sceneComponent;
	// the actor that is the 3D correspondence of this anchor
	AActor* extPair;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// file path to store anchors to
	inline static FString filePath =  "anchors.json";
	// struct that is stored
	inline static FAnchorStruct anchorStorage = FAnchorStruct();
	// handle to all anchors
	inline static TArray<AAnchor*> allAnchors = TArray<AAnchor*>();

	// external camera pose
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	FTransform gtExtPose;

	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	FString uuid;

	AAnchor();
	
	// erases the spatial anchor
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	void erase();
	// load anchors from storage
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	static void loadAnchors(TArray<FString>& uuids, TArray<FTransform>& transforms);
	// delete all anchors
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	static void resetAnchors();	
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	void saveAnchor();

	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	FString getUuid();
	// gets location of external anchor (from extPair)
	FVector getCurrentExtLocation();
	// gets pose of external anchor (from extPair)
	FTransform getCurrentExtPose();

	// sets extpair, and adjusts the pose for the current calibration that is used
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	void setExtPairAndCalibrationOffset(AActor* extActor);
	
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	AActor* getExtPair();
	
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	static int getNumAnchors();

	// sets the calibration parameters
	static void setCalibrationOffset(const FTransform& calibration);

	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	static double getAvgPairDistance();

	// Convert OSC message to uuids and transforms
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	static void TransformOSCALVRMessage(TArray<FString>& uuids, TArray<FTransform>& transforms, const TArray<float>& oscMessageFloats, const TArray<FString>& oscMessageStrings);
	
private:

	// erase this anchor from storage
	void eraseFromList();
	// add this anchor to storage
	void addToList();
	
	void setGtExtPose(FTransform extTrans);
	void setUuid(FString id);
	
	static void writeToJson();
	static void readFromJson();

	inline static FTransform cam2xr = FTransform::Identity;

	// changes the pose of this anchor to adjust for the current calibration 
	void addCalibrationOffset();
};
