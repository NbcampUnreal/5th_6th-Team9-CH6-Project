#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "SBGameUserSettings.generated.h"

class USoundControlBusMix;
class USoundControlBus;

UCLASS(Config = GameUserSettings, DefaultConfig, BlueprintType, Blueprintable)
class STILLBOUND_API USBGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BGMVolume = 1.0f;	

	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	// 오디오 모듈레이션 에셋들 (하드코딩 X, ini/디폴트로 지정)
	UPROPERTY(Config, EditAnywhere, Category = "Audio|Modulation")
	TSoftObjectPtr<USoundControlBusMix> UserBusMix;

	UPROPERTY(Config, EditAnywhere, Category = "Audio|Modulation")
	TSoftObjectPtr<USoundControlBus> MasterBus;

	UPROPERTY(Config, EditAnywhere, Category = "Audio|Modulation")
	TSoftObjectPtr<USoundControlBus> BGMBus;

	UPROPERTY(Config, EditAnywhere, Category = "Audio|Modulation")
	TSoftObjectPtr<USoundControlBus> SFXBus;

	UPROPERTY(Config, EditAnywhere, Category = "Audio|Modulation", meta = (ClampMin = "0.0"))
	float FadeTimeSec = 0.05f;

	UFUNCTION(BlueprintCallable)
	static USBGameUserSettings* Get();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	void ApplyAudioSettings(UObject* WorldContextObject);
};