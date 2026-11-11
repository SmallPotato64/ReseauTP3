// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractFeedback, APawn*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractAction, APawn*, Instigator);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TP3_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();
	
	// Déclenché UNIQUEMENT sur le client propriétaire du Pawn lorsque celui-ci entre dans la sphère d'influence
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractFeedback OnEnterFeedback;
	
	// Déclenché UNIQUEMENT sur le client propriétaire du Pawn lorsque celui-ci sort de la sphère d'influence
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractFeedback OnExitFeedback;

	// Déclenché UNIQUEMENT sur le serveur lorsqu'une interaction est validée
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractAction OnInteract;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
