// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InteractableComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TP3PlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UInputAction;
class UInteractorComponent;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ATP3PlayerController : public APlayerController
{
	GENERATED_BODY()
	
	ATP3PlayerController();
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> GoldWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> GoldWidget;
	
	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;
	
	void OnInteractPressed();
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;
	
public:
	UFUNCTION()
	void HandlePawnChanged(APawn* PreviousPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleGoldChanged(int32 NewGold);


private:
	UPROPERTY()
	TObjectPtr<class ATP3Character> BoundCharacter;
};
