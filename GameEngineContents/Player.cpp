#include "Player.h"

#include<iostream>

#include <GameEnginePlatform/GameEngineWindow.h>
#include <GameEngineBase/GameEnginePath.h>
#include <GameEngineCore/GameEngineResources.h>
#include <GameEngineCore/GameEngineRender.h>
#include <GameEnginePlatform/GameEngineInput.h>
#include <GameEngineCore/GameEngineLevel.h>
#include <GameEngineCore/GameEngineCollision.h>

#include "PlayerHPUI.h"
#include "PlayerGetDamagedUI.h"
#include "ContentsEnums.h"
#include "Weapon.h"
#include "PlayerGrave.h"

Player::Player() 
{
}

Player::~Player() 
{
}

void Player::Start()
{
	AnimationRender = CreateRender(WormsRenderOrder::Player);
	AnimationRender->SetScale({ 60, 60 });
	AnimationRender->SetPosition({ 0, -10 });
    CreateAnimation();

	//콜리전
	{
		BodyCollision = CreateCollision(WormsCollisionOrder::Player);
		BodyCollision->SetMove({ 0, -10 });
		BodyCollision->SetScale({ 30 , 30 });
	}

	//키 설정이 안되어있으면 키 설정( 플레이어가 여러명 일 경우를 대비하여 키 설정이 없으면 설정함 ) 
	if (GameEngineInput::IsKey("MoveLeft") == false)
	{
		//캐릭터 이동 및 행동
		GameEngineInput::CreateKey("MoveRight", 'D');
		GameEngineInput::CreateKey("MoveLeft", 'A');
		GameEngineInput::CreateKey("Jump", 'K');

		GameEngineInput::CreateKey("TestButton", 'M');
	}

    PlayerGraveImageStringView = "Grave5.bmp";

	SetHPUI("RedNumberRender.bmp", "RedNameTag.bmp", "PlayerSelectArrowRed.bmp");
	ChangeState(PlayerState::IDLE);
}

void Player::SetHPUI(const std::string_view& _HPNumberImage, const std::string_view& _NametagImage, const std::string_view& _ArrowImage) //HP 이미지 등을 세팅해주는 메서드
{
	PlayerHPNumberImageStringView = _HPNumberImage;

	HPUI = GetLevel()->CreateActor<PlayerHPUI>();
	HPUI->SetPlayerHPUI(_HPNumberImage, _NametagImage, _ArrowImage, &PlayerHP);
}

void Player::SetIsMyTurn(bool _Value)
{
	IsMyTurn = _Value;
	HPUI->SetSelectPlayerRender(_Value);
}

void Player::SetColImage(const std::string_view& _Name)
{
	ColImage = GameEngineResources::GetInst().ImageFind(_Name.data());
}

void Player::Test()
{
	if (GameEngineInput::IsDown("TestButton") && IsMyTurn == true)
	{
		Damaged(100);
        //ChangeState(PlayerState::Win);
	}
}

void Player::CheckAlive()
{
    if (PlayerHP <= 0)
    {
        TestChangeDeadState();
    }
}

void Player::TestChangeDeadState()
{
    if (true == IsAlive)
    {
        ChangeState(PlayerState::Dead);
        IsAlive = false;
    }
}

void Player::Damaged(int _Damage, float4 _Dir, float _Power)
{
	if (DamagedTime >= 0.5f)
	{
		PlayerHP -= _Damage;
		DamagedTime = 0.0f;
		
		PlayerGetDamagedUI* DamagedUI = GetLevel()->CreateActor<PlayerGetDamagedUI>();
		DamagedUI->SetDamagedUI(PlayerHPNumberImageStringView, _Damage);

		float4 DamagedUIPos = { HPUI->GetPos().x, HPUI->GetPos().y - 20.0f };
		DamagedUI->SetPos(DamagedUIPos);
	}
}

void Player::CheckTurn()
{
    if (false == IsMyTurn || nullptr == CurWeapon)
    {
        return;
    }

    if (true == CurWeapon->IsWeaponDone())
    {
        IsMyTurn = false;
        CurWeapon->Death();
        CurWeapon = nullptr;
    }
}

void Player::Update(float _DeltaTime)
{
	//MoveDir = float4::Zero; //매 프레임마다 MoveDir 초기화
    PlayerPixelCheck();
    UpdateState(_DeltaTime);
    GravityApplied(_DeltaTime);
    MoveCalculation(_DeltaTime);
	
    CheckAlive();

    IsGroundCheck();
    CheckTurn();
    Test();

	HPUI->SetPos({GetPos().x , GetPos().y - 50.0f}); //UI 프레임마다 위치 조정
	DamagedTime += _DeltaTime; //플레이어가 한번에 여러번의 데미지를 받지않기 위한 변수
}

void Player::GravityApplied(float _DeltaTime)
{
	MoveDir += (float4::Down * Gravity * _DeltaTime);
}

void Player::MoveCalculation(float _DeltaTime)
{
	float4 NextPos = GetPos() + (MoveDir * _DeltaTime);
	NextPos = PullUpCharacter(NextPos, _DeltaTime);

	SetMoveAngle();

    if (true == IsGround)
    {
        //움직이는 상태가 아니면 리턴함 중력으로 인해 점점 떨어지는 버그를 수정하기 위해
        if (PlayerState::Dead == StateValue ||
            PlayerState::IDLE == StateValue ||
            PlayerState::EQUIPWEAPON == StateValue ||
            PlayerState::Win == StateValue )
        {
            return;
        }
    }

    SetMove(MoveDir * _DeltaTime);

}

void Player::IsGroundCheck()
{
	float4 PlayerGroundCheckPos = { GetPos().x , GetPos().y + 1 };

	if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerGroundCheckPos, RGB(0, 0, 0)))
	{
		IsGround = true;
	}
	else
	{
		IsGround = false;
	}
}

void Player::PlayerPixelCheck()
{
    float4 PlayerLeftPixel = { GetPos().x - 10, GetPos().y - 10 };
    float4 PlayerRightPixel = { GetPos().x + 10, GetPos().y - 10 };
    float4 PlayerUpPixel = { GetPos().x , GetPos().y - 20 };
    float4 PlayerDownPixel = { GetPos().x , GetPos().y + 1 };
    float4 PlayerLeftUpPixel = { GetPos().x - 10, GetPos().y - 20};
    float4 PlayerRightUpPixel = { GetPos().x + 10, GetPos().y - 20 };

    if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerLeftPixel, RGB(0, 0, 0)))
    {
        LeftPixelCheck = true;
    }
    else
    {
        LeftPixelCheck = false;
    }

    if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerRightPixel, RGB(0, 0, 0)))
    {
        RightPixelCheck = true;
    }
    else
    {
        RightPixelCheck = false;
    }

    if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerUpPixel, RGB(0, 0, 0)))
    {
        UpPixelCheck = true;
    }
    else
    {
        UpPixelCheck = false;
    }

    if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerDownPixel, RGB(0, 0, 0)))
    {
        DownPixelCheck = true;
    }
    else
    {
        DownPixelCheck = false;
    }

    if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerLeftUpPixel, RGB(0, 0, 0)))
    {
        LeftUpPixelCheck = true;
    }
    else
    {
        LeftUpPixelCheck = false;
    }

    if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerRightUpPixel, RGB(0, 0, 0)))
    {
        RightUpPixelCheck = true;
    }
    else
    {
        RightUpPixelCheck = false;
    }
}

void Player::SetMoveAngle()
{
	float4 PlayerLeftPos = { GetPos().x - 5, GetPos().y };
	float4 PlayerRightPos = { GetPos().x + 5, GetPos().y };

	if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerLeftPos, RGB(0, 0, 0)))
	{
		while (true)
		{
			PlayerLeftPos = { PlayerLeftPos.x, PlayerLeftPos.y - 1 };

			if (RGB(0, 0, 255) != ColImage->GetPixelColor(PlayerLeftPos, RGB(0, 0, 0)))
			{
				break;
			}
		}
	}

	if (RGB(0, 0, 255) == ColImage->GetPixelColor(PlayerRightPos, RGB(0, 0, 0)))
	{
		while (true)
		{
			PlayerRightPos = { PlayerRightPos.x, PlayerRightPos.y - 1 };

			if (RGB(0, 0, 255) != ColImage->GetPixelColor(PlayerRightPos, RGB(0, 0, 0)))
			{
				break;
			}
		}

	}

	float4 LeftAngleVector = GetPos() - PlayerLeftPos;
	LeftMoveAngle = LeftAngleVector.NormalizeReturn().GetAnagleDeg();

	float4 RightAngleVector = GetPos() - PlayerRightPos;
	RightMoveAngle = RightAngleVector.NormalizeReturn().GetAnagleDeg();
}

bool Player::ReturnCanIMove(PlayerAngleDir _Dir)
{
    if (PlayerAngleDir::Left == _Dir)
    {
        if ((LeftMoveAngle <= 1.0f && LeftMoveAngle >= -1.0f) || LeftMoveAngle >= 270 + AngleLimit)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if ((RightMoveAngle <= 181.0f && RightMoveAngle >= 179.0f) || RightMoveAngle <= 270 - AngleLimit)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

float4 Player::PullUpCharacter(float4 _NextPos, float _DeltaTime)
{
	if (RGB(0, 0, 255) != ColImage->GetPixelColor(_NextPos, RGB(0, 0, 255)))
	{
		return _NextPos;
	}

    float4 NextMoveDir = MoveDir;

	while (true)
	{
		MoveDir.y -= 1;

		_NextPos = GetPos() + MoveDir * _DeltaTime;

		if (RGB(0, 0, 255) == ColImage->GetPixelColor(_NextPos, RGB(0, 0, 255)))
		{
			continue;
		}

		return _NextPos;
	}
}


void Player::DirCheck(const std::string_view& _AnimationName, int _CurIndex)
{
	//std::string PrevDirString = DirString;

 //   if (PlayerState::EQUIPWEAPON == StateValue)
 //   {
 //       AnimationRender->ChangeAnimation(DirString + _AnimationName.data(), _CurIndex, true);
 //   }
 //   else
 //   {
 //       AnimationRender->ChangeAnimation(DirString + _AnimationName.data(), _CurIndex);
 //   }

	//if (GameEngineInput::IsPress("MoveLeft"))
	//{
	//	DirString = "Left_";
	//}
	//else if (GameEngineInput::IsPress("MoveRight"))
	//{
	//	DirString = "Right_";
	//}

	//if (PrevDirString != DirString)
	//{
	//	AnimationRender->ChangeAnimation(DirString + _AnimationName.data(), _CurIndex, true);
	//}

    std::string PrevDirString = DirString;

    if (PlayerState::EQUIPWEAPON == StateValue)
    {
        AnimationRender->ChangeAnimation(DirString + _AnimationName.data(), _CurIndex, true);
    }
    else
    {
        AnimationRender->ChangeAnimation(DirString + _AnimationName.data(), _CurIndex);
    }

    if (true == IsMyTurn)
    {
        if (GameEngineInput::IsDown("MoveLeft"))
        {
            DirString = "Left_";
        }

        if (GameEngineInput::IsDown("MoveRight"))
        {
            DirString = "Right_";
        }
    }

    if (PrevDirString != DirString)
    {
        AnimationRender->ChangeAnimation(DirString + _AnimationName.data(), _CurIndex, true);
    }
}

void Player::CreatePlayerAnimation(const std::string_view& _AnimationName, const std::string_view& _ImageName, int _StartFrame, int _EndFrame, float _InterTime, bool _Loop)
{
	AnimationRender->CreateAnimation({ .AnimationName = _AnimationName, .ImageName = _ImageName, .Start = _StartFrame, .End = _EndFrame, .InterTime = _InterTime, .Loop = _Loop });
}

void Player::ChangePlayerAnimation(const std::string_view& _AnimationName, int _CurIndex)
{
	DirCheck(_AnimationName, _CurIndex);
}

void Player::SetPlayerAnimationFrame(int _Frame)
{
	AnimationRender->SetAnimationFrame(_Frame);
}

void Player::Render(float _DeltaTime)
{
	HDC DoubleDC = GameEngineWindow::GetDoubleBufferImage()->GetImageDC();
	//float4 ActorPos = GetPos() - GetLevel()->GetCameraPos();

	////위치 확인용
	//Rectangle(DoubleDC,
	//	ActorPos.ix() - 5,
	//	ActorPos.iy() - 5,
	//	ActorPos.ix() + 5,
	//	ActorPos.iy() + 5
	//);

	if (true == IsMyTurn)
	{
		std::string PlayerLeftAngle = "PlayerLeftAngle = ";
		PlayerLeftAngle = PlayerLeftAngle + std::to_string(LeftMoveAngle);
		GameEngineLevel::DebugTextPush(PlayerLeftAngle);

		std::string PlayerRightAngle = "PlayerRightAngle = ";
		PlayerRightAngle = PlayerRightAngle + std::to_string(RightMoveAngle);
		GameEngineLevel::DebugTextPush(PlayerRightAngle);

		std::string PlayerIsGround = "PlayerIsGround = ";
		PlayerIsGround = PlayerIsGround + std::to_string(IsGround);
		GameEngineLevel::DebugTextPush(PlayerIsGround);

        //float4 NextPos = (GetPos() + MoveDir * _DeltaTime) - GetLevel()->GetCameraPos();

        //Rectangle(DoubleDC,
        //    NextPos.ix() - 5,
        //    NextPos.iy() - 5,
        //    NextPos.ix() + 5,
        //    NextPos.iy() + 5
        //);

        float4 PlayerLeftPixel = { GetPos().x - 10, GetPos().y - 10 };
        float4 PlayerRightPixel = { GetPos().x + 10, GetPos().y - 10 };
        float4 PlayerUpPixel = { GetPos().x , GetPos().y - 20 };
        float4 PlayerDownPixel = { GetPos().x , GetPos().y + 1 };

        PlayerLeftPixel -= GetLevel()->GetCameraPos();
        PlayerRightPixel -= GetLevel()->GetCameraPos();
        PlayerUpPixel -= GetLevel()->GetCameraPos();
        PlayerDownPixel -= GetLevel()->GetCameraPos();

        Rectangle(DoubleDC,
            PlayerLeftPixel.ix() - 1,
            PlayerLeftPixel.iy() - 1,
            PlayerLeftPixel.ix() + 1,
            PlayerLeftPixel.iy() + 1
        );
        Rectangle(DoubleDC,
            PlayerRightPixel.ix() - 1,
            PlayerRightPixel.iy() - 1,
            PlayerRightPixel.ix() + 1,
            PlayerRightPixel.iy() + 1
        );
        Rectangle(DoubleDC,
            PlayerUpPixel.ix() - 1,
            PlayerUpPixel.iy() - 1,
            PlayerUpPixel.ix() + 1,
            PlayerUpPixel.iy() + 1
        );
        Rectangle(DoubleDC,
            PlayerDownPixel.ix() - 1,
            PlayerDownPixel.iy() - 1,
            PlayerDownPixel.ix() + 1,
            PlayerDownPixel.iy() + 1
        );

        if (nullptr != CurWeapon)
        {
            std::string PlayerCurWeapon = "PlayerCurWeapon = ";
            PlayerCurWeapon = PlayerCurWeapon + std::to_string(CurWeapon->GetWeaponNumber());
            GameEngineLevel::DebugTextPush(PlayerCurWeapon);
        }

        std::string PlayerStateText = "PlayerState = ";

        switch (StateValue)
        {
        case PlayerState::IDLE:
            PlayerStateText = PlayerStateText + "IDLE 상태";
            break;
        case PlayerState::MOVE:
            PlayerStateText = PlayerStateText + "MOVE 상태";
            break;
        case PlayerState::JUMP:
            PlayerStateText = PlayerStateText + "JUMP 상태";
            break;
        case PlayerState::EQUIPWEAPON:
            PlayerStateText = PlayerStateText + "EQUIPWEAPON 상태";
            break;
        default:
            PlayerStateText = PlayerStateText + "UNKNOWN 상태";
            break;
        }
        GameEngineLevel::DebugTextPush(PlayerStateText);

        std::string LeftPixelCheckString = "LeftPixelCheck = ";
        LeftPixelCheckString = LeftPixelCheckString + std::to_string(LeftPixelCheck);
        GameEngineLevel::DebugTextPush(LeftPixelCheckString);

        std::string RightPixelCheckString = "RightPixelCheck = ";
        RightPixelCheckString = RightPixelCheckString + std::to_string(RightPixelCheck);
        GameEngineLevel::DebugTextPush(RightPixelCheckString);

        std::string UpPixelCheckString = "UpPixelCheck = ";
        UpPixelCheckString = UpPixelCheckString + std::to_string(UpPixelCheck);
        GameEngineLevel::DebugTextPush(UpPixelCheckString);

        std::string DownPixelCheckString = "DownPixelCheck = ";
        DownPixelCheckString = DownPixelCheckString + std::to_string(DownPixelCheck);
        GameEngineLevel::DebugTextPush(DownPixelCheckString);

        std::string LeftUpPixelCheckString = "LeftUpPixelCheck = ";
        LeftUpPixelCheckString = LeftUpPixelCheckString + std::to_string(LeftUpPixelCheck);
        GameEngineLevel::DebugTextPush(LeftUpPixelCheckString);

        std::string RightUpPixelCheckString = "RightUpPixelCheck = ";
        RightUpPixelCheckString = RightUpPixelCheckString + std::to_string(RightUpPixelCheck);
        GameEngineLevel::DebugTextPush(RightUpPixelCheckString);
	}

}

void Player::PlayerDead()
{
    Off();
    HPUI->Off();
    SetGraveObject(PlayerGraveImageStringView);
}

void Player::SetGraveObject(const std::string_view& _GraveImage)
{
    PlayerGrave* GraveObject = GetLevel()->CreateActor<PlayerGrave>();
    GraveObject->SetPlayerGrave(_GraveImage, ColImage ,GetPos());
}

bool Player::IsPlayerAnimationEnd()
{
	return AnimationRender->IsAnimationEnd();
}