// Fill out your copyright notice in the Description page of Project Settings.


#include "Default4WVehiclePawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChaosWheeledVehicleMovementComponent.h"


ADefault4WVehiclePawn::ADefault4WVehiclePawn()
{
    UChaosWheeledVehicleMovementComponent* Vehicle = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    if (Vehicle == NULL) {
        UE_LOG(LogTemp, Error, TEXT("No Vehicle Component Found!"));
        return;
    }

#pragma region VehicleSetup
    //Default Torque Setup
    Vehicle->EngineSetup.MaxRPM = 5500.0f;
    Vehicle->EngineSetup.MaxTorque = 1000.0f;
    Vehicle->EngineSetup.EngineRevDownRate = 2400.0f;
    Vehicle->EngineSetup.EngineIdleRPM = 850.0f;
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(400.0f, 350.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1000.0f, 550.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1500.0f, 700.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(2000.0f, 800.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(3500.0f, 950.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(4500.0f, 1000.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5000.0f, 700.0f);
    Vehicle->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5500.0f, 0.0f);

    //Default Steering Setup
    Vehicle->SteeringSetup.SteeringCurve.GetRichCurve()->Reset();
    Vehicle->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
    Vehicle->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
    Vehicle->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(100.0f, 0.5f);

    //Default Transmission Setup
    Vehicle->TransmissionSetup.ForwardGearRatios.Reset();
    Vehicle->TransmissionSetup.ForwardGearRatios.Add(8.0f);
    Vehicle->TransmissionSetup.ForwardGearRatios.Add(5.0f);
    Vehicle->TransmissionSetup.ForwardGearRatios.Add(3.0f);
    Vehicle->TransmissionSetup.ForwardGearRatios.Add(2.8f);

    Vehicle->TransmissionSetup.ReverseGearRatios.Reset();
    Vehicle->TransmissionSetup.ReverseGearRatios.Add(8.0f);
    Vehicle->TransmissionSetup.ReverseGearRatios.Add(2.86f);

    Vehicle->DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
    Vehicle->DifferentialSetup.FrontRearSplit = 0.5f; 
    Vehicle->TransmissionSetup.TransmissionEfficiency = 1.0f;
    Vehicle->TransmissionSetup.bUseAutomaticGears = true;
    Vehicle->TransmissionSetup.GearChangeTime = 0.05f;
    Vehicle->TransmissionSetup.ChangeUpRPM = 4300.0f;
    Vehicle->TransmissionSetup.ChangeDownRPM = 1200.0f;

    Vehicle->DragCoefficient = 0.2f;

    AirMovementPitchForce = 3.0f;
    AirMovementYawForce = 3.0f;

#pragma endregion

#pragma region CamSetup

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));

    if (SpringArm == NULL) {
        UE_LOG(LogTemp, Error, TEXT("No Camera Arm Component Found!"));
        return;
    }

    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 800.0f;
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->bEnableCameraLag = true;

    SpringArm->CameraLagMaxDistance = 220.0f;
    SpringArm->CameraLagSpeed = 5.0f;
    SpringArm->CameraRotationLagSpeed = 10.0f;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCam"));

    if (ChaseCamera == NULL) {
        UE_LOG(LogTemp, Error, TEXT("No Camera Component Found!"));
        return;
    }

    ChaseCamera->SetupAttachment(SpringArm);
    ChaseCamera->bUsePawnControlRotation = false;
    ChaseCamera->FieldOfView = 110.0f;

#pragma endregion

    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Character"));
    CharacterMesh->SetupAttachment(RootComponent);

    TurnXRate = 3.0f;
    LookYRate = 3.0f;

    JumpResetWait = 0.15f;
    JumpMax = 2;
    JumpXVelocity = 1000.0f;
    JumpYVelocity = 1000.0f;
}

/// <summary>
/// Called Once At Start
/// </summary>
void ADefault4WVehiclePawn::BeginPlay()
{
    Super::BeginPlay(); //Call Parent Begin play function

    UE_LOG(LogTemp, Warning, TEXT("%s Spawned!"), *GetName());

    JumpResetTimer = JumpResetWait;
    jumpCounter = 0;
    VehicleState.bVehicleCanJump = true;
}

/// <summary>
/// Called Every Frame
/// </summary>
/// <param name="DeltaTime"></param>
void ADefault4WVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (jumpCounter >= JumpMax) VehicleState.bVehicleCanJump = false; //Prevent from being able to spam jump - bool reset on vehicleStateGrounded
    UpdateAirControl(DeltaTime);

    //UE_LOG(LogTemp, Display, TEXT("Velocity Z: %"), *GetVelocity().ToString());
}

#pragma region Throttle And Steering

void ADefault4WVehiclePawn::Throttle(float AxisValue)
{
    GetVehicleMovementComponent()->SetThrottleInput(AxisValue);
    /*
    if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, FString::Printf(TEXT("Speed (MPH): = %f"), GetVehicleMovementComponent()->GetForwardSpeedMPH()));
        }
     */
}

void ADefault4WVehiclePawn::BreakReverse(float AxisValue)
{
    GetVehicleMovementComponent()->SetBrakeInput(AxisValue);
}

void ADefault4WVehiclePawn::Steering(float AxisValue)
{
    GetVehicleMovementComponent()->SetSteeringInput(AxisValue);
}

#pragma endregion


#pragma region CameraControls

void ADefault4WVehiclePawn::MouseLookX(float AxisValue)
{

    if (AxisValue != 0)
    {
        AddControllerYawInput(AxisValue);
    }
}

void ADefault4WVehiclePawn::MouseLookY(float AxisValue)
{
    if (AxisValue != 0)
    {
        AddControllerPitchInput(AxisValue);
    }
}

void ADefault4WVehiclePawn::LookUpRate(float AxisValue)
{
    AddControllerPitchInput(-AxisValue * (LookYRate*10) * GetWorld()->GetDeltaSeconds());
}

void ADefault4WVehiclePawn::TurnRate(float AxisValue)
{
    AddControllerYawInput(AxisValue * (TurnXRate*10) * GetWorld()->GetDeltaSeconds());
}

void ADefault4WVehiclePawn::ToggleCameraFollow()
{
    SpringArm->bUsePawnControlRotation = !SpringArm->bUsePawnControlRotation;
}

#pragma endregion


void ADefault4WVehiclePawn::UpdateAirControl(float DeltaTime)
{
    if (UChaosWheeledVehicleMovementComponent* Vehicle = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
    {
        VehicleState.NumWheelsOnGround = 0;
        int NumWheels = 0;
        if (Vehicle->PhysicsVehicleOutput())
        {
            for (int WheelIdx = 0; WheelIdx < Vehicle->PhysicsVehicleOutput()->Wheels.Num(); WheelIdx++)
            {
                if (Vehicle->PhysicsVehicleOutput()->Wheels[WheelIdx].InContact)
                {
                    VehicleState.NumWheelsOnGround++;
                }
                NumWheels++;
            }
        }

        VehicleState.bAllWheelsOnGround = (VehicleState.NumWheelsOnGround == NumWheels);

        if (VehicleState.NumWheelsOnGround <= 1)
        {
             VehicleState.bVehicleInAir = true;
        }
        else
        {
            VehicleState.bVehicleInAir = false; //Vehicle isnt in the air

            if (VehicleState.bVehicleCanJump == false || jumpCounter != 0)
            {
                if (JumpResetTimer > 0)
                {
                    JumpResetTimer -= DeltaTime;
                }
                else
                {
                    JumpResetTimer = JumpResetWait;
                    VehicleState.bVehicleCanJump = true; //Reset can jump when vehicle is grounded
                    jumpCounter = 0;
                    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Reset Jump"));
                }

            }
        }
        //UE_LOG(LogTemp, Warning, TEXT("Num of Wheels found = %d  Num of Wheels on Ground: %d   AllWheelsOnGround = %s    Vehicle In Air = %s"), NumWheels, VehicleState.NumWheelsOnGround, VehicleState.bAllWheelsOnGround ? TEXT("T") : TEXT("F"), VehicleState.bVehicleInAir ? TEXT("T") : TEXT("F"));

        if (VehicleState.bVehicleInAir)
        {
            //if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Aired out"));
            if(!IsPlayerControlled()) return;
            
            const float FwdInput = InputComponent->GetAxisValue("AirPitch");
            const float RtInput = InputComponent->GetAxisValue("Steering");

            if (UPrimitiveComponent* VehicleMesh = Vehicle->UpdatedPrimitive) {
                const FVector MovementVector = FVector(0.f, FwdInput * AirMovementPitchForce, RtInput * AirMovementYawForce) * DeltaTime * 10.0f;
                const FVector NewAngularMvmnt = GetActorRotation().RotateVector(MovementVector);

                VehicleMesh->SetPhysicsAngularVelocityInDegrees(NewAngularMvmnt, true);
            }
        }
    }
}

void ADefault4WVehiclePawn::Jump()
{
    if (VehicleState.bVehicleCanJump)
    {
        //jump
        FVector Velocity = GetVelocity(); //Get Velocity of Actor
        
        if (UChaosWheeledVehicleMovementComponent* Vehicle = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
        {
            if (UPrimitiveComponent* VMesh = Vehicle->UpdatedPrimitive)
            {
                
                if (jumpCounter <= JumpMax-2) //If not final jump
                {
                    Velocity.Z = FMath::Max<FVector::FReal>(Velocity.Z, JumpZVelocity); //Set Saved Velocity to max value of either Current Actor velocity or Given Jump Velocity
                    VMesh->SetAllPhysicsLinearVelocity(Velocity);
                }
                else //When Final Jump
                {
                    //Jump In direction + flip
                    const float fwd = InputComponent->GetAxisValue("AirPitch");
                    const float right = InputComponent->GetAxisValue("Steering");

                    const FVector MvmntVector = FVector(fwd *JumpXVelocity, right * JumpYVelocity, 0.0f);
                    const FVector LocalMvmnt = GetActorRotation().RotateVector(MvmntVector);

                    Velocity.Z = FMath::Max<FVector::FReal>(Velocity.Z, JumpZVelocity/2); //Set Saved Velocity to max value of either Current Actor velocity or Given Jump Velocity
                    VMesh->SetAllPhysicsLinearVelocity(Velocity);
                    VMesh->SetAllPhysicsLinearVelocity(LocalMvmnt,true);

                    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Did Final Jump"));
                }
            }
        }
        jumpCounter++;
    }
}

void ADefault4WVehiclePawn::OnHandBreakPressed()
{
    if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Apply Handbreak"));
    GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void ADefault4WVehiclePawn::OnHandBreakReleased()
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Release Handbreak"));
    GetVehicleMovementComponent()->SetHandbrakeInput(false);
}


void ADefault4WVehiclePawn::OnResetCameraPressed()
{
    Controller->SetControlRotation(GetActorRotation()); //ChaseCamera->GetForwardVector().Rotation()
    ToggleCameraFollow();
}

void ADefault4WVehiclePawn::somersault()
{


}


void ADefault4WVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Handbreak", IE_Pressed, this, &ADefault4WVehiclePawn::OnHandBreakPressed);
    PlayerInputComponent->BindAction("Handbreak", IE_Released, this, &ADefault4WVehiclePawn::OnHandBreakReleased);
    PlayerInputComponent->BindAction("Jump",IE_Pressed, this, &ADefault4WVehiclePawn::Jump);
    PlayerInputComponent->BindAction("ResetCamera", IE_Pressed, this, &ADefault4WVehiclePawn::OnResetCameraPressed);
    PlayerInputComponent->BindAxis("Throttle", this, &ADefault4WVehiclePawn::Throttle);
    PlayerInputComponent->BindAxis("Break", this, &ADefault4WVehiclePawn::BreakReverse);
    PlayerInputComponent->BindAxis("Steering", this, &ADefault4WVehiclePawn::Steering);
    PlayerInputComponent->BindAxis("LookY", this, &ADefault4WVehiclePawn::MouseLookY);
    PlayerInputComponent->BindAxis("LookX", this, &ADefault4WVehiclePawn::MouseLookX);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ADefault4WVehiclePawn::LookUpRate);
    PlayerInputComponent->BindAxis("TurnRate", this, &ADefault4WVehiclePawn::TurnRate);
    PlayerInputComponent->BindAxis("AirPitch");
}
