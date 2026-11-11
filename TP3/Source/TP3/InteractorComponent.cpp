// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractorComponent.h"
#include "InteractableComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

// Sets default values for this component's properties
UInteractorComponent::UInteractorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerController = Cast<APlayerController>(GetOwner());
	if (!OwningPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UInteractorComponent doit être attaché à un PlayerController."));
		return;
	}

	// On s'abonne au changement de Pawn pour survivre aux morts/respawns/re-possessions
	OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UInteractorComponent::HandlePawnChanged);

	// Si un Pawn est déjà possédé au BeginPlay, on s'y lie immédiatement.
	if (APawn* InitialPawn = OwningPlayerController->GetPawn())
	{
		BindToPawn(InitialPawn);
	}
	
}

void UInteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// On retire les délégués pour éviter les pointeurs pendants.
	UnbindFromPawn();

	if (OwningPlayerController)
	{
		OwningPlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &UInteractorComponent::HandlePawnChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UInteractorComponent::HandlePawnChanged(APawn* PreviousPawn, APawn* NewPawn)
{
    // On commence toujours par se délier de l'ancien Pawn.
    UnbindFromPawn();

    // Puis on se lie au nouveau, s'il existe.
    if (NewPawn)
    {
        BindToPawn(NewPawn);
    }
}

void UInteractorComponent::BindToPawn(APawn* NewPawn)
{
    if (!NewPawn) return;

    CurrentPawn = NewPawn;

    // On cherche la sphère d'influence sur le Pawn
    CurrentInfluenceSphere = NewPawn->FindComponentByClass<USphereComponent>();
    if (!CurrentInfluenceSphere)
    {
        UE_LOG(LogTemp, Warning, TEXT("Le Pawn %s n'a pas de USphereComponent d'influence."), *NewPawn->GetName());
        return;
    }

    // On s'abonne aux overlaps de la sphère
    CurrentInfluenceSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractorComponent::OnSphereBeginOverlap);
    CurrentInfluenceSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractorComponent::OnSphereEndOverlap);
}

void UInteractorComponent::UnbindFromPawn()
{
    if (CurrentInfluenceSphere)
    {
        CurrentInfluenceSphere->OnComponentBeginOverlap.RemoveDynamic(this, &UInteractorComponent::OnSphereBeginOverlap);
        CurrentInfluenceSphere->OnComponentEndOverlap.RemoveDynamic(this, &UInteractorComponent::OnSphereEndOverlap);
    }

    CurrentInfluenceSphere = nullptr;
    CurrentPawn = nullptr;
    NearbyInteractables.Empty();
}

void UInteractorComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Sécurité : les feedbacks ne doivent être déclenchés que sur le client propriétaire.
    // Sur le serveur, on ne fait rien (le serveur ne gère pas les feedbacks cosmétiques).
    if (!OwningPlayerController || !OwningPlayerController->IsLocalController())
    {
        return;
    }

    if (!OtherActor || OtherActor == CurrentPawn)
    {
        return;
    }

    // On vérifie que l'acteur possède bien un UInteractableComponent.
    UInteractableComponent* Interactable = OtherActor->FindComponentByClass<UInteractableComponent>();
    if (!Interactable)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Player influence sphere hit something!"));

    // Ajout à la liste des objets à portée.
    if (!NearbyInteractables.Contains(OtherActor))
    {
        NearbyInteractables.Add(OtherActor);
    }

    // Déclenchement du feedback local, en passant le Pawn possédé comme Instigator.
    Interactable->OnEnterFeedback.Broadcast(CurrentPawn);
}

void UInteractorComponent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OwningPlayerController || !OwningPlayerController->IsLocalController())
    {
        return;
    }

    if (!OtherActor || !NearbyInteractables.Contains(OtherActor))
    {
        return;
    }

    UInteractableComponent* Interactable = OtherActor->FindComponentByClass<UInteractableComponent>();
    if (Interactable)
    {
        Interactable->OnExitFeedback.Broadcast(CurrentPawn);
    }

    NearbyInteractables.Remove(OtherActor);
}


// Ciblage par trace caméra
AActor* UInteractorComponent::GetTargetedInteractable() const
{
    if (!OwningPlayerController || !CurrentPawn || NearbyInteractables.IsEmpty())
    {
        return nullptr;
    }

    // On utilise le PlayerCameraManager pour obtenir une position de caméra fiable côté client
    APlayerCameraManager* CameraManager = OwningPlayerController->PlayerCameraManager;
    if (!CameraManager)
    {
        return nullptr;
    }

    const FVector TraceStart = CameraManager->GetCameraLocation();
    const FVector TraceEnd = TraceStart + CameraManager->GetCameraRotation().Vector() * TraceDistance;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(CurrentPawn);
    QueryParams.bTraceComplex = false;

    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (!bHit || !Hit.GetActor())
    {
        return nullptr;
    }

    AActor* HitActor = Hit.GetActor();

    // La cible n'est valide que si elle possède un UInteractableComponent et qu'elle est bien dans la liste des objets à portée
    if (HitActor->FindComponentByClass<UInteractableComponent>() && NearbyInteractables.Contains(HitActor))
    {
        return HitActor;
    }

    return nullptr;
}

// Point d'entrée côté client
void UInteractorComponent::Interact()
{
    UE_LOG(LogTemp, Warning, TEXT("Interact begun!"));
    // Sur l'hôte (Listen Server), le PlayerController a l'autorité.
    // On exécute directement la logique serveur, sans passer par la RPC.
    if (OwningPlayerController && OwningPlayerController->HasAuthority())
    {
        if (AActor* Target = GetTargetedInteractable())
        {
            if (UInteractableComponent* Interactable = Target->FindComponentByClass<UInteractableComponent>())
            {
                Interactable->OnInteract.Broadcast(CurrentPawn);
                UE_LOG(LogTemp, Warning, TEXT("Interact Brodcaster (Host)!"));
            }
        }
        return;
    }

    // Sur un client distant : on envoie la demande au serveur.
    if (AActor* Target = GetTargetedInteractable())
    {
        Server_Interact(Target);
    }
}

// Server RPC avec validation
bool UInteractorComponent::Server_Interact_Validate(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return false;
    }

    // Le Pawn de l'appelant existe.
    APawn* Pawn = OwningPlayerController ? OwningPlayerController->GetPawn() : nullptr;
    if (!Pawn)
    {
        return false;
    }

    // La cible possède un UInteractableComponent.
    UInteractableComponent* Interactable = TargetActor->FindComponentByClass<UInteractableComponent>();
    if (!Interactable)
    {
        return false;
    }

    // La cible est réellement à portée du Pawn
    USphereComponent* Sphere = Pawn->FindComponentByClass<USphereComponent>();
    if (!Sphere)
    {
        return false;
    }

    const float DistanceSq = FVector::DistSquared(Pawn->GetActorLocation(), TargetActor->GetActorLocation());
    const float SphereRadius = Sphere->GetScaledSphereRadius();
    const float MaxDistanceSq = FMath::Square(SphereRadius + 50.f); // Marge de tolérance

    return DistanceSq <= MaxDistanceSq;
}

void UInteractorComponent::Server_Interact_Implementation(AActor* TargetActor)
{
    // Si on arrive ici, la validation a réussi.
    // On déclenche l'événement OnInteract sur le composant interactif ciblé.
    if (UInteractableComponent* Interactable = TargetActor->FindComponentByClass<UInteractableComponent>())
    {
        APawn* InstigatorPawn = OwningPlayerController ? OwningPlayerController->GetPawn() : nullptr;
        Interactable->OnInteract.Broadcast(InstigatorPawn);
        UE_LOG(LogTemp, Warning, TEXT("Interact Brodcasted (Client)!"));
    }
}