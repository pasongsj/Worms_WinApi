#pragma once
#include "Weapon.h"

// ���� : F3-3 Minigun �̴ϰ� : �̴ϰ��� ����ϸ� 3�����Ӹ��� 1�߾� 20���� �Ѿ��� ���������� �߻��մϴ�.
class MapModifier;
class WeaponMinigun : public Weapon
{
public:
	// constrcuter destructer
	WeaponMinigun();
	~WeaponMinigun();

	// delete Function
	WeaponMinigun(const WeaponMinigun& _Other) = delete;
	WeaponMinigun(WeaponMinigun&& _Other) noexcept = delete;
	WeaponMinigun& operator=(const WeaponMinigun& _Other) = delete;
	WeaponMinigun& operator=(WeaponMinigun&& _Other) noexcept = delete;

	void ResetWeapon();

protected:
	void Start() override;
	void Update(float _DeltaTime) override;

private:

	int BulletCount = 20;
	float DelayTime = 0.05f;

	GameEngineRender* AimingLine = nullptr;
	std::vector <GameEngineCollision*> MinigunCollision;							// index�� �ش��ϴ� �Ѿ� �ݸ���
	std::vector<bool> isShooted;			//���� �ʿ�								// index�� �ش��ϴ� �Ѿ��� �߻�Ǿ�����

	bool IsDone();
	void WeaponMinigunInit();														// �Ѿ� �߰�
	void Firing(float _DeltaTime);
	void CheckFiring();

};
