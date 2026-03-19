//Altaractor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "Altaractor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class ABossArenaWall;
class AEnemyCharacter;
class UWB_AltarUI;


UCLASS()
class STILLBOUND_API AAltaractor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	

	AAltaractor();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Altar|Components")
	TObjectPtr<UStaticMeshComponent> AltarMesh;

	//제단 슬롯에 요구되는 아이템ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Altar|Components")
	FName RequiredItemID = FName("700003");

	//필요한 슬롯 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Altar|Config")
	int32 RequiredSlotCount = 3;

	//소환할 보스 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Altar|Components")
	TSubclassOf<ACharacter> BossClass;

	//보스 스폰 제단 기준 위치 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Altar|Config")
	FVector BossSpawnOffset = FVector(300.f, 0.f, 0.f);

	//블로킹 벽 반지름
	UPROPERTY(EditDefaultsOnly, Category = "Altar|Config")
	float ArenaRadius = 1500.f;

	//블로킹 벽 높이
	UPROPERTY(EditDefaultsOnly, Category = "Altar|Config")
	float ArenaWallHeight = 500.f;

	//벽 분할 수 
	UPROPERTY(EditDefaultsOnly, Category = "Altar|Config")
	int32 WallSegments = 12;

	//아레나 벽 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Altar|Config")
	TSubclassOf<ABossArenaWall> ArenaWallClass;

	//제단 UI위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Altar|UI")
	TSubclassOf<UUserWidget> AltarUIClass;

	//==== 제단 상태

	//현재 슬롯에 들어간 석판 수
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Altar|State")
	int32 FilledSlotCount = 0;

	//이미 소환 됐으면 중복 방지
	bool bActivated = false;

	//UI를 열고 있는 플레이어
	UPROPERTY()
	TObjectPtr<APlayerCharacter_SB> InteractingPlayer;

	//열려있는  UI 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> AltarUIInstance;

	//=====상호작용
public:	
	virtual void BeginFocus_Implementation() override;
	virtual void EndFocus_Implementation() override;
	virtual void BeginInteract_Implementation() override;
	virtual void EndInteract_Implementation() override;
	virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;
	virtual FInteractableData GetInteractableData_Implementation() override;
	virtual float GetInteractionDistance_Implementation() override;
	
	//=== UI에서 호출
	//UI에서 석판 넣기 버튼 클릭 시 호출. 인벤토리에서 1개 소비 해서 슬롯 채움.
	UFUNCTION(BlueprintCallable, Category = "Altar")
	bool TryInsertTablet();

	// 현재 채워진 슬롯 수 반환 
	UFUNCTION(BlueprintPure, Category = "Altar")
	int32 GetFilledSlotCount() const { return FilledSlotCount; }

	//요구 슬롯 수 반환
	UFUNCTION(BlueprintPure, Category = "Altar")
	int32 GetRequiredSlotCount() const { return RequiredSlotCount; }

	//플레이어가 멀어지거나 ESC키로 UI닫기
	UFUNCTION(BlueprintCallable, Category = "Altar")
	void CloseAltarUI();
	
private:

	//3개 다 채웟을 때 보스 소환 + 벽 생성 + 제단 소멸
	void ActivateAltar();

	//아레나 블로킹 벽 원형으로 배치
	void SpawnArenaWalls();


	UPROPERTY(EditAnywhere, Category = "Altar|Interactable")
	FInteractableData InteractableData;

	UPROPERTY()
	TArray<TObjectPtr<ABossArenaWall>> SpawnedWalls;

	// 제단 리스폰용 , 스폰 위치/회전 저장
	FVector AltarSpawnLocation;
	FRotator AltarSpawnRotation;

	// 보스 사망 감지용
	UPROPERTY()
	TObjectPtr<ACharacter> SpawnedBoss;

	// 보스 OnDestroyed 콜백
	UFUNCTION()
	void OnBossDestroyed(AActor* DestroyedActor);

	// 아레나 종료 처리
	void EndArena();

	UPROPERTY()
	TSubclassOf<AAltaractor> AltarBPClass;

	// 리스폰용 스케일 저장
	FVector AltarSpawnScale;
};
