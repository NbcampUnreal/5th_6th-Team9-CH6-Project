#include "Weapons/MeleeWeaponBase/ClubWeapon.h"
#include "GameplayTagContainer.h"

AClubWeapon::AClubWeapon()
{
    WeaponTypeTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.Melee.Club"), false);

    // 프로파일 키는 InputTag.* 로 (WeaponBase.GiveAbility의 DynamicAbilityTags와 동일)
    LightAttackTag = FGameplayTag::RequestGameplayTag(TEXT("InputTag.Attack.Primary"), false);

    if (LightAttackTag.IsValid())
    {
        FWeaponAttackProfile LightProfile;
        LightProfile.Montage = nullptr; // BP에서 지정
        LightProfile.MontagePlayRate = 1.0f;
        LightProfile.DamageMultiplier = 1.0f;
        LightProfile.bHitEachActorOnce = true;
        LightProfile.bHitFirstTargetOnly = true;

        AttackProfiles.Add(LightAttackTag, LightProfile);
    }
}