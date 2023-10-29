// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Default4WVehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class CDO_REDUX_API ADefault4WVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()
	
public:

	struct FMyVehicleState
	{
		int NumWheelsOnGround;
		bool bAllWheelsOnGround;
		bool bVehicleInAir;
		bool bVehicleCanJump;
	};


	ADefault4WVehiclePawn();

	void BeginPlay();
	void Tick(float DeltaTime);

	void Throttle(float AxisValue);
	void BreakReverse(float AxisValue);
	void Steering(float AxisValue);
	

	void UpdateAirControl(float DeltaTime);

	void MouseLookX(float AxisValue);
	void MouseLookY(float AxisValue);
	void LookUpRate(float AxisValue);
	void TurnRate(float AxisValue);

	void ToggleCameraFollow();

	void OnHandBreakPressed();
	void OnHandBreakReleased();
	void Jump();
	void OnResetCameraPressed();

	void somersault();


	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


protected:

	FMyVehicleState VehicleState;

	float JumpResetTimer;
	int jumpCounter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|SpringArm")
		class USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera|Chase Camera")
		class UCameraComponent* ChaseCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character|Mesh")
		class USkeletalMeshComponent* CharacterMesh;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Controls|Jumping", meta = (DisplayName = "Max Number of Jumps", ClampMin = "0", UIMin = "0"))
		int JumpMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Controls|Jumping", meta = (DisplayName = "Jump Z Velocity", ClampMin = "0", UIMin = "0"))
		float JumpZVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Controls|Jumping", meta = (DisplayName = "Jump X Velocity", ClampMin = "0", UIMin = "0"))
		float JumpXVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Controls|Jumping", meta = (DisplayName = "Jump Y Velocity", ClampMin = "0", UIMin = "0"))
		float JumpYVelocity;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Air Controls|Jumping", meta = (DisplayName = "Reset Jump Time", ClampMin = "0", UIMin="0"))
		float JumpResetWait;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Controls|Pitch Force", meta = (DisplayName = "Pitch Force", ClampMin = "0", UIMin = "0"))
		float AirMovementPitchForce;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Controls|Roll Force", meta = (DisplayName = "Roll Force", ClampMin = "0", UIMin = "0"))
		float AirMovementYawForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller", meta = (DisplayName = "Controller Y Sensitivity"))
		float LookYRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller", meta = (DisplayName = "Controller X Sensitivity"))
		float TurnXRate;
};
