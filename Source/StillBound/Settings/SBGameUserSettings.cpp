#include "Settings/SBGameUserSettings.h"

#include "AudioModulationStatics.h"   // UAudioModulationStatics :contentReference[oaicite:4]{index=4}
#include "AudioModulation.h"          // FAudioModulationManager, UpdateMix 등 :contentReference[oaicite:5]{index=5}
#include "SoundControlBus.h"          // USoundControlBus :contentReference[oaicite:6]{index=6}
#include "SoundControlBusMix.h"       // USoundControlBusMix, FSoundControlBusMixStage :contentReference[oaicite:7]{index=7}

USBGameUserSettings* USBGameUserSettings::Get()
{
	return Cast<USBGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

static float Clamp01(float V) { return FMath::Clamp(V, 0.f, 1.f); }

void USBGameUserSettings::ApplyAudioSettings(UObject* WorldContextObject)
{
	if (!WorldContextObject) return;

	UWorld* World = UAudioModulationStatics::GetAudioWorld(WorldContextObject);
	if (!World) return;

	AudioModulation::FAudioModulationManager* Mod = UAudioModulationStatics::GetModulation(World);
	if (!Mod) return;

	USoundControlBusMix* Mix = UserBusMix.LoadSynchronous();
	if (!Mix) return;

	// 믹스 활성화(스택에 올림)
	if (!Mod->IsBusMixActive(*Mix))
	{
		Mod->ActivateBusMix(*Mix);
	}

	const float Fade = FMath::Max(0.f, FadeTimeSec);

	auto AddStage = [&](TArray<FSoundControlBusMixStage>& Stages, const TSoftObjectPtr<USoundControlBus>& BusPtr, float Target)
		{
			USoundControlBus* Bus = BusPtr.LoadSynchronous();
			if (!Bus) return;

			FSoundControlBusMixStage Stage;
			Stage.Bus = Bus;
			Stage.Value.TargetValue = Clamp01(Target);
			Stage.Value.AttackTime = Fade;
			Stage.Value.ReleaseTime = Fade;
			Stages.Add(Stage);
		};

	TArray<FSoundControlBusMixStage> Stages;
	Stages.Reserve(3);

	AddStage(Stages, MasterBus, MasterVolume);
	AddStage(Stages, BGMBus, BGMVolume);
	AddStage(Stages, SFXBus, SFXVolume);

	// 런타임 인스턴스 업데이트 (에셋 디폴트는 건드리지 않음)
	Mod->UpdateMix(
		Stages,
		*Mix,
		/*bInUpdateObject*/ false,
		/*InFadeTime*/ Fade,
		/*Duration*/ Mix->Duration,
		/*bRetriggerOnActivation*/ false
	);
}