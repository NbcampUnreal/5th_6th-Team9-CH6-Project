#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "GuideQuestSubsystem.generated.h"

class UDataTable;
class UUserWidget;
class APlayerController;
class APlayerCharacter_SB;
class USBWorldSaveGame;

UENUM(BlueprintType)
enum class EGuideQuestEventType : uint8
{
	CollectItem,
	BuildPlaced,
	CraftItem,
	SellItem,
	BuyItem,
	KillEnemy,
	KillBoss
};

UENUM(BlueprintType)
enum class EGuideQuestRewardType : uint8
{
	None,
	Gold,
	Item
};

USTRUCT(BlueprintType)
struct FGuideQuestMasterRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = "true"))
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGuideQuestRewardType RewardType = EGuideQuestRewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RewardItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RewardItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName NextQuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName GuideActorTag = NAME_None;
};

USTRUCT(BlueprintType)
struct FGuideQuestObjectiveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGuideQuestEventType EventType = EGuideQuestEventType::CollectItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
};

USTRUCT()
struct FGuideQuestRuntimeObjective
{
	GENERATED_BODY()

	UPROPERTY()
	EGuideQuestEventType EventType = EGuideQuestEventType::CollectItem;

	UPROPERTY()
	FName TargetId = NAME_None;

	UPROPERTY()
	int32 RequiredCount = 1;

	UPROPERTY()
	int32 CurrentCount = 0;

	UPROPERTY()
	FText Description;

	bool Matches(EGuideQuestEventType InEventType, FName InTargetId) const
	{
		if (EventType != InEventType)
		{
			return false;
		}

		return TargetId.IsNone() || TargetId == InTargetId;
	}
};

DECLARE_LOG_CATEGORY_EXTERN(LogGuideQuest, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuideQuestUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuideQuestCompleted, FName, CompletedQuestId);

UCLASS()
class STILLBOUND_API UGuideQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "GuideQuest")
	FOnGuideQuestUpdated OnGuideQuestUpdated;

	UPROPERTY(BlueprintAssignable, Category = "GuideQuest")
	FOnGuideQuestCompleted OnGuideQuestCompleted;

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void InitializeQuestTables(UDataTable* InMasterTable, UDataTable* InObjectiveTable);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	bool StartQuest(FName QuestId, bool bResetProgress = true);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void EnsureStarted(FName FirstQuestId);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportProgress(EGuideQuestEventType EventType, FName TargetId, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportCollectItem(FName ItemId, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportBuildPlaced(FName BuildingId, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportCraftItem(FName ItemId, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportSellItem(FName ItemId, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportBuyItem(FName ItemId, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportKillEnemy(FName EnemyId = NAME_None, int32 DeltaCount = 1);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void ReportKillBoss(FName BossId = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "GuideQuest")
	void AttachWidget(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintPure, Category = "GuideQuest")
	bool HasActiveQuest() const;

	UFUNCTION(BlueprintPure, Category = "GuideQuest")
	FText GetActiveQuestTitleText() const;

	UFUNCTION(BlueprintPure, Category = "GuideQuest")
	FText GetActiveQuestSummaryText() const;

	UFUNCTION(BlueprintPure, Category = "GuideQuest")
	FText BuildObjectiveProgressText() const;

	UFUNCTION(BlueprintPure, Category = "GuideQuest")
	FText BuildRewardText() const;

	void ExportToSaveGame(USBWorldSaveGame* SaveGameObject) const;
	void ImportFromSaveGame(const USBWorldSaveGame* SaveGameObject);

private:
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> GuideQuestMasterTable = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> GuideQuestObjectiveTable = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<UUserWidget> ActiveWidget;

	UPROPERTY()
	FName ActiveQuestId = NAME_None;

	// DataTable row 포인터는 UPROPERTY 불가, 런타임 캐시로만 사용
	const FGuideQuestMasterRow* ActiveQuestRow = nullptr;

	UPROPERTY()
	TArray<FGuideQuestRuntimeObjective> ActiveObjectives;

	UPROPERTY()
	TArray<FName> CompletedQuestIds;

	bool BuildObjectivesForQuest(FName QuestId, TArray<FGuideQuestRuntimeObjective>& OutObjectives) const;
	bool IsQuestComplete() const;
	void CompleteActiveQuest();
	APlayerCharacter_SB* GetPlayerCharacter() const;
	bool GiveReward(APlayerCharacter_SB* PlayerCharacter);
};