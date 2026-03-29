#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Ticker.h"
#include "SBGameInstance.generated.h"

class UUserWidget;
class UUW_SBLoadingScreen;

UCLASS()
class STILLBOUND_API USBGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable)
    void RequestLoadingScreenOnce();

    // 가이드 퀘스트 마스터 테이블
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GuideQuest")
    TObjectPtr<UDataTable> GuideQuestMasterTable = nullptr;

    // 가이드 퀘스트 목표 테이블
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GuideQuest")
    TObjectPtr<UDataTable> GuideQuestObjectiveTable = nullptr;

    // 기존에는 마스터/목표 테이블만 사용
    // TObjectPtr<UDataTable> GuideQuestRewardTable = nullptr;

    // 가이드 퀘스트 보상 테이블
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GuideQuest")
    TObjectPtr<UDataTable> GuideQuestRewardTable = nullptr;

    // 첫 시작 퀘스트 RowName
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GuideQuest")
    FName FirstGuideQuestId = NAME_None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GuideQuest")
    TSubclassOf<UUserWidget> GuideQuestWidgetClass;
    //===============
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
    TSubclassOf<UUserWidget> LoadingScreenClass;

private:
    UFUNCTION()
    void BeginLoadingScreen(const FString& MapName);

    UFUNCTION()
    void EndLoadingScreen(UWorld* LoadedWorld);

    bool TickLoadingProgress(float DeltaTime);

private:
    UPROPERTY()
    TObjectPtr<UUW_SBLoadingScreen> ActiveLoadingWidget;

    FString LoadingMapName;
    FTSTicker::FDelegateHandle LoadingTickerHandle;

    bool bSkipFirstLoadScreen = true;
    bool bShowLoadingScreenOnce = false;
    float FakeLoadingProgress = 0.f;
};