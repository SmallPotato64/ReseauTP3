// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP3PlayerController.h"
#include "TP3Character.h"
#include "GoldWidget.h"
#include "InteractorComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "TP3.h"
#include "Widgets/Input/SVirtualJoystick.h"

ATP3PlayerController::ATP3PlayerController()
{
	// Le PlayerController est déjà répliqué par défaut, mais on s'assure que le composant
	// qui gère les interactions le soit aussi pour que le serveur puisse valider les RPC.
	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("InteractorComponent"));
}

void ATP3PlayerController::OnInteractPressed()
{
	// On délègue au composant d'interaction
	UE_LOG(LogTemp, Warning, TEXT("Interact Pressed!"));
	if (InteractorComponent)
	{
		InteractorComponent->Interact();
	}
}

void ATP3PlayerController::HandlePawnChanged(APawn* PreviousPawn, APawn* NewPawn)
{
	// Désabonnement de l'ancien Pawn.
	if (BoundCharacter)
	{
		BoundCharacter->OnGoldChanged.RemoveDynamic(this, &ATP3PlayerController::HandleGoldChanged);
		BoundCharacter = nullptr;
	}

	// Abonnement au nouveau Pawn, s'il s'agit bien de notre ATP3Character.
	if (ATP3Character* NewCharacter = Cast<ATP3Character>(NewPawn))
	{
		BoundCharacter = NewCharacter;
		BoundCharacter->OnGoldChanged.AddDynamic(this, &ATP3PlayerController::HandleGoldChanged);

		// Pousse la valeur courante au widget dès la possession.
		if (UGoldWidget* Widget = Cast<UGoldWidget>(GoldWidget))
		{
			Widget->SetGold(NewCharacter->GetGold());
		}
	}
}

void ATP3PlayerController::HandleGoldChanged(int32 NewGold)
{
	if (UGoldWidget* Widget = Cast<UGoldWidget>(GoldWidget))
	{
		Widget->SetGold(NewGold);
	}
}

void ATP3PlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	
	if (IsLocalPlayerController())
	{
		if (GoldWidgetClass)
		{
			GoldWidget = CreateWidget<UUserWidget>(this, GoldWidgetClass);
			if (GoldWidget)
			{
				GoldWidget->AddToPlayerScreen(0);
			}
		}

		OnPossessedPawnChanged.AddDynamic(this, &ATP3PlayerController::HandlePawnChanged);

		if (APawn* InitialPawn = GetPawn())
		{
			HandlePawnChanged(nullptr, InitialPawn);
		}
		
		// only spawn touch controls on local player controllers
		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);
			} else {

				UE_LOG(LogTP3, Error, TEXT("Could not spawn mobile controls widget."));
			}
		}
	}
	
}

void ATP3PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InteractAction)
		{
			// On utilise ETriggerEvent::Started pour déclencher l'interaction
			// dès que la touche est pressée, une seule fois.
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ATP3PlayerController::OnInteractPressed);
		}
	}
}

bool ATP3PlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
