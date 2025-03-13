#include "Anchor.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"

// Sets default values
AAnchor::AAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = sceneComponent;
}

// Called when the game starts or when spawned
void AAnchor::BeginPlay()
{
	Super::BeginPlay();
	allAnchors.Add(this);

}

void AAnchor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	eraseFromList();
	allAnchors.Remove(this);
	if(extPair != nullptr)
	{
		extPair->Destroy();
	}
}

void AAnchor::eraseFromList()
{
	int idx;
	bool found = anchorStorage.anchorUuids.Find(this->uuid, idx);
	if (found)
	{
		anchorStorage.anchorUuids.RemoveAt(idx);
		anchorStorage.anchorTransforms.RemoveAt(idx);
	}
}

void AAnchor::addToList()
{
	if(anchorStorage.anchorUuids.Num() >= 64)
		return;
	anchorStorage.anchorUuids.Add(this->uuid);
	anchorStorage.anchorTransforms.Add(this->gtExtPose);
}

void AAnchor::writeToJson()
{
	TSharedRef<FJsonObject> outJsonObject = MakeShareable(new FJsonObject);
	FJsonObjectConverter::UStructToJsonObject(FAnchorStruct::StaticStruct(), &anchorStorage, outJsonObject,0,0);

	FString outputString;
	TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&outputString);
	FJsonSerializer::Serialize(outJsonObject, writer);
	FFileHelper::SaveStringToFile(outputString, *(FPaths::ProjectDir() + filePath));
	UE_LOG(LogTemp, Display, TEXT("Wrote %d anchors to %s"), anchorStorage.anchorUuids.Num(), *filePath);
}

void AAnchor::readFromJson()
{
	FString out;
	FFileHelper::LoadFileToString(out, *(FPaths::ProjectDir() + filePath));
	
	TSharedPtr<FJsonObject> jObj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(out);
	if(!FJsonSerializer::Deserialize(reader, jObj))
	{
		UE_LOG(LogTemp, Display, TEXT("Can not create json from this string"));
		return;
	}
	FJsonObjectConverter::JsonObjectToUStruct(jObj.ToSharedRef(), FAnchorStruct::StaticStruct(), &anchorStorage, 0,0);
}

//TODO Send osc to also remove in ALVR
void AAnchor::erase()
{
    this->eraseFromList();
    this->writeToJson();
    this->Destroy();
    return;
}

void AAnchor::loadAnchors(TArray<FString>& uuids, TArray<FTransform>& transforms)
{
	UE_LOG(LogTemp, Display, TEXT("Loading anchors from %s"), *filePath);

	// read anchor uuids and external poses from JSON file into anchorStorage
	readFromJson();
	
	uuids = anchorStorage.anchorUuids;
	transforms = anchorStorage.anchorTransforms;
	
	return;
}

void AAnchor::saveAnchor()
{
	UE_LOG(LogTemp, Display, TEXT("Saving anchor to JSON"), *filePath);

	this->addToList();
	writeToJson();
	
	return;
}

void AAnchor::resetAnchors()
{
	anchorStorage.anchorTransforms.Empty();
	anchorStorage.anchorUuids.Empty();

	TArray<AAnchor*> copy = allAnchors;
	for(AAnchor* anchor: copy)
	{
		if(anchor)
			anchor->Destroy();
	}
	writeToJson();
}

void AAnchor::TransformOSCALVRMessage(TArray<FString>& uuids, TArray<FTransform>& transforms, const TArray<float>& oscMessageFloats, const TArray<FString>& oscMessageStrings){
	// oscMessage is a list of floats, where every first float represents the uuid, the next 3 floats represent the location, and the next 4 floats represent the quaternion

	// Parse the oscMessage into uuids and transforms
	for(int i = 0; i < oscMessageFloats.Num(); i+=7)
	{
		FVector location = FVector(oscMessageFloats[i], oscMessageFloats[i+1], oscMessageFloats[i+2]);
		FQuat quaternion = FQuat(oscMessageFloats[i+3], oscMessageFloats[i+4], oscMessageFloats[i+5], oscMessageFloats[i+6]);

		// Convert from openxr to Unreal coordinate system
		FVector transformedLocation = FVector(-location.Z, location.X, location.Y);

		FTransform transform = FTransform(quaternion, transformedLocation);

		// Convert from meters to centimeters
		transform.SetLocation(transform.GetLocation() * 100);

		transforms.Add(transform);
	}

	for (int i = 0; i < oscMessageStrings.Num(); i++)
	{
		FString uuid = oscMessageStrings[i];
		uuids.Add(uuid);
	}

}

void AAnchor::setUuid(FString id)
{
	this->uuid = id;
}

void AAnchor::addCalibrationOffset()
{
	FTransform currentPose = this->gtExtPose;
	FTransform newPose = cam2xr.Inverse() * currentPose;
	extPair->SetActorRelativeTransform(newPose);
}

int AAnchor::getNumAnchors()
{
	return allAnchors.Num();
}

void AAnchor::setCalibrationOffset(const FTransform& calibration)
{
	AAnchor::cam2xr = calibration;
	for(AAnchor* anchor: AAnchor::allAnchors)
	{
		anchor->addCalibrationOffset();
	}
}

double AAnchor::getAvgPairDistance()
{
	double distance = 0;
	for (AAnchor* anchor: AAnchor::allAnchors)
	{
		double localDistance = FVector::Distance(anchor->getCurrentExtLocation(), anchor->GetActorLocation());
		distance += localDistance;
	}
	return distance/allAnchors.Num();
}

FString AAnchor::getUuid()
{
	return uuid;
}

void AAnchor::setGtExtPose(FTransform extTrans)
{
	this->gtExtPose = extTrans;
}

FVector AAnchor::getCurrentExtLocation()
{
	if(extPair != nullptr)
		return extPair->GetActorLocation();
	return FVector::Zero();
}

FTransform AAnchor::getCurrentExtPose()
{
	if(extPair != nullptr)
		return extPair->GetActorTransform();
	return FTransform::Identity;
}

void AAnchor::setExtPairAndCalibrationOffset(AActor* extActor)
{
	this->extPair = extActor;
	addCalibrationOffset();
}

AActor* AAnchor::getExtPair()
{
	return extPair;
}

