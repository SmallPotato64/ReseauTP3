// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractorComponent.generated.h"

class UInteractableComponent;
class USphereComponent;
class APawn;
class APlayerController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TP3_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractorComponent();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Lie les délégués d'overlap de la sphère du nouveau Pawn et nettoie l'ancien
	void BindToPawn(APawn* NewPawn);
	void UnbindFromPawn();

	// Callback du délégué OnPossessedPawnChanged du PlayerController
	UFUNCTION()
	void HandlePawnChanged(APawn* PreviousPawn, APawn* NewPawn);

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Retourne l'acteur interactable visé par la caméra, s'il est à portée
	AActor* GetTargetedInteractable() const;
	
	// Envoie la demande d'interaction au serveur
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(AActor* TargetActor);
		
private:
	// Pointeur vers le PlayerController propriétaire
	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayerController;

	// Pointeur vers le Pawn actuellement possédé
	UPROPERTY()
	TObjectPtr<APawn> CurrentPawn;

	/// Sphère d'influence du Pawn actuellement lié
	UPROPERTY()
	TObjectPtr<USphereComponent> CurrentInfluenceSphere;

	/// Liste des acteurs interactables actuellement à portée
	UPROPERTY()
	TArray<TObjectPtr<AActor>> NearbyInteractables;

	// Portée maximale du trace de ciblage
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TraceDistance = 900.f;
};
