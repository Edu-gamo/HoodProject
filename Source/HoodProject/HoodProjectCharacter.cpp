// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "HoodProjectCharacter.h"
#include "HoodProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"

#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AHoodProjectCharacter

AHoodProjectCharacter::AHoodProjectCharacter()
{

	PrimaryActorTick.bCanEverTick = true; //Activa la funcion tick (update)

										  // Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);
}

void AHoodProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Update
void AHoodProjectCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime); // Call parent class tick function  

							//if (activePowerPressed) ActivePower();
	if (lastObjectOutlined != nullptr) {
		if (lastObjectOutlined->GetActor()->GetName().Equals("Keys")) {
			Cast<UPrimitiveComponent>(lastObjectOutlined->GetActor()->GetAttachParentActor()->GetRootComponent())->SetRenderCustomDepth(false);
		}
		else {
			lastObjectOutlined->GetComponent()->SetRenderCustomDepth(false);
		}
		lastObjectOutlined = nullptr;
	}
	lastObjectOutlined = ActivePower();

}

void AHoodProjectCharacter::NotifyActorBeginOverlap(AActor* other) {
	/*static ConstructorHelpers::FObjectFinder<USoundBase> Soundf(TEXT("/Musica/llaveColision_snd"));
	USoundBase* snd_key = Soundf.Object;
	UGameplayStatics::PlaySound2D(this, snd_key);*/
	if (other->GetName().Equals("Keys")) {
		if (lastObjectOutlined != nullptr) {
			if (lastObjectOutlined->GetActor()->GetName().Equals("Keys")) lastObjectOutlined = nullptr;
		}
		other->GetAttachParentActor()->Destroy();
		other->Destroy();
		hasKeys = true;
	}
}

//////////////////////////////////////////////////////////////////////////

// Input
void AHoodProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AHoodProjectCharacter::Crouch);

	PlayerInputComponent->BindAction("ChangePower", IE_Pressed, this, &AHoodProjectCharacter::ChangePower);

	//PlayerInputComponent->BindAxis("ChangePowerValue", this, &AHoodProjectCharacter::ChangePowerValue);

	//// Bind fire event (Powers)
	PlayerInputComponent->BindAction("ActivePower", IE_Pressed, this, &AHoodProjectCharacter::ActivatePower);
	PlayerInputComponent->BindAction("ActivePower", IE_Released, this, &AHoodProjectCharacter::DesactivatePower);
	/*PlayerInputComponent->BindAction("ActivePower", IE_Pressed, this, &AHoodProjectCharacter::ChangeActivePowerPressed);
	PlayerInputComponent->BindAction("ActivePower", IE_Released, this, &AHoodProjectCharacter::ChangeActivePowerPressed);*/

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AHoodProjectCharacter::ChangeInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AHoodProjectCharacter::ChangeInteract);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AHoodProjectCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AHoodProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHoodProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AHoodProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AHoodProjectCharacter::LookUpAtRate);
}

void AHoodProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AHoodProjectCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AHoodProjectCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AHoodProjectCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AHoodProjectCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AHoodProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHoodProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AHoodProjectCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AHoodProjectCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AHoodProjectCharacter::EndTouch);
		return true;
	}

	return false;
}

void AHoodProjectCharacter::ChangeActivePowerPressed() {
	activePowerPressed = !activePowerPressed;
}

void AHoodProjectCharacter::ActivatePower() {
	activePowerPressed = true;
}

void AHoodProjectCharacter::DesactivatePower() {
	activePowerPressed = false;
}

void AHoodProjectCharacter::ChangeInteract() {
	interact = !interact;
}

void AHoodProjectCharacter::ChangePower() {
	powerPush = !powerPush;
}

/*void AHoodProjectCharacter::ChangePowerValue(float value) {
	if (!isHoldingObject) {
		power += value * powerOffset;
		power = FMath::Clamp<float>(power, 0.f, maxPower);
	}
}*/

void AHoodProjectCharacter::Crouch() {
	if (!crouched) {
		ACharacter::Crouch(true);
		crouched = true;
	} else {
		ACharacter::UnCrouch(true);
		crouched = false;
	}
}

FHitResult* AHoodProjectCharacter::ActivePower() {

	FHitResult* hitResult = new FHitResult();

	FVector start = FirstPersonCameraComponent->GetComponentLocation();
	FVector forward = FirstPersonCameraComponent->GetForwardVector();
	FVector end = start + (forward * distancePower); //Distancia de efecto del poder
	FCollisionQueryParams* params = new FCollisionQueryParams();

	bool hitMetalObject = false;

	if (GetWorld()->LineTraceSingleByChannel(*hitResult, start, end, ECC_Visibility, *params)) {
		/*DrawDebugLine(GetWorld(), start, end, FColor::Red, true);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *hitResult->Actor->GetName()));*/
		if (hitResult->GetActor()->GetName().Equals("Keys")) {
			Cast<UPrimitiveComponent>(hitResult->GetActor()->GetAttachParentActor()->GetRootComponent())->SetRenderCustomDepth(true);
			hitMetalObject = true;
			if (activePowerPressed && power > 0) {
				Cast<UPrimitiveComponent>(hitResult->GetActor()->GetAttachParentActor()->GetRootComponent())->SetEnableGravity(false);
				Cast<UPrimitiveComponent>(hitResult->GetActor()->GetAttachParentActor()->GetRootComponent())->AddImpulse(forward * (powerPush ? power : -power));
				Cast<UPrimitiveComponent>(hitResult->GetActor()->GetAttachParentActor()->GetRootComponent())->SetEnableGravity(true);
			}
		}
		else {
			if (hitResult->GetComponent()->Mobility == EComponentMobility::Movable) {
				FString materialColision = hitResult->GetComponent()->GetMaterial(0)->GetName();
				if (materialColision.Contains("Metal")) {
					hitResult->GetComponent()->SetRenderCustomDepth(true);
					hitMetalObject = true;
					if (activePowerPressed && power > 0) {
						if (hitResult->GetComponent()->GetMass() < massLimitPower) { //Comprueba el peso del objeto
							hitResult->GetComponent()->SetEnableGravity(false);
							hitResult->GetComponent()->AddImpulse(forward * (powerPush ? power : -power));
							hitResult->GetComponent()->SetEnableGravity(true);
						} /*else {
							AddMovementInput(forward, (powerPush ? -power : power) / maxPower);
						}*/
					}
				}
			}
		}
	}

	hitPoint = hitResult->ImpactPoint;

	return hitMetalObject ? hitResult : nullptr;
}